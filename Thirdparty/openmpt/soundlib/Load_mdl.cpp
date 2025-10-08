/*
 * Load_mdl.cpp
 * ------------
 * Purpose: Digitrakker (MDL) module loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

// MDL file header
struct MDLFileHeader
{
	char  id[4];	// "DMDL"
	uint8 version;
};

MPT_BINARY_STRUCT(MDLFileHeader, 5)


// RIFF-style Chunk
struct MDLChunk
{
	// 16-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idInfo			= MagicLE("IN"),
		idMessage		= MagicLE("ME"),
		idPats			= MagicLE("PA"),
		idPatNames		= MagicLE("PN"),
		idTracks		= MagicLE("TR"),
		idInstrs		= MagicLE("II"),
		idVolEnvs		= MagicLE("VE"),
		idPanEnvs		= MagicLE("PE"),
		idFreqEnvs		= MagicLE("FE"),
		idSampleInfo	= MagicLE("IS"),
		ifSampleData	= MagicLE("SA"),
	};

	uint16le id;
	uint32le length;

	size_t GetLength() const
	{
		return length;
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(id.get());
	}
};

MPT_BINARY_STRUCT(MDLChunk, 6)


struct MDLInfoBlock
{
	char     title[32];
	char     composer[20];
	uint16le numOrders;
	uint16le restartPos;
	uint8le  globalVol;  // 1...255
	uint8le  speed;      // 1...255
	uint8le  tempo;      // 4...255
	uint8le  chnSetup[32];

	uint8 GetNumChannels() const
	{
		uint8 numChannels = 0;
		for(uint8 c = 0; c < 32; c++)
		{
			if(!(chnSetup[c] & 0x80))
				numChannels = c + 1;
		}
		return numChannels;
	}
};

MPT_BINARY_STRUCT(MDLInfoBlock, 91)


// Sample header in II block
struct MDLSampleHeader
{
	uint8le  smpNum;
	uint8le  lastNote;
	uint8le  volume;
	uint8le  volEnvFlags;	// 6 bits env #, 2 bits flags
	uint8le  panning;
	uint8le  panEnvFlags;
	uint16le fadeout;
	uint8le  vibSpeed;
	uint8le  vibDepth;
	uint8le  vibSweep;
	uint8le  vibType;
	uint8le  reserved;		// zero
	uint8le  freqEnvFlags;
};

MPT_BINARY_STRUCT(MDLSampleHeader, 14)


struct MDLEnvelope
{
	uint8 envNum;
	struct
	{
		uint8 x;	// Delta value from last point, 0 means no more points defined
		uint8 y;	// 0...63
	} nodes[15];
	uint8 flags;
	uint8 loop;		// Lower 4 bits = start, upper 4 bits = end

	void ConvertToMPT(InstrumentEnvelope &mptEnv) const
	{
		mptEnv.dwFlags.reset();
		mptEnv.clear();
		mptEnv.reserve(15);
		int16 tick = -nodes[0].x;
		for(uint8 n = 0; n < 15; n++)
		{
			if(!nodes[n].x)
				break;
			tick += nodes[n].x;
			mptEnv.push_back(EnvelopeNode(tick, std::min(nodes[n].y, uint8(64)))); // actually 0-63
		}

		mptEnv.nLoopStart = (loop & 0x0F);
		mptEnv.nLoopEnd = (loop >> 4);
		mptEnv.nSustainStart = mptEnv.nSustainEnd = (flags & 0x0F);

		if(flags & 0x10) mptEnv.dwFlags.set(ENV_SUSTAIN);
		if(flags & 0x20) mptEnv.dwFlags.set(ENV_LOOP);
	}
};

MPT_BINARY_STRUCT(MDLEnvelope, 33)


struct MDLPatternHeader
{
	uint8le channels;
	uint8le lastRow;
	char    name[16];
};

MPT_BINARY_STRUCT(MDLPatternHeader, 18)


enum
{
	MDLNOTE_NOTE	= 1 << 0,
	MDLNOTE_SAMPLE	= 1 << 1,
	MDLNOTE_VOLUME	= 1 << 2,
	MDLNOTE_EFFECTS	= 1 << 3,
	MDLNOTE_PARAM1	= 1 << 4,
	MDLNOTE_PARAM2	= 1 << 5,
};


static constexpr VibratoType MDLVibratoType[] = { VIB_SINE, VIB_RAMP_DOWN, VIB_SQUARE, VIB_SINE };

static constexpr EffectCommand MDLEffTrans[] =
{
	/* 0 */ CMD_NONE,
	/* 1st column only */
	/* 1 */ CMD_PORTAMENTOUP,
	/* 2 */ CMD_PORTAMENTODOWN,
	/* 3 */ CMD_TONEPORTAMENTO,
	/* 4 */ CMD_VIBRATO,
	/* 5 */ CMD_ARPEGGIO,
	/* 6 */ CMD_NONE,
	/* Either column */
	/* 7 */ CMD_TEMPO,
	/* 8 */ CMD_PANNING8,
	/* 9 */ CMD_S3MCMDEX,
	/* A */ CMD_NONE,
	/* B */ CMD_POSITIONJUMP,
	/* C */ CMD_GLOBALVOLUME,
	/* D */ CMD_PATTERNBREAK,
	/* E */ CMD_S3MCMDEX,
	/* F */ CMD_SPEED,
	/* 2nd column only */
	/* G */ CMD_VOLUMESLIDE, // up
	/* H */ CMD_VOLUMESLIDE, // down
	/* I */ CMD_RETRIG,
	/* J */ CMD_TREMOLO,
	/* K */ CMD_TREMOR,
	/* L */ CMD_NONE,
};


