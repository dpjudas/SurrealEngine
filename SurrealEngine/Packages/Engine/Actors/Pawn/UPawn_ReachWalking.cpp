
#include "Precomp.h"
#include "UPawn.h"
#include "Packages/Engine/Actors/Info/UZoneInfo.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Utils/Logger.h"

static constexpr int walkingSimulationMaxIterations = 32;
static constexpr float walkingSimulationFallDepth = 1024.0f;

bool UPawn::ReachableWalking(UActor* anActor)
{
	// To do: take zone changes into account?

	auto zone = Region().Zone;
	float gravityDirection = zone->ZoneGravity().z > 0.0f ? 1.0f : -1.0f;
	vec3 stepUpDelta(0.0f, 0.0f, -gravityDirection * MaxStepHeight());
	vec3 stepDownDelta(0.0f, 0.0f, gravityDirection * MaxStepHeight() * stepDownDeltaFactor);

	vec3 oldLocation = Location();
	bool reached = false;

	// Advance a step at a time and settle onto the floor after each one. Sweeping the whole
	// remaining distance in a single move samples nothing in between, so a gap or a ledge
	// between here and the goal goes unnoticed and the goal is reported reachable.
	const float stepLength = std::max(CollisionRadius() * 2.0f, 1.0f);
	const vec3 settleDelta = stepDownDelta - stepUpDelta;
	for (int iteration = 0; iteration < walkingSimulationMaxIterations; iteration++)
	{
		vec3 toGoal = anActor->Location() - Location();
		toGoal.z = 0.0f;
		float goalDist2 = dot(toGoal, toGoal);
		if (goalDist2 <= 1.0f)
		{
			reached = true;
			break;
		}

		vec3 moveDelta = toGoal;
		const float goalDist = std::sqrt(goalDist2);
		if (goalDist > stepLength)
			moveDelta = toGoal * (stepLength / goalDist);

		// step up first so we can get past stairs going up
		CollisionHit hit = TryMove(stepUpDelta, true);
		Location() += stepUpDelta * hit.Fraction;

		// move towards goal
		hit = TryMove(moveDelta, true);
		vec3 actuallyMoved = moveDelta * hit.Fraction;
		Location() += actuallyMoved;

		if (hit.Fraction < 1.0f)
		{
			vec3 remaining = moveDelta * (1.0f - hit.Fraction);
			vec3 alignedDelta = remaining - hit.Normal * dot(remaining, hit.Normal);
			if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
			{
				hit = TryMove(alignedDelta, true);
				Location() += alignedDelta * hit.Fraction;
				actuallyMoved += alignedDelta * hit.Fraction;
			}
			else
			{
				break;
			}
		}

		// Settle back onto the floor. Beyond a step down we are falling rather than walking,
		// which is still allowed because bots drop off ledges to reach things, but we have to
		// land on walkable ground. Whether the landing got anywhere is decided by the height
		// check after the loop, so falling into a pit fails there rather than here.
		auto settleOnto = [&](const vec3& delta)
			{
				const CollisionHit floorHit = TryMove(delta, true);
				if (floorHit.Fraction >= 1.0f || floorHit.Normal.z * -gravityDirection < 0.7071f)
					return false;
				Location() += delta * floorHit.Fraction;
				return true;
			};
		if (!settleOnto(settleDelta) && !settleOnto(vec3(0.0f, 0.0f, gravityDirection * walkingSimulationFallDepth)))
			break;

		float moveDist2 = dot(actuallyMoved, actuallyMoved);
		if (moveDist2 <= 1.0f)
			break;
	}

	if (reached)
	{
		// Step down + fall to goal
		vec3 moveDelta = anActor->Location() - Location();
		moveDelta.x = 0.0f;
		moveDelta.y = 0.0f;
		if ((moveDelta.z < -0.1f && gravityDirection == -1.0f) || (moveDelta.z > 0.1f && gravityDirection == 1.0f))
		{
			CollisionHit hit = TryMove(moveDelta, true);
			vec3 actuallyMoved = moveDelta * hit.Fraction;
			Location() += actuallyMoved;
		}

		// Did we get there vertically too?
		reached = std::abs(anActor->Location().z - Location().z) <= CollisionHeight();
	}

	Location() = oldLocation;
	return reached;
}
