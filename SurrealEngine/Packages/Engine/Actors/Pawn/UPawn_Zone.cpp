
#include "Precomp.h"
#include "UPawn.h"
#include "Engine.h"
#include "Packages/Engine/Actors/Info/UPlayerReplicationInfo.h"
#include "Packages/Engine/Actors/Info/UZoneInfo.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "VM/ScriptCall.h"

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
		CallEvent(this, EventName::FootZoneChange, { ExpressionValue::ObjectValue(newfootregion.Zone) });
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
		CallEvent(this, EventName::HeadZoneChange, { ExpressionValue::ObjectValue(newheadregion.Zone) });

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
