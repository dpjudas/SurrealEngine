/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ARCH_FEATURE_FENCE_HPP
#define MPT_ARCH_FEATURE_FENCE_HPP


#include "mpt/base/detect.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"

#include <atomic>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace arch {



MPT_FORCEINLINE void feature_fence_aquire() noexcept {
	std::atomic_signal_fence(std::memory_order_acquire);
}

MPT_FORCEINLINE void feature_fence_release() noexcept {
	std::atomic_signal_fence(std::memory_order_release);
}

class feature_fence_guard {
public:
	MPT_FORCEINLINE feature_fence_guard() noexcept {
		mpt::arch::feature_fence_aquire();
	}
	MPT_FORCEINLINE ~feature_fence_guard() noexcept {
		mpt::arch::feature_fence_release();
	}
	feature_fence_guard(feature_fence_guard &&) = delete;
	feature_fence_guard(const feature_fence_guard &) = delete;
	feature_fence_guard & operator=(feature_fence_guard &&) = delete;
	feature_fence_guard & operator=(const feature_fence_guard &) = delete;
};



} // namespace arch



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ARCH_FEATURE_FENCE_HPP