// receive an MDL effect, give back a 'normal' one.
static std::pair<EffectCommand, uint8> ConvertMDLCommand(const uint8 command, uint8 param)
{
	if(command >= std::size(MDLEffTrans))
		return {CMD_NONE, uint8(0)};

	EffectCommand cmd = MDLEffTrans[command];
	switch(command)
	{
#ifdef MODPLUG_TRACKER
	case 0x07: // Tempo
		// MDL supports any nonzero tempo value, but OpenMPT doesn't
		param = std::max(param, uint8(0x20));
		break;
#endif // MODPLUG_TRACKER
	case 0x08: // Panning
		param = (param & 0x7F) * 2u;
		break;
	case 0x09: // Set Envelope (we can only have one envelope per type...)
		if(param < 0x40)
			param = 0x78;  // Enable the one volume envelope we have
		else if (param < 0x80)
			param = 0x7A;  // Enable the one panning envelope we have
		else if(param < 0xC0)
			param = 0x7C;  // Enable the one pitch envelope we have
		else
			cmd = CMD_NONE;
		break;
	case 0x0C:	// Global volume
		param = static_cast<uint8>((param + 1) / 2u);
		break;
	case 0x0D: // Pattern Break
		// Convert from BCD
		param = static_cast<uint8>(10 * (param >> 4) + (param & 0x0F));
		break;
	case 0x0E: // Special
		switch(param >> 4)
		{
		case 0x0: // unused
		case 0x3: // unused
		case 0x8: // Set Samplestatus (loop type)
			cmd = CMD_NONE;
			break;
		case 0x1: // Pan Slide Left
			cmd = CMD_PANNINGSLIDE;
			param = (std::min(static_cast<uint8>(param & 0x0F), uint8(0x0E)) << 4) | 0x0F;
			break;
		case 0x2: // Pan Slide Right
			cmd = CMD_PANNINGSLIDE;
			param = 0xF0 | std::min(static_cast<uint8>(param & 0x0F), uint8(0x0E));
			break;
		case 0x4: // Vibrato Waveform
			param = 0x30 | (param & 0x0F);
			break;
		case 0x5:  // Set Finetune
			cmd = CMD_FINETUNE;
			param = (param << 4) ^ 0x80;
			break;
		case 0x6: // Pattern Loop
			param = 0xB0 | (param & 0x0F);
			break;
		case 0x7: // Tremolo Waveform
			param = 0x40 | (param & 0x0F);
			break;
		case 0x9: // Retrig
			cmd = CMD_RETRIG;
			param &= 0x0F;
			break;
		case 0xA: // Global vol slide up
			cmd = CMD_GLOBALVOLSLIDE;
			param = static_cast<uint8>(0xF0 & (((param & 0x0F) + 1) << 3));
			break;
		case 0xB: // Global vol slide down
			cmd = CMD_GLOBALVOLSLIDE;
			param = static_cast<uint8>(((param & 0x0F) + 1) >> 1);
			break;
		case 0xC: // Note cut
		case 0xD: // Note delay
		case 0xE: // Pattern delay
			// Nothing to change here
			break;
		case 0xF: // Offset -- further mangled later.
			cmd = CMD_OFFSET;
			break;
		}
		break;
	case 0x10: // Volslide up
		if(param < 0xE0)
		{
			// 00...DF regular slide - four times more precise than in XM
			param >>= 2;
			if(param > 0x0F)
				param = 0x0F;
			param <<= 4;
		} else if(param < 0xF0)
		{
			// E0...EF extra fine slide (on first tick, 4 times finer)
			param = (((param & 0x0F) << 2) | 0x0F);
		} else
		{
			// F0...FF regular fine slide (on first tick) - like in XM
			param = ((param << 4) | 0x0F);
		}
		break;
	case 0x11: // Volslide down
		if(param < 0xE0)
		{
			// 00...DF regular slide - four times more precise than in XM
			param >>= 2;
			if(param > 0x0F)
				param = 0x0F;
		} else if(param < 0xF0)
		{
			// E0...EF extra fine slide (on first tick, 4 times finer)
			param = (((param & 0x0F) >> 2) | 0xF0);
		} else
		{
			// F0...FF regular fine slide (on first tick) - like in XM
		}
		break;
	}
	return {cmd, param};
}


