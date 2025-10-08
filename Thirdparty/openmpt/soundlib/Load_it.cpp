/*
 * Load_it.cpp
 * -----------
 * Purpose: IT (Impulse Tracker) module loader / saver
 * Notes  : Also handles MPTM loading / saving, as the formats are almost identical.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "tuningcollection.h"
#include "mod_specifications.h"
#ifdef MODPLUG_TRACKER
#include "../mptrack/Moddoc.h"
#include "../mptrack/TrackerSettings.h"
#endif // MODPLUG_TRACKER
#ifdef MPT_EXTERNAL_SAMPLES
#include "../common/mptPathString.h"
#endif // MPT_EXTERNAL_SAMPLES
#include "../common/serialization_utils.h"
#ifndef MODPLUG_NO_FILESAVE
#include "../common/mptFileIO.h"
#endif // MODPLUG_NO_FILESAVE
#include "plugins/PlugInterface.h"
#include <sstream>
#include "../common/version.h"
#include "ITTools.h"
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"


OPENMPT_NAMESPACE_BEGIN


const uint16 verMptFileVer = 0x891;
const uint16 verMptFileVerLoadLimit = 0x1000; // If cwtv-field is greater or equal to this value,
											  // the MPTM file will not be loaded.

/*
MPTM version history for cwtv-field in "IT" header (only for MPTM files!):
0x890(1.18.02.00) -> 0x891(1.19.00.00): Pattern-specific time signatures
										Fixed behaviour of Pattern Loop command for rows > 255 (r617)
0x88F(1.18.01.00) -> 0x890(1.18.02.00): Removed volume command velocity :xy, added delay-cut command :xy.
0x88E(1.17.02.50) -> 0x88F(1.18.01.00): Numerous changes
0x88D(1.17.02.49) -> 0x88E(1.17.02.50): Changed ID to that of IT and undone the orderlist change done in
				       0x88A->0x88B. Now extended orderlist is saved as extension.
0x88C(1.17.02.48) -> 0x88D(1.17.02.49): Some tuning related changes - that part fails to read on older versions.
0x88B -> 0x88C: Changed type in which tuning number is printed to file: size_t -> uint16.
0x88A -> 0x88B: Changed order-to-pattern-index table type from uint8-array to vector<uint32>.
*/


#ifndef MODPLUG_NO_FILESAVE

static bool AreNonDefaultTuningsUsed(const CSoundFile& sf)
{
	const INSTRUMENTINDEX numIns = sf.GetNumInstruments();
	for(INSTRUMENTINDEX i = 1; i <= numIns; i++)
	{
		if(sf.Instruments[i] != nullptr && sf.Instruments[i]->pTuning != nullptr)
			return true;
	}
	return false;
}

static void WriteTuningCollection(std::ostream& oStrm, const CTuningCollection& tc)
{
	tc.Serialize(oStrm, U_("Tune specific tunings"));
}

static void WriteTuningMap(std::ostream& oStrm, const CSoundFile& sf)
{
	if(sf.GetNumInstruments() > 0)
	{
		//Writing instrument tuning data: first creating
		//tuning name <-> tuning id number map,
		//and then writing the tuning id for every instrument.
		//For example if there are 6 instruments and
		//first half use tuning 'T1', and the other half
		//tuning 'T2', the output would be something like
		//T1 1 T2 2 1 1 1 2 2 2

		//Creating the tuning address <-> tuning id number map.
		std::map<CTuning*, uint16> tNameToShort_Map;

		unsigned short figMap = 0;
		for(INSTRUMENTINDEX i = 1; i <= sf.GetNumInstruments(); i++)
		{
			CTuning *pTuning = nullptr;
			if(sf.Instruments[i] != nullptr)
			{
				pTuning = sf.Instruments[i]->pTuning;
			}
			auto iter = tNameToShort_Map.find(pTuning);
			if(iter != tNameToShort_Map.end())
				continue; //Tuning already mapped.

			tNameToShort_Map[pTuning] = figMap;
			figMap++;
		}

		//...and write the map with tuning names replacing
		//the addresses.
		const uint16 tuningMapSize = static_cast<uint16>(tNameToShort_Map.size());
		mpt::IO::WriteIntLE<uint16>(oStrm, tuningMapSize);
		for(auto &iter : tNameToShort_Map)
		{
			if(iter.first)
				mpt::IO::WriteSizedStringLE<uint8>(oStrm, mpt::ToCharset(mpt::Charset::UTF8, iter.first->GetName()));
			else //Case: Using original IT tuning.
				mpt::IO::WriteSizedStringLE<uint8>(oStrm, "->MPT_ORIGINAL_IT<-");

			mpt::IO::WriteIntLE<uint16>(oStrm, iter.second);
		}

		//Writing tuning data for instruments.
		for(INSTRUMENTINDEX i = 1; i <= sf.GetNumInstruments(); i++)
		{
			CTuning *pTuning = nullptr;
			if(sf.Instruments[i] != nullptr)
			{
				pTuning = sf.Instruments[i]->pTuning;
			}
			auto iter = tNameToShort_Map.find(pTuning);
			if(iter == tNameToShort_Map.end()) //Should never happen
			{
				sf.AddToLog(LogError, U_("Error: 210807_1"));
				return;
			}
			mpt::IO::WriteIntLE<uint16>(oStrm, iter->second);
		}
	}
}

#endif // MODPLUG_NO_FILESAVE


static void ReadTuningCollection(std::istream &iStrm, CTuningCollection &tc, const std::size_t dummy, mpt::Charset defaultCharset)
{
	MPT_UNREFERENCED_PARAMETER(dummy);
	mpt::ustring name;
	tc.Deserialize(iStrm, name, defaultCharset);
}


template<class TUNNUMTYPE, class STRSIZETYPE>
static bool ReadTuningMapTemplate(std::istream& iStrm, std::map<uint16, mpt::ustring> &shortToTNameMap, mpt::Charset charset, const size_t maxNum = 500)
{
	TUNNUMTYPE numTuning = 0;
	mpt::IO::ReadIntLE<TUNNUMTYPE>(iStrm, numTuning);
	if(numTuning > maxNum)
		return true;

	for(size_t i = 0; i < numTuning; i++)
	{
		std::string temp;
		uint16 ui = 0;
		if(!mpt::IO::ReadSizedStringLE<STRSIZETYPE>(iStrm, temp, 255))
			return true;

		mpt::IO::ReadIntLE<uint16>(iStrm, ui);
		shortToTNameMap[ui] = mpt::ToUnicode(charset, temp);
	}
	if(iStrm.good())
		return false;
	else
		return true;
}


static void ReadTuningMapImpl(std::istream& iStrm, CSoundFile& csf, mpt::Charset charset, const size_t = 0, bool old = false)
{
	std::map<uint16, mpt::ustring> shortToTNameMap;
	if(old)
	{
		ReadTuningMapTemplate<uint32, uint32>(iStrm, shortToTNameMap, charset);
	} else
	{
		ReadTuningMapTemplate<uint16, uint8>(iStrm, shortToTNameMap, charset);
	}

	// Read & set tunings for instruments
	std::vector<mpt::ustring> notFoundTunings;
	for(INSTRUMENTINDEX i = 1; i<=csf.GetNumInstruments(); i++)
	{
		uint16 ui = 0;
		mpt::IO::ReadIntLE<uint16>(iStrm, ui);
		auto iter = shortToTNameMap.find(ui);
		if(csf.Instruments[i] && iter != shortToTNameMap.end())
		{
			const mpt::ustring str = iter->second;

			if(str == U_("->MPT_ORIGINAL_IT<-"))
			{
				csf.Instruments[i]->pTuning = nullptr;
				continue;
			}

			csf.Instruments[i]->pTuning = csf.GetTuneSpecificTunings().GetTuning(str);
			if(csf.Instruments[i]->pTuning)
				continue;

#ifdef MODPLUG_TRACKER
			CTuning *localTuning = TrackerSettings::Instance().oldLocalTunings->GetTuning(str);
			if(localTuning)
			{
				std::unique_ptr<CTuning> pNewTuning = std::make_unique<CTuning>(*localTuning);
				CTuning *pT = csf.GetTuneSpecificTunings().AddTuning(std::move(pNewTuning));
				if(pT)
				{
					csf.AddToLog(LogInformation, U_("Local tunings are deprecated and no longer supported. Tuning '") + str + U_("' found in Local tunings has been copied to Tune-specific tunings and will be saved in the module file."));
					csf.Instruments[i]->pTuning = pT;
					if(csf.GetpModDoc() != nullptr)
					{
						csf.GetpModDoc()->SetModified();
					}
					continue;
				} else
				{
					csf.AddToLog(LogError, U_("Copying Local tuning '") + str + U_("' to Tune-specific tunings failed."));
				}
			}
#endif

			if(str == U_("12TET [[fs15 1.17.02.49]]") || str == U_("12TET"))
			{
				std::unique_ptr<CTuning> pNewTuning = csf.CreateTuning12TET(str);
				CTuning *pT = csf.GetTuneSpecificTunings().AddTuning(std::move(pNewTuning));
				if(pT)
				{
					#ifdef MODPLUG_TRACKER
						csf.AddToLog(LogInformation, U_("Built-in tunings will no longer be used. Tuning '") + str + U_("' has been copied to Tune-specific tunings and will be saved in the module file."));
						csf.Instruments[i]->pTuning = pT;
						if(csf.GetpModDoc() != nullptr)
						{
							csf.GetpModDoc()->SetModified();
						}
					#endif
					continue;
				} else
				{
					#ifdef MODPLUG_TRACKER
						csf.AddToLog(LogError, U_("Copying Built-in tuning '") + str + U_("' to Tune-specific tunings failed."));
					#endif
				}
			}

			// Checking if not found tuning already noticed.
			if(!mpt::contains(notFoundTunings, str))
			{
				notFoundTunings.push_back(str);
				csf.AddToLog(LogWarning, U_("Tuning '") + str + U_("' used by the module was not found."));
#ifdef MODPLUG_TRACKER
				if(csf.GetpModDoc() != nullptr)
				{
					csf.GetpModDoc()->SetModified(); // The tuning is changed so the modified flag is set.
				}
#endif // MODPLUG_TRACKER

			}
			csf.Instruments[i]->pTuning = nullptr;

		} else
		{
			//This 'else' happens probably only in case of corrupted file.
			if(csf.Instruments[i])
				csf.Instruments[i]->pTuning = nullptr;
		}

	}
	//End read&set instrument tunings
}


static void ReadTuningMap(std::istream& iStrm, CSoundFile& csf, const size_t dummy, mpt::Charset charset)
{
	ReadTuningMapImpl(iStrm, csf, charset, dummy, false);
}


//////////////////////////////////////////////////////////
// Impulse Tracker IT file support


size_t CSoundFile::ITInstrToMPT(FileReader &file, ModInstrument &ins, uint16 trkvers)
{
	if(trkvers < 0x0200)
	{
		// Load old format (IT 1.xx) instrument (early IT 2.xx modules may have cmwt set to 1.00 for backwards compatibility)
		ITOldInstrument instrumentHeader;
		if(!file.ReadStruct(instrumentHeader))
		{
			return 0;
		} else
		{
			instrumentHeader.ConvertToMPT(ins);
			return sizeof(ITOldInstrument);
		}
	} else
	{
		const FileReader::pos_type offset = file.GetPosition();

		// Try loading extended instrument... instSize will differ between normal and extended instruments.
		ITInstrumentEx instrumentHeader;
		file.ReadStructPartial(instrumentHeader);
		size_t instSize = instrumentHeader.ConvertToMPT(ins, GetType());
		file.Seek(offset + instSize);

		// Try reading modular instrument data.
		// Yes, it is completely idiotic that we have both this and LoadExtendedInstrumentProperties.
		// This is only required for files saved with *really* old OpenMPT versions (pre-1.17-RC1).
		// This chunk was also written in later versions (probably to maintain compatibility with
		// those ancient versions), but this also means that redundant information is stored in the file.
		// Starting from OpenMPT 1.25.02.07, this chunk is no longer written.
		if(file.ReadMagic("MSNI"))
		{
			//...the next piece of data must be the total size of the modular data
			FileReader modularData = file.ReadChunk(file.ReadUint32LE());
			instSize += 8 + modularData.GetLength();
			if(modularData.ReadMagic("GULP"))
			{
				ins.nMixPlug = modularData.ReadUint8();
				if(ins.nMixPlug > MAX_MIXPLUGINS) ins.nMixPlug = 0;
			}
		}

		return instSize;
	}
}


static void CopyPatternName(CPattern &pattern, FileReader &file)
{
	char name[MAX_PATTERNNAME] = "";
	file.ReadString<mpt::String::maybeNullTerminated>(name, MAX_PATTERNNAME);
	pattern.SetName(name);
}


// Get version of Impulse Tracker that was used to create an IT/S3M file.
mpt::ustring CSoundFile::GetImpulseTrackerVersion(uint16 cwtv, uint16 cmwt)
{
	mpt::ustring version;
	cwtv &= 0xFFF;
	if(cmwt > 0x0214)
	{
		version = UL_("Impulse Tracker 2.15");
	} else if(cwtv >= 0x0215 && cwtv <= 0x0217)
	{
		const mpt::uchar *versions[] = {UL_("1-2"), UL_("3"), UL_("4-5")};
		version = MPT_UFORMAT("Impulse Tracker 2.14p{}")(mpt::ustring_view(versions[cwtv - 0x0215]));
	} else
	{
		version = MPT_UFORMAT("Impulse Tracker {}.{}")((cwtv & 0x0F00) >> 8, mpt::ufmt::hex0<2>((cwtv & 0xFF)));
	}
	return version;
}


// Get version of Schism Tracker that was used to create an IT/S3M file.
mpt::ustring CSoundFile::GetSchismTrackerVersion(uint16 cwtv, uint32 reserved)
{
	// Schism Tracker version information in a nutshell:
	// < 0x020: a proper version (files saved by such versions are likely very rare)
	// = 0x020: any version between the 0.2a release (2005-04-29?) and 2007-04-17
	// = 0x050: anywhere from 2007-04-17 to 2009-10-31
	// > 0x050: the number of days since 2009-10-31
	// = 0xFFF: any version starting from 2020-10-28 (exact version stored in reserved value)

	cwtv &= 0xFFF;
	if(cwtv > 0x050)
	{
		int32 date = SchismTrackerEpoch + (cwtv < 0xFFF ? cwtv - 0x050 : reserved);
		int32 y = static_cast<int32>((Util::mul32to64(10000, date) + 14780) / 3652425);
		int32 ddd = date - (365 * y + y / 4 - y / 100 + y / 400);
		if(ddd < 0)
		{
			y--;
			ddd = date - (365 * y + y / 4 - y / 100 + y / 400);
		}
		int32 mi = (100 * ddd + 52) / 3060;
		return MPT_UFORMAT("Schism Tracker {}-{}-{}")(
			mpt::ufmt::dec0<4>(y + (mi + 2) / 12),
			mpt::ufmt::dec0<2>((mi + 2) % 12 + 1),
			mpt::ufmt::dec0<2>(ddd - (mi * 306 + 5) / 10 + 1));
	} else
	{
		return MPT_UFORMAT("Schism Tracker 0.{}")(mpt::ufmt::hex0<2>(cwtv));
	}
}


