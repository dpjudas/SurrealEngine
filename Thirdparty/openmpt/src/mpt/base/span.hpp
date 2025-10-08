/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_SPAN_HPP
#define MPT_BASE_SPAN_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"

#include <array>
#if MPT_CXX_AT_LEAST(20)
#include <span>
#else // !C++20
#include <iterator>
#include <limits>
#include <type_traits>
#endif // C++20

#if MPT_CXX_BEFORE(20)
#include <cstddef>
#endif // !C++20



namespace mpt {
inline namespace MPT_INLINE_NS {


#if MPT_CXX_AT_LEAST(20)

using std::dynamic_extent;
using std::span;

#else // !C++20

//  Simplified version of gsl::span.
//  Non-owning read-only or read-write view into a contiguous block of T
// objects, i.e. equivalent to a (beg,end) or (data,size) tuple.
//  Can eventually be replaced without further modifications with a full C++20
// std::span.

inline constexpr std::size_t dynamic_extent = std::numeric_limits<std::size_t>::max();

template <typename T>
class span {

public:
	using element_type = T;
	using value_type = typename std::remove_cv<T>::type;
	using index_type = std::size_t;
	using pointer = T *;
	using const_pointer = const T *;
	using reference = T &;
	using const_reference = const T &;

	using iterator = pointer;

	using difference_type = typename std::iterator_traits<iterator>::difference_type;

private:
	T * m_data;
	std::size_t m_size;

public:
	span() noexcept
		: m_data(nullptr)
		, m_size(0) {
	}

	span(pointer beg, pointer end)
		: m_data(beg)
		, m_size(end - beg) {
	}

	span(pointer data, index_type size)
		: m_data(data)
		, m_size(size) {
	}

	template <std::size_t N>
	span(element_type (&arr)[N])
		: m_data(arr)
		, m_size(N) {
	}

	template <std::size_t N>
	span(std::array<value_type, N> & arr)
		: m_data(arr.data())
		, m_size(arr.size()) {
	}

	template <std::size_t N>
	span(const std::array<value_type, N> & arr)
		: m_data(arr.data())
		, m_size(arr.size()) {
	}

	span(const span & other) noexcept = default;

	template <typename U>
	span(const span<U> & other)
		: m_data(other.data())
		, m_size(other.size()) {
	}

	span & operator=(const span & other) noexcept = default;

	iterator begin() const {
		return iterator(m_data);
	}

	iterator end() const {
		return iterator(m_data + m_size);
	}

	reference operator[](index_type index) {
		return m_data[index];
	}

	const_reference operator[](index_type index) const {
		return m_data[index];
	}

	pointer data() const noexcept {
		return m_data;
	}

	bool empty() const noexcept {
		return size() == 0;
	}

	index_type size() const noexcept {
		return m_size;
	}

	index_type length() const noexcept {
		return size();
	}

	span subspan(std::size_t offset, std::size_t count = mpt::dynamic_extent) const {
		return span(data() + offset, (count == mpt::dynamic_extent) ? (size() - offset) : count);
	}

	span first(std::size_t count) const {
		return span(data(), count);
	}

	span last(std::size_t count) const {
		return span(data() + (size() - count), count);
	}

}; // class span

#endif // C++20

template <typename T>
inline span<T> as_span(T * beg, T * end) {
	return span<T>(beg, end);
}

template <typename T>
inline span<T> as_span(T * data, std::size_t size) {
	return span<T>(data, size);
}

template <typename T, std::size_t N>
inline span<T> as_span(T (&arr)[N]) {
	return span<T>(std::begin(arr), std::end(arr));
}

template <typename T, std::size_t N>
inline span<T> as_span(std::array<T, N> & cont) {
	return span<T>(cont);
}

template <typename T, std::size_t N>
inline span<const T> as_span(const std::array<T, N> & cont) {
	return span<const T>(cont);
}

template <typename Ca, typename Cb>
bool span_elements_equal(const Ca & a, const Cb & b) {
	return a.size() == b.size() && (a.data() == b.data() || std::equal(a.begin(), a.end(), b.begin()));
}


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_SPAN_HPP
