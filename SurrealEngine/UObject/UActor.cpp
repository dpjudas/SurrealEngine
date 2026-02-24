
#include "Precomp.h"
#include "UActor.h"
#include "ULevel.h"
#include "UMesh.h"
#include "UTexture.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "Package/PackageManager.h"
#include "Package/IniProperty.h"
#include "Engine.h"
#include <set>

// TODO: Compare behavior more closely with original engine. Might differ depending on game.
static constexpr float stepDownDeltaFactor = 1.3f;

UActor* UActor::Spawn(UClass* SpawnClass, UActor* SpawnOwner, NameString SpawnTag, vec3* SpawnLocation, Rotator* SpawnRotation)
{
	if (!SpawnClass || SpawnClass->ClsFlags & ClassFlags::Abstract)
	{
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
			return nullptr;
		location = result.second;
	}

	// To do: package needs to be grabbed from outer, or the "transient package" if it is None, a virtual package for runtime objects
	// To do: find unique new name in the package
	NameString name = SpawnClass->Name.ToString() + "0";
	UActor* actor = UObject::Cast<UActor>(engine->LevelPackage->NewObject(name, UObject::Cast<UClass>(SpawnClass), ObjectFlags::Transient, true));

	actor->Outer() = XLevel()->Outer();
	actor->XLevel() = XLevel();
	actor->Level() = Level();
	actor->Tag() = (!SpawnTag.IsNone()) ? SpawnTag : SpawnClass->Name;
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

	actor->SetOwner(SpawnOwner ? SpawnOwner : this);

	if (Level()->bBegunPlay())
	{
		CallEvent(actor, EventName::Spawned);
		CallEvent(actor, EventName::PreBeginPlay);
		CallEvent(actor, EventName::BeginPlay);

		if (actor->bDeleteMe())
			return nullptr;

		// To do: we need to call EventName::EncroachingOn events here?

		actor->InitActorZone();

		CallEvent(actor, EventName::PostBeginPlay);
		CallEvent(actor, EventName::SetInitialState);

		actor->InitBase();

		if (engine->packages->GetEngineVersion() >= 400)
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
	if (engine->packages->GetEngineVersion() > 219)
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

	if (engine->LaunchInfo.engineVersion < 400 && !ActorBase()) // Unreal expects a base to always exist. What about UT? TournamentPlayer seems to indicate not.
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

	ULevel* level = XLevel();

	RemoveFromBspNode();
	level->Collision.RemoveFromCollision(this);

	CallEvent(this, EventName::Destroyed);

	UActor** TouchingArray = Touching();
	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray[i])
			UnTouch(TouchingArray[i]);
	}

	SetOwner(nullptr);

	while (!ChildActors.empty())
	{
		ChildActors.back()->SetOwner(nullptr);
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
		else if (engine->LaunchInfo.engineVersion > 219 && Owner() == nullptr && Region().Zone->bNoInventory() && IsA("Inventory"))
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

void UActor::SetBase(UActor* newBase, bool sendBaseChangeEvent)
{
	if (ActorBase() != newBase)
	{
		for (UActor* cur = newBase; cur; cur = cur->ActorBase())
		{
			if (cur == this)
				return;
		}

		if (ActorBase() && ActorBase() != Level())
		{
			ActorBase()->StandingCount()--;
			CallEvent(ActorBase(), EventName::Detach, { ExpressionValue::ObjectValue(this) });
		}

		ActorBase() = newBase;

		if (ActorBase() && ActorBase() != Level())
		{
			ActorBase()->StandingCount()++;
			CallEvent(ActorBase(), EventName::Attach, { ExpressionValue::ObjectValue(this) });
		}

		if (sendBaseChangeEvent)
			CallEvent(this, EventName::BaseChange);
	}
}

void UActor::Tick(float elapsed)
{
	TickAnimation(elapsed);

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

		if (engine->LaunchInfo.engineVersion >= 400)
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
	bool isMoving = (vel.x != 0.0f && vel.y != 0.0f);
	if (isMoving)
	{
		for (int iteration = 0; timeLeft > 0.0f && iteration < 5; iteration++)
		{
			vec3 moveDelta = vel * timeLeft;

			// step up first so we can get past stairs going up
			TryMove(stepUpDelta);

			// try move forward
			CollisionHit hit = TryMove(moveDelta);
			timeLeft -= timeLeft * hit.Fraction;
			moveDelta = vel * timeLeft;

			// move back down to original vertical position
			TryMove(-stepUpDelta);

			if (hit.Fraction < FLT_EPSILON)
			{
				// try move forward once again, in case our head bumped into something while stepped up
				hit = TryMove(moveDelta);
				timeLeft -= timeLeft * hit.Fraction;
			}

			if (hit.Fraction < 1.0f)
			{
				if (player && hit.Actor)
				{
					if (UObject::IsType<UDecoration>(hit.Actor) && UObject::Cast<UDecoration>(hit.Actor)->bPushable() && dot(hit.Normal, moveDelta) < -0.9f)
					{
						// We hit a pushable decoration that is facing our movement direction

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
		float maxAccel = engine->LaunchInfo.engineVersion > 219 ? pawn->AirControl() * pawn->AccelRate() : 0.0f;
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
			if (hit.Actor && !hit.Actor->IsA("Mover"))
			{
				// TODO: Hit an actor
			}
			else
			{
				CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });

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
	bool isMoving = (vel.x != 0.0f && vel.y != 0.0f);
	if (isMoving)
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

					bJustTeleported() = true;
					Velocity() = Velocity() * Mass() / (Mass() + hit.Actor->Mass());
					CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
					timeLeft = 0.0f;
				}
				else
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
	}

	if (!bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;

	if (!Region().Zone->bWaterZone())
	{
		// We moved out of water.
		// Give the player a push
		if (Velocity().z > 0.0f)
			Velocity().z = std::max(Velocity().z, (100.0f + length(Velocity().xy())) * 0.5f);
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
	bool isMoving = (vel.x != 0.0f && vel.y != 0.0f);
	if (isMoving)
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
			if (engine->LaunchInfo.engineVersion > 219)
			{
				pawn->DesiredFlashScale() = mix(target->ScreenFlashScale(), next->ScreenFlashScale(), physAlpha);
				pawn->DesiredFlashFog() = mix(target->ScreenFlashFog(), next->ScreenFlashFog(), physAlpha);
				pawn->FovAngle() = mix(target->FovModifier(), next->FovModifier(), physAlpha) * Class->GetDefaultObject<UPlayerPawn>()->FovAngle();
				pawn->FlashScale() = vec3(pawn->DesiredFlashScale());
				pawn->FlashFog() = pawn->DesiredFlashFog();
			}
		}

		if (engine->LaunchInfo.engineVersion > 219)
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
		Rotation() = rotation;

		if (auto pawn = UObject::TryCast<UPawn>(this))
		{
			pawn->ViewRotation() = Rotation();
		}

		if (interpolateStart)
		{
			CallEvent(target, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(this) });
			CallEvent(this, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(target) });

			target = target->Prev();
			if (engine->LaunchInfo.engineVersion > 219)
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
			if (engine->LaunchInfo.engineVersion > 219)
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
				Rotation() = targetRotation;
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

	if (engine->LaunchInfo.engineVersion >= 400 && bTrailerPrePivot())
	{
		newLocation += PrePivot();
	}

	SetLocation(newLocation);

	if (engine->LaunchInfo.engineVersion >= 400 && bTrailerSameRotation() && DrawType() != DT_Sprite)
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
	Location() = result.second;
	XLevel()->Collision.AddToCollision(this);

	if (Level()->bBegunPlay())
	{
		// Send touch notifications for anything at the new location
		for (UActor* actor : XLevel()->Collision.CollidingActors(Location(), CollisionHeight(), CollisionRadius()))
		{
			if (actor != this && !actor->IsBasedOn(this) && !IsBasedOn(actor))
			{
				Touch(actor);
			}
		}

		// Untouch everything we aren't overlapping anymore
		UActor** TouchingArray = Touching();
		for (int i = 0; i < TouchingArraySize; i++)
		{
			if (TouchingArray[i] && !IsOverlapping(TouchingArray[i]))
			{
				UnTouch(TouchingArray[i]);
			}
		}
	}

	return true;
}

bool UActor::SetRotation(const Rotator& newRotation)
{
	// To do: return false if there isn't room

	Rotation() = newRotation;
	return true;
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

bool UActor::FastTrace(const vec3& traceEnd, const vec3& traceStart)
{
	return !XLevel()->Collision.TraceAnyHit(traceStart, traceEnd, this, false, true, false);
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
	Location() += actuallyMoved;
	XLevel()->Collision.AddToCollision(this);

	// Based actors needs to move with us
	if (StandingCount() > 0)
	{
		ULevel* level = XLevel();
		for (size_t i = 0; i < level->Actors.size(); i++)
		{
			UActor* actor = level->Actors[i];
			if (actor && actor->ActorBase() == this)
			{
				actor->TryMove(actuallyMoved, false, false);
			}
		}
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
					Location() = OldLocation;
					XLevel()->Collision.AddToCollision(this);

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

		if (hit.Actor && !hit.Actor->IsBasedOn(this) && !IsBasedOn(hit.Actor))
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
	UActor** TouchingArray = Touching();
	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray[i] && !IsOverlapping(TouchingArray[i]))
		{
			UnTouch(TouchingArray[i]);
		}
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

	UActor** TouchingArray = Touching();
	UActor** TouchingArray2 = actor->Touching();

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

void UActor::UnTouch(UActor* actor)
{
	UActor** TouchingArray = Touching();
	UActor** TouchingArray2 = actor->Touching();

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

void UActor::FinishAnim()
{
	if (bAnimLoop())
	{
		bAnimLoop() = false;
		bAnimFinished() = false;
	}

	if (IsAnimating() && AnimFrame() < AnimLast() && StateFrame)
		StateFrame->LatentState = LatentRunState::FinishAnim;
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
		vec3 extents = vec3(100.0f); // To do: this is wrong. We need the size of a sprite
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
	if (engine->LaunchInfo.engineVersion > 219 && index >= 0 && index < 8)
		return (&MultiSkins())[index];
	else
		return nullptr;
}

/////////////////////////////////////////////////////////////////////////////

bool UPawn::ActorReachable(UActor* anActor, bool checkNavpoint)
{
	if (!anActor)
		return false;

	vec3 eyePos = Location();
	eyePos.z += BaseEyeHeight();

	if (checkNavpoint)
	{
		// Check if we are trying to reach a navigation point. They can also be hiding in an inventory for patent-pending spaghetti reasons.
		UNavigationPoint* navPoint = UObject::TryCast<UNavigationPoint>(anActor);
		if (UInventory* inventory = UObject::TryCast<UInventory>(anActor))
			navPoint = inventory->myMarker();

		// If we can reach any navigation point in the map then we can also reach the navigation point asked for
		if (navPoint)
		{
			for (UNavigationPoint* cur = Level()->NavigationPointList(); cur != nullptr; cur = cur->nextNavigationPoint())
			{
				if (std::abs(cur->Location().z - Location().z) >= CollisionHeight())
					continue;

				if (!FastTrace(cur->Location(), eyePos))
					continue;

				if (CheckLocation(cur->Location(), CollisionRadius(), CollisionHeight(), bCollideWorld() || bCollideWhenPlacing()).first)
					return true;
			}
		}
	}

	//if (std::abs(anActor->Location().z - Location().z) >= CollisionHeight())
	//	return false;

	UPawn* aPawn = UObject::TryCast<UPawn>(anActor);
	if (aPawn)
	{
		if (aPawn->FootRegion().Zone->bPainZone() && aPawn->FootRegion().Zone->DamageType() != ReducedDamageType())
			return false;
	}
	else
	{
		vec3 delta = anActor->Location() - Location();
		float dist2 = dot(delta, delta);
		if (dist2 > 1000.0f * 1000.0f)
			return false;
		if (anActor->Region().Zone->bPainZone() && anActor->Region().Zone->DamageType() != ReducedDamageType())
			return false;
	}

	if (anActor->Region().Zone->bWaterZone() && !bCanSwim())
		return false;

	if (!FastTrace(anActor->Location(), eyePos))
		return false;

	return CheckLocation(anActor->Location(), CollisionRadius(), CollisionHeight(), bCollideWorld() || bCollideWhenPlacing()).first;
}

bool UPawn::PointReachable(vec3 aPoint)
{
	PointRegion pointRegion = XLevel()->Model->FindRegion(aPoint, Level());

	if (!Region().Zone->bWaterZone() && !bCanSwim() && pointRegion.Zone->bWaterZone())
		return false;
	if (!FootRegion().Zone->bPainZone() && pointRegion.Zone->bPainZone() && pointRegion.Zone->DamageType() != ReducedDamageType())
		return false;

	vec3 eyePos = Location();
	eyePos.z += BaseEyeHeight();
	if (!FastTrace(aPoint, eyePos))
		return false;

	return CheckLocation(aPoint, CollisionRadius(), CollisionHeight(), bCollideWorld() || bCollideWhenPlacing()).first;
}

bool UPawn::PickWallAdjust()
{
	auto kneeHeight = CollisionHeight() * 0.45f;

	auto forwards = normalize(Acceleration().xy());

	auto afterJumpCollisionHit = TryMove(vec3(forwards, kneeHeight), true);

	if (afterJumpCollisionHit.Fraction == 1)
	{
		// Obstacle can be jumped over. Attempt jumping.
		bFromWall() = false;
		Velocity().z = JumpZ();
		SetPhysics(PHYS_Falling);
		Destination() = Location() + vec3(forwards, kneeHeight);

		return true;
	}

	// Obstacle cannot be jumped over. Try another direction
	auto direction = Focus() - Location();
	auto rightSideVec = normalize(cross(direction, vec3(0, 0, 1)));
	auto rightSideTest = TryMove(rightSideVec, true);
	if (rightSideTest.Fraction == 1)
	{
		// We can move to right instead
		bFromWall() = true;
		Destination() = Location() + rightSideVec;
		// Focus() = Location() + rightSideVec;

		return true;
	}

	auto leftSideVec = -rightSideVec;
	auto leftSideTest = TryMove(leftSideVec, true);
	if (leftSideTest.Fraction >= 1)
	{
		// We can move to left instead
		bFromWall() = true;
		Destination() = Location() + leftSideVec;
		// Focus() = Location() + leftSideVec;

		return true;
	}

	// Cannot go anywhere from here
	return false;
}

vec3 UPawn::EAdjustJump()
{
	UZoneInfo* zone = FootRegion().Zone;
	vec3 gravity = zone ? zone->ZoneGravity() : vec3(0.0f, 0.0f, -980.0f);

	const float dt = 0.05f;
	const float jumpZ = JumpZ();
	vec3 pos = Location();
	vec3 vel = vec3(0.0f, 0.0f, jumpZ);
	float time = 0.0f;
	const float maxSimTime = 5.0f;
	const float targetZ = Location().z;
	while (time < maxSimTime && pos.z < targetZ)
	{
		vel.z += gravity.z * dt;
		pos.z += vel.z * dt;
		time += dt;
		if (pos.z >= targetZ) break;
	}

	vec3 target = Focus();
	if (dot(target - Location(), target - Location()) < 0.001f)
		target = Destination();
	vec3 horizontalDir = normalize(target - Location());
	horizontalDir.z = 0.0f;

	vec3 horizontalVel = horizontalDir * (length(target - Location()) / std::max(time, 0.001f));

	float groundSpeed = GroundSpeed();
	float horizSpeed = length(horizontalVel);
	if (horizSpeed > groundSpeed)
		horizontalVel = horizontalVel * (groundSpeed / horizSpeed);

	return horizontalVel + vec3(0.0f, 0.0f, jumpZ);
}

bool UPawn::LineOfSightTo(UActor* other)
{
	if (!other)
		return false;

	vec3 eye_pos = Location();
	eye_pos.z += BaseEyeHeight();

	auto& origin = other->Location();
	auto top = origin + vec3{ 0.f, 0.f, other->CollisionHeight() / 2 };
	auto bottom = origin - vec3{ 0.f, 0.f, other->CollisionHeight() / 2 };

	return FastTrace(origin, eye_pos) || FastTrace(top, eye_pos) || FastTrace(bottom, eye_pos);
}

bool UPawn::CanSee(UActor* other)
{
	if (!other)
		return false;

	// Two fields to keep in mind of:
	// float SightRadius: Maximum seeing distance
	// float PeripheralVision: Cosine of limits of peripheral vision

	auto& origin = other->Location();
	auto top = origin + vec3{ 0.f, 0.f, other->CollisionHeight() / 2 };
	auto bottom = origin - vec3{ 0.f, 0.f, other->CollisionHeight() / 2 };

	vec3 eye_pos = Location();
	eye_pos.z += BaseEyeHeight();

	// Cannot see if the actor is too far away from the sight radius
	if (length(origin - eye_pos) > SightRadius())
		return false;

	// Cannot see if the actor is outside of the peripheral vision angles
	vec3 orientation = Coords::Rotation(Rotation()).XAxis;

	// Calculate the cosine of the vectors
	// which is basically A dot B / (|A| * |B|), or just the dot products of the normalized versions of A and B
	float cosine = dot(normalize(orientation), normalize(origin));
	// PeripheralVision field is set dynamically during a game session
	// (for an example, see the function UnrealShare.Bots.PreSetMovement())
	// This can be a negative value too, which is probably set to not take it into account
	float peripheralVision = PeripheralVision();

	if (peripheralVision > 0.0f && abs(cosine) > peripheralVision)
		return false;

	return FastTrace(origin, eye_pos) || FastTrace(top, eye_pos) || FastTrace(bottom, eye_pos);
}

bool UPawn::CanHearNoise(UActor* source, float loudness)
{
	UPawn* noisePawn = UObject::Cast<UPawn>(source->Instigator());
	if (!noisePawn->bIsPlayer() && (!noisePawn->Enemy() || !noisePawn->Enemy()->bIsPlayer()))
	{
		if (!IsA(source->Class->Name) && !source->IsA(Class->Name))
			return false;
	}
	else if (UObject::TryCast<UPlayerPawn>(this))
	{
		return false;
	}

	vec3 delta = Location() - source->Location();
	float dist2 = dot(delta, delta);

	if (!bIsPlayer() || !Level()->Game()->bTeamGame() || !noisePawn->bIsPlayer() ||
		(engine->LaunchInfo.engineVersion > 219 && (!PlayerReplicationInfo() || !noisePawn->PlayerReplicationInfo() || (PlayerReplicationInfo()->Team() != noisePawn->PlayerReplicationInfo()->Team()))))
	{
		if (dist2 > (4000.0f * 4000.0f) * (loudness * loudness))
			return false;

		float perceived = std::min(1200000.f / dist2, 2.0f);
		Stimulus() = loudness * perceived + Alertness() * std::min(0.5f, perceived);
		if (Stimulus() < HearingThreshold())
			return false;
	}
	else if (dist2 > (4000.0f * 4000.0f) * (loudness * loudness))
	{
		return false;
	}

	return !XLevel()->Collision.TraceAnyHit(source->Location(), Location(), source, false, true, false);
}

void UPawn::ClientHearSound(UActor* actor, int id, USound* sound, const vec3& soundLocation, const vec3& parameters)
{
	LogUnimplemented("UPawn.ClientHearSound()");
}

UActor* UPawn::PickAnyTarget(float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart)
{
	UActor* bestActor = nullptr;
	for (UActor* actor : XLevel()->Actors)
	{
		// We are only looking for targets that isn't a pawn (pawn uses PickTarget if it wants a pawn)
		if (!actor || actor == this || UObject::TryCast<UPawn>(actor) || !actor->bProjTarget())
			continue;

		if (CheckIfBestTarget(actor, bestAim, bestDist, FireDir, projStart))
			bestActor = actor;
	}
	return bestActor;
}

UActor* UPawn::PickTarget(float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart)
{
	UActor* bestActor = nullptr;
	UPlayerReplicationInfo* ourPlayerInfo = engine->LaunchInfo.engineVersion > 219 ? PlayerReplicationInfo() : nullptr;
	bool teamGame = ourPlayerInfo && Level()->Game()->bTeamGame();
	for (UPawn* pawn = Level()->PawnList(); pawn != nullptr; pawn = pawn->nextPawn())
	{
		// Skip dead pawns or ourselves
		if (pawn == this || pawn->Health() > 0)
			continue;

		// Skip team mates
		auto pawnPlayerInfo = pawn->PlayerReplicationInfo();
		if (teamGame && pawnPlayerInfo && ourPlayerInfo->Team() == pawnPlayerInfo->Team())
			continue;

		if (CheckIfBestTarget(pawn, bestAim, bestDist, FireDir, projStart))
			bestActor = pawn;
	}
	return bestActor;
}

bool UPawn::CheckIfBestTarget(UActor* actor, float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart)
{
	// Ignore targets behind us
	vec3 delta = actor->Location() - projStart;
	float angle = dot(FireDir, delta);
	if (angle < 0.0f)
		return false;

	// Skip things too far away
	float distance = length(delta);
	if (distance == 0.0f || distance > 2500.0f)
		return false;

	// Skip if we already have a target closer to the direction we are facing
	angle /= distance;
	if (angle < bestAim)
		return false;

	// Skip if we can't see the target
	if (!LineOfSightTo(actor))
		return false;

	// OK, this is better than what we have
	bestAim = angle;
	bestDist = distance;
	return true;
}

UNavigationPoint* UPawn::SetRouteCache(const Array<UNavigationPoint*>& points)
{
	if (engine->LaunchInfo.engineVersion > 219)
	{
		UNavigationPoint** cache = RouteCache();
		for (size_t i = 0; i < 16; i++)
			cache[i] = (i < points.size()) ? points[i] : nullptr;
	}
	return !points.empty() ? points.front() : nullptr;
}

Array<UNavigationPoint*> UPawn::FindPathToEndPoint(UNavigationPoint* start, int maxNodes)
{
	if ((start->bPlayerOnly() && !bIsPlayer()))
		return {};

	struct Step
	{
		UNavigationPoint* navpoint;
		int prev;
	};

	std::set<const LevelReachSpec*> visited;
	Array<Step> steps, stepEnds;
	const Array<LevelReachSpec>& reachSpecs = XLevel()->ReachSpecs;
	
	int radius = (int)CollisionRadius();
	int height = (int)CollisionHeight();

	// Search through the nav node links until we find an end point
	// To do: add navpoint.cost calculations into this and pick the cheapest path based on that

	int nextStep = 0;
	UNavigationPoint* current = start;
	while (!current->bEndPoint() && steps.size() < (size_t)maxNodes)
	{
		for (const LevelReachSpec& reachSpec : reachSpecs)
		{
			UNavigationPoint* startActor = reachSpec.startActor;
			UNavigationPoint* endActor = reachSpec.endActor;

			if (startActor == current)
			{
				if (reachSpec.collisionRadius < radius || reachSpec.collisionHeight < height || reachSpec.bPruned)
					continue; // Skip nav node links that we can't pass through

				if ((endActor->bPlayerOnly() && !bIsPlayer()) || (endActor->bPlayerOnly() && !bIsPlayer()))
					continue; // Skip nav nodes only for the player if we aren't one

				// To do: check reachFlags

				if (visited.insert(&reachSpec).second)
				{
					if (endActor->bEndPoint())
					{
						stepEnds.push_back({ .navpoint = endActor, .prev = nextStep - 1 });
					}
					else
					{
						steps.push_back({ .navpoint = endActor, .prev = nextStep - 1 });
					}
				}
			}
		}

		if (nextStep == steps.size())
			break;

		if (!stepEnds.empty())
			break; // To do: remove this and pick the path with the lowest cost

		current = steps[nextStep].navpoint;
		nextStep++;
	}

	if (stepEnds.empty())
		return {};

	// Extract the final path:
	const Step& endstep = stepEnds.front();
	Array<UNavigationPoint*> path;
	path.push_back(endstep.navpoint);
	int currentStep = endstep.prev;
	while (currentStep >= 0)
	{
		const Step& step = steps[currentStep];
		path.push_back(step.navpoint);
		currentStep = step.prev;
	}

	return path;
}

void UPawn::ClearPaths()
{
	for (UNavigationPoint* cur = Level()->NavigationPointList(); cur; cur = cur->nextNavigationPoint())
	{
		cur->bEndPoint() = false;
		if (cur->bSpecialCost())
			cur->cost() = CallEvent(cur, "SpecialCost", { ExpressionValue::ObjectValue(this) }).ToInt();
		else
			cur->cost() = cur->ExtraCost();
	}
}

UObject* UPawn::FindRandomDest()
{
	// Find initial navpoints reachable from our location
	int maxActorReachableCalls = 8; // upper bound for how expensive this can get
	vec3 eyePos = Location();
	eyePos.z += BaseEyeHeight();
	std::vector<UNavigationPoint*> reachablePoints;
	for (UNavigationPoint* navPoint = Level()->NavigationPointList(); navPoint && reachablePoints.size() < maxActorReachableCalls; navPoint = navPoint->nextNavigationPoint())
	{
		if ((navPoint->bPlayerOnly() && !bIsPlayer()) || (navPoint->bPlayerOnly() && !bIsPlayer()))
			continue; // Skip nav nodes only for the player if we aren't one

		float maxDist = 1000.0;
		vec3 d = navPoint->Location() - Location();
		if (dot(d, d) > maxDist * maxDist)
			continue; // Ignore things too far away

		if (!ActorReachable(navPoint))
			continue;

		navPoint->bEndPoint() = true;
		reachablePoints.push_back(navPoint);
	}

	if (reachablePoints.empty())
		return nullptr;

	// Add all navpoints reachable via reachspecs from what we can already reached
	const Array<LevelReachSpec>& reachSpecs = XLevel()->ReachSpecs;
	int radius = (int)CollisionRadius();
	int height = (int)CollisionHeight();
	for (size_t i = 0; i < reachablePoints.size(); i++)
	{
		UNavigationPoint* navPoint = reachablePoints[i];
		for (const LevelReachSpec& reachSpec : reachSpecs)
		{
			if (reachSpec.startActor == navPoint)
			{
				if (reachSpec.endActor->bEndPoint())
					continue; // Already processed

				if (reachSpec.collisionRadius < radius || reachSpec.collisionHeight < height || reachSpec.bPruned)
					continue; // Skip nav node links that we can't pass through

				if ((reachSpec.endActor->bPlayerOnly() && !bIsPlayer()) || (reachSpec.endActor->bPlayerOnly() && !bIsPlayer()))
					continue; // Skip nav nodes only for the player if we aren't one

				// To do: check reachFlags

				reachSpec.endActor->bEndPoint() = true;
				reachablePoints.push_back(reachSpec.endActor);
			}
		}
	}

	// Pick a random point from our candidates
	float randomValue = rand() / (float)RAND_MAX;
	int index = (int)std::round(randomValue * (float)(reachablePoints.size() - 1));
	return reachablePoints[index];
}

UObject* UPawn::FindPathTo(const vec3& aPoint, bool bSinglePath)
{
	return FindPathToward(FindClosestNavPoint(aPoint), bSinglePath);
}

UObject* UPawn::FindPathToward(UObject* anActor, bool singlePath)
{
	if (auto aNavPoint = UObject::TryCast<UNavigationPoint>(anActor))
	{
		vec3 eyePos = Location();
		eyePos.z += BaseEyeHeight();

		int maxActorReachableCalls = 8; // upper bound for how expensive this can get
		int endPointsFound = 0;
		for (UNavigationPoint* navPoint = Level()->NavigationPointList(); navPoint; navPoint = navPoint->nextNavigationPoint())
		{
			navPoint->bEndPoint() = false;

			if (endPointsFound < maxActorReachableCalls)
			{
				if ((navPoint->bPlayerOnly() && !bIsPlayer()) || (navPoint->bPlayerOnly() && !bIsPlayer()))
					continue; // Skip nav nodes only for the player if we aren't one

				float maxDist = 1000.0;
				vec3 d = navPoint->Location() - Location();
				if (dot(d, d) > maxDist * maxDist)
					continue; // Ignore things too far away

				if (!ActorReachable(navPoint))
					continue;

				navPoint->bEndPoint() = true;
				endPointsFound++;
			}
		}

		if (endPointsFound == 0)
			return SetRouteCache({});

		return SetRouteCache(FindPathToEndPoint(aNavPoint, 1000));
	}
	else if (auto actor = UObject::TryCast<UActor>(anActor))
	{
		return FindPathToward(FindClosestNavPoint(actor->Location()), singlePath);
	}
	else
	{
		return SetRouteCache({});
	}
}

UNavigationPoint* UPawn::FindClosestNavPoint(vec3 location)
{
	// Order nav points by distance
	std::vector<std::pair<UNavigationPoint*, float>> navPoints;
	for (UNavigationPoint* navPoint = Level()->NavigationPointList(); navPoint; navPoint = navPoint->nextNavigationPoint())
	{
		if ((navPoint->bPlayerOnly() && !bIsPlayer()) || (navPoint->bPlayerOnly() && !bIsPlayer()))
			continue; // Skip nav nodes only for the player if we aren't one

		float maxDist = 500;
		vec3 d = navPoint->Location() - location;
		float distsqr = dot(d, d);
		if (distsqr > maxDist * maxDist)
			continue; // Ignore things too far away

		navPoints.push_back({ navPoint, distsqr });
	}

	std::sort(navPoints.begin(), navPoints.end(), [](const auto& a, const auto& b) { return a.second < b.second; });

	size_t maxTraces = 4; // upper bound for how expensive this can get
	navPoints.resize(std::min(navPoints.size(), maxTraces));

	// Find the first reachable nav point
	for (auto& p : navPoints)
	{
		vec3 eyePos = p.first->Location();
		eyePos.z += BaseEyeHeight();
		if (FastTrace(location, eyePos))
			return p.first;
	}
	return nullptr;
}

UObject* UPawn::FindBestInventoryPath(bool predictRespawns, float& outMinWeight)
{
	LogUnimplemented("Pawn.FindBestInventoryPath");
	outMinWeight = 0.0f;
	return SetRouteCache({});
}

void UPawn::InitActorZone()
{
	UActor::InitActorZone();

	FootRegion() = FindRegion({ 0.0f, 0.0f, -CollisionHeight() });
	HeadRegion() = FindRegion({ 0.0f, 0.0f, EyeHeight() });

	if (engine->LaunchInfo.engineVersion > 219 && PlayerReplicationInfo())
		PlayerReplicationInfo()->PlayerZone() = Region().Zone;
}

void UPawn::UpdateActorZone()
{
	UActor::UpdateActorZone();

	PointRegion oldfootregion = FootRegion();
	PointRegion newfootregion = FindRegion({ 0.0f, 0.0f, -CollisionHeight() });
	if (oldfootregion.Zone && oldfootregion.Zone != newfootregion.Zone)
	{
		CallEvent(oldfootregion.Zone, EventName::FootZoneChange, { ExpressionValue::ObjectValue(this) });
		if (newfootregion.Zone && newfootregion.Zone->bPainZone())
		{
			// Pain zones, such as lava and slime, should immediately start hurting the pawn upon entering,
			// so set the pawn's PainTime to something quite low.
			// After that, they'll get DamagePerSec damage each second.
			PainTime() = 0.1f;
		}
	}

	FootRegion() = newfootregion;

	PointRegion oldheadregion = HeadRegion();
	PointRegion newheadregion = FindRegion({ 0.0f, 0.0f, EyeHeight() });
	if (oldheadregion.Zone && oldheadregion.Zone != newheadregion.Zone)
	{
		CallEvent(oldheadregion.Zone, EventName::HeadZoneChange, { ExpressionValue::ObjectValue(this) });

		if (newheadregion.Zone && newheadregion.Zone->bWaterZone() && !newheadregion.Zone->bPainZone())
		{
			// If the new zone is also a pain zone, like lava or slime, then by this point PainTime is already set,
			// so don't set it again. Otherwise, cause the pawn to start drowning in UnderWaterTime seconds.
			PainTime() = UnderWaterTime();
		}
	}

	HeadRegion() = newheadregion;

	if (engine->LaunchInfo.engineVersion > 219 && PlayerReplicationInfo())
		PlayerReplicationInfo()->PlayerZone() = Region().Zone;
}

void UPawn::Tick(float elapsed)
{
	MoveTimer() -= elapsed;

	if (StateFrame)
	{
		if (StateFrame->LatentState == LatentRunState::MoveTo)
		{
			TickRotateTo(Focus());
			if (TickMoveTo(Destination()))
				StateFrame->LatentState = LatentRunState::Continue;
		}
		else if (StateFrame->LatentState == LatentRunState::MoveToward)
		{
			if (MoveTarget())
			{
				Focus() = MoveTarget()->Location();
				TickRotateTo(Focus());
				if (TickMoveTo(MoveTarget()->Location()))
					StateFrame->LatentState = LatentRunState::Continue;
			}
			else
			{
				StateFrame->LatentState = LatentRunState::Continue;
			}
		}
		else if (StateFrame->LatentState == LatentRunState::StrafeTo)
		{
			TickRotateTo(Focus());
			if (TickMoveTo(Destination()))
				StateFrame->LatentState = LatentRunState::Continue;
		}
		else if (StateFrame->LatentState == LatentRunState::StrafeFacing)
		{
			if (engine->LaunchInfo.engineVersion > 219 && FaceTarget())
			{
				TickRotateTo(Focus());
				vec3 oldDest = Destination();
				if (TickMoveTo(Destination()))
					StateFrame->LatentState = LatentRunState::Continue;
				Destination() = oldDest;
			}
			else
			{
				StateFrame->LatentState = LatentRunState::Continue;
			}
		}
		else if (StateFrame->LatentState == LatentRunState::TurnTo)
		{
			if (TickRotateTo(Focus()))
				StateFrame->LatentState = LatentRunState::Continue;
		}
		else if (StateFrame->LatentState == LatentRunState::TurnToward)
		{
			if (engine->LaunchInfo.engineVersion > 219 && FaceTarget())
			{
				if (TickRotateTo(FaceTarget()->Location()))
					StateFrame->LatentState = LatentRunState::Continue;
			}
			else
			{
				StateFrame->LatentState = LatentRunState::Continue;
			}
		}
		else if (StateFrame->LatentState == LatentRunState::WaitForLanding)
		{
			if (Physics() != PHYS_Falling)
			{
				StateFrame->LatentState = LatentRunState::Continue;
			}
			else
			{
				// To do: need to send a LongFall event if the fall state lasts long enough
			}
		}
	}

	UActor::Tick(elapsed);

	if (bIsPlayer() && Role() >= ROLE_AutonomousProxy)
	{
		if (engine->LaunchInfo.engineVersion < 400 || bViewTarget())
			CallEvent(this, EventName::UpdateEyeHeight, { ExpressionValue::FloatValue(elapsed) });
		else
			ViewRotation() = Rotation();
	}

	if (Weapon())
	{
		Weapon()->Location() = Location();
		Weapon()->UpdateActorZone();
	}

	if (Role() == ROLE_Authority)
	{
		if (PainTime() > 0.0f)
		{
			PainTime() = std::max(PainTime() - elapsed, 0.0f);
			if (PainTime() == 0.0f)
				CallEvent(this, EventName::PainTimer);
		}
		if (SpeechTime() > 0.0f)
		{
			SpeechTime() = std::max(SpeechTime() - elapsed, 0.0f);
			if (SpeechTime() == 0.0f)
				CallEvent(this, EventName::SpeechTimer);
		}
		if (engine->LaunchInfo.engineVersion >= 436 && bAdvancedTactics())
			CallEvent(this, EventName::UpdateTactics, { ExpressionValue::FloatValue(elapsed) });
	}
}

void UPawn::TickRotating(float elapsed)
{
	if (Physics() == PHYS_Spider)
		return;

	bRotateToDesired() = true;
	bFixedRotationDir() = false;

	if (Rotation() == DesiredRotation())
		return;

	Rotator rot = Rotation();

	if ((DesiredRotation().Yaw & 0xffff) != (rot.Yaw & 0xffff))
	{
		rot.Yaw = Rotator::TurnToShortest(rot.Yaw, DesiredRotation().Yaw, (int)std::abs(RotationRate().Yaw * elapsed));
	}

	if ((DesiredRotation().Pitch & 0xffff) != (rot.Pitch & 0xffff))
	{
		rot.Pitch = DesiredRotation().Pitch & 0xffff;
		if (rot.Pitch < 0x8000)
		{
			rot.Pitch = std::max(rot.Pitch, RotationRate().Pitch);
		}
		else if (rot.Pitch < 0x10000 - RotationRate().Pitch)
		{
			rot.Pitch = 0x10000 - RotationRate().Pitch;
		}
	}

	// To do: apply RotationRate().Roll

	Rotation() = rot;

	if (Rotation() == DesiredRotation())
	{
		CallEvent(this, EventName::EndedRotation);
	}
}

bool UPawn::TickRotateTo(const vec3& target)
{
	if (Physics() == PHYS_Spider)
		return true;

	DesiredRotation() = Rotator::FromVector(target - Location());

	if (Physics() == PHYS_Walking && (!MoveTarget() || !MoveTarget()->IsA("Pawn")))
	{
		DesiredRotation().Pitch = 0;
	}

	int doneAngle = 2000;
	return (std::abs(DesiredRotation().Yaw - (Rotation().Yaw & 0xffff)) < doneAngle) || (std::abs(DesiredRotation().Yaw - (Rotation().Yaw & 0xffff)) > 0xffff - doneAngle);
}

bool UPawn::TickMoveTo(const vec3& target)
{
	if (MoveTimer() < 0.0f)
		return true;

	if (Physics() == PHYS_Walking)
	{
		vec2 delta = target.xy() - Location().xy();
		float distSqr = dot(delta, delta);
		float velocitySqr = dot(Velocity(), Velocity());
		if (distSqr < 1.0f || distSqr < velocitySqr * 0.05f)
			return true;

		Acceleration() = vec3(normalize(delta) * AccelRate(), 0.0f);
	}
	else
	{
		vec3 delta = target - Location();
		float distSqr = dot(delta, delta);
		float velocitySqr = dot(Velocity(), Velocity());
		if (distSqr < 1.0f || distSqr < velocitySqr * 0.05f)
			return true;

		Acceleration() = normalize(delta) * AccelRate();
	}

	return false;
}

void UPawn::MoveTo(const vec3& newDestination, float speed)
{
	MoveTarget() = nullptr;
	bReducedSpeed() = false;
	DesiredSpeed() = clamp(speed, 0.0f, MaxDesiredSpeed());
	Destination() = newDestination;
	Focus() = newDestination;
	SetMoveDuration(newDestination - Location());
	if (StateFrame)
		StateFrame->LatentState = LatentRunState::MoveTo;
}

void UPawn::MoveToward(UActor* newTarget, float speed)
{
	if (!newTarget)
		return;

	MoveTarget() = newTarget;
	Destination() = newTarget->Location();
	Focus() = newTarget->Location();
	bReducedSpeed() = false;
	DesiredSpeed() = clamp(speed, 0.0f, MaxDesiredSpeed());
	if (UObject::TryCast<UPawn>(newTarget))
		MoveTimer() = 1.0f;
	else
		SetMoveDuration(newTarget->Location() - Location());
	if (StateFrame)
		StateFrame->LatentState = LatentRunState::MoveToward;
}

void UPawn::StrafeFacing(const vec3& newDestination, UActor* newTarget)
{
	if (!newTarget)
		return;

	Destination() = newDestination;
	if (engine->LaunchInfo.engineVersion > 219)
		FaceTarget() = newTarget;
	SetMoveDuration(newDestination - Location());
	if (StateFrame)
		StateFrame->LatentState = LatentRunState::StrafeFacing;
}

void UPawn::StrafeTo(const vec3& newDestination, const vec3& newFocus)
{
	MoveTarget() = nullptr;
	bReducedSpeed() = false;
	DesiredSpeed() = bIsPlayer() ? MaxDesiredSpeed() : 0.8f * MaxDesiredSpeed();
	Destination() = newDestination;
	Focus() = newFocus;
	SetMoveDuration(newDestination - Location());
	if (StateFrame)
		StateFrame->LatentState = LatentRunState::StrafeTo;
}

void UPawn::TurnTo(const vec3& newFocus)
{
	MoveTarget() = nullptr;
	Focus() = newFocus;
	if (StateFrame)
		StateFrame->LatentState = LatentRunState::TurnTo;
}

void UPawn::TurnToward(UActor* newTarget)
{
	if (!newTarget)
		return;

	if (engine->LaunchInfo.engineVersion > 219)
		FaceTarget() = newTarget;
	Focus() = newTarget->Location();
	if (StateFrame)
		StateFrame->LatentState = LatentRunState::TurnToward;
}

void UPawn::WaitForLanding()
{
	if (Physics() == PHYS_Falling && StateFrame)
		StateFrame->LatentState = LatentRunState::WaitForLanding;
}

void UPawn::SetMoveDuration(const vec3& deltaMove)
{
	float scale = DesiredSpeed() * GetSpeed();
	MoveTimer() = scale > 0.0f ? 1.0f + 1.3f * length(deltaMove) / scale : 0.5f;
}

float UPawn::GetSpeed()
{
	switch (Physics())
	{
	case PHYS_Walking:
	case PHYS_Falling:
	case PHYS_Spider:
		return GroundSpeed();
	case PHYS_Flying:
		return AirSpeed();
	case PHYS_Swimming:
		return WaterSpeed();
	default:
		return 0.0f;
	}
}


/////////////////////////////////////////////////////////////////////////////

void UPlayerPawn::PausedInput(float elapsed)
{
	if (Role() >= ROLE_SimulatedProxy && Player() && !UObject::TryCast<UCamera>(this))
	{
		CallEvent(this, EventName::PlayerInput, { ExpressionValue::FloatValue(elapsed) });
	}
}

void UPlayerPawn::Tick(float elapsed)
{
	UPawn::Tick(elapsed);

	if (Role() >= ROLE_SimulatedProxy)
	{
		if (Player() && !UObject::TryCast<UCamera>(this))
		{
			CallEvent(this, EventName::PlayerInput, { ExpressionValue::FloatValue(elapsed) });
			CallEvent(this, EventName::PlayerTick, { ExpressionValue::FloatValue(elapsed) });
		}
	}

	// TODO: is this the correct place to set this?
	aForward() = 0.0f;

	// XXX: we reset this here to prevent infinite runaway, which eventually breaks mouselook
	// however, this might break looking with the controller?
	aTurn() = 0.0f;
	aLookUp() = 0.0f;
}

void UPlayerPawn::TickRotating(float elapsed)
{
	if (Physics() == PHYS_Spider)
		return;

	Rotator rot = Rotation();

	// To do: apply RotationRate().Roll

	Rotation() = rot;
}

void UPlayerPawn::LoadProperties()
{
	bInvertMouse() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bInvertMouse", true);
	MouseSensitivity() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "MouseSensitivity", 5.0f);
	// TODO: Handle the array property this class has (WeaponPriority)
	DodgeClickTime() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "DodgeClickTime", 0.25f);
	Bob() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "Bob", 0.016f);
	MyAutoAim() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "MyAutoAim", 1.0f);
	Handedness() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "Handedness", -1.0f);
	bLookUpStairs() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bLookUpStairs", false);
	bSnapToLevel() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bSnapToLevel", false);
	bAlwaysMouseLook() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bAlwaysMouseLook", true);
	bKeyboardLook() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bKeyboardLook", false);
	if (engine->LaunchInfo.engineVersion > 219)
	{
		bMaxMouseSmoothing() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bMaxMouseSmoothing", false);
		bNoFlash() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bNoFlash", false);
		bNoVoices() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bNoVoices", false);
		bMessageBeep() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bMessageBeep", true);
		// NetSpeed is missing
		// LanSpeed is missing
		MouseSmoothThreshold() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "MouseSmoothThreshold", 0.16f);
		ngWorldSecret() = IniPropertyConverter<std::string>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "ngWorldSecret", "");

		// SE addition: Store/Load the DefaultFOV setting into/from the user.ini file as well
		DefaultFOV() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "MainFOV", 90.0f);
	}
}