static bool ValidateHeader(const ITFileHeader &fileHeader)
{
	if((std::memcmp(fileHeader.id, "IMPM", 4) && std::memcmp(fileHeader.id, "tpm.", 4))
		|| fileHeader.insnum > 0xFF
		|| fileHeader.smpnum >= MAX_SAMPLES
		)
	{
		return false;
	}
	return true;
}


static uint64 GetHeaderMinimumAdditionalSize(const ITFileHeader &fileHeader)
{
	return fileHeader.ordnum + (fileHeader.insnum + fileHeader.smpnum + fileHeader.patnum) * 4;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderIT(MemoryFileReader file, const uint64 *pfilesize)
{
	ITFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(fileHeader))
	{
		return ProbeFailure;
	}
	return ProbeAdditionalSize(file, pfilesize, GetHeaderMinimumAdditionalSize(fileHeader));
}


bool CSoundFile::ReadIT(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	ITFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(!ValidateHeader(fileHeader))
	{
		return false;
	}
	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(GetHeaderMinimumAdditionalSize(fileHeader))))
	{
		return false;
	}
	if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	InitializeGlobals(MOD_TYPE_IT, 0);

	bool interpretModPlugMade = false;
	mpt::ustring madeWithTracker;

	// OpenMPT crap at the end of file
	size_t mptStartPos = 0;

	if(!memcmp(fileHeader.id, "tpm.", 4))
	{
		// Legacy MPTM files (old 1.17.02.4x releases)
		SetType(MOD_TYPE_MPT);
		file.Seek(file.GetLength() - 4);
		mptStartPos = file.ReadUint32LE();
	} else
	{
		if(fileHeader.cwtv > 0x888 && fileHeader.cwtv <= 0xFFF)
		{
			file.Seek(file.GetLength() - 4);
			mptStartPos = file.ReadUint32LE();
			if(mptStartPos >= 0x100 && mptStartPos < file.GetLength())
			{
				if(file.Seek(mptStartPos) && file.ReadMagic("228"))
				{
					SetType(MOD_TYPE_MPT);
					if(fileHeader.cwtv >= verMptFileVerLoadLimit)
					{
						AddToLog(LogError, U_("The file informed that it is incompatible with this version of OpenMPT. Loading was terminated."));
						return false;
					} else if(fileHeader.cwtv > verMptFileVer)
					{
						AddToLog(LogInformation, U_("The loaded file was made with a more recent OpenMPT version and this version may not be able to load all the features or play the file correctly."));
					}
				}
			}
		}

		if(GetType() == MOD_TYPE_IT)
		{
			// Which tracker was used to make this?
			if((fileHeader.cwtv & 0xF000) == 0x5000)
			{
				// OpenMPT Version number (Major.Minor)
				// This will only be interpreted as "made with ModPlug" (i.e. disable compatible playback etc) if the "reserved" field is set to "OMPT" - else, compatibility was used.
				uint32 mptVersion = (fileHeader.cwtv & 0x0FFF) << 16;
				if(!memcmp(&fileHeader.reserved, "OMPT", 4))
					interpretModPlugMade = true;
				else if(mptVersion >= 0x01'29'00'00)
					mptVersion |= fileHeader.reserved & 0xFFFF;
				m_dwLastSavedWithVersion = Version(mptVersion);
			} else if(fileHeader.cmwt == 0x888 || fileHeader.cwtv == 0x888)
			{
				// OpenMPT 1.17.02.26 (r122) to 1.18
				// Exact version number will be determined later.
				interpretModPlugMade = true;
				m_dwLastSavedWithVersion = MPT_V("1.17.00.00");
			} else if(fileHeader.cwtv == 0x0214 && fileHeader.cmwt == 0x0202 && fileHeader.reserved == 0)
			{
				// ModPlug Tracker b3.2 - 1.09, instruments 557 bytes apart
				m_dwLastSavedWithVersion = MPT_V("1.09.00.00");
				madeWithTracker = UL_("ModPlug Tracker b3.2 - 1.09");
				interpretModPlugMade = true;
			} else if(fileHeader.cwtv == 0x0300 && fileHeader.cmwt == 0x0300 && fileHeader.reserved == 0 && fileHeader.ordnum == 256 && fileHeader.sep == 128 && fileHeader.pwd == 0)
			{
				// A rare variant used from OpenMPT 1.17.02.20 (r113) to 1.17.02.25 (r121), found e.g. in xTr1m-SD.it
				m_dwLastSavedWithVersion = MPT_V("1.17.02.20");
				interpretModPlugMade = true;
			}
		}
	}

	m_SongFlags.set(SONG_LINEARSLIDES, (fileHeader.flags & ITFileHeader::linearSlides) != 0);
	m_SongFlags.set(SONG_ITOLDEFFECTS, (fileHeader.flags & ITFileHeader::itOldEffects) != 0);
	m_SongFlags.set(SONG_ITCOMPATGXX, (fileHeader.flags & ITFileHeader::itCompatGxx) != 0);
	m_SongFlags.set(SONG_EXFILTERRANGE, (fileHeader.flags & ITFileHeader::extendedFilterRange) != 0);

	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.songname);

	// Read row highlights
	if((fileHeader.special & ITFileHeader::embedPatternHighlights))
	{
		// MPT 1.09 and older (and maybe also newer) versions leave this blank (0/0), but have the "special" flag set.
		// Newer versions of MPT and OpenMPT 1.17 *always* write 4/16 here.
		// Thus, we will just ignore those old versions.
		// Note: OpenMPT 1.17.03.02 was the first version to properly make use of the time signature in the IT header.
		// This poses a small unsolvable problem:
		// - In compatible mode, we cannot distinguish this version from earlier 1.17 releases.
		//   Thus we cannot know when to read this field or not (m_dwLastSavedWithVersion will always be 1.17.00.00).
		//   Luckily OpenMPT 1.17.03.02 should not be very wide-spread.
		// - In normal mode the time signature is always present in the song extensions anyway. So it's okay if we read
		//   the signature here and maybe overwrite it later when parsing the song extensions.
		if(!m_dwLastSavedWithVersion || m_dwLastSavedWithVersion >= MPT_V("1.17.03.02"))
		{
			m_nDefaultRowsPerBeat = fileHeader.highlight_minor;
			m_nDefaultRowsPerMeasure = fileHeader.highlight_major;
		}
	}

	// Global Volume
	m_nDefaultGlobalVolume = fileHeader.globalvol << 1;
	if(m_nDefaultGlobalVolume > MAX_GLOBAL_VOLUME)
		m_nDefaultGlobalVolume = MAX_GLOBAL_VOLUME;
	if(fileHeader.speed)
		Order().SetDefaultSpeed(fileHeader.speed);
	Order().SetDefaultTempoInt(std::max(uint8(31), static_cast<uint8>(fileHeader.tempo)));
	m_nSamplePreAmp = std::min(static_cast<uint8>(fileHeader.mv), uint8(128));

	// Reading orders
	file.Seek(sizeof(ITFileHeader));
	if(GetType() == MOD_TYPE_MPT && fileHeader.cwtv > 0x88A && fileHeader.cwtv <= 0x88D)
	{
		// Deprecated format used for MPTm files created with OpenMPT 1.17.02.46 - 1.17.02.48.
		uint16 version = file.ReadUint16LE();
		if(version != 0)
			return false;
		uint32 numOrd = file.ReadUint32LE();
		if(numOrd > ModSpecs::mptm.ordersMax || !ReadOrderFromFile<uint32le>(Order(), file, numOrd))
			return false;
	} else
	{
		ReadOrderFromFile<uint8>(Order(), file, fileHeader.ordnum, 0xFF, 0xFE);
	}

	// Reading instrument, sample and pattern offsets
	std::vector<uint32le> insPos, smpPos, patPos;
	if(!file.ReadVector(insPos, fileHeader.insnum)
		|| !file.ReadVector(smpPos, fileHeader.smpnum)
		|| !file.ReadVector(patPos, fileHeader.patnum))
	{
		return false;
	}

	// Find the first parapointer.
	// This is used for finding out whether the edit history is actually stored in the file or not,
	// as some early versions of Schism Tracker set the history flag, but didn't save anything.
	// We will consider the history invalid if it ends after the first parapointer.
	uint32 minPtr = std::numeric_limits<decltype(minPtr)>::max();
	for(uint32 pos : insPos)
	{
		if(pos > 0 && pos < minPtr)
			minPtr = pos;
	}
	for(uint32 pos : smpPos)
	{
		if(pos > 0 && pos < minPtr)
			minPtr = pos;
	}
	for(uint32 pos : patPos)
	{
		if(pos > 0 && pos < minPtr)
			minPtr = pos;
	}
	if(fileHeader.special & ITFileHeader::embedSongMessage)
	{
		minPtr = std::min(minPtr, fileHeader.msgoffset.get());
	}

	const bool possiblyUNMO3 = fileHeader.cmwt == 0x0214 && (fileHeader.cwtv == 0x0214 || fileHeader.cwtv == 0)
		&& fileHeader.highlight_major == 0 && fileHeader.highlight_minor == 0
		&& fileHeader.pwd == 0 && fileHeader.reserved == 0
		&& (fileHeader.flags & (ITFileHeader::useMIDIPitchController | ITFileHeader::reqEmbeddedMIDIConfig)) == 0;

	if(possiblyUNMO3 && fileHeader.insnum == 0 && fileHeader.smpnum > 0 && file.GetPosition() + 4 * smpPos.size() + 2 <= minPtr)
	{
		// UNMO3 < v2.4.0.1 reserves some space for instrument parapointers even in sample mode.
		// This makes reading MIDI macros and plugin information impossible.
		// Note: While UNMO3 and CheeseTracker header fingerprints are almost identical, we cannot mis-detect CheeseTracker here,
		// as it always sets the instrument mode flag and writes non-zero row highlights.
		bool oldUNMO3 = true;
		for(uint16 i = 0; i < fileHeader.smpnum; i++)
		{
			if(file.ReadUint32LE() != 0)
			{
				oldUNMO3 = false;
				file.SkipBack(4 + i * 4);
				break;
			}
		}
		if(oldUNMO3)
		{
			madeWithTracker = UL_("UNMO3 <= 2.4");
		}
	}

	if(possiblyUNMO3 && fileHeader.cwtv == 0)
	{
		madeWithTracker = UL_("UNMO3 v0/1");
	}

	// Reading IT Edit History Info
	// This is only supposed to be present if bit 1 of the special flags is set.
	// However, old versions of Schism and probably other trackers always set this bit
	// even if they don't write the edit history count. So we have to filter this out...
	// This is done by looking at the parapointers. If the history data ends after
	// the first parapointer, we assume that it's actually no history data.
	if(fileHeader.special & ITFileHeader::embedEditHistory)
	{
		const uint16 nflt = file.ReadUint16LE();

		if(file.CanRead(nflt * sizeof(ITHistoryStruct)) && file.GetPosition() + nflt * sizeof(ITHistoryStruct) <= minPtr)
		{
			m_FileHistory.resize(nflt);
			for(auto &mptHistory : m_FileHistory)
			{
				ITHistoryStruct itHistory;
				file.ReadStruct(itHistory);
				itHistory.ConvertToMPT(mptHistory);
			}

			if(possiblyUNMO3 && nflt == 0)
			{
				if(fileHeader.special & ITFileHeader::embedPatternHighlights)
					madeWithTracker = UL_("UNMO3 <= 2.4.0.1");  // Set together with MIDI macro embed flag
				else
					madeWithTracker = UL_("UNMO3");  // Either 2.4.0.2+ or no MIDI macros embedded
			}
		} else
		{
			// Oops, we were not supposed to read this.
			file.SkipBack(2);
		}
	} else if(possiblyUNMO3 && fileHeader.special <= 1)
	{
		// UNMO3 < v2.4.0.1 will set the edit history special bit iff the MIDI macro embed bit is also set,
		// but it always writes the two extra bytes for the edit history length (zeros).
		// If MIDI macros are embedded, we are fine and end up in the first case of the if statement (read edit history).
		// Otherwise we end up here and might have to read the edit history length.
		if(file.ReadUint16LE() == 0)
		{
			madeWithTracker = UL_("UNMO3 <= 2.4");
		} else
		{
			// These were not zero bytes, but potentially belong to the upcoming MIDI config - need to skip back.
			// I think the only application that could end up here is CheeseTracker, if it allows to write 0 for both row highlight values.
			// IT 2.14 itself will always write the edit history.
			file.SkipBack(2);
		}
	}

	// Reading MIDI Output & Macros
	bool hasMidiConfig = (fileHeader.flags & ITFileHeader::reqEmbeddedMIDIConfig) || (fileHeader.special & ITFileHeader::embedMIDIConfiguration);
	if(hasMidiConfig && file.ReadStruct<MIDIMacroConfigData>(m_MidiCfg))
	{
		m_MidiCfg.Sanitize();
	}

	bool hasModPlugExtensions = false;

	// Read pattern names: "PNAM"
	FileReader patNames;
	if(file.ReadMagic("PNAM"))
	{
		patNames = file.ReadChunk(file.ReadUint32LE());
		hasModPlugExtensions = true;
	}

	// Read channel names: "CNAM"
	if(file.ReadMagic("CNAM"))
	{
		FileReader chnNames = file.ReadChunk(file.ReadUint32LE());
		ChnSettings.resize(std::min(MAX_BASECHANNELS, static_cast<CHANNELINDEX>(chnNames.GetLength() / MAX_CHANNELNAME)));
		hasModPlugExtensions = true;
		for(auto &chn : ChnSettings)
		{
			chnNames.ReadString<mpt::String::maybeNullTerminated>(chn.szName, MAX_CHANNELNAME);
		}
	}

	// Read mix plugins information
	FileReader pluginChunk = file.ReadChunk((minPtr >= file.GetPosition()) ? minPtr - file.GetPosition() : file.BytesLeft());
	const auto [hasPluginChunks, isBeRoTracker] = LoadMixPlugins(pluginChunk, false);
	if(hasPluginChunks)
		hasModPlugExtensions = true;

	if(fileHeader.cwtv == 0x0217 && fileHeader.cmwt == 0x0200 && fileHeader.reserved == 0 && !isBeRoTracker)
	{
		if(hasModPlugExtensions
			|| (!Order().empty() && Order().back() == PATTERNINDEX_INVALID)
			|| memchr(fileHeader.chnpan, 0xFF, sizeof(fileHeader.chnpan)) != nullptr)
		{
			m_dwLastSavedWithVersion = MPT_V("1.16");
			madeWithTracker = UL_("ModPlug Tracker 1.09 - 1.16");
		} else
		{
			// OpenMPT 1.17 disguised as this in compatible mode,
			// but never writes 0xFF in the pan map for unused channels (which is an invalid value).
			// It also doesn't write a final "---" pattern in the order list.
			// Could also be original ModPlug Tracker though if all 64 channels and no ModPlug extensions are used.
			m_dwLastSavedWithVersion = MPT_V("1.17");
			madeWithTracker = UL_("OpenMPT 1.17 (compatibility export)");
		}
		interpretModPlugMade = true;
	}

	// Read Song Message
	if((fileHeader.special & ITFileHeader::embedSongMessage) && fileHeader.msglength > 0 && file.Seek(fileHeader.msgoffset))
	{
		// Generally, IT files should use CR for line endings. However, ChibiTracker uses LF. One could do...
		// if(itHeader.cwtv == 0x0214 && itHeader.cmwt == 0x0214 && itHeader.reserved == ITFileHeader::chibiMagic) --> Chibi detected.
		// But we'll just use autodetection here:
		m_songMessage.Read(file, fileHeader.msglength, SongMessage::leAutodetect);
	}

	// Reading Instruments
	m_nInstruments = 0;
	if(fileHeader.flags & ITFileHeader::instrumentMode)
	{
		m_nInstruments = std::min(static_cast<INSTRUMENTINDEX>(fileHeader.insnum), static_cast<INSTRUMENTINDEX>(MAX_INSTRUMENTS - 1));
	}
	for(INSTRUMENTINDEX i = 0; i < GetNumInstruments(); i++)
	{
		if(insPos[i] > 0 && file.Seek(insPos[i]) && file.CanRead(fileHeader.cmwt < 0x200 ? sizeof(ITOldInstrument) : sizeof(ITInstrument)))
		{
			ModInstrument *instrument = AllocateInstrument(i + 1);
			if(instrument != nullptr)
			{
				ITInstrToMPT(file, *instrument, fileHeader.cmwt);
				// MIDI Pitch Wheel Depth is a global setting in IT. Apply it to all instruments.
				instrument->midiPWD = fileHeader.pwd;
			}
		}
	}

	// In order to properly compute the position, in file, of eventual extended settings
	// such as "attack" we need to keep the "real" size of the last sample as those extra
	// setting will follow this sample in the file
	FileReader::pos_type lastSampleOffset = 0;
	if(fileHeader.smpnum > 0)
	{
		lastSampleOffset = smpPos[fileHeader.smpnum - 1] + sizeof(ITSample);
	}

	bool possibleXMconversion = false;

	// There's a bug in IT somewhere that resets the "sample data present" flag in sample headers, but keeps the sample length
	// of a previously deleted sample (presumably).
	// As old ModPlug versions didn't set this flag under some circumstances (if a sample wasn't referenced by any instruments in instrument mode),
	// and because there appear to be some external tools that forget to set this flag at all, we only respect the flag if the file
	// vaguely looks like it was saved with IT. Some files that play garbage data if we don't do this:
	// astral projection.it by Lord Jon Ray
	// classic illusions.it by Blackstar
	// deep in dance.it by Simply DJ
	// There are many more such files but they don't reference the broken samples in their pattern data, or the sample data pointer
	// points right to the end of the file, so in both cases no audible problem can be observed.
	const bool muteBuggySamples = !interpretModPlugMade && fileHeader.cwtv >= 0x0100 && fileHeader.cwtv <= 0x0217
		&& (fileHeader.cwtv < 0x0207 || fileHeader.reserved != 0);

	// Reading Samples
	m_nSamples = std::min(static_cast<SAMPLEINDEX>(fileHeader.smpnum), static_cast<SAMPLEINDEX>(MAX_SAMPLES - 1));
	bool lastSampleCompressed = false, anyADPCM = false;
	for(SAMPLEINDEX i = 0; i < GetNumSamples(); i++)
	{
		ITSample sampleHeader;
		if(smpPos[i] > 0 && file.Seek(smpPos[i]) && file.ReadStruct(sampleHeader))
		{
			ModSample &sample = Samples[i + 1];
			size_t sampleOffset = sampleHeader.ConvertToMPT(sample);
			if(muteBuggySamples && !(sampleHeader.flags & ITSample::sampleDataPresent))
				sample.nLength = 0;

			m_szNames[i + 1] = mpt::String::ReadBuf(mpt::String::spacePadded, sampleHeader.name);

			if(!file.Seek(sampleOffset))
				continue;

			lastSampleCompressed = false;
			if(sample.uFlags[CHN_ADLIB])
			{
				// FM instrument in MPTM
				OPLPatch patch;
				if(file.ReadArray(patch))
				{
					sample.SetAdlib(true, patch);
				}
			} else if(!sample.uFlags[SMP_KEEPONDISK])
			{
				SampleIO sampleIO = sampleHeader.GetSampleFormat(fileHeader.cwtv);
				if(loadFlags & loadSampleData)
				{
					sampleIO.ReadSample(sample, file);
				} else
				{
					if(sampleIO.IsVariableLengthEncoded())
						lastSampleCompressed = true;
					else
						file.Skip(sampleIO.CalculateEncodedSize(sample.nLength));
				}
				if(sampleIO.GetEncoding() == SampleIO::unsignedPCM && sample.nLength != 0)
				{
					// There is some XM to IT converter (don't know which one) and it identifies as IT 2.04.
					// The only safe way to distinguish it from an IT-saved file are the unsigned samples.
					possibleXMconversion = true;
				} else if(sampleIO.GetEncoding() == SampleIO::ADPCM)
				{
					anyADPCM = true;
				}
			} else
			{
				// External sample in MPTM file
				size_t strLen;
				file.ReadVarInt(strLen);
				if((loadFlags & loadSampleData) && strLen)
				{
					std::string filenameU8;
					file.ReadString<mpt::String::maybeNullTerminated>(filenameU8, strLen);
#if defined(MPT_EXTERNAL_SAMPLES)
					SetSamplePath(i + 1, mpt::PathString::FromUTF8(filenameU8));
#elif !defined(LIBOPENMPT_BUILD_TEST)
					AddToLog(LogWarning, MPT_UFORMAT("Loading external sample {} ('{}') failed: External samples are not supported.")(i + 1, mpt::ToUnicode(mpt::Charset::UTF8, filenameU8)));
#endif  // MPT_EXTERNAL_SAMPLES
				} else
				{
					file.Skip(strLen);
				}
			}
			lastSampleOffset = std::max(lastSampleOffset, file.GetPosition());
		}
	}
	m_nSamples = std::max(SAMPLEINDEX(1), GetNumSamples());

	if(possibleXMconversion && fileHeader.cwtv == 0x0204 && fileHeader.cmwt == 0x0200 && fileHeader.special == 0 && fileHeader.reserved == 0
		&& (fileHeader.flags & ~ITFileHeader::linearSlides) == (ITFileHeader::useStereoPlayback | ITFileHeader::instrumentMode | ITFileHeader::itOldEffects)
		&& fileHeader.globalvol == 128 && fileHeader.mv == 48 && fileHeader.sep == 128 && fileHeader.pwd == 0 && fileHeader.msglength == 0)
	{
		for(uint8 pan : fileHeader.chnpan)
		{
			if(pan != 0x20 && pan != 0xA0)
				possibleXMconversion = false;
		}
		for(uint8 vol : fileHeader.chnvol)
		{
			if(vol != 0x40)
				possibleXMconversion = false;
		}
		for(size_t i = 20; i < std::size(fileHeader.songname); i++)
		{
			if(fileHeader.songname[i] != 0)
				possibleXMconversion = false;
		}
		if(possibleXMconversion)
			madeWithTracker = UL_("XM Conversion");
	}

	m_nMinPeriod = 0;
	m_nMaxPeriod = int32_max;

	PATTERNINDEX numPats = std::min(static_cast<PATTERNINDEX>(patPos.size()), GetModSpecifications().patternsMax);

	if(numPats != patPos.size())
	{
		// Hack: Notify user here if file contains more patterns than what can be read.
		AddToLog(LogWarning, MPT_UFORMAT("The module contains {} patterns but only {} patterns can be loaded in this OpenMPT version.")(patPos.size(), numPats));
	}

	if(!(loadFlags & loadPatternData))
	{
		numPats = 0;
	}

	// Checking for number of used channels, which is not explicitly specified in the file.
	CHANNELINDEX numChannels = std::max(GetNumChannels(), CHANNELINDEX(1));
	for(PATTERNINDEX pat = 0; pat < numPats; pat++)
	{
		if(patPos[pat] == 0 || !file.Seek(patPos[pat]))
			continue;

		uint16 len = file.ReadUint16LE();
		ROWINDEX numRows = file.ReadUint16LE();

		if(numRows < 1
			|| numRows > MAX_PATTERN_ROWS
			|| !file.Skip(4))
			continue;

		FileReader patternData = file.ReadChunk(len);
		ROWINDEX row = 0;
		std::vector<uint8> chnMask(numChannels);

		while(row < numRows && patternData.CanRead(1))
		{
			uint8 b = patternData.ReadUint8();
			if(!b)
			{
				row++;
				continue;
			}

			CHANNELINDEX ch = (b & IT_bitmask_patternChanField_c);   // 0x7f We have some data grab a byte keeping only 7 bits
			if(ch)
			{
				ch = (ch - 1);// & IT_bitmask_patternChanMask_c;   // 0x3f mask of the byte again, keeping only 6 bits
			}

			if(ch >= chnMask.size())
			{
				chnMask.resize(ch + 1, 0);
			}

			if(b & IT_bitmask_patternChanEnabled_c)  // 0x80 check if the upper bit is enabled.
			{
				chnMask[ch] = patternData.ReadUint8();  // set the channel mask for this channel.
			}
			// Channel used
			if(chnMask[ch] & 0x0F)  // if this channel is used set m_nChannels
			{
				if(ch >= numChannels && ch < MAX_BASECHANNELS)
				{
					numChannels = ch + 1;
				}

				// Skip a number of bytes depending on note, instrument, volume, effect being present.
				static constexpr uint8 maskToSkips[] = {0, 1, 1, 2, 1, 2, 2, 3, 2, 3, 3, 4, 3, 4, 4, 5};
				patternData.Skip(maskToSkips[chnMask[ch] & 0x0F]);
			}
		}
		lastSampleOffset = std::max(lastSampleOffset, file.GetPosition());
	}
	ChnSettings.resize(numChannels);

	// Compute extra instruments settings position
	if(lastSampleOffset > 0)
	{
		file.Seek(lastSampleOffset);
		if(lastSampleCompressed)
		{
			// If the last sample was compressed, we do not know where it ends.
			// Hence, in case we decided not to decode the sample data, we now
			// have to emulate this until we reach EOF or some instrument / song properties.
			while(file.CanRead(4))
			{
				if(file.ReadMagic("XTPM") || file.ReadMagic("STPM"))
				{
					uint32 id = file.ReadUint32LE();
					file.SkipBack(8);
					// Our chunk IDs should only contain ASCII characters
					if(!(id & 0x80808080) && (id & 0x60606060))
					{
						break;
					}
				}
				file.Skip(file.ReadUint16LE());
			}
		}
	}

	// Load instrument and song extensions.
	const bool hasExtendedInstrumentProperties = LoadExtendedInstrumentProperties(file);
	interpretModPlugMade |= hasExtendedInstrumentProperties;
	if(interpretModPlugMade && !isBeRoTracker)
	{
		m_playBehaviour.reset();
		m_nMixLevels = MixLevels::Original;
	}
	// Need to do this before reading the patterns because m_nChannels might be modified by LoadExtendedSongProperties. *sigh*
	const bool hasExtendedSongProperties = LoadExtendedSongProperties(file, false, &interpretModPlugMade);

	// Reading Channels Pan Positions
	const CHANNELINDEX headerChannels = std::min(GetNumChannels(), CHANNELINDEX(64));
	for(CHANNELINDEX i = 0; i < headerChannels; i++)
	{
		if(fileHeader.chnpan[i] == 0xFF)
			continue;
		ChnSettings[i].nVolume = Clamp<uint8, uint8>(fileHeader.chnvol[i], 0, 64);
		if(fileHeader.chnpan[i] & 0x80)
			ChnSettings[i].dwFlags.set(CHN_MUTE);
		uint8 n = fileHeader.chnpan[i] & 0x7F;
		if(n <= 64)
			ChnSettings[i].nPan = n * 4;
		if(n == 100)
			ChnSettings[i].dwFlags.set(CHN_SURROUND);
	}

	// Reading Patterns
	Patterns.ResizeArray(numPats);
	bool hasVolColOffset = false;
	for(PATTERNINDEX pat = 0; pat < numPats; pat++)
	{
		if(patPos[pat] == 0 || !file.Seek(patPos[pat]))
		{
			// Empty 64-row pattern
			if(!Patterns.Insert(pat, 64))
			{
				AddToLog(LogWarning, MPT_UFORMAT("Allocating patterns failed starting from pattern {}")(pat));
				break;
			}
			// Now (after the Insert() call), we can read the pattern name.
			CopyPatternName(Patterns[pat], patNames);
			continue;
		}

		uint16 len = file.ReadUint16LE();
		ROWINDEX numRows = file.ReadUint16LE();

		if(!file.Skip(4)
			|| !Patterns.Insert(pat, numRows))
			continue;

		FileReader patternData = file.ReadChunk(len);

		// Now (after the Insert() call), we can read the pattern name.
		CopyPatternName(Patterns[pat], patNames);

		std::vector<uint8> chnMask(GetNumChannels());
		std::vector<ModCommand> lastValue(GetNumChannels(), ModCommand{});

		auto patData = Patterns[pat].begin();
		ROWINDEX row = 0;
		ModCommand dummy{};
		while(row < numRows && patternData.CanRead(1))
		{
			uint8 b = patternData.ReadUint8();
			if(!b)
			{
				row++;
				patData += GetNumChannels();
				continue;
			}

			CHANNELINDEX ch = b & IT_bitmask_patternChanField_c; // 0x7f

			if(ch)
			{
				ch = (ch - 1); //& IT_bitmask_patternChanMask_c; // 0x3f
			}

			if(ch >= chnMask.size())
			{
				chnMask.resize(ch + 1, 0);
				lastValue.resize(ch + 1, ModCommand{});
				MPT_ASSERT(chnMask.size() <= GetNumChannels());
			}

			if(b & IT_bitmask_patternChanEnabled_c)  // 0x80
			{
				chnMask[ch] = patternData.ReadUint8();
			}

			// Now we grab the data for this particular row/channel.
			ModCommand &m = ch < GetNumChannels() ? patData[ch] : dummy;

			if(chnMask[ch] & 0x10)
			{
				m.note = lastValue[ch].note;
			}
			if(chnMask[ch] & 0x20)
			{
				m.instr = lastValue[ch].instr;
			}
			if(chnMask[ch] & 0x40)
			{
				m.volcmd = lastValue[ch].volcmd;
				m.vol = lastValue[ch].vol;
			}
			if(chnMask[ch] & 0x80)
			{
				m.command = lastValue[ch].command;
				m.param = lastValue[ch].param;
			}
			if(chnMask[ch] & 1)	// Note
			{
				uint8 note = patternData.ReadUint8();
				if(note < 0x80)
					note += NOTE_MIN;
				else if(note == 0xFF)
					note = NOTE_KEYOFF;
				else if(note == 0xFE)
					note = NOTE_NOTECUT;
				else if(note == 0xFD && GetType() != MOD_TYPE_MPT)
					note = NOTE_NONE;  // Note: in MPTM format, NOTE_FADE is written as 0xFD to preserve compatibility with older OpenMPT versions.
				else
					note = NOTE_FADE;
				m.note = lastValue[ch].note = note;
			}
			if(chnMask[ch] & 2)
			{
				m.instr = lastValue[ch].instr = patternData.ReadUint8();
			}
			if(chnMask[ch] & 4)
			{
				uint8 vol = patternData.ReadUint8();
				// 0-64: Set Volume
				if(vol <= 64) { m.volcmd = VOLCMD_VOLUME; m.vol = vol; } else
				// 128-192: Set Panning
				if(vol >= 128 && vol <= 192) { m.volcmd = VOLCMD_PANNING; m.vol = vol - 128; } else
				// 65-74: Fine Volume Up
				if(vol < 75) { m.volcmd = VOLCMD_FINEVOLUP; m.vol = vol - 65; } else
				// 75-84: Fine Volume Down
				if(vol < 85) { m.volcmd = VOLCMD_FINEVOLDOWN; m.vol = vol - 75; } else
				// 85-94: Volume Slide Up
				if(vol < 95) { m.volcmd = VOLCMD_VOLSLIDEUP; m.vol = vol - 85; } else
				// 95-104: Volume Slide Down
				if(vol < 105) { m.volcmd = VOLCMD_VOLSLIDEDOWN; m.vol = vol - 95; } else
				// 105-114: Pitch Slide Up
				if(vol < 115) { m.volcmd = VOLCMD_PORTADOWN; m.vol = vol - 105; } else
				// 115-124: Pitch Slide Down
				if(vol < 125) { m.volcmd = VOLCMD_PORTAUP; m.vol = vol - 115; } else
				// 193-202: Portamento To
				if(vol >= 193 && vol <= 202) { m.volcmd = VOLCMD_TONEPORTAMENTO; m.vol = vol - 193; } else
				// 203-212: Vibrato depth
				if(vol >= 203 && vol <= 212)
				{
					m.volcmd = VOLCMD_VIBRATODEPTH;
					m.vol = vol - 203;
					// Old versions of ModPlug saved this as vibrato speed instead, so let's fix that.
					if(m.vol && m_dwLastSavedWithVersion && m_dwLastSavedWithVersion <= MPT_V("1.17.02.54"))
						m.volcmd = VOLCMD_VIBRATOSPEED;
				} else
				// 213-222: Unused (was velocity)
				// 223-232: Offset
				if(vol >= 223 && vol <= 232) { m.volcmd = VOLCMD_OFFSET; m.vol = vol - 223; hasVolColOffset = true; }
				lastValue[ch].volcmd = m.volcmd;
				lastValue[ch].vol = m.vol;
			}
			// Reading command/param
			if(chnMask[ch] & 8)
			{
				const auto [command, param] = patternData.ReadArray<uint8, 2>();
				S3MConvert(m, command, param, true);

				// IT 1.xx does not support high offset command
				if(m.command == CMD_S3MCMDEX && (m.param & 0xF0) == 0xA0 && fileHeader.cwtv < 0x0200)
					m.command = CMD_DUMMY;
				// Fix handling of commands V81-VFF in ITs made with old Schism Tracker versions
				// (fixed in https://github.com/schismtracker/schismtracker/commit/ab5517d4730d4c717f7ebffb401445679bd30888 - one of the last versions to identify as v0.50)
				else if(m.command == CMD_GLOBALVOLUME && m.param > 0x80 && fileHeader.cwtv >= 0x1000 && fileHeader.cwtv <= 0x1050)
					m.param = 0x80;

				// In some IT-compatible trackers, it is possible to input a parameter without a command.
				// In this case, we still need to update the last value memory (so that we don't reuse a previous non-empty effect).
				// OpenMPT didn't do this until v1.25.01.07.
				// Example: ckbounce.it
				lastValue[ch].command = m.command;
				lastValue[ch].param = m.param;
			}
		}
	}
	// Remove (default) cue points if no volume column offset is found (unless it's a new enough MPTM file, which could contain intentionally-placed custom cue points that we don't want to lose)
	if(!hasVolColOffset && (GetType() != MOD_TYPE_MPT || m_dwLastSavedWithVersion < MPT_V("1.24.02.06")))
	{
		for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
		{
			Samples[smp].RemoveAllCuePoints();
		}
	}

	if(!m_dwLastSavedWithVersion && fileHeader.cwtv == 0x0888)
	{
		// Up to OpenMPT 1.17.02.45 (r165), it was possible that the "last saved with" field was 0
		// when saving a file in OpenMPT for the first time.
		m_dwLastSavedWithVersion = MPT_V("1.17.00.00");
	}

	if(m_dwLastSavedWithVersion && madeWithTracker.empty())
	{
		madeWithTracker = UL_("OpenMPT ") + mpt::ufmt::val(m_dwLastSavedWithVersion);
		
		bool isCompatExport = memcmp(&fileHeader.reserved, "OMPT", 4) && (fileHeader.cwtv & 0xF000) == 0x5000;
		if(m_dwLastSavedWithVersion == MPT_V("1.17.00.00"))
			isCompatExport = !hasExtendedInstrumentProperties && !hasExtendedSongProperties && !hasModPlugExtensions;

		if(isCompatExport)
		{
			madeWithTracker += UL_(" (compatibility export)");
		} else if(m_dwLastSavedWithVersion.IsTestVersion())
		{
			madeWithTracker += UL_(" (test build)");
		}
	} else
	{
		const int32 schismDateVersion = SchismTrackerEpoch + ((fileHeader.cwtv == 0x1FFF) ? fileHeader.reserved : (fileHeader.cwtv - 0x1050));
		switch(fileHeader.cwtv >> 12)
		{
		case 0:
			if(isBeRoTracker)
			{
				// Old versions
				madeWithTracker = UL_("BeRoTracker");
			} else if(fileHeader.cwtv == 0x0214 && fileHeader.cmwt == 0x0200 && fileHeader.flags == 9 && fileHeader.special == 0
				&& fileHeader.highlight_major == 0 && fileHeader.highlight_minor == 0
				&& fileHeader.insnum == 0 && fileHeader.patnum + 1 == fileHeader.ordnum
				&& fileHeader.globalvol == 128 && fileHeader.mv == 100 && fileHeader.speed == 1 && fileHeader.sep == 128 && fileHeader.pwd == 0
				&& fileHeader.msglength == 0 && fileHeader.msgoffset == 0 && fileHeader.reserved == 0)
			{
				madeWithTracker = UL_("OpenSPC conversion");
			} else if(fileHeader.cwtv == 0x0202 && fileHeader.cmwt == 0x0200 && fileHeader.highlight_major == 0 && fileHeader.highlight_minor == 0 && fileHeader.reserved == 0 && !patPos.empty() && !smpPos.empty() && patPos[0] != 0 && patPos[0] < smpPos[0])
			{
				// ModPlug Tracker 1.0 pre-alpha up to alpha 4, patterns located before instruments / samples
				m_dwLastSavedWithVersion = MPT_V("1.00.00.A0");
				madeWithTracker = UL_("ModPlug Tracker 1.0 pre-alpha / alpha");
				interpretModPlugMade = true;
			} else if(fileHeader.cwtv == 0x0214 && fileHeader.cmwt == 0x0200 && fileHeader.highlight_major == 0 && fileHeader.highlight_minor == 0 && fileHeader.reserved == 0)
			{
				if(fileHeader.special & (ITFileHeader::embedPatternHighlights | ITFileHeader::embedEditHistory))
				{
					// ModPlug Tracker 1.0a6/b1/b2
					// Instruments are 557 bytes apart in beta 2.3, in beta 1 still 560 bytes like in earlier versions
					if(insPos.size() >= 2 && insPos[1] - insPos[0] == 557)
					{
						m_dwLastSavedWithVersion = MPT_V("1.00.00.B2");
						madeWithTracker = UL_("ModPlug Tracker 1.0b2");
					} else
					{
						m_dwLastSavedWithVersion = MPT_V("1.00.00.B1");
						madeWithTracker = UL_("ModPlug Tracker 1.0 alpha / beta");
					}
				} else
				{
					// ModPlug Tracker 1.0a5, instruments 560 bytes apart
					m_dwLastSavedWithVersion = MPT_V("1.00.00.A5");
					madeWithTracker = UL_("ModPlug Tracker 1.0a5");
				}
				interpretModPlugMade = true;
			} else if(fileHeader.cwtv == 0x0214 && fileHeader.cmwt == 0x0214 && !memcmp(&fileHeader.reserved, "CHBI", 4))
			{
				madeWithTracker = UL_("ChibiTracker");
				m_playBehaviour.reset(kITShortSampleRetrig);
				m_nSamplePreAmp /= 2;
			} else if(fileHeader.cwtv == 0x0214 && fileHeader.cmwt == 0x0214 && fileHeader.special <= 1 && fileHeader.pwd == 0 && fileHeader.reserved == 0
				&& (fileHeader.flags & (ITFileHeader::vol0Optimisations | ITFileHeader::instrumentMode | ITFileHeader::useMIDIPitchController | ITFileHeader::reqEmbeddedMIDIConfig | ITFileHeader::extendedFilterRange)) == ITFileHeader::instrumentMode
				&& m_nSamples > 1 && (Samples[1].filename == "XXXXXXXX.YYY"))
			{
				madeWithTracker = UL_("CheeseTracker");
			} else if(fileHeader.cwtv == 0 && madeWithTracker.empty())
			{
				madeWithTracker = UL_("Unknown");
			} else if(fileHeader.cwtv >= 0x0208 && fileHeader.cwtv <= 0x0214 && !fileHeader.reserved && m_FileHistory.empty() && madeWithTracker.empty())
			{
				// Any file made with IT starting from v2.07 onwards should have an edit history
				madeWithTracker = UL_("Unknown");
			} else if(fileHeader.cmwt < 0x0300 && madeWithTracker.empty())
			{
				madeWithTracker = GetImpulseTrackerVersion(fileHeader.cwtv, fileHeader.cmwt);
				if(m_FileHistory.empty() && fileHeader.reserved != 0)
				{
					// Starting from  version 2.07, IT stores the total edit time of a module in the "reserved" field
					uint32 editTime = DecodeITEditTimer(fileHeader.cwtv, fileHeader.reserved);

					FileHistory hist;
					hist.openTime = static_cast<uint32>(editTime * (HISTORY_TIMER_PRECISION / 18.2));
					m_FileHistory.push_back(hist);
				}
			}
			break;
		case 1:
			madeWithTracker = GetSchismTrackerVersion(fileHeader.cwtv, fileHeader.reserved);
			{
				static constexpr std::pair<int32, PlayBehaviour> SchismQuirks[] =
				{
					{SchismVersionFromDate<2015,  1, 29>::date, kPeriodsAreHertz              },  // https://github.com/schismtracker/schismtracker/commit/671b30311082a0e7df041fca25f989b5d2478f69
					{SchismVersionFromDate<2016,  5, 13>::date, kITShortSampleRetrig          },  // https://github.com/schismtracker/schismtracker/commit/e7b1461fe751554309fd403713c2a1ef322105ca
					{SchismVersionFromDate<2021,  5,  2>::date, kITDoNotOverrideChannelPan    },  // https://github.com/schismtracker/schismtracker/commit/a34ec86dc819915debc9e06f4727b77bf2dd29ee
					{SchismVersionFromDate<2021,  5,  2>::date, kITPanningReset               },  // https://github.com/schismtracker/schismtracker/commit/648f5116f984815c69e11d018b32dfec53c6b97a
					{SchismVersionFromDate<2021, 11,  1>::date, kITPitchPanSeparation         },  // https://github.com/schismtracker/schismtracker/commit/6e9f1207015cae0fe1b829fff7bb867e02ec6dea
					{SchismVersionFromDate<2022,  4, 30>::date, kITEmptyNoteMapSlot           },  // https://github.com/schismtracker/schismtracker/commit/1b2f7d5522fcb971f134a6664182ca569f7c8008
					{SchismVersionFromDate<2022,  4, 30>::date, kITPortamentoSwapResetsPos    },  // https://github.com/schismtracker/schismtracker/commit/1b2f7d5522fcb971f134a6664182ca569f7c8008
					{SchismVersionFromDate<2022,  4, 30>::date, kITMultiSampleInstrumentNumber},  // https://github.com/schismtracker/schismtracker/commit/1b2f7d5522fcb971f134a6664182ca569f7c8008
					{SchismVersionFromDate<2023,  3,  9>::date, kITInitialNoteMemory          },  // https://github.com/schismtracker/schismtracker/commit/73e9d60676c2b48c8e94e582373e29517105b2b1
					{SchismVersionFromDate<2023, 10, 17>::date, kITDCTBehaviour               },  // https://github.com/schismtracker/schismtracker/commit/31d36dc00013fc5ab0efa20c782af18e8b006e07
					{SchismVersionFromDate<2023, 10, 19>::date, kITSampleAndHoldPanbrello     },  // https://github.com/schismtracker/schismtracker/commit/411ec16b190ba1a486d8b0907ad8d74f8fdc2840
					{SchismVersionFromDate<2023, 10, 19>::date, kITPortaNoNote                },  // https://github.com/schismtracker/schismtracker/commit/8ff0a86a715efb50c89770fb9095d4c4089ff187
					{SchismVersionFromDate<2023, 10, 22>::date, kITFirstTickHandling          },  // https://github.com/schismtracker/schismtracker/commit/b9609e4f827e1b6ce9ebe6573b85e69388ca0ea0
					{SchismVersionFromDate<2023, 10, 22>::date, kITMultiSampleInstrumentNumber},  // https://github.com/schismtracker/schismtracker/commit/a9e5df533ab52c35190fcc1cbfed4f0347b660bb
					{SchismVersionFromDate<2024,  3,  9>::date, kITPanbrelloHold              },  // https://github.com/schismtracker/schismtracker/commit/ebdebaa8c8a735a7bf49df55debded1b7aac3605
					{SchismVersionFromDate<2024,  5, 12>::date, kITNoSustainOnPortamento      },  // https://github.com/schismtracker/schismtracker/commit/6f68f2855a7e5e4ffe825869244e631e15741037
					{SchismVersionFromDate<2024,  5, 12>::date, kITEmptyNoteMapSlotIgnoreCell },  // https://github.com/schismtracker/schismtracker/commit/aa84148e019a65f3d52ecd33fd84bfecfdb87bf4
					{SchismVersionFromDate<2024,  5, 27>::date, kITOffsetWithInstrNumber      },  // https://github.com/schismtracker/schismtracker/commit/9237960d45079a54ad73f87bacfe5dd8ae82e273
					{SchismVersionFromDate<2024, 10, 13>::date, kITDoublePortamentoSlides     },  // https://github.com/schismtracker/schismtracker/commit/223e327d9448561931b8cac8a55180286b17276c
					{SchismVersionFromDate<2025,  1,  8>::date, kITCarryAfterNoteOff          },  // https://github.com/schismtracker/schismtracker/commit/ff7a817df327c8f13d97b8c6546a9329f59edff8
				};
				for(const auto &quirk : SchismQuirks)
				{
					if(schismDateVersion < quirk.first)
						m_playBehaviour.reset(quirk.second);
				}
			}
			// Hertz in Amiga mode: Added 2021-05-02, https://github.com/schismtracker/schismtracker/commit/c656a6cbd5aaf81198a7580faf81cb7960cb6afa
			if(schismDateVersion < SchismVersionFromDate<2021, 05, 02>::date && !m_SongFlags[SONG_LINEARSLIDES])
				m_playBehaviour.reset(kPeriodsAreHertz);
			// Imprecise calculation of ping-pong loop wraparound: Added 2021-11-01, https://github.com/schismtracker/schismtracker/commit/22cbb9b676e9c2c9feb7a6a17deca7a17ac138cc
			if(schismDateVersion < SchismVersionFromDate<2021, 11, 01>::date)
				m_playBehaviour.set(kImprecisePingPongLoops);
			break;
		case 4:
			madeWithTracker = MPT_UFORMAT("pyIT {}.{}")((fileHeader.cwtv & 0x0F00) >> 8, mpt::ufmt::hex0<2>(fileHeader.cwtv & 0xFF));
			break;
		case 6:
			madeWithTracker = UL_("BeRoTracker");
			break;
		case 7:
			if(fileHeader.cwtv == 0x7FFF && fileHeader.cmwt == 0x0215)
				madeWithTracker = UL_("munch.py");
			else
				madeWithTracker = MPT_UFORMAT("ITMCK {}.{}.{}")((fileHeader.cwtv >> 8) & 0x0F, (fileHeader.cwtv >> 4) & 0x0F, fileHeader.cwtv & 0x0F);
			break;
		case 0xD:
			if(fileHeader.cwtv == 0xDAEB)
				madeWithTracker = UL_("spc2it");
			else if(fileHeader.cwtv == 0xD1CE)
				madeWithTracker = UL_("itwriter");
			else
				madeWithTracker = UL_("Unknown");
			break;
		}
	}

	if(anyADPCM)
		madeWithTracker += UL_(" (ADPCM packed)");

	// Ignore MIDI data. Fixes some files like denonde.it that were made with old versions of Impulse Tracker (which didn't support Zxx filters) and have Zxx effects in the patterns.
	// Example: denonde.it by Mystical
	// Note: Only checking the cwtv "made with" version is not enough: spx-visionsofthepast.it has the strange combination of cwtv=2.00, cmwt=2.16
	// Hence to be sure, we check that both values are below 2.14.
	// Note that all ModPlug Tracker alpha versions do not support filters yet. Earlier alphas identify as cwtv=2.02, cmwt=2.00, but later alpha versions identify as IT 2.14.
	// Apart from that, there's an unknown XM conversion tool declaring a lower comaptible version, which naturally also does not support filters, so it's okay that it is caught here.
	if((fileHeader.cwtv < 0x0214 && fileHeader.cmwt < 0x0214) || (m_dwLastSavedWithVersion && m_dwLastSavedWithVersion <= MPT_V("1.00.00.A6")))
	{
		m_MidiCfg.ClearZxxMacros();
	}

	if(GetType() == MOD_TYPE_MPT)
	{
		// START - mpt specific:
		if(fileHeader.cwtv > 0x0889 && file.Seek(mptStartPos))
		{
			LoadMPTMProperties(file, fileHeader.cwtv);
		}
	}

	m_modFormat.formatName = (GetType() == MOD_TYPE_MPT) ? U_("OpenMPT MPTM") : MPT_UFORMAT("Impulse Tracker {}.{}")(fileHeader.cmwt >> 8, mpt::ufmt::hex0<2>(fileHeader.cmwt & 0xFF));
	m_modFormat.type = (GetType() == MOD_TYPE_MPT) ? U_("mptm") : U_("it");
	m_modFormat.madeWithTracker = std::move(madeWithTracker);
	m_modFormat.charset = m_dwLastSavedWithVersion ? mpt::Charset::Windows1252 : mpt::Charset::CP437;