// Returns true if command was lost
static bool ImportMDLCommands(ModCommand &m, uint8 vol, uint8 cmd1, uint8 cmd2, uint8 param1, uint8 param2)
{
	// Map second effect values 1-6 to effects G-L
	if(cmd2 >= 1 && cmd2 <= 6)
		cmd2 += 15;

	auto [e1, p1] = ConvertMDLCommand(cmd1, param1);
	auto [e2, p2] = ConvertMDLCommand(cmd2, param2);
	/* From the Digitrakker documentation:
		* EFx -xx - Set Sample Offset
		This  is a  double-command.  It starts the
		sample at adress xxx*256.
		Example: C-5 01 -- EF1 -23 ->starts sample
		01 at address 12300 (in hex).
	Kind of screwy, but I guess it's better than the mess required to do it with IT (which effectively
	requires 3 rows in order to set the offset past 0xff00). If we had access to the entire track, we
	*might* be able to shove the high offset SAy into surrounding rows (or 2x MPTM #xx), but it wouldn't
	always be possible, it'd make the loader a lot uglier, and generally would be more trouble than
	it'd be worth to implement.

	What's more is, if there's another effect in the second column, it's ALSO processed in addition to the
	offset, and the second data byte is shared between the two effects. */
	uint32 offset = uint32_max;
	EffectCommand otherCmd = CMD_NONE;
	if(e1 == CMD_OFFSET)
	{
		// EFy -xx => offset yxx00
		offset = ((p1 & 0x0F) << 8) | p2;
		p1 = (p1 & 0x0F) ? 0xFF : p2;
		if(e2 == CMD_OFFSET)
			e2 = CMD_NONE;
		else
			otherCmd = e2;
	} else if (e2 == CMD_OFFSET)
	{
		// --- EFy => offset y0000
		offset = (p2 & 0x0F) << 8;
		p2 = (p2 & 0x0F) ? 0xFF : 0;
		otherCmd = e1;
	}

	if(offset != uint32_max && offset > 0xFF && ModCommand::GetEffectWeight(otherCmd) < ModCommand::GetEffectWeight(CMD_OFFSET))
	{
		m.SetEffectCommand(CMD_OFFSET, static_cast<ModCommand::PARAM>(offset & 0xFF));
		m.SetVolumeCommand(VOLCMD_OFFSET, static_cast<ModCommand::VOL>(offset >> 8));
		return otherCmd != CMD_NONE || vol != 0;
	}

	if(vol)
	{
		m.SetVolumeCommand(VOLCMD_VOLUME, static_cast<ModCommand::VOL>((vol + 2) / 4u));
	}

	// If we have Dxx + G00, or Dxx + H00, combine them into Lxx/Kxx.
	ModCommand::CombineEffects(e1, p1, e2, p2);

	// Try to preserve the "best" effect.
	if(e1 == CMD_NONE || (e1 == e2 && e1 != CMD_S3MCMDEX))
	{
		// Digitrakker processes the effects left-to-right, so if both effects are the same, the
		// second essentially overrides the first.
		m.SetEffectCommand(e2, p2);
		return false;
	} else if(e2 == CMD_NONE)
	{
		m.SetEffectCommand(e1, p1);
		return false;
	} else if(!vol)
	{
		return m.FillInTwoCommands(e1, p1, e2, p2).first != CMD_NONE;
	} else
	{
		if(ModCommand::GetEffectWeight(e1) > ModCommand::GetEffectWeight(e2))
			m.SetEffectCommand(e1, p1);
		else
			m.SetEffectCommand(e2, p2);
		return true;
	}
}


