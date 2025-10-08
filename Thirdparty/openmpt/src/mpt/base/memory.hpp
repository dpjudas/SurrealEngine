/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_MEMORY_HPP
#define MPT_BASE_MEMORY_HPP



#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/span.hpp"

#include <type_traits>

#include <cstddef>
#include <cstring>



namespace mpt {
inline namespace MPT_INLINE_NS {



using byte_span = mpt::span<std::byte>;
using const_byte_span = mpt::span<const std::byte>;



// Tell which types are safe for mpt::byte_cast.
// signed char is actually not allowed to alias into an object representation,
// which means that, if the actual type is not itself signed char but char or
// unsigned char instead, dereferencing the signed char pointer is undefined
// behaviour.
template <typename T>
struct is_byte_castable : public std::false_type { };
template <>
struct is_byte_castable<char> : public std::true_type { };
template <>
struct is_byte_castable<unsigned char> : public std::true_type { };
template <>
struct is_byte_castable<std::byte> : public std::true_type { };
template <>
struct is_byte_castable<const char> : public std::true_type { };
template <>
struct is_byte_castable<const unsigned char> : public std::true_type { };
template <>
struct is_byte_castable<const std::byte> : public std::true_type { };


template <typename T>
struct is_byte : public std::false_type { };
template <>
struct is_byte<std::byte> : public std::true_type { };
template <>
struct is_byte<const std::byte> : public std::true_type { };


template <typename T>
constexpr bool declare_binary_safe(const T &) noexcept {
	return false;
}

constexpr bool declare_binary_safe(const char &) noexcept {
	return true;
}
constexpr bool declare_binary_safe(const uint8 &) noexcept {
	return true;
}
constexpr bool declare_binary_safe(const int8 &) noexcept {
	return true;
}
constexpr bool declare_binary_safe(const std::byte &) noexcept {
	return true;
}

// Tell which types are safe to binary write into files.
// By default, no types are safe.
// When a safe type gets defined,
// also specialize this template so that IO functions will work.
template <typename T>
struct is_binary_safe : public std::conditional<declare_binary_safe(T{}), std::true_type, std::false_type>::type { };

// Generic Specialization for arrays.
template <typename T, std::size_t N>
struct is_binary_safe<T[N]> : public is_binary_safe<typename std::remove_const<T>::type> { };
template <typename T, std::size_t N>
struct is_binary_safe<const T[N]> : public is_binary_safe<typename std::remove_const<T>::type> { };
template <typename T, std::size_t N>
struct is_binary_safe<std::array<T, N>> : public is_binary_safe<typename std::remove_const<T>::type> { };
template <typename T, std::size_t N>
struct is_binary_safe<const std::array<T, N>> : public is_binary_safe<typename std::remove_const<T>::type> { };

template <typename T>
struct is_binary_safe<mpt::span<T>> : public is_binary_safe<typename std::remove_const<T>::type> { };
template <typename T>
struct is_binary_safe<const mpt::span<T>> : public is_binary_safe<typename std::remove_const<T>::type> { };


template <typename T>
constexpr bool check_binary_size(std::size_t size) noexcept {
	return true
		&& (sizeof(T) == size)
		&& (alignof(T) == 1)
		&& std::is_standard_layout<T>::value
#if !defined(MPT_LIBCXX_QUIRK_NO_HAS_UNIQUE_OBJECT_REPRESENTATIONS)
		&& std::has_unique_object_representations<T>::value
#endif
		&& mpt::is_binary_safe<T>::value;
}


template <typename Tdst, typename Tsrc>
struct byte_cast_impl {
	inline Tdst operator()(Tsrc src) const noexcept {
		static_assert(sizeof(Tsrc) == sizeof(std::byte));
		static_assert(sizeof(Tdst) == sizeof(std::byte));
		// not checking is_byte_castable here because we are actually
		// doing a static_cast and converting the value
		static_assert(std::is_integral<Tsrc>::value || mpt::is_byte<Tsrc>::value);
		static_assert(std::is_integral<Tdst>::value || mpt::is_byte<Tdst>::value);
		return static_cast<Tdst>(src);
	}
};

template <typename Tdst, typename Tsrc>
struct byte_cast_impl<mpt::span<Tdst>, mpt::span<Tsrc>> {
	inline mpt::span<Tdst> operator()(mpt::span<Tsrc> src) const noexcept {
		static_assert(sizeof(Tsrc) == sizeof(std::byte));
		static_assert(sizeof(Tdst) == sizeof(std::byte));
		static_assert(mpt::is_byte_castable<Tsrc>::value);
		static_assert(mpt::is_byte_castable<Tdst>::value);
		static_assert(std::is_integral<Tsrc>::value || mpt::is_byte<Tsrc>::value);
		static_assert(std::is_integral<Tdst>::value || mpt::is_byte<Tdst>::value);
		return mpt::as_span(mpt::byte_cast_impl<Tdst *, Tsrc *>()(src.data()), mpt::byte_cast_impl<Tdst *, Tsrc *>()(src.data() + src.size()));
	}
};

template <typename Tdst, typename Tsrc>
struct byte_cast_impl<Tdst *, Tsrc *> {
	inline Tdst * operator()(Tsrc * src) const noexcept {
		static_assert(sizeof(Tsrc) == sizeof(std::byte));
		static_assert(sizeof(Tdst) == sizeof(std::byte));
		static_assert(mpt::is_byte_castable<Tsrc>::value);
		static_assert(mpt::is_byte_castable<Tdst>::value);
		static_assert(std::is_integral<Tsrc>::value || mpt::is_byte<Tsrc>::value);
		static_assert(std::is_integral<Tdst>::value || mpt::is_byte<Tdst>::value);
		return reinterpret_cast<Tdst *>(src);
	}
};

template <typename Tdst, typename Tsrc>
struct void_cast_impl;

template <typename Tdst>
struct void_cast_impl<Tdst *, void *> {
	inline Tdst * operator()(void * src) const noexcept {
		static_assert(sizeof(Tdst) == sizeof(std::byte));
		static_assert(mpt::is_byte_castable<Tdst>::value);
		static_assert(std::is_integral<Tdst>::value || mpt::is_byte<Tdst>::value);
		return reinterpret_cast<Tdst *>(src);
	}
};

template <typename Tdst>
struct void_cast_impl<Tdst *, const void *> {
	inline Tdst * operator()(const void * src) const noexcept {
		static_assert(sizeof(Tdst) == sizeof(std::byte));
		static_assert(mpt::is_byte_castable<Tdst>::value);
		static_assert(std::is_integral<Tdst>::value || mpt::is_byte<Tdst>::value);
		return reinterpret_cast<Tdst *>(src);
	}
};

template <typename Tsrc>
struct void_cast_impl<void *, Tsrc *> {
	inline void * operator()(Tsrc * src) const noexcept {
		static_assert(sizeof(Tsrc) == sizeof(std::byte));
		static_assert(mpt::is_byte_castable<Tsrc>::value);
		static_assert(std::is_integral<Tsrc>::value || mpt::is_byte<Tsrc>::value);
		return reinterpret_cast<void *>(src);
	}
};

template <typename Tsrc>
struct void_cast_impl<const void *, Tsrc *> {
	inline const void * operator()(Tsrc * src) const noexcept {
		static_assert(sizeof(Tsrc) == sizeof(std::byte));
		static_assert(mpt::is_byte_castable<Tsrc>::value);
		static_assert(std::is_integral<Tsrc>::value || mpt::is_byte<Tsrc>::value);
		return reinterpret_cast<const void *>(src);
	}
};

// casts between different byte (char) types or pointers to these types
template <typename Tdst, typename Tsrc>
inline Tdst byte_cast(Tsrc src) noexcept {
	return byte_cast_impl<Tdst, Tsrc>()(src);
}

// casts between pointers to void and pointers to byte
template <typename Tdst, typename Tsrc>
inline Tdst void_cast(Tsrc src) noexcept {
	return void_cast_impl<Tdst, Tsrc>()(src);
}



template <typename T>
MPT_CONSTEXPRINLINE std::byte as_byte(T src) noexcept {
	static_assert(std::is_integral<T>::value);
	return static_cast<std::byte>(static_cast<uint8>(src));
}



template <typename T>
struct as_raw_memory_impl {
	inline mpt::const_byte_span operator()(const T & v) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<const std::byte *>(&v), sizeof(T));
	}
	inline mpt::byte_span operator()(T & v) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<std::byte *>(&v), sizeof(T));
	}
};