#if MPT_TIME_UTC_ON_DISK
#ifdef MODPLUG_TRACKER
	m_modFormat.timezone = (m_dwLastSavedWithVersion && (m_dwLastSavedWithVersion >= MPT_TIME_UTC_ON_DISK_VERSION)) ? mpt::Date::LogicalTimezone::UTC : mpt::Date::LogicalTimezone::Local;
#else
	m_modFormat.timezone = (m_dwLastSavedWithVersion && (m_dwLastSavedWithVersion >= MPT_TIME_UTC_ON_DISK_VERSION)) ? mpt::Date::LogicalTimezone::UTC : mpt::Date::LogicalTimezone::Unspecified;
#endif
#else
#ifdef MODPLUG_TRACKER
	m_modFormat.timezone = mpt::Date::LogicalTimezone::Local;
#else
	m_modFormat.timezone = mpt::Date::LogicalTimezone::Unspecified;
#endif
#endif

	return true;
}


void CSoundFile::LoadMPTMProperties(FileReader &file, uint16 cwtv)
{
	std::istringstream iStrm(mpt::buffer_cast<std::string>(file.GetRawDataAsByteVector()));

	if(cwtv >= 0x88D)
	{
		srlztn::SsbRead ssb(iStrm);
		ssb.BeginRead("mptm", Version::Current().GetRawVersion());
		int8 useUTF8Tuning = 0;
		ssb.ReadItem(useUTF8Tuning, "UTF8Tuning");
		mpt::Charset TuningCharset = useUTF8Tuning ? mpt::Charset::UTF8 : GetCharsetInternal();
		ssb.ReadItem(GetTuneSpecificTunings(), "0", [TuningCharset](std::istream &iStrm, CTuningCollection &tc, const std::size_t dummy){ return ReadTuningCollection(iStrm, tc, dummy, TuningCharset); });
		ssb.ReadItem(*this, "1", [TuningCharset](std::istream& iStrm, CSoundFile& csf, const std::size_t dummy){ return ReadTuningMap(iStrm, csf, dummy, TuningCharset); });
		ssb.ReadItem(Order, "2", &ReadModSequenceOld);
		ssb.ReadItem(Patterns, FileIdPatterns, &ReadModPatterns);
		mpt::Charset sequenceDefaultCharset = GetCharsetInternal();
		ssb.ReadItem(Order, FileIdSequences, [sequenceDefaultCharset](std::istream &iStrm, ModSequenceSet &seq, std::size_t nSize){ return ReadModSequences(iStrm, seq, nSize, sequenceDefaultCharset); });

		if(ssb.HasFailed())
		{
			AddToLog(LogError, U_("Unknown error occurred while deserializing file."));
		}
	} else
	{
		// Loading for older files.
		mpt::ustring name;
		if(GetTuneSpecificTunings().Deserialize(iStrm, name, GetCharsetInternal()) != Tuning::SerializationResult::Success)
		{
			AddToLog(LogError, U_("Loading tune specific tunings failed."));
		} else
		{
			ReadTuningMapImpl(iStrm, *this, GetCharsetInternal(), 0, cwtv < 0x88C);
		}
	}
}


