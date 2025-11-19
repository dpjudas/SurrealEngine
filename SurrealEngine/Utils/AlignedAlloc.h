#pragma once

#include <cstdlib>

inline void* AlignedAlloc(std::size_t alignment, std::size_t size)
{
#ifdef _MSC_VER
	return _aligned_malloc(size, alignment);
#else
	return std::aligned_alloc(alignment, size);
#endif
}

inline void AlignedFree(void* data)
{
#ifdef _MSC_VER
	_aligned_free(data);
#else
	std::free(data);
#endif
}