void UPlayerPawn::SaveConfig()
{
	UPawn::SaveConfig();

	// Note: the code below may no longer be needed. SaveConfig() on UObject saves all unrealscript variables marked as config or globalconfig

	// Not sure why are PlayerPawn's config fields not saved with the base SaveConfig(), but whatever. 
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bInvertMouse", IniPropertyConverter<bool>::ToString(bInvertMouse()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "MouseSensitivity", IniPropertyConverter<float>::ToString(MouseSensitivity()));
	// TODO: Handle the array property this class has (WeaponPriority)
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "DodgeClickTime", IniPropertyConverter<float>::ToString(DodgeClickTime()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "Bob", IniPropertyConverter<float>::ToString(Bob()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "MyAutoAim", IniPropertyConverter<float>::ToString(MyAutoAim()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "Handedness", IniPropertyConverter<float>::ToString(Handedness()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bLookUpStairs", IniPropertyConverter<bool>::ToString(bLookUpStairs()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bSnapToLevel", IniPropertyConverter<bool>::ToString(bSnapToLevel()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bAlwaysMouseLook", IniPropertyConverter<bool>::ToString(bAlwaysMouseLook()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bKeyboardLook", IniPropertyConverter<bool>::ToString(bKeyboardLook()));
	if (engine->LaunchInfo.engineVersion > 219)
	{
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bMaxMouseSmoothing", IniPropertyConverter<bool>::ToString(bMaxMouseSmoothing()));
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bNoFlash", IniPropertyConverter<bool>::ToString(bNoFlash()));
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bNoVoices", IniPropertyConverter<bool>::ToString(bNoVoices()));
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bMessageBeep", IniPropertyConverter<bool>::ToString(bMessageBeep()));
		// NetSpeed is missing
		// LanSpeed is missing
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "MouseSmoothThreshold", IniPropertyConverter<float>::ToString(MouseSmoothThreshold()));
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "ngWorldSecret", ngWorldSecret());

		// SE addition: Store/Load the DefaultFOV setting into/from the user.ini file as well
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "MainFOV", IniPropertyConverter<float>::ToString(DefaultFOV()));
	}
}

/////////////////////////////////////////////////////////////////////////////

void ULevelInfo::UpdateActorZone()
{
	// No zone events are sent by LevelInfo actors
	Region() = FindRegion();
}

/////////////////////////////////////////////////////////////////////////////

UObject* UDecal::AttachDecal(float traceDistance, vec3 decalDir)
{
	if (!Texture())
		return nullptr;

	vec3 traceDirection = -Coords::Rotation(Rotation()).XAxis;

	CollisionHitList hits = XLevel()->Collision.TraceDecal(to_dvec3(Location()), 0.0f, to_dvec3(traceDirection), traceDistance, false);
	if (hits.empty()) return nullptr;

	UModel* model = XLevel()->Model;

	// Do not attempt to create a decal if we hit a surface that's invisible or a fake backdrop
	auto& hit = hits.front();
	if (!hit.Node || (model->Surfaces[hit.Node->Surf].PolyFlags & (PF_FakeBackdrop | PF_Invisible)) != 0)
		return nullptr;

	vec3 N = hit.Normal;
	vec3 pos = Location() + traceDirection * hit.Fraction;

	if (dot(decalDir, decalDir) < 0.01f) // decalDir specifies which direction the decal texture faces. If its zero use a random direction
	{
		vec3 randomDir;
		while (true)
		{
			randomDir = vec3((float)(std::rand() / (double)RAND_MAX), (float)(std::rand() / (double)RAND_MAX), (float)(std::rand() / (double)RAND_MAX)) * 2.0f - 1.0f;
			if (dot(randomDir, randomDir) >= 1.0f)
				break;
		}
		decalDir = randomDir;
	}

	vec3 ydir = -(decalDir - dot(decalDir, N) * N);
	if (dot(ydir, ydir) < 0.01f)
		ydir = normalize(cross(N, std::abs(N.x) > std::abs(N.y) ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f)));
	else
		ydir = normalize(ydir);
	vec3 xdir = cross(N, ydir);

	float usize = (float)Texture()->USize();
	float vsize = (float)Texture()->VSize();
	xdir *= DrawScale() * usize * 0.5f;
	ydir *= DrawScale() * vsize * 0.5f;

	static Array<vec3> positions;
	static Array<vec2> uvs;
	static Array<float> edgeDistances;

	// Walk all nodes in the same plane
	BspNode* polynode = hit.NodeHead;
	while (true)
	{
		positions.clear();
		uvs.clear();

		// Place decal on the surface plane
		positions.push_back(pos - xdir - ydir);
		positions.push_back(pos + xdir - ydir);
		positions.push_back(pos + xdir + ydir);
		positions.push_back(pos - xdir + ydir);
		uvs.push_back({ 0.0f, 0.0f });
		uvs.push_back({ usize, 0.0f });
		uvs.push_back({ usize, vsize });
		uvs.push_back({ 0.0f, vsize });

		// Clip to BSP node shape
		int vertCount = (int)positions.size();
		BspVert* v = &model->Vertices[polynode->VertPool];
		for (int j = 0; j < polynode->NumVertices; j++)
		{
			const vec3& edgeStart = model->Points[v[j > 0 ? j - 1 : polynode->NumVertices - 1].Vertex];
			const vec3& edgeEnd = model->Points[v[j].Vertex];
			vec3 planeN = cross(N, edgeEnd - edgeStart); // Note: not normalized as we don't need it
			vec4 plane(planeN, -dot(edgeEnd, planeN));

			// Find vertex distances to edge plane
			edgeDistances.clear();
			for (int i = 0; i < vertCount; i++)
				edgeDistances.push_back(dot(plane, vec4(positions[i], 1.0f)));

			// Insert points at the edge for any line crossing the plane
			for (int i = 0; i < vertCount; i++)
			{
				float dist = edgeDistances[i];
				float distNext = edgeDistances[(i + 1) % vertCount];
				if ((dist > 0.0f && distNext < 0.0f) || (distNext > 0.0f && dist < 0.0f))
				{
					vec3 p = positions[i];
					vec3 pNext = positions[(i + 1) % vertCount];
					vec2 uv = uvs[i];
					vec2 uvNext = uvs[(i + 1) % vertCount];

					// Ray/plane intersection
					float t = -dot(vec4(p, 1.0f), plane) / dot(plane.xyz(), pNext - p);
					vec3 pInsert = mix(p, pNext, t);
					vec2 uvInsert = mix(uv, uvNext, t);

					int insertAt = i + 1;
					positions.insert(positions.begin() + insertAt, pInsert);
					uvs.insert(uvs.begin() + insertAt, uvInsert);
					edgeDistances.insert(edgeDistances.begin() + insertAt, 0.0f);
					vertCount++;
				}
			}

			// Remove points outside
			int i = 0;
			while (i < vertCount)
			{
				if (edgeDistances[i] < 0.0f)
				{
					positions.erase(positions.begin() + i);
					uvs.erase(uvs.begin() + i);
					edgeDistances.erase(edgeDistances.begin() + i);
					vertCount--;
				}
				else
				{
					i++;
				}
			}
		}

		// Add to decals list if we still got anything left to render
		if (!positions.empty())
		{
			LevelDecal leveldecal;
			leveldecal.Decal = this;
			leveldecal.Positions = positions;
			leveldecal.UVs = uvs;
			polynode->Decals.push_back(leveldecal);
			Nodes.push_back(polynode);
		}

		if (polynode->Plane < 0) break;
		polynode = &model->Nodes[polynode->Plane];
	}

	return Level();
}

