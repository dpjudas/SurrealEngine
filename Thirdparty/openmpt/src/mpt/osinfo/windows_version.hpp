/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_OSINFO_WINDOWS_VERSION_HPP
#define MPT_OSINFO_WINDOWS_VERSION_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"

#if MPT_OS_WINDOWS
#include <windows.h>
#endif // MPT_OS_WINDOWS



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace osinfo {

namespace windows {



class Version {

public:
	enum Number : uint64 {
		WinNT4 = 0x0000000400000000ull,
		Win2000 = 0x0000000500000000ull,
		WinXP = 0x0000000500000001ull,
		WinXP64 = 0x0000000500000002ull,
		WinVista = 0x0000000600000000ull,
		Win7 = 0x0000000600000001ull,
		Win8 = 0x0000000600000002ull,
		Win81 = 0x0000000600000003ull,
		Win10Pre = 0x0000000600000004ull,
		Win10 = 0x0000000a00000000ull,
		WinNewer = Win10 + 1ull
	};

	struct System {
		uint32 Major = 0;
		uint32 Minor = 0;
		constexpr System() noexcept
			: Major(0)
			, Minor(0) {
		}
		constexpr System(Number number) noexcept
			: Major(static_cast<uint32>((static_cast<uint64>(number) >> 32) & 0xffffffffu))
			, Minor(static_cast<uint32>((static_cast<uint64>(number) >> 0) & 0xffffffffu)) {
		}
		explicit constexpr System(uint64 number) noexcept
			: Major(static_cast<uint32>((number >> 32) & 0xffffffffu))
			, Minor(static_cast<uint32>((number >> 0) & 0xffffffffu)) {
		}
		explicit constexpr System(uint32 major, uint32 minor) noexcept
			: Major(major)
			, Minor(minor) {
		}
		constexpr operator uint64() const noexcept {
			return (static_cast<uint64>(Major) << 32) | (static_cast<uint64>(Minor) << 0);
		}
	};

	struct ServicePack {
		uint16 Major = 0;
		uint16 Minor = 0;
		constexpr ServicePack() noexcept
			: Major(0)
			, Minor(0) {
		}
		explicit constexpr ServicePack(uint16 major, uint16 minor) noexcept
			: Major(major)
			, Minor(minor) {
		}
		constexpr bool HasServicePack() const noexcept {
			return Major != 0 || Minor != 0;
		}
		constexpr operator uint32() const noexcept {
			return (static_cast<uint32>(Major) << 16) | (static_cast<uint32>(Minor) << 0);
		}
	};

	typedef uint32 Build;

	typedef uint32 TypeId;

private:
	bool m_SystemIsWindows;

	System m_System;
	ServicePack m_ServicePack;
	Build m_Build;
	TypeId m_Type;

private:
	constexpr Version() noexcept
		: m_SystemIsWindows(false)
		, m_System()
		, m_ServicePack()
		, m_Build()
		, m_Type() {
	}

public:
	static constexpr Version NoWindows() noexcept {
		return Version();
	}

	static constexpr Version AnyWindows() noexcept {
		Version result = Version();
		result.m_SystemIsWindows = true;
		return result;
	}

	constexpr Version(mpt::osinfo::windows::Version::System system, mpt::osinfo::windows::Version::ServicePack servicePack, mpt::osinfo::windows::Version::Build build, mpt::osinfo::windows::Version::TypeId type) noexcept
		: m_SystemIsWindows(true)
		, m_System(system)
		, m_ServicePack(servicePack)
		, m_Build(build)
		, m_Type(type) {
	}

public:
#if MPT_OS_WINDOWS

	static mpt::osinfo::windows::Version FromSDK() noexcept {
		// Initialize to used SDK version
#if MPT_WINNT_AT_LEAST(MPT_WIN_11_24H2)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 26100, 0);
//#elif MPT_WINNT_AT_LEAST(MPT_WIN_11_23H2)
//		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 22631, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_11_22H2)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 22621, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_11) // 21H2
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 22000, 0);
//#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_22H2)
//		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 19045, 0);
//#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_21H2)
//		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 19044, 0);
//#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_21H1)
//		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 19043, 0);
//#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_20H2)
//		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 19042, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_2004)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 19041, 0);
//#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_1909)
//		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 18363, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_1903)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 18362, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_1809)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 17763, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_1803)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 17134, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_1709)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 16299, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_1703)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 15063, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_1607)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 14393, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_1511)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 10586, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10) // 1507
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10, mpt::osinfo::windows::Version::ServicePack(0, 0), 10240, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_10_PRE)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win10Pre, mpt::osinfo::windows::Version::ServicePack(((NTDDI_VERSION & 0xffffu) >> 8) & 0xffu, ((NTDDI_VERSION & 0xffffu) >> 0) & 0xffu), 0, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_81)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win81, mpt::osinfo::windows::Version::ServicePack(((NTDDI_VERSION & 0xffffu) >> 8) & 0xffu, ((NTDDI_VERSION & 0xffffu) >> 0) & 0xffu), 0, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_8)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win8, mpt::osinfo::windows::Version::ServicePack(((NTDDI_VERSION & 0xffffu) >> 8) & 0xffu, ((NTDDI_VERSION & 0xffffu) >> 0) & 0xffu), 0, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_7)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win7, mpt::osinfo::windows::Version::ServicePack(((NTDDI_VERSION & 0xffffu) >> 8) & 0xffu, ((NTDDI_VERSION & 0xffffu) >> 0) & 0xffu), 0, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::WinVista, mpt::osinfo::windows::Version::ServicePack(((NTDDI_VERSION & 0xffffu) >> 8) & 0xffu, ((NTDDI_VERSION & 0xffffu) >> 0) & 0xffu), 0, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_XP64)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::WinXP64, mpt::osinfo::windows::Version::ServicePack(((NTDDI_VERSION & 0xffffu) >> 8) & 0xffu, ((NTDDI_VERSION & 0xffffu) >> 0) & 0xffu), 0, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_XP)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::WinXP, mpt::osinfo::windows::Version::ServicePack(((NTDDI_VERSION & 0xffffu) >> 8) & 0xffu, ((NTDDI_VERSION & 0xffffu) >> 0) & 0xffu), 0, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_2000)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::Win2000, mpt::osinfo::windows::Version::ServicePack(((NTDDI_VERSION & 0xffffu) >> 8) & 0xffu, ((NTDDI_VERSION & 0xffffu) >> 0) & 0xffu), 0, 0);
#elif MPT_WINNT_AT_LEAST(MPT_WIN_NT4)
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::WinNT4, mpt::osinfo::windows::Version::ServicePack(((NTDDI_VERSION & 0xffffu) >> 8) & 0xffu, ((NTDDI_VERSION & 0xffffu) >> 0) & 0xffu), 0, 0);
#else
		return mpt::osinfo::windows::Version(mpt::osinfo::windows::Version::System((static_cast<uint64>(MPT_WIN_VERSION) & 0xff000000u) >> 24, (static_cast<uint64>(MPT_WIN_VERSION) & 0x00ff0000u) >> 16), mpt::osinfo::windows::Version::ServicePack((static_cast<uint64>(MPT_WIN_VERSION) & 0x0000ff00u) >> 8, (static_cast<uint64>(MPT_WIN_VERSION) & 0x000000ffu) >> 0), 0, 0);
