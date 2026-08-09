
#include "Precomp.h"
#include "UActor.h"
#include "VM/ScriptCall.h"
#include "Engine.h"
#include "Packages/Engine/Actors/UProjectile.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"

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

		CheckPendingTouch();
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

bool UActor::HasLeftWorld()
{
	if (Region().ZoneNumber != 0)
		return false;

	CallEvent(this, EventName::FellOutOfWorld);
	return true;
}

void UActor::ApplyMovementAcceleration(float elapsed, float accelRate, float friction, float maxSpeed)
{
	if (dot(Acceleration(), Acceleration()) > 0.0001f)
	{
		// Acceleration must never exceed the acceleration rate
		float accelSpeed = length(Acceleration());
		vec3 accelDir = Acceleration() * (1.0f / accelSpeed);
		if (accelSpeed > accelRate)
			Acceleration() = accelDir * accelRate;

		float speed = length(Velocity());
		Velocity() = Velocity() - (Velocity() - accelDir * speed) * (friction * elapsed);
	}
	else
	{
		float speed = length(Velocity());
		if (speed > 0.0f)
		{
			float newSpeed = std::max(speed - speed * friction * 2.0f * elapsed, 0.0f);
			Velocity() = Velocity() * (newSpeed / speed);
		}
	}

	Velocity() = Velocity() + Acceleration() * elapsed;

	float speed = length(Velocity());
	if (speed > 0.0f && speed > maxSpeed)
		Velocity() = Velocity() * (maxSpeed / speed);
}

bool UActor::ShouldAbortMovementTick(uint8_t expectedPhysics)
{
	return bDeleteMe() || Physics() != expectedPhysics;
}

UPawn* UActor::PreparePawnMovementTick()
{
	UPawn* pawn = UObject::TryCast<UPawn>(this);
	if (!pawn)
		return nullptr;

	if (HasLeftWorld())
		return nullptr;

	OldLocation() = Location();
	bJustTeleported() = false;

	return pawn;
}

void UActor::FireHitWall(const CollisionHit& hit)
{
	CallEvent(this, EventName::HitWall, { ExpressionValue::VectorValue(hit.Normal), ExpressionValue::ObjectValue(hit.Actor ? hit.Actor : Level()) });
}

bool UActor::TryStepToGround(vec3 stepDownDelta)
{
	CollisionHit floorHit = TryMove(stepDownDelta, true);

	//check if floo was reached, or if we would be falling now
	if (floorHit.Fraction == 1.0f || floorHit.Normal.z < 0.7071f)
	{
		//if we would be falling, move back up
		return false;
	}

	// We could reach the ground. Step down there.
	floorHit = TryMove(stepDownDelta);
	if (floorHit.Fraction != 1.0f)
		SetBase(floorHit.Actor, true);
	return true;
}

void UActor::RecomputeVelocityFromDisplacement(float elapsed)
{
	if (!bJustTeleported())
		Velocity() = (Location() - OldLocation()) / elapsed;
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

bool UActor::IsOverlapping(UActor* other)
{
	return XLevel()->Collision.IsOverlapping(this, other);
}

BoundingBox UActor::GetWorldCollisionBox(bool bVisual)
{
	LogUnimplemented("Actor.GetWorldCollisionBox");
	return {};
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
