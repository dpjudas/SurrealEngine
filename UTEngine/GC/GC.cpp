
#include "Precomp.h"
#include "GC.h"

static GCRoot* roots;
static GCAllocation* allocations;
static GCStats stats;

#define GC_UNREFERENCED_FLAG ((size_t)1)

GCRoot::GCRoot()
{
	if (roots)
		roots->next = this;
	prev = roots;
	roots = this;
}

GCRoot::~GCRoot()
{
	if (prev)
	{
		prev->next = next;
	}
	else
	{
		roots = next;
	}

	if (next)
	{
		next->prev = prev;
	}
}

void* GC::Alloc(GCType* type, size_t count)
{
	size_t memsize = sizeof(GCAllocation) + type->size * count;
	GCAllocation* allocation = (GCAllocation*)calloc(1, memsize);
	if (allocation == nullptr)
		throw std::bad_alloc();
	allocation->allocklistNext = allocations;
	allocation->type = type;
	allocation->flags |= GC_UNREFERENCED_FLAG;
	allocation->count = count;
	allocations = allocation;
	stats.numObjects++;
	stats.memoryUsage += memsize;
	return allocation + 1;
}

void GC::Collect()
{
	GCAllocation* marklist = nullptr;
	for (GCRoot* root = roots; root != nullptr; root = root->next)
		marklist = MarkData(marklist, root->obj);

	while (marklist)
	{
		marklist = Mark(marklist);
	}

	Sweep();
}

GCStats GC::GetStats()
{
	return stats;
}

GCAllocation* GC::MarkData(GCAllocation* marklist, void* data)
{
	if (data)
	{
		GCAllocation* allocation = ((GCAllocation*)data) - 1;
		if (allocation->flags & GC_UNREFERENCED_FLAG)
		{
			allocation->flags &= ~GC_UNREFERENCED_FLAG;
			allocation->marklistNext = marklist;
			marklist = allocation;
		}
	}
	return marklist;
}

GCAllocation* GC::Mark(GCAllocation* marklist)
{
	GCAllocation* marklistout = nullptr;
	for (GCAllocation* allocation = marklist; allocation != nullptr; allocation = allocation->marklistNext)
	{
		GCType* type = allocation->type;

		size_t size = type->size;
		size_t count = allocation->count;

		for (const GCTypeMember& member : type->members)
		{
			uint8_t* d = (uint8_t*)(allocation + 1);
			for (size_t i = 0; i < count; i++)
			{
				void* memberptr = *(void**)(d + member.offset);
				marklistout = MarkData(marklistout, memberptr);
				d += size;
			}
		}
	}
	return marklistout;
}

void GC::Sweep()
{
	GCAllocation* prev = nullptr;
	GCAllocation* cur = allocations;
	while (cur)
	{
		if (cur->flags & GC_UNREFERENCED_FLAG)
		{
			GCAllocation* unreferenced = cur;
			unreferenced->flags &= ~GC_UNREFERENCED_FLAG;

			cur = cur->allocklistNext;
			if (prev)
				prev->allocklistNext = cur;
			else
				allocations = cur;

			stats.memoryUsage -= unreferenced->type->size * unreferenced->count;
			stats.numObjects--;
			free(unreferenced);
		}
		else
		{
			cur->flags |= GC_UNREFERENCED_FLAG;
			prev = cur;
			cur = cur->allocklistNext;
		}
	}
}