#ifndef MODPLUG_NO_FILESAVE

// Save edit history. Pass a null pointer for *f to retrieve the number of bytes that would be written.
static uint32 SaveITEditHistory(const CSoundFile &sndFile, std::ostream *file)
{
	size_t num = sndFile.GetFileHistory().size();
#ifdef MODPLUG_TRACKER
	const CModDoc *pModDoc = sndFile.GetpModDoc();
	num += (pModDoc != nullptr) ? 1 : 0;	// + 1 for this session
#endif // MODPLUG_TRACKER

	uint16 fnum = mpt::saturate_cast<uint16>(num);	// Number of entries that are actually going to be written
	const uint32 bytesWritten = 2 + fnum * 8;		// Number of bytes that are actually going to be written

	if(!file)
	{
		return bytesWritten;
	}
	std::ostream & f = *file;

	// Write number of history entries
	mpt::IO::WriteIntLE(f, fnum);

	// Write history data
	const size_t start = (num > uint16_max) ? num - uint16_max : 0;
	for(size_t n = start; n < num; n++)
	{
		FileHistory mptHistory;

#ifdef MODPLUG_TRACKER
		if(n < sndFile.GetFileHistory().size())
#endif // MODPLUG_TRACKER
		{
			// Previous timestamps
			mptHistory = sndFile.GetFileHistory()[n];
#ifdef MODPLUG_TRACKER
		} else if(pModDoc != nullptr)
		{
			// Current ("new") timestamp
			const mpt::Date::Unix creationTime = pModDoc->GetCreationTime();
			if(sndFile.GetTimezoneInternal() == mpt::Date::LogicalTimezone::UTC)
			{
				mptHistory.loadDate = mpt::Date::forget_timezone(mpt::Date::UnixAsUTC(creationTime));
			} else if(sndFile.GetTimezoneInternal() == mpt::Date::LogicalTimezone::Local)
			{
				mptHistory.loadDate = mpt::Date::forget_timezone(mpt::Date::UnixAsLocal(creationTime));
			} else
			{
				// assume UTC
				mptHistory.loadDate = mpt::Date::forget_timezone(mpt::Date::UnixAsUTC(creationTime));
			}
			mptHistory.openTime = static_cast<uint32>(mpt::round((mpt::Date::UnixAsSeconds(mpt::Date::UnixNow()) - mpt::Date::UnixAsSeconds(creationTime)) * HISTORY_TIMER_PRECISION));
#endif // MODPLUG_TRACKER
		}

		ITHistoryStruct itHistory;
		itHistory.ConvertToIT(mptHistory);
		mpt::IO::Write(f, itHistory);
	}

	return bytesWritten;
}