static void MDLReadEnvelopes(FileReader file, std::vector<MDLEnvelope> &envelopes)
{
	if(!file.CanRead(1))
		return;

	envelopes.resize(64);
	uint8 numEnvs = file.ReadUint8();
	while(numEnvs--)
	{
		MDLEnvelope mdlEnv;
		if(!file.ReadStruct(mdlEnv) || mdlEnv.envNum > 63)
			continue;
		envelopes[mdlEnv.envNum] = mdlEnv;
	}
}


static void CopyEnvelope(InstrumentEnvelope &mptEnv, uint8 flags, std::vector<MDLEnvelope> &envelopes)
{
	uint8 envNum = flags & 0x3F;
	if(envNum < envelopes.size())
		envelopes[envNum].ConvertToMPT(mptEnv);
	mptEnv.dwFlags.set(ENV_ENABLED, (flags & 0x80) && !mptEnv.empty());
}


static uint8 GetMDLPatternChannelCount(FileReader chunk, uint8 numChannels, const uint8 fileVersion)
{
	const uint8 numPats = chunk.ReadUint8();
	for(uint8 pat = 0; pat < numPats && numChannels < 32; pat++)
	{
		uint8 readChans = 32;
		if(fileVersion >= 0x10)
		{
			MDLPatternHeader patHead;
			chunk.ReadStruct(patHead);
			readChans = patHead.channels;
		}
		for(uint8 chn = 0; chn < readChans; chn++)
		{
			if(chunk.ReadUint16LE() > 0 && chn >= numChannels && chn < 32)
				numChannels = chn + 1;
		}
	}
	return numChannels;
}


