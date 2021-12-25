
#include "Precomp.h"
#include "Iterator.h"
#include "Engine.h"
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
	return false;
}

/////////////////////////////////////////////////////////////////////////////

BasedActorsIterator::BasedActorsIterator(UObject* BaseClass, UObject** Actor) : BaseClass(BaseClass), Actor(Actor)
{
	engine->LogUnimplemented("Actor.BasedActors");
}

bool BasedActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

ChildActorsIterator::ChildActorsIterator(UObject* BaseClass, UObject** Actor) : BaseClass(BaseClass), Actor(Actor)
{
	engine->LogUnimplemented("Actor.ChildActors");
}

bool ChildActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

RadiusActorsIterator::RadiusActorsIterator(UObject* BaseClass, UObject** Actor, float Radius, vec3 Location) : BaseClass(BaseClass), Actor(Actor), Radius(Radius), Location(Location)
{
	engine->LogUnimplemented("Actor.RadiusActors");
}

bool RadiusActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

TouchingActorsIterator::TouchingActorsIterator(UObject* BaseClass, UObject** Actor) : BaseClass(BaseClass), Actor(Actor)
{
	engine->LogUnimplemented("Actor.TouchingActors");
}

bool TouchingActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

TraceActorsIterator::TraceActorsIterator(UObject* BaseClass, UObject** Actor, vec3* HitLoc, vec3* HitNorm, const vec3& End, const vec3& Start, const vec3& Extent) : BaseClass(BaseClass), Actor(Actor), HitLoc(HitLoc), HitNorm(HitNorm), End(End), Start(Start), Extent(Extent)
{
	engine->LogUnimplemented("Actor.TraceActors");
}

bool TraceActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

VisibleActorsIterator::VisibleActorsIterator(UObject* BaseClass, UObject** Actor, float Radius, const vec3& Location) : BaseClass(BaseClass), Actor(Actor), Radius(Radius), Location(Location)
{
	engine->LogUnimplemented("Actor.VisibleActor");
}

bool VisibleActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

VisibleCollidingActorsIterator::VisibleCollidingActorsIterator(UObject* BaseClass, UObject** ReturnValue, float Radius, const vec3& Location, bool IgnoreHidden) : BaseClass(BaseClass), ReturnValue(ReturnValue), Radius(Radius), Location(Location), IgnoreHidden(IgnoreHidden)
{
	HitActors = engine->Level->CollidingActors(Location, Radius);
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
	return false;
}

/////////////////////////////////////////////////////////////////////////////

ZoneActorsIterator::ZoneActorsIterator(UZoneInfo* zone, UObject* BaseClass, UObject** Actor) : Zone(zone), BaseClass(BaseClass), Actor(Actor)
{
	engine->LogUnimplemented("ZoneInfo.ZoneActors");
}

bool ZoneActorsIterator::Next()
{
	return false;
}
