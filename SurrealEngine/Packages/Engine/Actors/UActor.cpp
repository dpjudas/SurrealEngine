
#include "Precomp.h"
#include "UActor.h"
#include "Package/PackageManager.h"
#include "Packages/Core/UClass.h"
#include "Packages/Engine/UViewport.h"
#include "Packages/Engine/Actors/UProjectile.h"
#include "Packages/Engine/Actors/USpawnNotify.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Info/UGameInfo.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Actors/Inventory/UInventory.h"
#include "Packages/Engine/Actors/Decoration/UDecoration.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Packages/Engine/Actors/Keypoint/UInterpolationPoint.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Packages/Engine/Resources/Mesh/USkeletalMesh.h"
#include "Packages/Engine/Subsystems/USurrealAudioDevice.h"
#include "Packages/ConSys/UConversation.h"
#include "Packages/ConSys/UConversationList.h"
#include "Packages/ConSys/UConListItem.h"
#include "Packages/ConSys/UConItem.h"
#include "Utils/Logger.h"
#include "Engine.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"

UActor* UActor::Spawn(UClass* SpawnClass, std::optional<UActor*> SpawnOwner, std::optional<NameString> SpawnTag, std::optional<vec3> SpawnLocation, std::optional<Rotator> SpawnRotation)
{
	if (!SpawnClass || SpawnClass->ClsFlags & ClassFlags::Abstract)
	{
		LogMessage("Could not spawn class: " + (SpawnClass ? SpawnClass->Name.ToString() : std::string("null")));
		return nullptr;
	}

	vec3 location = SpawnLocation ? *SpawnLocation : Location();
	Rotator rotation = SpawnRotation ? *SpawnRotation : Rotation();

	float radius = SpawnClass->GetDefaultObject<UActor>()->CollisionRadius();
	float height = SpawnClass->GetDefaultObject<UActor>()->CollisionHeight();
	bool bCollideWorld = SpawnClass->GetDefaultObject<UActor>()->bCollideWorld();
	bool bCollideWhenPlacing = SpawnClass->GetDefaultObject<UActor>()->bCollideWhenPlacing();
	if (bCollideWorld || bCollideWhenPlacing)
	{
		auto result = CheckLocation(location, radius, height, bCollideWorld || bCollideWhenPlacing);
		if (!result.first)
		{
			LogMessage("Could not find usable location when trying to spawn: " + SpawnClass->Name.ToString());
			return nullptr;
		}
		location = result.second;
	}

	// To do: package needs to be grabbed from outer, or the "transient package" if it is None, a virtual package for runtime objects
	// To do: find unique new name in the package
	static std::map<NameString, int> nextIndex;
	NameString name = SpawnClass->Name.ToString() + std::to_string(nextIndex[SpawnClass->Name]++);
	UActor* actor = UObject::Cast<UActor>(engine->LevelPackage->NewObject(name, UObject::Cast<UClass>(SpawnClass), ObjectFlags::Transient, true));

	actor->Outer() = XLevel()->Outer();
	actor->XLevel() = XLevel();
	actor->Level() = Level();
	actor->Tag() = (SpawnTag && !SpawnTag->IsNone()) ? *SpawnTag : SpawnClass->Name;
	actor->bTicked() = bTicked(); // To do: should it tick in the same world tick it was spawned in or wait until the next one?
	actor->Instigator() = Instigator();
	actor->Brush() = nullptr;
	actor->Location() = location;
	actor->OldLocation() = location;
	actor->Rotation() = rotation;
	actor->Region().Zone = actor->Level();
	actor->Index = (int)XLevel()->Actors.size();
	XLevel()->Actors.push_back(actor);
	XLevel()->Collision.AddToCollision(actor);
	XLevel()->Light.AddLight(actor);

	actor->SetOwner(SpawnOwner.has_value() && SpawnOwner.value() ? *SpawnOwner : nullptr);

	if (Level()->bBegunPlay())
	{
		CallEvent(actor, EventName::Spawned);
		CallEvent(actor, EventName::PreBeginPlay);
		CallEvent(actor, EventName::BeginPlay);

		if (actor->bDeleteMe())
		{
			LogMessage("Object deleted itself during Spawn!");
			return nullptr;
		}

		// To do: we need to call EventName::EncroachingOn events here?

		actor->InitActorZone();

		CallEvent(actor, EventName::PostBeginPlay);
		CallEvent(actor, EventName::SetInitialState);
		if (engine->LaunchInfo.IsDeusEx())
			CallEvent(actor, "PostPostBeginPlay");

		actor->InitBase();

		if (engine->LaunchInfo.ue1Version >= 400)
		{
			static bool spawnNotificationLocked = false;
			if (!spawnNotificationLocked)
			{
				struct NotificationLockGuard
				{
					NotificationLockGuard() { spawnNotificationLocked = true; }
					~NotificationLockGuard() { spawnNotificationLocked = false; }
				} lockGuard;

				for (USpawnNotify* notifyObj = Level()->SpawnNotify(); notifyObj != nullptr; notifyObj = notifyObj->Next())
				{
					UClass* cls = notifyObj->ActorClass();
					if (cls && actor->IsA(cls->Name))
						actor = UObject::Cast<UGameInfo>(CallEvent(notifyObj, EventName::SpawnNotification, { ExpressionValue::ObjectValue(actor) }).ToObject());
				}
			}
		}
	}

	return actor;
}

void UActor::InitBase()
{
	if (engine->LaunchInfo.ue1Version > 219)
	{
		NameString attachTag = AttachTag();
		if (!attachTag.IsNone())
		{
			for (UActor* levelActor : XLevel()->Actors)
			{
				if (levelActor && levelActor->Tag() == attachTag)
				{
					levelActor->SetBase(this, false);
				}
			}
			return;
		}
	}

	// Find base for certain types
	bool isDecorationInventoryOrPawn = UObject::TryCast<UDecoration>(this) || UObject::TryCast<UInventory>(this) || UObject::TryCast<UPawn>(this);
	if (isDecorationInventoryOrPawn && !ActorBase() && bCollideWorld() && (Physics() == PHYS_None || Physics() == PHYS_Rotating))
	{
		CollisionHitList hits = XLevel()->Collision.OverlapTest(this);
		if (!hits.empty())
		{
			SetBase(hits.front().Actor, true);
		}
	}

	if (engine->LaunchInfo.ue1Version < 400 && !ActorBase()) // Unreal expects a base to always exist. What about UT? TournamentPlayer seems to indicate not.
	{
		SetBase(Level(), false);
	}
}

std::pair<bool, vec3> UActor::CheckLocation(vec3 location, float radius, float height, bool check)
{
	// Search for a valid spot near the location

	if (!check)
		return { true, location };

	// What is a reasonable size for this grid? what did UE1 do?
	int offset[] = { 0, 1, -1 };
	bool found = false;
	float scale = std::max(radius, height);
	for (int z = 0; z < 3 && !found; z++)
	{
		for (int y = 0; y < 3 && !found; y++)
		{
			for (int x = 0; x < 3 && !found; x++)
			{
				vec3 testlocation = location + vec3(offset[x] * scale, offset[y] * scale, offset[z] * scale);
				CollisionHitList hits = XLevel()->Collision.OverlapTest(testlocation, height, radius, false, true, false);
				if (hits.empty())
				{
					location = testlocation;
					found = true;
				}
			}
		}
	}
	return { found, location };
}

bool UActor::Destroy()
{
	//engine->LogMessage("UActor.Destroy(" + Class->FriendlyName.ToString() + ")");

	if (bStatic() || bNoDelete())
		return false;
	if (bDeleteMe())
		return true;

	bDeleteMe() = true;

	//GotoState({}, {}); // What should happen to function calls after Destroy() has been called? Razor2 calls SetRoll afterwards!
	SetBase(nullptr, true);

	engine->audiodev->ActorDestroyed(this);

	ULevel* level = XLevel();

	RemoveFromBspNode();
	level->Collision.RemoveFromCollision(this);
	level->Light.RemoveLight(this);

	CallEvent(this, EventName::Destroyed);

	if (engine->LaunchInfo.IsUnrealTournament_469())
	{
		for (const auto actor : Touching_UT469())
			if (actor)
				UnTouch(actor);
	}
	else
	{
		for (const auto actor : Touching())
			if (actor)
				UnTouch(actor);
	}


	SetOwner(nullptr);

	while (!ChildActors.empty())
	{
		ChildActors.back()->SetOwner(nullptr);
	}
	while (!BasedActors.empty())
	{
		BasedActors.back()->SetBase(nullptr, true);
	}

	if (Index == -1)
		throw std::runtime_error("Actor index was never set!");
	level->Actors[Index] = nullptr;

	return true;
}

PointRegion UActor::FindRegion(const vec3& offset)
{
	return XLevel()->Model->FindRegion(Location() + offset, Level());
}

void UActor::InitActorZone()
{
	Region() = FindRegion();
	if (Region().Zone->bWaterZone() && !this->IsA("Projectile"))
	{
		SetPhysics(PHYS_Swimming);
		SetBase(nullptr, true);
	}
}

void UActor::UpdateActorZone()
{
	PointRegion oldregion = Region();
	PointRegion newregion = FindRegion();

	if (oldregion.Zone && oldregion.Zone != newregion.Zone)
		CallEvent(oldregion.Zone, EventName::ActorLeaving, { ExpressionValue::ObjectValue(this) });

	Region() = newregion;

	if (newregion.Zone && oldregion.Zone != newregion.Zone)
	{
		CallEvent(this, EventName::ZoneChange, { ExpressionValue::ObjectValue(newregion.Zone) });
		CallEvent(newregion.Zone, EventName::ActorEntered, { ExpressionValue::ObjectValue(this) });
	}

	if (Region().Zone)
	{
		if (Region().Zone->bDestructive() && IsA("Carcass"))
		{
			// If the actor is a Carcass and the zone is marked as bDestructive, destroy it.
			Destroy();
		}
		else if (engine->LaunchInfo.ue1Version > 219 && Owner() == nullptr && Region().Zone->bNoInventory() && IsA("Inventory"))
		{
			// If the new zone is bNoInventory, destroy Inventory that's not owned by anyone (i.e. in pickup state).
			Destroy();
		}
	}
}

void UActor::SetOwner(UActor* newOwner)
{
	if (Owner())
	{
		CallEvent(Owner(), EventName::LostChild, { ExpressionValue::ObjectValue(this) });
		Owner()->RemoveChildActor(this);
	}

	Owner() = newOwner;

	if (Owner())
	{
		CallEvent(Owner(), EventName::GainedChild, { ExpressionValue::ObjectValue(this) });
		Owner()->AddChildActor(this);
	}
}

void UActor::AddChildActor(UActor* actor)
{
	if (actor)
		ChildActors.push_back(actor);
}

void UActor::RemoveChildActor(UActor* actor)
{
	if (!actor)
		return;

	auto it = ChildActors.begin();

	while (it != ChildActors.end())
	{
		if (*it == actor)
		{
			ChildActors.erase(it);
			return;
		}
		it++;
	}
}

void UActor::AddBasedActor(UActor* actor)
{
	if (actor)
		BasedActors.push_back(actor);
}

void UActor::RemoveBasedActor(UActor* actor)
{
	if (!actor)
		return;

	auto it = BasedActors.begin();

	while (it != BasedActors.end())
	{
		if (*it == actor)
		{
			BasedActors.erase(it);
			return;
		}
		it++;
	}
}

