
#include "Precomp.h"
#include "UPawn.h"
#include "Packages/Engine/Actors/Info/UZoneInfo.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Utils/Logger.h"

bool UPawn::ReachableFlying(UActor* anActor)
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