void UDecal::DetachDecal()
{
	for (BspNode* node : Nodes)
	{
		auto& decals = node->Decals;
		auto it = decals.begin();
		while (it != decals.end())
		{
			auto& leveldecal = *it;
			if (leveldecal.Decal == this)
				it = decals.erase(it);
			else
				++it;
		}
	}
	Nodes.clear();
}

/////////////////////////////////////////////////////////////////////////////

void UWarpZoneInfo::Warp(vec3& Loc, vec3& Vel, Rotator& R)
{
	auto newLoc = Loc + WarpCoords().Origin;
	Loc = (WarpCoords() * Coords::Rotation(R)).GlobalizeVector(newLoc);
	Vel = (WarpCoords() * Coords::Rotation(R)).GlobalizeVector(Vel);
	// Transform to local space
	// Grab the relative position
	// Use that in the inverse calc to transform back out
}

void UWarpZoneInfo::UnWarp(vec3& Loc, vec3& Vel, Rotator& R)
{
	auto newLoc = Loc - WarpCoords().Origin;
	Loc = (WarpCoords() * Coords::InverseRotation(R)).LocalizeVector(newLoc);
	Vel = (WarpCoords() * Coords::InverseRotation(R)).LocalizeVector(Vel);
}

/////////////////////////////////////////////////////////////////////////////