void UActor::SetBase(UActor* newBase, bool sendBaseChangeEvent)
{
	if (ActorBase() != newBase)
	{
		if (this->IsBasedOn(newBase))
			return; // don't allow any cycles in the tree

		if (ActorBase() && ActorBase() != Level())
		{
			ActorBase()->RemoveBasedActor(this);
			ActorBase()->StandingCount() = (uint8_t)std::min<size_t>(ActorBase()->BasedActors.size(), 0xff);
			CallEvent(ActorBase(), EventName::Detach, { ExpressionValue::ObjectValue(this) });
		}

		ActorBase() = newBase;

		if (ActorBase() && ActorBase() != Level())
		{
			ActorBase()->AddBasedActor(this);
			ActorBase()->StandingCount() = (uint8_t)std::min<size_t>(ActorBase()->BasedActors.size(), 0xff);
			// Note: in the unlikely case of an actor having > 255 bases, StandingCount() won't be an accurate number.
			CallEvent(ActorBase(), EventName::Attach, { ExpressionValue::ObjectValue(this) });
		}

		if (sendBaseChangeEvent)
			CallEvent(this, EventName::BaseChange);
	}
}

void UActor::RelinkBasedActor()
{
	if (ActorBase() && ActorBase() != Level())
	{
		ActorBase()->AddBasedActor(this);
		ActorBase()->StandingCount() = (uint8_t)std::min<size_t>(ActorBase()->BasedActors.size(), 0xff);
	}
}

void UActor::Tick(float elapsed)
{
	if (engine->LaunchInfo.IsDeusEx())
	{
		DistanceFromPlayer() = length(engine->viewport->Actor()->Location() - Location());
	}

	TickAnimation(elapsed);
	if (engine->LaunchInfo.IsDeusEx())
		TickBlendAnimation(elapsed);

	if (Role() >= ROLE_SimulatedProxy && IsEventEnabled(EventName::Tick))
	{
		CallEvent(this, EventName::Tick, { ExpressionValue::FloatValue(elapsed) });
	}

	if (StateFrame)
	{
		if (StateFrame->LatentState == LatentRunState::Sleep)
		{
			SleepTimeLeft = std::max(SleepTimeLeft - elapsed, 0.0f);
			if (SleepTimeLeft == 0.0f)
				StateFrame->LatentState = LatentRunState::Continue;
		}
		else if (StateFrame->LatentState == LatentRunState::FinishInterpolation)
		{
			if (!bInterpolating())
				StateFrame->LatentState = LatentRunState::Continue;
		}

		if (Role() >= ROLE_SimulatedProxy && StateFrame->LatentState == LatentRunState::Continue)
		{
			StateFrame->Tick();
		}
	}

	TickPhysics(elapsed);

	if (TimerRate() > 0.0f) // Role() == ROLE_Authority && RemoteRole() == ROLE_AutonomousProxy
	{
		TimerCounter() += elapsed;
		while (TimerRate() > 0.0f && TimerCounter() > TimerRate())
		{
			TimerCounter() -= TimerRate();
			if (!bTimerLoop())
				TimerRate() = 0.0f;
			CallEvent(this, EventName::Timer);
		}
	}
}

void UActor::TickPhysics(float elapsed)
{
	for (float timeLeft = elapsed; timeLeft > 0.0f && !bDeleteMe(); timeLeft -= 0.02f)
	{
		float physTimeElapsed = std::min(timeLeft, 0.02f);
		int mode = Physics();
		if (mode != PHYS_None)
		{
			switch (mode)
			{
			case PHYS_Walking: TickWalking(physTimeElapsed); break;
			case PHYS_Falling: TickFalling(physTimeElapsed); break;
			case PHYS_Swimming: TickSwimming(physTimeElapsed); break;
			case PHYS_Flying: TickFlying(physTimeElapsed); break;
			case PHYS_Rotating: break;
			case PHYS_Projectile: TickProjectile(physTimeElapsed); break;
			case PHYS_Rolling: TickRolling(physTimeElapsed); break;
			case PHYS_Interpolating: TickInterpolating(physTimeElapsed); break;
			case PHYS_MovingBrush: TickMovingBrush(physTimeElapsed); break;
			case PHYS_Spider: TickSpider(physTimeElapsed); break;
			case PHYS_Trailer: TickTrailer(physTimeElapsed); break;
			}
			TickRotating(physTimeElapsed); // Rotation logic applies to multiple physics modes and not just PHYS_Rotating
		}

		if (engine->LaunchInfo.ue1Version >= 400)
		{
			if (PendingTouch())
			{
				CallEvent(PendingTouch(), EventName::PostTouch, { ExpressionValue::ObjectValue(this) });
				if (PendingTouch())
				{
					UActor* cur = PendingTouch();
					UActor* next = cur->PendingTouch();
					PendingTouch() = next;
					cur->PendingTouch() = nullptr;
				}
			}
		}
	}
}

void UActor::TickWalking(float elapsed)
{
	// Only pawns can walk!
	UPawn* pawn = UObject::TryCast<UPawn>(this);
	if (!pawn)
		return;

	if (Region().ZoneNumber == 0)
	{
		CallEvent(this, EventName::FellOutOfWorld);
		return;
	}

	// Save our starting point and state

	OldLocation() = Location();
	bJustTeleported() = false;

	// Update the actor velocity based on the acceleration and zone

	UZoneInfo* zone = Region().Zone;
	// UDecoration* decor = UObject::TryCast<UDecoration>(this);
	UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(this);

	Velocity().z = 0.0f;

	if (dot(Acceleration(), Acceleration()) > 0.0001f)
	{
		float accelRate = pawn->AccelRate();
		if (player && player->bIsWalking())
			accelRate *= 0.3f;

		// Acceleration must never exceed the acceleration rate
		float accelSpeed = length(Acceleration());
		vec3 accelDir = Acceleration() * (1.0f / accelSpeed);
		if (accelSpeed > accelRate)
			Acceleration() = accelDir * accelRate;

		float speed = length(Velocity());
		Velocity() = Velocity() - (Velocity() - accelDir * speed) * (zone->ZoneGroundFriction() * elapsed);
	}
	else
	{
		float speed = length(Velocity());
		if (speed > 0.0f)
		{
			float newSpeed = std::max(speed - speed * zone->ZoneGroundFriction() * 2.0f * elapsed, 0.0f);
			Velocity() = Velocity() * (newSpeed / speed);
		}
	}

	Velocity() = Velocity() + Acceleration() * elapsed;

	float maxSpeed = player ? player->GroundSpeed() : pawn->GroundSpeed() * pawn->DesiredSpeed();
	if (player && player->bIsWalking())
		maxSpeed *= 0.3f;

	float speed = length(Velocity());
	if (speed > 0.0f && speed > maxSpeed)
		Velocity() = Velocity() * (maxSpeed / speed);

	Velocity().z = 0.0f;

	// The classic step up, move and step down algorithm:

	float gravityDirection = zone->ZoneGravity().z > 0.0f ? 1.0f : -1.0f;
	vec3 stepUpDelta(0.0f, 0.0f, -gravityDirection * pawn->MaxStepHeight());
	vec3 stepDownDelta(0.0f, 0.0f, gravityDirection * pawn->MaxStepHeight() * stepDownDeltaFactor);

	// "Step up and move" as long as we have time left and only hitting surfaces with low enough slope that it could be walked
	float timeLeft = elapsed;
	vec3 vel = Velocity() + zone->ZoneVelocity() * elapsed * 25.0f;

	//included Z in check - if its not 0 due to Zone properties, no action would have been taken previously
	//could lead to latend bugs
	if (length(vel) > 0)
	{
		for (int iteration = 0; timeLeft > 0.0f && iteration < 5; iteration++)
		{
			vec3 moveDelta = vel * timeLeft;

			//movement logic was inverted, causing overhaed buttons to be to easy to push
			// -> kevlar suit button in VortexRikers activates by moving under it
			//alternative approach: first move without stepUp -> only step up on collision
			//also yields a simpler code path since it avoids the need for "headbump" checks

			// try move forward
			CollisionHit hit = TryMove(moveDelta);
			timeLeft -= timeLeft * hit.Fraction;
			moveDelta = vel * timeLeft;

			// if hit, step up and try again - maybe there was a ledge to get over
			if (hit.Fraction < 1.0f)
			{
				TryMove(stepUpDelta);
				hit = TryMove(moveDelta);
				timeLeft -= timeLeft * hit.Fraction;
				// move back down to original vertical position
				TryMove(-stepUpDelta);
			}

			if (hit.Fraction < 1.0f)
			{
				if (player && hit.Actor)
				{
					if (UObject::IsType<UDecoration>(hit.Actor) && UObject::Cast<UDecoration>(hit.Actor)->bPushable() && dot(hit.Normal, moveDelta) < -0.9f)
					{
						// We hit a pushable decoration that is facing our movement direction

						//why does hitting a pushable decoration set the teleport flag?
						bJustTeleported() = true;
						vel = Velocity() = Velocity() * Mass() / (Mass() + hit.Actor->Mass());
						CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
						timeLeft = 0.0f;
					}
					else if (hit.Actor->bCollideActors() && hit.Actor->CollisionHeight() > 0.0f && hit.Actor->CollisionRadius() > 0.0f)
					{
						// TODO: We hit a non-movable actor

					}
				}
				else if (hit.Normal.z < 0.2f && hit.Normal.z > -0.2f)
				{
					// We hit a wall
					CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });

					vec3 alignedDelta = (moveDelta - hit.Normal * dot(moveDelta, hit.Normal)) * (1.0f - hit.Fraction);
					if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
					{
						hit = TryMove(alignedDelta);
						timeLeft -= timeLeft * hit.Fraction;
						if (hit.Fraction < 1.0f)
						{
							CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
						}
					}
					else
					{
						timeLeft = 0.0f;
					}
				}
			}

			// Check if unrealscript got us out of walking mode
			if (Physics() != PHYS_Walking)
				return;

			// Can we reach the ground from here if we step down? (dry run)
			CollisionHit floorHit = TryMove(stepDownDelta, true);
			if (floorHit.Fraction == 1.0f || floorHit.Normal.z < 0.7071f)
			{
				// No we couldn't. We are falling
				SetPhysics(PHYS_Falling);
				SetBase(nullptr, true);
				return;
			}

			// We could reach the ground. Step down there.
			floorHit = TryMove(stepDownDelta);
			if (floorHit.Fraction != 1.0f)
				SetBase(floorHit.Actor, true);
		}
	}
	else
	{
		// Can we reach the ground from here?
		CollisionHit floorHit = TryMove(stepDownDelta, true);
		if (floorHit.Fraction == 1.0f || floorHit.Normal.z < 0.7071f)
		{
			// No we couldn't. We are falling
			SetPhysics(PHYS_Falling);
			SetBase(nullptr, true);
		}
	}

	if (!bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;
	Velocity().z = 0.0f;
}

