/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ARCH_ARCH_HPP
#define MPT_ARCH_ARCH_HPP


#include "mpt/arch/feature_fence.hpp"
#include "mpt/arch/feature_flags.hpp"
#include "mpt/arch/x86_amd64.hpp"
#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"

#include <string>



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace arch {



namespace unknown {

template <typename tag>
struct no_flags {
	constexpr no_flags() noexcept = default;
	constexpr no_flags operator~() noexcept {
		return no_flags{};
	}
	constexpr no_flags & operator&=(no_flags) noexcept {
		return *this;
	}
	constexpr no_flags & operator|=(no_flags) noexcept {
		return *this;
	}
	constexpr no_flags & operator^=(no_flags) noexcept {
		return *this;
	}
	friend constexpr no_flags operator&(no_flags, no_flags) noexcept {
		return no_flags{};
	}
	friend constexpr no_flags operator|(no_flags, no_flags) noexcept {
		return no_flags{};
	}
	friend constexpr no_flags operator^(no_flags, no_flags) noexcept {
		return no_flags{};
	}
	friend constexpr bool operator==(no_flags, no_flags) noexcept {
		return true;
	}
	friend constexpr bool operator!=(no_flags, no_flags) noexcept {
		return false;
	}
	constexpr bool supports(no_flags) noexcept {
		return true;
	}
	explicit constexpr operator bool() const noexcept {
		return true;
	}
	constexpr bool operator!() const noexcept {
		return false;
	}
};

struct no_feature_flags_tag {
};

struct no_mode_flags_tag {
};

using feature_flags = mpt::arch::basic_feature_flags<no_flags<no_feature_flags_tag>>;
using mode_flags = mpt::arch::basic_feature_flags<no_flags<no_mode_flags_tag>>;

namespace feature {
inline constexpr feature_flags none = feature_flags{};
} // namespace feature

namespace mode {
inline constexpr mode_flags none = mode_flags{};
} // namespace mode

struct cpu_info {
public:
	cpu_info() = default;
public:
	MPT_CONSTEXPRINLINE bool operator[](feature_flags) const noexcept {
		return true;
	}
	MPT_CONSTEXPRINLINE bool has_features(feature_flags) const noexcept {
		return true;
	}
	MPT_CONSTEXPRINLINE feature_flags get_features() const noexcept {
		return {};
	}
	MPT_CONSTEXPRINLINE bool operator[](mode_flags) const noexcept {
		return true;
	}
	MPT_CONSTEXPRINLINE bool enabled_modes(mode_flags) const noexcept {
		return true;
	}
	MPT_CONSTEXPRINLINE mode_flags get_modes() const noexcept {
		return {};
	}
};

[[nodiscard]] MPT_CONSTEVAL feature_flags assumed_features() noexcept {
	return {};
}

[[nodiscard]] MPT_CONSTEVAL mode_flags assumed_modes() noexcept {
	return {};
}

} // namespace unknown



#if MPT_ARCH_X86
namespace current = x86;
#elif MPT_ARCH_AMD64
namespace current = amd64;
#else
namespace current = unknown;
#endif

using cpu_info = mpt::arch::current::cpu_info;

inline const cpu_info & get_cpu_info() {
	static cpu_info info;
	return info;
}

namespace detail {

struct info_initializer {
	inline info_initializer() noexcept {
		get_cpu_info();
	}
};

#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wglobal-constructors"
#endif // MPT_COMPILER_CLANG
inline info_initializer g_info_initializer;
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG


} // namespace detail

struct flags_cache {
private:
	const mpt::arch::current::feature_flags Features;
	const mpt::arch::current::mode_flags Modes;
public:
	MPT_CONSTEXPRINLINE flags_cache(const mpt::arch::cpu_info & info) noexcept
		: Features(info.get_features())
		, Modes(info.get_modes()) {
		return;
	}
	[[nodiscard]] MPT_CONSTEXPRINLINE bool operator[](mpt::arch::current::feature_flags query_features) const noexcept {
		return ((Features & query_features) == query_features);
	}
	[[nodiscard]] MPT_CONSTEXPRINLINE bool has_features(mpt::arch::current::feature_flags query_features) const noexcept {
		return ((Features & query_features) == query_features);
	}
	[[nodiscard]] MPT_CONSTEXPRINLINE mpt::arch::current::feature_flags get_features() const noexcept {
		return Features;
	}
	[[nodiscard]] MPT_CONSTEXPRINLINE bool operator[](mpt::arch::current::mode_flags query_modes) const noexcept {
		return ((Modes & query_modes) == query_modes);
	}
	[[nodiscard]] MPT_CONSTEXPRINLINE bool has_features(mpt::arch::current::mode_flags query_modes) const noexcept {
		return ((Modes & query_modes) == query_modes);
	}
	[[nodiscard]] MPT_CONSTEXPRINLINE mpt::arch::current::mode_flags get_modes() const noexcept {
		return Modes;
	}
};



} // namespace arch



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ARCH_ARCH_HPP