bool CSoundFile::SaveIT(std::ostream &f, const mpt::PathString &filename, bool compatibilityExport)
{

	const CModSpecifications &specs = (GetType() == MOD_TYPE_MPT ? ModSpecs::mptm : (compatibilityExport ? ModSpecs::it : ModSpecs::itEx));

	uint32 dwChnNamLen;
	ITFileHeader itHeader;
	uint64 dwPos = 0;
	uint32 dwHdrPos = 0, dwExtra = 0;

	// Writing Header
	MemsetZero(itHeader);
	dwChnNamLen = 0;
	memcpy(itHeader.id, "IMPM", 4);
	mpt::String::WriteBuf(mpt::String::nullTerminated, itHeader.songname) = m_songName;

	itHeader.highlight_minor = mpt::saturate_cast<uint8>(m_nDefaultRowsPerBeat);
	itHeader.highlight_major = mpt::saturate_cast<uint8>(m_nDefaultRowsPerMeasure);

	if(GetType() == MOD_TYPE_MPT)
	{
		itHeader.ordnum = Order().GetLengthTailTrimmed();
		if(Order().NeedsExtraDatafield() && itHeader.ordnum > 256)
		{
			// If there are more order items, write them elsewhere.
			itHeader.ordnum = 256;
		}
	} else
	{
		// An additional "---" pattern is appended so Impulse Tracker won't ignore the last order item.
		// Interestingly, this can exceed IT's 256 order limit. Also, IT will always save at least two orders.
		itHeader.ordnum = std::min(Order().GetLengthTailTrimmed(), specs.ordersMax) + 1;
		if(itHeader.ordnum < 2)
			itHeader.ordnum = 2;
	}

	itHeader.insnum = std::min(m_nInstruments, specs.instrumentsMax);
	itHeader.smpnum = std::min(m_nSamples, specs.samplesMax);
	itHeader.patnum = std::min(Patterns.GetNumPatterns(), specs.patternsMax);

	// Parapointers
	std::vector<uint32le> patpos(itHeader.patnum);
	std::vector<uint32le> smppos(itHeader.smpnum);
	std::vector<uint32le> inspos(itHeader.insnum);

	//VERSION
	if(GetType() == MOD_TYPE_MPT)
	{
		// MPTM
		itHeader.cwtv = verMptFileVer;	// Used in OMPT-hack versioning.
		itHeader.cmwt = 0x888;
	} else
	{
		// IT
		const uint32 mptVersion = Version::Current().GetRawVersion();
		itHeader.cwtv = 0x5000 | static_cast<uint16>((mptVersion >> 16) & 0x0FFF); // format: txyy (t = tracker ID, x = version major, yy = version minor), e.g. 0x5117 (OpenMPT = 5, 117 = v1.17)
		itHeader.cmwt = 0x0214;	// Common compatible tracker :)
		// Hack from schism tracker:
		for(INSTRUMENTINDEX nIns = 1; nIns <= GetNumInstruments(); nIns++)
		{
			if(Instruments[nIns] && Instruments[nIns]->PitchEnv.dwFlags[ENV_FILTER])
			{
				itHeader.cmwt = 0x0216;
				break;
			}
		}

		if(compatibilityExport)
			itHeader.reserved = mptVersion & 0xFFFF;
		else
			memcpy(&itHeader.reserved, "OMPT", 4);
	}

	itHeader.flags = ITFileHeader::useStereoPlayback | ITFileHeader::useMIDIPitchController;
	itHeader.special = ITFileHeader::embedEditHistory | ITFileHeader::embedPatternHighlights;
	if(m_nInstruments) itHeader.flags |= ITFileHeader::instrumentMode;
	if(m_SongFlags[SONG_LINEARSLIDES]) itHeader.flags |= ITFileHeader::linearSlides;
	if(m_SongFlags[SONG_ITOLDEFFECTS]) itHeader.flags |= ITFileHeader::itOldEffects;
	if(m_SongFlags[SONG_ITCOMPATGXX]) itHeader.flags |= ITFileHeader::itCompatGxx;
	if(m_SongFlags[SONG_EXFILTERRANGE] && !compatibilityExport) itHeader.flags |= ITFileHeader::extendedFilterRange;

	itHeader.globalvol = static_cast<uint8>(m_nDefaultGlobalVolume / 2u);
	itHeader.mv = static_cast<uint8>(std::min(m_nSamplePreAmp, uint32(128)));
	itHeader.speed = mpt::saturate_cast<uint8>(Order().GetDefaultSpeed());
 	itHeader.tempo = mpt::saturate_cast<uint8>(Order().GetDefaultTempo().GetInt()); // We save the real tempo in an extension below if it exceeds 255.
	itHeader.sep = 128; // pan separation
	// IT doesn't have a per-instrument Pitch Wheel Depth setting, so we just store the first non-zero PWD setting in the header.
	for(INSTRUMENTINDEX ins = 1; ins <= GetNumInstruments(); ins++)
	{
		if(Instruments[ins] != nullptr && Instruments[ins]->midiPWD != 0)
		{
			itHeader.pwd = static_cast<uint8>(std::abs(Instruments[ins]->midiPWD));
			break;
		}
	}

	dwHdrPos = sizeof(itHeader) + itHeader.ordnum;
	// Channel Pan and Volume
	memset(itHeader.chnpan, 0xA0, 64);
	memset(itHeader.chnvol, 64, 64);

	for(CHANNELINDEX ich = 0; ich < std::min(GetNumChannels(), CHANNELINDEX(64)); ich++) // Header only has room for settings for 64 chans...
	{
		itHeader.chnpan[ich] = (uint8)(ChnSettings[ich].nPan >> 2);
		if (ChnSettings[ich].dwFlags[CHN_SURROUND]) itHeader.chnpan[ich] = 100;
		itHeader.chnvol[ich] = (uint8)(ChnSettings[ich].nVolume);
#ifdef MODPLUG_TRACKER
		if(TrackerSettings::Instance().MiscSaveChannelMuteStatus)
#endif
		if (ChnSettings[ich].dwFlags[CHN_MUTE]) itHeader.chnpan[ich] |= 0x80;
	}

	// Channel names
	if(!compatibilityExport)
	{
		for(CHANNELINDEX i = 0; i < GetNumChannels(); i++)
		{
			if(ChnSettings[i].szName[0])
			{
				dwChnNamLen = (i + 1) * MAX_CHANNELNAME;
			}
		}
		if(dwChnNamLen) dwExtra += dwChnNamLen + 8;
	}

	if(!m_MidiCfg.IsMacroDefaultSetupUsed())
	{
		itHeader.flags |= ITFileHeader::reqEmbeddedMIDIConfig;
		itHeader.special |= ITFileHeader::embedMIDIConfiguration;
		dwExtra += sizeof(MIDIMacroConfigData);
	}

	// Pattern Names
	const PATTERNINDEX numNamedPats = compatibilityExport ? 0 : Patterns.GetNumNamedPatterns();
	if(numNamedPats > 0)
	{
		dwExtra += (numNamedPats * MAX_PATTERNNAME) + 8;
	}

	// Mix Plugins. Just calculate the size of this extra block for now.
	if(!compatibilityExport)
	{
		dwExtra += SaveMixPlugins(nullptr, true);
	}

	// Edit History. Just calculate the size of this extra block for now.
	dwExtra += SaveITEditHistory(*this, nullptr);

	// Comments
	uint16 msglength = 0;
	if(!m_songMessage.empty())
	{
		itHeader.special |= ITFileHeader::embedSongMessage;
		itHeader.msglength = msglength = mpt::saturate_cast<uint16>(m_songMessage.length() + 1u);
		itHeader.msgoffset = dwHdrPos + dwExtra + (itHeader.insnum + itHeader.smpnum + itHeader.patnum) * 4;
	}

	// Write file header
	mpt::IO::Write(f, itHeader);

	Order().WriteAsByte(f, itHeader.ordnum);
	mpt::IO::Write(f, inspos);
	mpt::IO::Write(f, smppos);
	mpt::IO::Write(f, patpos);

	// Writing edit history information
	SaveITEditHistory(*this, &f);

	// Writing midi cfg
	if(itHeader.flags & ITFileHeader::reqEmbeddedMIDIConfig)
	{
		mpt::IO::Write(f, static_cast<MIDIMacroConfigData &>(m_MidiCfg));
	}

	// Writing pattern names
	if(numNamedPats)
	{
		mpt::IO::WriteRaw(f, "PNAM", 4);
		mpt::IO::WriteIntLE<uint32>(f, numNamedPats * MAX_PATTERNNAME);

		for(PATTERNINDEX pat = 0; pat < numNamedPats; pat++)
		{
			char name[MAX_PATTERNNAME];
			mpt::String::WriteBuf(mpt::String::maybeNullTerminated, name) = Patterns[pat].GetName();
			mpt::IO::Write(f, name);
		}
	}

	// Writing channel names
	if(dwChnNamLen && !compatibilityExport)
	{
		mpt::IO::WriteRaw(f, "CNAM", 4);
		mpt::IO::WriteIntLE<uint32>(f, dwChnNamLen);
		uint32 nChnNames = dwChnNamLen / MAX_CHANNELNAME;
		for(uint32 inam = 0; inam < nChnNames; inam++)
		{
			char name[MAX_CHANNELNAME];
			mpt::String::WriteBuf(mpt::String::maybeNullTerminated, name) = ChnSettings[inam].szName;
			mpt::IO::Write(f, name);
		}
	}

	// Writing mix plugins info
	if(!compatibilityExport)
	{
		SaveMixPlugins(&f, false);
	}

	// Writing song message
	dwPos = dwHdrPos + dwExtra + (itHeader.insnum + itHeader.smpnum + itHeader.patnum) * 4;
	if(itHeader.special & ITFileHeader::embedSongMessage)
	{
		dwPos += msglength;
		mpt::IO::WriteRaw(f, m_songMessage.c_str(), msglength);
	}

	// Writing instruments
	const ModInstrument dummyInstr;
	for(INSTRUMENTINDEX nins = 1; nins <= itHeader.insnum; nins++)
	{
		ITInstrumentEx iti;
		uint32 instSize;

		const ModInstrument &instr = (Instruments[nins] != nullptr) ? *Instruments[nins] : dummyInstr;
		instSize = iti.ConvertToIT(instr, compatibilityExport, *this);

		// Writing instrument
		inspos[nins - 1] = static_cast<uint32>(dwPos);
		dwPos += instSize;
		mpt::IO::WritePartial(f, iti, instSize);
	}

	// Writing dummy sample headers (until we know the correct sample data offset)
	ITSample itss;
	MemsetZero(itss);
	for(SAMPLEINDEX smp = 0; smp < itHeader.smpnum; smp++)
	{
		smppos[smp] = static_cast<uint32>(dwPos);
		dwPos += sizeof(ITSample);
		mpt::IO::Write(f, itss);
	}

	// Writing Patterns
	bool needsMptPatSave = false;
	for(PATTERNINDEX pat = 0; pat < itHeader.patnum; pat++)
	{
		uint32 dwPatPos = static_cast<uint32>(dwPos);
		if (!Patterns.IsValidPat(pat)) continue;

		if(Patterns[pat].GetOverrideSignature())
			needsMptPatSave = true;

		// Check for empty pattern
		if(Patterns[pat].GetNumRows() == 64 && Patterns.IsPatternEmpty(pat))
		{
			patpos[pat] = 0;
			continue;
		}

		patpos[pat] = static_cast<uint32>(dwPos);

		// Write pattern header
		ROWINDEX writeRows = mpt::saturate_cast<uint16>(Patterns[pat].GetNumRows());
		if(compatibilityExport)
			writeRows = std::clamp(writeRows, ROWINDEX(32), ROWINDEX(200));
		if(writeRows != Patterns[pat].GetNumRows())
			AddToLog(LogWarning, MPT_UFORMAT("Warning: Pattern {} was resized from {} to {} rows.")(pat, Patterns[pat].GetNumRows(), writeRows));
		uint16 writeSize = 0;
		uint16le patinfo[4];
		patinfo[0] = 0;
		patinfo[1] = static_cast<uint16>(writeRows);
		patinfo[2] = 0;
		patinfo[3] = 0;

		mpt::IO::Write(f, patinfo);
		dwPos += 8;

		struct ChnState { uint8 note = 0, instr = 0, vol = 0, command = 0, param = 0, valid = 0, mask = 0xFF; };
		const CHANNELINDEX maxChannels = std::min(specs.channelsMax, GetNumChannels());
		std::vector<ChnState> chnStates(maxChannels);
		// Maximum 7 bytes per cell, plus end of row marker, so this buffer is always large enough to cover one row.
		std::vector<uint8> buf(7 * maxChannels + 1);

		const ROWINDEX readRows = std::min(writeRows, Patterns[pat].GetNumRows());
		for(ROWINDEX row = 0; row < readRows; row++)
		{
			uint32 len = 0;
			const ModCommand *m = Patterns[pat].GetpModCommand(row, 0);
			bool writePatternBreak = (readRows < writeRows && row + 1 == readRows && !Patterns[pat].RowHasJump(row));
			for(CHANNELINDEX ch = 0; ch < maxChannels; ch++, m++)
			{
				// Skip mptm-specific notes.
				if(m->IsPcNote())
				{
					needsMptPatSave = true;
					continue;
				}

				auto &chnState = chnStates[ch];
				uint8 b = 1;
				uint8 vol = 0xFF;
				uint8 note = m->note;
				if(note >= NOTE_MIN && note <= NOTE_MIN + 119)
					note = m->note - NOTE_MIN;
				else if(note == NOTE_FADE)
					note = (GetType() == MOD_TYPE_MPT) ? 0xFD : 0xF6;
				else if(note == NOTE_NOTECUT)
					note = 0xFE;
				else if(note == NOTE_KEYOFF)
					note = 0xFF;
				else
					b = 0;
				
				if(m->instr)
					b |= 2;
				if(m->volcmd != VOLCMD_NONE)
				{
					vol = std::min(m->vol, uint8(9));
					switch(m->volcmd)
					{
					case VOLCMD_VOLUME:         vol = std::min(m->vol, uint8(64)); break;
					case VOLCMD_PANNING:        vol = std::min(m->vol, uint8(64)) + 128; break;
					case VOLCMD_VOLSLIDEUP:     vol += 85; break;
					case VOLCMD_VOLSLIDEDOWN:   vol += 95; break;
					case VOLCMD_FINEVOLUP:      vol += 65; break;
					case VOLCMD_FINEVOLDOWN:    vol += 75; break;
					case VOLCMD_VIBRATODEPTH:   vol += 203; break;
					case VOLCMD_TONEPORTAMENTO: vol += 193; break;
					case VOLCMD_PORTADOWN:      vol += 105; break;
					case VOLCMD_PORTAUP:        vol += 115; break;
					case VOLCMD_VIBRATOSPEED:   vol = 203; break;
					case VOLCMD_OFFSET:
						if(!compatibilityExport)
							vol += 223;
						else
							vol = 0xFF;
						break;
					default: vol = 0xFF;
					}
				}
				if(vol != 0xFF)
					b |= 4;
				uint8 command = 0, param = 0;
				if(m->command == CMD_VOLUME && vol == 0xFF)
				{
					vol = std::min(m->param, ModCommand::PARAM(64));
					b |= 4;
				} else if(m->command != CMD_NONE)
				{
					S3MSaveConvert(*m, command, param, true, compatibilityExport);
					if (command) b |= 8;
				}
				if(writePatternBreak && !(b & 8))
				{
					b |= 8;
					command = 'C' ^ 0x40;
					writePatternBreak = false;
				}
				// Packing information
				if (b)
				{
					// Same note ?
					if (b & 1)
					{
						if ((note == chnState.note) && (chnState.valid & 1))
						{
							b &= ~1;
							b |= 0x10;
						} else
						{
							chnState.note = note;
							chnState.valid |= 1;
						}
					}
					// Same instrument ?
					if (b & 2)
					{
						if ((m->instr == chnState.instr) && (chnState.valid & 2))
						{
							b &= ~2;
							b |= 0x20;
						} else
						{
							chnState.instr = m->instr;
							chnState.valid |= 2;
						}
					}
					// Same volume column byte ?
					if (b & 4)
					{
						if ((vol == chnState.vol) && (chnState.valid & 4))
						{
							b &= ~4;
							b |= 0x40;
						} else
						{
							chnState.vol = vol;
							chnState.valid |= 4;
						}
					}
					// Same command / param ?
					if (b & 8)
					{
						if ((command == chnState.command) && (param == chnState.param) && (chnState.valid & 8))
						{
							b &= ~8;
							b |= 0x80;
						} else
						{
							chnState.command = command;
							chnState.param = param;
							chnState.valid |= 8;
						}
					}
					if (b != chnState.mask)
					{
						chnState.mask = b;
						buf[len++] = static_cast<uint8>((ch + 1) | IT_bitmask_patternChanEnabled_c);
						buf[len++] = b;
					} else
					{
						buf[len++] = static_cast<uint8>(ch + 1);
					}
					if (b & 1) buf[len++] = note;
					if (b & 2) buf[len++] = m->instr;
					if (b & 4) buf[len++] = vol;
					if (b & 8)
					{
						buf[len++] = command;
						buf[len++] = param;
					}
				}
			}
			buf[len++] = 0;
			if(writeSize > uint16_max - len)
			{
				AddToLog(LogWarning, MPT_UFORMAT("Warning: File format limit was reached. Some pattern data may not get written to file. (pattern {})")(pat));
				break;
			} else
			{
				writeSize += static_cast<uint16>(len);
				mpt::IO::WriteRaw(f, buf.data(), len);
			}
			if(writePatternBreak)
			{
				// Didn't manage to put a pattern break, so put it on the next row instead.
				const uint8 patternBreak[] = {1 | IT_bitmask_patternChanEnabled_c, 8, 'C' ^ 0x40, 0};
				mpt::IO::Write(f, patternBreak);
				writeSize += sizeof(patternBreak);
			}
		}
		if(readRows < writeRows)
		{
			// Invent empty rows at end (if we end up here, the pattern is very short and we don't have to care about writeSize overflowing the 16-bit limit)
			writeSize += static_cast<uint16>(writeRows - readRows);
			buf.assign(writeRows - readRows, 0);
			mpt::IO::Write(f, buf);
		}
		dwPos += writeSize;

		mpt::IO::SeekAbsolute(f, dwPatPos);
		patinfo[0] = writeSize;
		mpt::IO::Write(f, patinfo);
		mpt::IO::SeekAbsolute(f, dwPos);
	}
	// Writing Sample Data
	for(SAMPLEINDEX smp = 1; smp <= itHeader.smpnum; smp++)
	{
		const ModSample &sample = Samples[smp];
#ifdef MODPLUG_TRACKER
		uint32 type = GetType() == MOD_TYPE_IT ? 1 : 4;
		if(compatibilityExport) type = 2;
		bool compress = ((((sample.GetNumChannels() > 1) ? TrackerSettings::Instance().MiscITCompressionStereo : TrackerSettings::Instance().MiscITCompressionMono) & type) != 0);
#else
		bool compress = false;
#endif // MODPLUG_TRACKER
		// Old MPT, DUMB and probably other libraries will only consider the IT2.15 compression flag if the header version also indicates IT2.15.
		// MilkyTracker <= 0.90.85 assumes IT2.15 compression with cmwt == 0x215, ignoring the delta flag completely.
		itss.ConvertToIT(sample, GetType(), compress, itHeader.cmwt >= 0x215, GetType() == MOD_TYPE_MPT);
		const bool isExternal = itss.cvt == ITSample::cvtExternalSample;

		mpt::String::WriteBuf(mpt::String::nullTerminated, itss.name) = m_szNames[smp];

		itss.samplepointer = static_cast<uint32>(dwPos);
		if(dwPos > uint32_max)
		{
			// Sample position does not fit into sample pointer!
			AddToLog(LogWarning, MPT_UFORMAT("Cannot save sample {}: File size exceeds 4 GB.")(smp));
			itss.samplepointer = 0;
			itss.length = 0;
		}
		SmpLength smpLength = itss.length;	// Possibly truncated to 2^32 samples
		mpt::IO::SeekAbsolute(f, smppos[smp - 1]);
		mpt::IO::Write(f, itss);
		if(dwPos > uint32_max)
		{
			continue;
		}
		// TODO this actually wraps around at 2 GB, so we either need to use the 64-bit seek API or warn earlier!
		mpt::IO::SeekAbsolute(f, dwPos);
		if(!isExternal)
		{
			if(sample.nLength > smpLength && smpLength != 0)
			{
				// Sample length does not fit into IT header!
				AddToLog(LogWarning, MPT_UFORMAT("Truncating sample {}: Length exceeds exceeds 4 gigasamples.")(smp));
			}
			dwPos += itss.GetSampleFormat().WriteSample(f, sample, smpLength);
		} else
		{
#ifdef MPT_EXTERNAL_SAMPLES
			const std::string filenameU8 = mpt::AbsolutePathToRelative(GetSamplePath(smp), filename.GetDirectoryWithDrive()).ToUTF8();
			const size_t strSize = filenameU8.size();
			size_t intBytes = 0;
			if(mpt::IO::WriteVarInt(f, strSize, &intBytes))
			{
				dwPos += intBytes + strSize;
				mpt::IO::WriteRaw(f, filenameU8.data(), strSize);
			}
#else
			MPT_UNREFERENCED_PARAMETER(filename);
#endif // MPT_EXTERNAL_SAMPLES
		}
	}

	//Save hacked-on extra info
	if(!compatibilityExport)
	{
		if(GetNumInstruments())
		{
			SaveExtendedInstrumentProperties(0, GetType(), f);
		}
		SaveExtendedSongProperties(f);
	}

	// Updating offsets
	mpt::IO::SeekAbsolute(f, dwHdrPos);
	mpt::IO::Write(f, inspos);
	mpt::IO::Write(f, smppos);
	mpt::IO::Write(f, patpos);

	if(GetType() == MOD_TYPE_IT)
	{
		return true;
	}

	//hack
	//BEGIN: MPT SPECIFIC:

	bool success = true;

	mpt::IO::SeekEnd(f);

	const mpt::IO::Offset MPTStartPos = mpt::IO::TellWrite(f);

	srlztn::SsbWrite ssb(f);
	ssb.BeginWrite("mptm", Version::Current().GetRawVersion());

	if(GetTuneSpecificTunings().GetNumTunings() > 0 || AreNonDefaultTuningsUsed(*this))
		ssb.WriteItem(int8(1), "UTF8Tuning");
	if(GetTuneSpecificTunings().GetNumTunings() > 0)
		ssb.WriteItem(GetTuneSpecificTunings(), "0", &WriteTuningCollection);
	if(AreNonDefaultTuningsUsed(*this))
		ssb.WriteItem(*this, "1", &WriteTuningMap);
	if(Order().NeedsExtraDatafield())
		ssb.WriteItem(Order, "2", &WriteModSequenceOld);
	if(needsMptPatSave)
		ssb.WriteItem(Patterns, FileIdPatterns, &WriteModPatterns);
	ssb.WriteItem(Order, FileIdSequences, &WriteModSequences);

	ssb.FinishWrite();

	if(ssb.HasFailed())
	{
		AddToLog(LogError, U_("Error occurred in writing MPTM extensions."));
	}

	//Last 4 bytes should tell where the hack mpt things begin.
	if(!f.good())
	{
		f.clear();
		success = false;
	}
	mpt::IO::WriteIntLE<uint32>(f, static_cast<uint32>(MPTStartPos));

	mpt::IO::SeekEnd(f);

	//END  : MPT SPECIFIC

	//NO WRITING HERE ANYMORE.

	return success;
}