void UActor::TickFalling(float elapsed)
{
	if (Region().ZoneNumber == 0)
	{
		CallEvent(this, EventName::FellOutOfWorld);
		return;
	}

	UZoneInfo* zone = Region().Zone;
	UDecoration* decor = UObject::TryCast<UDecoration>(this);
	UPawn* pawn = UObject::TryCast<UPawn>(this);

	// UnrealScript property references
	vec3& acceleration = Acceleration();
	vec3& velocity = Velocity();
	vec3& oldLocation = OldLocation();
	vec3& location = Location();
	float groundSpeed = 0.0f;

	if (pawn)
	{
		groundSpeed = pawn->GroundSpeed();
		float maxAccel = engine->LaunchInfo.ue1Version > 219 ? pawn->AirControl() * pawn->AccelRate() : 0.0f;
		float accel = length(acceleration);
		if (accel > maxAccel)
			acceleration = normalize(acceleration) * maxAccel;
	}

	float gravityScale = 2.0f;
	float fluidFriction = 0.0f;

	if (decor && decor->bBobbing())
	{
		gravityScale = 1.0f;
	}
	else if (pawn && pawn->FootRegion().Zone->bWaterZone() && velocity.z < 0.0f)
	{
		fluidFriction = pawn->FootRegion().Zone->ZoneFluidFriction();
	}

	OldLocation() = Location();
	bJustTeleported() = false;

	float fluidFactor = 1.0f - fluidFriction * elapsed;
	vec3 accelVector = acceleration * 1.5f;
	vec3 gravityVector = gravityScale * zone->ZoneGravity();

	vec3 oldVelocity = velocity;
	vec3 newVelocity = oldVelocity * fluidFactor + (accelVector + gravityVector) * 0.5f * elapsed;

	// Limit air control to controlling which direction we are moving in the XY plane, but not increase the speed beyond the ground speed
	vec2 velocity2d = velocity.xy();
	vec2 newVelocity2d = newVelocity.xy();
	float curSpeedSquared = dot(velocity2d, velocity2d);
	if (pawn && curSpeedSquared >= (groundSpeed * groundSpeed) && dot(newVelocity2d, newVelocity2d) > curSpeedSquared)
	{
		float xySpeed = length(velocity2d);
		newVelocity = vec3(normalize(newVelocity2d) * xySpeed, newVelocity.z);
	}
	velocity = newVelocity;

	float timeLeft = elapsed;
	for (int iteration = 0; timeLeft > 0.0f && iteration < 5; iteration++)
	{
		float zoneTerminalVelocity = zone->ZoneTerminalVelocity();
		if (dot(velocity, velocity) > zoneTerminalVelocity * zoneTerminalVelocity)
		{
			velocity = normalize(velocity) * zoneTerminalVelocity;
			newVelocity = velocity;
		}

		vec3 moveDelta = (newVelocity + zone->ZoneVelocity() * elapsed * 25.0f) * timeLeft;
		vec3 dirNormal = normalize(newVelocity);

		CollisionHit hit = TryMove(moveDelta);
		timeLeft -= timeLeft * hit.Fraction;

		if (hit.Fraction < 1.0f)
		{
			if (hit.Actor && hit.Actor->IsA("Pawn"))
			{
				// So projectiles don't think they hit a wall.
			}
			else
			{
				CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
			}

			// Hit the level
			if (bBounce())
			{
				vec3 reflectedDelta = reflect(moveDelta, hit.Normal);
				hit = TryMove(reflectedDelta);
			}
			else
			{
				if (hit.Normal.z < 0.7071f)
				{
					// We hit a slope. Try to follow it.
					vec3 alignedDelta = (moveDelta - hit.Normal * dot(moveDelta, hit.Normal)) * (1.0f - hit.Fraction);
					if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
					{
						hit = TryMove(alignedDelta);
						if (hit.Fraction < 1.0f && hit.Normal.z > 0.7071f)
						{
							PhysLanded(hit.Actor, hit.Normal);
							return;
						}
					}

					// adjust velocity along the slope
					if (!bBounce() && !bJustTeleported())
						velocity = (location - oldLocation) / elapsed;

					timeLeft = 0.0f;
				}
				else
				{
					PhysLanded(hit.Actor, hit.Normal);
					timeLeft = 0.0f;
				}
			}
		}
	}
}

void UActor::TickSwimming(float elapsed)
{
	// Only pawns can swim!
	UPawn* pawn = UObject::TryCast<UPawn>(this);

	if (!pawn)
		return;

	if (Region().ZoneNumber == 0)
	{
		CallEvent(this, EventName::FellOutOfWorld);
		return;
	}

	// Save our starting point and state
	OldLocation() = Location();
	bJustTeleported() = false;

	// Update the actor velocity based on the acceleration and zone

	UZoneInfo* zone = Region().Zone;
	// UDecoration* decor = UObject::TryCast<UDecoration>(this);
	UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(this);

	if (dot(Acceleration(), Acceleration()) > 0.0001f)
	{
		float accelRate = pawn->AccelRate() * 0.3f;

		// Acceleration must never exceed the acceleration rate
		float accelSpeed = length(Acceleration());
		vec3 accelDir = Acceleration() * (1.0f / accelSpeed);
		if (accelSpeed > accelRate)
			Acceleration() = accelDir * accelRate;

		float speed = length(Velocity());
		Velocity() = Velocity() - (Velocity() - accelDir * speed) * (zone->ZoneFluidFriction() * elapsed);
	}
	else
	{
		float speed = length(Velocity());
		if (speed > 0.0f)
		{
			float newSpeed = std::max(speed - speed * zone->ZoneFluidFriction() * 2.0f * elapsed, 0.0f);
			Velocity() = Velocity() * (newSpeed / speed);
		}
	}

	Velocity() = Velocity() + Acceleration() * elapsed;

	float maxSpeed = player ? player->WaterSpeed() : pawn->WaterSpeed() * pawn->DesiredSpeed();

	float speed = length(Velocity());
	if (speed > 0.0f && speed > maxSpeed)
		Velocity() = Velocity() * (maxSpeed / speed);

	//float gravityDirection = zone->ZoneGravity().z > 0.0f ? 1.0f : -1.0f;

	float timeLeft = elapsed;
	vec3 vel = Velocity() + zone->ZoneVelocity() * elapsed * 25.0f;

	//same as tick walking, having any velosity at all should probably enable this branch
	if (length(vel))
	{
		for (int iteration = 0; timeLeft > 0.0f && iteration < 5; iteration++)
		{
			vec3 moveDelta = vel * timeLeft;

			CollisionHit hit = TryMove(moveDelta);
			timeLeft -= timeLeft * hit.Fraction;
			moveDelta = vel * timeLeft;

			if (hit.Fraction < 1.0f)
			{
				if (player && UObject::IsType<UDecoration>(hit.Actor) && UObject::Cast<UDecoration>(hit.Actor)->bPushable() && dot(hit.Normal, moveDelta) < -0.9f)
				{
					// We hit a pushable decoration that is facing our movement direction

					//same question as with tick walk -> why set teleport flag on decoration?
					bJustTeleported() = true;
					Velocity() = Velocity() * Mass() / (Mass() + hit.Actor->Mass());
					CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
					timeLeft = 0.0f;
				}
				else
				{
					// We hit a wall
					CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });

					//removed the second scaling, that caused the "exiting the water is difficult" bug
					//it appears to not fix it completely, but it helps
					vec3 alignedDelta = moveDelta - hit.Normal * dot(moveDelta, hit.Normal);

					if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
					{
						hit = TryMove(alignedDelta);
						timeLeft -= timeLeft * hit.Fraction;
						if (hit.Fraction < 1.0f)
						{
							CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
						}
					}
					else
					{
						timeLeft = 0.0f;
					}
				}
			}
		}
	}

	if (!bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;

	if (!Region().Zone->bWaterZone())
	{
		if(Velocity().z > 0.0f)
		{
			Velocity().z = std::max(Velocity().z, 100.0f);
		}
		if (Physics() == PHYS_Swimming)
			SetPhysics(PHYS_Falling);
	}
}

void UActor::TickFlying(float elapsed)
{
	// Only pawns can fly!
	UPawn* pawn = UObject::TryCast<UPawn>(this);
	if (!pawn)
		return;

	if (Region().ZoneNumber == 0)
	{
		CallEvent(this, EventName::FellOutOfWorld);
		return;
	}

	// Save our starting point and state

	OldLocation() = Location();
	bJustTeleported() = false;

	// Update the actor velocity based on the acceleration and zone

	UZoneInfo* zone = Region().Zone;
	UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(this);

	if (dot(Acceleration(), Acceleration()) > 0.0001f)
	{
		float accelRate = pawn->AccelRate();

		// Acceleration must never exceed the acceleration rate
		float accelSpeed = length(Acceleration());
		vec3 accelDir = Acceleration() * (1.0f / accelSpeed);
		if (accelSpeed > accelRate)
			Acceleration() = accelDir * accelRate;

		float speed = length(Velocity());
		Velocity() = Velocity() - (Velocity() - accelDir * speed) * (zone->ZoneFluidFriction() * elapsed);
	}
	else
	{
		float speed = length(Velocity());
		if (speed > 0.0f)
		{
			float newSpeed = std::max(speed - speed * zone->ZoneFluidFriction() * 2.0f * elapsed, 0.0f);
			Velocity() = Velocity() * (newSpeed / speed);
		}
	}

	Velocity() = Velocity() + Acceleration() * elapsed;

	float maxSpeed = player ? player->AirSpeed() : pawn->AirSpeed() * pawn->DesiredSpeed();

	float speed = length(Velocity());
	if (speed > 0.0f && speed > maxSpeed)
		Velocity() = Velocity() * (maxSpeed / speed);

	float timeLeft = elapsed;
	vec3 vel = Velocity() + zone->ZoneVelocity() * elapsed * 25.0f;
	if (length(vel))
	{
		for (int iteration = 0; timeLeft > 0.0f && iteration < 5; iteration++)
		{
			vec3 moveDelta = vel * timeLeft;

			CollisionHit hit = TryMove(moveDelta);
			timeLeft -= timeLeft * hit.Fraction;
			moveDelta = vel * timeLeft;

			if (hit.Fraction < 1.0f)
			{
				// We hit a wall
				CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });

				vec3 alignedDelta = (moveDelta - hit.Normal * dot(moveDelta, hit.Normal)) * (1.0f - hit.Fraction);
				if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
				{
					hit = TryMove(alignedDelta);
					timeLeft -= timeLeft * hit.Fraction;
					if (hit.Fraction < 1.0f)
					{
						CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
					}
				}
				else
				{
					timeLeft = 0.0f;
				}
			}
		}
	}

	if (!bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;
	Velocity().z = 0.0f;
}

void UActor::TickRotating(float elapsed)
{
	if (bRotateToDesired())
	{
		if (Rotation() != DesiredRotation())
		{
			Rotator rot = Rotation();
			if (bFixedRotationDir())
			{
				rot.Yaw = Rotator::TurnToFixed(rot.Yaw, DesiredRotation().Yaw, (int)(RotationRate().Yaw * elapsed));
				rot.Pitch = Rotator::TurnToFixed(rot.Pitch, DesiredRotation().Pitch, (int)(RotationRate().Pitch * elapsed));
				rot.Roll = Rotator::TurnToFixed(rot.Roll, DesiredRotation().Roll, (int)(RotationRate().Roll * elapsed));
			}
			else
			{
				rot.Yaw = Rotator::TurnToShortest(rot.Yaw, DesiredRotation().Yaw, (int)std::abs(RotationRate().Yaw * elapsed));
				rot.Pitch = Rotator::TurnToShortest(rot.Pitch, DesiredRotation().Pitch, (int)std::abs(RotationRate().Pitch * elapsed));
				rot.Roll = Rotator::TurnToShortest(rot.Roll, DesiredRotation().Roll, (int)std::abs(RotationRate().Roll * elapsed));
			}
			Rotation() = rot;

			if (Rotation() == DesiredRotation())
			{
				CallEvent(this, EventName::EndedRotation);
			}
		}
	}
	else if (bFixedRotationDir())
	{
		Rotation() += RotationRate() * elapsed;
	}
}