#endif
	}

	static mpt::osinfo::windows::Version GatherWindowsVersion() noexcept {
#if MPT_OS_WINDOWS_WINRT
		return mpt::osinfo::windows::Version::FromSDK();
#else // !MPT_OS_WINDOWS_WINRT
		OSVERSIONINFOEXW versioninfoex{};
		versioninfoex.dwOSVersionInfoSize = sizeof(versioninfoex);
#if MPT_COMPILER_MSVC
#pragma warning(push)
#pragma warning(disable : 4996)  // 'GetVersionExW': was declared deprecated
#pragma warning(disable : 28159) // Consider using 'IsWindows*' instead of 'GetVersionExW'. Reason: Deprecated. Use VerifyVersionInfo* or IsWindows* macros from VersionHelpers.
#endif                           // MPT_COMPILER_MSVC
#if MPT_COMPILER_CLANG
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif // MPT_COMPILER_CLANG
		if (GetVersionExW((LPOSVERSIONINFOW)&versioninfoex) == FALSE) {
			return mpt::osinfo::windows::Version::FromSDK();
		}
#if MPT_COMPILER_MSVC
#pragma warning(pop)
#endif // MPT_COMPILER_MSVC
#if MPT_COMPILER_CLANG
#pragma clang diagnostic pop
#endif // MPT_COMPILER_CLANG
		if (versioninfoex.dwPlatformId != VER_PLATFORM_WIN32_NT) {
			return mpt::osinfo::windows::Version::FromSDK();
		}
		DWORD dwProductType = 0;
#if MPT_WINNT_AT_LEAST(MPT_WIN_VISTA)
		dwProductType = PRODUCT_UNDEFINED;
		if (GetProductInfo(versioninfoex.dwMajorVersion, versioninfoex.dwMinorVersion, versioninfoex.wServicePackMajor, versioninfoex.wServicePackMinor, &dwProductType) == FALSE) {
			dwProductType = PRODUCT_UNDEFINED;
		}
#endif
		return mpt::osinfo::windows::Version(
			mpt::osinfo::windows::Version::System(versioninfoex.dwMajorVersion, versioninfoex.dwMinorVersion),
			mpt::osinfo::windows::Version::ServicePack(versioninfoex.wServicePackMajor, versioninfoex.wServicePackMinor),
			versioninfoex.dwBuildNumber,
			dwProductType);
#endif // MPT_OS_WINDOWS_WINRT
	}

