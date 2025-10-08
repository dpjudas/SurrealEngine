/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_ARCH_X86_AMD64_HPP
#define MPT_ARCH_X86_AMD64_HPP


#include "mpt/arch/feature_flags.hpp"
#include "mpt/base/bit.hpp"
#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/base/utility.hpp"
#include "mpt/osinfo/windows_version.hpp"

#include <algorithm>
#include <array>
#if MPT_ARCH_X86 || MPT_ARCH_AMD64
#if MPT_COMPILER_GCC
#include <atomic>
#endif
#endif
#include <optional>
#include <string>
#include <string_view>

#include <cstddef>

#if MPT_OS_DJGPP
#include <dpmi.h>
#include <pc.h>
#endif

#if MPT_OS_WINDOWS
#include <windows.h>
#endif

#if MPT_ARCH_X86 || MPT_ARCH_AMD64
#if MPT_COMPILER_MSVC
#include <intrin.h>
#endif
#if MPT_COMPILER_MSVC
#include <immintrin.h>
#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG
#include <cpuid.h>
#include <x86intrin.h>
#endif
#endif

// skip assumed features for amd64
#define MPT_ARCH_X86_AMD64_FAST_DETECT 0



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace arch {



namespace x86 {



using feature_flags = mpt::arch::basic_feature_flags<uint32>;


using mode_flags = mpt::arch::basic_feature_flags<uint8>;



// clang-format off

namespace feature {
inline constexpr feature_flags none           = feature_flags{};
inline constexpr feature_flags intel386       = feature_flags{ 0x0000'0001 };
inline constexpr feature_flags fpu            = feature_flags{ 0x0000'0002 };
inline constexpr feature_flags fsin           = feature_flags{ 0x0000'0004 };
inline constexpr feature_flags intel486       = feature_flags{ 0x0000'0008 };  // XADD, BSWAP, CMPXCHG
inline constexpr feature_flags cpuid          = feature_flags{ 0x0000'0010 };
inline constexpr feature_flags tsc            = feature_flags{ 0x0000'0020 };
inline constexpr feature_flags cx8            = feature_flags{ 0x0000'0040 };
inline constexpr feature_flags cmov           = feature_flags{ 0x0000'0080 };
inline constexpr feature_flags mmx            = feature_flags{ 0x0000'0100 };
inline constexpr feature_flags mmxext         = feature_flags{ 0x0000'0200 };
inline constexpr feature_flags x3dnow         = feature_flags{ 0x0000'0400 };
inline constexpr feature_flags x3dnowext      = feature_flags{ 0x0000'0800 };
inline constexpr feature_flags x3dnowprefetch = feature_flags{ 0x0000'1000 };
inline constexpr feature_flags fxsr           = feature_flags{ 0x0000'2000 };
inline constexpr feature_flags sse            = feature_flags{ 0x0000'4000 };
inline constexpr feature_flags sse2           = feature_flags{ 0x0000'8000 };
inline constexpr feature_flags sse3           = feature_flags{ 0x0001'0000 };
inline constexpr feature_flags ssse3          = feature_flags{ 0x0002'0000 };
inline constexpr feature_flags sse4_1         = feature_flags{ 0x0004'0000 };
inline constexpr feature_flags sse4_2         = feature_flags{ 0x0008'0000 };
inline constexpr feature_flags xsave          = feature_flags{ 0x0010'0000 };
inline constexpr feature_flags avx            = feature_flags{ 0x0020'0000 };
inline constexpr feature_flags avx2           = feature_flags{ 0x0040'0000 };
inline constexpr feature_flags cx16           = feature_flags{ 0x0080'0000 };
inline constexpr feature_flags lahf           = feature_flags{ 0x0100'0000 };
inline constexpr feature_flags popcnt         = feature_flags{ 0x0200'0000 };
inline constexpr feature_flags bmi1           = feature_flags{ 0x0400'0000 };
inline constexpr feature_flags bmi2           = feature_flags{ 0x0800'0000 };
inline constexpr feature_flags f16c           = feature_flags{ 0x1000'0000 };
inline constexpr feature_flags fma            = feature_flags{ 0x2000'0000 };
inline constexpr feature_flags lzcnt          = feature_flags{ 0x4000'0000 };
inline constexpr feature_flags movbe          = feature_flags{ 0x8000'0000 };
} // namespace feature

namespace mode {
inline constexpr mode_flags base      = mode_flags{ 0x00 };
inline constexpr mode_flags xmm128sse = mode_flags{ 0x01 };
inline constexpr mode_flags ymm256avx = mode_flags{ 0x02 };
} // namespace mode

namespace featureset {
inline constexpr feature_flags intel386        = feature::intel386;
inline constexpr feature_flags intel486SX      = featureset::intel386      | feature::intel486;
inline constexpr feature_flags intel486DX      = featureset::intel486SX    | feature::fpu | feature::fsin;
inline constexpr feature_flags intel586        = featureset::intel486DX    | feature::cpuid | feature::tsc | feature::cx8;
inline constexpr feature_flags intel586_mmx    = featureset::intel586      | feature::mmx;
inline constexpr feature_flags intel686        = featureset::intel586      | feature::cmov;
inline constexpr feature_flags intel686_mmx    = featureset::intel686      | feature::mmx;
inline constexpr feature_flags intel686_sse    = featureset::intel686_mmx  | feature::mmxext | feature::fxsr | feature::sse;
inline constexpr feature_flags intel686_sse2   = featureset::intel686_sse  | feature::sse2;
inline constexpr feature_flags intel786        = featureset::intel686_sse2;
inline constexpr feature_flags amd64           = featureset::intel686_sse2;
inline constexpr feature_flags amd64_v2        = featureset::amd64     | feature::cx16 | feature::lahf | feature::popcnt | feature::sse3 | feature::ssse3 | feature::sse4_1 | feature::sse4_2;
inline constexpr feature_flags amd64_v3        = featureset::amd64_v2  | feature::xsave | feature::avx | feature::avx2 | feature::bmi1 | feature::bmi2 | feature::f16c | feature::fma | feature::lzcnt | feature::movbe;
inline constexpr feature_flags msvc_x86_1998   = featureset::intel386 | feature::fpu | feature::fsin;
inline constexpr feature_flags msvc_x86_2005   = featureset::intel486DX;
inline constexpr feature_flags msvc_x86_2008   = featureset::intel586;
inline constexpr feature_flags msvc_x86_sse    = featureset::intel686_sse;
inline constexpr feature_flags msvc_x86_sse2   = featureset::intel686_sse2;
inline constexpr feature_flags msvc_x86_avx    = featureset::intel686_sse2 | feature::x3dnowprefetch | feature::xsave | feature::avx;
inline constexpr feature_flags msvc_x86_avx2   = featureset::intel686_sse2 | feature::x3dnowprefetch | feature::xsave | feature::avx | feature::avx2 | feature::fma | feature::bmi1;
inline constexpr feature_flags msvc_amd64      = featureset::amd64;
inline constexpr feature_flags msvc_amd64_avx  = featureset::amd64 | feature::xsave | feature::avx;
inline constexpr feature_flags msvc_amd64_avx2 = featureset::amd64 | feature::xsave | feature::avx | feature::avx2 | feature::fma | feature::bmi1;
} // namespace featureset

namespace modeset {
inline constexpr mode_flags intel386        = mode::base;
inline constexpr mode_flags intel486SX      = mode::base;
inline constexpr mode_flags intel486DX      = mode::base;
inline constexpr mode_flags intel586        = mode::base;
inline constexpr mode_flags intel586_mmx    = mode::base;
inline constexpr mode_flags intel686        = mode::base;
inline constexpr mode_flags intel686_mmx    = mode::base;
inline constexpr mode_flags intel686_sse    = mode::base | mode::xmm128sse;
inline constexpr mode_flags intel686_sse2   = mode::base | mode::xmm128sse;
inline constexpr mode_flags intel786        = mode::base | mode::xmm128sse;
inline constexpr mode_flags amd64           = mode::base | mode::xmm128sse;
inline constexpr mode_flags amd64_v2        = mode::base | mode::xmm128sse;
inline constexpr mode_flags amd64_v3        = mode::base | mode::xmm128sse | mode::ymm256avx;
inline constexpr mode_flags msvc_x86_1998   = mode::base;
inline constexpr mode_flags msvc_x86_2005   = mode::base;
inline constexpr mode_flags msvc_x86_2008   = mode::base;
inline constexpr mode_flags msvc_x86_sse    = mode::base | mode::xmm128sse;
inline constexpr mode_flags msvc_x86_sse2   = mode::base | mode::xmm128sse;
inline constexpr mode_flags msvc_x86_avx    = mode::base | mode::xmm128sse | mode::ymm256avx;
inline constexpr mode_flags msvc_x86_avx2   = mode::base | mode::xmm128sse | mode::ymm256avx;
inline constexpr mode_flags msvc_amd64      = mode::base | mode::xmm128sse;
inline constexpr mode_flags msvc_amd64_avx  = mode::base | mode::xmm128sse | mode::ymm256avx;
inline constexpr mode_flags msvc_amd64_avx2 = mode::base | mode::xmm128sse | mode::ymm256avx;
} // namespace modeset

// clang-format on


enum class vendor : uint8 {
	unknown = 0,
	AMD,
	Centaur,
	Cyrix,
	Intel,
	Transmeta,
	NSC,
	NexGen,
	Rise,
	SiS,
	UMC,
	VIA,
	DMnP,
	Zhaoxin,
	Hygon,
	Elbrus,
	MiSTer,
	bhyve,
	KVM,
	QEMU,
	HyperV,
	Parallels,
	VMWare,
	Xen,
	ACRN,
	QNX,
}; // enum class vendor



// clang-format off
[[nodiscard]] MPT_CONSTEVAL feature_flags assumed_features() noexcept {
	feature_flags flags{};
#if MPT_ARCH_X86 || MPT_ARCH_AMD64
	#ifdef MPT_ARCH_X86_I386
		flags |= feature::intel386;
	#endif
	#ifdef MPT_ARCH_X86_FPU
		flags |= feature::fpu;
	#endif
	#ifdef MPT_ARCH_X86_FSIN
		flags |= feature::fsin;
	#endif
	#ifdef MPT_ARCH_X86_I486
		flags |= feature::intel486;
	#endif
	#ifdef MPT_ARCH_X86_CPUID
		flags |= feature::cpuid;
	#endif
	#ifdef MPT_ARCH_X86_TSC
		flags |= feature::tsc;
	#endif
	#ifdef MPT_ARCH_X86_CX8
		flags |= feature::cx8;
	#endif
	#ifdef MPT_ARCH_X86_CMOV
		flags |= feature::cmov;
	#endif
	#ifdef MPT_ARCH_X86_MMX
		flags |= feature::mmx;
	#endif
	#ifdef MPT_ARCH_X86_MMXEXT
		flags |= feature::mmxext;
	#endif
	#ifdef MPT_ARCH_X86_3DNOW
		flags |= feature::x3dnow;
	#endif
	#ifdef MPT_ARCH_X86_3DNOWEXT
		flags |= feature::x3dnowext;
	#endif
	#ifdef MPT_ARCH_X86_3DNOWPREFETCH
		flags |= feature::x3dnowprefetch;
	#endif
	#ifdef MPT_ARCH_X86_FXSR
		flags |= feature::fxsr;
	#endif
	#ifdef MPT_ARCH_X86_SSE
		flags |= feature::sse;
	#endif
	#ifdef MPT_ARCH_X86_SSE2
		flags |= feature::sse2;
	#endif
	#ifdef MPT_ARCH_X86_SSE3
		flags |= feature::sse3;
	#endif
	#ifdef MPT_ARCH_X86_SSSE3
		flags |= feature::ssse3;
	#endif
	#ifdef MPT_ARCH_X86_SSE4_1
		flags |= feature::sse4_1;
	#endif
	#ifdef MPT_ARCH_X86_SSE4_2
		flags |= feature::sse4_2;
	#endif
	#ifdef MPT_ARCH_X86_XSAVE
		flags |= feature::xsave;
	#endif
	#ifdef MPT_ARCH_X86_AVX
		flags |= feature::avx;
	#endif
	#ifdef MPT_ARCH_X86_AVX2
		flags |= feature::avx2;
	#endif
	#ifdef MPT_ARCH_X86_CX16
		flags |= feature::cx16;
	#endif
	#ifdef MPT_ARCH_X86_LAHF
		flags |= feature::lahf;
	#endif
	#ifdef MPT_ARCH_X86_POPCNT
		flags |= feature::popcnt;
	#endif
	#ifdef MPT_ARCH_X86_BMI1
		flags |= feature::bmi1;
	#endif
	#ifdef MPT_ARCH_X86_BMI2
		flags |= feature::bmi2;
	#endif
	#ifdef MPT_ARCH_X86_F16C
		flags |= feature::f16c;
	#endif
	#ifdef MPT_ARCH_X86_FMA
		flags |= feature::fma;
	#endif
	#ifdef MPT_ARCH_X86_LZCNT
		flags |= feature::lzcnt;
	#endif
	#ifdef MPT_ARCH_X86_MOVBE
		flags |= feature::movbe;
	#endif
#endif // MPT_ARCH_X86 || MPT_ARCH_AMD64
	return flags;
}
// clang-format on


// clang-format off
[[nodiscard]] MPT_CONSTEVAL mode_flags assumed_modes() noexcept {
	mode_flags flags{};
#if MPT_ARCH_X86 || MPT_ARCH_AMD64
	#ifdef MPT_ARCH_X86_SSE
		flags |= mode::xmm128sse;
	#endif
	#ifdef MPT_ARCH_X86_AVX
		flags |= mode::ymm256avx;
	#endif
#endif // MPT_ARCH_X86 || MPT_ARCH_AMD64
	return flags;
}
// clang-format on



template <std::size_t N>
struct fixed_string {
	std::array<char, N> m_data = {};
	[[nodiscard]] constexpr const char & operator[](std::size_t i) const noexcept {
		return m_data[i];
	}
	[[nodiscard]] constexpr char & operator[](std::size_t i) noexcept {
		return m_data[i];
	}
	[[nodiscard]] constexpr std::size_t size() const noexcept {
		return m_data.size();
	}
	[[nodiscard]] constexpr const char * data() const noexcept {
		return m_data.data();
	}
	[[nodiscard]] constexpr char * data() noexcept {
		return m_data.data();
	}
	[[nodiscard]] constexpr const char * begin() const noexcept {
		return m_data.data();
	}
	[[nodiscard]] constexpr char * begin() noexcept {
		return m_data.data();
	}
	[[nodiscard]] constexpr const char * end() const noexcept {
		return m_data.data() + m_data.size();
	}
	[[nodiscard]] constexpr char * end() noexcept {
		return m_data.data() + m_data.size();
	}
	[[nodiscard]] constexpr operator std::string_view() const noexcept {
#if MPT_CXX_AT_LEAST(20) && !defined(MPT_COMPILER_QUIRK_NO_STRING_VIEW_ITERATOR_CTOR)
		return std::string_view(m_data.begin(), std::find(m_data.begin(), m_data.end(), '\0'));
#else
		return std::string_view(m_data.data(), std::find(m_data.begin(), m_data.end(), '\0') - m_data.begin());
#endif
	}
	template <std::size_t M>
	[[nodiscard]] friend MPT_CONSTEXPR20_FUN auto operator+(fixed_string<N> a, fixed_string<M> b) -> fixed_string<N + M> {
		fixed_string<N + M> result;
		std::copy(a.begin(), a.end(), result.data() + 0);
		std::copy(b.begin(), b.end(), result.data() + N);
		return result;
	}
	[[nodiscard]] inline explicit operator std::string() const {
		return std::string(m_data.begin(), std::find(m_data.begin(), m_data.end(), '\0'));
	}
};


struct cpu_info {

private:

	feature_flags Features{};
	mode_flags Modes{};
	uint32 CPUID = 0;
	vendor Vendor = vendor::unknown;
	uint16 Family = 0;
	uint8 Model = 0;
	uint8 Stepping = 0;
	fixed_string<12> VendorID;
	fixed_string<48> BrandID;
	bool Virtualized = false;
	fixed_string<12> HypervisorVendor;
	fixed_string<4> HypervisorInterface;
#if !MPT_ARCH_AMD64
	bool LongMode = false;
#endif // !MPT_ARCH_AMD64

public:

	[[nodiscard]] MPT_CONSTEXPRINLINE bool operator[](feature_flags query_features) const noexcept {
		return ((Features & query_features) == query_features);
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE bool has_features(feature_flags query_features) const noexcept {
		return ((Features & query_features) == query_features);
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE feature_flags get_features() const noexcept {
		return Features;
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE bool operator[](mode_flags query_modes) const noexcept {
		return ((Modes & query_modes) == query_modes);
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE bool enabled_modes(mode_flags query_modes) const noexcept {
		return ((Modes & query_modes) == query_modes);
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE mode_flags get_modes() const noexcept {
		return Modes;
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE uint32 get_cpuid() const noexcept {
		return CPUID;
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE vendor get_vendor() const noexcept {
		return Vendor;
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE uint16 get_family() const noexcept {
		return Family;
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE uint8 get_model() const noexcept {
		return Model;
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE uint8 get_stepping() const noexcept {
		return Stepping;
	}

	[[nodiscard]] inline std::string get_vendor_string() const {
		return std::string(VendorID);
	}

	[[nodiscard]] inline std::string get_brand_string() const {
		return std::string(BrandID);
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE bool is_virtual() const noexcept {
		return Virtualized;
	}

	[[nodiscard]] MPT_CONSTEXPRINLINE bool can_long_mode() const noexcept {
#if !MPT_ARCH_AMD64
		return LongMode;
#else  // MPT_ARCH_AMD64
		return true;
#endif // !MPT_ARCH_AMD64
	}

private:

#if MPT_ARCH_X86 || MPT_ARCH_AMD64

	struct cpuid_result {

		uint32 a = 0;
		uint32 b = 0;
		uint32 c = 0;
		uint32 d = 0;

		[[nodiscard]] MPT_CONSTEXPR20_FUN fixed_string<4> as_text4() const noexcept {
			fixed_string<4> result;
			result[0 + 0] = static_cast<char>((a >> 0) & 0xff);
			result[0 + 1] = static_cast<char>((a >> 8) & 0xff);
			result[0 + 2] = static_cast<char>((a >> 16) & 0xff);
			result[0 + 3] = static_cast<char>((a >> 24) & 0xff);
			return result;
		}

		[[nodiscard]] MPT_CONSTEXPR20_FUN fixed_string<12> as_text12bcd() const noexcept {
			fixed_string<12> result;
			result[0 + 0] = static_cast<char>((b >> 0) & 0xff);
			result[0 + 1] = static_cast<char>((b >> 8) & 0xff);
			result[0 + 2] = static_cast<char>((b >> 16) & 0xff);
			result[0 + 3] = static_cast<char>((b >> 24) & 0xff);
			result[4 + 0] = static_cast<char>((c >> 0) & 0xff);
			result[4 + 1] = static_cast<char>((c >> 8) & 0xff);
			result[4 + 2] = static_cast<char>((c >> 16) & 0xff);
			result[4 + 3] = static_cast<char>((c >> 24) & 0xff);
			result[8 + 0] = static_cast<char>((d >> 0) & 0xff);
			result[8 + 1] = static_cast<char>((d >> 8) & 0xff);
			result[8 + 2] = static_cast<char>((d >> 16) & 0xff);
			result[8 + 3] = static_cast<char>((d >> 24) & 0xff);
			return result;
		}

		[[nodiscard]] MPT_CONSTEXPR20_FUN fixed_string<12> as_text12bdc() const noexcept {
			fixed_string<12> result;
			result[0 + 0] = static_cast<char>((b >> 0) & 0xff);
			result[0 + 1] = static_cast<char>((b >> 8) & 0xff);
			result[0 + 2] = static_cast<char>((b >> 16) & 0xff);
			result[0 + 3] = static_cast<char>((b >> 24) & 0xff);
			result[4 + 0] = static_cast<char>((d >> 0) & 0xff);
			result[4 + 1] = static_cast<char>((d >> 8) & 0xff);
			result[4 + 2] = static_cast<char>((d >> 16) & 0xff);
			result[4 + 3] = static_cast<char>((d >> 24) & 0xff);
			result[8 + 0] = static_cast<char>((c >> 0) & 0xff);
			result[8 + 1] = static_cast<char>((c >> 8) & 0xff);
			result[8 + 2] = static_cast<char>((c >> 16) & 0xff);
			result[8 + 3] = static_cast<char>((c >> 24) & 0xff);
			return result;
		}

		[[nodiscard]] MPT_CONSTEXPR20_FUN fixed_string<16> as_text16() const noexcept {
			fixed_string<16> result;
			result[0 + 0] = static_cast<char>((a >> 0) & 0xff);
			result[0 + 1] = static_cast<char>((a >> 8) & 0xff);
			result[0 + 2] = static_cast<char>((a >> 16) & 0xff);
			result[0 + 3] = static_cast<char>((a >> 24) & 0xff);
			result[4 + 0] = static_cast<char>((b >> 0) & 0xff);
			result[4 + 1] = static_cast<char>((b >> 8) & 0xff);
			result[4 + 2] = static_cast<char>((b >> 16) & 0xff);
			result[4 + 3] = static_cast<char>((b >> 24) & 0xff);
			result[8 + 0] = static_cast<char>((c >> 0) & 0xff);
			result[8 + 1] = static_cast<char>((c >> 8) & 0xff);
			result[8 + 2] = static_cast<char>((c >> 16) & 0xff);
			result[8 + 3] = static_cast<char>((c >> 24) & 0xff);
			result[12 + 0] = static_cast<char>((d >> 0) & 0xff);
			result[12 + 1] = static_cast<char>((d >> 8) & 0xff);
			result[12 + 2] = static_cast<char>((d >> 16) & 0xff);
			result[12 + 3] = static_cast<char>((d >> 24) & 0xff);
			return result;
		}
	};

#if MPT_COMPILER_MSVC || MPT_COMPILER_GCC || MPT_COMPILER_CLANG

	[[nodiscard]] static cpuid_result cpuid(uint32 function) noexcept {

#if MPT_COMPILER_MSVC

		cpuid_result result;
		int CPUInfo[4]{};
		__cpuid(CPUInfo, function);
		result.a = CPUInfo[0];
		result.b = CPUInfo[1];
		result.c = CPUInfo[2];
		result.d = CPUInfo[3];
		return result;

#elif MPT_COMPILER_GCC || (MPT_COMPILER_CLANG && !MPT_ARCH_AMD64) || MPT_CLANG_AT_LEAST(13, 0, 0)
		// <https://reviews.llvm.org/D101338>

		cpuid_result result;
		unsigned int regeax{};
		unsigned int regebx{};
		unsigned int regecx{};
		unsigned int regedx{};
		__cpuid(function, regeax, regebx, regecx, regedx);
		result.a = regeax;
		result.b = regebx;
		result.c = regecx;
		result.d = regedx;
		return result;

#elif MPT_COMPILER_CLANG && MPT_ARCH_AMD64
		// <https://reviews.llvm.org/D101338>

		cpuid_result result;
		unsigned int a{};
		unsigned int b{};
		unsigned int c{};
		unsigned int d{};
		// clang-format off
		__asm__ __volatile__ (
			"xchgq %%rbx,%q1 \n\t"
			"cpuid \n\t"
			"xchgq %%rbx,%q1 \n\t"
			: "=a" (a), "=r" (b), "=c" (c), "=d" (d)
			: "0" (function));
		// clang-format on
		result.a = a;
		result.b = b;
		result.c = c;
		result.d = d;
		return result;

#elif 0

		cpuid_result result;
		unsigned int a{};
		unsigned int b{};
		unsigned int c{};
		unsigned int d{};
		// clang-format off
		__asm__ __volatile__ (
			"cpuid \n\t"
			: "=a" (a), "=b" (b), "=c" (c), "=d" (d)
			: "0" (function));
		// clang-format on
		result.a = a;
		result.b = b;
		result.c = c;
		result.d = d;
		return result;

#else // MPT_COMPILER

		return cpuid_result result{};

#endif // MPT_COMPILER
	}

	[[nodiscard]] static cpuid_result cpuidex(uint32 function_a, uint32 function_c) noexcept {

#if MPT_COMPILER_MSVC

		cpuid_result result;
		int CPUInfo[4]{};
		__cpuidex(CPUInfo, function_a, function_c);
		result.a = CPUInfo[0];
		result.b = CPUInfo[1];
		result.c = CPUInfo[2];
		result.d = CPUInfo[3];
		return result;

#elif MPT_COMPILER_GCC || (MPT_COMPILER_CLANG && !MPT_ARCH_AMD64) || MPT_CLANG_AT_LEAST(13, 0, 0)
		// <https://reviews.llvm.org/D101338>

		cpuid_result result;
		unsigned int regeax{};
		unsigned int regebx{};
		unsigned int regecx{};
		unsigned int regedx{};
		__cpuid_count(function_a, function_c, regeax, regebx, regecx, regedx);
		result.a = regeax;
		result.b = regebx;
		result.c = regecx;
		result.d = regedx;
		return result;

#elif MPT_COMPILER_CLANG && MPT_ARCH_AMD64
		// <https://reviews.llvm.org/D101338>

		cpuid_result result;
		unsigned int a{};
		unsigned int b{};
		unsigned int c{};
		unsigned int d{};
		// clang-format off
		__asm__ __volatile__ (
			"xchgq %%rbx,%q1 \n\t"
			"cpuid \n\t"
			"xchgq %%rbx,%q1 \n\t"
			: "=a" (a), "=r" (b), "=c" (c), "=d" (d)
			: "0" (function_a), "2" (function_c));
		// clang-format on
		result.a = a;
		result.b = b;
		result.c = c;
		result.d = d;
		return result;

#elif 0

		cpuid_result result;
		unsigned int a{};
		unsigned int b{};
		unsigned int c{};
		unsigned int d{};
		// clang-format off
		__asm__ __volatile__ (
			"cpuid \n\t"
			: "=a" (a), "=b" (b), "=c" (c), "=d" (d)
			: "0" (function_a), "2" (function_c));
		// clang-format on
		result.a = a;
		result.b = b;
		result.c = c;
		result.d = d;
		return result;

#else // MPT_COMPILER

		return cpuid_result result{};

#endif // MPT_COMPILER
	}

#if MPT_MODE_KERNEL

	[[nodiscard]] static uint32 read_cr4() noexcept {
		return __readcr4();
	}

#endif // MPT_MODE_KERNEL

	[[nodiscard]] static uint64 read_xcr(uint32 num) noexcept {

#if MPT_COMPILER_MSVC

		return _xgetbv(num);

#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG

		uint32 param_ecx = num;
		uint32 result_eax = 0;
		uint32 result_edx = 0;
		// clang-format off
		__asm__ __volatile__("xgetbv" : "=a" (result_eax), "=d" (result_edx) : "c" (param_ecx));
		// clang-format on
		return static_cast<uint64>(result_eax) + (static_cast<uint64>(result_edx) << 32);

#else

		return _xgetbv(num);

#endif
	}

#endif // MPT_COMPILER_MSVC || MPT_COMPILER_GCC || MPT_COMPILER_CLANG

private:

#if MPT_ARCH_X86 || !MPT_ARCH_X86_AMD64_FAST_DETECT

#if MPT_OS_DJGPP

	[[nodiscard]] static uint8 detect_cpu_level() noexcept {

#if 0
		uint8 result = 0;
		__dpmi_version_ret dpmi_version{};
		if (__dpmi_get_version(&dpmi_version) == 0) {
			result = dpmi_version.cpu;
		}
		return result;
#else
		uint8 result = 0;
		__dpmi_regs regs{};
		regs.x.ax = 0x0400;
		if (__dpmi_int(0x31, &regs) == 0) {
			unsigned int cf = (regs.x.flags >> 0) & 1u;
			if (cf == 0) {
				result = regs.h.cl;
			}
		}
		return result;
#endif
	}

#endif // MPT_OS_DJGPP

	[[nodiscard]] static uint8 detect_fpu_level() noexcept {

#if MPT_OS_DJGPP

#if 1
		uint8 result = 0;
		int coprocessor_status = __dpmi_get_coprocessor_status();
		if (coprocessor_status == -1) {
			// error = __dpmi_error
			return 0;
		}
		result = (static_cast<unsigned int>(coprocessor_status) & 0x00f0u) >> 4;
		return result;
#else
		uint8 result = 0;
		__dpmi_regs regs{};
		regs.x.ax = 0x0e00;
		if (__dpmi_int(0x31, &regs) == 0) {
			unsigned int cf = (regs.x.flags >> 0) & 1u;
			if (cf == 0) {
				result = (regs.x.ax & 0x00f0u) >> 4;
			}
		}
		return result;
#endif

#elif MPT_OS_WINDOWS

		uint8 result = 0;
#if MPT_WINNT_AT_LEAST(MPT_WIN_NT4)
		if (mpt::osinfo::windows::Version::Current().IsAtLeast(mpt::osinfo::windows::Version::Win2000)) {
			if (IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED) == 0) {
				result = 3;
			}
		} else {
			// logic is inverted on NT4
			if (IsProcessorFeaturePresent(PF_FLOATING_POINT_EMULATED) != 0) {
				result = 3;
			}
		}
#else
		if ((assumed_features() & feature::fpu) && (assumed_features() & feature::fsin)) {
			result = 3;
		} else if (assumed_features() & feature::fpu) {
			result = 2;
		} else {
			result = 0;
		}
#endif
		return result;

#elif MPT_COMPILER_MSVC && MPT_MODE_KERNEL

		uint8 result = 0;
		const std::size_t cr0 = __readcr0();
		if (!(cr0 & (1u << 2))) { // EM
			result = 2;
			if (cr0 & (1u << 4)) { // ET
				result = 3;
			}
		}
		return result;

#else

		uint8 result = 0;
		if ((assumed_features() & feature::fpu) && (assumed_features() & feature::fsin)) {
			result = 3;
		} else if (assumed_features() & feature::fpu) {
			result = 2;
		} else {
			result = 0;
		}
		return result;

#endif
	}

#if MPT_COMPILER_MSVC || MPT_COMPILER_GCC || MPT_COMPILER_CLANG

	[[nodiscard]] static bool can_toggle_eflags(std::size_t mask) noexcept {
		std::size_t eflags_old = __readeflags();
		std::size_t eflags_flipped = eflags_old ^ mask;
		__writeeflags(eflags_flipped);
		std::size_t eflags_testchanged = __readeflags();
		__writeeflags(eflags_old);
		return ((eflags_testchanged ^ eflags_old) & mask) == mask;
	}

#endif

	[[nodiscard]] static bool can_toggle_eflags_ac() noexcept {
#if MPT_COMPILER_MSVC || MPT_COMPILER_GCC || MPT_COMPILER_CLANG
		return can_toggle_eflags(0x0004'0000);
#else  // MPT_COMPILER
		return (assumed_features() & feature::intel486) != 0;
#endif // MPT_COMPILER
	}

	[[nodiscard]] static bool can_toggle_eflags_id() noexcept {
#if MPT_COMPILER_MSVC || MPT_COMPILER_GCC || MPT_COMPILER_CLANG
		return can_toggle_eflags(0x0020'0000);
#else  // MPT_COMPILER
		return (assumed_features() & feature::tsc) != 0;
#endif // MPT_COMPILER
	}

	[[nodiscard]] static bool detect_nexgen() noexcept {

#if MPT_ARCH_X86 && MPT_COMPILER_MSVC

		uint8 result = 0;
		// clang-format off
		_asm {
			mov eax, 0x5555
			xor edx, edx
			mov ecx, 2
			clc
			div ecx
			jz  found
			jmp done
			found:
				mov result, 1
				jmp done
			done:
		}
		// clang-format on
		return (result != 0);

#elif MPT_ARCH_X86 && (MPT_COMPILER_GCC || MPT_COMPILER_CLANG)

		unsigned int result = 0;
		// clang-format off
		__asm__ __volatile(
			"movl $0x5555, %%eax\n"
			"xorl %%edx, %%edx\n"
			"movl $2, %%ecx\n"
			"clc\n"
			"divl %%ecx\n"
			"jz detect_nexgen_found\n"
			"movl $0, %%eax\n"
			"jmp detect_nexgen_done\n"
			"detect_nexgen_found:\n"
			"movl $0, %%eax\n"
			"jmp detect_nexgen_done\n"
			"detect_nexgen_done:\n"
			: "=a" (result)
			:
			: "%ebx", "%ecx"
		);
		// clang-format on
		return (result != 0);

#else

		// assume false
		return false;

#endif
	}

	[[nodiscard]] static bool detect_cyrix() noexcept {

#if MPT_ARCH_X86 && MPT_COMPILER_MSVC

		uint8 result = 0;
		// clang-format off
		_asm {
			xor  ax, ax
			sahf
			mov  ax, 5
			mov  bx, 2
			div  bl
			lahf
			cmp  ah, 2
			jne  not_cyrix
				mov  result, 1
			not_cyrix:
		}
		// clang-format on
		return (result != 0);

#elif MPT_ARCH_X86 && (MPT_COMPILER_GCC || MPT_COMPILER_CLANG)

		unsigned int result = 0;
		// clang-format off
		__asm__ __volatile(
			"xor %%ax, %%ax\n"
			"sahf\n"
			"movw $5, %%ax\n"
			"movw $2, %%bx\n"
			"divb %%bl\n"
			"lahf\n"
			"cmpb $2, %%ah\n"
			"jne detect_cyrix_done_not_cyrix\n"
			"movl $1, %%eax\n"
			"jmp detect_cyrix_done\n"
			"detect_cyrix_done_not_cyrix:\n"
			"movl $0, %%eax\n"
			"jmp detect_cyrix_done\n"
			"detect_cyrix_done:\n"
			: "=a" (result)
			:
			: "%ebx"
		);
		// clang-format on
		return (result != 0);

#else

		// assume false
		return false;

#endif
	}

	[[nodiscard]] static uint16 read_cyrix_id() noexcept {

#if MPT_OS_DJGPP

		uint16 result = 0;
		outportb(0x22, 0x00);
		result |= static_cast<uint16>(inportb(0x23)) << 8;
		outportb(0x22, 0x01);
		result |= static_cast<uint16>(inportb(0x23)) << 0;
		outportb(0x22, 0x00);
		inportb(0x23);
		return result;

#elif MPT_COMPILER_MSVC && MPT_MODE_KERNEL

		uint16 result = 0;
		__outbyte(0x22, 0x00);
		result |= static_cast<uint16>(__inbyte(0x23)) << 8;
		__outbyte(0x22, 0x01);
		result |= static_cast<uint16>(__inbyte(0x23)) << 0;
		__outbyte(0x22, 0x00);
		__inbyte(0x23);
		return result;

#else

		return 0x00'00;

#endif
	}

#endif // MPT_ARCH_X86

#endif // MPT_ARCH_X86 || MPT_ARCH_AMD64

public:

	cpu_info() noexcept {

#if MPT_ARCH_X86 || MPT_ARCH_AMD64

#if MPT_COMPILER_MSVC || MPT_COMPILER_GCC || MPT_COMPILER_CLANG

#if MPT_ARCH_X86 || !MPT_ARCH_X86_AMD64_FAST_DETECT

		Features |= featureset::intel386;

		if (can_toggle_eflags_ac()) {
			Features |= feature::intel486;
		}
		if (can_toggle_eflags_id()) {
			Features |= feature::cpuid;
		}
		if (!Features.supports(feature::cpuid)) {
			// without cpuid
			if (!Features.supports(feature::intel486)) {
				// 386
				const uint8 fpu_level = detect_fpu_level();
				if (fpu_level >= 2) {
					Features |= feature::fpu;
				}
				if (fpu_level >= 3) {
					Features |= feature::fsin;
				}
				if (detect_nexgen()) {
					Vendor = vendor::NexGen;
				} else {
					Vendor = vendor::unknown;
				}
			} else {
				// 486+
				const uint8 fpu_level = detect_fpu_level();
				if (fpu_level >= 2) {
					Features |= feature::fpu;
				}
				if (fpu_level >= 3) {
					Features |= feature::fsin;
				}
				if (detect_cyrix()) {
					Vendor = vendor::Cyrix;
					uint16 id = read_cyrix_id();
					if (id <= 0x07'00) {
						// Cx486SLC / Cx486DLC
						Family = 3;
						Model = static_cast<uint8>((id & 0xff'00) >> 8);
						Stepping = static_cast<uint8>(id & 0x00'ff);
					} else if ((0x10'00 <= id) && (id <= 0x13'00)) {
						// Cx486S
						Family = 4;
						Model = static_cast<uint8>((id & 0xff'00) >> 8);
						Stepping = static_cast<uint8>(id & 0x00'ff);
					} else if ((0x1a'00 <= id) && (id <= 0x1f'00)) {
						// Cx486DX
						Family = 4;
						Model = static_cast<uint8>((id & 0xff'00) >> 8);
						Stepping = static_cast<uint8>(id & 0x00'ff);
					} else if ((0x28'00 <= id) && (id <= 0x2e'00)) {
						// Cx5x86
						Family = 4;
						Model = static_cast<uint8>((id & 0xff'00) >> 8);
						Stepping = static_cast<uint8>(id & 0x00'ff);
					} else if ((0x40'00 <= id) && (id <= 0x4f'00)) {
						// MediaGx / MediaGXm
						Family = 4;
						Model = 4;
						Stepping = static_cast<uint8>(id & 0x00'ff);
					} else if ((0x30'00 <= id) && (id <= 0x34'00)) {
						// Cx6x86 / Cx6x86L
						Family = 5;
						Model = 2;
						Stepping = static_cast<uint8>(id & 0x00'ff);
						if ((id & 0x00'ff) > 0x21) {
							// Cx6x86L
							Features |= feature::cx8;
						}
					} else if ((0x50'00 <= id) && (id <= 0x5f'00)) {
						// Cx6x86MX
						Family = 6;
						Model = 0;
						Stepping = static_cast<uint8>(id & 0x00'ff);
						Features |= feature::cx8;
						Features |= feature::tsc;
						Features |= feature::mmx;
						Features |= feature::cmov;
					}
				} else {
					Vendor = vendor::unknown;
				}
			}
		}

#elif MPT_ARCH_AMD64

		Features |= featureset::amd64;
		Modes |= modeset::amd64;

#endif // MPT_ARCH

		bool have_osxsave = false;

		if (Features.supports(feature::cpuid)) {
			// with cpuid
			cpuid_result VendorString = cpuid(0x0000'0000u);
			VendorID = VendorString.as_text12bdc();
			if (VendorID == std::string_view("            ")) {
				Vendor = vendor::unknown;
			} else if (VendorID == std::string_view("AMDisbetter!")) {
				Vendor = vendor::AMD;
			} else if (VendorID == std::string_view("AuthenticAMD")) {
				Vendor = vendor::AMD;
			} else if (VendorID == std::string_view("CentaurHauls")) {
				Vendor = vendor::Centaur;
			} else if (VendorID == std::string_view("CyrixInstead")) {
				Vendor = vendor::Cyrix;
			} else if (VendorID == std::string_view("GenuineIntel")) {
				Vendor = vendor::Intel;
			} else if (VendorID == std::string_view("TransmetaCPU")) {
				Vendor = vendor::Transmeta;
			} else if (VendorID == std::string_view("GenuineTMx86")) {
				Vendor = vendor::Transmeta;
			} else if (VendorID == std::string_view("Geode by NSC")) {
				Vendor = vendor::NSC;
			} else if (VendorID == std::string_view("NexGenDriven")) {
				Vendor = vendor::NexGen;
			} else if (VendorID == std::string_view("RiseRiseRise")) {
				Vendor = vendor::Rise;
			} else if (VendorID == std::string_view("SiS SiS SiS ")) {
				Vendor = vendor::SiS;
			} else if (VendorID == std::string_view("UMC UMC UMC ")) {
				Vendor = vendor::UMC;
			} else if (VendorID == std::string_view("VIA VIA VIA ")) {
				Vendor = vendor::VIA;
			} else if (VendorID == std::string_view("Vortex86 SoC")) {
				Vendor = vendor::DMnP;
			} else if (VendorID == std::string_view("  Shanghai  ")) {
				Vendor = vendor::Zhaoxin;
			} else if (VendorID == std::string_view("HygonGenuine")) {
				Vendor = vendor::Hygon;
			} else if (VendorID == std::string_view("E2K MACHINE")) {
				Vendor = vendor::Elbrus;
			} else if (VendorID == std::string_view("MiSTer AO486")) {
				Vendor = vendor::MiSTer;
			} else if (VendorID == std::string_view("bhyve bhyve ")) {
				Vendor = vendor::bhyve;
			} else if (VendorID == std::string_view(" KVMKVMKVM  ")) {
				Vendor = vendor::KVM;
			} else if (VendorID == std::string_view("TCGTCGTCGTCG")) {
				Vendor = vendor::QEMU;
			} else if (VendorID == std::string_view("Microsoft Hv")) {
				Vendor = vendor::HyperV;
			} else if (VendorID == std::string_view(" lrpepyh  vr")) {
				Vendor = vendor::Parallels;
			} else if (VendorID == std::string_view("VMwareVMware")) {
				Vendor = vendor::VMWare;
			} else if (VendorID == std::string_view("XenVMMXenVMM")) {
				Vendor = vendor::Xen;
			} else if (VendorID == std::string_view("ACRNACRNACRN")) {
				Vendor = vendor::ACRN;
			} else if (VendorID == std::string_view(" QNXQVMBSQG ")) {
				Vendor = vendor::QNX;
			}
			// Cyrix 6x86 and 6x86MX do not specify the value returned in eax.
			// They both support 0x00000001u however.
			if ((VendorString.a >= 0x0000'0001u) || (Vendor == vendor::Cyrix)) {
				cpuid_result StandardFeatureFlags = cpuid(0x0000'0001u);
				CPUID = StandardFeatureFlags.a;
				// clang-format off
				uint32 BaseStepping = (StandardFeatureFlags.a >>  0) & 0x0f;
				uint32 BaseModel    = (StandardFeatureFlags.a >>  4) & 0x0f;
				uint32 BaseFamily   = (StandardFeatureFlags.a >>  8) & 0x0f;
				uint32 ExtModel     = (StandardFeatureFlags.a >> 16) & 0x0f;
				uint32 ExtFamily    = (StandardFeatureFlags.a >> 20) & 0xff;
				// clang-format on
				if (BaseFamily == 0xf) {
					Family = static_cast<uint16>(ExtFamily + BaseFamily);
				} else {
					Family = static_cast<uint16>(BaseFamily);
				}
				if ((Vendor == vendor::AMD) && (BaseFamily == 0xf)) {
					Model = static_cast<uint8>((ExtModel << 4) | (BaseModel << 0));
				} else if ((Vendor == vendor::Centaur) && (BaseFamily >= 0x6)) {
					// Newer Zhaoxin CPUs use extended family also with BaseFamily=7.
					Model = static_cast<uint8>((ExtModel << 4) | (BaseModel << 0));
				} else if ((BaseFamily == 0x6) || (BaseFamily == 0xf)) {
					Model = static_cast<uint8>((ExtModel << 4) | (BaseModel << 0));
				} else {
					Model = static_cast<uint8>(BaseModel);
				}
				Stepping = static_cast<uint8>(BaseStepping);
				// clang-format off
				Features |= (StandardFeatureFlags.d & (1u <<  0)) ? (feature::fpu | feature::fsin) : feature::none;
				Features |= (StandardFeatureFlags.d & (1u <<  4)) ? (feature::tsc) : feature::none;
				Features |= (StandardFeatureFlags.d & (1u <<  8)) ? (feature::cx8) : feature::none;
				Features |= (StandardFeatureFlags.d & (1u << 15)) ? (feature::cmov) : feature::none;
				Features |= (StandardFeatureFlags.d & (1u << 23)) ? (feature::mmx) : feature::none;
				Features |= (StandardFeatureFlags.d & (1u << 24)) ? (feature::fxsr) : feature::none;
				Features |= (StandardFeatureFlags.d & (1u << 25)) ? (feature::sse | feature::mmxext) : feature::none;
				Features |= (StandardFeatureFlags.d & (1u << 26)) ? (feature::sse2) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u <<  0)) ? (feature::sse3) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u <<  9)) ? (feature::ssse3) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u << 12)) ? (feature::fma) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u << 13)) ? (feature::cx16) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u << 19)) ? (feature::sse4_1) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u << 20)) ? (feature::sse4_2) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u << 22)) ? (feature::movbe) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u << 23)) ? (feature::popcnt) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u << 26)) ? (feature::xsave) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u << 28)) ? (feature::avx) : feature::none;
				Features |= (StandardFeatureFlags.c & (1u << 29)) ? (feature::f16c) : feature::none;
				// clang-format on
				if (StandardFeatureFlags.c & (1u << 27)) {
					have_osxsave = true;
				}
				if (StandardFeatureFlags.c & (1u << 31)) {
					Virtualized = true;
				}
			}
			if (VendorString.a >= 0x0000'0007u) {
				cpuid_result ExtendedFeatures = cpuidex(0x0000'0007u, 0x0000'0000u);
				// clang-format off
				Features |= (ExtendedFeatures.b & (1u <<  3)) ? (feature::bmi1) : feature::none;
				Features |= (ExtendedFeatures.b & (1u <<  5)) ? (feature::avx2) : feature::none;
				Features |= (ExtendedFeatures.b & (1u <<  8)) ? (feature::bmi2) : feature::none;
				// clang-format on
			}
			// 3DNow! manual recommends to just execute 0x8000'0000u.
			// It is totally unknown how earlier CPUs from other vendors
			// would behave.
			// Thus we only execute 0x80000000u on other vendors CPUs for the earliest
			// that we found it documented for and that actually supports 3DNow!.
			bool ecpuid = false;
			bool x3dnowknown = false;
			if (Vendor == vendor::Intel) {
				ecpuid = true;
			} else if (Vendor == vendor::AMD) {
				if ((Family > 5) || ((Family == 5) && (Model >= 8))) {
					// >= K6-2 (K6 = Family 5, K6-2 = Model 8)
					// Not sure if earlier AMD CPUs support 0x80000000u.
					// AMD 5k86 and AMD K5 manuals do not mention it.
					ecpuid = true;
					x3dnowknown = true;
				}
			} else if (Vendor == vendor::Centaur) {
				// Centaur (IDT WinChip or VIA C3)
				if (Family == 5) {
					// IDT
					if (Model >= 8) {
						// >= WinChip 2
						ecpuid = true;
						x3dnowknown = true;
					}
				} else if (Family >= 6) {
					// VIA
					if ((Family >= 7) || ((Family == 6) && (Model >= 7))) {
						// >= C3 Samuel 2
						ecpuid = true;
						x3dnowknown = true;
					}
				}
			} else if (Vendor == vendor::Cyrix) {
				// Cyrix
				// 6x86    : 5.2.x
				// 6x86L   : 5.2.x
				// MediaGX : 4.4.x
				// 6x86MX  : 6.0.x
				// MII     : 6.0.x
				// MediaGXm: 5.4.x
				// well, doh ...
				if ((Family == 5) && (Model >= 4)) {
					// Cyrix MediaGXm
					ecpuid = true;
					x3dnowknown = true;
				}
			} else if (Vendor == vendor::NSC) {
				// National Semiconductor
				if ((Family > 5) || ((Family == 5) && (Model >= 5))) {
					// >= Geode GX2
					ecpuid = true;
					x3dnowknown = true;
				}
			} else {
				// Intel specification allows 0x8000'0000u on earlier CPUs,
				// thus we execute it on unknown vendors.
				ecpuid = true;
			}
			if (ecpuid) {
				cpuid_result ExtendedVendorString = cpuid(0x8000'0000u);
				if ((ExtendedVendorString.a & 0xffff'0000u) == 0x8000'0000u) {
					if (ExtendedVendorString.a >= 0x8000'0001u) {
						// clang-format off
						cpuid_result ExtendedFeatureFlags = cpuid(0x8000'0001u);
#if !MPT_ARCH_AMD64
						if (ExtendedFeatureFlags.d & (1u << 29)) {
							LongMode = true;
						}
#endif // !MPT_ARCH_AMD64
						Features |= (ExtendedFeatureFlags.c & (1u <<  0)) ? (feature::lahf) : feature::none;
						Features |= (ExtendedFeatureFlags.c & (1u <<  5)) ? (feature::lzcnt) : feature::none;
						if (x3dnowknown) {
							Features |= (ExtendedFeatureFlags.d & (1u << 31)) ? (feature::x3dnow) : feature::none;
						}
						if (Vendor == vendor::AMD) {
							Features |= (ExtendedFeatureFlags.d & (1u << 22)) ? (feature::mmxext) : feature::none;
							Features |= (ExtendedFeatureFlags.d & (1u << 30)) ? (feature::x3dnowext) : feature::none;
							Features |= (ExtendedFeatureFlags.c & (1u <<  5)) ? (feature::popcnt) : feature::none;
							Features |= (ExtendedFeatureFlags.c & (1u <<  8)) ? (feature::x3dnowprefetch) : feature::none;
						}
						// clang-format on
					}
					if (ExtendedVendorString.a >= 0x8000'0004u) {
						BrandID = cpuid(0x8000'0002u).as_text16() + cpuid(0x8000'0003u).as_text16() + cpuid(0x8000'0004u).as_text16();
					}
				}
			}

#if MPT_ARCH_AMD64

			// clang-format off
			Modes |= mode::xmm128sse;
			const bool have_xsave =
#ifdef MPT_ARCH_X86_XSAVE
				true;
#else
				Features.supports(feature::xsave);
#endif
			MPT_MAYBE_CONSTANT_IF (have_xsave && have_osxsave) {
				const uint64 xcr0 = read_xcr(0x0000'0000u);
				Modes |= (xcr0 & (1ull << 2)) ? mode::ymm256avx : mode::base;
			}
			// clang-format on

#else // !MPT_ARCH_AMD64

			// clang-format off
			const bool have_xsave =
#ifdef MPT_ARCH_X86_XSAVE
				true;
#else
				Features.supports(feature::xsave);
#endif
			MPT_MAYBE_CONSTANT_IF (have_xsave && have_osxsave) {
				const uint64 xcr0 = read_xcr(0x0000'0000u);
				Modes |= (xcr0 & (1ull << 1)) ? mode::xmm128sse : mode::base;
				Modes |= (xcr0 & (1ull << 2)) ? mode::ymm256avx : mode::base;
			} else {
				const bool have_fxsr =
#ifdef MPT_ARCH_X86_FXSR
					true;
#else
					Features.supports(feature::fxsr);
#endif
				MPT_MAYBE_CONSTANT_IF (have_fxsr) {
#if MPT_MODE_KERNEL
					const uint32 cr4 = read_cr4();
					Modes |= (cr4 & (1u << 9)) ? mode::xmm128sse : mode::base;
#else // !MPT_MODE_KERNEL
					// There is no way for user-mode code to check for SSE enabled in CR4.
					// Assume based on FXSR and SSE and platform heuristics.
					// Avoid assumption on DOS.
#if MPT_OS_WINDOWS
					// get from platform API
#if MPT_OS_WINDOWS_WINNT
#ifdef PF_XMMI_INSTRUCTIONS_AVAILABLE
					Modes |= (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) != 0) ? (mode::xmm128sse) : mode::base;
#endif
#elif MPT_WIN9X_AT_LEAST(MPT_WIN_WIN98)
					const bool have_sse =
#ifdef MPT_ARCH_X86_SSE
						true;
#else
						Features.supports(feature::sse);
#endif
					MPT_MAYBE_CONSTANT_IF (have_sse) {
						Modes |= mode::xmm128sse;
					}
#else // MPT_OS_WINDOWS
					// nothing
#endif // MPT_OS_WINDOWS
#elif MPT_OS_DJGPP || MPT_OS_UNKNOWN
					// avoid SSE on DOS and unknown systems,
					// however if we directly targeting >= SSE, then assume it's activated because the process will crash otherwise anyway
#ifdef MPT_ARCH_X86_SSE
					Modes |= mode::xmm128sse;
#endif
#else // MPT_OS
					// assume based on FXSR and SSE
					const bool have_sse =
#ifdef MPT_ARCH_X86_SSE
						true;
#else
						Features.supports(feature::sse);
#endif
					MPT_MAYBE_CONSTANT_IF (have_sse) {
						Modes |= mode::xmm128sse;
					}
#endif // MPT_OS
#endif // MPT_MODE_KERNEL
				}
			}
			// clang-format on

#endif // MPT_ARCH_AMD64

			if (Virtualized) {
				cpuid_result HypervisorVendorID = cpuid(0x4000'0000u);
				if (HypervisorVendorID.a >= 0x4000'0000u) {
					HypervisorVendor = HypervisorVendorID.as_text12bcd();
					if (HypervisorVendor == std::string_view("Microsoft Hv")) {
						if (HypervisorVendorID.a >= 0x4000'0001u) {
							cpuid_result HypervisorInterfaceID = cpuid(0x4000'0001u);
							HypervisorInterface = HypervisorInterfaceID.as_text4();
						}
					} else if (HypervisorVendor == std::string_view("KVMKVMKVM")) {
						// nothing
					}
				}
			}
		}

#elif MPT_OS_WINDOWS

#if MPT_ARCH_X86 || !MPT_ARCH_X86_AMD64_FAST_DETECT

		SYSTEM_INFO si = {};
		GetSystemInfo(&si);
		switch (si.wProcessorArchitecture) {
			case PROCESSOR_ARCHITECTURE_INTEL:
			case PROCESSOR_ARCHITECTURE_AMD64:
			case PROCESSOR_ARCHITECTURE_IA32_ON_WIN64:
			case PROCESSOR_ARCHITECTURE_IA32_ON_ARM64:
				switch (si.dwProcessorType) {
					case PROCESSOR_INTEL_386:
						Family = si.wProcessorLevel;
						if (((si.wProcessorRevision & 0xff00) >> 8) == 0xff) {
							Model = ((si.wProcessorRevision & 0x00f0) >> 4) - 0xa;
							Stepping = ((si.wProcessorRevision & 0x000f) >> 0);
						} else {
							Model = ((si.wProcessorRevision & 0xff00) >> 8) + static_cast<unsigned char>('A');
							Stepping = ((si.wProcessorRevision & 0x00ff) >> 0);
						}
						Model = (si.wProcessorRevision & 0xff00) >> 8;
						Stepping = (si.wProcessorRevision & 0x00ff) >> 0;
						Features |= featureset::intel386;
						break;
					case PROCESSOR_INTEL_486:
						Family = si.wProcessorLevel;
						if (((si.wProcessorRevision & 0xff00) >> 8) == 0xff) {
							Model = ((si.wProcessorRevision & 0x00f0) >> 4) - 0xa;
							Stepping = ((si.wProcessorRevision & 0x000f) >> 0);
						} else {
							Model = ((si.wProcessorRevision & 0xff00) >> 8) + static_cast<unsigned char>('A');
							Stepping = ((si.wProcessorRevision & 0x00ff) >> 0);
						}
						Model = (si.wProcessorRevision & 0xff00) >> 8;
						Stepping = (si.wProcessorRevision & 0x00ff) >> 0;
						Features |= featureset::intel486SX;
						break;
					case PROCESSOR_INTEL_PENTIUM:
						Family = si.wProcessorLevel;
						Model = (si.wProcessorRevision & 0xff00) >> 8;
						Stepping = (si.wProcessorRevision & 0x00ff) >> 0;
						// rely on IsProcessorFeaturePresent() for > 486 features
						// Features |= featureset::intel586;
						Features |= featureset::intel486SX;
						break;
				}
				break;
		}
		Features |= featureset::intel386;
		const uint8 fpu_level = detect_fpu_level();
		if (fpu_level >= 2) {
			Features |= feature::fpu;
		}
		if (fpu_level >= 3) {
			Features |= feature::fsin;
		}

#elif MPT_ARCH_AMD64

		Features |= featureset::amd64;

#endif // MPT_ARCH

#if MPT_OS_WINDOWS_WINNT
		// clang-format off
		Features |= (IsProcessorFeaturePresent(PF_RDTSC_INSTRUCTION_AVAILABLE) != 0)   ? (feature::tsc | feature::intel486) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_COMPARE_EXCHANGE_DOUBLE) != 0)       ? (feature::cx8 | feature::intel486) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_MMX_INSTRUCTIONS_AVAILABLE) != 0)    ? (feature::mmx | feature::intel486) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_3DNOW_INSTRUCTIONS_AVAILABLE) != 0)  ? (feature::x3dnow) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) != 0)   ? (feature::sse | feature::mmxext | feature::fxsr | feature::cmov) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != 0) ? (feature::sse2) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE) != 0)   ? (feature::sse3) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_SSSE3_INSTRUCTIONS_AVAILABLE) != 0)  ? (feature::ssse3) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_SSE4_1_INSTRUCTIONS_AVAILABLE) != 0) ? (feature::sse4_1) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_SSE4_2_INSTRUCTIONS_AVAILABLE) != 0) ? (feature::sse4_1) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_AVX_INSTRUCTIONS_AVAILABLE) != 0)    ? (feature::avx | feature::xsave) : feature::none;
		Features |= (IsProcessorFeaturePresent(PF_AVX2_INSTRUCTIONS_AVAILABLE) != 0)   ? (feature::avx2 | feature::fma | feature::bmi1) : feature::none;
		Modes |= (IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE) != 0)   ? (mode::xmm128sse) : mode::base;
		Modes |= (IsProcessorFeaturePresent(PF_XMMI64_INSTRUCTIONS_AVAILABLE) != 0) ? (mode::xmm128sse) : mode::base;
		Modes |= (IsProcessorFeaturePresent(PF_SSE3_INSTRUCTIONS_AVAILABLE) != 0)   ? (mode::xmm128sse) : mode::base;
		Modes |= (IsProcessorFeaturePresent(PF_SSSE3_INSTRUCTIONS_AVAILABLE) != 0)  ? (mode::xmm128sse) : mode::base;
		Modes |= (IsProcessorFeaturePresent(PF_SSE4_1_INSTRUCTIONS_AVAILABLE) != 0) ? (mode::xmm128sse) : mode::base;
		Modes |= (IsProcessorFeaturePresent(PF_SSE4_2_INSTRUCTIONS_AVAILABLE) != 0) ? (mode::xmm128sse) : mode::base;
		Modes |= (IsProcessorFeaturePresent(PF_AVX_INSTRUCTIONS_AVAILABLE) != 0)    ? (mode::xmm128sse | mode::ymm256avx) : mode::base;
		Modes |= (IsProcessorFeaturePresent(PF_AVX2_INSTRUCTIONS_AVAILABLE) != 0)   ? (mode::xmm128sse | mode::ymm256avx) : mode::base;
		// clang-format on
#endif

#elif MPT_OS_DJGPP

		const uint8 cpu_level = detect_cpu_level();
		Features |= (cpu_level >= 3) ? featureset::intel386 : feature::none;
		Features |= (cpu_level >= 4) ? featureset::intel486SX : feature::none;
		const uint8 fpu_level = detect_fpu_level();
		Features |= (fpu_level >= 2) ? feature::fpu : feature::none;
		Features |= (fpu_level >= 3) ? feature::fsin : feature::none;

#endif

#endif // MPT_ARCH_X86 || MPT_ARCH_AMD64
	}
};



struct floating_point {

public:

	static inline constexpr uint16 FCW_IM = (1 << 0);
	static inline constexpr uint16 FCW_DM = (1 << 1);
	static inline constexpr uint16 FCW_ZM = (1 << 2);
	static inline constexpr uint16 FCW_OM = (1 << 3);
	static inline constexpr uint16 FCW_UM = (1 << 4);
	static inline constexpr uint16 FCW_PM = (1 << 5);
	static inline constexpr uint16 FCW_PC = 0x0300;
	static inline constexpr uint16 FCW_RC = 0x0c00;
	static inline constexpr uint16 FCW_X = (1 << 12);

	static inline constexpr uint32 MXCSR_IE = (1 << 0);
	static inline constexpr uint32 MXCSR_DE = (1 << 1);
	static inline constexpr uint32 MXCSR_ZE = (1 << 2);
	static inline constexpr uint32 MXCSR_OE = (1 << 3);
	static inline constexpr uint32 MXCSR_UE = (1 << 4);
	static inline constexpr uint32 MXCSR_PE = (1 << 5);
	static inline constexpr uint32 MXCSR_DAZ = (1 << 6);
	static inline constexpr uint32 MXCSR_IM = (1 << 7);
	static inline constexpr uint32 MXCSR_DM = (1 << 8);
	static inline constexpr uint32 MXCSR_ZM = (1 << 9);
	static inline constexpr uint32 MXCSR_OM = (1 << 10);
	static inline constexpr uint32 MXCSR_UM = (1 << 11);
	static inline constexpr uint32 MXCSR_PM = (1 << 12);
	static inline constexpr uint32 MXCSR_RC = (1 << 13) | (1 << 14);
	static inline constexpr uint32 MXCSR_FTZ = (1 << 15);

	enum class precision : uint8 {
		single24 = 0,
		reserved = 1,
		double53 = 2,
		extended64 = 3,
	};

	enum class rounding : uint8 {
		nearest = 0,
		down = 1,
		up = 2,
		zero = 3,
	};

	struct alignas(16) fxsave_state {
		uint16 fcw;
		uint16 fsw;
		uint16 ftw;
		uint16 fop;
		uint32 fip;
		uint32 fcs;
		uint32 foo;
		uint32 fos;
		uint32 mxcsr;
		uint32 mxcsr_mask;
		uint32 st_space[32];
		uint32 xmm_space[32];
		uint8 padding[224];
	};
	static_assert(sizeof(fxsave_state) == 512);

	struct control_state {
		uint8 x87_level = 0;
		uint16 x87fcw = 0; // default 0x37f (glibc) / 0x27f (msvc)
		uint32 mxcsr_mask = 0;
		uint32 mxcsr = 0; // default: 0x00001f80
	};

#if MPT_ARCH_X86

#if MPT_COMPILER_MSVC

	[[nodiscard]] static MPT_FORCEINLINE uint16 get_x87fcw() noexcept {
		uint16 tmp = 0;
		// clang-format off
		_asm {
			fwait
			fnstcw tmp
		}
		// clang-format on
		return tmp;
	}

	static MPT_FORCEINLINE void set_x87fcw(uint16 fcw) noexcept {
		// clang-format off
		_asm {
			fldcw fcw
		}
		// clang-format on
	}

	[[nodiscard]] static MPT_FORCEINLINE uint32 get_mxcsr() noexcept {
		return _mm_getcsr();
	}

	static MPT_FORCEINLINE void set_mxcsr(uint32 csr) noexcept {
		_mm_setcsr(csr);
	}

	static MPT_FORCEINLINE void fxsave(fxsave_state * state) noexcept {
		_fxsave(state);
	}

	static MPT_FORCEINLINE void fxrstor(const fxsave_state * state) noexcept {
		_fxrstor(state);
	}

#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG

	[[nodiscard]] static MPT_FORCEINLINE uint16 get_x87fcw() noexcept {
		typedef unsigned int fpu_control_t __attribute__((__mode__(__HI__)));
		fpu_control_t tmp = 0;
		// clang-format off
		__asm__ __volatile__("fwait" "\n" "fnstcw %0" : "=m" (*&tmp));
		// clang-format on
		return static_cast<uint16>(tmp);
	}

	static MPT_FORCEINLINE void set_x87fcw(uint16 fcw) noexcept {
		typedef unsigned int fpu_control_t __attribute__((__mode__(__HI__)));
		fpu_control_t tmp = fcw;
		// clang-format off
		__asm__ __volatile__("fldcw %0" : : "m" (*&tmp));
		// clang-format on
	}

	[[nodiscard]] static MPT_FORCEINLINE uint32 get_mxcsr() noexcept {
#ifdef MPT_ARCH_X86_SSE
		return __builtin_ia32_stmxcsr();
#else
		uint32 csr = 0;
		// clang-format off
		__asm__ __volatile__("stmxcsr %0" : "=m" (csr));
		// clang-format on
		return csr;
#endif
	}

	static MPT_FORCEINLINE void set_mxcsr(uint32 csr) noexcept {
#ifdef MPT_ARCH_X86_SSE
#if MPT_COMPILER_GCC
		// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55752
		std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
		__builtin_ia32_ldmxcsr(csr);
#if MPT_COMPILER_GCC
		// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55752
		std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
#else
		// clang-format off
		__asm__ __volatile__("ldmxcsr %0" : : "m" (csr));
		// clang-format on
#endif
	}

	static MPT_FORCEINLINE void fxsave(fxsave_state * state) noexcept {
#ifdef MPT_ARCH_X86_FXSR
		__builtin_ia32_fxsave(state);
#else
		// clang-format off
		__asm__ __volatile__("fxsave %0" : : "m" (*state));
		// clang-format on
#endif
	}

	static MPT_FORCEINLINE void fxrstor(const fxsave_state * state) noexcept {
#ifdef MPT_ARCH_X86_FXSR
		__builtin_ia32_fxrstor(const_cast<fxsave_state *>(state));
#else
		// clang-format off
		__asm__ __volatile__("fxrstor %0" : : "m" (*state));
		// clang-format on
#endif
	}

#endif // MPT_COMPILER

	static MPT_FORCEINLINE bool have_fxsr() noexcept {
#ifdef MPT_ARCH_X86_FXSR
		return true;
#else
		return cpu_info{}[mpt::arch::x86::feature::fxsr];
#endif
	}

	static MPT_FORCEINLINE bool have_sse() noexcept {
#ifdef MPT_ARCH_X86_SSE
		return true;
#else
		const cpu_info cpu_info;
		return cpu_info[mpt::arch::x86::feature::sse] && cpu_info[mpt::arch::x86::mode::xmm128sse];
#endif
	}

	static MPT_FORCEINLINE uint8 get_fpu_level() noexcept {
#ifdef MPT_ARCH_X86_FSIN
		return 3;
#elif defined(MPT_ARCH_X86_FPU)
		return cpu_info{}[mpt::arch::x86::feature::fsin] ? 3 : 2;
#else
		cpu_info tmp{};
		return tmp[mpt::arch::x86::feature::fsin] ? 3 : tmp[mpt::arch::x86::feature::fpu] ? 2
																						  : 0;
#endif
	}

	static MPT_FORCEINLINE control_state get_state() noexcept {
		control_state result;
#ifdef MPT_ARCH_X86_FXSR
		fxsave_state tmp = {};
		fxsave(&tmp);
		result.x87_level = 3;
		result.x87fcw = tmp.fcw;
		result.mxcsr_mask = tmp.mxcsr_mask;
		result.mxcsr = tmp.mxcsr;
#else
		if (have_fxsr()) {
			fxsave_state tmp = {};
			fxsave(&tmp);
			result.x87_level = 3;
			result.x87fcw = tmp.fcw;
			result.mxcsr_mask = tmp.mxcsr_mask;
			result.mxcsr = tmp.mxcsr;
		} else {
			result.x87_level = get_fpu_level();
			if (result.x87_level > 0) {
				result.x87fcw = get_x87fcw();
			}
		}
#endif
		return result;
	}

	static MPT_FORCEINLINE void set_state(control_state state) noexcept {
#ifdef MPT_ARCH_X86_SSE
		if (state.x87_level) {
			set_x87fcw(state.x87fcw);
		}
		if (state.mxcsr_mask) {
			set_mxcsr(state.mxcsr);
		}
#else
		if (have_sse()) {
			if (state.x87_level) {
				set_x87fcw(state.x87fcw);
			}
			if (state.mxcsr_mask) {
				set_mxcsr(state.mxcsr);
			}
		} else {
#ifdef MPT_ARCH_X86_FXSR
			fxsave_state tmp = {};
			fxsave(&tmp);
			if (state.x87_level) {
				tmp.fcw = state.x87fcw;
			}
			if (state.mxcsr_mask) {
				tmp.mxcsr = state.mxcsr;
			}
			fxrstor(&tmp);
#else
			if (have_fxsr()) {
				fxsave_state tmp = {};
				fxsave(&tmp);
				if (state.x87_level) {
					tmp.fcw = state.x87fcw;
				}
				if (state.mxcsr_mask) {
					tmp.mxcsr = state.mxcsr;
				}
				fxrstor(&tmp);
			} else {
				if (state.x87_level) {
					set_x87fcw(state.x87fcw);
				}
			}
#endif
		}
#endif
	}

#elif MPT_ARCH_AMD64

#if MPT_COMPILER_MSVC

	[[nodiscard]] static MPT_FORCEINLINE uint16 get_x87fcw() noexcept {
		fxsave_state state = {};
		fxsave(&state);
		return state.fcw;
	}

	static MPT_FORCEINLINE void set_x87fcw(uint16 fcw) noexcept {
		fxsave_state state = {};
		fxsave(&state);
		state.fcw = fcw;
		fxrstor(&state);
	}

	[[nodiscard]] static MPT_FORCEINLINE uint32 get_mxcsr() noexcept {
		return _mm_getcsr();
	}

	static MPT_FORCEINLINE void set_mxcsr(uint32 csr) noexcept {
		_mm_setcsr(csr);
	}

	static MPT_FORCEINLINE void fxsave(fxsave_state * state) noexcept {
		_fxsave(state);
	}

	static MPT_FORCEINLINE void fxrstor(const fxsave_state * state) noexcept {
		_fxrstor(state);
	}

	static MPT_FORCEINLINE bool have_fxsr() noexcept {
		return true;
	}

	static MPT_FORCEINLINE control_state get_state() noexcept {
		control_state result;
		fxsave_state tmp = {};
		fxsave(&tmp);
		result.x87_level = 3;
		result.x87fcw = tmp.fcw;
		result.mxcsr_mask = tmp.mxcsr_mask;
		result.mxcsr = tmp.mxcsr;
		return result;
	}

	static MPT_FORCEINLINE void set_state(control_state state) noexcept {
		fxsave_state tmp = {};
		fxsave(&tmp);
		tmp.fcw = state.x87fcw;
		tmp.mxcsr_mask = state.mxcsr_mask;
		tmp.mxcsr = state.mxcsr;
		fxrstor(&tmp);
	}

#elif MPT_COMPILER_GCC || MPT_COMPILER_CLANG

	[[nodiscard]] static MPT_FORCEINLINE uint16 get_x87fcw() noexcept {
		typedef unsigned int fpu_control_t __attribute__((__mode__(__HI__)));
		fpu_control_t tmp = 0;
		// clang-format off
		__asm__ __volatile__("fwait" "\n" "fnstcw %0" : "=m" (*&tmp));
		// clang-format on
		return static_cast<uint16>(tmp);
	}

	static MPT_FORCEINLINE void set_x87fcw(uint16 fcw) noexcept {
		typedef unsigned int fpu_control_t __attribute__((__mode__(__HI__)));
		fpu_control_t tmp = fcw;
		// clang-format off
		__asm__ __volatile__("fldcw %0" : : "m" (*&tmp));
		// clang-format on
	}

	[[nodiscard]] static MPT_FORCEINLINE uint32 get_mxcsr() noexcept {
		return __builtin_ia32_stmxcsr();
	}

	static MPT_FORCEINLINE void set_mxcsr(uint32 csr) noexcept {
#if MPT_COMPILER_GCC
		// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55752
		std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
		__builtin_ia32_ldmxcsr(csr);
#if MPT_COMPILER_GCC
		// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=55752
		std::atomic_thread_fence(std::memory_order_seq_cst);
#endif
	}

	static MPT_FORCEINLINE void fxsave(fxsave_state * state) noexcept {
		__builtin_ia32_fxsave(state);
	}

	static MPT_FORCEINLINE void fxrstor(const fxsave_state * state) noexcept {
		__builtin_ia32_fxrstor(const_cast<fxsave_state *>(state));
	}

	static MPT_FORCEINLINE bool have_fxsr() noexcept {
		return true;
	}

	static MPT_FORCEINLINE control_state get_state() noexcept {
		control_state result;
		result.x87_level = 3;
		result.x87fcw = get_x87fcw();
		result.mxcsr_mask = 0x0000'ffff;
		result.mxcsr = get_mxcsr();
		return result;
	}

	static MPT_FORCEINLINE void set_state(control_state state) noexcept {
		set_x87fcw(state.x87fcw);
		set_mxcsr(state.mxcsr);
	}

#endif // MPT_COMPILER

	class guard {

	private:

		const control_state m_oldstate;

	public:

		MPT_FORCEINLINE guard(std::optional<rounding> rounding, std::optional<bool> denormals_as_zero, std::optional<precision> precision, std::optional<bool> infinity_projective) noexcept
			: m_oldstate(get_state()) {
			control_state state = m_oldstate;
			if (rounding) {
				if (state.x87_level) {
					state.x87fcw = (state.x87fcw & ~FCW_RC) | (mpt::to_underlying(*rounding) << mpt::countr_zero(FCW_RC));
				}
				if ((state.mxcsr_mask & MXCSR_RC) == MXCSR_RC) {
					state.mxcsr = (state.mxcsr & ~MXCSR_RC) | (mpt::to_underlying(*rounding) << mpt::countr_zero(MXCSR_RC));
				}
			}
			if (denormals_as_zero) {
				if (state.mxcsr_mask) {
					state.mxcsr = (state.mxcsr & ~(MXCSR_FTZ | MXCSR_DAZ)) | ((*denormals_as_zero) ? ((MXCSR_FTZ | MXCSR_DAZ) & state.mxcsr_mask) : 0);
				}
			}
			if (precision) {
				if (state.x87_level) {
					state.x87fcw = (state.x87fcw & ~FCW_PC) | (mpt::to_underlying(*precision) << mpt::countr_zero(FCW_PC));
				}
			}
			if (infinity_projective) {
				if (state.x87_level <= 2) {
					state.x87fcw = (state.x87fcw & ~FCW_X) | ((*infinity_projective) ? 0 : FCW_X);
				}
			}
			set_state(state);
		}

		MPT_FORCEINLINE ~guard() {
			set_state(m_oldstate);
		}
	};

#endif // MPT_ARCH
};



} // namespace x86

namespace amd64 = x86;



} // namespace arch



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_ARCH_X86_AMD64_HPP