void UActor::TickProjectile(float elapsed)
{
	if (Region().ZoneNumber == 0)
	{
		Destroy();
		return;
	}

	UZoneInfo* zone = Region().Zone;
	UProjectile* projectile = UObject::TryCast<UProjectile>(this);
	UPawn* pawn = UObject::TryCast<UPawn>(this);

	if (zone->bWaterZone())
		Velocity() = Velocity() * std::max(1.0f - zone->ZoneFluidFriction() * 0.2f * elapsed, 0.0f);

	Velocity() = Velocity() + Acceleration() * elapsed;

	if (projectile)
	{
		float maxSpeed = projectile->MaxSpeed();
		if (dot(Velocity(), Velocity()) > maxSpeed * maxSpeed)
		{
			Velocity() = normalize(Velocity()) * maxSpeed;
		}
	}

	OldLocation() = Location();
	bJustTeleported() = false;

	CollisionHit hit = TryMove(Velocity() * elapsed);

	if (hit.Fraction < 1.0f && !hit.Actor && !bDeleteMe() && !bJustTeleported())
	{
		CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
	}

	if (!bBounce() && !bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;
}

void UActor::TickRolling(float elapsed)
{
	if (Region().ZoneNumber == 0)
	{
		CallEvent(this, EventName::FellOutOfWorld);
		return;
	}

	// Save our starting point and state

	OldLocation() = Location();
	bJustTeleported() = false;

	// Update the actor velocity based on the acceleration and zone

	UZoneInfo* zone = Region().Zone;

	float speed = length(Velocity());
	Velocity() = Velocity() - speed * (normalize(Velocity()) - normalize(Acceleration())) * zone->ZoneGroundFriction() * elapsed;
	Velocity() = Velocity() * (1.0f - zone->ZoneFluidFriction() * elapsed) + Acceleration() * elapsed;


	vec3 moveDelta = (Velocity() + zone->ZoneVelocity() * elapsed * 25.0f) * elapsed;
	CollisionHit hit = TryMove(moveDelta);

	if (hit.Fraction < 1.0f && hit.Normal.z < 0.7071f)
	{
		CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });

		vec3 alignedDelta = (moveDelta - hit.Normal * dot(moveDelta, hit.Normal)) * (1.0f - hit.Fraction);
		if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
			TryMove(alignedDelta);
	}

	if (Physics() != PHYS_Rolling)
		return;

	float gravityDirection = zone->ZoneGravity().z > 0.0f ? 1.0f : -1.0f;
	constexpr float stepHeightRatio = 25.0f / 47.5f; // not sure what this should be. Appears that humans have a step height of 25.0 and collision height of 47.5, so I guess I'll use that ratio
	vec3 stepDownDelta(0.0f, 0.0f, gravityDirection * stepHeightRatio * CollisionHeight() * stepDownDeltaFactor);

	// Can we reach the ground from here?
	CollisionHit floorHit = TryMove(stepDownDelta, true);
	if (floorHit.Fraction == 1.0f || floorHit.Normal.z < 0.7071f)
	{
		// No we couldn't. We are falling
		SetPhysics(PHYS_Falling);
		SetBase(nullptr, true);
	}
	else
	{
		// We could reach the ground. Step down there.
		floorHit = TryMove(stepDownDelta);
		if (floorHit.Fraction != 1.0f)
			SetBase(floorHit.Actor, true);
	}

	if (!bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;
}

void UActor::TickInterpolating(float elapsed)
{
	OldLocation() = Location();

	float timeLeft = elapsed;
	while (timeLeft > 0.0f)
	{
		if (PhysRate() == 0.0f || !bInterpolating())
			break;

		UInterpolationPoint* target = UObject::Cast<UInterpolationPoint>(Target());
		UInterpolationPoint* next = target ? target->Next() : nullptr;
		if (!target || !next)
			break;

		float physAlpha = PhysAlpha();

		if (auto pawn = UObject::TryCast<UPlayerPawn>(this))
		{
			if (engine->LaunchInfo.ue1Version > 219)
			{
				pawn->DesiredFlashScale() = mix(target->ScreenFlashScale(), next->ScreenFlashScale(), physAlpha);
				pawn->DesiredFlashFog() = mix(target->ScreenFlashFog(), next->ScreenFlashFog(), physAlpha);
				pawn->FovAngle() = mix(target->FovModifier(), next->FovModifier(), physAlpha) * Class->GetDefaultObject<UPlayerPawn>()->FovAngle();
				pawn->FlashScale() = vec3(pawn->DesiredFlashScale());
				pawn->FlashFog() = pawn->DesiredFlashFog();
			}
		}

		if (engine->LaunchInfo.ue1Version > 219)
			Level()->TimeDilation() = mix(target->GameSpeedModifier(), next->GameSpeedModifier(), physAlpha);

		float rateModifier = mix(target->RateModifier(), next->RateModifier(), physAlpha);
		float physRate = PhysRate() * rateModifier;
		if (physRate == 0.0f)
			break;

		bool interpolateStart = false, interpolateEnd = false;
		physAlpha += physRate * timeLeft;
		if (physRate < 0.0f && physAlpha < 0.0f)
		{
			timeLeft = physAlpha / physRate;
			physAlpha = 0.0f;
			interpolateStart = true;
		}
		else if (physRate > 0.0f && physAlpha > 1.0f)
		{
			timeLeft = (physAlpha - 1.0f) / physRate;
			physAlpha = 1.0f;
			interpolateEnd = true;
		}
		else
		{
			timeLeft = 0.0f;
		}

		UInterpolationPoint* prev = target->Prev();
		UInterpolationPoint* nextnext = next->Next();
		vec3 location;
		Rotator rotation;
		if (prev && nextnext)
		{
			location = spline(prev->Location(), target->Location(), next->Location(), nextnext->Location(), physAlpha);
			rotation = spline(prev->Rotation(), target->Rotation(), next->Rotation(), nextnext->Rotation(), physAlpha);
		}
		else
		{
			location = mix(target->Location(), next->Location(), physAlpha);
			rotation = mix(target->Rotation(), next->Rotation(), physAlpha);
		}

		PhysAlpha() = physAlpha;
		TryMove(location - Location());
		SetRotation(rotation);

		if (auto pawn = UObject::TryCast<UPawn>(this))
		{
			pawn->ViewRotation() = Rotation();
		}

		if (interpolateStart)
		{
			CallEvent(target, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(this) });
			CallEvent(this, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(target) });

			target = target->Prev();
			if (engine->LaunchInfo.ue1Version > 219)
			{
				while (target && target->bSkipNextPath())
					target = target->Prev();
			}

			Target() = target;
			PhysAlpha() = 1.0f;
		}
		else if (interpolateEnd)
		{
			CallEvent(target, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(this) });
			CallEvent(this, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(target) });

			target = target->Next();
			if (engine->LaunchInfo.ue1Version > 219)
			{
				while (target && target->bSkipNextPath())
					target = target->Next();
			}

			Target() = target;
			PhysAlpha() = 0.0f;
		}
	}

	if (elapsed > 0.0f)
		Velocity() = (Location() - OldLocation()) / elapsed;
}

void UActor::TickMovingBrush(float elapsed)
{
	OldLocation() = Location();

	UMover* mover = UObject::TryCast<UMover>(this);
	if (mover)
	{
		float timeLeft = elapsed;
		while (timeLeft > 0.0f)
		{
			if (!bInterpolating())
				break;

			if (PhysRate() <= 0.0f)
				break;

			float physAlpha = PhysAlpha();
			float physRate = PhysRate();

			physAlpha += physRate * timeLeft;
			if (physAlpha > 1.0f)
			{
				timeLeft = (physAlpha - 1.0f) / physRate;
				physAlpha = 1.0f;
			}
			else
			{
				timeLeft = 0.0f;
			}

			float t = physAlpha;
			if (mover->MoverGlideType() == 1/*MV_GlideByTime*/)
				t = smoothstep(0.0f, 1.0f, t);

			int keyIndex = clamp((int)mover->KeyNum(), 0, 7);
			vec3 oldpos = mover->OldPos();
			vec3 basepos = mover->BasePos();
			vec3 keypos = mover->KeyPos()[keyIndex];
			Rotator oldrot = mover->OldRot();
			Rotator baserot = mover->BaseRot();
			Rotator keyrot = mover->KeyRot()[keyIndex];

			vec3 deltapos = basepos + keypos - oldpos;
			vec3 targetPos = oldpos + deltapos * t;

			Rotator targetRotation = oldrot + (baserot + keyrot - oldrot) * t;

			// LogMessage("Moving brush: " + std::to_string(t) + " key=" + std::to_string(keyIndex) +" keypos=(" + std::to_string(keypos.x) + "," + std::to_string(keypos.y) + "," + std::to_string(keypos.z) + ")");

			if (TryMove(targetPos - Location()).Fraction == 1.0f)
			{
				SetRotation(targetRotation);
				PhysAlpha() = physAlpha;

				if (physAlpha == 1.0f)
				{
					bInterpolating() = false;
					CallEvent(this, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(nullptr) });
				}
			}
		}
	}

	if (elapsed > 0.0f)
		Velocity() = (Location() - OldLocation()) / elapsed;
}

void UActor::TickSpider(float elapsed)
{
}

void UActor::TickTrailer(float elapsed)
{
	if (!Owner())
		return;

	vec3 newLocation = Owner()->Location();

	if (engine->LaunchInfo.ue1Version >= 400 && bTrailerPrePivot())
	{
		newLocation += PrePivot();
	}

	SetLocation(newLocation);

	if ((engine->LaunchInfo.ue1Version < 400 || bTrailerSameRotation()) && DrawType() != DT_Sprite)
	{
		SetRotation(Owner()->Rotation());
	}
}

void UActor::PhysLanded(UActor* hitActor, const vec3& hitNormal)
{
	// landed on the floor
	CallEvent(this, EventName::Landed, { ExpressionValue::VectorValue(hitNormal) });

	if (Physics() == PHYS_Falling) // Landed event might have changed the physics mode
	{
		if (UObject::TryCast<UPawn>(this))
		{
			SetPhysics(PHYS_Walking);
			SetBase(hitActor, true);
		}
		else
		{
			SetPhysics(PHYS_None);
			SetBase(hitActor, true);
			Velocity() = vec3(0.0f);
		}
	}
}

void UActor::SetPhysics(uint8_t newPhysics)
{
	Physics() = newPhysics;
}

void UActor::SetCollision(bool newColActors, bool newBlockActors, bool newBlockPlayers)
{
	XLevel()->Collision.RemoveFromCollision(this);
	bCollideActors() = newColActors;
	bBlockActors() = newBlockActors;
	bBlockPlayers() = newBlockPlayers;
	XLevel()->Collision.AddToCollision(this);
}

bool UActor::SetLocation(const vec3& newLocation)
{
	auto result = CheckLocation(newLocation, CollisionRadius(), CollisionHeight(), bCollideWorld() || bCollideWhenPlacing());
	if (!result.first)
		return false;

	XLevel()->Collision.RemoveFromCollision(this);
	XLevel()->Light.RemoveLight(this);
	Location() = result.second;
	XLevel()->Collision.AddToCollision(this);
	XLevel()->Light.AddLight(this);

	if (Level()->bBegunPlay())
	{
		// Send touch notifications for anything at the new location
		for (UActor* actor : XLevel()->Collision.CollidingActors(Location(), CollisionHeight(), CollisionRadius()))
		{
			if (actor != this && !actor->IsBasedOn(this) && !IsBasedOn(actor) && bCollideActors() && actor->bCollideActors())
			{
				Touch(actor);
			}
		}

		// Untouch everything we aren't overlapping anymore
		if (engine->LaunchInfo.IsUnrealTournament_469())
		{
			for (const auto actor : Touching_UT469())
			{
				if (actor && !IsOverlapping(actor))
					UnTouch(actor);
			}
		}
		else
		{
			for (const auto actor : Touching())
			{
				if (actor && !IsOverlapping(actor))
					UnTouch(actor);
			}
		}
	}

	return true;
}

bool UActor::SetRotation(const Rotator& newRotation)
{
	// To do: return false if there isn't room

	Rotator delta = newRotation - Rotation();
	Rotation() = newRotation;
	TurnBasedActors(delta);
	return true;
}