template <typename T, std::size_t N>
struct as_raw_memory_impl<T[N]> {
	inline mpt::const_byte_span operator()(const T (&v)[N]) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<const std::byte *>(v), N * sizeof(T));
	}
	inline mpt::byte_span operator()(T (&v)[N]) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<std::byte *>(v), N * sizeof(T));
	}
};

template <typename T, std::size_t N>
struct as_raw_memory_impl<const T[N]> {
	inline mpt::const_byte_span operator()(const T (&v)[N]) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<const std::byte *>(v), N * sizeof(T));
	}
};

template <typename T>
struct as_raw_memory_impl<mpt::span<T>> {
	inline mpt::const_byte_span operator()(const mpt::span<const T> & v) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<const std::byte *>(v.data()), v.size() * sizeof(T));
	}
	inline mpt::byte_span operator()(const mpt::span<T> & v) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<std::byte *>(v.data()), v.size() * sizeof(T));
	}
};

template <typename T>
struct as_raw_memory_impl<mpt::span<const T>> {
	inline mpt::const_byte_span operator()(const mpt::span<const T> & v) const {
		static_assert(mpt::is_binary_safe<typename std::remove_const<T>::type>::value);
		return mpt::as_span(reinterpret_cast<const std::byte *>(v.data()), v.size() * sizeof(T));
	}
};

// In order to be able to partially specialize it,
// as_raw_memory is implemented via a class template.
// Do not overload or specialize as_raw_memory directly.
// Using a wrapper (by default just around a cast to const std::byte *),
// allows for implementing raw memory access
// via on-demand generating a cached serialized representation.
template <typename T>
inline mpt::const_byte_span as_raw_memory(const T & v) {
	return mpt::as_raw_memory_impl<T>()(v);
}
template <typename T>
inline mpt::byte_span as_raw_memory(T & v) {
	return mpt::as_raw_memory_impl<T>()(v);
}



template <class T>
inline void memclear(T & x) {
	static_assert(std::is_standard_layout<T>::value);
	static_assert((std::is_trivially_default_constructible<T>::value && std::is_trivially_copyable<T>::value) || mpt::is_binary_safe<T>::value);
	std::memset(&x, 0, sizeof(T));
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_MEMORY_HPP
