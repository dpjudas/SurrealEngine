#pragma once

#include "ExpressionValue.h"

class UZoneInfo;
class UActor;

class Iterator
{
public:
	virtual ~Iterator() = default;
	virtual bool Next() = 0;

	size_t StartStatementIndex = 0;
	size_t EndStatementIndex = 0;
};

class AllObjectsIterator : public Iterator
{
public:
	AllObjectsIterator(UObject* BaseClass, UObject** ReturnValue, NameString MatchTag);
	bool Next() override;

private:
	UObject* BaseClass = nullptr;
	UObject** ReturnValue = nullptr;
	NameString MatchTag;
	size_t index = 0;
};

class BasedActorsIterator : public Iterator
{
public:
	BasedActorsIterator(UActor* Caller, UObject* BaseClass, UObject** Actor);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	size_t index = 0;
};

// An Iterator for iterating through all child actors of a given actor (e.g. due to being spawned by them)
class ChildActorsIterator : public Iterator
{
public:
	ChildActorsIterator(UActor* Caller, UObject* BaseClass, UObject** Actor);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	size_t index = 0;

	std::vector<UActor*> ChildActors;
	std::vector<UActor*>::iterator iterator;
};

class RadiusActorsIterator : public Iterator
{
public:
	RadiusActorsIterator(UActor* Caller, UObject* BaseClass, UObject** Actor, float Radius, vec3 Location);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	float Radius = 0.0f;
	vec3 Location;
	size_t index = 0;

	std::vector<UActor*> RadiusActors;
	std::vector<UActor*>::iterator iterator;
};

class TouchingActorsIterator : public Iterator
{
public:
	TouchingActorsIterator(UActor* Caller, UObject* BaseClass, UObject** outActor);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** outActor = nullptr;
	size_t index = 0;

	std::vector<UActor*> TouchingActors;
	std::vector<UActor*>::iterator iterator;
};

class TraceActorsIterator : public Iterator
{
public:
	TraceActorsIterator(UObject* BaseClass, UObject** Actor, vec3* HitLoc, vec3* HitNorm, const vec3& End, const vec3& Start, const vec3& Extent);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	vec3* HitLoc = nullptr;
	vec3* HitNorm = nullptr;
	vec3 End = vec3(0.0f);
	vec3 Start = vec3(0.0f);
	vec3 Extent = vec3(0.0f);
	size_t index = 0;

	struct TraceInfo
	{
		UActor* tracedActor;
		vec3 HitLoc;
		vec3 HitNorm;
	};

	std::vector<TraceInfo> tracedActors;
	std::vector<TraceInfo>::iterator iterator;
};

class VisibleActorsIterator : public Iterator
{
public:
	VisibleActorsIterator(UActor* Caller, UObject* BaseClass, UObject** Actor, float Radius, const vec3& Location);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	float Radius = 0.0f;
	vec3 Location = vec3(0.0f);
	size_t index = 0;

	std::vector<UActor*> VisibleActors;
	std::vector<UActor*>::iterator iterator;
};

class VisibleCollidingActorsIterator : public Iterator
{
public:
	VisibleCollidingActorsIterator(UObject* BaseClass, UObject** ReturnValue, float Radius, const vec3& Location, bool IgnoreHidden);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** ReturnValue = nullptr;
	float Radius = 0.0f;
	vec3 Location = vec3(0.0f);
	bool IgnoreHidden = false;
	size_t index = 0;
	std::vector<UActor*> HitActors;
};

// Iterator for iterating through all actors in a given Zone
class ZoneActorsIterator : public Iterator
{
public:
	ZoneActorsIterator(UZoneInfo* zone, UObject* BaseClass, UObject** Actor);
	bool Next() override;

	UZoneInfo* Zone = nullptr;
	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	size_t index = 0;

	std::vector<UActor*> ZoneActors;
	std::vector<UActor*>::iterator iterator;
};
