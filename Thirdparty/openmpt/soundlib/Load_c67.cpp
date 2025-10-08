/*
 * Load_c67.cpp
 * ------------
 * Purpose: C67 (CDFM Composer) module loader
 * Notes  : C67 is the composer format; 670 files can be converted back to C67 using the converter that comes with CDFM.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct C67SampleHeader
{
	uint32le unknown; // Probably placeholder for in-memory address, 0 on disk
	uint32le length;
	uint32le loopStart;
	uint32le loopEnd;
};

MPT_BINARY_STRUCT(C67SampleHeader, 16)


struct C67FileHeader
{
	using InstrName = std::array<char, 13>;
	using OPLInstr = std::array<uint8, 11>;

	uint8           speed;
	uint8           restartPos;
	InstrName       sampleNames[32];
	C67SampleHeader samples[32];
	InstrName       fmInstrNames[32];
	OPLInstr        fmInstr[32];
	uint8           orders[256];
};

MPT_BINARY_STRUCT(C67FileHeader, 1954)


static bool ValidateHeader(const C67FileHeader &fileHeader)
{
	if(fileHeader.speed < 1 || fileHeader.speed > 15)
		return false;
	for(auto ord : fileHeader.orders)
	{
		if(ord >= 128 && ord != 0xFF)
			return false;
	}

	bool anyNonSilent = false;
	for(SAMPLEINDEX smp = 0; smp < 32; smp++)
	{
		if(fileHeader.sampleNames[smp][12] != 0
			|| fileHeader.samples[smp].unknown != 0
			|| fileHeader.samples[smp].length > 0xFFFFF
			|| fileHeader.fmInstrNames[smp][12] != 0
			|| (fileHeader.fmInstr[smp][0] & 0xF0) // No OPL3
			|| (fileHeader.fmInstr[smp][5] & 0xFC) // No OPL3
			|| (fileHeader.fmInstr[smp][10] & 0xFC)) // No OPL3
		{
			return false;
		}
		if(fileHeader.samples[smp].length != 0 && fileHeader.samples[smp].loopEnd < 0xFFFFF)
		{
			if(fileHeader.samples[smp].loopEnd > fileHeader.samples[smp].length
				|| fileHeader.samples[smp].loopStart > fileHeader.samples[smp].loopEnd)
			{
				return false;
			}
		}
		if(!anyNonSilent && (fileHeader.samples[smp].length != 0 || fileHeader.fmInstr[smp] != C67FileHeader::OPLInstr{{}}))
		{
			anyNonSilent = true;
		}
	}
	return anyNonSilent;
}


static uint64 GetHeaderMinimumAdditionalSize(const C67FileHeader &)
{
	return 1024; // Pattern offsets and lengths
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderC67(MemoryFileReader file, const uint64 *pfilesize)
{
	C67FileHeader fileHeader;
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


static void TranslateVolume(ModCommand &m, uint8 volume, bool isFM)
{
	// CDFM uses a linear volume scale for FM instruments.
	// ScreamTracker, on the other hand, directly uses the OPL chip's logarithmic volume scale.
	// Neither FM nor PCM instruments can be fully muted in CDFM.
	static constexpr uint8 fmVolume[16] =
	{
		0x08, 0x10, 0x18, 0x20, 0x28, 0x2C, 0x30, 0x34,
		0x36, 0x38, 0x3A, 0x3C, 0x3D, 0x3E, 0x3F, 0x40,
	};

	volume &= 0x0F;
	m.volcmd = VOLCMD_VOLUME;
	m.vol = isFM ? fmVolume[volume] : static_cast<ModCommand::VOL>((4u + volume * 4u));
}


bool CSoundFile::ReadC67(FileReader &file, ModLoadingFlags loadFlags)
{
	C67FileHeader fileHeader;

	file.Rewind();
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(!ValidateHeader(fileHeader))
	{
		return false;
	}
	if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(GetHeaderMinimumAdditionalSize(fileHeader))))
	{
		return false;
	}

	// Validate pattern offsets and lengths
	uint32le patOffsets[128], patLengths[128];
	file.ReadArray(patOffsets);
	file.ReadArray(patLengths);
	for(PATTERNINDEX pat = 0; pat < 128; pat++)
	{
		if(patOffsets[pat] > 0xFFFFFF
			|| patLengths[pat] < 3      // Smallest well-formed pattern consists of command 0x40 followed by command 0x60
			|| patLengths[pat] > 0x1000 // Any well-formed pattern is smaller than this
			|| !file.LengthIsAtLeast(2978 + patOffsets[pat] + patLengths[pat]))
		{
			return false;
		}
	}

	InitializeGlobals(MOD_TYPE_S3M, 4 + 9);

	m_modFormat.formatName = UL_("CDFM");
	m_modFormat.type = UL_("c67");
	m_modFormat.madeWithTracker = UL_("Composer 670");
	m_modFormat.charset = mpt::Charset::CP437;

	Order().SetDefaultSpeed(fileHeader.speed);
	Order().SetDefaultTempoInt(143);
	Order().SetRestartPos(fileHeader.restartPos);
	m_nSamples = 64;
	m_playBehaviour.set(kOPLBeatingOscillators);
	m_SongFlags.set(SONG_IMPORTED);

	// Pan PCM channels only
	for(CHANNELINDEX chn = 0; chn < 4; chn++)
	{
		ChnSettings[chn].nPan = (chn & 1) ? 192 : 64;
	}

	// PCM instruments
	for(SAMPLEINDEX smp = 0; smp < 32; smp++)
	{
		ModSample &mptSmp = Samples[smp + 1];
		mptSmp.Initialize(MOD_TYPE_S3M);
		m_szNames[smp + 1] = mpt::String::ReadBuf(mpt::String::nullTerminated, fileHeader.sampleNames[smp]);
		mptSmp.nLength = fileHeader.samples[smp].length;
		if(fileHeader.samples[smp].loopEnd <= fileHeader.samples[smp].length)
		{
			mptSmp.nLoopStart = fileHeader.samples[smp].loopStart;
			mptSmp.nLoopEnd = fileHeader.samples[smp].loopEnd;
			mptSmp.uFlags = CHN_LOOP;
		}
		mptSmp.nC5Speed = 8287;
	}
	// OPL instruments
	for(SAMPLEINDEX smp = 0; smp < 32; smp++)
	{
		ModSample &mptSmp = Samples[smp + 33];
		mptSmp.Initialize(MOD_TYPE_S3M);
		m_szNames[smp + 33] = mpt::String::ReadBuf(mpt::String::nullTerminated, fileHeader.fmInstrNames[smp]);
		// Reorder OPL patch bytes (interleave modulator and carrier)
		const auto &fm = fileHeader.fmInstr[smp];
		OPLPatch patch{{}};
		patch[0] = fm[1]; patch[1] = fm[6];
		patch[2] = fm[2]; patch[3] = fm[7];
		patch[4] = fm[3]; patch[5] = fm[8];
		patch[6] = fm[4]; patch[7] = fm[9];
		patch[8] = fm[5]; patch[9] = fm[10];
		patch[10] = fm[0];
		mptSmp.SetAdlib(true, patch);
	}

	ReadOrderFromArray<uint8>(Order(), fileHeader.orders, 256, 0xFF);
	Patterns.ResizeArray(128);
	for(PATTERNINDEX pat = 0; pat < 128; pat++)
	{
		file.Seek(2978 + patOffsets[pat]);
		FileReader patChunk = file.ReadChunk(patLengths[pat]);
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
		{
			continue;
		}
		CPattern &pattern = Patterns[pat];
		ROWINDEX row = 0;
		while(row < 64 && patChunk.CanRead(1))
		{
			uint8 cmd = patChunk.ReadUint8();
			if(cmd <= 0x0C)
			{
				// Note, instrument, volume
				ModCommand &m = *pattern.GetpModCommand(row, cmd);
				const auto [note, instrVol] = patChunk.ReadArray<uint8, 2>();
				bool fmChn = (cmd >= 4);
				m.note = static_cast<ModCommand::NOTE>(NOTE_MIN + (fmChn ? 12 : 36) + (note & 0x0F) + ((note >> 4) & 0x07) * 12);
				m.instr = static_cast<ModCommand::INSTR>((fmChn ? 33 : 1) + (instrVol >> 4) + ((note & 0x80) >> 3));
				TranslateVolume(m, instrVol, fmChn);
			} else if(cmd >= 0x20 && cmd <= 0x2C)
			{
				// Volume
				TranslateVolume(*pattern.GetpModCommand(row, cmd - 0x20), patChunk.ReadUint8(), cmd >= 0x24);
			} else if(cmd == 0x40)
			{
				// Delay (row done)
				row += patChunk.ReadUint8();
			} else if(cmd == 0x60)
			{
				// End of pattern
				if(row > 0)
				{
					pattern.GetpModCommand(row - 1, 0)->command = CMD_PATTERNBREAK;
				}
				break;
			} else
			{
				return false;
			}
		}
	}
	if(loadFlags & loadSampleData)
	{
		for(SAMPLEINDEX smp = 1; smp <= 32; smp++)
		{
			SampleIO(SampleIO::_8bit, SampleIO::mono, SampleIO::littleEndian, SampleIO::unsignedPCM).ReadSample(Samples[smp], file);
		}
	}
	return true;
}

OPENMPT_NAMESPACE_END
