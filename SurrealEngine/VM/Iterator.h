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

// As seen on Unreal Gold 227
class AllFilesIterator : public Iterator
{
public:
	// Iterates through all files of a type (Extensions are the usual Unreal Package extensions, or all files, if the FileExtension string is empty)
	AllFilesIterator(const std::string& FileExtension, const std::string& FilePrefix, std::string& FileName);
	bool Next() override;

private:
	std::string FileExtension;
	std::string FilePrefix;
	std::string& FileName;
	Array<std::string> FoundFiles;
	Array<std::string>::iterator iterator;
};

class BasedActorsIterator : public Iterator
{
public:
	BasedActorsIterator(UActor* Caller, UObject* BaseClass, UObject** Actor);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** Actor = nullptr;
	size_t index = 0;

	Array<UActor*> BasedActors;
	Array<UActor*>::iterator iterator;
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

	Array<UActor*> ChildActors;
	Array<UActor*>::iterator iterator;
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

	Array<UActor*> RadiusActors;
	Array<UActor*>::iterator iterator;
};

class TouchingActorsIterator : public Iterator
{
public:
	TouchingActorsIterator(UActor* Caller, UObject* BaseClass, UObject** outActor);
	bool Next() override;

	UObject* BaseClass = nullptr;
	UObject** outActor = nullptr;
	size_t index = 0;

	Array<UActor*> TouchingActors;
	Array<UActor*>::iterator iterator;
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

	Array<TraceInfo> tracedActors;
	Array<TraceInfo>::iterator iterator;
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

	Array<UActor*> VisibleActors;
	Array<UActor*>::iterator iterator;
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
	Array<UActor*> HitActors;
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

	Array<UActor*> ZoneActors;
	Array<UActor*>::iterator iterator;
};
