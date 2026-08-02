
#include "Precomp.h"
#include "UPawn.h"
#include "UPlayerPawn.h"
#include "Packages/Core/UClass.h"
#include "Packages/Engine/Actors/Info/ULevelInfo.h"
#include "Packages/Engine/Actors/Info/UGameInfo.h"
#include "Packages/Engine/Actors/Info/UPlayerReplicationInfo.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Utils/Logger.h"
#include "Engine.h"

bool UPawn::LineOfSightTo(UActor* other, bool ignoreDistance)
{
	if (!other)
		return false;

	if (engine->LaunchInfo.IsUnreal1_227() &&
		(SightCheckType() == EPawnSightCheck::SEE_None ||
		(SightCheckType() == EPawnSightCheck::SEE_PlayersOnly && !Cast<UPawn>(other)->bIsPlayer())))
		return false;

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

	// Calculate the cosine of the vectors
	// which is basically A dot B / (|A| * |B|), or just the dot products of the normalized versions of A and B
	float cosine = dot(normalize(orientation), normalize(origin - eye_pos));
	float peripheralVision = PeripheralVision();
	if (cosine < peripheralVision)
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
