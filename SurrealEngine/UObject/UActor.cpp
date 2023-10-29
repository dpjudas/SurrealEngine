
#include "Precomp.h"
#include "UActor.h"
#include "ULevel.h"
#include "UMesh.h"
#include "UTexture.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "Package/PackageManager.h"
#include "Engine.h"
#include "Collision/OverlapCylinderLevel.h"

static std::string tickEventName = "Tick";

UActor* UActor::Spawn(UClass* SpawnClass, UActor* SpawnOwner, NameString SpawnTag, vec3* SpawnLocation, Rotator* SpawnRotation)
{
	// To do: return null if spawn location isn't valid

	if (!SpawnClass) // To do: return null if spawn class is abstract
	{
		return nullptr;
	}

	vec3 location = SpawnLocation ? *SpawnLocation : Location();
	Rotator rotation = SpawnRotation ? *SpawnRotation : Rotation();

	float radius = SpawnClass->GetDefaultObject()->GetFloat("CollisionRadius");
	float height = SpawnClass->GetDefaultObject()->GetFloat("CollisionHeight");
	bool bCollideWorld = SpawnClass->GetDefaultObject()->GetBool("bCollideWorld");
	bool bCollideWhenPlacing = SpawnClass->GetDefaultObject()->GetBool("bCollideWhenPlacing");
	if (bCollideWorld || bCollideWhenPlacing)
	{
		auto result = CheckLocation(location, radius, height, bCollideWorld || bCollideWhenPlacing);
		if (!result.first)
			return nullptr;
		location = result.second;
	}

	// To do: package needs to be grabbed from outer, or the "transient package" if it is None, a virtual package for runtime objects
	UActor* actor = UObject::Cast<UActor>(engine->packages->GetPackage("Engine")->NewObject("", UObject::Cast<UClass>(SpawnClass), ObjectFlags::Transient, true));

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

	XLevel()->Actors.push_back(actor);
	XLevel()->Hash.AddToCollision(actor);

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
		OverlapCylinderLevel overlap;
		CollisionHitList hits = overlap.TestOverlap(XLevel(), Location(), CollisionHeight(), CollisionRadius(), true, false, false);
		if (!hits.empty())
		{
			SetBase(hits.front().Actor, true);
		}
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
	float scale = std::max(radius * 0.5f, height * 0.5f);
	for (int z = 0; z < 3 && !found; z++)
	{
		for (int y = 0; y < 3 && !found; y++)
		{
			for (int x = 0; x < 3 && !found; x++)
			{
				vec3 testlocation = location + vec3(offset[x] * scale, offset[y] * scale, offset[z] * scale);
				OverlapCylinderLevel overlap;
				CollisionHitList hits = overlap.TestOverlap(XLevel(), testlocation, height, radius, false, true, false);
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
	level->Hash.RemoveFromCollision(this);

	CallEvent(this, EventName::Destroyed);

	UActor** TouchingArray = Touching();
	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray[i])
			UnTouch(TouchingArray[i]);
	}

	SetOwner(nullptr);

	for (size_t i = 0; i < level->Actors.size(); i++)
	{
		UActor* actor = level->Actors[i];
		if (actor && actor->Owner() == this)
		{
			actor->SetOwner(nullptr);
		}
		if (actor == this)
		{
			level->Actors[i] = nullptr;
		}
	}

	return true;
}

PointRegion UActor::FindRegion(const vec3& offset)
{
	PointRegion region;
	region.BspLeaf = 0;
	region.ZoneNumber = 0;

	vec4 location = vec4(Location() + offset, 1.0f);

	// Search the BSP
	BspNode* nodes = XLevel()->Model->Nodes.data();
	BspNode* node = nodes;
	while (true)
	{
		vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
		float side = dot(location, plane);
		if (node->Front >= 0 && side >= 0.0f)
		{
			node = nodes + node->Front;
		}
		else if (node->Back >= 0 && side <= 0.0f)
		{
			node = nodes + node->Back;
		}
		else
		{
			region.ZoneNumber = node->Zone1;
			region.BspLeaf = side >= 0.0f ? node->Leaf0 : node->Leaf1;
			break;
		}
	}

	region.Zone = UObject::Cast<UZoneInfo>(XLevel()->Model->Zones[region.ZoneNumber].ZoneActor);
	if (!region.Zone)
		region.Zone = Level();

	return region;
}

void UActor::InitActorZone()
{
	Region() = FindRegion();
	if (Region().Zone->bWaterZone())
	{
		SetPhysics(PHYS_Swimming);
	}
}

void UActor::UpdateActorZone()
{
	PointRegion oldregion = Region();
	PointRegion newregion = FindRegion();

	if (Region().Zone && oldregion.Zone != newregion.Zone)
		CallEvent(oldregion.Zone, EventName::ActorLeaving, { ExpressionValue::ObjectValue(this) });

	Region() = newregion;

	if (Region().Zone && oldregion.Zone != newregion.Zone)
	{
		CallEvent(this, EventName::ZoneChange, { ExpressionValue::ObjectValue(newregion.Zone) });
		CallEvent(newregion.Zone, EventName::ActorEntered, { ExpressionValue::ObjectValue(this) });
	}
}

void UActor::SetOwner(UActor* newOwner)
{
	if (Owner())
		CallEvent(Owner(), EventName::LostChild, { ExpressionValue::ObjectValue(this) });

	Owner() = newOwner;

	if (Owner())
		CallEvent(Owner(), EventName::GainedChild, { ExpressionValue::ObjectValue(this) });
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

void UActor::Tick(float elapsed, bool tickedFlag)
{
	bTicked() = tickedFlag;

	TickAnimation(elapsed);

	if (Role() >= ROLE_SimulatedProxy && IsEventEnabled(EventName::Tick))
	{
		CallEvent(this, EventName::Tick, { ExpressionValue::FloatValue(elapsed) });
	}

	if (StateFrame && StateFrame->LatentState == LatentRunState::Sleep)
	{
		SleepTimeLeft = std::max(SleepTimeLeft - elapsed, 0.0f);
		if (SleepTimeLeft == 0.0f)
			StateFrame->LatentState = LatentRunState::Continue;
	}

	if (Role() >= ROLE_SimulatedProxy && StateFrame && StateFrame->LatentState == LatentRunState::Continue)
	{
		StateFrame->Tick();
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
		}

		TickRotating(physTimeElapsed);

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
	vec3 stepDownDelta(0.0f, 0.0f, gravityDirection * pawn->MaxStepHeight() * 2.0f);

	// "Step up and move" as long as we have time left and only hitting surfaces with low enough slope that it could be walked
	float timeLeft = elapsed;
	vec3 vel = Velocity();
	if (vel.x != 0.0f && vel.y != 0.0f)
	{
		for (int iteration = 0; timeLeft > 0.0f && iteration < 5; iteration++)
		{
			vec3 moveDelta = vel * timeLeft;

			TryMove(stepUpDelta);
			CollisionHit hit = TryMove(moveDelta);
			timeLeft -= timeLeft * hit.Fraction;

			if (hit.Fraction < 1.0f)
			{
				if (player && UObject::TryCast<UDecoration>(hit.Actor) && static_cast<UDecoration*>(hit.Actor)->bPushable() && dot(hit.Normal, moveDelta) < -0.9f)
				{
					// We hit a pushable decoration that is facing our movement direction

					bJustTeleported() = true;
					vel = Velocity() = Velocity() * Mass() / (Mass() + hit.Actor->Mass());
					CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
					timeLeft = 0.0f;
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

					timeLeft = 0.0f;
				}
			}

			// Check if unrealscript got us out of walking mode
			if (Physics() != PHYS_Walking)
				return;
		}

		// Step down after movement to see if we are still walking or if we are now falling
		CollisionHit hit = TryMove(stepDownDelta);
		if (Physics() == PHYS_Walking && (hit.Fraction == 1.0f || dot(hit.Normal, vec3(0.0f, 0.0f, 1.0f)) < 0.7071f))
		{
			SetPhysics(PHYS_Falling);
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

	if (pawn)
	{
		float maxAccel = pawn->AirControl() * pawn->AccelRate();
		float accel = length(Acceleration());
		if (accel > maxAccel)
			Acceleration() = normalize(Acceleration()) * maxAccel;
	}

	float gravityScale = 2.0f;
	float fluidFriction = 0.0f;

	if (decor && decor->bBobbing())
	{
		gravityScale = 1.0f;
	}
	else if (pawn && pawn->FootRegion().Zone->bWaterZone() && Velocity().z < 0.0f)
	{
		fluidFriction = pawn->FootRegion().Zone->ZoneFluidFriction();
	}

	OldLocation() = Location();
	bJustTeleported() = false;

	gravityVector = zone->ZoneGravity();
	double gravityMag = length(gravityVector);
	vec3 oldVelocity = Velocity();
	vec3 newVelocity = Velocity() * (1.0f - fluidFriction * elapsed) + ((Acceleration() * 2.0f) + (gravityScale * gravityVector)) * 0.5f * elapsed;

	// Limit air control to controlling which direction we are moving in the XY plane, but not increase the speed beyond the ground speed
	float curSpeedSquared = dot(Velocity().xy(), Velocity().xy());
	if (pawn && curSpeedSquared >= (pawn->GroundSpeed() * pawn->GroundSpeed()) && dot(newVelocity.xy(), newVelocity.xy()) > curSpeedSquared)
	{
		float xySpeed = length(Velocity().xy());
		newVelocity = vec3(normalize(newVelocity.xy()) * xySpeed, newVelocity.z);
	}

	Velocity() = newVelocity;

	float zoneTerminalVelocity = zone->ZoneTerminalVelocity();
	if (dot(Velocity(), Velocity()) > zoneTerminalVelocity * zoneTerminalVelocity)
	{
		Velocity() = normalize(Velocity()) * zoneTerminalVelocity;
	}

	CollisionHit hit = TryMove((Velocity() + zone->ZoneVelocity()) * elapsed);
	if (hit.Fraction < 1.0f)
	{
		if (bBounce())
		{
			CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
			// TODO: perform bounce
		}
		else
		{
			CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });

			// slide along surfaces sloped steeper than 45 degrees
			if (hit.Normal.z < 0.7071f && hit.Normal.z >= 0.0f)
			{
				Rotator rot = Rotator::FromVector(hit.Normal);
				vec3 at, left, up;
				Coords::Rotation(rot).GetAxes(at, left, up);
				float z = newVelocity.z;

				gravityVector.x = at.x;
				gravityVector.y = -at.z;
				gravityVector.z = at.y * 0.5f;
				gravityVector *= (float)(gravityMag * 0.5f);

				newVelocity = oldVelocity * (1.0f - fluidFriction * elapsed) + ((Acceleration() * 0.3f) + gravityScale * gravityVector) * 0.75f * elapsed;
				if (hit.Normal.z == 0.0f)
					newVelocity.z = z;

				// Limit air control to controlling which direction we are moving in the XY plane, but not increase the speed beyond the ground speed
				curSpeedSquared = dot(Velocity().xy(), Velocity().xy());
				if (pawn && curSpeedSquared >= (pawn->GroundSpeed() * pawn->GroundSpeed()) && dot(newVelocity.xy(), newVelocity.xy()) > curSpeedSquared)
				{
					float xySpeed = length(Velocity().xy());
					newVelocity = vec3(normalize(newVelocity.xy()) * xySpeed, newVelocity.z);
				}

				Velocity() = newVelocity;
				MoveSmooth(newVelocity * elapsed);
			}
			else
			{
				CallEvent(this, EventName::Landed, { ExpressionValue::VectorValue(hit.Normal) });

				if (Physics() == PHYS_Falling) // Landed event might have changed the physics mode
				{
					if (UObject::TryCast<UPawn>(this))
					{
						SetPhysics(PHYS_Walking);
					}
					else
					{
						SetPhysics(PHYS_None);
						Velocity() = vec3(0.0f);
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

	// The classic step up, move and step down algorithm:

	float gravityDirection = zone->ZoneGravity().z > 0.0f ? 1.0f : -1.0f;

	// "Step up and move" as long as we have time left and only hitting surfaces with low enough slope that it could be walked
	float timeLeft = elapsed;
	for (int iteration = 0; timeLeft > 0.0f && iteration < 5; iteration++)
	{
		vec3 moveDelta = Velocity() * timeLeft;

		CollisionHit hit = TryMove(moveDelta);
		timeLeft -= timeLeft * hit.Fraction;

		if (hit.Fraction < 1.0f)
		{
			if (player && UObject::TryCast<UDecoration>(hit.Actor) && static_cast<UDecoration*>(hit.Actor)->bPushable() && dot(hit.Normal, moveDelta) < -0.9f)
			{
				// We hit a pushable decoration that is facing our movement direction

				bJustTeleported() = true;
				Velocity() = Velocity() * Mass() / (Mass() + hit.Actor->Mass());
				CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
				timeLeft = 0.0f;
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

				timeLeft = 0.0f;
			}
		}
	}

	if (!bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;
}

void UActor::TickFlying(float elapsed)
{
	CollisionHit hit = TryMove(Velocity() * elapsed);
	if (hit.Fraction < 1.0f)
	{
		// is this correct?
		if (bBounce())
		{
			CallEvent(this, EventName::HitWall, {ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level())});
		}
	}
}

void UActor::TickRotating(float elapsed)
{
	if (Physics() == PHYS_Rotating)
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

		if (dynamic_cast<UPlayerPawn*>(this))
		{
			UPlayerPawn* pawn = static_cast<UPlayerPawn*>(this);
			pawn->DesiredFlashScale() = mix(target->ScreenFlashScale(), next->ScreenFlashScale(), physAlpha);
			pawn->DesiredFlashFog() = mix(target->ScreenFlashFog(), next->ScreenFlashFog(), physAlpha);
			pawn->FovAngle() = mix(target->FovModifier(), next->FovModifier(), physAlpha) * Class->GetDefaultObject()->GetFloat("FovAngle");
			pawn->FlashScale() = vec3(pawn->DesiredFlashScale());
			pawn->FlashFog() = pawn->DesiredFlashFog();
		}

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

		UInterpolationPoint* prev = target->Position() > 0 ? target->Prev() : target;
		UInterpolationPoint* nextnext = next->Next();
		if (!nextnext) nextnext = next;
		if (next->bEndOfPath()) nextnext = next;
		if (target->bEndOfPath()) { next = target; nextnext = target; }
		vec3 location = spline(prev->Location(), target->Location(), next->Location(), nextnext->Location(), physAlpha);
		Rotator rotation = spline(prev->Rotation(), target->Rotation(), next->Rotation(), nextnext->Rotation(), physAlpha);

		PhysAlpha() = physAlpha;
		TryMove(location - Location());
		Rotation() = rotation;

		if (dynamic_cast<UPawn*>(this))
		{
			static_cast<UPawn*>(this)->ViewRotation() = Rotation();
		}

		if (interpolateStart)
		{
			CallEvent(target, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(this) });
			CallEvent(this, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(target) });

			target = target->Prev();
			while (target && target->bSkipNextPath())
				target = target->Prev();

			Target() = target;
			PhysAlpha() = 1.0f;
		}
		else if (interpolateEnd)
		{
			CallEvent(target, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(this) });
			CallEvent(this, EventName::InterpolateEnd, { ExpressionValue::ObjectValue(target) });

			target = target->Next();
			while (target && target->bSkipNextPath())
				target = target->Next();

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
			if (PhysRate() <= 0.0f || !bInterpolating())
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

			// engine->LogMessage("Moving brush: " + std::to_string(t) + " key=" + std::to_string(keyIndex) +" keypos=(" + std::to_string(keypos.x) + "," + std::to_string(keypos.y) + "," + std::to_string(keypos.z) + ")");

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

	if (bTrailerPrePivot())
	{
		newLocation += PrePivot();
	}

	SetLocation(newLocation);

	if (bTrailerSameRotation() && DrawType() != DT_Sprite)
	{
		SetRotation(Owner()->Rotation());
	}
}

void UActor::SetPhysics(uint8_t newPhysics)
{
	Physics() = newPhysics;
}

void UActor::SetCollision(bool newColActors, bool newBlockActors, bool newBlockPlayers)
{
	XLevel()->Hash.RemoveFromCollision(this);
	bCollideActors() = newColActors;
	bBlockActors() = newBlockActors;
	bBlockPlayers() = newBlockPlayers;
	XLevel()->Hash.AddToCollision(this);
}

bool UActor::SetLocation(const vec3& newLocation)
{
	auto result = CheckLocation(newLocation, CollisionRadius(), CollisionHeight(), bCollideWorld() || bCollideWhenPlacing());
	if (!result.first)
		return false;

	XLevel()->Hash.RemoveFromCollision(this);
	Location() = result.second;
	XLevel()->Hash.AddToCollision(this);

	if (Level()->bBegunPlay())
	{
		// Send touch notifications for anything at the new location
		for (UActor* actor : XLevel()->Hash.CollidingActors(Location(), CollisionHeight(), CollisionRadius()))
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

	XLevel()->Hash.RemoveFromCollision(this);
	CollisionRadius() = newRadius;
	CollisionHeight() = newHeight;
	XLevel()->Hash.AddToCollision(this);
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

	CollisionHit hit = XLevel()->TraceFirstHit(traceStart, traceEnd, this, extent, flags);
	hitNormal = hit.Normal;
	hitLocation = traceStart + (traceEnd - traceStart) * hit.Fraction;
	return hit.Actor;
}

bool UActor::FastTrace(const vec3& traceEnd, const vec3& traceStart)
{
	return !XLevel()->TraceRayAnyHit(traceStart, traceEnd, this, false, true, false);
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
	return CollisionHash::CylinderActorOverlap(to_dvec3(Location()), CollisionHeight(), CollisionRadius(), other);
}

CollisionHit UActor::TryMove(const vec3& delta)
{
	// Static and non-movable objects can't move
	if (bStatic() || !bMovable())
	{
		CollisionHit hit;
		hit.Fraction = 0.0f;
		return hit;
	}

	// Avoid moving if movement is too small as the physics code doesn't like very small numbers
	if (dot(delta, delta) < 0.0001f)
		return {};

	// Analyze what we will hit if we move as requested and stop if it is the level or a blocking actor
	bool useBlockPlayers = UObject::TryCast<UPlayerPawn>(this) || UObject::TryCast<UProjectile>(this);
	CollisionHit blockingHit;
	CollisionHitList hits = XLevel()->Trace(Location(), Location() + delta, CollisionHeight(), CollisionRadius(), bCollideActors(), bCollideWorld(), false);
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
				if (isBlocking && !hit.Actor->IsBasedOn(this) && !IsBasedOn(hit.Actor))
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

	vec3 actuallyMoved = delta * blockingHit.Fraction;
	vec3 OldLocation = Location();

	XLevel()->Hash.RemoveFromCollision(this);
	Location() += actuallyMoved;
	XLevel()->Hash.AddToCollision(this);

	// Based actors needs to move with us
	if (StandingCount() > 0)
	{
		ULevel* level = XLevel();
		for (size_t i = 0; i < level->Actors.size(); i++)
		{
			UActor* actor = level->Actors[i];
			if (actor && actor->ActorBase() == this)
			{
				actor->TryMove(actuallyMoved);
			}
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

void UActor::Touch(UActor* actor)
{
	UActor** TouchingArray = Touching();
	UActor** TouchingArray2 = actor->Touching();

	// Do nothing if actors are already touching
	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray[i] == actor)
			return;
	}

#if 0
	{
		UPawn* instigator0 = actor->Instigator();
		UPawn* instigator1 = Instigator();
		std::string name0 = actor->Class->FriendlyName.ToString();
		std::string name1 = Class->FriendlyName.ToString();
		std::string instname0 = instigator0 ? instigator0->Class->FriendlyName.ToString() : std::string("null");
		std::string instname1 = instigator1 ? instigator1->Class->FriendlyName.ToString() : std::string("null");
		std::string state0 = actor->GetStateName().ToString();
		engine->LogMessage("Touch detected between " + name0 + " (instigator " + instname0 + ", state " + state0 + ") and " + name1 + " (instigator " + instname1 + ")");
	}
#endif

	// Only attempt to touch actors if there's a free slot in both
	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (!TouchingArray[i])
		{
			for (int j = 0; j < TouchingArraySize; j++)
			{
				if (!TouchingArray2[j])
				{
					TouchingArray[i] = actor;
					CallEvent(this, EventName::Touch, { ExpressionValue::ObjectValue(actor) });

					if (!TouchingArray2[j])
					{
						TouchingArray2[j] = this;
						CallEvent(actor, EventName::Touch, { ExpressionValue::ObjectValue(this) });
					}
					return;
				}
			}
		}
	}
}

void UActor::UnTouch(UActor* actor)
{
	UActor** TouchingArray = Touching();
	UActor** TouchingArray2 = actor->Touching();

	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray[i] == actor)
		{
			TouchingArray[i] = nullptr;
			CallEvent(this, EventName::UnTouch, { ExpressionValue::ObjectValue(actor) });
		}
	}

	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray2[i] == this)
		{
			TouchingArray2[i] = nullptr;
			CallEvent(actor, EventName::UnTouch, { ExpressionValue::ObjectValue(this) });
		}
	}
}

bool UActor::Move(const vec3& delta)
{
	return TryMove(delta).Fraction == 1.0f;
}

bool UActor::MoveSmooth(const vec3& delta)
{
	CollisionHit hit = TryMove(delta);
	if (hit.Fraction != 1.0f)
	{
		// We hit a slope. Try to follow it.
		vec3 alignedDelta = (delta - hit.Normal * dot(delta, hit.Normal)) * (1.0f - hit.Fraction);
		if (dot(delta, alignedDelta) >= 0.0f) // Don't end up going backwards
		{
			CollisionHit hit2 = TryMove(alignedDelta);
		}
	}

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

void UActor::UpdateBspInfo()
{
	vec3 extents;
	if (LightBrightness() == 0)
	{
		extents = { VisibilityRadius(), VisibilityRadius(), VisibilityHeight() };
	}
	else
	{
		extents = { WorldLightRadius(), WorldLightRadius(), WorldLightRadius() };
	}

	// Is actor still in the bsp tree at the correct location?
	if (!BspInfo.Node || BspInfo.Location != Location() || BspInfo.Extents != extents)
	{
		RemoveFromBspNode();

		BspInfo.Location = Location();
		BspInfo.Extents = extents;

		ULevel* level = XLevel();
		BspNode* node = &level->Model->Nodes[0];
		while (node)
		{
			int side = NodeAABBOverlap(BspInfo.Location, BspInfo.Extents, node);
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

/////////////////////////////////////////////////////////////////////////////

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

	if (!bIsPlayer() || !Level()->Game()->bTeamGame() || !noisePawn->bIsPlayer() || !PlayerReplicationInfo() || !noisePawn->PlayerReplicationInfo() || (PlayerReplicationInfo()->Team() != noisePawn->PlayerReplicationInfo()->Team()))
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

	return !XLevel()->TraceRayAnyHit(source->Location(), Location(), source, false, true, false);
}

void UPawn::InitActorZone()
{
	UActor::InitActorZone();

	FootRegion() = FindRegion({ 0.0f, 0.0f, -CollisionHeight() });
	HeadRegion() = FindRegion({ 0.0f, 0.0f, EyeHeight() });

	if (PlayerReplicationInfo())
		PlayerReplicationInfo()->PlayerZone() = Region().Zone;
}

void UPawn::UpdateActorZone()
{
	UActor::UpdateActorZone();

	PointRegion oldfootregion = FootRegion();
	PointRegion newfootregion = FindRegion({ 0.0f, 0.0f, -CollisionHeight() });
	if (FootRegion().Zone && oldfootregion.Zone != newfootregion.Zone)
		CallEvent(FootRegion().Zone, EventName::FootZoneChange, { ExpressionValue::ObjectValue(this) });
	FootRegion() = newfootregion;

	PointRegion oldheadregion = HeadRegion();
	PointRegion newheadregion = FindRegion({ 0.0f, 0.0f, EyeHeight() });
	if (HeadRegion().Zone && oldheadregion.Zone != newheadregion.Zone)
		CallEvent(HeadRegion().Zone, EventName::HeadZoneChange, { ExpressionValue::ObjectValue(this) });
	HeadRegion() = newheadregion;

	if (PlayerReplicationInfo())
		PlayerReplicationInfo()->PlayerZone() = Region().Zone;
}

void UPawn::Tick(float elapsed, bool tickedFlag)
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
			if (FaceTarget())
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
			if (FaceTarget())
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

	UActor::Tick(elapsed, tickedFlag);

	if (bIsPlayer() && Role() >= ROLE_AutonomousProxy)
	{
		if (bViewTarget())
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
		if (bAdvancedTactics())
			CallEvent(this, EventName::UpdateTactics, { ExpressionValue::FloatValue(elapsed) });
	}
}

void UPawn::TickRotating(float elapsed)
{
	if (Physics() == PHYS_Spider)
		return;

	bRotateToDesired() = true;
	bFixedRotationDir() = false;

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
	// To do: there is probably more to it than this!

	Acceleration() = normalize(target - Location()) * AccelRate();

	if (Physics() == PHYS_Walking)
		Acceleration().z = 0.0f;

	return MoveTimer() < 0.0f || length(target - Location()) < length(Velocity()) * 0.02f;
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
	bReducedSpeed() = false;
	DesiredSpeed() = clamp(speed, 0.0f, MaxDesiredSpeed());
	SetMoveDuration(newTarget->Location() - Location());
	if (StateFrame)
		StateFrame->LatentState = LatentRunState::MoveToward;
}

void UPawn::StrafeFacing(const vec3& newDestination, UActor* newTarget)
{
	if (!newTarget)
		return;

	Destination() = newDestination;
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
	MoveTimer() = scale != 0.0f ? 1.0f + 1.3f * length(deltaMove) / scale : 0.5f;
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

void UPlayerPawn::Tick(float elapsed, bool tickedFlag)
{
	UPawn::Tick(elapsed, tickedFlag);

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
}

void UPlayerPawn::TickRotating(float elapsed)
{
	if (Physics() == PHYS_Spider)
		return;

	Rotator rot = Rotation();

	// To do: apply RotationRate().Roll

	Rotation() = rot;
}

/////////////////////////////////////////////////////////////////////////////

void ULevelInfo::UpdateActorZone()
{
	// No zone events are sent by LevelInfo actors
	Region() = FindRegion();
}

/////////////////////////////////////////////////////////////////////////////

UObject* UDecal::AttachDecal(float traceDistance, const vec3& decalDir)
{
	if (!Texture())
		return nullptr;

	vec3 dirNormalized;
	if (dot(decalDir, decalDir) < 0.01f)
	{
		dirNormalized = normalize(vec3(
			(float)(std::rand() / (double)RAND_MAX),
			(float)(std::rand() / (double)RAND_MAX),
			(float)(std::rand() / (double)RAND_MAX)));
	}
	else
	{
		dirNormalized = normalize(decalDir);
	}

	CollisionHitList hits = XLevel()->Model->TraceRay(to_dvec3(Location()), 0.1f, to_dvec3(dirNormalized), traceDistance, false);
	if (hits.empty()) return nullptr;

	vec3 N = hits.front().Normal;
	vec3 xdir = normalize(cross(N, std::abs(N.x) > std::abs(N.y) ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f)));
	vec3 ydir = cross(N, xdir);
	vec3 pos = Location() + dirNormalized * hits.front().Fraction + N;

	float usize = (float)Texture()->USize();
	float vsize = (float)Texture()->VSize();
	xdir *= DrawScale() * usize * 0.5f;
	ydir *= DrawScale() * vsize * 0.5f;

	// To do: clip this to model surfaces

	auto leveldecal = std::make_unique<LevelDecal>();
	leveldecal->Decal = this;
	leveldecal->Positions[0] = pos - xdir - ydir;
	leveldecal->Positions[1] = pos + xdir - ydir;
	leveldecal->Positions[2] = pos + xdir + ydir;
	leveldecal->Positions[3] = pos - xdir + ydir;
	leveldecal->UVs[0] = vec2(0.0f, 0.0f);
	leveldecal->UVs[1] = vec2(usize, 0.0f);
	leveldecal->UVs[2] = vec2(usize, vsize);
	leveldecal->UVs[3] = vec2(0.0f, vsize);
	XLevel()->Decals.push_back(std::move(leveldecal));

	return Level();
}

void UDecal::DetachDecal()
{
	auto& decals = XLevel()->Decals;
	auto it = decals.begin();
	while (it != decals.end())
	{
		auto& leveldecal = *it;
		if (leveldecal->Decal == this)
			it = decals.erase(it);
		else
			++it;
	}
}
