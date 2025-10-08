/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_ALLOC_HPP
#define MPT_BASE_ALLOC_HPP



#include "mpt/base/namespace.hpp"

#include "mpt/base/memory.hpp"
#include "mpt/base/span.hpp"

#include <iterator>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>



namespace mpt {
inline namespace MPT_INLINE_NS {



template <typename T>
inline mpt::span<T> as_span(std::vector<T> & cont) {
	return mpt::span<T>(cont.data(), cont.data() + cont.size());
}

template <typename T>
inline mpt::span<const T> as_span(const std::vector<T> & cont) {
	return mpt::span<const T>(cont.data(), cont.data() + cont.size());
}

template <typename T>
inline span<T> as_span(std::basic_string<T> & str) {
	return span<T>(str.data(), str.size());
}

template <typename T>
inline span<const T> as_span(const std::basic_string<T> & str) {
	return span<const T>(str.data(), str.size());
}



template <typename T>
inline std::vector<typename std::remove_const<T>::type> make_vector(T * beg, T * end) {
	return std::vector<typename std::remove_const<T>::type>(beg, end);
}

template <typename T>
inline std::vector<typename std::remove_const<T>::type> make_vector(T * data, std::size_t size) {
	return std::vector<typename std::remove_const<T>::type>(data, data + size);
}

template <typename T>
inline std::vector<typename std::remove_const<T>::type> make_vector(mpt::span<T> data) {
	return std::vector<typename std::remove_const<T>::type>(data.data(), data.data() + data.size());
}

template <typename T, std::size_t N>
inline std::vector<typename std::remove_const<T>::type> make_vector(T (&arr)[N]) {
	return std::vector<typename std::remove_const<T>::type>(std::begin(arr), std::end(arr));
}

template <typename T>
inline std::vector<typename std::remove_const<T>::type> make_vector(const std::basic_string<T> & str) {
	return std::vector<typename std::remove_const<T>::type>(str.begin(), str.end());
}



template <typename T>
inline std::basic_string<typename std::remove_const<T>::type> make_basic_string(T * beg, T * end) {
	return std::basic_string<typename std::remove_const<T>::type>(beg, end);
}

template <typename T>
inline std::basic_string<typename std::remove_const<T>::type> make_basic_string(T * data, std::size_t size) {
	return std::basic_string<typename std::remove_const<T>::type>(data, data + size);
}

template <typename T>
inline std::basic_string<typename std::remove_const<T>::type> make_basic_string(mpt::span<T> data) {
	return std::basic_string<typename std::remove_const<T>::type>(data.data(), data.data() + data.size());
}

template <typename T, std::size_t N>
inline std::basic_string<typename std::remove_const<T>::type> make_basic_string(T (&arr)[N]) {
	return std::basic_string<typename std::remove_const<T>::type>(std::begin(arr), std::end(arr));
}

template <typename T>
inline std::basic_string<typename std::remove_const<T>::type> make_basic_string(const std::vector<T> & str) {
	return std::vector<typename std::remove_const<T>::type>(str.begin(), str.end());
}



template <typename Tcont2, typename Tcont1>
inline Tcont1 & append(Tcont1 & cont1, const Tcont2 & cont2) {
	cont1.insert(cont1.end(), cont2.begin(), cont2.end());
	return cont1;
}

template <typename Tit2, typename Tcont1>
inline Tcont1 & append(Tcont1 & cont1, Tit2 beg, Tit2 end) {
	cont1.insert(cont1.end(), beg, end);
	return cont1;
}



template <typename Tdst, typename Tsrc>
struct buffer_cast_impl {
	inline Tdst operator()(const Tsrc & src) const {
		return Tdst(mpt::byte_cast<const typename Tdst::value_type *>(src.data()), mpt::byte_cast<const typename Tdst::value_type *>(src.data()) + src.size());
	}
};

// casts between vector<->string of byte-castable types
template <typename Tdst, typename Tsrc>
inline Tdst buffer_cast(const Tsrc & src) {
	return buffer_cast_impl<Tdst, Tsrc>()(src);
}



template <typename T>
struct as_raw_memory_impl<std::vector<T>> {
	inline mpt::const_byte_span operator()(const std::vector<T> & v) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<const std::byte *>(v.data()), v.size() * sizeof(T));
	}
	inline mpt::byte_span operator()(std::vector<T> & v) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<std::byte *>(v.data()), v.size() * sizeof(T));
	}
};

template <typename T>
struct as_raw_memory_impl<const std::vector<T>> {
	inline mpt::const_byte_span operator()(const std::vector<T> & v) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<const std::byte *>(v.data()), v.size() * sizeof(T));
	}
};



template <typename T>
class heap_value {
private:
	std::unique_ptr<T> m_value{};

public:
	template <typename... Targs>
	heap_value(Targs &&... args)
		: m_value(std::make_unique<T>(std::forward<Targs>(args)...)) {
		return;
	}
	const T & operator*() const {
		return *m_value;
	}
	T & operator*() {
		return *m_value;
	}
	const T * operator->() const {
		return m_value.get();
	}
	T * operator->() {
		return m_value.get();
	}
	const T * get() const {
		return m_value.get();
	}
	T * get() {
		return m_value.get();
	}
};



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_ALLOC_HPP
