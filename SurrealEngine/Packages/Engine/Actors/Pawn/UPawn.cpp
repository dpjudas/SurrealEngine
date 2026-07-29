
#include "Precomp.h"
#include "UPawn.h"
#include "PawnVision.h"
#include "UPlayerPawn.h"
#include "Packages/Core/UClass.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Info/UGameInfo.h"
#include "Packages/Engine/Actors/Info/UPlayerReplicationInfo.h"
#include "Packages/Engine/Actors/NavigationPoint/UNavigationPoint.h"
#include "Packages/Engine/Actors/NavigationPoint/UInventorySpot.h"
#include "Packages/Engine/Actors/Inventory/UInventory.h"
#include "Packages/Engine/Actors/Inventory/UWeapon.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Utils/Logger.h"
#include "Engine.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"

static constexpr int walkingSimulationMaxIterations = 32;
static constexpr float walkingSimulationFallDepth = 1024.0f;

bool UPawn::ActorReachable(UActor* anActor, bool checkNavpoint)
{
	if (!anActor)
		return false;

	UPawn* aPawn = UObject::TryCast<UPawn>(anActor);

	// If actor is not a pawn we assume we can't reach if they are too far away
	if (!aPawn)
	{
		vec3 delta = anActor->Location() - Location();
		float dist2 = dot(delta, delta);
		if (dist2 > 1000.0f * 1000.0f)
			return false;
	}

	// Navpoints may not be reachable at all according to reachspecs
	if (checkNavpoint)
	{
		// Check if we are trying to reach a navigation point.
		// They can also be hiding in an inventory as an inventory (pickup item) can be linked to a navigation point.
		UNavigationPoint* navPoint = UObject::TryCast<UNavigationPoint>(anActor);
		if (UInventory* inventory = UObject::TryCast<UInventory>(anActor))
			navPoint = inventory->myMarker();

		if (navPoint)
		{
			// Check if the navigation point is theoretically reachable at all according to reachspecs.
			bool couldBeReachable = false;
			float radius = CollisionRadius();
			float height = CollisionHeight();
			for (UNavigationPoint* cur = Level()->NavigationPointList(); cur != nullptr; cur = cur->nextNavigationPoint())
			{
				const auto& specs = XLevel()->ReachSpecs;
				for (int index : cur->Paths())
				{
					if (index < 0 || (size_t)index >= specs.size())
						break;
					const LevelReachSpec& reachSpec = specs[index];

					if (reachSpec.endActor != navPoint)
						continue; // Not a path to this nav point

					if (reachSpec.collisionRadius < radius || reachSpec.collisionHeight < height || reachSpec.bPruned)
						continue; // Skip nav node links that we can't pass through

					if (reachSpec.endActor->bPlayerOnly() && !bIsPlayer())
						continue; // Skip nav nodes only for the player if we aren't one

					// To do: check reachFlags

					couldBeReachable = true;
					break;
				}

				if (couldBeReachable)
					break;

				for (int index : cur->PrunedPaths())
				{
					if (index < 0 || (size_t)index >= specs.size())
						break;
					const LevelReachSpec& reachSpec = specs[index];

					if (reachSpec.endActor != navPoint)
						continue; // Not a path to this nav point

					if (reachSpec.collisionRadius < radius || reachSpec.collisionHeight < height || reachSpec.bPruned)
						continue; // Skip nav node links that we can't pass through

					if (reachSpec.endActor->bPlayerOnly() && !bIsPlayer())
						continue; // Skip nav nodes only for the player if we aren't one

					// To do: check reachFlags

					couldBeReachable = true;
					break;
				}

				if (couldBeReachable)
					break;
			}

			if (!couldBeReachable)
				return false;
		}
	}

	// If the actor is in a pain zone and we don't like pain we can't go there
	if (aPawn)
	{
		if (aPawn->FootRegion().Zone->bPainZone() && aPawn->FootRegion().Zone->DamageType() != ReducedDamageType())
			return false;
	}
	else
	{
		if (anActor->Region().Zone->bPainZone() && anActor->Region().Zone->DamageType() != ReducedDamageType())
			return false;
	}

	// If the actor is in the water and we can't swim we can't go there
	if (anActor->Region().Zone->bWaterZone() && !bCanSwim())
		return false;

	vec3 eyePos = Location();
	eyePos.z += BaseEyeHeight();

	// If we can't see the actor we can't go there
	if (!FastTrace(anActor->Location(), eyePos))
		return false;

	// If we can't stand at the actor location we can't go there
	if (!CheckLocation(anActor->Location(), CollisionRadius(), CollisionHeight(), bCollideWorld() || bCollideWhenPlacing()).first)
		return false;

	// Try simulate movement to see if we can get to the actor
	int mode = Physics();
	if (mode == PHYS_Walking)
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
	else if (mode == PHYS_Flying || mode == PHYS_Swimming)
	{
		// To do: take zone changes into account?

		vec3 oldLocation = Location();
		bool reached = false;
		for (int iteration = 0; iteration < 5; iteration++)
		{
			vec3 moveDelta = anActor->Location() - Location();
			float goalDist2 = dot(moveDelta, moveDelta);
			if (goalDist2 <= 1.0f)
			{
				reached = true;
				break;
			}

			CollisionHit hit = TryMove(moveDelta, true);
			vec3 actuallyMoved = moveDelta * hit.Fraction;
			Location() += actuallyMoved;

			if (hit.Fraction < 1.0f)
			{
				moveDelta = anActor->Location() - Location();
				vec3 alignedDelta = (moveDelta - hit.Normal * dot(moveDelta, hit.Normal)) * (1.0f - hit.Fraction);
				if (dot(moveDelta, alignedDelta) >= 0.0f) // Don't end up going backwards
				{
					hit = TryMove(alignedDelta, true);
					actuallyMoved = moveDelta * hit.Fraction;
					Location() += actuallyMoved;
				}
				else
				{
					break;
				}
			}

			float moveDist2 = dot(actuallyMoved, actuallyMoved);
			if (moveDist2 <= 1.0f)
				break;
		}

		Location() = oldLocation;
		return reached;
	}
	else
	{
		// Hopefully not a physics mode the bots use when calling ActorReachable
		LogUnimplemented("ActorReachable called for unsupported physics mode");
		return false;
	}
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

bool UPawn::LineOfSightTo(UActor* other, bool ignoreDistance)
{
	if (!other)
		return false;

	// Additional 227 checks because of Pawn.SightCheckType being a variable there
	// Since we don't have any 227-only fields added yet, this part remains as a proof of concept
	// if (engine->packages->IsUnreal1_227() &&
	// 	(SightCheckType() == EPawnSightCheck::SEE_None ||
	// 	(SightCheckType() == EPawnSightCheck::SEE_PlayersOnly && !Cast<UPawn>(other)->bIsPlayer())))
	// 	return false;

	if (!ignoreDistance && length(Location() - other->Location()) > SightRadius())
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
	if (!PawnVision::IsWithinVisionCone(eye_pos, origin, orientation, PeripheralVision()))
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
		(engine->LaunchInfo.ue1Version > 219 && (!PlayerReplicationInfo() || !noisePawn->PlayerReplicationInfo() || (PlayerReplicationInfo()->Team() != noisePawn->PlayerReplicationInfo()->Team()))))
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
	UPlayerReplicationInfo* ourPlayerInfo = engine->LaunchInfo.ue1Version > 219 ? PlayerReplicationInfo() : nullptr;
	bool teamGame = ourPlayerInfo && Level()->Game()->bTeamGame();
	for (UPawn* pawn = Level()->PawnList(); pawn != nullptr; pawn = pawn->nextPawn())
	{
		// Skip dead pawns or ourselves
		if (pawn == this || pawn->Health() <= 0)
			continue;

		// Skip team mates
		if (engine->LaunchInfo.ue1Version > 219)
		{
			auto pawnPlayerInfo = pawn->PlayerReplicationInfo();
			if (teamGame && pawnPlayerInfo && ourPlayerInfo->Team() == pawnPlayerInfo->Team())
				continue;
		}

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
	if (!LineOfSightTo(actor, false))
		return false;

	// OK, this is better than what we have
	bestAim = angle;
	bestDist = distance;
	return true;
}

UNavigationPoint* UPawn::SetRouteCache(const Array<UNavigationPoint*>& points)
{
	if (engine->LaunchInfo.ue1Version > 219)
	{
		auto cache = RouteCache();
		for (size_t i = 0; i < cache.size(); i++)
			cache[i] = (i < points.size()) ? points[i] : nullptr;
	}
	return !points.empty() ? points.front() : nullptr;
}

UActor* UPawn::PathSpecialHandling(const Array<UNavigationPoint*>& bestPath)
{
#if 0
	return SetRouteCache(bestPath);
#else
	IsInPathSpecialHandling = true;
	UActor* oldBestPoint = SetRouteCache(bestPath);
	if (!oldBestPoint)
	{
		IsInPathSpecialHandling = false;
		return nullptr;
	}

	UActor* bestPoint = oldBestPoint;

	if (oldBestPoint->IsEventEnabled(EventName::SpecialHandling))
	{
		bestPoint = UObject::Cast<UActor>(CallEvent(oldBestPoint, EventName::SpecialHandling, { ExpressionValue::ObjectValue(this) }).ToObject());
		if (!bCanDoSpecial())
			bestPoint = nullptr;
		SpecialGoal() = bestPoint;

		if (bestPoint && bestPoint != oldBestPoint)
		{
			if (!ActorReachable(bestPoint))
			{
				bestPoint = UObject::Cast<UActor>(FindPathToward(bestPoint, false));
			}
		}
	}
	else
	{
		if (SpecialGoal() == oldBestPoint)
			SpecialGoal() = nullptr;
	}

	IsInPathSpecialHandling = false;
	return bestPoint;
#endif
}


std::pair<Array<UNavigationPoint*>, int32_t> UPawn::FindPathToEndPoint(UNavigationPoint* start, int maxNodes)
{
	if ((start->bPlayerOnly() && !bIsPlayer()))
		return { {}, 0 };

	// If we can already reach the end point, just go there directly
	if (start->bEndPoint())
		return { { start }, 0 };

	struct Step
	{
		UNavigationPoint* navpoint;
		int prev;
		int32_t distance;
	};

	std::unordered_map<UNavigationPoint*, int32_t> shortestDistance;
	std::set<int> visited;
	Array<Step> steps;
	Array<size_t> stepEnds;
	const Array<LevelReachSpec>& reachSpecs = XLevel()->ReachSpecs;

	int radius = (int)CollisionRadius();
	int height = (int)CollisionHeight();

	// Search through the nav node links until we find an end point

	int prevStep = -1;
	UNavigationPoint* current = start;
	while (steps.size() < (size_t)maxNodes)
	{
		if (!current->bEndPoint())
		{
			for (int specIndex : current->upstreamPaths())
			{
				if (specIndex < 0 || (size_t)specIndex >= reachSpecs.size())
					break;
				const LevelReachSpec& reachSpec = reachSpecs[specIndex];

				// Note: startActor instead of endActor because upstreamPaths is the reverse travel direction
				UNavigationPoint* endActor = reachSpec.startActor;

				if (reachSpec.collisionRadius < radius || reachSpec.collisionHeight < height || reachSpec.bPruned)
					continue; // Skip nav node links that we can't pass through

				if (endActor->bPlayerOnly() && !bIsPlayer())
					continue; // Skip nav nodes only for the player if we aren't one

				// To do: check reachFlags

				// How far have we travelled so far?
				int32_t distance = reachSpec.distance;
				if (prevStep >= 0)
					distance += steps[prevStep].distance;

				// Is this distance shorter than last time we reached this point?
				int32_t& pointDistance = shortestDistance[endActor];
				if (pointDistance == 0 || distance < pointDistance)
				{
					// Yes. Track this path and reject any future paths going through here that are longer.
					pointDistance = distance;
					if (endActor->bEndPoint())
						stepEnds.push_back(steps.size());
					steps.push_back({ .navpoint = endActor, .prev = prevStep, .distance = distance });
				}
			}
		}

		prevStep++;
		if (prevStep == steps.size())
			break;

		current = steps[prevStep].navpoint;
	}

	if (stepEnds.empty())
		return { {}, 0 };

	std::sort(stepEnds.begin(), stepEnds.end(), [&](size_t a, size_t b) { return steps[a].distance < steps[b].distance; });

	// Extract the final path:
	Array<UNavigationPoint*> path;
	int currentStep = (int)stepEnds.front();
	while (currentStep >= 0)
	{
		const Step& step = steps[currentStep];

		// Skip path parts we already are touching
		float minDist = (float)radius;
		vec3 d = step.navpoint->Location() - Location();
		float heightDiff = step.navpoint->Location().z - Location().z;
		if (dot(d, d) < minDist * minDist && std::abs(heightDiff) < (float)height)
		{
			path.clear();
		}
		else
		{
			path.push_back(step.navpoint);
		}

		currentStep = step.prev;
	}
	path.push_back(start);

	return { path, steps[stepEnds.front()].distance };
}

void UPawn::ClearPaths()
{
	for (UNavigationPoint* cur = Level()->NavigationPointList(); cur; cur = cur->nextNavigationPoint())
	{
		cur->bEndPoint() = false;
		if (!engine->LaunchInfo.IsKlingonHonorGuard())
		{
			if (cur->bSpecialCost())
				cur->cost() = CallEvent(cur, "SpecialCost", { ExpressionValue::ObjectValue(this) }).ToInt();
			else
				cur->cost() = cur->ExtraCost();
		}
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
		if (navPoint->bPlayerOnly() && !bIsPlayer())
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

		for (int specIndex : navPoint->Paths())
		{
			if (specIndex < 0 || (size_t)specIndex >= reachSpecs.size())
				break;
			const LevelReachSpec& reachSpec = reachSpecs[specIndex];

			if (reachSpec.endActor->bEndPoint())
				continue; // Already processed

			if (reachSpec.collisionRadius < radius || reachSpec.collisionHeight < height || reachSpec.bPruned)
				continue; // Skip nav node links that we can't pass through

			if (reachSpec.endActor->bPlayerOnly() && !bIsPlayer())
				continue; // Skip nav nodes only for the player if we aren't one

			// To do: check reachFlags

			reachSpec.endActor->bEndPoint() = true;
			reachablePoints.push_back(reachSpec.endActor);
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

bool UPawn::MarkReachableNavEndPoints()
{
	int maxActorReachableCalls = 8; // upper bound for how expensive this can get
	int endPointsFound = 0;
	for (UNavigationPoint* navPoint = Level()->NavigationPointList(); navPoint; navPoint = navPoint->nextNavigationPoint())
	{
		navPoint->bEndPoint() = false;

		if (endPointsFound < maxActorReachableCalls)
		{
			if (navPoint->bPlayerOnly() && !bIsPlayer())
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

	return endPointsFound > 0;
}

float UPawn::AICanHear(UActor* other, std::optional<float> volume, std::optional<float> radius)
{
	LogUnimplemented("Pawn.AICanHear() [Deus Ex]");
	return 0.0f;
}

float UPawn::AICanSee(UActor* other, std::optional<float> visibility, std::optional<bool> bCheckVisibility, std::optional<bool> bCheckDir, std::optional<bool> bCheckCylinder, std::optional<bool> bCheckLOS)
{
	LogUnimplemented("Pawn.AICanSee() [Deus Ex]");
	return 0.0f;
}

float UPawn::AICanSmell(UActor* other, std::optional<float> smell)
{
	LogUnimplemented("Pawn.AICanSmell() [Deus Ex]");
	return 0.0f;
}

UObject* UPawn::FindPathToward(UObject* anActor, bool singlePath)
{
	if (auto aNavPoint = UObject::TryCast<UNavigationPoint>(anActor))
	{
		if (!MarkReachableNavEndPoints())
			return SetRouteCache({});
		if (!IsInPathSpecialHandling)
			return PathSpecialHandling(FindPathToEndPoint(aNavPoint, 1000).first);
		return SetRouteCache({});
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
		if (navPoint->bPlayerOnly() && !bIsPlayer())
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

UObject* UPawn::FindBestInventoryPath(bool predictRespawns, float& outBestWeight)
{
	if (!MarkReachableNavEndPoints())
	{
		outBestWeight = 0.0f;
		return SetRouteCache({});
	}

	float bestWeight = 0.0f;
	UInventorySpot* bestSpot = nullptr;
	Array<UNavigationPoint*> bestPath;

	for (UNavigationPoint* navPoint = Level()->NavigationPointList(); navPoint; navPoint = navPoint->nextNavigationPoint())
	{
		auto invSpot = UObject::TryCast<UInventorySpot>(navPoint);
		if (!invSpot)
			continue;
		auto inv = invSpot->markedItem();
		if (!inv)
			continue;

		if (inv->GetStateName() != "PickUp")
			continue;

		float desire = CallEvent(inv, "BotDesireability", { ExpressionValue::ObjectValue(this) }).ToFloat();
		if (desire > 0.0f)
		{
			auto [path, pathDist] = FindPathToEndPoint(invSpot, 1000);

			// To do: how to take path costs into account?
			//int cost = 0;
			//for (auto nav : path)
			//	cost += nav->cost();

			float distance = std::max((float)pathDist, 1.0f);
			float weight = desire / distance;

			if (!bestSpot || weight > bestWeight)
			{
				bestSpot = invSpot;
				bestWeight = weight;
				bestPath = std::move(path);
			}
		}
	}

	if (bestSpot)
	{
		outBestWeight = bestWeight;
		return PathSpecialHandling(bestPath);
	}
	else
	{
		outBestWeight = 0.0f;
		return SetRouteCache({});
	}
}

void UPawn::InitActorZone()
{
	UActor::InitActorZone();

	FootRegion() = FindRegion({ 0.0f, 0.0f, -CollisionHeight() });
	HeadRegion() = FindRegion({ 0.0f, 0.0f, EyeHeight() });

	if (engine->LaunchInfo.ue1Version > 219 && PlayerReplicationInfo())
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

	if (engine->LaunchInfo.ue1Version > 219 && PlayerReplicationInfo())
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
			if (engine->LaunchInfo.ue1Version > 219 && FaceTarget())
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
			if (engine->LaunchInfo.ue1Version > 219 && FaceTarget())
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
		if (engine->LaunchInfo.ue1Version < 400 || bViewTarget())
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
		if (engine->LaunchInfo.ue1Version >= 436 && bAdvancedTactics())
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
	if (engine->LaunchInfo.ue1Version > 219)
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

	if (engine->LaunchInfo.ue1Version > 219)
		FaceTarget() = newTarget;
	Focus() = newTarget->Location();
	if (StateFrame)
		StateFrame->LatentState = LatentRunState::TurnToward;
}

void UPawn::WaitForLanding()
{
	if (StateFrame)
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
