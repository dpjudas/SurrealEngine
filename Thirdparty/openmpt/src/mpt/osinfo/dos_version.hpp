/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_OSINFO_DOS_VERSION_HPP
#define MPT_OSINFO_DOS_VERSION_HPP



#include "mpt/base/detect.hpp"
#include "mpt/base/integer.hpp"
#include "mpt/base/namespace.hpp"
#include "mpt/string/buffer.hpp"

#include <string>

#if MPT_OS_DJGPP
#include <dos.h>
#include <dpmi.h>
#include <errno.h>
#include <go32.h>
#include <sys/farptr.h>
#endif // MPT_OS_DJGPP



namespace mpt {
inline namespace MPT_INLINE_NS {



namespace osinfo {

namespace dos {



class Version {

public:

	enum class Host {
		DOS,
		Win2,
		Win3,
		Win95,
		Win98,
		WinME,
		WinNT,
		OS2,
		OS2Warp,
		DOSEmu,
	};

	struct System {
		uint8 Major = 0;
		uint8 Minor = 0;
		constexpr System() noexcept
			: Major(0)
			, Minor(0) {
		}
		explicit constexpr System(uint16 number) noexcept
			: Major(static_cast<uint8>((number >> 8) & 0xffu))
			, Minor(static_cast<uint8>((number >> 0) & 0xffu)) {
		}
		explicit constexpr System(uint8 major, uint8 minor) noexcept
			: Major(major)
			, Minor(minor) {
		}
		constexpr operator uint16() const noexcept {
			return (static_cast<uint16>(Major) << 8) | (static_cast<uint16>(Minor) << 0);
		}
	};

	struct DPMI {
		uint8 Major = 0;
		uint8 Minor = 0;
		constexpr DPMI() noexcept
			: Major(0)
			, Minor(0) {
		}
		explicit constexpr DPMI(uint8 major, uint8 minor) noexcept
			: Major(major)
			, Minor(minor) {
		}
	};

private:
	bool m_SystemIsDos = false;

	std::string m_OEM{};
	System m_System;
	System m_SystemEmulated;

	DPMI m_DPMI;
	std::string m_DPMIVendor{};
	DPMI m_DPMIHost;

	Host m_Host = Host::DOS;
	int m_HostVersion = 0;
	int m_HostRevision = 0;
	int m_HostPatch = 0;
	bool m_HostMultitasking = false;
	bool m_HostFixedTimer = false;

	std::string m_BIOSDate{};

private:
	Version() {
		return;
	}

public:
	static Version NoDos() {
		return Version();
	}

	Version(std::string oem, System version, System version_emulated, DPMI dpmi, std::string dpmi_vendor, DPMI dpmi_host, Host host, int host_version, int host_revision, int host_patch, bool multitasking, bool fixedtimer, std::string bios_date)
		: m_SystemIsDos(true)
		, m_OEM(oem)
		, m_System(version)
		, m_SystemEmulated(version_emulated)
		, m_DPMI(dpmi)
		, m_DPMIVendor(dpmi_vendor)
		, m_DPMIHost(dpmi_host)
		, m_Host(host)
		, m_HostVersion(host_version)
		, m_HostRevision(host_revision)
		, m_HostPatch(host_patch)
		, m_HostMultitasking(multitasking)
		, m_HostFixedTimer(fixedtimer)
		, m_BIOSDate(bios_date) {
		return;
	}

public:

#if MPT_OS_DJGPP
	static mpt::osinfo::dos::Version GatherDosVersion() {
		uint16 dos_version = _get_dos_version(0);
		uint16 dos_version_emulated = dos_version;
		if (dos_version >= 0x0500) {
			dos_version = _get_dos_version(1);
		}
		uint8 dpmi_version_major = 0;
		uint8 dpmi_version_minor = 0;
		__dpmi_version_ret dpmi_version{};
		if (__dpmi_get_version(&dpmi_version) == 0) {
			dpmi_version_major = dpmi_version.major;
			dpmi_version_minor = dpmi_version.minor;
		}
		std::string dpmi_host_vendor;
		uint8 dpmi_host_major = 0;
		uint8 dpmi_host_minor = 0;
		/* if (dpmi_version_major >= 1) */ {
			int dpmi_flags = 0;
			std::array<char, 128> dpmi_vendor = {};
			if (__dpmi_get_capabilities(&dpmi_flags, dpmi_vendor.data()) == 0) {
				char buf[126] = {};
				std::memcpy(buf, dpmi_vendor.data() + 2, 126);
				dpmi_host_vendor = mpt::ReadAutoBuf(buf);
				dpmi_host_major = dpmi_vendor[0];
				dpmi_host_minor = dpmi_vendor[1];
			}
		}
		if (dpmi_host_vendor.empty()) {
			dpmi_host_vendor = "unknown";
		}
		bool detected = false;
		Host host = Host::DOS;
		int host_version = 0;
		int host_revision = 0;
		int host_patch = 0;
		bool host_multitasking = false;
		bool host_fixedtimer = false;
		if (!detected) {
			char * os = std::getenv("OS");
			if (os) {
				if (std::string(os) == std::string("Windows_NT")) {
					host = Host::WinNT;
					host_version = 0;
					host_revision = 0;
					host_patch = 0;
					host_multitasking = true;
					host_fixedtimer = true;
					detected = true;
				}
			}
		}
		if (!detected) {
			__dpmi_regs r{};
			r.x.ax = 0x1600;
			__dpmi_int(0x2f, &r);
			if ((r.h.al == 0x01) || (r.h.al == 0xFF)) {
				host = Host::Win2;
				host_version = 2;
				host_revision = 0;
				host_patch = 0;
				host_multitasking = true;
				host_fixedtimer = true;
				detected = true;
			} else if ((r.h.al != 0x00) && (r.h.al != 0x01) && (r.h.al != 0x80) && (r.h.al != 0xFF)) {
				host_version = r.h.al;
				host_revision = r.h.ah;
				host_patch = 0;
				if (host_version > 4) {
					host = Host::WinME;
				} else if ((host_version == 4) && (host_revision >= 90)) {
					host = Host::WinME;
				} else if ((host_version == 4) && (host_revision >= 10)) {
					host = Host::Win98;
				} else if (host_version == 4) {
					host = Host::Win95;
				} else {
					host = Host::Win3;
				}
				host_multitasking = true;
				host_fixedtimer = true;
				detected = true;
			}
		}
		if (!detected) {
			__dpmi_regs r{};
			r.x.ax = 0x4010;
			__dpmi_int(0x2f, &r);
			if (r.x.ax != 0x4010) {
				if (r.x.ax == 0x0000u) {
					host = Host::OS2Warp;
				} else {
					host = Host::OS2;
				}
				host_version = r.h.bh;
				host_revision = r.h.bl;
				host_patch = 0;
				host_multitasking = true;
				host_fixedtimer = true;
				detected = true;
			}
		}
		if (!detected) {
			char buf[9] = {};
			for (uint32 i = 0; i < 8; ++i) {
				buf[i] = _farpeekb(_dos_ds, (0xF000u * 16) + (0xfff5u + i));
			}
			buf[8] = '\0';
			if (std::string(mpt::ReadAutoBuf(buf)) == std::string("02/25/93")) {
				__dpmi_regs r{};
				r.x.ax = 0;
				__dpmi_int(0xe6, &r);
				if (r.x.ax == 0xaa55u) {
					host = Host::DOSEmu;
					host_version = r.h.bh;
					host_revision = r.h.bl;
					host_patch = r.x.cx;
					host_multitasking = true;
					host_fixedtimer = true;
					detected = true;
				}
			}
		}
		if (!detected) {
			__dpmi_regs r{};
			r.x.ax = 0x3000;
			__dpmi_int(0x21, &r);
			host = Host::DOS;
			host_version = r.h.al;
			host_revision = r.h.ah;
			host_patch = 0;
			host_multitasking = false;
			host_fixedtimer = false;
			detected = true;
		}
		if (!host_multitasking) {
			errno = 0;
			__dpmi_yield();
			if (errno == 0) {
				host_multitasking = true;
			}
			errno = 0;
		}
		std::string bios_date{};
		{
			{
				char buf[8 + 1] = {};
				for (uint32 i = 0; i < 8; ++i) {
					buf[i] = _farpeekb(_dos_ds, (0xf000u * 16) + (0xfff5u + i));
				}
				bios_date = mpt::ReadAutoBuf(buf);
			}
		}
		return mpt::osinfo::dos::Version(
			_os_flavor,
			mpt::osinfo::dos::Version::System(dos_version),
			mpt::osinfo::dos::Version::System(dos_version_emulated),
			mpt::osinfo::dos::Version::DPMI(dpmi_version_major, dpmi_version_minor),
			dpmi_host_vendor,
			mpt::osinfo::dos::Version::DPMI(dpmi_host_major, dpmi_host_minor),
			host,
			host_version,
			host_revision,
			host_patch,
			host_multitasking,
			host_fixedtimer,
			bios_date);
	}

#endif // MPT_OS_DJGPP

public:
	static inline mpt::osinfo::dos::Version Current() {
#if MPT_OS_DJGPP
		static mpt::osinfo::dos::Version s_cachedVersion = GatherDosVersion();
		return s_cachedVersion;
#else  // !MPT_OS_DJGPP
		return mpt::osinfo::dos::Version::NoDos();
#endif // MPT_OS_DJGPP
	}

public:
	bool IsDos() const noexcept {
		return m_SystemIsDos;
	}

	std::string GetOEM() const {
		return m_OEM;
	}

	mpt::osinfo::dos::Version::System GetSystem() const noexcept {
		return m_System;
	}

	mpt::osinfo::dos::Version::System GetSystemEmulated() const noexcept {
		return m_SystemEmulated;
	}

	mpt::osinfo::dos::Version::DPMI GetDPMI() const noexcept {
		return m_DPMI;
	}

	std::string GetDPMIVendor() const {
		return m_DPMIVendor;
	}

	mpt::osinfo::dos::Version::DPMI GetDPMIHost() const noexcept {
		return m_DPMIHost;
	}

	mpt::osinfo::dos::Version::Host GetHost() const noexcept {
		return m_Host;
	}

	std::string GetHostName() const {
		std::string result{};
		switch (m_Host) {
			case Host::DOS:
				result = "DOS";
				break;
			case Host::Win2:
				result = "Windows/386 2.x";
				break;
			case Host::Win3:
				result = "Windows 3.x";
				break;
			case Host::Win95:
				result = "Windows 95";
				break;
			case Host::Win98:
				result = "Windows 98";
				break;
			case Host::WinME:
				result = "Windows ME";
				break;
			case Host::WinNT:
				result = "Windows NT";
				break;
			case Host::OS2:
				result = "OS/2";
				break;
			case Host::OS2Warp:
				result = "OS/2 Warp";
				break;
			case Host::DOSEmu:
				result = "DOSEmu";
				break;
		}
		return result;
	}

	int GetHostVersion() const noexcept {
		return m_HostVersion;
	}

	int GetHostRevision() const noexcept {
		return m_HostRevision;
	}

	int GetHostPatch() const noexcept {
		return m_HostPatch;
	}

	bool IsHostMultitasking() const noexcept {
		return m_HostMultitasking;
	}

	bool HasHostFixedTimer() const noexcept {
		return m_HostFixedTimer;
	}

	std::string GetBIOSDate() const {
		return m_BIOSDate;
	}

}; // class Version


} // namespace dos

} // namespace osinfo



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_OSINFO_DOS_VERSION_HPP
