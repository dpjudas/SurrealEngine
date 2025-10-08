/*
 * Load_fmt.cpp
 * ------------
 * Purpose: Davey W Taylor's FM Tracker module loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct FMTChannelSetting
{
	char name[8];
	char settings[11];
};

MPT_BINARY_STRUCT(FMTChannelSetting, 19)


struct FMTFileHeader
{
	char magic[11];  // Includes format version number for simplicity
	char trackerName[20];
	char songName[32];

	FMTChannelSetting channels[8];
	uint8 lastRow;
	uint8 lastOrder;
	uint8 lastPattern;
};

MPT_BINARY_STRUCT(FMTFileHeader, 218)


static uint64 GetHeaderMinimumAdditionalSize(const FMTFileHeader &fileHeader)
{
	// Order list + pattern delays, pattern mapping + at least one byte per channel
	return (fileHeader.lastOrder + 1u) * 2u + (fileHeader.lastPattern + 1u) * 9u;
}


static bool ValidateHeader(const FMTFileHeader &fileHeader)
{
	if(memcmp(fileHeader.magic, "FMTracker\x01\x01", 11))
		return false;

	for(const auto &channel : fileHeader.channels)
	{
		// Reject anything that resembles OPL3
		if((channel.settings[8] & 0xFC) || (channel.settings[9] & 0xFC) || (channel.settings[10] & 0xF0))
			return false;
	}

	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderFMT(MemoryFileReader file, const uint64 *pfilesize)
{
	FMTFileHeader fileHeader;
	if(!file.Read(fileHeader))
		return ProbeWantMoreData;
	if(!ValidateHeader(fileHeader))
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, GetHeaderMinimumAdditionalSize(fileHeader));
}


bool CSoundFile::ReadFMT(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	FMTFileHeader fileHeader;
	if(!file.Read(fileHeader))
		return false;
	if(!ValidateHeader(fileHeader))
		return false;
	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(GetHeaderMinimumAdditionalSize(fileHeader))))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_S3M, 8);
	m_nSamples = 8;
	Order().SetDefaultTempo(TEMPO(45.5));  // 18.2 Hz timer
	m_playBehaviour.set(kOPLNoteStopWith0Hz);
	m_SongFlags.set(SONG_IMPORTED | SONG_FORMAT_NO_VOLCOL);
	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.songName);

	for(CHANNELINDEX chn = 0; chn < 8; chn++)
	{
		const auto name = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.channels[chn].name);
		ChnSettings[chn].szName = name;

		ModSample &mptSmp = Samples[chn + 1];
		mptSmp.Initialize(MOD_TYPE_S3M);
		OPLPatch patch{{}};
		memcpy(patch.data(), fileHeader.channels[chn].settings, 11);
		mptSmp.SetAdlib(true, patch);
		mptSmp.nC5Speed = 8215;
		m_szNames[chn + 1] = name;
	}

	const ORDERINDEX numOrders = fileHeader.lastOrder + 1u;
	ReadOrderFromFile<uint8>(Order(), file, numOrders);

	std::vector<uint8> delays;
	file.ReadVector(delays, numOrders);
	for(uint8 delay : delays)
	{
		if(delay < 1 || delay > 8)
			return false;
	}
	Order().SetDefaultSpeed(delays[0]);

	const PATTERNINDEX numPatterns = fileHeader.lastPattern + 1u;
	const ROWINDEX numRows = fileHeader.lastRow + 1u;
	std::vector<uint8> patternMap;
	file.ReadVector(patternMap, numPatterns);

	Patterns.ResizeArray(numPatterns);
	for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
	{
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(patternMap[pat], numRows))
			break;
		auto &pattern = Patterns[patternMap[pat]];
		for(CHANNELINDEX chn = 0; chn < 8; chn++)
		{
			for(ROWINDEX row = 0; row < pattern.GetNumRows(); row++)
			{
				uint8 data = file.ReadUint8();
				if(data & 0x80)
				{
					row += data & 0x7F;
				} else
				{
					ModCommand &m = *pattern.GetpModCommand(row, chn);
					if(data == 1)
					{
						m.note = NOTE_NOTECUT;
					} else if(data >= 2 && data <= 97)
					{
						m.note = data + NOTE_MIN + 11u;
						m.instr = static_cast<ModCommand::INSTR>(chn + 1u);
					}
				}
			}
		}
	}

	// Write song speed to patterns... due to a quirk in the original playback routine
	// (delays is applied before notes are triggered, not afterwards), a pattern's delay
	// already applies to the last row of the previous pattern.
	// In case you wonder if anyone would ever notice: My own songs written with this tracker
	// actively work around this issue and will sound wrong if tempo is changed on the first row.
	for(ORDERINDEX ord = 0; ord < numOrders; ord++)
	{
		if(!Order().IsValidPat(ord))
		{
			if(PATTERNINDEX pat = Patterns.InsertAny(numRows); pat != PATTERNINDEX_INVALID)
				Order()[ord] = pat;
			else
				continue;
		}
		auto m = Patterns[Order()[ord]].end() - 1;
		auto delay = delays[(ord + 1u) % numOrders];
		if(m->param == delay)
			continue;

		if(m->command == CMD_SPEED)
		{
			PATTERNINDEX newPat = Order().EnsureUnique(ord);
			if(newPat != PATTERNINDEX_INVALID)
				m = Patterns[newPat].end() - 1;
		}
		m->command = CMD_SPEED;
		m->param = delay;
	}

	m_modFormat.formatName = UL_("FM Tracker");
	m_modFormat.type = UL_("fmt");
	m_modFormat.madeWithTracker = mpt::ToUnicode(mpt::Charset::CP437, mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.trackerName));
	m_modFormat.charset = mpt::Charset::CP437;

	return true;
}

OPENMPT_NAMESPACE_END
