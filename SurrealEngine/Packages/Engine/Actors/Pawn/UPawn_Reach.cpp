
#include "Precomp.h"
#include "UPawn.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/NavigationPoint/UNavigationPoint.h"
#include "Packages/Engine/Actors/NavigationPoint/UInventorySpot.h"
#include "Packages/Engine/Actors/Inventory/UInventory.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Utils/Logger.h"

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
	switch (Physics())
	{
	case PHYS_Walking:
		return ReachableWalking(anActor);
	case PHYS_Falling:
		return false; // Should we be able to reach anything in this state?
	case PHYS_Swimming:
		return ReachableSwimming(anActor);
	case PHYS_Flying:
		return ReachableFlying(anActor);
	case PHYS_Spider:
		return ReachableSpider(anActor);
	case PHYS_Rotating:
	case PHYS_Projectile:
	case PHYS_Rolling:
	case PHYS_Interpolating:
	case PHYS_MovingBrush:
	case PHYS_Trailer:
	default:
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