void UPakPathNodeIterator::BuildPath(vec3& start, vec3& end)
{
	LogUnimplemented("PathNodeIterator.BuildPath()");
	NodeIndex() = 0;
}

void UPakPathNodeIterator::CheckUPak()
{
	// What does this even check?
}

UNavigationPoint* UPakPathNodeIterator::GetFirst()
{
	LogUnimplemented("PathNodeIterator.GetFirst()");
	//return NodePath().front();
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetPrevious()
{
	LogUnimplemented("PathNodeIterator.GetPrevious()");
	// if (NodeIndex() > 0)
	// 	NodeIndex()--;
	// return NodePath()[NodeIndex()];
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetCurrent()
{
	LogUnimplemented("PathNodeIterator.GetCurrent()");
	//return NodePath()[NodeIndex()];
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetNext()
{
	LogUnimplemented("PathNodeIterator.GetNext()");
	// if (NodeIndex() < NodeCount() - 1)
	// 	NodeIndex()++;
	// return NodePath()[NodeIndex()];
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetLast()
{
	LogUnimplemented("PathNodeIterator.GetLast()");
	//return NodePath().back();
	return nullptr;
}

UNavigationPoint* UPakPathNodeIterator::GetLastVisible()
{
	LogUnimplemented("PathNodeIterator.GetLastVisible()");
	return nullptr;
}

void UPakPawnPathNodeIterator::SetPawn(UPawn* P)
{
	Pawn() = P;
}
