
#include "Precomp.h"
#include "UActor.h"
#include "ULevel.h"
#include "UMesh.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "Package/PackageManager.h"
#include "Engine.h"

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

	XLevel()->Actors.push_back(actor);
	XLevel()->AddToCollision(actor);

	actor->SetOwner(SpawnOwner ? SpawnOwner : this);
	actor->InitActorZone();

	if (Level()->bBegunPlay())
	{
		CallEvent(actor, "Spawned");
		CallEvent(actor, "PreBeginPlay");
		CallEvent(actor, "BeginPlay");

		if (actor->bDeleteMe())
			return nullptr;

		// To do: we need to call touch events here

		CallEvent(actor, "PostBeginPlay");
		CallEvent(actor, "SetInitialState");

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
						actor = UObject::Cast<UGameInfo>(CallEvent(notifyObj, "SpawnNotification", { ExpressionValue::ObjectValue(actor) }).ToObject());
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
#if 0
		UActor* hitActor = nullptr;
		auto onHit = [&](UActor* testActor) -> bool
		{
			for (UActor* cur = testActor; cur; cur = cur->Owner())
			{
				if (cur == hitActor)
					return true;
			}
			hitActor = testActor;
			return false;
		};
		engine->collision->TraceHit(actor->Location(), actor->Location() + vec3(0.0f, 0.0f, -10.0f), onHit);
		actor->SetBase(hitActor);
#else
		UActor* hitActor = Level();
		SetBase(hitActor, true);
#endif
	}
}

bool UActor::Destroy()
{
	if (bStatic() || bNoDelete())
		return false;
	if (bDeleteMe())
		return true;

	bDeleteMe() = true;

	GotoState({}, {});
	SetBase(nullptr, true);

	ULevel* level = XLevel();

	level->RemoveFromCollision(this);

	CallEvent(this, "Destroyed");

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
}

void UActor::UpdateActorZone()
{
	PointRegion oldregion = Region();
	PointRegion newregion = FindRegion();

	if (Region().Zone && oldregion.Zone != newregion.Zone)
		CallEvent(Region().Zone, "ActorLeaving", { ExpressionValue::ObjectValue(this) });

	Region() = newregion;

	if (Region().Zone && oldregion.Zone != newregion.Zone)
		CallEvent(Region().Zone, "ActorEntered", { ExpressionValue::ObjectValue(this) });
}

void UActor::SetOwner(UActor* newOwner)
{
	if (Owner())
		CallEvent(Owner(), "LostChild", { ExpressionValue::ObjectValue(this) });

	Owner() = newOwner;

	if (Owner())
		CallEvent(Owner(), "GainedChild", { ExpressionValue::ObjectValue(this) });
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
			CallEvent(ActorBase(), "Detach", { ExpressionValue::ObjectValue(this) });
		}

		ActorBase() = newBase;

		if (ActorBase() && ActorBase() != Level())
		{
			ActorBase()->StandingCount()++;
			CallEvent(ActorBase(), "Attach", { ExpressionValue::ObjectValue(this) });
		}

		if (sendBaseChangeEvent)
			CallEvent(this, "BaseChange");
	}
}