// carried items and actors on movers should rotate with the actor their based on
void UActor::TurnBasedActors(const Rotator& deltaRotation)
{
	if ((deltaRotation.Yaw & 0xffff) == 0)
		return;
	Coords yawRot = Coords::YawRotation(deltaRotation.YawRadians());
	vec3 baseLoc = Location();
	for (size_t i = 0; i < BasedActors.size(); )
	{
		UActor* basedActor = BasedActors[i];
		if (!basedActor) { i++; continue; }
		vec3 basedLoc = basedActor->Location();
		vec3 rotatedOffset = yawRot * (basedLoc - baseLoc);
		basedActor->TryMove((baseLoc + rotatedOffset) - basedLoc, false, false);
		basedActor->SetRotation(basedActor->Rotation() + deltaRotation);
		if (UPawn* pawn = UObject::TryCast<UPawn>(basedActor))
			pawn->ViewRotation().Yaw += deltaRotation.Yaw;
		// UnrealScript events triggered in TryMove can call methods such as SetBase or Destroy, so need to guard while iterating.
		if (i < BasedActors.size() && BasedActors[i] == basedActor)
			i++;
	}
}

bool UActor::SetCollisionSize(float newRadius, float newHeight)
{
	// To do: return false if there isn't room

	XLevel()->Collision.RemoveFromCollision(this);
	CollisionRadius() = newRadius;
	CollisionHeight() = newHeight;
	XLevel()->Collision.AddToCollision(this);
	return true;
}

UObject* UActor::Trace(vec3& hitLocation, vec3& hitNormal, const vec3& traceEnd, const vec3& traceStart, bool bTraceActors, const vec3& extent)
{
	TraceFlags flags;
	flags.movers = true;
	flags.world = true;
	if (bTraceActors)
	{
		flags.pawns = true;
		flags.others = true;
		flags.onlyProjectiles = true;
	}

	// hack?
	if (IsA("ChallengeHUD"))
	{
		flags.zoneChanges = true;
	}

	CollisionHit hit = XLevel()->Collision.TraceFirstHit(traceStart, traceEnd, this, extent, flags);
	hitNormal = hit.Normal;
	hitLocation = traceStart + (traceEnd - traceStart) * hit.Fraction;
	return hit.Actor;
}

UObject* UActor::Trace(vec3& hitLocation, vec3& hitNormal, const vec3& traceEnd, const vec3& traceStart, bool bTraceActors, const vec3& extent, bool bTraceBSP, uint8_t BSPTraceFlags)
{
	LogUnimplemented("Actor.Trace() [U227 - BSPTraceFlags parameter isn't implemented");
	TraceFlags flags;
	flags.movers = true;
	flags.world = bTraceBSP;
	if (bTraceActors)
	{
		flags.pawns = true;
		flags.others = true;
		flags.onlyProjectiles = true;
	}

	// hack?
	if (IsA("ChallengeHUD"))
	{
		flags.zoneChanges = true;
	}

	CollisionHit hit = XLevel()->Collision.TraceFirstHit(traceStart, traceEnd, this, extent, flags);
	hitNormal = hit.Normal;
	hitLocation = traceStart + (traceEnd - traceStart) * hit.Fraction;
	return hit.Actor;
}

bool UActor::FastTrace(const vec3& traceEnd, const vec3& traceStart)
{
	return !XLevel()->Collision.TraceAnyHit(traceStart, traceEnd, this, false, true, false);
}

bool UActor::TraceSurfHitInfo(vec3& Start, vec3& End, vec3* HitLocation, vec3* HitNormal, UTexture* HitTex, int* HitFlags)
{
	const TraceFlags flags = {
		.movers = true,
		.world = true
	};

	const auto hit = XLevel()->Collision.TraceFirstHit(Start, End, this, vec3(), flags);

	if (!hit.Node)
		return false;

	if (HitLocation)
		*HitLocation = Start + (End - Start) * hit.Fraction;

	if (HitNormal)
		*HitNormal = hit.Normal;

	if (HitTex)
		HitTex = XLevel()->Model->Surfaces[hit.Node->Surf].Material;

	if (HitFlags)
		*HitFlags = hit.Node->NodeFlags;

	return true;
}

bool UActor::TraceThisActor(vec3& TraceEnd, vec3 TraceStart, vec3* HitLocation, vec3* HitNormal, std::optional<vec3> Extent)
{
	TraceFlags flags{
		.pawns = true,
		.movers = true,
		.others = true,
		.world = true
	};

	const auto hit = XLevel()->Collision.TraceFirstHit(TraceStart, TraceEnd, this, Extent ? *Extent : vec3(), flags);

	if (!hit.Node && !hit.Actor)
		return false;

	if (HitLocation)
		*HitLocation = TraceStart + (TraceEnd - TraceStart) * hit.Fraction;

	if (HitNormal)
		*HitNormal = hit.Normal;

	return true;
}

bool UActor::IsBasedOn(UActor* other)
{
	for (UActor* cur = other; cur; cur = cur->ActorBase())
	{
		if (cur == this)
		{
			return true;
		}
	}
	return false;
}

bool UActor::IsOwnedBy(UActor* owner)
{
	for (UActor* cur = this; cur; cur = cur->Owner())
	{
		if (cur == owner)
		{
			return true;
		}
	}
	return false;
}

bool UActor::IsOverlapping(UActor* other)
{
	return XLevel()->Collision.IsOverlapping(this, other);
}

CollisionHit UActor::TryMove(const vec3& delta, bool dryRun, bool isOwnBaseBlocking)
{
	// Static and non-movable objects can't move
	if (bStatic() || !bMovable())
	{
		CollisionHit hit;
		hit.Fraction = 0.0f;
		return hit;
	}

	// Avoid moving if movement is too small as the physics code doesn't like very small numbers
	if (dot(delta, delta) < 0.00000001f)
		return {};

	// Analyze what we will hit if we move as requested and stop if it is the level or a blocking actor
	bool useBlockPlayers = UObject::TryCast<UPlayerPawn>(this) || UObject::TryCast<UProjectile>(this);
	CollisionHit blockingHit;
	CollisionHitList hits;
	if (!Brush())
	{
		hits = XLevel()->Collision.Trace(Location(), Location() + delta, CollisionHeight(), CollisionRadius(), bCollideActors(), bCollideWorld(), false);
		if (bCollideWorld() || bBlockActors() || bBlockPlayers())
		{
			for (auto& hit : hits)
			{
				if (hit.Actor)
				{
					bool isBlocking;
					if (useBlockPlayers || UObject::TryCast<UPlayerPawn>(hit.Actor) || UObject::TryCast<UProjectile>(hit.Actor))
						isBlocking = hit.Actor->bBlockPlayers() && bBlockPlayers();
					else
						isBlocking = hit.Actor->bBlockActors() && bBlockActors();

					// We never hit ourselves or anything moving along with us
					if (isBlocking && (isOwnBaseBlocking || !hit.Actor->IsBasedOn(this)) && !IsBasedOn(hit.Actor))
					{
						blockingHit = hit;
						break;
					}
				}
				else
				{
					blockingHit = hit;
					break;
				}
			}
		}
	}

	if (dryRun)
		return blockingHit;

	vec3 actuallyMoved = delta * blockingHit.Fraction;
	vec3 OldLocation = Location();

	XLevel()->Collision.RemoveFromCollision(this);
	XLevel()->Light.RemoveLight(this);
	Location() += actuallyMoved;
	XLevel()->Collision.AddToCollision(this);
	XLevel()->Light.AddLight(this);

	for (size_t i = 0; i < BasedActors.size(); )
	{
		UActor* basedActor = BasedActors[i];
		basedActor->TryMove(actuallyMoved, false, false);
		// UnrealScript events triggered in TryMove can call methods such as SetBase or Destroy, so need to guard while iterating.
		if (i < BasedActors.size() && BasedActors[i] == basedActor)
			i++;
	}

	// Notify actor of encroachment
	if (Brush() && (bBlockPlayers() || bBlockActors() || bCollideActors()))
	{
		Array<UActor*> encroachingActors = XLevel()->Collision.EncroachingActors(this);
		for (UActor* actor : encroachingActors)
		{
			if (actor == this || actor->Brush())
				continue;

			bool isBlocking;
			if (useBlockPlayers || UObject::TryCast<UPlayerPawn>(actor) || UObject::TryCast<UProjectile>(actor))
				isBlocking = actor->bBlockPlayers() && bBlockPlayers();
			else
				isBlocking = actor->bBlockActors() && bBlockActors();

			if (isBlocking)
			{
				bool stopMovement = CallEvent(this, EventName::EncroachingOn, { ExpressionValue::ObjectValue(actor) }).ToBool();
				if (stopMovement)
				{
					XLevel()->Collision.RemoveFromCollision(this);
					XLevel()->Light.RemoveLight(this);
					Location() = OldLocation;
					XLevel()->Collision.AddToCollision(this);
					XLevel()->Light.AddLight(this);

					CollisionHit hit;
					hit.Fraction = 0.0f;
					return hit;
				}
			}
		}

		for (UActor* actor : encroachingActors)
		{
			if (actor == this)
				continue;

			bool isBlocking;
			if (useBlockPlayers || UObject::TryCast<UPlayerPawn>(actor) || UObject::TryCast<UProjectile>(actor))
				isBlocking = actor->bBlockPlayers() && bBlockPlayers();
			else
				isBlocking = actor->bBlockActors() && bBlockActors();

			if (isBlocking)
				CallEvent(actor, EventName::EncroachedBy, { ExpressionValue::ObjectValue(this) }).ToBool();
		}
	}

	// Send bump notification if we hit an actor
	if (blockingHit.Actor)
	{
		if (!blockingHit.Actor->IsBasedOn(this))
		{
			CallEvent(blockingHit.Actor, EventName::Bump, { ExpressionValue::ObjectValue(this) });
			CallEvent(this, EventName::Bump, { ExpressionValue::ObjectValue(blockingHit.Actor) });
		}
	}

	// Send touch notifications for anything we crossed while moving
	for (auto& hit : hits)
	{
		if (hit.Fraction >= blockingHit.Fraction)
			break;

		if (hit.Actor && !hit.Actor->IsBasedOn(this) && !IsBasedOn(hit.Actor) && bCollideActors() && hit.Actor->bCollideActors())
		{
			// We can't touch stuff we are blocked by
			bool isBlocking;
			if (useBlockPlayers || UObject::TryCast<UPlayerPawn>(hit.Actor) || UObject::TryCast<UProjectile>(hit.Actor))
				isBlocking = hit.Actor->bBlockPlayers() && bBlockPlayers();
			else
				isBlocking = hit.Actor->bBlockActors() && bBlockActors();
			if (!isBlocking)
				Touch(hit.Actor);
		}
	}

	// Untouch everything we aren't overlapping anymore
	if (engine->LaunchInfo.IsUnrealTournament_469())
	{
		for (const auto actor : Touching_UT469())
			if (actor && !IsOverlapping(actor))
				UnTouch(actor);
	}
	else
	{
		for (const auto actor : Touching())
			if (actor && !IsOverlapping(actor))
				UnTouch(actor);
	}

	UpdateActorZone();

	return blockingHit;
}

CollisionHit UActor::TryMoveSmooth(const vec3& delta)
{
	CollisionHit hit = TryMove(delta);
	if (hit.Fraction != 1.0f)
	{
		// We hit a slope. Try to follow it.
		vec3 alignedDelta = (delta - hit.Normal * dot(delta, hit.Normal)) * (1.0f - hit.Fraction);
		if (dot(delta, alignedDelta) >= 0.0f) // Don't end up going backwards
		{
			CollisionHit hit2 = TryMove(alignedDelta);
			return hit2; // XXX: does this break anything?
		}
	}

	return hit;
}

