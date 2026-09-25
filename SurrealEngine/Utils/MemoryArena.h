#pragma once

#include "Array.h"
#include <new>

class MemoryArena
{
public:
	explicit MemoryArena(size_t size) : capacity(size)
	{
		buffers.push_back(static_cast<char*>(::operator new(size)));
	}

	~MemoryArena()
	{
		Reset();
		for (auto buffer : buffers)
			::operator delete(buffer);
	}

	void* Allocate(size_t size, size_t alignment)
	{
		if (alignment > capacity)
			throw std::bad_alloc();

		while (true)
		{
			char* current_ptr = buffers[bufferIndex] + offset;
			size_t space = capacity - offset;
			void* aligned_ptr = current_ptr;
			if (std::align(alignment, size, aligned_ptr, space))
			{
				offset = static_cast<char*>(aligned_ptr) - buffers.back() + size;
				return aligned_ptr;
			}

			if (offset == 0)
				throw std::bad_alloc();

			if (bufferIndex + 1 == buffers.size())
				buffers.push_back(static_cast<char*>(::operator new(capacity)));
			offset = 0;
			bufferIndex++;
		}
	}

	template <typename T, typename... Args>
	T* Create(Args&&... args)
	{
		size_t oldBufferIndex = 0;
		size_t oldOffset = 0;
		void* ptr = Allocate(sizeof(T), alignof(T));
		if constexpr (std::is_trivially_destructible_v<T>)
		{
			return ptr;
		}
		else
		{
			try
			{
				void* dnode_ptr = Allocate(sizeof(DestructNode), alignof(DestructNode));
				T* obj = new (ptr) T(std::forward<Args>(args)...);
				auto dtor_call = [](void* p) { static_cast<T*>(p)->~T(); };
				tail = new (dnode_ptr) DestructNode{ dtor_call, tail, obj };
				return obj;
			}
			catch (...)
			{
				bufferIndex = oldBufferIndex;
				offset = oldOffset;
				throw;
			}
		}
	}

	void Reset()
	{
		CallDestructors();
		bufferIndex = 0;
		offset = 0;
	}

private:
	struct DestructNode
	{
		void (*dtor)(void*);
		DestructNode* prev;
		void* obj;
	};

	size_t capacity;
	size_t bufferIndex = 0;
	size_t offset = 0;
	Array<char*> buffers;
	DestructNode* tail = nullptr;

	void CallDestructors()
	{
		while (tail)
		{
			tail->dtor(tail->obj);
			tail = tail->prev;
		}
	}

	MemoryArena(const MemoryArena&) = delete;
	MemoryArena(MemoryArena&&) = delete;
};

template <typename T>
class ArenaAllocator
{
public:
	using value_type = T;

	explicit ArenaAllocator(MemoryArena* arena) noexcept : arena(arena) {}

	template <typename U>
	ArenaAllocator(const ArenaAllocator<U>& other) noexcept : arena(other.arena) {}

	template <typename U>
	friend class ArenaAllocator;

	T* allocate(std::size_t n)
	{
		return static_cast<T*>(arena->Allocate(n * sizeof(T), alignof(T)));
	}

	void deallocate(T* /*p*/, std::size_t /*n*/) noexcept
	{
		// No-op; arena manages memory lifetime
	}

	bool operator==(const ArenaAllocator& other) const noexcept
	{
		return arena == other.arena;
	}

	bool operator!=(const ArenaAllocator& other) const noexcept
	{
		return !(*this == other);
	}

private:
	MemoryArena* arena;
};
