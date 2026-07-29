
#include "Precomp.h"
#include "UActor.h"
#include "VM/ScriptCall.h"
#include "Packages/Engine/Actors/Decoration/UDecoration.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Brush/UMover.h"
#include "Packages/Engine/Actors/Pawn/UPlayerPawn.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Engine.h"

#include <algorithm>
#include <cmath>

namespace
{
	constexpr int painZoneFallPredictionSteps = 24;
	constexpr int painZoneFallMaxSamplesPerStep = 8;
	constexpr float painZoneFallPredictionDelta = 1.0f / 16.0f;

	struct FallPredictionState
	{
		vec3 Location;
		vec3 Velocity;
		bool Valid = true;
	};

	bool IsFinite(const vec3& value)
	{
		return std::isfinite(value.x) && std::isfinite(value.y) && std::isfinite(value.z);
	}

	FallPredictionState PredictFallStep(FallPredictionState state,
		const vec3& acceleration, const vec3& gravity, const vec3& zoneVelocity,
		float groundSpeed, float terminalVelocity)
	{
		if (!state.Valid || !IsFinite(state.Location) || !IsFinite(state.Velocity)
			|| !IsFinite(acceleration) || !IsFinite(gravity) || !IsFinite(zoneVelocity)
			|| !std::isfinite(groundSpeed) || !std::isfinite(terminalVelocity)
			|| groundSpeed < 0.0f || terminalVelocity < 0.0f)
		{
			state.Valid = false;
			return state;
		}

		vec3 velocity = state.Velocity
			+ (acceleration * 1.5f + gravity * 2.0f) * (0.5f * painZoneFallPredictionDelta);
		const float oldSpeedSquared = dot(state.Velocity.xy(), state.Velocity.xy());
		const float newSpeedSquared = dot(velocity.xy(), velocity.xy());
		if (oldSpeedSquared >= groundSpeed * groundSpeed && newSpeedSquared > oldSpeedSquared)
			velocity = vec3(normalize(velocity.xy()) * std::sqrt(oldSpeedSquared), velocity.z);
		if (dot(velocity, velocity) > terminalVelocity * terminalVelocity)
			velocity = normalize(velocity) * terminalVelocity;

		state.Velocity = velocity;
		state.Location += (velocity
			+ zoneVelocity * painZoneFallPredictionDelta * 25.0f)
			* painZoneFallPredictionDelta;
		state.Valid = IsFinite(state.Location) && IsFinite(state.Velocity);
		return state;
	}

	bool IsHarmfulPainZone(UPawn* pawn, UZoneInfo* zone)
	{
		return zone && zone->bPainZone() && zone->DamageType() != pawn->ReducedDamageType();
	}

	bool IsAutonomousPlayerBot(UPawn* pawn)
	{
		UPlayerPawn* player = UObject::TryCast<UPlayerPawn>(pawn);
		return pawn->bIsPlayer() && (!player || !player->Player());
	}

	bool PredictedFallEntersHarmfulPainZone(
		UPawn* pawn, const vec3& initialVelocity, const vec3& acceleration)
	{
		UZoneInfo* startZone = pawn->FootRegion().Zone;
		UZoneInfo* physicsZone = pawn->Region().Zone;
		if (!startZone || !physicsZone)
			return false;

		FallPredictionState prediction = { pawn->Location(), initialVelocity };
		const TraceFlags traceFlags = { .movers = true, .world = true };
		const vec3 traceExtent(
			pawn->CollisionRadius(), pawn->CollisionRadius(), pawn->CollisionHeight());
		const float maxSpacing = std::max(pawn->MaxStepHeight(), 1.0f);

		for (int index = 0; index < painZoneFallPredictionSteps; index++)
		{
			FallPredictionState next = PredictFallStep(prediction, acceleration,
				physicsZone->ZoneGravity(), physicsZone->ZoneVelocity(),
				pawn->GroundSpeed(), physicsZone->ZoneTerminalVelocity());
			if (!next.Valid)
				return false;

			const CollisionHit hit = pawn->XLevel()->Collision.TraceFirstHit(
				prediction.Location, next.Location, pawn, traceExtent, traceFlags);
			const vec3 segment = (next.Location - prediction.Location) * hit.Fraction;
			const int sampleCount = std::min(
				static_cast<int>(std::ceil(length(segment) / maxSpacing)),
				painZoneFallMaxSamplesPerStep);
			for (int sampleIndex = 1; sampleIndex <= sampleCount; sampleIndex++)
			{
				const vec3 sample = prediction.Location
					+ segment * (static_cast<float>(sampleIndex) / sampleCount);
				UZoneInfo* sampleZone = pawn->XLevel()->Model->FindRegion(
					sample - vec3(0.0f, 0.0f, pawn->CollisionHeight()),
					pawn->Level()).Zone;
				if (IsHarmfulPainZone(pawn, sampleZone))
					return true;
				if (sampleZone != startZone)
					return false;
			}
			if (hit.Fraction < 1.0f)
				return false;
			prediction = next;
		}
		return false;
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
			const vec3 iterationStartLocation = Location();
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
				if (pawn->bCanJump())
					CallEvent(pawn, EventName::MayFall);
				if (pawn->bDeleteMe() || pawn->Physics() != PHYS_Walking)
					return;

				bool restoreGrounded = !pawn->bCanJump();
				if (!restoreGrounded && IsAutonomousPlayerBot(pawn)
					&& pawn->Region().Zone && pawn->Region().Zone->ZoneGravity().z < 0.0f
					&& !IsHarmfulPainZone(pawn, pawn->FootRegion().Zone))
				{
					vec3 fallAcceleration = pawn->Acceleration();
					const float maxAirAcceleration = engine->LaunchInfo.ue1Version > 219
						? pawn->AirControl() * pawn->AccelRate() : 0.0f;
					const float fallAccelerationLength = length(fallAcceleration);
					if (fallAccelerationLength > maxAirAcceleration)
						fallAcceleration = normalize(fallAcceleration) * maxAirAcceleration;
					restoreGrounded = PredictedFallEntersHarmfulPainZone(
						pawn, pawn->Velocity(), fallAcceleration);
				}

				if (restoreGrounded)
				{
					TryMove(iterationStartLocation - Location());
					pawn->Velocity() = vec3(0.0f);
					pawn->Acceleration() = vec3(0.0f);
					pawn->MoveTimer() = -1.0f;
					return;
				}

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