void UActor::Touch(UActor* actor)
{
	// Don't setup touch if any object has been destroyed
	if (bDeleteMe() || actor->bDeleteMe())
		return;

	if (engine->LaunchInfo.IsUnrealTournament_469())
	{
		auto TouchingArray = Touching_UT469();
		auto TouchingArray2 = actor->Touching_UT469();

		// Do nothing if actors are already touching
		for (int i = 0; i < TouchingArray.size(); i++)
		{
			if (TouchingArray[i] == actor)
				return;
		}

		// Only setup touch if we have room in both arrays
		int slot1 = -1, slot2 = -1;
		for (int i = 0; i < TouchingArray.size(); i++)
		{
			if (slot1 == -1 && TouchingArray[i] == nullptr)
				slot1 = i;
			if (slot2 == -1 && TouchingArray2[i] == nullptr)
				slot2 = i;
		}
		if (slot1 == -1 || slot2 == -1)
			return;

		// Setup links first so Destroy or recursive Touch calls always finds the touch binding
		TouchingArray[slot1] = actor;
		TouchEventSent[slot1] = true;
		TouchingArray2[slot2] = this;
		actor->TouchEventSent[slot2] = false;

		// Notify unrealscript for first actor
		CallEvent(this, EventName::Touch, { ExpressionValue::ObjectValue(actor) });

		// Notify unrealscript for second actor
		if (!actor->bDeleteMe())
		{
			for (int i = 0; i < TouchingArray.size(); i++)
			{
				if (TouchingArray2[i] == this && !actor->TouchEventSent[i])
				{
					actor->TouchEventSent[i] = true;
					CallEvent(actor, EventName::Touch, { ExpressionValue::ObjectValue(this) });
					break;
				}
			}
		}
	}
	else
	{
		auto TouchingArray = Touching();
		auto TouchingArray2 = actor->Touching();

		// Do nothing if actors are already touching
		for (int i = 0; i < TouchingArraySize; i++)
		{
			if (TouchingArray[i] == actor)
				return;
		}

		// Only setup touch if we have room in both arrays
		int slot1 = -1, slot2 = -1;
		for (int i = 0; i < TouchingArraySize; i++)
		{
			if (slot1 == -1 && TouchingArray[i] == nullptr)
				slot1 = i;
			if (slot2 == -1 && TouchingArray2[i] == nullptr)
				slot2 = i;
		}
		if (slot1 == -1 || slot2 == -1)
			return;

		// Setup links first so Destroy or recursive Touch calls always finds the touch binding
		TouchingArray[slot1] = actor;
		TouchEventSent[slot1] = true;
		TouchingArray2[slot2] = this;
		actor->TouchEventSent[slot2] = false;

		// Notify unrealscript for first actor
		CallEvent(this, EventName::Touch, { ExpressionValue::ObjectValue(actor) });

		// Notify unrealscript for second actor
		if (!actor->bDeleteMe())
		{
			for (int i = 0; i < TouchingArraySize; i++)
			{
				if (TouchingArray2[i] == this && !actor->TouchEventSent[i])
				{
					actor->TouchEventSent[i] = true;
					CallEvent(actor, EventName::Touch, { ExpressionValue::ObjectValue(this) });
					break;
				}
			}
		}
	}
}

void UActor::UnTouch(UActor* actor)
{
	auto TouchingArray = Touching();
	auto TouchingArray2 = actor->Touching();

	if (!bDeleteMe())
	{
		for (int i = 0; i < TouchingArraySize; i++)
		{
			if (TouchingArray[i] == actor)
			{
				TouchingArray[i] = nullptr;
				if (TouchEventSent[i])
				{
					TouchEventSent[i] = false;
					CallEvent(this, EventName::UnTouch, { ExpressionValue::ObjectValue(actor) });
				}
			}
		}
	}

	if (!actor->bDeleteMe())
	{
		for (int i = 0; i < TouchingArraySize; i++)
		{
			if (TouchingArray2[i] == this)
			{
				TouchingArray2[i] = nullptr;
				if (actor->TouchEventSent[i])
				{
					actor->TouchEventSent[i] = false;
					CallEvent(actor, EventName::UnTouch, { ExpressionValue::ObjectValue(this) });
				}
			}
		}
	}
}

bool UActor::Move(const vec3& delta)
{
	return TryMove(delta).Fraction == 1.0f;
}

bool UActor::MoveSmooth(const vec3& delta)
{
	CollisionHit hit = TryMoveSmooth(delta);
	return hit.Fraction != 1.0f;
}

bool UActor::HasAnim(const NameString& sequence)
{
	return Mesh() && Mesh()->GetSequence(sequence);
}

bool UActor::IsAnimating()
{
	return AnimRate() != 0.0f;
}

bool UActor::IsAnimating_HP(std::optional<NameString> RootBone)
{
	LogUnimplemented("Actor.IsAnimating_HP");
	return IsAnimating();
}

void UActor::FinishAnim()
{
	if (bAnimLoop())
	{
		bAnimLoop() = false;
		bAnimFinished() = false;
	}

	if (StateFrame)
		StateFrame->LatentState = LatentRunState::FinishAnim;
}

void UActor::FinishAnim_HP(std::optional<NameString> RootBone)
{
	LogUnimplemented("Actor.FinishAnim_HP");
	FinishAnim();
}

NameString UActor::GetAnimGroup(const NameString& sequence)
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
			return seq->Group;
	}
	return {};
}

// UnrealScript variables controlling animation:
// 
// Tweening means animating (using vertex interpolation) from the last animation's frame to the current animation's first frame
//
// Mesh          - the mesh the animation belongs to
// AnimSequence  - current active animation sequence
// AnimFrame     - how far we've gotten in an animation 0.0 to 1.0 for current animation, negative for interpolation from old animation when tweening
// AnimLast      - end point for AnimFrame (when to stop/loop). It is zero when only tweening (don't play the animation). It is the start of the last frame (1-1/numframes) when playing an animation
// AnimRate      - how far AnimFrame moves in 1 second (AnimFrame += AnimRate * timeElapsed). If negative it is a scale factor used to convert Velocity length to animation speed
// AnimMinRate   - the minimum animation speed when AnimRate is negative (negative AnimRate means it should use length(Velocity) * abs(AnimRate) as the anim speed)
// TweenRate     - how fast to move when AnimFrame is negative (AnimFrame += TweenRate * timeElapsed)
// OldAnimRate   - AnimRate from previous call to PlayAnim/LoopAnim/TweenAnim
// bAnimLoop     - true if the animation should loop when AnimLast is reached
// bAnimNotify   - true if animation notify events should be fired when animating
// bAnimFinished - true if AnimLast was reached and there's no looping

void UActor::PlayAnim(const NameString& sequence, float rate, float tweenTime)
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
		{
			SetTweenFromAnimFrame();

			AnimSequence() = sequence;

			if (seq->NumFrames > 1)
			{
				AnimFrame() = tweenTime > 0.0f ? -1.0f / seq->NumFrames : 0.0f;
				AnimLast() = 1.0f - 1.0f / seq->NumFrames;
				AnimRate() = rate * seq->Rate / seq->NumFrames;
				TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
				bAnimNotify() = !seq->Notifys.empty();
				OldAnimRate() = AnimRate();
			}
			else
			{
				// Special case for 1 frame animations. Simply keep drawing the animation for 0.1 second (or tween duration, if tweening).

				AnimFrame() = -1.0f;
				AnimLast() = 0.0f;
				AnimRate() = 0.0f;
				TweenRate() = tweenTime > 0.0f ? 1.0f / tweenTime : 10.0f;
				bAnimNotify() = false;
				OldAnimRate() = 0.0f;
				AnimMinRate() = 0.0f;
			}

			bAnimLoop() = false;
			bAnimFinished() = false;
		}
	}
}

void UActor::PlayBlendAnim(const NameString& sequenceName, float rate, float tweenTime, int blendSlot)
{
	LogUnimplemented("Actor.PlayBlendAnim");
	if (blendSlot < 0 || blendSlot > 3)
	{
		LogMessage("Invalid channel for PlayBlendAnim!");
		return;
	}
	if (!Mesh())
	{
		LogMessage("No mesh for PlayBlendAnim");
		return;
	}

	MeshAnimSeq* sequence = Mesh()->GetSequence(sequenceName);
	if (!sequence)
	{
		LogMessage("Sequence not found for PlayBlendAnim");
		return;
	}

	int numFrames = sequence->NumFrames;
	float sequenceRate = sequence->Rate;

	SetTweenFromBlendAnimFrame(blendSlot);

	BlendAnimSequence()[blendSlot] = sequenceName;

	BlendAnimFrame()[blendSlot] = -1.0f / numFrames;

	BlendAnimRate()[blendSlot] = (rate * sequenceRate) / numFrames;

	BlendAnimLast()[blendSlot] = 1.0f - (1.0f / numFrames);

	if (BlendAnimLast()[blendSlot] == 0.0f)
	{
		BlendAnimRate()[blendSlot] = 0.0f;
		BlendAnimFrame()[blendSlot] = 0.0f;

		BlendTweenRate()[blendSlot] = (tweenTime <= 0.0f) ? 10.0f : (1.0f / tweenTime);
	}
	else if (tweenTime <= 0.0f)
	{
		if (tweenTime == -1.0f)
		{
			BlendAnimFrame()[blendSlot] = 0.0f;
			if (BlendAnimMinRate()[blendSlot] <= 0.0f)
			{
				if (BlendAnimMinRate()[blendSlot] == 0.0f)
				{
					BlendTweenRate()[blendSlot] = 1.0f / (numFrames * 0.025f);
				}
				else
				{
					float speed = length(Velocity());
					float computed = speed * (-BlendAnimMinRate()[blendSlot]);
					float minVal = BlendAnimRate()[blendSlot] * 0.5f;

					BlendTweenRate()[blendSlot] = std::max(computed, minVal);
				}
			}
			else
			{
				BlendTweenRate()[blendSlot] = BlendAnimMinRate()[blendSlot];
			}
		}
		else
		{
			BlendTweenRate()[blendSlot] = 0.0f;
			BlendAnimFrame()[blendSlot] = 0.001f;
		}
	}
	else
	{
		BlendTweenRate()[blendSlot] = 1.0f / (numFrames * tweenTime);
	}

	float oldX = SimBlendAnim()[blendSlot].x;
	float oldY = SimBlendAnim()[blendSlot].y;
	float oldZ = SimBlendAnim()[blendSlot].z;
	float oldW = SimBlendAnim()[blendSlot].w;

	SimBlendAnim()[blendSlot].z = BlendAnimFrame()[blendSlot] * 10000.0f;
	SimBlendAnim()[blendSlot].w = BlendAnimRate()[blendSlot] * 10000.0f;
	SimBlendAnim()[blendSlot].x = BlendTweenRate()[blendSlot] * 1000.0f;
	SimBlendAnim()[blendSlot].y = BlendAnimLast()[blendSlot] * 10000.0f;

	if (oldZ == SimBlendAnim()[blendSlot].z && oldW == SimBlendAnim()[blendSlot].w && oldX == SimBlendAnim()[blendSlot].x && oldY == SimBlendAnim()[blendSlot].y)
	{
		SimBlendAnim()[blendSlot].y += 1.0f;
	}

	OldBlendAnimRate()[blendSlot] = BlendAnimRate()[blendSlot];
}

