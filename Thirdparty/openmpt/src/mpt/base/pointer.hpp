/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_POINTER_HPP
#define MPT_BASE_POINTER_HPP



#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"

#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {


inline constexpr int arch_bits = sizeof(void *) * 8;
inline constexpr std::size_t pointer_size = sizeof(void *);


template <typename Tdst, typename Tsrc>
struct pointer_cast_helper {
	static constexpr Tdst cast(const Tsrc & src) noexcept {
		return src;
	}
};

template <typename Tdst, typename Tptr>
struct pointer_cast_helper<Tdst, const Tptr *> {
	static constexpr Tdst cast(const Tptr * const & src) noexcept {
		return reinterpret_cast<const Tdst>(src);
	}
};
template <typename Tdst, typename Tptr>
struct pointer_cast_helper<Tdst, Tptr *> {
	static constexpr Tdst cast(const Tptr * const & src) noexcept {
		return reinterpret_cast<const Tdst>(src);
	}
};


template <typename Tdst, typename Tsrc>
constexpr Tdst pointer_cast(const Tsrc & src) noexcept {
	return pointer_cast_helper<Tdst, Tsrc>::cast(src);
}


template <typename T>
class void_ptr {
private:
	T * m_ptr = nullptr;
public:
	MPT_FORCEINLINE explicit void_ptr(void * ptr)
		: m_ptr(reinterpret_cast<T *>(ptr)) {
		return;
	}
	MPT_FORCEINLINE T & operator*() {
		return *m_ptr;
	}
	MPT_FORCEINLINE T * operator->() {
		return m_ptr;
	}
	MPT_FORCEINLINE operator void *() {
		return m_ptr;
	}
};


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_POINTER_HPP