#endif // MODPLUG_NO_FILESAVE


#ifndef MODPLUG_NO_FILESAVE

uint32 CSoundFile::SaveMixPlugins(std::ostream *file, bool updatePlugData)
{
#ifndef NO_PLUGINS
	uint32 totalSize = 0;

	for(PLUGINDEX i = 0; i < MAX_MIXPLUGINS; i++)
	{
		const SNDMIXPLUGIN &plugin = m_MixPlugins[i];
		if(plugin.IsValidPlugin())
		{
			uint32 chunkSize = sizeof(SNDMIXPLUGININFO) + 4; // plugininfo+4 (datalen)
			if(plugin.pMixPlugin && updatePlugData)
			{
				plugin.pMixPlugin->SaveAllParameters();
			}

			const uint32 extraDataSize =
				4 + sizeof(IEEE754binary32LE) + // 4 for ID and size of dryRatio
				4 + sizeof(int32);              // Default Program
			// For each extra entity, add 4 for ID, plus 4 for size of entity, plus size of entity

			chunkSize += extraDataSize + 4; // +4 is for size field itself

			const uint32 plugDataSize = std::min(mpt::saturate_cast<uint32>(plugin.pluginData.size()), uint32_max - chunkSize);
			chunkSize += plugDataSize;

			if(file)
			{
				std::ostream &f = *file;
				// Chunk ID (= plugin ID)
				char id[4] = { 'F', 'X', '0', '0' };
				if(i >= 100) id[1] = static_cast<unsigned char>(static_cast<unsigned char>('0') + (i / 100u));
				id[2] = static_cast<unsigned char>(static_cast<unsigned char>('0') + ((i / 10u) % 10u));
				id[3] = static_cast<unsigned char>(static_cast<unsigned char>('0') + (i % 10u));
				mpt::IO::WriteRaw(f, id, 4);

				// Write chunk size, plugin info and plugin data chunk
				mpt::IO::WriteIntLE<uint32>(f, chunkSize);
				mpt::IO::Write(f, m_MixPlugins[i].Info);
				mpt::IO::WriteIntLE<uint32>(f, plugDataSize);
				if(plugDataSize)
				{
					mpt::IO::WriteRaw(f, m_MixPlugins[i].pluginData.data(), plugDataSize);
				}

				mpt::IO::WriteIntLE<uint32>(f, extraDataSize);

				// Dry/Wet ratio
				mpt::IO::WriteRaw(f, "DWRT", 4);
				// DWRT chunk does not include a size, so better make sure we always write 4 bytes here.
				static_assert(sizeof(IEEE754binary32LE) == 4);
				mpt::IO::Write(f, IEEE754binary32LE(m_MixPlugins[i].fDryRatio));

				// Default program
				mpt::IO::WriteRaw(f, "PROG", 4);
				// PROG chunk does not include a size, so better make sure we always write 4 bytes here.
				static_assert(sizeof(m_MixPlugins[i].defaultProgram) == sizeof(int32));
				mpt::IO::WriteIntLE<int32>(f, m_MixPlugins[i].defaultProgram);

				// Please, if you add any more chunks here, don't repeat history (see above) and *do* add a size field for your chunk, mmmkay?
			}
			totalSize += chunkSize + 8;
		}
	}
	std::vector<uint32le> chinfo(GetNumChannels());
	uint32 numChInfo = 0;
	for(CHANNELINDEX j = 0; j < GetNumChannels(); j++)
	{
		if((chinfo[j] = ChnSettings[j].nMixPlugin) != 0)
		{
			numChInfo = j + 1;
		}
	}
	if(numChInfo)
	{
		if(file)
		{
			std::ostream &f = *file;
			mpt::IO::WriteRaw(f, "CHFX", 4);
			mpt::IO::WriteIntLE<uint32>(f, numChInfo * 4);
			chinfo.resize(numChInfo);
			mpt::IO::Write(f, chinfo);
		}
		totalSize += numChInfo * 4 + 8;
	}
	return totalSize;
#else
	MPT_UNREFERENCED_PARAMETER(file);
	MPT_UNREFERENCED_PARAMETER(updatePlugData);
	return 0;
#endif // NO_PLUGINS
}

