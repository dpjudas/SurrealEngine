/*
 * version.cpp
 * -----------
 * Purpose: OpenMPT version handling.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "version.h"

#include "mpt/format/join.hpp"
#include "mpt/parse/parse.hpp"
#include "mpt/string/utility.hpp"

#include "mptString.h"
#include "mptStringFormat.h"

#include "versionNumber.h"

#if __has_include("svn_version.h")
#include "svn_version.h"
#else
#include "../build/svn_version/svn_version.h"
#endif



OPENMPT_NAMESPACE_BEGIN



#define MPT_MAKE_VERSION_NUMERIC_HELPER(prefix,v0,v1,v2,v3) Version( prefix ## v0 , prefix ## v1 , prefix ## v2 , prefix ## v3 )
#define MPT_MAKE_VERSION_NUMERIC(v0,v1,v2,v3) MPT_MAKE_VERSION_NUMERIC_HELPER(0x, v0, v1, v2, v3)

#define MPT_VERSION_CURRENT MPT_MAKE_VERSION_NUMERIC(VER_MAJORMAJOR,VER_MAJOR,VER_MINOR,VER_MINORMINOR)



static_assert((MPT_VERSION_CURRENT.GetRawVersion() & 0xffffu) != 0x0000u, "Version numbers ending in .00.00 shall never exist again, as they make interpreting the version number ambiguous for file formats which can only store the two major parts of the version number (e.g. IT and S3M).");



Version Version::Current() noexcept
{
	return MPT_VERSION_CURRENT;
}

mpt::ustring Version::GetOpenMPTVersionString() const
{
	return U_("OpenMPT ") + ToUString();
}

Version Version::Parse(const mpt::ustring &s)
{
	uint32 result = 0;
	std::vector<mpt::ustring> numbers = mpt::split(s, U_("."));
	for (std::size_t i = 0; i < numbers.size() && i < 4; ++i)
	{
		result |= (mpt::parse_hex<unsigned int>(numbers[i]) & 0xff) << ((3 - i) * 8);
	}
	return Version(result);
}

mpt::ustring Version::ToUString() const
{
	uint32 v = m_Version;
	if(v == 0)
	{
		// Unknown version
		return U_("Unknown");
	} else if((v & 0xFFFF) == 0)
	{
		// Only parts of the version number are known (e.g. when reading the version from the IT or S3M file header)
		return MPT_UFORMAT("{}.{}")(mpt::ufmt::HEX((v >> 24) & 0xFF), mpt::ufmt::HEX0<2>((v >> 16) & 0xFF));
	} else
	{
		// Full version info available
		return MPT_UFORMAT("{}.{}.{}.{}")(mpt::ufmt::HEX((v >> 24) & 0xFF), mpt::ufmt::HEX0<2>((v >> 16) & 0xFF), mpt::ufmt::HEX0<2>((v >> 8) & 0xFF), mpt::ufmt::HEX0<2>((v) & 0xFF));
	}
}

Version Version::WithoutTestNumber() const noexcept
{
	return Version(m_Version & 0xFFFFFF00u);
}

Version Version::WithoutPatchOrTestNumbers() const noexcept
{
	return Version(m_Version & 0xFFFF0000u);
}

bool Version::IsTestVersion() const noexcept
{
	return (
			// Legacy
			(*this > MPT_V("1.17.02.54") && *this < MPT_V("1.18.02.00") && *this != MPT_V("1.18.00.00"))
		||
			// Test builds have non-zero VER_MINORMINOR
			(*this > MPT_V("1.18.02.00") && ((m_Version & 0xFFFFFF00u) != m_Version))
		);
}



namespace Source {

static mpt::ustring GetUrl()
{
	#ifdef OPENMPT_VERSION_URL
		return mpt::ToUnicode(mpt::Charset::ASCII, OPENMPT_VERSION_URL);
	#else
		return mpt::ustring();
	#endif
}

static int GetRevision()
{
	//#if defined(OPENMPT_VERSION_REVISION)
	//	return OPENMPT_VERSION_REVISION;
	//#elif defined(OPENMPT_VERSION_SVNVERSION)
	//	std::string svnversion = OPENMPT_VERSION_SVNVERSION;
	//	if(svnversion.length() == 0)
	//	{
	//		return 0;
	//	}
	//	if(svnversion.find(":") != std::string::npos)
	//	{
	//		svnversion = svnversion.substr(svnversion.find(":") + 1);
	//	}
	//	if(svnversion.find("-") != std::string::npos)
	//	{
	//		svnversion = svnversion.substr(svnversion.find("-") + 1);
	//	}
	//	if(svnversion.find("M") != std::string::npos)
	//	{
	//		svnversion = svnversion.substr(0, svnversion.find("M"));
	//	}
	//	if(svnversion.find("S") != std::string::npos)
	//	{
	//		svnversion = svnversion.substr(0, svnversion.find("S"));
	//	}
	//	if(svnversion.find("P") != std::string::npos)
	//	{
	//		svnversion = svnversion.substr(0, svnversion.find("P"));
	//	}
	//	return mpt::parse<int>(svnversion);
	//#else
	//	MPT_WARNING_STATEMENT("SVN revision unknown. Please check your build system.");
		
	// XXX: don't need this for building in SurrealEngine
	return 0;

	//#endif
}

static bool IsDirty()
{
	#if defined(OPENMPT_VERSION_DIRTY)
		return OPENMPT_VERSION_DIRTY != 0;
	#elif defined(OPENMPT_VERSION_SVNVERSION)
		std::string svnversion = OPENMPT_VERSION_SVNVERSION;
		if(svnversion.length() == 0)
		{
			return false;
		}
		if(svnversion.find("M") != std::string::npos)
		{
			return true;
		}
		return false;
	#else
		return false;
	#endif
}

static bool HasMixedRevisions()
{
	#if defined(OPENMPT_VERSION_MIXEDREVISIONS)
		return OPENMPT_VERSION_MIXEDREVISIONS != 0;
	#elif defined(OPENMPT_VERSION_SVNVERSION)
		std::string svnversion = OPENMPT_VERSION_SVNVERSION;
		if(svnversion.length() == 0)
		{
			return false;
		}
		if(svnversion.find(":") != std::string::npos)
		{
			return true;
		}
		if(svnversion.find("-") != std::string::npos)
		{
			return true;
		}
		if(svnversion.find("S") != std::string::npos)
		{
			return true;
		}
		if(svnversion.find("P") != std::string::npos)
		{
			return true;
		}
		return false;
	#else
		return false;
	#endif
}

static bool IsPackage()
{
	#if defined(OPENMPT_VERSION_IS_PACKAGE)
		return OPENMPT_VERSION_IS_PACKAGE != 0;
	#else
		return false;
	#endif
}

static mpt::ustring GetSourceDate()
{
	#if defined(OPENMPT_VERSION_DATE)
		return mpt::ToUnicode(mpt::Charset::ASCII, OPENMPT_VERSION_DATE);
	#else
		return mpt::ustring();
	#endif
}

} // namespace Source

SourceInfo::SourceInfo()
	: m_Url(Source::GetUrl())
	, m_Revision(Source::GetRevision())
	, m_IsDirty(Source::IsDirty())
	, m_HasMixedRevisions(Source::HasMixedRevisions())
	, m_IsPackage(Source::IsPackage())
	, m_Date(Source::GetSourceDate())
{
}

mpt::ustring SourceInfo::GetUrlWithRevision() const
{
	if(m_Url.empty() || (m_Revision == 0))
	{
		return mpt::ustring();
	}
	return m_Url + UL_("@") + mpt::ufmt::val(m_Revision);
}

mpt::ustring SourceInfo::GetStateString() const
{
	mpt::ustring retval;
	if(m_IsDirty)
	{
		retval += UL_("+dirty");
	}
	if(m_HasMixedRevisions)
	{
		retval += UL_("+mixed");
	}
	if(retval.empty())
	{
		retval += UL_("clean");
	}
	if(m_IsPackage)
	{
		retval += UL_("-pkg");
	}
	return retval;
}

SourceInfo SourceInfo::Current()
{
	return SourceInfo();
}



VersionWithRevision VersionWithRevision::Current()
{
	return {Version::Current(), static_cast<uint64>(SourceInfo::Current().Revision())};
}

VersionWithRevision VersionWithRevision::Parse(const mpt::ustring &s)
{
	Version version = Version::Parse(mpt::ustring());
	uint64 revision = 0;
	const auto tokens = mpt::split(s, U_("-"));
	if(tokens.size() >= 1)
	{
		version = Version::Parse(tokens[0]);
	}
	if(tokens.size() >= 2)
	{
		revision = mpt::parse<uint64>(tokens[1].substr(1));
	}
	return {version, revision};
}

mpt::ustring VersionWithRevision::ToUString() const
{
	if(!HasRevision())
	{
		return mpt::ufmt::val(version);
	}
	if(!version.IsTestVersion())
	{
		return mpt::ufmt::val(version);
	}
	return MPT_UFORMAT("{}-r{}")(version, revision);
}




namespace Build {

bool IsReleasedBuild()
{
	return !(Version::Current().IsTestVersion() || IsDebugBuild() || Source::IsDirty() || Source::HasMixedRevisions());
}

bool IsDebugBuild()
{
	#if defined(MPT_BUILD_DEBUG) || defined(DEBUG) || defined(_DEBUG)
		return true;
	#else
		return false;
	#endif
}

mpt::ustring GetBuildDateString()
{
	mpt::ustring result;
	#ifdef MODPLUG_TRACKER
		#if defined(OPENMPT_BUILD_DATE)
			result = mpt::ToUnicode(mpt::Charset::ASCII, OPENMPT_BUILD_DATE );
		#else
			result = mpt::ToUnicode(mpt::Charset::ASCII, __DATE__ " " __TIME__ );
		#endif
	#else // !MODPLUG_TRACKER
		result = SourceInfo::Current().Date();
	#endif // MODPLUG_TRACKER
	return result;
}

static mpt::ustring GetBuildFlagsString()
{
	mpt::ustring retval;
	#ifdef MODPLUG_TRACKER
		#if defined(MPT_BUILD_RETRO)
			retval += UL_(" RETRO");
		#endif // MPT_BUILD_RETRO
		if(Version::Current().IsTestVersion())
		{
			retval += UL_(" TEST");
		}
	#endif // MODPLUG_TRACKER
	if(IsDebugBuild())
	{
		retval += UL_(" DEBUG");
	}
	return retval;
}

mpt::ustring GetBuildFeaturesString()
{
	mpt::ustring retval;
	#ifdef LIBOPENMPT_BUILD
		retval = UL_("")
		#if defined(MPT_WITH_ZLIB)
			UL_(" +ZLIB")
		#endif
		#if defined(MPT_WITH_MINIZ)
			UL_(" +MINIZ")
		#endif
		#if !defined(MPT_WITH_ZLIB) && !defined(MPT_WITH_MINIZ)
			UL_(" -INFLATE")
		#endif
		#if defined(MPT_WITH_MPG123)
			UL_(" +MPG123")
		#endif
		#if defined(MPT_WITH_MINIMP3)
			UL_(" +MINIMP3")
		#endif
		#if defined(MPT_WITH_MEDIAFOUNDATION)
			UL_(" +MF")
		#endif
		#if !defined(MPT_WITH_MPG123) && !defined(MPT_WITH_MINIMP3) && !defined(MPT_WITH_MEDIAFOUNDATION)
			UL_(" -MP3")
		#endif
		#if defined(MPT_WITH_OGG) && defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISFILE)
			UL_(" +VORBIS")
		#endif
		#if defined(MPT_WITH_STBVORBIS)
			UL_(" +STBVORBIS")
		#endif
		#if !(defined(MPT_WITH_OGG) && defined(MPT_WITH_VORBIS) && defined(MPT_WITH_VORBISFILE)) && !defined(MPT_WITH_STBVORBIS)
			UL_(" -VORBIS")
		#endif
		#if !defined(NO_PLUGINS)
			UL_(" +PLUGINS")
		#else
			UL_(" -PLUGINS")
		#endif
		#if defined(MPT_WITH_DMO)
			UL_(" +DMO")
		#endif
		;
	#endif
	#ifdef MODPLUG_TRACKER
		retval += UL_("")
		#if defined(UNICODE)
			UL_(" UNICODE")
		#else
			UL_(" ANSI")
		#endif
		#ifndef MPT_WITH_VST
			UL_(" NO_VST")
		#endif
		#ifndef MPT_WITH_DMO
			UL_(" NO_DMO")
		#endif
		#ifdef NO_PLUGINS
			UL_(" NO_PLUGINS")
		#endif
			;
	#endif
	return retval;
}

mpt::ustring GetBuildCompilerString()
{
	mpt::ustring retval;
	#if MPT_COMPILER_GENERIC
		retval += U_("Generic C++11 Compiler");
	#elif MPT_COMPILER_MSVC
		#if defined(_MSC_FULL_VER) && defined(_MSC_BUILD) && (_MSC_BUILD > 0)
			retval += MPT_UFORMAT("Microsoft Compiler {}.{}.{}.{}")
				( _MSC_FULL_VER / 10000000
				, mpt::ufmt::dec0<2>((_MSC_FULL_VER / 100000) % 100)
				, mpt::ufmt::dec0<5>(_MSC_FULL_VER % 100000)
				, mpt::ufmt::dec0<2>(_MSC_BUILD)
				);
		#elif defined(_MSC_FULL_VER)
			retval += MPT_UFORMAT("Microsoft Compiler {}.{}.{}")
				( _MSC_FULL_VER / 10000000
				, mpt::ufmt::dec0<2>((_MSC_FULL_VER / 100000) % 100)
				, mpt::ufmt::dec0<5>(_MSC_FULL_VER % 100000)
				);
		#else
			retval += MPT_UFORMAT("Microsoft Compiler {}.{}")(MPT_COMPILER_MSVC_VERSION / 100, MPT_COMPILER_MSVC_VERSION % 100);
		#endif
	#elif MPT_COMPILER_GCC
		retval += MPT_UFORMAT("GNU Compiler Collection {}.{}.{}")(MPT_COMPILER_GCC_VERSION / 10000, (MPT_COMPILER_GCC_VERSION / 100) % 100, MPT_COMPILER_GCC_VERSION % 100);
	#elif MPT_COMPILER_CLANG
		retval += MPT_UFORMAT("Clang {}.{}.{}")(MPT_COMPILER_CLANG_VERSION / 10000, (MPT_COMPILER_CLANG_VERSION / 100) % 100, MPT_COMPILER_CLANG_VERSION % 100);
	#else
		retval += U_("unknown");
	#endif
	return retval;
}

static mpt::ustring GetRevisionString()
{
	mpt::ustring result;
	if(Source::GetRevision() == 0)
	{
		return result;
	}
	result = U_("-r") + mpt::ufmt::val(Source::GetRevision());
	if(Source::HasMixedRevisions())
	{
		result += UL_("!");
	}
	if(Source::IsDirty())
	{
		result += UL_("+");
	}
	if(Source::IsPackage())
	{
		result += UL_("p");
	}
	return result;
}

mpt::ustring GetVersionString(FlagSet<Build::Strings> strings)
{
	std::vector<mpt::ustring> result;
	if(strings[StringVersion])
	{
		result.push_back(mpt::ufmt::val(Version::Current()));
	}
	if(strings[StringRevision])
	{
		if(!IsReleasedBuild())
		{
			result.push_back(GetRevisionString());
		}
	}
	if(strings[StringSourceInfo])
	{
		const SourceInfo sourceInfo = SourceInfo::Current();
		if(!sourceInfo.GetUrlWithRevision().empty())
		{
			result.push_back(MPT_UFORMAT(" {}")(sourceInfo.GetUrlWithRevision()));
		}
		if(!sourceInfo.Date().empty())
		{
			result.push_back(MPT_UFORMAT(" ({})")(sourceInfo.Date()));
		}
		if(!sourceInfo.GetStateString().empty())
		{
			result.push_back(MPT_UFORMAT(" {}")(sourceInfo.GetStateString()));
		}
	}
	if(strings[StringBuildFlags])
	{
		if(!IsReleasedBuild())
		{
			result.push_back(GetBuildFlagsString());
		}
	}
	if(strings[StringBuildFeatures])
	{
		result.push_back(GetBuildFeaturesString());
	}
	return mpt::trim(mpt::join_format<mpt::ustring>(result, U_("")));
}

mpt::ustring GetVersionStringPure()
{
	FlagSet<Build::Strings> strings;
	strings |= Build::StringVersion;
	strings |= Build::StringRevision;
	return GetVersionString(strings);
}

mpt::ustring GetVersionStringSimple()
{
	FlagSet<Build::Strings> strings;
	strings |= Build::StringVersion;
	strings |= Build::StringRevision;
	strings |= Build::StringBuildFlags;
	return GetVersionString(strings);
}

mpt::ustring GetVersionStringExtended()
{
	FlagSet<Build::Strings> strings;
	strings |= Build::StringVersion;
	strings |= Build::StringRevision;
	#ifndef MODPLUG_TRACKER
		strings |= Build::StringSourceInfo;
	#endif
	strings |= Build::StringBuildFlags;
	#ifdef MODPLUG_TRACKER
		strings |= Build::StringBuildFeatures;
	#endif
	return GetVersionString(strings);
}

mpt::ustring GetURL(Build::Url key)
{
	mpt::ustring result;
	switch(key)
	{
		case Url::Website:
			#ifdef LIBOPENMPT_BUILD
				result = U_("https://lib.openmpt.org/");
			#else
				result = U_("https://openmpt.org/");
			#endif
			break;
		case Url::Download:
			#ifdef MODPLUG_TRACKER
				result = IsReleasedBuild() ? U_("https://openmpt.org/download") : U_("https://builds.openmpt.org/builds/");
			#else
				result = U_("https://lib.openmpt.org/libopenmpt/download/");
			#endif
			break;
		case Url::Forum:
			result = U_("https://forum.openmpt.org/");
			break;
		case Url::Bugtracker:
			result = U_("https://bugs.openmpt.org/");
			break;
		case Url::Updates:
			result = U_("https://openmpt.org/download");
			break;
		case Url::TopPicks:
			result = U_("https://openmpt.org/top_picks");
			break;
	}
	return result;
}

mpt::ustring GetFullCreditsString()
{
	return mpt::ToUnicode(mpt::Charset::UTF8,
#ifdef MODPLUG_TRACKER
		"OpenMPT / Open ModPlug Tracker\n"
#else
		"libopenmpt (based on OpenMPT / Open ModPlug Tracker)\n"
#endif
		"\n"
		"Copyright \xC2\xA9 2004-2025 OpenMPT Project Developers and Contributors\n"
		"Copyright \xC2\xA9 1997-2003 Olivier Lapicque\n"
		"\n"
		"Developers:\n"
		"Johannes Schultz (2008-2025)\n"
		"J\xC3\xB6rn Heusipp (2012-2025)\n"
		"Ahti Lepp\xC3\xA4nen (2005-2011)\n"
		"Robin Fernandes (2004-2007)\n"
		"Sergiy Pylypenko (2007)\n"
		"Eric Chavanon (2004-2005)\n"
		"Trevor Nunes (2004)\n"
		"Olivier Lapicque (1997-2003)\n"
		"\n"
		"Additional contributors:\n"
		"coda (https://coda.s3m.us/)\n"
		"cs127 (https://cs127.github.io/)\n"
		"Jo\xC3\xA3o Baptista de Paula e Silva (https://joaobapt.com/)\n"
		"kode54 (https://kode54.net/)\n"
		"Revenant (https://revenant1.net/)\n"
		"SYRiNX\n"
		"xaimus (http://xaimus.com/)\n"
		"zersal\n"
		"\n"
		"Thanks to:\n"
		"\n"
		"Konstanty for the XMMS-ModPlug resampling implementation\n"
		"http://modplug-xmms.sourceforge.net/\n"
		"\n"
#ifdef MODPLUG_TRACKER
		"Geraint Luff for Signalsmith Stretch\n"
		"https://signalsmith-audio.co.uk/code/stretch/\n"
		"\n"
		"Aleksey Vaneev of Voxengo for r8brain sample rate converter\n"
		"https://github.com/avaneev/r8brain-free-src\n"
		"\n"
#endif
#ifdef MPT_WITH_VST
		"Hermann Seib for his example VST Host implementation\n"
		"http://www.hermannseib.com/english/vsthost.htm\n"
		"\n"
		"Benjamin \"BeRo\" Rosseaux for his independent VST header\n"
		"https://blog.rosseaux.net/\n"
		"\n"
#endif
		"Storlek for all the IT compatibility hints and testcases\n"
		"as well as the IMF, MDL, OKT and ULT loaders\n"
		"http://schismtracker.org/\n"
		"\n"
		"Sergei \"x0r\" Kolzun for various hints on Scream Tracker 2 compatibility\n"
		"https://github.com/viiri/st2play\n"
		"\n"
		"Laurent Cl\xc3\xA9vy for unofficial MO3 documentation and decompression code\n"
		"https://github.com/lclevy/unmo3\n"
		"\n"
		"Ben \"GreaseMonkey\" Russell for IT sample compression code\n"
		"https://github.com/iamgreaser/it2everything/\n"
		"\n"
		"Antti S. Lankila for Amiga resampler implementation\n"
		"https://web.archive.org/web/20221228071135/https://bel.fi/alankila/modguide/\n"
		"\n"
		"Shayde / Reality Productions for Opal OPL3 emulator\n"
		"https://www.3eality.com/\n"
		"\n"
		"Ryuhei Mori for TinyFFT\n"
		"https://github.com/ryuhei-mori/tinyfft\n"
		"\n"
#ifdef MPT_WITH_ZLIB
		"Jean-loup Gailly and Mark Adler for zlib\n"
		"https://zlib.net/\n"
		"\n"
#endif
#ifdef MPT_WITH_MINIZ
		"Rich Geldreich et al. for miniz\n"
		"https://github.com/richgel999/miniz\n"
		"\n"
#endif
#ifdef MPT_WITH_LHASA
		"Simon Howard for lhasa\n"
		"https://fragglet.github.io/lhasa/\n"
		"\n"
#endif
#ifdef MPT_WITH_UNRAR
		"Alexander L. Roshal for UnRAR\n"
		"https://rarlab.com/\n"
		"\n"
#endif
#ifdef MPT_WITH_ANCIENT
		"Teemu Suutari for ancient\n"
		"https://github.com/temisu/ancient\n"
		"\n"
#endif
#ifdef MPT_WITH_PORTAUDIO
		"PortAudio contributors\n"
		"http://www.portaudio.com/\n"
		"\n"
#endif
#ifdef MPT_WITH_RTAUDIO
		"Gary P. Scavone, McGill University for RtAudio\n"
		"https://www.music.mcgill.ca/~gary/rtaudio/\n"
		"\n"
#endif
#ifdef MPT_WITH_FLAC
		"Josh Coalson / Xiph.Org Foundation for libFLAC\n"
		"https://xiph.org/flac/\n"
		"\n"
#endif
#if defined(MPT_WITH_MPG123)
		"The mpg123 project for libmpg123\n"
		"https://mpg123.de/\n"
		"\n"
#endif
#ifdef MPT_WITH_MINIMP3
		"Lion (github.com/lieff) for minimp3\n"
		"https://github.com/lieff/minimp3/\n"
		"\n"
#endif
#ifdef MPT_WITH_STBVORBIS
		"Sean Barrett for stb_vorbis\n"
		"https://github.com/nothings/stb/\n"
		"\n"
#endif
#ifdef MPT_WITH_OGG
		"Xiph.Org Foundation for libogg\n"
		"https://xiph.org/ogg/\n"
		"\n"
#endif
#if defined(MPT_WITH_VORBIS) || defined(MPT_WITH_LIBVORBISFILE)
		"Xiph.Org Foundation for libvorbis\n"
		"https://xiph.org/vorbis/\n"
		"\n"
#endif
#if defined(MPT_WITH_OPUS)
		"Xiph.Org, Skype Limited, Octasic, Jean-Marc Valin, Timothy B. Terriberry,\n"
		"CSIRO, Gregory Maxwell, Mark Borgerding, Erik de Castro Lopo,\n"
		"Xiph.Org Foundation, Microsoft Corporation, Broadcom Corporation for libopus\n"
		"https://opus-codec.org/\n"
		"\n"
#endif
#if defined(MPT_WITH_OPUSFILE)
		"Xiph.Org Foundation and contributors for libopusfile\n"
		"https://opus-codec.org/\n"
		"\n"
#endif
#if defined(MPT_WITH_OPUSENC)
		"Xiph.Org Foundation, Jean-Marc Valin and contributors for libopusenc\n"
		"https://git.xiph.org/?p=libopusenc.git;a=summary\n"
		"\n"
#endif
#if defined(MPT_WITH_LAME)
		"The LAME project for LAME\n"
		"https://lame.sourceforge.io/\n"
		"\n"
#endif
#if defined(MPT_WITH_NLOHMANNJSON)
		"Niels Lohmann et al. for nlohmann-json\n"
		"https://github.com/nlohmann/json\n"
		"\n"
#endif
#ifdef MODPLUG_TRACKER
		"Lennart Poettering and David Henningsson for RealtimeKit\n"
		"http://git.0pointer.net/rtkit.git/\n"
		"\n"
		"Gary P. Scavone for RtMidi\n"
		"https://www.music.mcgill.ca/~gary/rtmidi/\n"
		"\n"
		"Alexander Uckun for decimal input field\n"
		"https://www.codeproject.com/Articles/21257/_\n"
		"\n"
		"\xc3\x9alfur Kolka for application icon, splash and about screen\n"
		"https://www.behance.net/ulfurkolka\n"
		"\n"
		"Nobuyuki for file icon\n"
		"https://github.com/nobuyukinyuu/\n"
		"\n"
#endif
		"Daniel Collin (emoon/TBL) for providing test infrastructure\n"
		"https://mastodon.gamedev.place/@daniel_collin\n"
		"\n"
		"The people in the ModPlug community for crucial contribution\n"
		"in the form of ideas, testing and support;\n"
		"thanks particularly to:\n"
		"33, 8bitbubsy, AliceLR, Anboi, BooT-SectoR-ViruZ, Bvanoudtshoorn\n"
		"a11cf0, christofori, cubaxd, Diamond, Ganja, Georg, Goor00,\n"
		"Harbinger, jmkz, KrazyKatz, LPChip, MiDoRi, Nofold, Rakib, Sam Zen\n"
		"Skaven, Skilletaudio, Snu, Squirrel Havoc, Teimoso, Waxhead\n"
		"\n"
#ifdef MPT_WITH_VST
		"VST PlugIn Technology by Steinberg Media Technologies GmbH\n"
		"\n"
#endif
#ifdef MPT_WITH_ASIO
		"ASIO Technology by Steinberg Media Technologies GmbH\n"
		"\n"
#endif
		);
}

mpt::ustring GetLicenseString()
{
	return MPT_UTF8(
		"Copyright (c) 2004-2025, OpenMPT Project Developers and Contributors" "\n"
		"Copyright (c) 1997-2003, Olivier Lapicque" "\n"
		"All rights reserved." "\n"
		"" "\n"
		"Redistribution and use in source and binary forms, with or without" "\n"
		"modification, are permitted provided that the following conditions are met:" "\n"
		"    * Redistributions of source code must retain the above copyright" "\n"
		"      notice, this list of conditions and the following disclaimer." "\n"
		"    * Redistributions in binary form must reproduce the above copyright" "\n"
		"      notice, this list of conditions and the following disclaimer in the" "\n"
		"      documentation and/or other materials provided with the distribution." "\n"
		"    * Neither the name of the OpenMPT project nor the" "\n"
		"      names of its contributors may be used to endorse or promote products" "\n"
		"      derived from this software without specific prior written permission." "\n"
		"" "\n"
		"THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS \"AS IS\"" "\n"
		"AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE" "\n"
		"IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE" "\n"
		"DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE" "\n"
		"FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL" "\n"
		"DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR" "\n"
		"SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER" "\n"
		"CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY," "\n"
		"OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE" "\n"
		"OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE." "\n"
		);
}

} // namespace Build



OPENMPT_NAMESPACE_END