void UActor::TweenBlendAnim(const NameString& sequenceName, float time, int blendSlot)
{
	if (blendSlot < 0 || blendSlot > 3)
	{
		LogMessage("Invalid channel for TweenBlendAnim!");
		return;
	}
	if (!Mesh())
	{
		LogMessage("No mesh for TweenBlendAnim");
		return;
	}

	MeshAnimSeq* sequence = Mesh()->GetSequence(sequenceName);
	if (!sequence || sequence->Name != sequenceName)
	{
		LogMessage("TweenBlendAnim: Sequence '" + sequenceName.ToString() + "' not found in mesh for slot " + std::to_string(blendSlot));
		return;
	}
	int numFrames = sequence->NumFrames;
	LogMessage("TweenBlendAnim: seq='" + sequenceName.ToString() + "' slot=" + std::to_string(blendSlot) + " time=" + std::to_string(time) + " numFrames=" + std::to_string(numFrames) + " StartFrame=" + std::to_string(sequence->StartFrame));

	BlendAnimSequence()[blendSlot] = sequenceName;
	BlendAnimLast()[blendSlot] = 0.0;
	BlendAnimMinRate()[blendSlot] = 0.0;
	BlendAnimRate()[blendSlot] = 0.0;
	OldBlendAnimRate()[blendSlot] = 0.0;
	if (time <= 0.0)
	{
		BlendTweenRate()[blendSlot] = 0.0;
		BlendAnimFrame()[blendSlot] = 0.0;
	}
	else
	{
		BlendTweenRate()[blendSlot] = 1.0f / (numFrames * time);
		BlendAnimFrame()[blendSlot] = 1.0f / numFrames;
	}
	// Don't worry about simblendanim for now
	return;
}

void UActor::LoopAnim(const NameString& sequence, float rate, float tweenTime, float minRate)
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
		{
			if (AnimSequence() == sequence && IsAnimating() && bAnimLoop())
			{
				if (seq->NumFrames > 1)
				{
					AnimRate() = rate * seq->Rate / seq->NumFrames;
					AnimMinRate() = minRate * seq->Rate / seq->NumFrames;
					TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
					OldAnimRate() = AnimRate();
				}
			}
			else
			{
				SetTweenFromAnimFrame();

				AnimSequence() = sequence;
				if (seq->NumFrames > 1)
				{
					AnimFrame() = tweenTime > 0.0f ? -1.0f / seq->NumFrames : 0.0f;
					AnimLast() = 1.0f - 1.0f / seq->NumFrames;
					bAnimNotify() = !seq->Notifys.empty();
					AnimRate() = rate * seq->Rate / seq->NumFrames;
					AnimMinRate() = minRate * seq->Rate / seq->NumFrames;
					TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
					OldAnimRate() = AnimRate();
				}
				else
				{
					// Special case for 1 frame animations. Simply keep drawing the animation for 0.1 second (or tween duration, if tweening).

					AnimFrame() = -1.0f;
					AnimLast() = 0.0f;
					AnimRate() = 0.0f;
					TweenRate() = tweenTime > 0.0f ? 1.0f / tweenTime : 10.0f;
					bAnimNotify() = false;
					OldAnimRate() = 0.0f;
					AnimMinRate() = 0.0f;
				}
				bAnimFinished() = false;
				bAnimLoop() = true;
			}
		}
	}
}

void UActor::TweenAnim(const NameString& sequence, float tweenTime)
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
		{
			SetTweenFromAnimFrame();

			AnimSequence() = sequence;
			AnimFrame() = tweenTime > 0.0f ? -1.0f / seq->NumFrames : 0.0f;
			AnimLast() = 0.0f;
			AnimRate() = 0.0f;
			AnimMinRate() = 0.0f;
			TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
			OldAnimRate() = AnimRate();
			bAnimNotify() = false;
			bAnimFinished() = false;
			bAnimLoop() = false;
		}
	}
}

void UActor::TickAnimation(float elapsed)
{
	if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
	{
		if (!IsAnimating() || AnimFrame() >= AnimLast())
			StateFrame->LatentState = LatentRunState::Continue;
	}

	for (int i = 0; elapsed > 0.0f && i < 10; i++)
	{
		// If AnimFrame is positive we are doing a normal animation. If it is negative we are doing a tween animation.
		float fromAnimTime = AnimFrame();
		if (fromAnimTime >= 0.0f)
		{
			// If AnimRate is positive we are animating at a fixed rate. If it is negative we animate based on velocity (using AnimRate as a speed scale factor)
			float animRate = (AnimRate() >= 0) ? AnimRate() : std::max(AnimMinRate(), -AnimRate() * length(Velocity()));
			if (animRate == 0.0f)
				break;

			// Find what time will we be at the end of the animation
			float toAnimTime = fromAnimTime + animRate * elapsed;

			// Stop at the next notify event, if any
			if (Mesh() && bAnimNotify())
			{
				MeshAnimSeq* seq = Mesh()->GetSequence(AnimSequence());
				if (seq)
				{
					bool foundEvent = false;
					for (const MeshAnimNotify& n : seq->Notifys)
					{
						if (n.Time > fromAnimTime && n.Time <= toAnimTime)
						{
							if (FindEventFunction(this, n.Function))
							{
								toAnimTime = n.Time;
								elapsed -= (toAnimTime - fromAnimTime) / animRate;
								AnimFrame() = toAnimTime;
								foundEvent = true;
								CallEvent(this, n.Function);
								break;
							}
						}
					}
					if (foundEvent)
						continue;
				}
			}

			// Looped animations got their AnimEnd notify event at the AnimLast point, NOT when the loop finishes!
			if (bAnimLoop() && AnimLast() > fromAnimTime && AnimLast() <= toAnimTime)
			{
				toAnimTime = AnimLast();
				elapsed -= (toAnimTime - fromAnimTime) / animRate;
				AnimFrame() = toAnimTime;

				if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
					StateFrame->LatentState = LatentRunState::Continue;

				CallEvent(this, EventName::AnimEnd);
				continue;
			}

			// Clamp elapsed time to the animation end. This differs for looping animations as they also have to take the last frame into account before looping.
			float animEndTime = bAnimLoop() ? 1.0f : AnimLast();
			if (toAnimTime < fromAnimTime) // This can happen if FinishAnim is called after a looping animation made it past the AnimLast point
			{
				toAnimTime = fromAnimTime;
				animEndTime = fromAnimTime;
				elapsed = 0.0f;
			}
			else if (toAnimTime >= animEndTime)
			{
				elapsed -= (animEndTime - fromAnimTime) / animRate;
				toAnimTime = animEndTime;
			}
			else
			{
				elapsed = 0.0f;
			}

			AnimFrame() = toAnimTime;

			if (toAnimTime == animEndTime)
			{
				if (bAnimLoop())
				{
					AnimFrame() = 0.0f;
				}
				else
				{
					AnimRate() = 0.0f;
					bAnimFinished() = true;
				}
			}

			if (!bAnimLoop() && fromAnimTime < animEndTime && toAnimTime >= animEndTime)
			{
				if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
					StateFrame->LatentState = LatentRunState::Continue;

				CallEvent(this, EventName::AnimEnd);
			}
		}
		else
		{
			float tweenRate = TweenRate();
			if (tweenRate == 0.0f)
				break;

			float toAnimTime = fromAnimTime + tweenRate * elapsed;

			float animEndTime = 0.0f;
			if (toAnimTime >= animEndTime)
			{
				elapsed -= (animEndTime - fromAnimTime) / tweenRate;
				toAnimTime = animEndTime;
			}
			else
			{
				elapsed = 0.0f;
			}

			AnimFrame() = toAnimTime;

			if (toAnimTime == animEndTime && AnimRate() == 0.0f)
			{
				if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
					StateFrame->LatentState = LatentRunState::Continue;

				bAnimFinished() = true;
				//engine->LogMessage("CallEvent(AnimEnd) for " + Class->FriendlyName.ToString() + "");
				CallEvent(this, EventName::AnimEnd);
			}
		}
	}
}

void UActor::TickBlendAnimation(float elapsed)
{
	for (int i = 0; elapsed > 0.0f && i < 4; i++)
	{
		if (BlendAnimSequence()[i].IsNone())
			continue;

		if (BlendAnimFrame()[i] >= BlendAnimLast()[i])
			continue;

		float oldFrame = BlendAnimFrame()[i];

		if (BlendAnimFrame()[i] < 0.0f)
		{
			BlendAnimFrame()[i] += elapsed * BlendTweenRate()[i];

			if (BlendAnimFrame()[i] < 0.0f)
				continue;

			BlendAnimFrame()[i] = 0.0f;

			elapsed = (BlendAnimFrame()[i] * elapsed) / (BlendAnimFrame()[i] - oldFrame);
			continue;
		}

		if (BlendAnimRate()[i] < 0.0f)
		{
			float speed = length(Velocity());

			float adjustedRate = -speed * BlendAnimRate()[i];

			float minRate = BlendAnimLast()[i];
			if (adjustedRate > minRate)
				adjustedRate = minRate;

			BlendAnimFrame()[i] += adjustedRate * elapsed;
		}
		else
		{
			BlendAnimFrame()[i] += BlendAnimRate()[i] * elapsed;
		}

		if (BlendAnimFrame()[i] >= BlendAnimLast()[i])
		{
			float endFrame = BlendAnimLast()[i];

			BlendAnimFrame()[i] = endFrame;
			BlendAnimRate()[i] = 0.0f;

			elapsed = ((BlendAnimFrame()[i] - endFrame) * elapsed) / (BlendAnimFrame()[i] - oldFrame);

			if (RemoteRole() < ENetRole::ROLE_SimulatedProxy)
			{
				SimBlendAnim()[i].z = BlendAnimFrame()[i] * 10000.0f;

				float rate = BlendAnimRate()[i] * 5000.0f;
				if (rate > 32767.0f)
					rate = 32767.0f;

				SimBlendAnim()[i].w = rate;
			}
		}
	}
}

void UActor::SetTweenFromAnimFrame()
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(AnimSequence());
		if (seq)
		{
			float animFrame = std::max(AnimFrame(), 0.0f) * seq->NumFrames;
			int frame0 = (int)animFrame;
			int frame1 = frame0 + 1;
			frame0 = frame0 % seq->NumFrames;
			frame1 = frame1 % seq->NumFrames;
			TweenFromAnimFrame.V0 = (seq->StartFrame + frame0) * Mesh()->FrameVerts;
			TweenFromAnimFrame.V1 = (seq->StartFrame + frame1) * Mesh()->FrameVerts;
			TweenFromAnimFrame.T = animFrame - (float)frame0;
		}
		else // For safety. Should never happen.
		{
			TweenFromAnimFrame.V0 = 0;
			TweenFromAnimFrame.V1 = 0;
			TweenFromAnimFrame.T = -1.0f;
		}
	}
}

void UActor::SetTweenFromBlendAnimFrame(int slot)
{
	if (slot < 0 || slot > 3)
		return;

	if (!Mesh())
		return;

	if (BlendAnimSequence()[slot].IsNone())
	{
		TweenFromBlendAnimFrame[slot].V0 = 0;
		TweenFromBlendAnimFrame[slot].V1 = 0;
		TweenFromBlendAnimFrame[slot].T = -1.0f;
		return;
	}

	MeshAnimSeq* seq = Mesh()->GetSequence(BlendAnimSequence()[slot]);
	if (seq)
	{
		float frame = std::max(BlendAnimFrame()[slot], 0.0f) * seq->NumFrames;
		int frame0 = (int)frame;
		int frame1 = frame0 + 1;
		frame0 = frame0 % seq->NumFrames;
		frame1 = frame1 % seq->NumFrames;
		TweenFromBlendAnimFrame[slot].V0 = (seq->StartFrame + frame0) * Mesh()->FrameVerts;
		TweenFromBlendAnimFrame[slot].V1 = (seq->StartFrame + frame1) * Mesh()->FrameVerts;
		TweenFromBlendAnimFrame[slot].T = frame - (float)frame0;
	}
	else
	{
		TweenFromBlendAnimFrame[slot].V0 = 0;
		TweenFromBlendAnimFrame[slot].V1 = 0;
		TweenFromBlendAnimFrame[slot].T = -1.0f;
	}
}

