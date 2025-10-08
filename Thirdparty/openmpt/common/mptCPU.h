/*
 * mptCPU.h
 * --------
 * Purpose: CPU feature detection.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mpt/base/macros.hpp"
#ifdef MPT_ENABLE_ARCH_INTRINSICS
#include "mpt/arch/arch.hpp"
#endif


OPENMPT_NAMESPACE_BEGIN


namespace CPU
{


#ifdef MPT_ENABLE_ARCH_INTRINSICS


#if defined(MODPLUG_TRACKER) && !defined(MPT_BUILD_WINESUPPORT)


namespace detail
{

inline MPT_CONSTINIT mpt::arch::current::feature_flags EnabledFeatures;
inline MPT_CONSTINIT mpt::arch::current::mode_flags EnabledModes;

} // namespace detail

inline void EnableAvailableFeatures() noexcept
{
	CPU::detail::EnabledFeatures = mpt::arch::get_cpu_info().get_features();
	CPU::detail::EnabledModes = mpt::arch::get_cpu_info().get_modes();
}

// enabled processor features for inline asm and intrinsics
[[nodiscard]] MPT_FORCEINLINE mpt::arch::current::feature_flags GetEnabledFeatures() noexcept
{
	return CPU::detail::EnabledFeatures;
}
[[nodiscard]] MPT_FORCEINLINE mpt::arch::current::mode_flags GetEnabledModes() noexcept
{
	return CPU::detail::EnabledModes;
}

struct Info
{
	[[nodiscard]] MPT_FORCEINLINE bool HasFeatureSet(mpt::arch::current::feature_flags features) const noexcept
	{
		return features == (GetEnabledFeatures() & features);
	}
	[[nodiscard]] MPT_FORCEINLINE bool HasModesEnabled(mpt::arch::current::mode_flags modes) const noexcept
	{
		return modes == (GetEnabledModes() & modes);
	}
};


#else // !MODPLUG_TRACKER


struct Info
{
private:
	const mpt::arch::flags_cache m_flags{mpt::arch::get_cpu_info()};
public:
	[[nodiscard]] MPT_FORCEINLINE bool HasFeatureSet(mpt::arch::current::feature_flags features) const noexcept
	{
		return m_flags[features];
	}
	[[nodiscard]] MPT_FORCEINLINE bool HasModesEnabled(mpt::arch::current::mode_flags modes) const noexcept
	{
		return m_flags[modes];
	}
};


#endif // MODPLUG_TRACKER


namespace feature = mpt::arch::current::feature;
namespace mode = mpt::arch::current::mode;

[[nodiscard]] MPT_FORCEINLINE bool HasFeatureSet(mpt::arch::current::feature_flags features) noexcept
{
	return CPU::Info{}.HasFeatureSet(features);
}

[[nodiscard]] MPT_FORCEINLINE bool HasModesEnabled(mpt::arch::current::mode_flags modes) noexcept
{
	return CPU::Info{}.HasModesEnabled(modes);
}


#endif // MPT_ENABLE_ARCH_INTRINSICS


} // namespace CPU


OPENMPT_NAMESPACE_END
