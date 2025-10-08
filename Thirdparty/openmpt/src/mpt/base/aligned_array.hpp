/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_ALIGNED_ARRAY_HPP
#define MPT_BASE_ALIGNED_ARRAY_HPP



#include "mpt/base/bit.hpp"
#include "mpt/base/namespace.hpp"

#include <array>
#include <memory>
#include <new>

#include <cassert>
#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {


template <typename T, std::size_t count, std::align_val_t alignment>
struct alignas(static_cast<std::size_t>(alignment)) aligned_array
	: std::array<T, count> {
	static_assert(static_cast<std::size_t>(alignment) >= alignof(T));
	static_assert(((count * sizeof(T)) % static_cast<std::size_t>(alignment)) == 0);
	static_assert(sizeof(std::array<T, count>) == (sizeof(T) * count));
};

static_assert(sizeof(mpt::aligned_array<float, 4, std::align_val_t{sizeof(float) * 4}>) == sizeof(std::array<float, 4>));



template <std::size_t alignment_elements, std::size_t expected_elements, typename T, std::size_t N>
T * align_elements(std::array<T, N> & a) {
	static_assert(mpt::has_single_bit(alignof(T)));
	static_assert(mpt::has_single_bit(sizeof(T)));
	static_assert(mpt::has_single_bit(alignment_elements));
	static_assert((expected_elements + alignment_elements - 1) <= N);
	void * buf = a.data();
	std::size_t size = N * sizeof(T);
	void * result = std::align(alignment_elements * sizeof(T), expected_elements * sizeof(T), buf, size);
	assert(result);
	return reinterpret_cast<T *>(result);
}

template <std::size_t alignment_elements, std::size_t expected_elements, typename T, std::size_t N>
T * align_elements(T (&a)[N]) {
	static_assert(mpt::has_single_bit(alignof(T)));
	static_assert(mpt::has_single_bit(sizeof(T)));
	static_assert(mpt::has_single_bit(alignment_elements));
	static_assert((expected_elements + alignment_elements - 1) <= N);
	void * buf = a;
	std::size_t size = N * sizeof(T);
	void * result = std::align(alignment_elements * sizeof(T), expected_elements * sizeof(T), buf, size);
	assert(result);
	return reinterpret_cast<T *>(result);
}



template <std::size_t alignment_bytes, std::size_t expected_elements, typename T, std::size_t N>
T * align_bytes(std::array<T, N> & a) {
	static_assert(mpt::has_single_bit(alignof(T)));
	static_assert(mpt::has_single_bit(sizeof(T)));
	static_assert(mpt::has_single_bit(alignment_bytes));
	static_assert((alignment_bytes % alignof(T)) == 0);
	static_assert(((expected_elements * sizeof(T)) + alignment_bytes - 1) <= (N * sizeof(T)));
	void * buf = a.data();
	std::size_t size = N * sizeof(T);
	void * result = std::align(alignment_bytes, expected_elements * sizeof(T), buf, size);
	assert(result);
	return reinterpret_cast<T *>(result);
}

template <std::size_t alignment_bytes, std::size_t expected_elements, typename T, std::size_t N>
T * align_bytes(T (&a)[N]) {
	static_assert(mpt::has_single_bit(alignof(T)));
	static_assert(mpt::has_single_bit(sizeof(T)));
	static_assert(mpt::has_single_bit(alignment_bytes));
	static_assert((alignment_bytes % alignof(T)) == 0);
	static_assert(((expected_elements * sizeof(T)) + alignment_bytes - 1) <= (N * sizeof(T)));
	void * buf = a;
	std::size_t size = N * sizeof(T);
	void * result = std::align(alignment_bytes, expected_elements * sizeof(T), buf, size);
	assert(result);
	return reinterpret_cast<T *>(result);
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_ALIGNED_ARRAY_HPP
