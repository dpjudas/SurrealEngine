#pragma once

#include "ExpressionValue.h"

class UZoneInfo;

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
	AllObjectsIterator(UObject* BaseClass, UObject** ReturnValue, std::string MatchTag);
	bool Next() override;

private:
	UObject* BaseClass = nullptr;
	UObject** ReturnValue = nullptr;
	std::string MatchTag;
	size_t index = 0;
};

class BasedActorsIterator : public Iterator
{
public:
	BasedActorsIterator(UObject* BaseClass, UObject** Actor);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	size_t index = 0;
};

class ChildActorsIterator : public Iterator
{
public:
	ChildActorsIterator(UObject* BaseClass, UObject** Actor);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	size_t index = 0;
};

class RadiusActorsIterator : public Iterator
{
public:
	RadiusActorsIterator(UObject* BaseClass, UObject** Actor, float Radius, vec3 Location);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	float Radius = 0.0f;
	vec3 Location;
	size_t index = 0;
};

class TouchingActorsIterator : public Iterator
{
public:
	TouchingActorsIterator(UObject* BaseClass, UObject** Actor);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	size_t index = 0;
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
};

class VisibleActorsIterator : public Iterator
{
public:
	VisibleActorsIterator(UObject* BaseClass, UObject** Actor, float Radius, const vec3& Location);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	float Radius = 0.0f;
	vec3 Location = vec3(0.0f);
	size_t index = 0;
};

class VisibleCollidingActorsIterator : public Iterator
{
public:
	VisibleCollidingActorsIterator(UObject* BaseClass, UObject** Actor, float Radius, const vec3& Location, bool IgnoreHidden);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	float Radius = 0.0f;
	vec3 Location = vec3(0.0f);
	bool IgnoreHidden = false;
	size_t index = 0;
};

class ZoneActorsIterator : public Iterator
{
public:
	ZoneActorsIterator(UZoneInfo* zone, UObject* BaseClass, UObject** Actor);
	bool Next() override;

	UZoneInfo* Zone = nullptr;
	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	size_t index = 0;
};
