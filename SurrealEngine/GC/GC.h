#pragma once

#include <vector>
#include <list>
#include <memory>

struct GCAllocation;
class GCObjectList;

class GCObject
{
protected:
	virtual ~GCObject() = default;
	virtual GCAllocation* Mark(GCAllocation* marklist) = 0;

	GCAllocation* Allocation();
	friend class GC;
};

struct GCAllocation
{
	GCObject* object() { return reinterpret_cast<GCObject*>(this + 1); }
	uint64_t unreferencedFlag : 1;
	uint64_t memsize : 63;
	GCAllocation* marklistNext;
	GCAllocation* allocklistNext;
};

struct GCStats
{
	size_t numObjects = 0;
	size_t memoryUsage = 0;
};

class GCRootNode
{
public:
	GCRootNode();
	~GCRootNode();

	void set(GCObject* value) { obj = value; }
	GCObject* get() const { return obj; }

private:
	GCObject* obj = nullptr;
	GCRootNode* prev = nullptr;
	GCRootNode* next = nullptr;

	GCRootNode(const GCRootNode&) = delete;
	GCRootNode& operator=(const GCRootNode&) = delete;
	friend class GC;
};

template<typename T>
class GCRoot
{
public:
	GCRoot() : node(std::make_unique<GCRootNode>()) {}
	GCRoot(T* value) : GCRoot() { set(value); }

	void set(T* value) { node->set(value); }
	T* get() const { return static_cast<T*>(node->get()); }

	explicit operator bool() const { return get(); }
	T* operator->() { return get(); }
	const T* operator->() const { return get(); }

private:
	std::unique_ptr<GCRootNode> node;
};

class GC
{
public:
	template<typename T, typename... Args>
	static T* Alloc(Args&&... args)
	{
		GCAllocation* alloc = AllocMemory(sizeof(T));
		try
		{
			return new((unsigned char*)alloc->object()) T(std::forward<Args>(args)...);
		}
		catch (...)
		{
			FreeMemory(alloc);
			throw;
		}
	}

	static void Collect();
	static GCStats GetStats();
	static GCObjectList GetObjects();

	static GCAllocation* MarkObject(GCAllocation* marklist, GCObject* obj);

private:
	static GCAllocation* GetAllocations();
	static GCAllocation* AllocMemory(size_t size);
	static void FreeMemory(GCAllocation* allocation);
	static GCAllocation* Mark(GCAllocation* allocation);
	static void Sweep();

	friend class GCObjectList;
};

class GCObjectList
{
public:
	class iterator
	{
	public:
		iterator(GCAllocation* item) : item(item) {}

		GCObject* operator*() const { return item->object(); }
		iterator operator++() { return item->allocklistNext; }

		bool operator==(const iterator& other) const { return item == other.item; }

	private:
		GCAllocation* item;
	};

	iterator begin() { return GC::GetAllocations(); }
	iterator end() { return nullptr; }
};

inline GCObjectList GC::GetObjects() { return {}; }

inline GCAllocation* GC::MarkObject(GCAllocation* marklist, GCObject* obj)
{
	if (obj)
	{
		GCAllocation* allocation = ((GCAllocation*)obj) - 1;
		if (allocation->unreferencedFlag)
		{
			allocation->unreferencedFlag = false;
			allocation->marklistNext = marklist;
			marklist = allocation;
		}
	}
	return marklist;
}

inline GCAllocation* GCObject::Allocation() { return reinterpret_cast<GCAllocation*>(this) - 1; }
