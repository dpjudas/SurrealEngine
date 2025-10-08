/*
 * Load_rtm.cpp
 * ------------
 * Purpose: Real Tracker 2 (RTM) module Loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct RTMObjectHeader
{
	char     id[4];  // "RTMM" for song header, "RTND" for patterns, "RTIN" for instruments, "RTSM" for samples
	uint8    space;
	char     name[32];
	uint8    eof;
	uint16le version;
	uint16le objectSize;

	bool IsValid() const
	{
		return !memcmp(id, "RTMM", 4)
			&& space == 0x20
			&& eof == 0x1A
			&& version >= 0x100 && version <= 0x112
			&& objectSize >= 98;
	}
};

MPT_BINARY_STRUCT(RTMObjectHeader, 42)


struct RTMMHeader
{
	enum SongFlags
	{
		songLinearSlides = 0x01,
		songTrackNames   = 0x02,
	};

	char     software[20];
	char     composer[32];
	uint16le flags;
	uint8    numChannels;
	uint8    numInstruments;
	uint16le numOrders;
	uint16le numPatterns;
	uint8    speed;
	uint8    tempo;
	int8     panning[32];
	uint32le extraDataSize;  // Order list / track names
	char     originalName[32];

	bool IsValid() const
	{
		return numChannels > 0 && numChannels <= 32
			&& numOrders <= 999
			&& numPatterns <= 999
			&& speed != 0
			&& extraDataSize < 0x10000;
	}

	static constexpr size_t HeaderProbingSize()
	{
		return offsetof(RTMMHeader, originalName);
	}

	uint32 GetHeaderMinimumAdditionalSize() const
	{
		return extraDataSize;
	}
};

MPT_BINARY_STRUCT(RTMMHeader, 130)


// Pattern header (RTND magic bytes)
struct RTMPatternHeader
{
	uint16le flags;  // Always 1
	uint8    numTracks;
	uint16le numRows;
	uint32le packedSize;
};

MPT_BINARY_STRUCT(RTMPatternHeader, 9)


struct RTMEnvelope
{
	enum EnvelopeFlags
	{
		envEnabled = 0x01,
		envSustain = 0x02,
		envLoop    = 0x04,
	};

	struct EnvPoint
	{
		uint32le x;
		int32le y;
	};

	uint8    numPoints;
	EnvPoint points[12];
	uint8    sustainPoint;
	uint8    loopStart;
	uint8    loopEnd;
	uint16le flags;

	void ConvertToMPT(InstrumentEnvelope &mptEnv, int8 offset) const
	{
		mptEnv.resize(std::min(numPoints, uint8(12)));
		mptEnv.nSustainStart = mptEnv.nSustainEnd = sustainPoint;
		mptEnv.nLoopStart = loopStart;
		mptEnv.nLoopEnd = loopEnd;
		mptEnv.dwFlags.set(ENV_ENABLED, flags & envEnabled);
		mptEnv.dwFlags.set(ENV_SUSTAIN, flags & envSustain);
		mptEnv.dwFlags.set(ENV_LOOP, flags & envLoop);

		for(size_t i = 0; i < mptEnv.size(); i++)
		{
			mptEnv[i].tick = mpt::saturate_cast<EnvelopeNode::tick_t>(points[i].x.get());
			mptEnv[i].value = mpt::saturate_cast<EnvelopeNode::value_t>(points[i].y + offset);
		}
	}
};

MPT_BINARY_STRUCT(RTMEnvelope, 102)


// Instrument header (RTIN magic bytes)
struct RTINHeader
{
	enum InstrumentFlags
	{
		insDefaultPanning = 0x01,
		insMuteSamples    = 0x02,
	};

	uint8    numSamples;
	uint16le flags;
	uint8    samples[120];

	RTMEnvelope volumeEnv;
	RTMEnvelope panningEnv;

	uint8    vibratoType;
	uint8    vibratoSweep;
	uint8    vibratoDepth;
	uint8    vibratoRate;
	uint16le fadeOut;
	uint8    midiPort;
	uint8    midiChannel;
	uint8    midiProgram;
	uint8    midiEnable;
	int8     midiTranspose;
	uint8    midiBenderRange;
	uint8    midiBaseVolume;
	uint8    midiUseVelocity;

	void ConvertToMPT(ModInstrument &mptIns, SAMPLEINDEX baseSample) const
	{
		mptIns.nFadeOut = fadeOut / 2;
		volumeEnv.ConvertToMPT(mptIns.VolEnv, 0);
		panningEnv.ConvertToMPT(mptIns.PanEnv, ENVELOPE_MID);
		if(flags & insMuteSamples)
			mptIns.nGlobalVol = 0;
		static_assert(mpt::array_size<decltype(samples)>::size <= mpt::array_size<decltype(mptIns.Keyboard)>::size);
		for(size_t i = 0; i < std::size(samples); i++)
		{
			mptIns.Keyboard[i] = baseSample + samples[i];
		}
		if(midiEnable)
		{
			mptIns.nMidiChannel = MidiFirstChannel + midiChannel;
			mptIns.nMidiProgram = midiProgram + 1;
			mptIns.midiPWD = midiBenderRange;
		}
	}
};

MPT_BINARY_STRUCT(RTINHeader, 341)


// Sample header (RTSM magic bytes)
struct RTSMHeader
{
	enum SampleFlags
	{
		smp16Bit = 0x02,
		smpDelta = 0x04,
	};

	uint16le flags;
	uint8    baseVolume;     // 0...64
	uint8    defaultVolume;  // 0...64
	uint32le length;
	uint8    loopType;  // 0 = no loop, 1 = forward loop, 2 = ping-pong loop
	char     reserved[3];
	uint32le loopStart;
	uint32le loopEnd;
	uint32le sampleRate;
	uint8    baseNote;
	int8     panning;  // -64...64

	void ConvertToMPT(ModSample &mptSmp, const RTINHeader &insHeader) const
	{
		mptSmp.Initialize(MOD_TYPE_IT);
		mptSmp.nVolume = defaultVolume * 4;
		mptSmp.nGlobalVol = baseVolume;
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		if(flags & smp16Bit)
		{
			mptSmp.nLength /= 2;
			mptSmp.nLoopStart /= 2;
			mptSmp.nLoopEnd /= 2;
		}
		mptSmp.uFlags.set(CHN_PANNING, insHeader.flags & RTINHeader::insDefaultPanning);
		mptSmp.uFlags.set(CHN_LOOP, loopType != 0);
		mptSmp.uFlags.set(CHN_SUSTAINLOOP, loopType == 2);
		mptSmp.nC5Speed = sampleRate;
		mptSmp.Transpose((48 - baseNote) / 12.0);
		mptSmp.nPan = static_cast<uint16>((panning + 64) * 2);

		mptSmp.nVibType = static_cast<VibratoType>(insHeader.vibratoType);
		mptSmp.nVibDepth = insHeader.vibratoDepth * 2;
		mptSmp.nVibRate = insHeader.vibratoRate / 2;
		mptSmp.nVibSweep = insHeader.vibratoSweep;
		if(mptSmp.nVibSweep != 0)
			mptSmp.nVibSweep = mpt::saturate_cast<decltype(mptSmp.nVibSweep)>(Util::muldivr_unsigned(mptSmp.nVibDepth, 256, mptSmp.nVibSweep));
		else
			mptSmp.nVibSweep = 255;
	}

	SampleIO GetSampleFormat() const
	{
		return SampleIO{(flags & smp16Bit) ? SampleIO::_16bit : SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::littleEndian,
			(flags & smpDelta) ? SampleIO::deltaPCM: SampleIO::signedPCM};
	}
};

MPT_BINARY_STRUCT(RTSMHeader, 26)


static void ConvertRTMEffect(ModCommand &m, const uint8 command, const uint8 param, const CSoundFile &sndFile)
{
	// Commands not handled: M (Select MIDI controller), V (Select MIDI controller value)
	if(command == 8)
		m.SetEffectCommand(CMD_PANNING8, mpt::saturate_cast<ModCommand::PARAM>(param * 2));
	else if(command == 'S' - 55 && (param & 0xF0) == 0xA0)
		m.SetEffectCommand(CMD_S3MCMDEX, param);
	else if(command <= 'X' - 55)
	{
		CSoundFile::ConvertModCommand(m, command, param);
		m.Convert(MOD_TYPE_XM, MOD_TYPE_IT, sndFile);
	} else if(command == 36)
		m.SetEffectCommand(CMD_VOLUMESLIDE, param);
	else if(command == 37)
		m.SetEffectCommand(CMD_PORTAMENTOUP, param);
	else if(command == 38)
		m.SetEffectCommand(CMD_PORTAMENTODOWN, param);
	else if(command == 39)
		m.SetEffectCommand(CMD_VIBRATOVOL, param);
	else if(command == 40)
		m.SetEffectCommand(CMD_SPEED, param);
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderRTM(MemoryFileReader file, const uint64* pfilesize)
{
	RTMObjectHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
		return ProbeFailure;
	RTMMHeader songHeader;
	if(file.ReadStructPartial(songHeader, RTMMHeader::HeaderProbingSize()) < RTMMHeader::HeaderProbingSize())
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.objectSize - RTMMHeader::HeaderProbingSize() + songHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadRTM(FileReader& file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	RTMObjectHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return false;
	if(!fileHeader.IsValid())
		return false;
	RTMMHeader songHeader;
	if(file.ReadStructPartial(songHeader, fileHeader.objectSize) < fileHeader.objectSize)
		return false;
	if(!songHeader.IsValid())
		return false;
	if(!file.CanRead(songHeader.GetHeaderMinimumAdditionalSize()))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_IT, songHeader.numChannels);
	m_nInstruments = std::min(static_cast<INSTRUMENTINDEX>(songHeader.numInstruments), static_cast<INSTRUMENTINDEX>(MAX_INSTRUMENTS - 1));
	m_SongFlags = SONG_ITCOMPATGXX | SONG_ITOLDEFFECTS;
	m_SongFlags.set(SONG_LINEARSLIDES, songHeader.flags & RTMMHeader::songLinearSlides);
	Order().SetDefaultTempoInt(songHeader.tempo);
	Order().SetDefaultSpeed(songHeader.speed);

	m_songArtist = mpt::ToUnicode(mpt::Charset::CP437, mpt::String::ReadBuf(mpt::String::maybeNullTerminated, songHeader.composer));
	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.name);
	if(fileHeader.version >= 0x112)
	{
		if(m_songName.empty())
			m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, songHeader.originalName);
		else
			m_songMessage.SetRaw(mpt::String::ReadBuf(mpt::String::maybeNullTerminated, songHeader.originalName));
	}

	FileReader extraData = file.ReadChunk(songHeader.extraDataSize);
	ReadOrderFromFile<uint16le>(Order(), extraData, songHeader.numOrders);
	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].nPan = static_cast<uint16>((songHeader.panning[chn] + 64) * 2);
		if(songHeader.flags & RTMMHeader::songTrackNames)
			extraData.ReadString<mpt::String::maybeNullTerminated>(ChnSettings[chn].szName, 16);
	}

	m_modFormat.formatName = MPT_UFORMAT("Real Tracker {}.{}")(fileHeader.version >> 8, mpt::ufmt::hex0<2>(fileHeader.version & 0xFF));
	m_modFormat.type = UL_("rtm");
	m_modFormat.madeWithTracker = mpt::ToUnicode(mpt::Charset::CP437, mpt::String::ReadBuf(mpt::String::maybeNullTerminated, songHeader.software));
	m_modFormat.charset = mpt::Charset::CP437;

	Patterns.ResizeArray(songHeader.numPatterns);
	for(PATTERNINDEX pat = 0; pat < songHeader.numPatterns; pat++)
	{
		RTMObjectHeader objectHeader;
		if(!file.ReadStruct(objectHeader))
			return false;
		RTMPatternHeader patHeader;
		file.ReadStructPartial(patHeader, objectHeader.objectSize);
		FileReader patternData = file.ReadChunk(patHeader.packedSize);
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, patHeader.numRows))
			continue;

		Patterns[pat].SetName(mpt::String::ReadBuf(mpt::String::maybeNullTerminated, objectHeader.name));
		ROWINDEX row = 0;
		CHANNELINDEX chn = 0;
		auto rowData = Patterns[pat].GetRow(0);
		while(row < patHeader.numRows && patternData.CanRead(1))
		{
			const uint8 b = patternData.ReadUint8();
			if(b == 0)
			{
				row++;
				chn = 0;
				if(row < patHeader.numRows)
					rowData = Patterns[pat].GetRow(row);
				continue;
			}

			if(b & 0x01)
				chn = patternData.ReadUint8();
			if(chn >= GetNumChannels())
				return false;

			ModCommand &m = rowData[chn];
			if(b & 0x02)
			{
				uint8 note = patternData.ReadUint8();
				if(note == 0xFE)
					m.note = NOTE_KEYOFF;
				else if(note < 120)
					m.note = note + NOTE_MIDDLEC - 48;
			}
			if(b & 0x04)
				m.instr = patternData.ReadUint8();

			uint8 cmd1 = 0, param1 = 0, cmd2 = 0, param2 = 0;
			if(b & 0x08)
				cmd1 = patternData.ReadUint8();
			if(b & 0x10)
				param1 = patternData.ReadUint8();
			if(b & 0x20)
				cmd2 = patternData.ReadUint8();
			if(b & 0x40)
				param2 = patternData.ReadUint8();

			if(cmd1 || param1)
				ConvertRTMEffect(m, cmd1, param1, *this);
			if(cmd2 || param2)
			{
				ModCommand dummy;
				ConvertRTMEffect(dummy, cmd2, param2, *this);
				m.FillInTwoCommands(m.command, m.param, dummy.command, dummy.param);
			}
			chn++;
		}
	}

	for(INSTRUMENTINDEX instr = 1; instr <= m_nInstruments; instr++)
	{
		RTMObjectHeader objectHeader;
		if(!file.ReadStruct(objectHeader))
			return false;
		RTINHeader insHeader;
		file.ReadStructPartial(insHeader, objectHeader.objectSize);
		if(!AllocateInstrument(instr))
			return false;
		insHeader.ConvertToMPT(*Instruments[instr], m_nSamples + 1);
		Instruments[instr]->name = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, objectHeader.name);
		for(SAMPLEINDEX smp = 0; smp < insHeader.numSamples; smp++)
		{
			RTMObjectHeader smpObjectHeader;
			if(!file.ReadStruct(smpObjectHeader))
				return false;
			RTSMHeader smpHeader;
			file.ReadStructPartial(smpHeader, smpObjectHeader.objectSize);
			FileReader sampleData = file.ReadChunk(smpHeader.length);
			if(!CanAddMoreSamples())
				continue;
			ModSample &mptSmp = Samples[++m_nSamples];
			smpHeader.ConvertToMPT(mptSmp, insHeader);
			m_szNames[m_nSamples] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, smpObjectHeader.name);
			if(loadFlags & loadSampleData)
				smpHeader.GetSampleFormat().ReadSample(mptSmp, sampleData);
		}
	}

	return true;
}

OPENMPT_NAMESPACE_END