#endif // MODPLUG_NO_FILESAVE


std::pair<bool, bool> CSoundFile::LoadMixPlugins(FileReader &file, bool ignoreChannelCount)
{
	bool hasPluginChunks = false, isBeRoTracker = false;
	while(file.CanRead(9))
	{
		char code[4];
		file.ReadArray(code);
		const uint32 chunkSize = file.ReadUint32LE();
		if(!memcmp(code, "IMPI", 4)     // IT instrument, we definitely read too far
		   || !memcmp(code, "IMPS", 4)  // IT sample, ditto
		   || !memcmp(code, "XTPM", 4)  // Instrument extensions, ditto
		   || !memcmp(code, "STPM", 4)  // Song extensions, ditto
		   || !file.CanRead(chunkSize))
		{
			file.SkipBack(8);
			return std::make_pair(hasPluginChunks, isBeRoTracker);
		}
		FileReader chunk = file.ReadChunk(chunkSize);

		// Channel FX
		if(!memcmp(code, "CHFX", 4))
		{
			if(!ignoreChannelCount)
			{
				ChnSettings.resize(std::clamp(static_cast<CHANNELINDEX>(chunkSize / 4), GetNumChannels(), MAX_BASECHANNELS));
			}
			for(auto &chn : ChnSettings)
			{
				chn.nMixPlugin = static_cast<PLUGINDEX>(chunk.ReadUint32LE());
			}
			hasPluginChunks = true;
#ifndef NO_PLUGINS
		}
		// Plugin Data FX00, ... FX99, F100, ... F255
#define MPT_ISDIGIT(x) (code[(x)] >= '0' && code[(x)] <= '9')
		else if(code[0] == 'F' && (code[1] == 'X' || MPT_ISDIGIT(1)) && MPT_ISDIGIT(2) && MPT_ISDIGIT(3))
#undef MPT_ISDIGIT
		{
			uint16 fxplug = static_cast<uint16>((code[2] - '0') * 10 + (code[3] - '0'));  //calculate plug-in number.
			if(code[1] != 'X') fxplug += static_cast<uint16>((code[1] - '0') * 100);
			if(fxplug < MAX_MIXPLUGINS)
			{
				PLUGINDEX plug = static_cast<PLUGINDEX>(fxplug);
				ReadMixPluginChunk(chunk, m_MixPlugins[plug]);
			}
			hasPluginChunks = true;
#endif // NO_PLUGINS
		} else if(!memcmp(code, "MODU", 4))
		{
			isBeRoTracker = true;
			m_dwLastSavedWithVersion = Version();	// Reset MPT detection for old files that have a similar fingerprint
		}
	}
	return std::make_pair(hasPluginChunks, isBeRoTracker);
}


#ifndef NO_PLUGINS
void CSoundFile::ReadMixPluginChunk(FileReader &file, SNDMIXPLUGIN &plugin)
{
	// MPT's standard plugin data. Size not specified in file.. grrr..
	file.ReadStruct(plugin.Info);
	mpt::String::SetNullTerminator(plugin.Info.szName.buf);
	mpt::String::SetNullTerminator(plugin.Info.szLibraryName.buf);
	plugin.editorX = plugin.editorY = int32_min;

	// Plugin user data
	FileReader pluginDataChunk = file.ReadChunk(file.ReadUint32LE());
	plugin.pluginData.resize(mpt::saturate_cast<size_t>(pluginDataChunk.BytesLeft()));
	pluginDataChunk.ReadRaw(mpt::as_span(plugin.pluginData));

	if(FileReader modularData = file.ReadChunk(file.ReadUint32LE()); modularData.IsValid())
	{
		while(modularData.CanRead(5))
		{
			// do we recognize this chunk?
			char code[4];
			modularData.ReadArray(code);
			uint32 dataSize = 0;
			if(!memcmp(code, "DWRT", 4) || !memcmp(code, "PROG", 4))
			{
				// Legacy system with fixed size chunks
				dataSize = 4;
			} else
			{
				dataSize = modularData.ReadUint32LE();
			}
			FileReader dataChunk = modularData.ReadChunk(dataSize);

			if(!memcmp(code, "DWRT", 4))
			{
				plugin.fDryRatio = mpt::safe_clamp(dataChunk.ReadFloatLE(), 0.0f, 1.0f);
			} else if(!memcmp(code, "PROG", 4))
			{
				plugin.defaultProgram = dataChunk.ReadUint32LE();
			} else if(!memcmp(code, "MCRO", 4))
			{
				// Read plugin-specific macros
				//dataChunk.ReadStructPartial(plugin.macros, dataChunk.GetLength());
			}
		}
	}
}
#endif // NO_PLUGINS


#ifndef MODPLUG_NO_FILESAVE

