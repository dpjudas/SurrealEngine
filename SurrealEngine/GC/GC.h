#pragma once

#include <vector>
#include <list>

struct GCTypeMember
{
	size_t offset;
};

struct GCType
{
	size_t size;
	std::vector<GCTypeMember> members;
};

struct GCAllocation
{
	union
	{
		GCType* type;
		size_t flags;
	};
	size_t count;
	GCAllocation* marklistNext;
	GCAllocation* allocklistNext;
};

struct GCStats
{
	size_t numObjects;
	size_t memoryUsage;
};

class GCRoot
{
public:
	GCRoot();
	~GCRoot();

	void set(void* value) { obj = value; }
	void* get() const { return obj; }

private:
	void* obj = nullptr;
	GCRoot* prev = nullptr;
	GCRoot* next = nullptr;

	GCRoot(const GCRoot&) = delete;
	GCRoot& operator=(const GCRoot&) = delete;
	friend class GC;
};

class GC
{
public:
	static void* Alloc(GCType* type, size_t count = 1);
	static void Collect();
	static GCStats GetStats();

private:
	static GCAllocation* MarkData(GCAllocation* marklist, void* data);
	static GCAllocation* Mark(GCAllocation* allocation);
	static void Sweep();
};
