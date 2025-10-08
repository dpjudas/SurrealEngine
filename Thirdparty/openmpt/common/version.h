/*
 * version.h
 * ---------
 * Purpose: OpenMPT version handling.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "mptString.h"
#include "openmpt/base/FlagSet.hpp"

#include <stdexcept>


OPENMPT_NAMESPACE_BEGIN


class Version
{

private:

	uint32 m_Version; // e.g. 0x01170208

public:

	enum class Field
	{
		Major,
		Minor,
		Patch,
		Test,
	};

public:

	static Version Current() noexcept;

public:

	MPT_CONSTEXPRINLINE Version() noexcept
		: m_Version(0)
	{}

	explicit MPT_CONSTEXPRINLINE Version(uint32 version) noexcept
		: m_Version(version)
	{}

	explicit MPT_CONSTEXPRINLINE Version(uint8 v1, uint8 v2, uint8 v3, uint8 v4) noexcept
		: m_Version((static_cast<uint32>(v1) << 24) | (static_cast<uint32>(v2) << 16) | (static_cast<uint32>(v3) << 8) | (static_cast<uint32>(v4) << 0))
	{}

public:

	mpt::ustring ToUString() const; // e.g "1.17.02.08"

	// Returns numerical version value from given version string.
	static Version Parse(const mpt::ustring &s);

public:

	explicit MPT_CONSTEXPRINLINE operator bool () const noexcept
	{
		return m_Version != 0;
	}
	MPT_CONSTEXPRINLINE bool operator ! () const noexcept
	{
		return m_Version == 0;
	}

	MPT_CONSTEXPRINLINE uint32 GetRawVersion() const noexcept
	{
		return m_Version;
	}

	MPT_CONSTEXPRINLINE Version Masked(uint32 mask) const noexcept
	{
		return Version(m_Version & mask);
	}

	MPT_CONSTEXPRINLINE uint8 GetField(Field field) const noexcept
	{
		return
			(field == Field::Major) ? static_cast<uint8>((m_Version >> 24) & 0xffu) :
			(field == Field::Minor) ? static_cast<uint8>((m_Version >> 16) & 0xffu) :
			(field == Field::Patch) ? static_cast<uint8>((m_Version >>  8) & 0xffu) :
			(field == Field::Test ) ? static_cast<uint8>((m_Version >>  0) & 0xffu) :
			0u;
	}

	// Return a version without build number (the last number in the version).
	// The current versioning scheme uses this number only for test builds, and it should be 00 for official builds,
	// So sometimes it might be wanted to do comparisons without the build number.
	Version WithoutTestNumber() const noexcept;

	Version WithoutPatchOrTestNumbers() const noexcept;

public:

	// Return a OpenMPT version string suitable for file format tags 
	mpt::ustring GetOpenMPTVersionString() const; // e.g. "OpenMPT 1.17.02.08"

	// Returns true if a given version number is from a test build, false if it's a release build.
	bool IsTestVersion() const noexcept;

public:

	struct LiteralParser
	{
	
	public:

		// Work-around for GCC 5 which complains about instanciating non-literal type inside a constexpr function when using mpt::constexpr_throw(std::runtime_error("")).
		struct ParseException {};

	private:

		static MPT_CONSTEXPRINLINE uint8 NibbleFromChar(char x)
		{
			return
				('0' <= x && x <= '9') ? static_cast<uint8>(x - '0' +  0) :
				('a' <= x && x <= 'z') ? static_cast<uint8>(x - 'a' + 10) :
				('A' <= x && x <= 'Z') ? static_cast<uint8>(x - 'A' + 10) :
				mpt::constexpr_throw<uint8>(std::domain_error(""));
		}

	public:

		static MPT_CONSTEXPRINLINE Version Parse(const char * str, std::size_t len)
		{
			// 0123456789
			// 1.23.45.67
			uint8 v[4] = {0, 0, 0, 0};
			std::size_t field = 0;
			std::size_t fieldlen = 0;
			for(std::size_t i = 0; i < len; ++i)
			{
				char c = str[i];
				if(c == '.')
				{
					if(field >= 3)
					{
						mpt::constexpr_throw(ParseException());
					}
					if(fieldlen == 0)
					{
						mpt::constexpr_throw(ParseException());
					}
					field++;
					fieldlen = 0;
				} else if(('0' <= c && c <= '9') || ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'))
				{
					fieldlen++;
					if(fieldlen > 2)
					{
						mpt::constexpr_throw(ParseException());
					}
					v[field] <<= 4;
					v[field] |= NibbleFromChar(c);
				} else
				{
					mpt::constexpr_throw(ParseException());
				}
			}
			if(fieldlen == 0)
			{
				mpt::constexpr_throw(ParseException());
			}
			return Version(v[0], v[1], v[2], v[3]);
		}

	};

};

MPT_CONSTEXPRINLINE bool operator == (const Version &a, const Version &b) noexcept
{
	return a.GetRawVersion() == b.GetRawVersion();
}
MPT_CONSTEXPRINLINE bool operator != (const Version &a, const Version &b) noexcept
{
	return a.GetRawVersion() != b.GetRawVersion();
}
MPT_CONSTEXPRINLINE bool operator <= (const Version &a, const Version &b) noexcept
{
	return a.GetRawVersion() <= b.GetRawVersion();
}
MPT_CONSTEXPRINLINE bool operator >= (const Version &a, const Version &b) noexcept
{
	return a.GetRawVersion() >= b.GetRawVersion();
}
MPT_CONSTEXPRINLINE bool operator < (const Version &a, const Version &b) noexcept
{
	return a.GetRawVersion() < b.GetRawVersion();
}
MPT_CONSTEXPRINLINE bool operator > (const Version &a, const Version &b) noexcept
{
	return a.GetRawVersion() > b.GetRawVersion();
}


MPT_CONSTEVAL Version operator ""_LiteralVersionImpl (const char * str, std::size_t len)
{
	return Version::LiteralParser::Parse(str, len);
}

// Create Version object from version string and check syntax, all at compile time.
// cppcheck false-positive
// cppcheck-suppress preprocessorErrorDirective
#define MPT_V(strver) MPT_FORCE_CONSTEVAL_VALUE(Version{( strver ## _LiteralVersionImpl ).GetRawVersion()})



class SourceInfo
{
private:
	mpt::ustring m_Url; // svn repository url (or empty string)
	int m_Revision; // svn revision (or 0)
	bool m_IsDirty; // svn working copy is dirty (or false)
	bool m_HasMixedRevisions; // svn working copy has mixed revisions (or false)
	bool m_IsPackage; // source code originates from a packaged version of the source code
	mpt::ustring m_Date; // svn date (or empty string)
private:
	SourceInfo();
public:
	static SourceInfo Current();
public:
	const mpt::ustring & Url() const { return m_Url; }
	int Revision() const { return m_Revision; }
	bool IsDirty() const { return m_IsDirty; }
	bool HasMixedRevisions() const { return m_HasMixedRevisions; }
	bool IsPackage() const { return m_IsPackage; }
	const mpt::ustring & Date() const { return m_Date; }
public:
	mpt::ustring GetUrlWithRevision() const; // i.e. "https://source.openmpt.org/svn/openmpt/trunk/OpenMPT@1234" or empty string
	mpt::ustring GetStateString() const; // i.e. "+dirty" or "clean"
};



struct VersionWithRevision
{
	Version version;
	uint64 revision;
	static VersionWithRevision Current();
	static VersionWithRevision Parse(const mpt::ustring &s);
	mpt::ustring ToUString() const;
	constexpr bool HasRevision() const noexcept
	{
		return revision != 0;
	}
	constexpr bool IsEqualTo(VersionWithRevision other) const noexcept
	{
		return version == other.version && revision == other.revision;
	}
	constexpr bool IsEquivalentTo(VersionWithRevision other) const noexcept
	{
		if(version == other.version && revision == other.revision)
		{
			return true;
		}
		if(HasRevision() && other.HasRevision())
		{
			return false;
		}
		return version == other.version;
	}
	constexpr bool IsNewerThan(VersionWithRevision other) const noexcept
	{
		if(version < other.version)
		{
			return false;
		}
		if(version > other.version)
		{
			return true;
		}
		if(!HasRevision() && !other.HasRevision())
		{
			return false;
		}
		if(HasRevision() && other.HasRevision())
		{
			if(revision < other.revision)
			{
				return false;
			}
			if(revision > other.revision)
			{
				return true;
			}
			return false;
		}
		return false;
	}
	constexpr bool IsOlderThan(VersionWithRevision other) const noexcept
	{
		if(version < other.version)
		{
			return true;
		}
		if(version > other.version)
		{
			return false;
		}
		if(!HasRevision() && !other.HasRevision())
		{
			return false;
		}
		if(HasRevision() && other.HasRevision())
		{
			if(revision < other.revision)
			{
				return true;
			}
			if(revision > other.revision)
			{
				return false;
			}
			return false;
		}
		return false;
	}
};



namespace Build
{

	// Returns true if all conditions for an official release build are met
	bool IsReleasedBuild();

	// Return true if this is a debug build with no optimizations
	bool IsDebugBuild();

	// Return a string decribing the time of the build process (if built from a svn working copy and tsvn was available during build, otherwise it returns the time version.cpp was last rebuild which could be unreliable as it does not get rebuild every time without tsvn)
	mpt::ustring GetBuildDateString();

	// Return a string decribing some of the build features
	mpt::ustring GetBuildFeaturesString(); // e.g. " NO_VST NO_DSOUND"

	// Return a string describing the compiler version used for building.
	mpt::ustring GetBuildCompilerString(); // e.g. "Microsoft Compiler 15.00.20706.01"

	enum Strings
	{
		StringsNone         = 0,
		StringVersion       = 1<<0, // "1.23.35.45"
		StringRevision      = 1<<2, // "-r1234+"
		StringSourceInfo    = 1<<5, // "https://source.openmpt.org/svn/openmpt/trunk/OpenMPT@1234 (2016-01-02) +dirty"
		StringBuildFlags    = 1<<6, // "TEST DEBUG"
		StringBuildFeatures = 1<<7, // "NO_VST NO_DSOUND"
	};
	MPT_DECLARE_ENUM(Strings)

	// Returns a versions string with the fields selected via @strings.
	mpt::ustring GetVersionString(FlagSet<Build::Strings> strings);

	// Returns a pure version string
	mpt::ustring GetVersionStringPure(); // e.g. "1.17.02.08-r1234+"

	// Returns a simple version string
	mpt::ustring GetVersionStringSimple(); // e.g. "1.17.02.08-r1234+ TEST"

	// Returns Version::CurrentAsString() if the build is a clean release build straight from the repository or an extended string otherwise (if built from a svn working copy and tsvn was available during build)
	mpt::ustring GetVersionStringExtended(); // e.g. "1.17.02.08-r1234+ DEBUG"

	enum class Url
	{
		Website,
		Download,
		Forum,
		Bugtracker,
		Updates,
		TopPicks,
	};
	// Returns a URL for the respective key.
	mpt::ustring GetURL(Build::Url key);

	// Returns a multi-line string containing the full credits for the code base
	mpt::ustring GetFullCreditsString();

	// Returns the OpenMPT license text
	mpt::ustring GetLicenseString();

} //namespace Build



OPENMPT_NAMESPACE_END
