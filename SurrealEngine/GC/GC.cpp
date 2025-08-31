
#include "Precomp.h"
#include "GC.h"

static GCRootNode* roots;
static GCAllocation* allocations;
static GCStats stats;

GCRootNode::GCRootNode()
{
	if (roots)
		roots->next = this;
	prev = roots;
	roots = this;
}

GCRootNode::~GCRootNode()
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

GCAllocation* GC::AllocMemory(size_t size)
{
	size_t memsize = sizeof(GCAllocation) + size;
	GCAllocation* allocation = (GCAllocation*)calloc(1, memsize);
	if (allocation == nullptr)
		throw std::bad_alloc();
	allocation->allocklistNext = allocations;
	allocation->memsize = memsize;
	allocation->unreferencedFlag = true;
	allocations = allocation;
	stats.numObjects++;
	stats.memoryUsage += memsize;
	return allocation;
}

void GC::FreeMemory(GCAllocation* allocation)
{
	free(allocation);
}

void GC::Collect()
{
	GCAllocation* marklist = nullptr;
	for (GCRootNode* root = roots; root != nullptr; root = root->next)
		marklist = GC::MarkObject(marklist, root->obj);

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

GCAllocation* GC::Mark(GCAllocation* marklist)
{
	GCAllocation* marklistout = nullptr;
	for (GCAllocation* allocation = marklist; allocation != nullptr; allocation = allocation->marklistNext)
	{
		allocation->object()->Mark(marklistout);
	}
	return marklistout;
}

void GC::Sweep()
{
	GCAllocation* prev = nullptr;
	GCAllocation* cur = allocations;
	while (cur)
	{
		if (cur->unreferencedFlag)
		{
			GCAllocation* unreferenced = cur;

			cur = cur->allocklistNext;
			if (prev)
				prev->allocklistNext = cur;
			else
				allocations = cur;

			stats.memoryUsage -= unreferenced->memsize;
			stats.numObjects--;

			unreferenced->object()->~GCObject();
			free(unreferenced);
		}
		else
		{
			cur->unreferencedFlag = true;
			prev = cur;
			cur = cur->allocklistNext;
		}
	}
}
