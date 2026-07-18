
#include "Precomp.h"
#include "GC.h"

static GCRootNode* roots;
static GCAllocation* allocations;
static GCStats stats;

GCRootNode::GCRootNode()
{
	// Insert at the head of the list. `roots` is the head, nodes chain head->tail via `next`, and both
	// ~GCRootNode() and GC::Collect() traverse from `roots` via `next` - so insertion has to build the list
	// in that same direction. The previous version linked the other way (`roots->next = this; prev = roots`),
	// which left `roots` pointing at the newest node while its `next` was null: Collect() then only ever
	// marked that one node, and destroying it (the common case for a short-lived GCRoot, e.g. the per-frame
	// one in VRHands::Tick) took the dtor's `if (prev)` branch without updating `roots`, leaving the global
	// head dangling at freed memory. The next GCRootNode() then wrote `roots->next = this` into that freed
	// block, corrupting the heap.
	next = roots;
	prev = nullptr;
	if (roots)
		roots->prev = this;
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

GCAllocation* GC::GetAllocations()
{
	return allocations;
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
