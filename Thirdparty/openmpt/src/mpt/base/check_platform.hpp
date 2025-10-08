/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_BASE_CHECK_PLATFORM_HPP
#define MPT_BASE_CHECK_PLATFORM_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/pointer.hpp"

#include <limits>

#include <cassert>
#include <cstddef>
#include <cstdint>

#if MPT_LIBC_DJGPP
#include <crt0.h>
#endif // MPT_LIBC_DJGPP


namespace mpt {
inline namespace MPT_INLINE_NS {


static_assert(sizeof(std::uintptr_t) == sizeof(void *));
static_assert(std::numeric_limits<unsigned char>::digits == 8);

static_assert(sizeof(char) == 1);

static_assert(sizeof(std::byte) == 1);
static_assert(alignof(std::byte) == 1);

static_assert(mpt::arch_bits == static_cast<int>(mpt::pointer_size) * 8);


#if MPT_LIBC_DJGPP

namespace platform {

namespace detail {

struct libc_checker {
private:
	bool m_libc_check_failure = false;
	bool m_libc_has_implicit_code_locking = false;
public:
	[[nodiscard]] inline libc_checker() {
		assert(((_crt0_startup_flags & _CRT0_FLAG_NONMOVE_SBRK) == _CRT0_FLAG_NONMOVE_SBRK) && ((_crt0_startup_flags & _CRT0_FLAG_UNIX_SBRK) == 0));
		if (((_crt0_startup_flags & _CRT0_FLAG_NONMOVE_SBRK) == _CRT0_FLAG_NONMOVE_SBRK) && ((_crt0_startup_flags & _CRT0_FLAG_UNIX_SBRK) != 0)) {
			m_libc_check_failure = true;
		}
		assert((_crt0_startup_flags & _CRT0_DISABLE_SBRK_ADDRESS_WRAP) == _CRT0_DISABLE_SBRK_ADDRESS_WRAP);
		if ((_crt0_startup_flags & _CRT0_DISABLE_SBRK_ADDRESS_WRAP) != _CRT0_DISABLE_SBRK_ADDRESS_WRAP) {
			m_libc_check_failure = true;
		}
		if ((_crt0_startup_flags & _CRT0_FLAG_LOCK_MEMORY) == _CRT0_FLAG_LOCK_MEMORY) {
			m_libc_has_implicit_code_locking = true;
		}
	}
	libc_checker(const libc_checker &) = delete;
	libc_checker & operator=(const libc_checker &) = delete;
	[[nodiscard]] inline bool is_ok() const noexcept {
		return !m_libc_check_failure;
	}
	[[nodiscard]] inline bool has_implicit_code_locking() const noexcept {
		return m_libc_has_implicit_code_locking;
	}
};

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif // MPT_COMPILER_CLANG
inline const libc_checker g_libc_checker;
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG

} // namespace detail

[[nodiscard]] inline const mpt::platform::detail::libc_checker & libc() noexcept {
	return mpt::platform::detail::g_libc_checker;
}

} // namespace platform

#endif // MPT_LIBC_DJGPP


} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_BASE_CHECK_PLATFORM_HPP
