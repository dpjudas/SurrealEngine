
#include "Precomp.h"
#include "Iterator.h"
#include "Utils/File.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"

AllObjectsIterator::AllObjectsIterator(UObject* BaseClass, UObject** ReturnValue, NameString MatchTag) : BaseClass(BaseClass), ReturnValue(ReturnValue), MatchTag(MatchTag)
{
}

bool AllObjectsIterator::Next()
{
	bool matchTag = !MatchTag.IsNone();
	size_t size = engine->Level->Actors.size();
	while (index < size)
	{
		UActor* actor = engine->Level->Actors[index++];
		if (actor && actor->IsA(BaseClass->Name) && (!matchTag || actor->Tag() == MatchTag))
		{
			*ReturnValue = actor;
			return true;
		}
	}
	*ReturnValue = nullptr;
	return false;
}

/////////////////////////////////////////////////////////////////////////////

AllFilesIterator::AllFilesIterator(const std::string& FileExtension, const std::string& FilePrefix, std::string& FileName) : FileExtension(FileExtension), FilePrefix(FilePrefix), FileName(FileName)
{
	auto packageNames = engine->packages->GetPackageNames();

	for (auto& packageName : packageNames)
	{
		auto package = engine->packages->GetPackage(packageName);

		if ((FileExtension.empty() || fs::path(package->GetPackageFilename()).extension().string() == FileExtension) &&
			(FilePrefix.empty() || package->GetPackageFilename().find(FilePrefix) != std::string::npos))
		{
			FoundFiles.push_back(packageName.ToString());
		}
	}

	iterator = FoundFiles.begin();
}

bool AllFilesIterator::Next()
{
	if (iterator == FoundFiles.end())
	{
		FileName = {};
		return false;
	}

	FileName = *iterator;
	iterator++;

	return true;
}

/////////////////////////////////////////////////////////////////////////////

BasedActorsIterator::BasedActorsIterator(UActor* Caller, UObject* BaseClass, UObject** Actor) : BaseClass(BaseClass), Actor(Actor)
{
	for (UActor* levelActor : engine->Level->Actors)
	{
		if (!levelActor || !levelActor->Class) continue;
		if (levelActor->IsA(BaseClass->Name) && levelActor->IsBasedOn(Caller))
		{
			LogMessage("Checking actor: " + std::to_string((ptrdiff_t)levelActor) + " class: " + (levelActor->Class ? levelActor->Class->Name.ToString() : "NULL"));
			BasedActors.push_back(levelActor);
		}
	}

	iterator = BasedActors.begin();
}

bool BasedActorsIterator::Next()
{
	if (iterator == BasedActors.end())
	{
		*Actor = nullptr;
		return false;
	}

	*Actor = *iterator;
	iterator++;

	return *Actor;
}

/////////////////////////////////////////////////////////////////////////////

ChildActorsIterator::ChildActorsIterator(UActor* Caller, UObject* BaseClass, UObject** Actor) : BaseClass(BaseClass), Actor(Actor)
{
	for (UActor* levelActor : Caller->ChildActors)
	{
		if (levelActor->IsA(BaseClass->Name))
			ChildActors.push_back(levelActor);
	}

	iterator = ChildActors.begin();
}

bool ChildActorsIterator::Next()
{
	if (iterator == ChildActors.end())
	{
		*Actor = nullptr;
		return false;
	}

	*Actor = *iterator;
	iterator++;

	return *Actor;
}

/////////////////////////////////////////////////////////////////////////////
CycleActorsIterator::CycleActorsIterator(UObject* BaseClass, UObject** Actor, int* outIndex)  : BaseClass(BaseClass), Actor(Actor), outIndex(outIndex)  
{  
    for (UActor* levelActor : engine->Level->Actors)  
    {  
        if (levelActor && levelActor->IsA(BaseClass->Name))  
            matchedActors.push_back(levelActor); 
    }  
    totalActors = matchedActors.size();  
}  

bool CycleActorsIterator::Next()  
{  
    if (matchedActors.empty()) return false;  
    if (currentIndex >= matchedActors.size())  
    {  
        return false;  
    }  
    *Actor = matchedActors[currentIndex];  
    if (outIndex) *outIndex = static_cast<int>(currentIndex);  
    ++currentIndex;  
    return true;  
}

/////////////////////////////////////////////////////////////////////////////

RadiusActorsIterator::RadiusActorsIterator(UActor* Caller, UObject* BaseClass, UObject** Actor, float Radius, vec3 Location) : BaseClass(BaseClass), Actor(Actor), Radius(Radius), Location(Location)
{
	for (UActor* levelActor : engine->Level->Actors)
	{
		if (levelActor && levelActor->IsA(BaseClass->Name) && length(levelActor->Location() - Location) <= Radius)
			RadiusActors.push_back(levelActor);
	}

	iterator = RadiusActors.begin();
}

bool RadiusActorsIterator::Next()
{
	if (iterator == RadiusActors.end())
	{
		*Actor = nullptr;
		return false;
	}

	*Actor = *iterator;
	iterator++;

	return *Actor;
}

/////////////////////////////////////////////////////////////////////////////

