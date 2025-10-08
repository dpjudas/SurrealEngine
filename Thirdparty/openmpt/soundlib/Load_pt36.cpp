/*
 * Load_pt36.cpp
 * -------------
 * Purpose: ProTracker 3.6 wrapper format loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct PT36IffChunk
{
	// IFF chunk names
	enum ChunkIdentifiers
	{
		idVERS = MagicBE("VERS"),
		idINFO = MagicBE("INFO"),
		idCMNT = MagicBE("CMNT"),
		idPTDT = MagicBE("PTDT"),
	};

	uint32be signature;  // IFF chunk name
	uint32be chunksize;  // chunk size without header
};

MPT_BINARY_STRUCT(PT36IffChunk, 8)

struct PT36InfoChunk
{
	char     name[32];
	uint16be numSamples;
	uint16be numOrders;
	uint16be numPatterns;
	uint16be volume;
	uint16be tempo;
	uint16be flags;
	uint16be dateDay;
	uint16be dateMonth;
	uint16be dateYear;
	uint16be dateHour;
	uint16be dateMinute;
	uint16be dateSecond;
	uint16be playtimeHour;
	uint16be playtimeMinute;
	uint16be playtimeSecond;
	uint16be playtimeMsecond;
};

MPT_BINARY_STRUCT(PT36InfoChunk, 64)


struct PT36Header
{
	char     magicFORM[4]; // "FORM"
	uint32be size;
	char     magicMODL[4]; // "MODL"

	bool IsValid() const
	{
		if(std::memcmp(magicFORM, "FORM", 4))
			return false;
		if(std::memcmp(magicMODL, "MODL", 4))
			return false;
		return true;
	}
};

MPT_BINARY_STRUCT(PT36Header, 12)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderPT36(MemoryFileReader file, const uint64 *pfilesize)
{
	PT36Header fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
		return ProbeFailure;
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


// ProTracker 3.6 version of the MOD format
// Basically just a normal ProTracker mod with different magic, wrapped in an IFF file.
// The "PTDT" chunk is passed to the normal MOD loader.
bool CSoundFile::ReadPT36(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	PT36Header fileHeader;
	if(!file.ReadStruct(fileHeader))
		return false;
	if(!fileHeader.IsValid())
		return false;

	bool ok = false, infoOk = false;
	FileReader commentChunk;
	mpt::ustring version;
	PT36InfoChunk info;
	MemsetZero(info);

	// Go through IFF chunks...
	PT36IffChunk iffHead;
	if(!file.ReadStruct(iffHead))
	{
		return false;
	}
	// First chunk includes "MODL" magic in size
	iffHead.chunksize -= 4;

	do
	{
		// All chunk sizes include chunk header
		iffHead.chunksize -= 8;
		if(loadFlags == onlyVerifyHeader && iffHead.signature == PT36IffChunk::idPTDT)
		{
			return true;
		}

		FileReader chunk = file.ReadChunk(iffHead.chunksize);
		if(!chunk.IsValid())
		{
			break;
		}

		switch(iffHead.signature)
		{
		case PT36IffChunk::idVERS:
			chunk.Skip(4);
			if(chunk.ReadMagic("PT") && iffHead.chunksize > 6)
			{
				chunk.ReadString<mpt::String::maybeNullTerminated>(version, mpt::Charset::Amiga_no_C1, iffHead.chunksize - 6);
			}
			break;

		case PT36IffChunk::idINFO:
			infoOk = chunk.ReadStruct(info);
			break;

		case PT36IffChunk::idCMNT:
			commentChunk = chunk;
			break;

		case PT36IffChunk::idPTDT:
			ok = ReadMOD(chunk, loadFlags);
			break;
		}
	} while(file.ReadStruct(iffHead));

	if(version.empty())
	{
		version = UL_("3.6");
	}

	// both an info chunk and a module are required
	if(ok && infoOk)
	{
		bool vblank = (info.flags & 0x100) == 0;
		m_playBehaviour.set(kMODVBlankTiming, vblank);
		if(info.volume != 0)
			m_nSamplePreAmp = std::min(uint16(64), static_cast<uint16>(info.volume));
		if(info.tempo != 0 && !vblank)
			Order().SetDefaultTempoInt(info.tempo);

		if(info.name[0])
			m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, info.name);

		if(mpt::is_in_range(info.dateMonth, 1, 12) && mpt::is_in_range(info.dateDay, 1, 31) && mpt::is_in_range(info.dateHour, 0, 23)
		   && mpt::is_in_range(info.dateMinute, 0, 59) && mpt::is_in_range(info.dateSecond, 0, 59))
		{
#ifdef MODPLUG_TRACKER
			m_modFormat.timezone = mpt::Date::LogicalTimezone::Local;
#else
			m_modFormat.timezone = mpt::Date::LogicalTimezone::Unspecified;
#endif
			FileHistory mptHistory;
			mptHistory.loadDate.year = info.dateYear + 1900;
			mptHistory.loadDate.month = info.dateMonth;
			mptHistory.loadDate.day = info.dateDay;
			mptHistory.loadDate.hours = info.dateHour;
			mptHistory.loadDate.minutes = info.dateMinute;
			mptHistory.loadDate.seconds = info.dateSecond;
			m_FileHistory.push_back(mptHistory);
		}
	}
	if(ok)
	{
		if(commentChunk.IsValid())
		{
			std::string author;
			commentChunk.ReadString<mpt::String::maybeNullTerminated>(author, 32);
			if(author != "UNNAMED AUTHOR")
				m_songArtist = mpt::ToUnicode(mpt::Charset::Amiga_no_C1, author);
			if(!commentChunk.NoBytesLeft())
			{
				m_songMessage.ReadFixedLineLength(commentChunk, commentChunk.BytesLeft(), 40, 0);
			}
		}

		m_modFormat.madeWithTracker = UL_("ProTracker ") + version;
	}
	m_SongFlags.set(SONG_PT_MODE);
	m_playBehaviour.set(kMODIgnorePanning);
	m_playBehaviour.set(kMODOneShotLoops);
	m_playBehaviour.reset(kMODSampleSwap);

	return ok;
}

OPENMPT_NAMESPACE_END