void CSoundFile::SaveExtendedSongProperties(std::ostream &f) const
{
	const CModSpecifications &specs = GetModSpecifications();
	// Extra song data - Yet Another Hack.
	mpt::IO::WriteIntLE<uint32>(f, MagicBE("MPTS"));

	const auto WriteModularHeader = [](std::ostream &f, uint32 code, size_t fsize)
	{
		mpt::IO::WriteIntLE<uint32>(f, code);
		MPT_ASSERT(mpt::in_range<uint16>(fsize));
		mpt::IO::WriteIntLE<uint16>(f, static_cast<uint16>(fsize));
	};
	const auto WriteModular = [&WriteModularHeader](std::ostream &f, uint32 code, auto field)
	{
		WriteModularHeader(f, code, sizeof(field));
		mpt::IO::WriteIntLE(f, field);
	};

	if(Order().GetDefaultTempo().GetInt() > 255)
	{
		uint32 tempo = Order().GetDefaultTempo().GetInt();
		WriteModular(f, MagicBE("DT.."), tempo);
	}
	if(Order().GetDefaultTempo().GetFract() != 0 && specs.hasFractionalTempo)
	{
		uint32 tempo = Order().GetDefaultTempo().GetFract();
		WriteModular(f, MagicLE("DTFR"), tempo);
	}

	if(m_nDefaultRowsPerBeat > 255 || m_nDefaultRowsPerMeasure > 255 || GetType() == MOD_TYPE_XM)
	{
		WriteModular(f, MagicBE("RPB."), m_nDefaultRowsPerBeat);
		WriteModular(f, MagicBE("RPM."), m_nDefaultRowsPerMeasure);
	}

	if(GetType() != MOD_TYPE_XM)
	{
		WriteModular(f, MagicBE("C..."), GetNumChannels());
	}

	if((GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT)) && GetNumChannels() > 64)
	{
		// IT header has only room for 64 channels. Save the settings that do not fit to the header here as an extension.
		WriteModularHeader(f, MagicBE("ChnS"), (GetNumChannels() - 64) * 2);
		for(CHANNELINDEX chn = 64; chn < GetNumChannels(); chn++)
		{
			uint8 panvol[2];
			panvol[0] = (uint8)(ChnSettings[chn].nPan >> 2);
			if (ChnSettings[chn].dwFlags[CHN_SURROUND]) panvol[0] = 100;
			if (ChnSettings[chn].dwFlags[CHN_MUTE]) panvol[0] |= 0x80;
			panvol[1] = (uint8)ChnSettings[chn].nVolume;
			mpt::IO::Write(f, panvol);
		}
	}

	if(m_nTempoMode != TempoMode::Classic)
	{
		WriteModularHeader(f, MagicBE("TM.."), 1);
		uint8 mode = static_cast<uint8>(m_nTempoMode);
		mpt::IO::WriteIntLE(f, mode);
	}

	const int32 tmpMixLevels = static_cast<int32>(m_nMixLevels);
	WriteModular(f, MagicBE("PMM."), tmpMixLevels);

	if(m_dwCreatedWithVersion)
	{
		WriteModular(f, MagicBE("CWV."), m_dwCreatedWithVersion.GetRawVersion());
	}

	WriteModular(f, MagicBE("LSWV"), Version::Current().GetRawVersion());
	if(GetType() == MOD_TYPE_XM || m_nSamplePreAmp > 128)
	{
		WriteModular(f, MagicBE("SPA."), m_nSamplePreAmp);
	}
	WriteModular(f, MagicBE("VSTV"), m_nVSTiVolume);

	if(GetType() == MOD_TYPE_XM && m_nDefaultGlobalVolume != MAX_GLOBAL_VOLUME)
	{
		WriteModular(f, MagicBE("DGV."), m_nDefaultGlobalVolume);
	}

	if(GetType() != MOD_TYPE_XM && Order().GetRestartPos() != 0)
	{
		WriteModular(f, MagicBE("RP.."), Order().GetRestartPos());
	}

	if(m_nResampling != SRCMODE_DEFAULT && specs.hasDefaultResampling)
	{
		WriteModular(f, MagicLE("RSMP"), static_cast<uint32>(m_nResampling));
	}

	// Sample cues
	if(GetType() == MOD_TYPE_MPT)
	{
		for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
		{
			const ModSample &sample = Samples[smp];
			if(sample.nLength && sample.HasCustomCuePoints())
			{
				// Write one chunk for every sample.
				// Rationale: chunks are limited to 65536 bytes, which can easily be reached
				// with the amount of samples that OpenMPT supports.
				WriteModularHeader(f, MagicLE("CUES"), 2 + std::size(sample.cues) * 4);
				mpt::IO::WriteIntLE<uint16>(f, smp);
				for(auto cue : sample.cues)
				{
					mpt::IO::WriteIntLE<uint32>(f, cue);
				}
			}
		}
	}

	// Tempo Swing Factors
	if(!m_tempoSwing.empty())
	{
		std::ostringstream oStrm;
		TempoSwing::Serialize(oStrm, m_tempoSwing);
		std::string data = oStrm.str();
		uint16 length = mpt::saturate_cast<uint16>(data.size());
		WriteModularHeader(f, MagicLE("SWNG"), length);
		mpt::IO::WriteRaw(f, data.data(), length);
	}

	// Playback compatibility flags
	{
		const auto supportedBehaviours = GetSupportedPlaybackBehaviour(GetBestSaveFormat());
		std::array<uint8, (kMaxPlayBehaviours + 7) / 8u> bits;
		bits.fill(0);
		size_t maxBit = 0;
		for(size_t i = 0; i < kMaxPlayBehaviours; i++)
		{
			if(m_playBehaviour[i] && supportedBehaviours[i])
			{
				bits[i >> 3] |= 1 << (i & 0x07);
				maxBit = i + 8;
			}
		}
		uint16 numBytes = static_cast<uint16>(maxBit / 8u);
		WriteModularHeader(f, MagicBE("MSF."), numBytes);
		mpt::IO::WriteRaw(f, bits.data(), numBytes);
	}

	if(!m_songArtist.empty() && specs.hasArtistName)
	{
		std::string songArtistU8 = mpt::ToCharset(mpt::Charset::UTF8, m_songArtist);
		uint16 length = mpt::saturate_cast<uint16>(songArtistU8.length());
		WriteModularHeader(f, MagicLE("AUTH"), length);
		mpt::IO::WriteRaw(f, songArtistU8.c_str(), length);
	}

#ifdef MODPLUG_TRACKER
	// MIDI mapping directives
	if(GetMIDIMapper().GetCount() > 0)
	{
		const size_t objectsize = GetMIDIMapper().Serialize();
		if(!mpt::in_range<uint16>(objectsize))
		{
			AddToLog(LogWarning, U_("Too many MIDI Mapping directives to save; data won't be written."));
		} else
		{
			WriteModularHeader(f, MagicBE("MIMA"), objectsize);
			GetMIDIMapper().Serialize(&f);
		}
	}

	// Channel colors
	{
		CHANNELINDEX numChannels = 0;
		for(CHANNELINDEX i = 0; i < GetNumChannels(); i++)
		{
			if(ChnSettings[i].color != ModChannelSettings::INVALID_COLOR)
			{
				numChannels = i + 1;
			}
		}
		if(numChannels > 0)
		{
			WriteModularHeader(f, MagicLE("CCOL"), numChannels * 4);
			for(CHANNELINDEX i = 0; i < numChannels; i++)
			{
				uint32 color = ChnSettings[i].color;
				if(color != ModChannelSettings::INVALID_COLOR)
					color &= 0x00FFFFFF;
				std::array<uint8, 4> rgb{static_cast<uint8>(color), static_cast<uint8>(color >> 8), static_cast<uint8>(color >> 16), static_cast<uint8>(color >> 24)};
				mpt::IO::Write(f, rgb);
			}
		}
	}
#endif
}

#endif // MODPLUG_NO_FILESAVE


/*
The following song properties can be read and written:
AUTH [all] Song artist
C... [IT / MPTM] Number of channels (for IT / MPTM where there is no explicit channel count and we want to keep the properties of channels beyond the last channel that contains any pattern data)
CCOL [all] Channel colors
ChnS [IT / MPTM] Channel settings for channels 65-127 if needed (doesn't fit to IT header).
CUES [MPTM] Sample cue points
CWV. [all] Created With Version
DGV. [XM] Default Global Volume
DT.. [all] Default Tempo, if it doesn't fit in the header value
DTFR [MPTM] Fractional part of default tempo
LSWV [all] Last Saved With Version
MIMA [all] MIdi MApping directives
MSF. [all] Mod(Specific)Flags
PMM. [all] Mix Mode
RP.. [IT / MPTM] Legacy Restart Position
RPB. [all] Rows Per Beat (if not supported / value doesn't fit in header)
RPM. [all] Per Measure (if not supported / value doesn't fit in header)
RSMP [MPTM] Default resampling
SPA. [all] Sample Pre-Amp (if not supported / value doesn't fit in header)
SWNG [MPTM] Tempo swing factors
TM.. [all] Tempo Mode
VSTV [all] Synth volume
*/

template<typename T>
void ReadField(FileReader &chunk, std::size_t size, T &field)
{
	field = chunk.ReadSizedIntLE<T>(size);
}


template<typename T>
void ReadFieldCast(FileReader &chunk, std::size_t size, T &field)
{
	static_assert(sizeof(T) <= sizeof(int32));
	field = static_cast<T>(chunk.ReadSizedIntLE<int32>(size));
}


bool CSoundFile::LoadExtendedSongProperties(FileReader &file, bool ignoreChannelCount, bool *pInterpretMptMade)
{
	if(!file.ReadMagic("STPM"))	// 'MPTS'
	{
		return false;
	}

	// Found MPTS, interpret the file MPT made.
	if(pInterpretMptMade != nullptr)
		*pInterpretMptMade = true;

	// HACK: Reset mod flags to default values here, as they are not always written.
	m_playBehaviour.reset();

	while(file.CanRead(7))
	{
		const uint32 code = file.ReadUint32LE();
		const uint16 size = file.ReadUint16LE();

		// Start of MPTM extensions, non-ASCII ID or truncated field
		if(code == MagicLE("228\x04"))
		{
			file.SkipBack(6);
			break;
		} else if((code & 0x80808080) || !(code & 0x60606060) || !file.CanRead(size))
		{
			break;
		}

		FileReader chunk = file.ReadChunk(size);

		switch (code)					// interpret field code
		{
			case MagicBE("DT.."): { uint32 tempo; ReadField(chunk, size, tempo); Order().SetDefaultTempo(TEMPO(tempo, Order().GetDefaultTempo().GetFract())); break; }
			case MagicLE("DTFR"): { uint32 tempoFract; ReadField(chunk, size, tempoFract); Order().SetDefaultTempo(TEMPO(Order().GetDefaultTempo().GetInt(), tempoFract)); break; }
			case MagicBE("RPB."): ReadField(chunk, size, m_nDefaultRowsPerBeat); break;
			case MagicBE("RPM."): ReadField(chunk, size, m_nDefaultRowsPerMeasure); break;
				// FIXME: If there are only PC events on the last few channels in an MPTM MO3, they won't be imported!
			case MagicBE("C..."): if(!ignoreChannelCount) { CHANNELINDEX chn = 0; ReadField(chunk, size, chn); ChnSettings.resize(Clamp(chn, GetNumChannels(), MAX_BASECHANNELS)); } break;
			case MagicBE("TM.."): ReadFieldCast(chunk, size, m_nTempoMode); break;
			case MagicBE("PMM."): ReadFieldCast(chunk, size, m_nMixLevels); break;
			case MagicBE("CWV."): { uint32 ver = 0; ReadField(chunk, size, ver); m_dwCreatedWithVersion = Version(ver); break; }
			case MagicBE("LSWV"): { uint32 ver = 0; ReadField(chunk, size, ver); if(ver != 0) { m_dwLastSavedWithVersion = Version(ver); } break; }
			case MagicBE("SPA."): ReadField(chunk, size, m_nSamplePreAmp); break;
			case MagicBE("VSTV"): ReadField(chunk, size, m_nVSTiVolume); break;
			case MagicBE("DGV."): ReadField(chunk, size, m_nDefaultGlobalVolume); break;
			case MagicBE("RP.."): if(GetType() != MOD_TYPE_XM) { ORDERINDEX restartPos; ReadField(chunk, size, restartPos); Order().SetRestartPos(restartPos); } break;
			case MagicLE("RSMP"):
				ReadFieldCast(chunk, size, m_nResampling);
				if(!Resampling::IsKnownMode(m_nResampling)) m_nResampling = SRCMODE_DEFAULT;
				break;
#ifdef MODPLUG_TRACKER
			case MagicBE("MIMA"): GetMIDIMapper().Deserialize(chunk); break;

			case MagicLE("CCOL"):
				// Channel colors
				{
					const CHANNELINDEX channelsInFile = static_cast<CHANNELINDEX>(size / 4u);
					if(!ignoreChannelCount)
						ChnSettings.resize(std::clamp(GetNumChannels(), channelsInFile, MAX_BASECHANNELS));
					const CHANNELINDEX numChannels = std::min(channelsInFile, GetNumChannels());
					for(CHANNELINDEX i = 0; i < numChannels; i++)
					{
						auto rgb = chunk.ReadArray<uint8, 4>();
						if(rgb[3])
							ChnSettings[i].color = ModChannelSettings::INVALID_COLOR;
						else
							ChnSettings[i].color = rgb[0] | (rgb[1] << 8) | (rgb[2] << 16);
					}
				}
				break;
#endif
			case MagicLE("AUTH"):
				{
					std::string artist;
					chunk.ReadString<mpt::String::spacePadded>(artist, chunk.GetLength());
					m_songArtist = mpt::ToUnicode(mpt::Charset::UTF8, artist);
				}
				break;
			case MagicBE("ChnS"):
				// Channel settings for channels 65+
				static_assert(MAX_BASECHANNELS >= 64);
				if(size <= (MAX_BASECHANNELS - 64) * 2 && (size % 2u) == 0 && (GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT)))
				{
					const CHANNELINDEX channelsInFile = mpt::saturate_cast<CHANNELINDEX>(64 + size / 2);
					if(!ignoreChannelCount)
						ChnSettings.resize(std::clamp(GetNumChannels(), channelsInFile, MAX_BASECHANNELS));
					const CHANNELINDEX numChannels = std::min(channelsInFile, GetNumChannels());
					for(CHANNELINDEX chn = 64; chn < numChannels; chn++)
					{
						auto [pan, vol] = chunk.ReadArray<uint8, 2>();
						if(pan != 0xFF)
						{
							ChnSettings[chn].nVolume = vol;
							ChnSettings[chn].nPan = 128;
							ChnSettings[chn].dwFlags.reset();
							if(pan & 0x80) ChnSettings[chn].dwFlags.set(CHN_MUTE);
							pan &= 0x7F;
							if(pan <= 64) ChnSettings[chn].nPan = pan << 2;
							if(pan == 100) ChnSettings[chn].dwFlags.set(CHN_SURROUND);
						}
					}
				}
				break;

			case MagicLE("CUES"):
				// Sample cues
				if(size > 2)
				{
					SAMPLEINDEX smp = chunk.ReadUint16LE();
					if(smp > 0 && smp <= GetNumSamples())
					{
						ModSample &sample = Samples[smp];
						for(auto &cue : sample.cues)
						{
							if(chunk.CanRead(4))
								cue = chunk.ReadUint32LE();
							else
								cue = MAX_SAMPLE_LENGTH;
						}
					}
				}
				break;

			case MagicLE("SWNG"):
				// Tempo Swing Factors
				if(size > 2)
				{
					std::istringstream iStrm(mpt::buffer_cast<std::string>(chunk.ReadRawDataAsByteVector()));
					TempoSwing::Deserialize(iStrm, m_tempoSwing, chunk.GetLength());
				}
				break;

			case MagicBE("MSF."):
				// Playback compatibility flags
				{
					size_t bit = 0;
					m_playBehaviour.reset();
					while(chunk.CanRead(1) && bit < m_playBehaviour.size())
					{
						uint8 b = chunk.ReadUint8();
						for(uint8 i = 0; i < 8; i++, bit++)
						{
							if((b & (1 << i)) && bit < m_playBehaviour.size())
							{
								m_playBehaviour.set(bit);
							}
						}
					}
				}
				break;
		}
	}

	// Validate read values.
	Order().SetDefaultTempo(Clamp(Order().GetDefaultTempo(), GetModSpecifications().GetTempoMin(), GetModSpecifications().GetTempoMax()));
	if(m_nTempoMode >= TempoMode::NumModes)
		m_nTempoMode = TempoMode::Classic;
	if(m_nMixLevels >= MixLevels::NumMixLevels)
		m_nMixLevels = MixLevels::Original;
	//m_dwCreatedWithVersion
	//m_dwLastSavedWithVersion

	return true;
}


OPENMPT_NAMESPACE_END
