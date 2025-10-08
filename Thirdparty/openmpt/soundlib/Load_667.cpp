/*
 * Load_667.cpp
 * ------------
 * Purpose: Composer 667 module loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct _667FileHeader
{
	using InstrName = std::array<char, 8>;

	char      magic[2];  // 'gf' (0x6667, ha ha)
	InstrName names[64];
	uint8     speed;
	uint8     numOrders;
	uint16le  patOffsets[128];  // Relative to end of instrument definitions

	bool IsValid() const
	{
		if(memcmp(magic, "gf", 2) || speed < 1 || speed > 15 || numOrders > 128)
			return false;
		for(const auto &name : names)
		{
			for(const char c : name)
			{
				if(static_cast<unsigned char>(c) <= 31)
					return false;
			}
		}
		int32 prevOffset = -1;
		for(const int32 offset : patOffsets)
		{
			if(offset <= prevOffset)
				return false;
			prevOffset = offset;
		}
		return true;
	}

	uint32 GetHeaderMinimumAdditionalSize() const
	{
		return numOrders + 64 * 11;
	}
};

MPT_BINARY_STRUCT(_667FileHeader, 772)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeader667(MemoryFileReader file, const uint64 *pfilesize)
{
	_667FileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::Read667(FileReader &file, ModLoadingFlags loadFlags)
{
	_667FileHeader fileHeader;

	file.Rewind();
	if(!file.ReadStruct(fileHeader) || !fileHeader.IsValid())
		return false;
	if(!file.CanRead(fileHeader.GetHeaderMinimumAdditionalSize()))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_S3M, 18);
	m_SongFlags.set(SONG_IMPORTED);
	Order().SetDefaultTempoInt(150);
	Order().SetDefaultSpeed(fileHeader.speed);
	m_nSamples = 64;

	ReadOrderFromFile<uint8>(Order(), file, fileHeader.numOrders);
	for(PATTERNINDEX pat : Order())
	{
		if(pat >= 128)
			return false;
	}

	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		// Reorder OPL patch bytes (interleave modulator and carrier)
		const auto fm = file.ReadArray<uint8, 11>();
		OPLPatch patch{{}};
		patch[0] = fm[1]; patch[1] = fm[6];
		patch[2] = fm[2]; patch[3] = fm[7];
		patch[4] = fm[3]; patch[5] = fm[8];
		patch[6] = fm[4]; patch[7] = fm[9];
		patch[8] = fm[5]; patch[9] = fm[10];
		patch[10] = fm[0];

		ModSample &mptSmp = Samples[smp];
		mptSmp.Initialize(MOD_TYPE_S3M);
		mptSmp.SetAdlib(true, patch);

		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.names[smp - 1]);
	}

	if(loadFlags & loadPatternData)
	{
		const auto patternOffset = file.GetPosition();
		bool leftChn = false, rightChn = false;
		Patterns.ResizeArray(128);
		for(PATTERNINDEX pat = 0; pat < 128; pat++)
		{
			// 4674 bytes is the largest sensible pattern size (every cell is written to exactly once + jump at end of pattern)
			FileReader patData = file.GetChunkAt(patternOffset + fileHeader.patOffsets[pat], 4674);
			if(!patData.IsValid() || !Patterns.Insert(pat, 32))
				break;
			ROWINDEX row = 0;
			auto rowData = Patterns[pat].GetRow(row);
			while(patData.CanRead(1))
			{
				uint8 b = patData.ReadUint8();
				if(b == 0xFF)
				{
					// End of row
					uint8 skip = patData.ReadUint8();
					row += skip;
					if(row >= 32 || skip == 0)
						break;
					rowData = Patterns[pat].GetRow(row);
				} else if(b == 0xFE)
				{
					// Instrument
					auto instr = patData.ReadArray<uint8, 2>();
					if(instr[0] >= GetNumChannels() || instr[1] > 63)
						return false;
					rowData[instr[0]].instr = instr[1] + 1;
				} else if(b == 0xFD)
				{
					// Volume
					auto vol = patData.ReadArray<uint8, 2>();
					if(vol[0] >= GetNumChannels() || vol[1] > 63)
						return false;
					rowData[vol[0]].SetVolumeCommand(VOLCMD_VOLUME, 63u - vol[1]);
				} else if(b == 0xFC)
				{
					// Jump to pattern
					uint8 target = patData.ReadUint8();
					rowData[0].SetEffectCommand(CMD_POSITIONJUMP, target);
				} else if(b == 0xFB)
				{
					// Pattern break
					rowData[0].SetEffectCommand(CMD_PATTERNBREAK, 0);
				} else if(b < GetNumChannels())
				{
					// Note data
					uint8 note = patData.ReadUint8();
					if(note >= 0x7C)
						return false;
					rowData[b].note = static_cast<ModCommand::NOTE>(NOTE_MIN + 12 + (note & 0x0F) + (note >> 4) * 12);
					if(b % 2u)
						rightChn = true;
					else
						leftChn = true;
				} else
				{
					return false;
				}
			}
		}
		if(leftChn && rightChn)
		{
			for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
			{
				ChnSettings[chn].nPan = (chn % 2u) ? 256 : 0;
			}
		}
	}

	m_modFormat.formatName = UL_("Composer 667");
	m_modFormat.type = UL_("667");
	m_modFormat.madeWithTracker = UL_("Composer 667");
	m_modFormat.charset = mpt::Charset::CP437;

	return true;
}


OPENMPT_NAMESPACE_END
