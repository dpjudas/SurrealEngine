
#include "Precomp.h"
#include "ULevelInfo.h"
#include "Packages/Engine/Resources/Level/ULevel.h"
#include "Packages/Engine/Resources/Level/UModel.h"
#include "Utils/Logger.h"

void ULevelInfo::UpdateActorZone()
{
	// No zone events are sent by LevelInfo actors
	Region() = FindRegion();
}

PointRegion ULevelInfo::GetLocZone(const vec3& pos, std::optional<UActor*> InActor)
{
	if (InActor)
		LogUnimplemented("ULevelInfo.GetLocZone() - U227k optional parameter InActor");
	return XLevel()->Model->FindRegion(pos, Level());
}