#endif // MPT_OS_WINDOWS

public:
	static inline mpt::osinfo::windows::Version Current() noexcept {
#if MPT_OS_WINDOWS
		static mpt::osinfo::windows::Version s_cachedVersion = GatherWindowsVersion();
		return s_cachedVersion;
#else  // !MPT_OS_WINDOWS
		return mpt::osinfo::windows::Version::NoWindows();
#endif // MPT_OS_WINDOWS
	}

public:
	bool IsWindows() const noexcept {
		return m_SystemIsWindows;
	}

	bool IsBefore(mpt::osinfo::windows::Version::System version) const noexcept {
		if (!m_SystemIsWindows) {
			return false;
		}
		return m_System < version;
	}

	bool IsBefore(mpt::osinfo::windows::Version::System version, mpt::osinfo::windows::Version::ServicePack servicePack) const noexcept {
		if (!m_SystemIsWindows) {
			return false;
		}
		if (m_System > version) {
			return false;
		}
		if (m_System < version) {
			return true;
		}
		return m_ServicePack < servicePack;
	}

	bool IsBefore(mpt::osinfo::windows::Version::System version, mpt::osinfo::windows::Version::Build build) const noexcept {
		if (!m_SystemIsWindows) {
			return false;
		}
		if (m_System > version) {
			return false;
		}
		if (m_System < version) {
			return true;
		}
		return m_Build < build;
	}

	bool IsBefore(mpt::osinfo::windows::Version::System version, mpt::osinfo::windows::Version::ServicePack servicePack, mpt::osinfo::windows::Version::Build build) const noexcept {
		if (!m_SystemIsWindows) {
			return false;
		}
		if (m_System > version) {
			return false;
		}
		if (m_System < version) {
			return true;
		}
		if (m_ServicePack > servicePack) {
			return false;
		}
		if (m_ServicePack < servicePack) {
			return true;
		}
		return m_Build < build;
	}

	bool IsBefore(mpt::osinfo::windows::Version version) const noexcept {
		return IsBefore(version.GetSystem(), version.GetServicePack(), version.GetBuild());
	}

	bool IsAtLeast(mpt::osinfo::windows::Version::System version) const noexcept {
		if (!m_SystemIsWindows) {
			return false;
		}
		return m_System >= version;
	}

	bool IsAtLeast(mpt::osinfo::windows::Version::System version, mpt::osinfo::windows::Version::ServicePack servicePack) const noexcept {
		if (!m_SystemIsWindows) {
			return false;
		}
		if (m_System < version) {
			return false;
		}
		if (m_System > version) {
			return true;
		}
		return m_ServicePack >= servicePack;
	}

	bool IsAtLeast(mpt::osinfo::windows::Version::System version, mpt::osinfo::windows::Version::Build build) const noexcept {
		if (!m_SystemIsWindows) {
			return false;
		}
		if (m_System < version) {
			return false;
		}
		if (m_System > version) {
			return true;
		}
		return m_Build >= build;
	}

	bool IsAtLeast(mpt::osinfo::windows::Version::System version, mpt::osinfo::windows::Version::ServicePack servicePack, mpt::osinfo::windows::Version::Build build) const noexcept {
		if (!m_SystemIsWindows) {
			return false;
		}
		if (m_System < version) {
			return false;
		}
		if (m_System > version) {
			return true;
		}
		if (m_ServicePack < servicePack) {
			return false;
		}
		if (m_ServicePack > servicePack) {
			return true;
		}
		return m_Build >= build;
	}

	bool IsAtLeast(mpt::osinfo::windows::Version version) const noexcept {
		return IsAtLeast(version.GetSystem(), version.GetServicePack(), version.GetBuild());
	}

	mpt::osinfo::windows::Version::System GetSystem() const noexcept {
		return m_System;
	}

	mpt::osinfo::windows::Version::ServicePack GetServicePack() const noexcept {
		return m_ServicePack;
	}

	mpt::osinfo::windows::Version::Build GetBuild() const noexcept {
		return m_Build;
	}

	mpt::osinfo::windows::Version::TypeId GetTypeId() const noexcept {
		return m_Type;
	}

}; // class Version



} // namespace windows

} // namespace osinfo



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_OSINFO_WINDOWS_VERSION_HPP