TouchingActorsIterator::TouchingActorsIterator(UActor* Caller, UObject* BaseClass, UObject** outActor) : BaseClass(BaseClass), outActor(outActor)
{
	CollisionHitList hitList = Caller->XLevel()->Collision.OverlapTest(Caller->Location(), Caller->CollisionHeight(), Caller->CollisionRadius(), true, false, false);
	for (auto& hit : hitList)
	{
		// Only allow the Actors of type BaseClass
		if (hit.Actor && hit.Actor->IsA(BaseClass->Name))
			TouchingActors.push_back(hit.Actor);
	}

	iterator = TouchingActors.begin();
}

bool TouchingActorsIterator::Next()
{
	if (iterator == TouchingActors.end())
	{
		*outActor = nullptr;
		return false;
	}
	
	*outActor = *iterator;
	iterator++;

	return *outActor;
}

/////////////////////////////////////////////////////////////////////////////

TraceActorsIterator::TraceActorsIterator(UObject* BaseClass, UObject** Actor, vec3* HitLoc, vec3* HitNorm, const vec3& End, const vec3& Start, const vec3& Extent) : BaseClass(BaseClass), Actor(Actor), HitLoc(HitLoc), HitNorm(HitNorm), End(End), Start(Start), Extent(Extent)
{
	UActor* BaseActor = UObject::TryCast<UActor>(BaseClass);
	if(!BaseActor) return; 
	vec3 startPoint = Start;

	UActor* tracedActor = UObject::TryCast<UActor>(BaseActor->Trace(*HitLoc, *HitNorm, End, startPoint, true, Extent));

	do {
		if(!tracedActor || !tracedActor->Class)
		{
			continue;
		}	
		else
		{
			// Only allow the Actors of type BaseClass
			if (tracedActor->IsA(BaseClass->Name))
				tracedActors.push_back({ tracedActor, *HitLoc, *HitNorm });
			startPoint = *HitLoc;	// Make hit location the start point for the next trace
			tracedActor = UObject::TryCast<UActor>(tracedActor->Trace(*HitLoc, *HitNorm, End, startPoint, true, Extent));
		}
	} while (tracedActor);

	iterator = tracedActors.begin();
}

bool TraceActorsIterator::Next()
{
	if (iterator == tracedActors.end() || tracedActors.empty())
	{
		*Actor = nullptr;
		*HitLoc = vec3(0.0f);
		*HitNorm = vec3(0.0f);
		return false;
	}

	*Actor = iterator->tracedActor;
	*HitLoc = iterator->HitLoc;
	*HitNorm = iterator->HitNorm;
	iterator++;

	return *Actor;
}

/////////////////////////////////////////////////////////////////////////////

VisibleActorsIterator::VisibleActorsIterator(UActor* Caller, UObject* BaseClass, UObject** Actor, float Radius, const vec3& Location) : BaseClass(BaseClass), Actor(Actor), Radius(Radius), Location(Location)
{
	for (auto levelActor : engine->Level->Actors)
	{
		// Our checks:
		// * Whether the actor we're dealing with is not hidden and is the class of BaseClass
		// * Then whether the distance of the actor from our given Location is no more than Radius
		if (levelActor && !levelActor->bHidden() && levelActor->IsA(BaseClass->Name) &&
			length(levelActor->Location() - Location) <= Radius && Caller->FastTrace(levelActor->Location(), Location))
		{
			VisibleActors.push_back(levelActor);
		}
	}

	iterator = VisibleActors.begin();
}

bool VisibleActorsIterator::Next()
{
	if (iterator == VisibleActors.end())
	{
		*Actor = nullptr;
		return false;
	}

	*Actor = *iterator;
	iterator++;

	return *Actor;
}

/////////////////////////////////////////////////////////////////////////////

VisibleCollidingActorsIterator::VisibleCollidingActorsIterator(UObject* BaseClass, UObject** ReturnValue, float Radius, const vec3& Location, bool IgnoreHidden) : BaseClass(BaseClass), ReturnValue(ReturnValue), Radius(Radius), Location(Location), IgnoreHidden(IgnoreHidden)
{
	HitActors = engine->Level->Collision.CollidingActors(Location, Radius);
}

bool VisibleCollidingActorsIterator::Next()
{
	size_t size = HitActors.size();
	while (index < size)
	{
		UActor* actor = HitActors[index++];
		if (actor && (IgnoreHidden || !actor->bHidden()) && actor->IsA(BaseClass->Name))
		{
			*ReturnValue = actor;
			return true;
		}
	}
	*ReturnValue = nullptr;
	return false;
}

/////////////////////////////////////////////////////////////////////////////

ZoneActorsIterator::ZoneActorsIterator(UZoneInfo* zone, UObject* BaseClass, UObject** Actor) : Zone(zone), BaseClass(BaseClass), Actor(Actor)
{
	for (UActor* levelActor : engine->Level->Actors)
	{
		if (levelActor && levelActor->Region().Zone == zone && levelActor->IsA(BaseClass->Name))
		{
			ZoneActors.push_back(levelActor);
		}
	}

	iterator = ZoneActors.begin();
}

bool ZoneActorsIterator::Next()
{
	if (iterator == ZoneActors.end())
	{
		*Actor = nullptr;
		return false;
	}

	*Actor = *iterator;
	iterator++;

	return *Actor;
}