void UActor::MakeNoise(float loudness)
{
	UPawn* noisePawn = UObject::Cast<UPawn>(Instigator());

	if (!noisePawn || Level()->NetMode() == NM_Client)
		return;

	float currentTime = Level()->TimeSeconds();
	vec3 delta1 = noisePawn->noise1spot() - Location();
	vec3 delta2 = noisePawn->noise2spot() - Location();
	if ((noisePawn->noise1time() > currentTime - 0.2f && dot(delta1, delta1) < 2500.0f && noisePawn->noise1loudness() >= 0.9f * loudness) ||
		(noisePawn->noise2time() > currentTime - 0.2f && dot(delta2, delta2) < 2500.0f && noisePawn->noise2loudness() >= 0.9f * loudness))
	{
		return;
	}

	if (noisePawn->noise1time() < currentTime - 0.18f)
	{
		noisePawn->noise1time() = currentTime;
		noisePawn->noise1spot() = Location();
		noisePawn->noise1loudness() = loudness;
	}
	else if (noisePawn->noise2time() < currentTime - 0.18f)
	{
		noisePawn->noise2time() = currentTime;
		noisePawn->noise2spot() = Location();
		noisePawn->noise2loudness() = loudness;
	}
	else if (dot(delta1, delta1) < 2500.0f)
	{
		noisePawn->noise1time() = currentTime;
		noisePawn->noise1spot() = Location();
		noisePawn->noise1loudness() = loudness;
	}
	else if (noisePawn->noise2loudness() <= loudness)
	{
		noisePawn->noise2time() = currentTime;
		noisePawn->noise2spot() = Location();
		noisePawn->noise2loudness() = loudness;
	}

	for (UPawn* pawn = Level()->PawnList(); pawn != nullptr; pawn = pawn->nextPawn())
	{
		if (pawn != noisePawn && pawn->CanHearNoise(this, loudness))
		{
			CallEvent(pawn, EventName::HearNoise, { ExpressionValue::FloatValue(loudness), ExpressionValue::ObjectValue(this) });
		}
	}
}

bool UActor::PlayerCanSeeMe()
{
	for (UPawn* pawn = Level()->PawnList(); pawn != nullptr; pawn = pawn->nextPawn())
	{
		if (pawn == this)
			continue;

		vec3 L = Location() - pawn->Location();
		float dist2 = dot(L, L);

		// Too far away
		if (dist2 > 500 * 500)
			continue;

		// Without behind view the pawn can only see in a 75 degree cone in front of them
		if (!pawn->bBehindView())
		{
			vec3 viewDirection = Coords::Rotation(pawn->ViewRotation()).XAxis;
			if (dot(viewDirection, L) < 0.2588190451f * dist2)
				continue;
		}

		// Try check for line of sight
		vec3 eyePos = pawn->Location();
		eyePos.z += pawn->BaseEyeHeight();
		if (pawn->FastTrace(Location(), eyePos))
			return true;
	}
	return false;
}

void UActor::UpdateBspInfo()
{
	// Figure out where the actor is visually located in the world
	BBox bbox;
	EDrawType dt = (EDrawType)DrawType();
	if (dt == DT_Mesh && Mesh())
	{
		UMesh* mesh = Mesh();
		Coords rotation = Coords::Rotation(Rotation());
		mat4 objectToWorld = mat4::translate(Location() + PrePivot()) * Coords::Rotation(Rotation()).ToMatrix() * mat4::scale(DrawScale());
		mat4 meshToWorld = objectToWorld * mesh->meshToObject;
		bbox = mesh->BoundingBox.transform(meshToWorld);
	}
	else if ((dt == DT_Sprite || dt == DT_SpriteAnimOnce) && (Texture()))
	{
		vec3 location = Location();
		auto texWidth = Texture()->UsedMipmaps[0].Width;
		auto texHeight = Texture()->UsedMipmaps[0].Height;
		// vec3 extents = vec3(100.0f); // To do: this is wrong. We need the size of a sprite
		vec3 extents = vec3(std::max(texWidth, texHeight) * 0.5f * DrawScale());
		bbox.min = location - extents;
		bbox.max = location + extents;
	}
	else if (dt == DT_Brush && Brush())
	{
		UModel* brush = Brush();
		if (UMover* mover = UObject::TryCast<UMover>(this))
		{
			mat4 objectToWorld = mat4::translate(Location()) * Coords::Rotation(Rotation()).ToMatrix() * mat4::scale(mover->MainScale().Scale) * mat4::translate(-PrePivot());
			bbox = brush->BoundingBox.transform(objectToWorld);
		}
		else
		{
			bbox.min = vec3(0.0f);
			bbox.max = vec3(0.0f);
		}
	}
	else
	{
		bbox.min = vec3(0.0f);
		bbox.max = vec3(0.0f);
	}

	// Is actor still in the bsp tree at the correct location?
	if (!BspInfo.Node || BspInfo.BoundingBox != bbox)
	{
		RemoveFromBspNode();

		BspInfo.BoundingBox = bbox;

		vec3 location = bbox.center();
		vec3 extents = bbox.extents();

		ULevel* level = XLevel();
		BspNode* node = level ? &level->Model->Nodes[0] : nullptr;
		while (node)
		{
			int side = NodeAABBOverlap(location, extents, node);
			if (side == 0 || (side < 0 && node->Front < 0) || (side > 0 && node->Back < 0))
			{
				AddToBspNode(node);
				break;
			}
			else if (side < 0)
			{
				node = &level->Model->Nodes[node->Front];
			}
			else
			{
				node = &level->Model->Nodes[node->Back];
			}
		}
	}
}

void UActor::AddToBspNode(BspNode* node)
{
	BspInfo.Node = node;

	if (node->ActorList)
	{
		node->ActorList->BspInfo.Prev = this;
		BspInfo.Next = node->ActorList;
	}

	node->ActorList = this;
}

void UActor::RemoveFromBspNode()
{
	if (BspInfo.Node)
	{
		if (BspInfo.Next)
		{
			BspInfo.Next->BspInfo.Prev = BspInfo.Prev;
		}
		if (BspInfo.Prev)
		{
			BspInfo.Prev->BspInfo.Next = BspInfo.Next;
		}
		if (BspInfo.Node->ActorList == this)
		{
			BspInfo.Node->ActorList = BspInfo.Next;
		}
		BspInfo.Node = nullptr;
		BspInfo.Prev = nullptr;
		BspInfo.Next = nullptr;
	}
}

// -1 = inside, 0 = intersects, 1 = outside
int UActor::NodeAABBOverlap(const vec3& center, const vec3& extents, BspNode* node)
{
	float e = extents.x * std::abs(node->PlaneX) + extents.y * std::abs(node->PlaneY) + extents.z * std::abs(node->PlaneZ);
	float s = center.x * node->PlaneX + center.y * node->PlaneY + center.z * node->PlaneZ - node->PlaneW;
	if (s - e > 0.0f)
		return -1;
	else if (s + e < 0.0f)
		return 1;
	else
		return 0;
}

UTexture* UActor::GetMultiskin(int index)
{
	if (engine->LaunchInfo.ue1Version > 219 && index >= 0 && index < 8)
		return MultiSkins()[index];
	else
		return nullptr;
}

void UActor::DeusExConBindEvents()
{
	auto mission = UObject::Cast<UConversationList>(engine->GetDeusExMission());
	if (!mission)
		return;

	UClass* clsConListItem = engine->packages->FindClass("ConSys.ConListItem");
	UConListItem* conListItem = nullptr;

	NameString bindName = BindName();
	if (!bindName.IsNone())
	{
		for (UConItem* item = mission->conversations(); item; item = item->Next())
		{
			auto conversation = UObject::Cast<UConversation>(item->ConObject());
			NameString conOwnerName = conversation->conOwnerName();
			if (conOwnerName == bindName)
			{
				NameString name;
				UConListItem* newItem = UObject::Cast<UConListItem>(engine->LevelPackage->NewObject(name, clsConListItem, ObjectFlags::Transient, true));
				newItem->con() = conversation;
				newItem->Next() = conListItem;
				conListItem = newItem;
			}
		}
	}

	NameString barkBindName = BarkBindName();
	if (!barkBindName.IsNone())
	{
		for (UConItem* item = mission->conversations(); item; item = item->Next())
		{
			auto conversation = UObject::Cast<UConversation>(item->ConObject());
			NameString conOwnerName = conversation->conOwnerName();
			if (conOwnerName == barkBindName)
			{
				NameString name;
				UConListItem* newItem = UObject::Cast<UConListItem>(engine->LevelPackage->NewObject(name, clsConListItem, ObjectFlags::Transient, true));
				newItem->con() = conversation;
				newItem->Next() = conListItem;
				conListItem = newItem;
			}
		}
	}

	ConListItems() = conListItem;
}

void UActor::PlayAnim_HP(const NameString& Sequence, std::optional<float> Rate, std::optional<float> TweenTime, std::optional<EAnimType> Type, std::optional<NameString> RootBone)
{
	LogUnimplemented("Actor.PlayAnim_HP");
}

void UActor::LoopAnim_HP(const NameString& Sequence, std::optional<float> Rate, std::optional<float> TweenTime, std::optional<float> MinRate, std::optional<EAnimType> Type, std::optional<NameString> RootBone)
{
	LogUnimplemented("Actor.LoopAnim_HP");
}

BoundingBox UActor::GetWorldCollisionBox(bool bVisual)
{
	LogUnimplemented("Actor.GetWorldCollisionBox");
	return {};
}

vec3 UActor::GetRenderExtent()
{
	LogUnimplemented("Actor.GetRenderExtent");
	return vec3(100.0f);
}

UActor* UActor::CreateAnimChannel(UClass* NewClass, EAnimType Type, const NameString& RootBone, bool bTransient)
{
	auto animChannel = Spawn(NewClass, {}, {}, {}, {});
	LogUnimplemented("Actor.CreateAnimChannel");
	return animChannel;
}

int UActor::BoneNumber(const NameString& Bone)
{
	LogUnimplemented("Actor.BoneNumber");
	return 0;
}

NameString UActor::BoneName(int Bone)
{
	LogUnimplemented("Actor.BoneName");
	return {};
}

vec3 UActor::BonePos(const NameString& Bone)
{
	LogUnimplemented("Actor.BonePos");
	return vec3(0.0f);
}

UTexture* UActor::CreateTextureFromScreenShot(UViewport* vport)
{
	LogUnimplemented("Actor.CreateTextureFromScreenShot");
	return nullptr;
}

UTexture* UActor::CreateTextureFromBMP(const std::string& name, const std::string& filename)
{
	LogUnimplemented("Actor.CreateTextureFromBMP");
	return nullptr;
}

bool UActor::SaveObjectAsFile(const std::string& dir, UObject* object)
{
	LogUnimplemented("Actor.SaveObjectAsFile");
	return false;
}

bool UActor::LoadObjectAsFile(const std::string& dir, UObject* object)
{
	LogUnimplemented("Actor.LoadObjectAsFile");
	return false;
}

bool UActor::SaveGameSaveInfo(const std::string& dir, UObject* object)
{
	LogUnimplemented("Actor.SaveGameSaveInfo");
	return false;
}

bool UActor::LoadGameSaveInfo(const std::string& dir, UObject* object)
{
	LogUnimplemented("Actor.LoadGameSaveInfo");
	return false;
}

bool UActor::IsOSVer2kOrXP()
{
	return true;
}