static bool ValidateHeader(const MDLFileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.id, "DMDL", 4)
		|| fileHeader.version >= 0x20)
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderMDL(MemoryFileReader file, const uint64 *pfilesize)
{
	MDLFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return ProbeWantMoreData;
	}
	if(!ValidateHeader(fileHeader))
	{
		return ProbeFailure;
	}
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadMDL(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	MDLFileHeader fileHeader;
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

	ChunkReader chunkFile(file);
	ChunkReader::ChunkList<MDLChunk> chunks = chunkFile.ReadChunks<MDLChunk>(0);

	// Read global info
	FileReader chunk = chunks.GetChunk(MDLChunk::idInfo);
	MDLInfoBlock info;
	if(!chunk.IsValid() || !chunk.ReadStruct(info))
	{
		return false;
	}

	// In case any muted channels contain data, be sure that we import them as well.
	const uint8 numChannels = std::max(GetMDLPatternChannelCount(chunks.GetChunk(MDLChunk::idPats), info.GetNumChannels(), fileHeader.version), uint8(1));
	InitializeGlobals(MOD_TYPE_MDL, numChannels);
	m_SongFlags = SONG_ITCOMPATGXX;
	m_playBehaviour.set(kPerChannelGlobalVolSlide);
	m_playBehaviour.set(kApplyOffsetWithoutNote);
	m_playBehaviour.reset(kPeriodsAreHertz);
	m_playBehaviour.reset(kITVibratoTremoloPanbrello);
	m_playBehaviour.reset(kITSCxStopsSample);	// Gate effect in underbeat.mdl

	m_modFormat.formatName = UL_("Digitrakker");
	m_modFormat.type = UL_("mdl");
	m_modFormat.madeWithTracker = U_("Digitrakker ") + (
		(fileHeader.version == 0x11) ? UL_("3") // really could be 2.99b - close enough
		: (fileHeader.version == 0x10) ? UL_("2.3")
		: (fileHeader.version == 0x00) ? UL_("2.0 - 2.2b") // there was no 1.x release
		: UL_(""));
	m_modFormat.charset = mpt::Charset::CP437;

	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, info.title);
	m_songArtist = mpt::ToUnicode(mpt::Charset::CP437, mpt::String::ReadBuf(mpt::String::spacePadded, info.composer));

	m_nDefaultGlobalVolume = info.globalVol + 1;
	Order().SetDefaultSpeed(Clamp<uint8, uint8>(info.speed, 1, 255));
	Order().SetDefaultTempoInt(Clamp<uint8, uint8>(info.tempo, 4, 255));

	ReadOrderFromFile<uint8>(Order(), chunk, info.numOrders);
	Order().SetRestartPos(info.restartPos);

	for(CHANNELINDEX c = 0; c < GetNumChannels(); c++)
	{
		ChnSettings[c].nPan = (info.chnSetup[c] & 0x7F) * 2u;
		if(ChnSettings[c].nPan == 254)
			ChnSettings[c].nPan = 256;
		if(info.chnSetup[c] & 0x80)
			ChnSettings[c].dwFlags.set(CHN_MUTE);
		chunk.ReadString<mpt::String::spacePadded>(ChnSettings[c].szName, 8);
	}

	// Read song message
	chunk = chunks.GetChunk(MDLChunk::idMessage);
	m_songMessage.Read(chunk, chunk.GetLength(), SongMessage::leCR);

	// Read sample info and data
	chunk = chunks.GetChunk(MDLChunk::idSampleInfo);
	if(chunk.IsValid())
	{
		FileReader dataChunk = chunks.GetChunk(MDLChunk::ifSampleData);

		uint8 numSamples = chunk.ReadUint8();
		for(uint8 smp = 0; smp < numSamples; smp++)
		{
			const SAMPLEINDEX sampleIndex = chunk.ReadUint8();
			if(sampleIndex == 0 || sampleIndex >= MAX_SAMPLES || !chunk.CanRead(32 + 8 + 2 + 12 + 2))
				break;

			if(sampleIndex > GetNumSamples())
				m_nSamples = sampleIndex;

			ModSample &sample = Samples[sampleIndex];
			sample.Initialize();
			sample.Set16BitCuePoints();

			chunk.ReadString<mpt::String::spacePadded>(m_szNames[sampleIndex], 32);
			chunk.ReadString<mpt::String::spacePadded>(sample.filename, 8);

			uint32 c4speed;
			if(fileHeader.version < 0x10)
				c4speed = chunk.ReadUint16LE();
			else
				c4speed = chunk.ReadUint32LE();
			sample.nC5Speed = c4speed * 2u;
			sample.nLength = chunk.ReadUint32LE();
			sample.nLoopStart = chunk.ReadUint32LE();
			sample.nLoopEnd = chunk.ReadUint32LE();
			if(sample.nLoopEnd != 0)
			{
				sample.uFlags.set(CHN_LOOP);
				sample.nLoopEnd += sample.nLoopStart;
			}
			uint8 volume = chunk.ReadUint8();
			if(fileHeader.version < 0x10)
				sample.nVolume = volume;
			uint8 flags = chunk.ReadUint8();

			if(flags & 0x01)
			{
				sample.uFlags.set(CHN_16BIT);
				sample.nLength /= 2u;
				sample.nLoopStart /= 2u;
				sample.nLoopEnd /= 2u;
			}

			sample.uFlags.set(CHN_PINGPONGLOOP, (flags & 0x02) != 0);

			SampleIO sampleIO(
				(flags & 0x01) ? SampleIO::_16bit : SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				(flags & 0x0C) ? SampleIO::MDL : SampleIO::signedPCM);

			if(loadFlags & loadSampleData)
			{
				sampleIO.ReadSample(sample, dataChunk);
			}
		}
	}

	chunk = chunks.GetChunk(MDLChunk::idInstrs);
	if(chunk.IsValid())
	{
		std::vector<MDLEnvelope> volEnvs, panEnvs, pitchEnvs;
		MDLReadEnvelopes(chunks.GetChunk(MDLChunk::idVolEnvs), volEnvs);
		MDLReadEnvelopes(chunks.GetChunk(MDLChunk::idPanEnvs), panEnvs);
		MDLReadEnvelopes(chunks.GetChunk(MDLChunk::idFreqEnvs), pitchEnvs);

		uint8 numInstruments = chunk.ReadUint8();
		for(uint8 i = 0; i < numInstruments; i++)
		{
			const auto [ins, numSamples] = chunk.ReadArray<uint8, 2>();
			uint8 firstNote = 0;
			ModInstrument *mptIns = nullptr;
			if(ins == 0
				|| !chunk.CanRead(32 + sizeof(MDLSampleHeader) * numSamples)
				|| (mptIns = AllocateInstrument(ins)) == nullptr)
			{
				chunk.Skip(32 + sizeof(MDLSampleHeader) * numSamples);
				continue;
			}

			chunk.ReadString<mpt::String::spacePadded>(mptIns->name, 32);
			for(uint8 smp = 0; smp < numSamples; smp++)
			{
				MDLSampleHeader sampleHeader;
				chunk.ReadStruct(sampleHeader);
				if(sampleHeader.smpNum == 0 || sampleHeader.smpNum > GetNumSamples())
					continue;

				LimitMax(sampleHeader.lastNote, static_cast<uint8>(std::size(mptIns->Keyboard)));
				for(uint8 n = firstNote; n <= sampleHeader.lastNote; n++)
				{
					mptIns->Keyboard[n] = sampleHeader.smpNum;
				}
				firstNote = sampleHeader.lastNote + 1;

				CopyEnvelope(mptIns->VolEnv, sampleHeader.volEnvFlags, volEnvs);
				CopyEnvelope(mptIns->PanEnv, sampleHeader.panEnvFlags, panEnvs);
				CopyEnvelope(mptIns->PitchEnv, sampleHeader.freqEnvFlags, pitchEnvs);
				mptIns->nFadeOut = (sampleHeader.fadeout + 1u) / 2u;
#ifdef MODPLUG_TRACKER
				if((mptIns->VolEnv.dwFlags & (ENV_ENABLED | ENV_LOOP)) == ENV_ENABLED)
				{
					// Fade-out is only supposed to happen on key-off, not at the end of a volume envelope.
					// Fake it by putting a loop at the end.
					mptIns->VolEnv.nLoopStart = mptIns->VolEnv.nLoopEnd = static_cast<uint8>(mptIns->VolEnv.size() - 1);
					mptIns->VolEnv.dwFlags.set(ENV_LOOP);
				}
				for(auto &p : mptIns->PitchEnv)
				{
					// Scale pitch envelope
					p.value = (p.value * 6u) / 16u;
				}
#endif // MODPLUG_TRACKER

				// Samples were already initialized above. Let's hope they are not going to be re-used with different volume / panning / vibrato...
				ModSample &mptSmp = Samples[sampleHeader.smpNum];

				// This flag literally enables and disables the default volume of a sample. If you disable this flag,
				// the sample volume of a previously sample is re-used, even if you put an instrument number next to the note.
				if(sampleHeader.volEnvFlags & 0x40)
					mptSmp.nVolume = sampleHeader.volume;
				else
					mptSmp.uFlags.set(SMP_NODEFAULTVOLUME);
				mptSmp.nPan = std::min(static_cast<uint16>(sampleHeader.panning * 2), uint16(254));
				mptSmp.nVibType = MDLVibratoType[sampleHeader.vibType & 3];
				mptSmp.nVibSweep = sampleHeader.vibSweep;
				mptSmp.nVibDepth = static_cast<uint8>((sampleHeader.vibDepth + 3u) / 4u);
				mptSmp.nVibRate = sampleHeader.vibSpeed;
				// Convert to IT-like vibrato sweep
				if(mptSmp.nVibSweep != 0)
					mptSmp.nVibSweep = mpt::saturate_cast<decltype(mptSmp.nVibSweep)>(Util::muldivr_unsigned(mptSmp.nVibDepth, 256, mptSmp.nVibSweep));
				else
					mptSmp.nVibSweep = 255;
				if(sampleHeader.panEnvFlags & 0x40)
					mptSmp.uFlags.set(CHN_PANNING);
			}
		}
	}

	// Read pattern tracks
	std::vector<FileReader> tracks;
	if((loadFlags & loadPatternData) && (chunk = chunks.GetChunk(MDLChunk::idTracks)).IsValid())
	{
		uint32 numTracks = chunk.ReadUint16LE();
		tracks.resize(numTracks + 1);
		for(uint32 i = 1; i <= numTracks; i++)
		{
			tracks[i] = chunk.ReadChunk(chunk.ReadUint16LE());
		}
	}

	// Read actual patterns
	if((loadFlags & loadPatternData) && (chunk = chunks.GetChunk(MDLChunk::idPats)).IsValid())
	{
		PATTERNINDEX numPats = chunk.ReadUint8();
		Patterns.ResizeArray(numPats);
		for(PATTERNINDEX pat = 0; pat < numPats; pat++)
		{
			CHANNELINDEX numChans = 32;
			ROWINDEX numRows = 64;
			std::string name;
			if(fileHeader.version >= 0x10)
			{
				MDLPatternHeader patHead;
				chunk.ReadStruct(patHead);
				numChans = patHead.channels;
				numRows = patHead.lastRow + 1;
				name = mpt::String::ReadBuf(mpt::String::spacePadded, patHead.name);
			}

			if(!Patterns.Insert(pat, numRows))
			{
				chunk.Skip(2 * numChans);
				continue;
			}
			Patterns[pat].SetName(name);

			for(CHANNELINDEX chn = 0; chn < numChans; chn++)
			{
				uint16 trkNum = chunk.ReadUint16LE();
				if(!trkNum || trkNum >= tracks.size() || chn >= GetNumChannels())
					continue;

				FileReader &track = tracks[trkNum];
				track.Rewind();
				ROWINDEX row = 0;
				while(row < numRows && track.CanRead(1))
				{
					ModCommand *m = Patterns[pat].GetpModCommand(row, chn);
					uint8 b = track.ReadUint8();
					uint8 x = (b >> 2), y = (b & 3);
					switch(y)
					{
					case 0:
						// (x + 1) empty notes follow
						row += x + 1;
						break;
					case 1:
						// Repeat previous note (x + 1) times
						if(row > 0)
						{
							ModCommand &orig = *Patterns[pat].GetpModCommand(row - 1, chn);
							do
							{
								*m = orig;
								m += GetNumChannels();
								row++;
							} while (row < numRows && x--);
						}
						break;
					case 2:
						// Copy note from row x
						if(row > x)
						{
							*m = *Patterns[pat].GetpModCommand(x, chn);
						}
						row++;
						break;
					case 3:
						// New note data
						if(x & MDLNOTE_NOTE)
						{
							b = track.ReadUint8();
							m->note = (b > 120) ? static_cast<ModCommand::NOTE>(NOTE_KEYOFF) : static_cast<ModCommand::NOTE>(b);
						}
						if(x & MDLNOTE_SAMPLE)
						{
							m->instr = track.ReadUint8();
						}
						{
							uint8 vol = 0, e1 = 0, e2 = 0, p1 = 0, p2 = 0;
							if(x & MDLNOTE_VOLUME)
							{
								vol = track.ReadUint8();
							}
							if(x & MDLNOTE_EFFECTS)
							{
								b = track.ReadUint8();
								e1 = (b & 0x0F);
								e2 = (b >> 4);
							}
							if(x & MDLNOTE_PARAM1)
								p1 = track.ReadUint8();
							if(x & MDLNOTE_PARAM2)
								p2 = track.ReadUint8();
							ImportMDLCommands(*m, vol, e1, e2, p1, p2);
						}

						row++;
						break;
					}
				}
			}
		}
	}

	if((loadFlags & loadPatternData) && (chunk = chunks.GetChunk(MDLChunk::idPatNames)).IsValid())
	{
		PATTERNINDEX i = 0;
		while(i < Patterns.Size() && chunk.CanRead(16))
		{
			char name[17];
			chunk.ReadString<mpt::String::spacePadded>(name, 16);
			Patterns[i].SetName(name);
		}
	}

	return true;
}


OPENMPT_NAMESPACE_END