void UActor::Tick(float elapsed, bool tickedFlag)
{
	bTicked() = tickedFlag;

	TickAnimation(elapsed);

	if (Role() >= ROLE_SimulatedProxy && IsEventEnabled(tickEventName))
	{
		CallEvent(this, tickEventName, { ExpressionValue::FloatValue(elapsed) });
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
			CallEvent(this, "Timer");
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
			case PHYS_Rotating: TickRotating(physTimeElapsed); break;
			case PHYS_Projectile: TickProjectile(physTimeElapsed); break;
			case PHYS_Rolling: TickRolling(physTimeElapsed); break;
			case PHYS_Interpolating: TickInterpolating(physTimeElapsed); break;
			case PHYS_MovingBrush: TickMovingBrush(physTimeElapsed); break;
			case PHYS_Spider: TickSpider(physTimeElapsed); break;
			case PHYS_Trailer: TickTrailer(physTimeElapsed); break;
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
		CallEvent(this, "FellOutOfWorld");
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

	if (Velocity() == vec3(0.0f))
		return;

	// The classic step up, move and step down algorithm:

	float gravityDirection = zone->ZoneGravity().z > 0.0f ? 1.0f : -1.0f;
	vec3 stepUpDelta(0.0f, 0.0f, -gravityDirection * pawn->MaxStepHeight());
	vec3 stepDownDelta(0.0f, 0.0f, gravityDirection * pawn->MaxStepHeight() * 2.0f);

	// "Step up and move" as long as we have time left and only hitting surfaces with low enough slope that it could be walked
	float timeLeft = elapsed;
	for (int iteration = 0; timeLeft > 0.0f && iteration < 5; iteration++)
	{
		vec3 moveDelta = Velocity() * timeLeft;

		TryMove(stepUpDelta);
		SweepHit hit = TryMove(moveDelta);
		timeLeft -= timeLeft * hit.Fraction;

		if (hit.Fraction < 1.0f)
		{
			if (player && UObject::TryCast<UDecoration>(hit.Actor) && static_cast<UDecoration*>(hit.Actor)->bPushable() && dot(hit.Normal, moveDelta) < -0.9f)
			{
				// We hit a pushable decoration that is facing our movement direction

				bJustTeleported() = true;
				Velocity() = Velocity() * Mass() / (Mass() + hit.Actor->Mass());
				CallEvent(this, "HitWall", { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor) });
				timeLeft = 0.0f;
			}
			else if (hit.Normal.z < 0.2f && hit.Normal.z > -0.2f)
			{
				// We hit a wall

				CallEvent(this, "HitWall", { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor) });

				vec3 alignedDelta = (moveDelta - hit.Normal * dot(moveDelta, hit.Normal)) * (1.0f - hit.Fraction);
				if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
				{
					hit = TryMove(alignedDelta);
					timeLeft -= timeLeft * hit.Fraction;
					if (hit.Fraction < 1.0f)
					{
						CallEvent(this, "HitWall", { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor) });
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
	SweepHit hit = TryMove(stepDownDelta);
	if (hit.Fraction == 1.0f)
	{
		SetPhysics(PHYS_Falling);
	}

	if (!bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;
	Velocity().z = 0.0f;
}

void UActor::TickFalling(float elapsed)
{
	if (Region().ZoneNumber == 0)
	{
		CallEvent(this, "FellOutOfWorld");
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

	vec3 oldVel = Velocity();

	if (decor && decor->bBobbing())
	{
		gravityScale = 1.0f;
	}
	else if (pawn && pawn->FootRegion().Zone->bWaterZone() && oldVel.z < 0.0f)
	{
		fluidFriction = pawn->FootRegion().Zone->ZoneFluidFriction();
	}

	OldLocation() = Location();
	bJustTeleported() = false;

	Velocity() = oldVel * (1.0f - fluidFriction * elapsed) + (Acceleration() + gravityScale * zone->ZoneGravity()) * 0.5f * elapsed;

	float zoneTerminalVelocity = zone->ZoneTerminalVelocity();
	if (dot(Velocity(), Velocity()) > zoneTerminalVelocity * zoneTerminalVelocity)
	{
		Velocity() = normalize(Velocity()) * zoneTerminalVelocity;
	}

	SweepHit hit = TryMove((Velocity() + zone->ZoneVelocity()) * elapsed);
	if (hit.Fraction < 1.0f)
	{
		if (bBounce())
		{
			CallEvent(this, "HitWall", { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
		}
		else
		{
			// To do: slide along surfaces not pointing straight up
			// To do: if we do slide along a surface we need to fire HitWall before continueing movement

			CallEvent(this, "Landed", { ExpressionValue::VectorValue(hit.Normal) });

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

void UActor::TickSwimming(float elapsed)
{
}

void UActor::TickFlying(float elapsed)
{
}

void UActor::TickRotating(float elapsed)
{
	if (bRotateToDesired() && Rotation() != DesiredRotation())
	{
		// To do: rotate by RotationRate until we reach the rotation. Then fire "EndedRotation" event.
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

	SweepHit hit = TryMove(Velocity() * elapsed);

	if (hit.Fraction < 1.0f && !bDeleteMe() && !bJustTeleported())
	{
		CallEvent(this, "HitWall", { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
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
			CallEvent(target, "InterpolateEnd", { ExpressionValue::ObjectValue(this) });
			CallEvent(this, "InterpolateEnd", { ExpressionValue::ObjectValue(target) });

			target = target->Prev();
			while (target && target->bSkipNextPath())
				target = target->Prev();

			Target() = target;
			PhysAlpha() = 1.0f;
		}
		else if (interpolateEnd)
		{
			CallEvent(target, "InterpolateEnd", { ExpressionValue::ObjectValue(this) });
			CallEvent(this, "InterpolateEnd", { ExpressionValue::ObjectValue(target) });

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
}

void UActor::TickSpider(float elapsed)
{
}

void UActor::TickTrailer(float elapsed)
{
}

void UActor::SetPhysics(uint8_t newPhysics)
{
	Physics() = newPhysics;
}

void UActor::SetCollision(bool newColActors, bool newBlockActors, bool newBlockPlayers)
{
	XLevel()->RemoveFromCollision(this);
	bCollideActors() = newColActors;
	bBlockActors() = newBlockActors;
	bBlockPlayers() = newBlockPlayers;
	XLevel()->AddToCollision(this);
}

bool UActor::SetLocation(const vec3& newLocation)
{
	// To do: do overlap test and return false if the object cannot be moved to this location

	XLevel()->RemoveFromCollision(this);
	Location() = newLocation;
	XLevel()->AddToCollision(this);
	return true;
}

bool UActor::SetRotation(const Rotator& newRotation)
{
	// To do: do overlap test and return false if the object cannot be changed to this rotation
	
	Rotation() = newRotation;
	return true;
}

bool UActor::SetCollisionSize(float newRadius, float newHeight)
{
	// To do: do overlap test and return false if the object cannot be changed to this new size

	XLevel()->RemoveFromCollision(this);
	CollisionRadius() = newRadius;
	CollisionHeight() = newHeight;
	XLevel()->AddToCollision(this);
	return true;
}

UObject* UActor::Trace(vec3& hitLocation, vec3& hitNormal, const vec3& traceEnd, const vec3& traceStart, bool bTraceActors, const vec3& extent)
{
	// To do: this needs to do an AABB sweep (hmm, does UE1 treat all actor cylinders as AABB?)

	std::vector<SweepHit> hits = XLevel()->Sweep(traceStart, traceEnd, extent.z, extent.x, bTraceActors, true, false);
	for (SweepHit& hit : hits)
	{
		if (hit.Actor)
		{
			if (hit.Actor != this)
			{
				hitNormal = hit.Normal;
				hitLocation = traceStart + (traceEnd - traceStart) * hit.Fraction;
				return hit.Actor;
			}
		}
		else
		{
			hitNormal = hit.Normal;
			hitLocation = traceStart + (traceEnd - traceStart) * hit.Fraction;
			return Level();
		}
	}

	return nullptr;
}

bool UActor::FastTrace(const vec3& traceEnd, const vec3& traceStart)
{
	return XLevel()->TraceAnyHit(traceStart, traceEnd, this, false, true, false);
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

bool UActor::IsOverlapping(UActor* other)
{
	vec3 dist = Location() - other->Location();
	float h = CollisionHeight() + other->CollisionHeight();
	float r = CollisionRadius() + other->CollisionRadius();
	return dist.z < h && dot(dist.xy(), dist.xy()) < r * r;
}

SweepHit UActor::TryMove(const vec3& delta)
{
	// Static and non-movable objects can't move
	if (bStatic() || !bMovable())
	{
		SweepHit hit;
		hit.Fraction = 0.0f;
		return hit;
	}

	// Avoid moving if movement is too small as the physics code doesn't like very small numbers
	if (dot(delta, delta) < 0.0001f)
		return {};

	// Analyze what we will hit if we move as requested and stop if it is the level or a blocking actor
	bool useBlockPlayers = UObject::TryCast<UPlayerPawn>(this) || UObject::TryCast<UProjectile>(this);
	SweepHit blockingHit;
	std::vector<SweepHit> hits = XLevel()->Sweep(Location(), Location() + delta, CollisionHeight(), CollisionRadius(), bCollideActors(), bCollideWorld(), false);
	if (bCollideWorld() || bBlockActors() || bBlockPlayers())
	{
		for (auto& hit : hits)
		{
			if (hit.Actor)
			{
				bool isBlocking;
				if (useBlockPlayers || UObject::TryCast<UPlayerPawn>(hit.Actor) || UObject::TryCast<UProjectile>(hit.Actor))
					isBlocking = hit.Actor->bBlockPlayers();
				else
					isBlocking = hit.Actor->bBlockActors();

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

	XLevel()->RemoveFromCollision(this);
	Location() += actuallyMoved;
	XLevel()->AddToCollision(this);

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
			CallEvent(blockingHit.Actor, "Bump", { ExpressionValue::ObjectValue(this) });
			CallEvent(this, "Bump", { ExpressionValue::ObjectValue(blockingHit.Actor) });
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
					CallEvent(this, "Touch", { ExpressionValue::ObjectValue(actor) });

					if (!TouchingArray2[j])
					{
						TouchingArray2[j] = this;
						CallEvent(actor, "Touch", { ExpressionValue::ObjectValue(this) });
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
			CallEvent(this, "UnTouch", { ExpressionValue::ObjectValue(actor) });
		}
	}

	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray2[i] == this)
		{
			TouchingArray2[i] = nullptr;
			CallEvent(actor, "UnTouch", { ExpressionValue::ObjectValue(this) });
		}
	}
}

bool UActor::Move(const vec3& delta)
{
	return TryMove(delta).Fraction == 1.0f;
}

bool UActor::MoveSmooth(const vec3& delta)
{
	SweepHit hit = TryMove(delta);
	if (hit.Fraction != 1.0f)
	{
		// We hit a slope. Try to follow it.
		vec3 alignedDelta = (delta - hit.Normal * dot(delta, hit.Normal)) * (1.0f - hit.Fraction);
		if (dot(delta, alignedDelta) >= 0.0f) // Don't end up going backwards
		{
			SweepHit hit2 = TryMove(alignedDelta);
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
			AnimSequence() = sequence;
			AnimFrame() = tweenTime > 0.0f ? -1.0f / seq->NumFrames : 0.0f;
			AnimLast() = 1.0f - 1.0f / seq->NumFrames;
			AnimRate() = rate * seq->Rate / seq->NumFrames;
			AnimMinRate() = 0.0f;
			TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
			OldAnimRate() = AnimRate();
			bAnimNotify() = !seq->Notifys.empty();
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
			if (AnimSequence() != sequence || !IsAnimating())
			{
				AnimSequence() = sequence;
				AnimFrame() = tweenTime > 0.0f ? -1.0f / seq->NumFrames : 0.0f;
				AnimLast() = 1.0f - 1.0f / seq->NumFrames;
				bAnimNotify() = !seq->Notifys.empty();
				bAnimFinished() = false;
				bAnimLoop() = true;
			}
			AnimRate() = rate * seq->Rate / seq->NumFrames;
			AnimMinRate() = minRate * seq->Rate / seq->NumFrames;
			TweenRate() = tweenTime > 0.0f ? 1.0f / (tweenTime * seq->NumFrames) : 0.0f;
			OldAnimRate() = AnimRate();
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
		if (Mesh())
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
				if (bAnimNotify())
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

				// Clamp elapsed time to the animation end. This differs for looping animations as they also have to take the last frame into account before looping.
				float animEndTime = bAnimLoop() ? 1.0f : AnimLast();
				if (toAnimTime < fromAnimTime)
				{
					// This can happen if FinishAnimation is called after a looping animation made it past the AnimLast point
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
				else
				{
					AnimFrame() = toAnimTime;
				}

				if (fromAnimTime < animEndTime && toAnimTime >= animEndTime)
				{
					if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
						StateFrame->LatentState = LatentRunState::Continue;

					CallEvent(this, "AnimEnd");
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
					CallEvent(this, "AnimEnd");
				}
			}
		}
		else
		{
			elapsed = 0.0f;
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
			CallEvent(pawn, "HearNoise", { ExpressionValue::FloatValue(loudness), ExpressionValue::ObjectValue(this) });
		}
	}
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

	return !XLevel()->TraceAnyHit(source->Location(), Location(), source, false, true, false);
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
		CallEvent(FootRegion().Zone, "FootZoneChange", { ExpressionValue::ObjectValue(this) });
	FootRegion() = newfootregion;

	PointRegion oldheadregion = HeadRegion();
	PointRegion newheadregion = FindRegion({ 0.0f, 0.0f, EyeHeight() });
	if (HeadRegion().Zone && oldheadregion.Zone != newheadregion.Zone)
		CallEvent(HeadRegion().Zone, "HeadZoneChange", { ExpressionValue::ObjectValue(this) });
	HeadRegion() = newheadregion;

	if (PlayerReplicationInfo())
		PlayerReplicationInfo()->PlayerZone() = Region().Zone;
}

void UPawn::Tick(float elapsed, bool tickedFlag)
{
	UActor::Tick(elapsed, tickedFlag);

	if (bIsPlayer() && Role() >= ROLE_AutonomousProxy)
	{
		if (bViewTarget())
			CallEvent(this, "UpdateEyeHeight", { ExpressionValue::FloatValue(elapsed) });
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
				CallEvent(this, "PainTimer");
		}
		if (SpeechTime() > 0.0f)
		{
			SpeechTime() = std::max(SpeechTime() - elapsed, 0.0f);
			if (SpeechTime() == 0.0f)
				CallEvent(this, "SpeechTimer");
		}
		if (bAdvancedTactics())
			CallEvent(this, "UpdateTactics", { ExpressionValue::FloatValue(elapsed) });
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
			CallEvent(this, "PlayerInput", { ExpressionValue::FloatValue(elapsed) });
			CallEvent(this, "PlayerTick", { ExpressionValue::FloatValue(elapsed) });
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void ULevelInfo::UpdateActorZone()
{
	// No zone events are sent by LevelInfo actors
	Region() = FindRegion();
}
