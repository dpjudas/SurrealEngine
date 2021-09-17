
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
	engine->LogUnimplemented("Actor.BasedActors not implemented");
}

bool BasedActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

ChildActorsIterator::ChildActorsIterator(UObject* BaseClass, UObject** Actor) : BaseClass(BaseClass), Actor(Actor)
{
	engine->LogUnimplemented("Actor.ChildActors not implemented");
}

bool ChildActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

RadiusActorsIterator::RadiusActorsIterator(UObject* BaseClass, UObject** Actor, float Radius, vec3 Location) : BaseClass(BaseClass), Actor(Actor), Radius(Radius), Location(Location)
{
	engine->LogUnimplemented("Actor.RadiusActors not implemented");
}

bool RadiusActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

TouchingActorsIterator::TouchingActorsIterator(UObject* BaseClass, UObject** Actor) : BaseClass(BaseClass), Actor(Actor)
{
	engine->LogUnimplemented("Actor.TouchingActors not implemented");
}

bool TouchingActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

TraceActorsIterator::TraceActorsIterator(UObject* BaseClass, UObject** Actor, vec3* HitLoc, vec3* HitNorm, const vec3& End, const vec3& Start, const vec3& Extent) : BaseClass(BaseClass), Actor(Actor), HitLoc(HitLoc), HitNorm(HitNorm), End(End), Start(Start), Extent(Extent)
{
	engine->LogUnimplemented("Actor.TraceActors not implemented");
}

bool TraceActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

VisibleActorsIterator::VisibleActorsIterator(UObject* BaseClass, UObject** Actor, float Radius, const vec3& Location) : BaseClass(BaseClass), Actor(Actor), Radius(Radius), Location(Location)
{
	engine->LogUnimplemented("Actor.VisibleActor not implemented");
}

bool VisibleActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

VisibleCollidingActorsIterator::VisibleCollidingActorsIterator(UObject* BaseClass, UObject** Actor, float Radius, const vec3& Location, bool IgnoreHidden) : BaseClass(BaseClass), Actor(Actor), Radius(Radius), Location(Location), IgnoreHidden(IgnoreHidden)
{
	engine->LogUnimplemented("Actor.VisibleCollidingActors not implemented");
}

bool VisibleCollidingActorsIterator::Next()
{
	return false;
}

/////////////////////////////////////////////////////////////////////////////

ZoneActorsIterator::ZoneActorsIterator(UZoneInfo* zone, UObject* BaseClass, UObject** Actor) : Zone(zone), BaseClass(BaseClass), Actor(Actor)
{
	engine->LogUnimplemented("ZoneInfo.ZoneActors not implemented");
}

bool ZoneActorsIterator::Next()
{
	return false;
}
