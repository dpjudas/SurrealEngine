/*
 * Load_imf.cpp
 * ------------
 * Purpose: IMF (Imago Orpheus) module loader
 * Notes  : Reverb and Chorus are not supported.
 * Authors: Storlek (Original author - http://schismtracker.org/ - code ported with permission)
 *          Johannes Schultz (OpenMPT Port, tweaks)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct IMFChannel
{
	char  name[12];  // Channel name (ASCIIZ-String, max 11 chars)
	uint8 chorus;    // Default chorus
	uint8 reverb;    // Default reverb
	uint8 panning;   // Pan positions 00-FF
	uint8 status;    // Channel status: 0 = enabled, 1 = mute, 2 = disabled (ignore effects!)
};

MPT_BINARY_STRUCT(IMFChannel, 16)

struct IMFFileHeader
{
	enum SongFlags
	{
		linearSlides = 0x01,
	};

	char     title[32];  // Songname (ASCIIZ-String, max. 31 chars)
	uint16le ordNum;     // Number of orders saved
	uint16le patNum;     // Number of patterns saved
	uint16le insNum;     // Number of instruments saved
	uint16le flags;      // See SongFlags
	uint8le  unused1[8];
	uint8le  tempo;   // Default tempo (Axx, 1...255)
	uint8le  bpm;     // Default beats per minute (BPM) (Txx, 32...255)
	uint8le  master;  // Default master volume (Vxx, 0...64)
	uint8le  amp;     // Amplification factor (mixing volume, 4...127)
	uint8le  unused2[8];
	char     im10[4];         // 'IM10'
	IMFChannel channels[32];  // Channel settings

	CHANNELINDEX GetNumChannels() const
	{
		uint8 detectedChannels = 0;
		for(uint8 chn = 0; chn < 32; chn++)
		{
			if(channels[chn].status < 2)
				detectedChannels = chn + 1;
			else if(channels[chn].status > 2)
				return 0;
		}
		return detectedChannels;
	}
};

MPT_BINARY_STRUCT(IMFFileHeader, 576)

struct IMFEnvelope
{
	enum EnvFlags
	{
		envEnabled = 0x01,
		envSustain = 0x02,
		envLoop    = 0x04,
	};

	uint8 points;     // Number of envelope points
	uint8 sustain;    // Envelope sustain point
	uint8 loopStart;  // Envelope loop start point
	uint8 loopEnd;    // Envelope loop end point
	uint8 flags;      // See EnvFlags
	uint8 unused[3];
};

MPT_BINARY_STRUCT(IMFEnvelope, 8)

struct IMFEnvNode
{
	uint16le tick;
	uint16le value;
};

MPT_BINARY_STRUCT(IMFEnvNode, 4)

struct IMFInstrument
{
	enum EnvTypes
	{
		volEnv    = 0,
		panEnv    = 1,
		filterEnv = 2,
	};

	char        name[32];  // Inst. name (ASCIIZ-String, max. 31 chars)
	uint8le     map[120];  // Multisample settings
	uint8le     unused[8];
	IMFEnvNode  nodes[3][16];
	IMFEnvelope env[3];
	uint16le    fadeout;  // Fadeout rate (0...0FFFH)
	uint16le    smpNum;   // Number of samples in instrument
	char        ii10[4];  // 'II10' (not verified by Orpheus)

	void ConvertEnvelope(InstrumentEnvelope &mptEnv, EnvTypes e) const
	{
		const uint8 shift = (e == volEnv) ? 0 : 2;
		const uint8 mirror = (e == filterEnv) ? 0xFF : 0x00;

		mptEnv.dwFlags.set(ENV_ENABLED, (env[e].flags & 1) != 0);
		mptEnv.dwFlags.set(ENV_SUSTAIN, (env[e].flags & 2) != 0);
		mptEnv.dwFlags.set(ENV_LOOP, (env[e].flags & 4) != 0);

		mptEnv.resize(Clamp(env[e].points, uint8(2), uint8(16)));
		mptEnv.nLoopStart = env[e].loopStart;
		mptEnv.nLoopEnd = env[e].loopEnd;
		mptEnv.nSustainStart = mptEnv.nSustainEnd = env[e].sustain;

		uint16 minTick = 0; // minimum tick value for next node
		for(uint32 n = 0; n < mptEnv.size(); n++)
		{
			mptEnv[n].tick = minTick = std::max(minTick, nodes[e][n].tick.get());
			minTick++;
			uint8 value = static_cast<uint8>(nodes[e][n].value ^ mirror) >> shift;
			mptEnv[n].value = std::min(value, uint8(ENVELOPE_MAX));
		}
		mptEnv.Convert(MOD_TYPE_XM, MOD_TYPE_IT);
	}

	// Convert an IMFInstrument to OpenMPT's internal instrument representation.
	void ConvertToMPT(ModInstrument &mptIns, SAMPLEINDEX firstSample) const
	{
		mptIns.name = mpt::String::ReadBuf(mpt::String::nullTerminated, name);

		if(smpNum)
		{
			for(size_t note = 0; note < std::min(std::size(map), std::size(mptIns.Keyboard) - 12u); note++)
			{
				mptIns.Keyboard[note + 12] = firstSample + map[note];
			}
		}

		mptIns.nFadeOut = fadeout;
		mptIns.midiPWD = 1;  // For CMD_FINETUNE

		ConvertEnvelope(mptIns.VolEnv, volEnv);
		ConvertEnvelope(mptIns.PanEnv, panEnv);
		ConvertEnvelope(mptIns.PitchEnv, filterEnv);
		if(mptIns.PitchEnv.dwFlags[ENV_ENABLED])
			mptIns.PitchEnv.dwFlags.set(ENV_FILTER);

		// hack to get === to stop notes
		if(!mptIns.VolEnv.dwFlags[ENV_ENABLED] && !mptIns.nFadeOut)
			mptIns.nFadeOut = 32767;
	}
};

MPT_BINARY_STRUCT(IMFInstrument, 384)

struct IMFSample
{
	enum SampleFlags
	{
		smpLoop			= 0x01,
		smpPingPongLoop	= 0x02,
		smp16Bit		= 0x04,
		smpPanning		= 0x08,
	};

	char     filename[13];  // Sample filename (12345678.ABC) */
	uint8le  unused1[3];
	uint32le length;        // Length (in bytes)
	uint32le loopStart;     // Loop start (in bytes)
	uint32le loopEnd;       // Loop end (in bytes)
	uint32le c5Speed;       // Samplerate
	uint8le  volume;        // Default volume (0...64)
	uint8le  panning;       // Default pan (0...255)
	uint8le  unused2[14];
	uint8le  flags;  // Sample flags
	uint8le  unused3[5];
	uint16le ems;      // Reserved for internal usage
	uint32le dram;     // Reserved for internal usage
	char     is10[4];  // 'IS10' or 'IW10' (not verified by Orpheus)

	// Convert an IMFSample to OpenMPT's internal sample representation.
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize(MOD_TYPE_IMF);
		mptSmp.filename = mpt::String::ReadBuf(mpt::String::nullTerminated, filename);

		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		mptSmp.nC5Speed = c5Speed;
		mptSmp.nVolume = volume * 4;
		mptSmp.nPan = panning;
		if(flags & smpLoop)
			mptSmp.uFlags.set(CHN_LOOP);
		if(flags & smpPingPongLoop)
			mptSmp.uFlags.set(CHN_PINGPONGLOOP);
		if(flags & smp16Bit)
		{
			mptSmp.uFlags.set(CHN_16BIT);
			mptSmp.nLength /= 2;
			mptSmp.nLoopStart /= 2;
			mptSmp.nLoopEnd /= 2;
		}
		if(flags & smpPanning)
			mptSmp.uFlags.set(CHN_PANNING);
	}
};

MPT_BINARY_STRUCT(IMFSample, 64)


static constexpr EffectCommand imfEffects[] =
{
	CMD_NONE,
	CMD_SPEED,           // 0x01 1xx Set Tempo
	CMD_TEMPO,           // 0x02 2xx Set BPM
	CMD_TONEPORTAMENTO,  // 0x03 3xx Tone Portamento
	CMD_TONEPORTAVOL,    // 0x04 4xy Tone Portamento + Volume Slide
	CMD_VIBRATO,         // 0x05 5xy Vibrato
	CMD_VIBRATOVOL,      // 0x06 6xy Vibrato + Volume Slide
	CMD_FINEVIBRATO,     // 0x07 7xy Fine Vibrato
	CMD_TREMOLO,         // 0x08 8xy Tremolo
	CMD_ARPEGGIO,        // 0x09 9xy Arpeggio
	CMD_PANNING8,        // 0x0A Axx Set Pan Position
	CMD_PANNINGSLIDE,    // 0x0B Bxy Pan Slide
	CMD_VOLUME,          // 0x0C Cxx Set Volume
	CMD_VOLUMESLIDE,     // 0x0D Dxy Volume Slide
	CMD_VOLUMESLIDE,     // 0x0E Exy Fine Volume Slide
	CMD_FINETUNE,        // 0x0F Fxx Set Finetune
	CMD_NOTESLIDEUP,     // 0x10 Gxy Note Slide Up
	CMD_NOTESLIDEDOWN,   // 0x11 Hxy Note Slide Down
	CMD_PORTAMENTOUP,    // 0x12 Ixx Slide Up
	CMD_PORTAMENTODOWN,  // 0x13 Jxx Slide Down
	CMD_PORTAMENTOUP,    // 0x14 Kxx Fine Slide Up
	CMD_PORTAMENTODOWN,  // 0x15 Lxx Fine Slide Down
	CMD_MIDI,            // 0x16 Mxx Set Filter Cutoff
	CMD_MIDI,            // 0x17 Nxy Filter Slide + Resonance
	CMD_OFFSET,          // 0x18 Oxx Set Sample Offset
	CMD_NONE,            // 0x19 Pxx Set Fine Sample Offset - XXX
	CMD_KEYOFF,          // 0x1A Qxx Key Off
	CMD_RETRIG,          // 0x1B Rxy Retrig
	CMD_TREMOR,          // 0x1C Sxy Tremor
	CMD_POSITIONJUMP,    // 0x1D Txx Position Jump
	CMD_PATTERNBREAK,    // 0x1E Uxx Pattern Break
	CMD_GLOBALVOLUME,    // 0x1F Vxx Set Mastervolume
	CMD_GLOBALVOLSLIDE,  // 0x20 Wxy Mastervolume Slide
	CMD_S3MCMDEX,        // 0x21 Xxx Extended Effect
	                     //      X1x Set Filter
	                     //      X3x Glissando
	                     //      X5x Vibrato Waveform
	                     //      X8x Tremolo Waveform
	                     //      XAx Pattern Loop
	                     //      XBx Pattern Delay
	                     //      XCx Note Cut
	                     //      XDx Note Delay
	                     //      XEx Ignore Envelope
	                     //      XFx Invert Loop
	CMD_NONE,            // 0x22 Yxx Chorus - XXX
	CMD_NONE,            // 0x23 Zxx Reverb - XXX
};

static std::pair<EffectCommand, uint8> TranslateIMFEffect(uint8 command, uint8 param)
{
	uint8 n;
	// fix some of them
	switch(command)
	{
	case 0xE: // fine volslide
		// hackaround to get almost-right behavior for fine slides (i think!)
		if(param == 0)
			/* nothing */;
		else if(param == 0xF0)
			param = 0xEF;
		else if(param == 0x0F)
			param = 0xFE;
		else if(param & 0xF0)
			param |= 0x0F;
		else
			param |= 0xF0;
		break;
	case 0xF: // set finetune
		param ^= 0x80;
		break;
	case 0x14: // fine slide up
	case 0x15: // fine slide down
		// this is about as close as we can do...
		if(param >> 4)
			param = 0xF0 | (param >> 4);
		else
			param |= 0xE0;
		break;
	case 0x16: // cutoff
		param = static_cast<uint8>((0xFF - param) / 2u);
		break;
	case 0x17: // cutoff slide + resonance (TODO: cutoff slide is currently not handled)
		param = 0x80 | (param & 0x0F);
		break;
	case 0x1F: // set global volume
		param = mpt::saturate_cast<uint8>(param * 2);
		break;
	case 0x21:
		n = 0;
		switch(param >> 4)
		{
		case 0:
			/* undefined, but since S0x does nothing in IT anyway, we won't care.
			this is here to allow S00 to pick up the previous value (assuming IMF
			even does that -- I haven't actually tried it) */
			break;
		default: // undefined
		case 0x1: // set filter
		case 0xF: // invert loop
			command = 0;
			break;
		case 0x3: // glissando
			n = 0x20;
			break;
		case 0x5: // vibrato waveform
			n = 0x30;
			break;
		case 0x8: // tremolo waveform
			n = 0x40;
			break;
		case 0xA: // pattern loop
			n = 0xB0;
			break;
		case 0xB: // pattern delay
			n = 0xE0;
			break;
		case 0xC: // note cut
		case 0xD: // note delay
			// Apparently, Imago Orpheus doesn't cut samples on tick 0.
			if(!param)
				command = 0;
			break;
		case 0xE: // ignore envelope
			switch(param & 0x0F)
			{
			// All envelopes
			// Predicament: we can only disable one envelope at a time. Volume is probably most noticeable, so let's go with that.
			case 0: param = 0x77; break;
			// Volume
			case 1: param = 0x77; break;
			// Panning
			case 2: param = 0x79; break;
			// Filter
			case 3: param = 0x7B; break;
			}
			break;
		case 0x18: // sample offset
			// O00 doesn't pick up the previous value
			if(!param)
				command = 0;
			break;
		}
		if(n)
			param = n | (param & 0x0F);
		break;
	}
	return {(command < std::size(imfEffects)) ? imfEffects[command] : CMD_NONE, param};
}


static bool ValidateHeader(const IMFFileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.im10, "IM10", 4)
	   || fileHeader.ordNum > 256
	   || fileHeader.insNum >= MAX_INSTRUMENTS
	   || fileHeader.bpm < 32
	   || fileHeader.master > 64
	   || fileHeader.amp < 4
	   || fileHeader.amp > 127
	   || !fileHeader.GetNumChannels())
	{
		return false;
	}
	return true;
}


static uint64 GetHeaderMinimumAdditionalSize(const IMFFileHeader &fileHeader)
{
	return 256 + fileHeader.patNum * 4 + fileHeader.insNum * sizeof(IMFInstrument);
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderIMF(MemoryFileReader file, const uint64 *pfilesize)
{
	IMFFileHeader fileHeader;
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


bool CSoundFile::ReadIMF(FileReader &file, ModLoadingFlags loadFlags)
{
	IMFFileHeader fileHeader;
	file.Rewind();
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

	InitializeGlobals(MOD_TYPE_IMF, fileHeader.GetNumChannels());

	m_modFormat.formatName = UL_("Imago Orpheus");
	m_modFormat.type = UL_("imf");
	m_modFormat.charset = mpt::Charset::CP437;

	// Read channel configuration
	std::bitset<32> ignoreChannels;  // bit set for each channel that's completely disabled
	uint64 channelMuteStatus = 0;
	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].nPan = static_cast<uint16>(fileHeader.channels[chn].panning * 256 / 255);
		ChnSettings[chn].szName = mpt::String::ReadBuf(mpt::String::nullTerminated, fileHeader.channels[chn].name);
		channelMuteStatus |= static_cast<uint64>(fileHeader.channels[chn].status) << (chn * 2);
		// TODO: reverb/chorus?
		switch(fileHeader.channels[chn].status)
		{
		case 0:  // enabled; don't worry about it
			break;
		case 1:  // mute
			ChnSettings[chn].dwFlags = CHN_MUTE;
			break;
		case 2:  // disabled
			ChnSettings[chn].dwFlags = CHN_MUTE;
			ignoreChannels[chn] = true;
			break;
		}
	}
	// BEHIND.IMF: All channels but the first are muted
	// mikmod refers to this as an Orpheus bug, but I haven't seen any other files like this, so maybe it's just an incorrectly saved file?
	if(GetNumChannels() == 16 && channelMuteStatus == 0x5555'5554)
	{
		for(CHANNELINDEX chn = 1; chn < GetNumChannels(); chn++)
			ChnSettings[chn].dwFlags.reset(CHN_MUTE);
	}

	// Song Name
	m_songName = mpt::String::ReadBuf(mpt::String::nullTerminated, fileHeader.title);

	m_SongFlags.set(SONG_LINEARSLIDES, fileHeader.flags & IMFFileHeader::linearSlides);
	Order().SetDefaultSpeed(fileHeader.tempo);
	Order().SetDefaultTempoInt(fileHeader.bpm);
	m_nDefaultGlobalVolume = fileHeader.master * 4u;
	m_nSamplePreAmp = fileHeader.amp;

	m_nInstruments = fileHeader.insNum;
	m_nSamples = 0; // Will be incremented later

	uint8 orders[256];
	file.ReadArray(orders);
	ReadOrderFromArray(Order(), orders, fileHeader.ordNum, uint16_max, 0xFF);

	// Read patterns
	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(fileHeader.patNum);
	for(PATTERNINDEX pat = 0; pat < fileHeader.patNum; pat++)
	{
		const uint16 length = file.ReadUint16LE(), numRows = file.ReadUint16LE();
		FileReader patternChunk = file.ReadChunk(length - 4);

		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, numRows))
		{
			continue;
		}

		ModCommand dummy;
		ROWINDEX row = 0;
		while(row < numRows)
		{
			uint8 mask = patternChunk.ReadUint8();
			if(mask == 0)
			{
				row++;
				continue;
			}

			uint8 channel = mask & 0x1F;
			ModCommand &m = (channel < GetNumChannels()) ? *Patterns[pat].GetpModCommand(row, channel) : dummy;

			if(mask & 0x20)
			{
				// Read note/instrument
				const auto [note, instr] = patternChunk.ReadArray<uint8, 2>();
				m.note = note;
				m.instr = instr;

				if(m.note == 160)
				{
					m.note = NOTE_KEYOFF;
				} else if(m.note == 255)
				{
					m.note = NOTE_NONE;
				} else
				{
					m.note = static_cast<ModCommand::NOTE>((m.note >> 4) * 12 + (m.note & 0x0F) + 12 + 1);
					if(!m.IsNoteOrEmpty())
					{
						m.note = NOTE_NONE;
					}
				}
			}
			if((mask & 0xC0) == 0xC0)
			{
				// Read both effects and figure out what to do with them
				const auto [e1c, e1d, e2c, e2d] = patternChunk.ReadArray<uint8, 4>();  // Command 1, Data 1, Command 2, Data 2
				const auto [command1, param1] = TranslateIMFEffect(e1c, e1d);
				const auto [command2, param2] = TranslateIMFEffect(e2c, e2d);
				m.FillInTwoCommands(command1, param1, command2, param2, true);
			} else if(mask & 0xC0)
			{
				// There's one effect, just stick it in the effect column (unless it's a volume command)
				const auto [e1c, e1d] = patternChunk.ReadArray<uint8, 2>();  // Command 1, Data 1, Command 2, Data 2
				const auto [command, param] = TranslateIMFEffect(e1c, e1d);
				if(command == CMD_VOLUME)
					m.SetVolumeCommand(VOLCMD_VOLUME, param);
				else
					m.SetEffectCommand(command, param);
			}
			if(ignoreChannels[channel] && m.IsGlobalCommand())
				m.command = CMD_NONE;
		}
	}

	SAMPLEINDEX firstSample = 1; // first sample index of the current instrument

	// read instruments
	for(INSTRUMENTINDEX ins = 0; ins < GetNumInstruments(); ins++)
	{
		ModInstrument *instr = AllocateInstrument(ins + 1);
		IMFInstrument instrumentHeader;
		if(!file.ReadStruct(instrumentHeader) || instr == nullptr)
		{
			continue;
		}

		// Orpheus does not check this!
		//if(memcmp(instrumentHeader.ii10, "II10", 4) != 0)
		//	return false;
		instrumentHeader.ConvertToMPT(*instr, firstSample);

		// Read this instrument's samples
		for(SAMPLEINDEX smp = 0; smp < instrumentHeader.smpNum; smp++)
		{
			IMFSample sampleHeader;
			file.ReadStruct(sampleHeader);

			const SAMPLEINDEX smpID = firstSample + smp;
			if(smpID >= MAX_SAMPLES)
			{
				file.Skip(sampleHeader.length);
				continue;
			}

			m_nSamples = smpID;
			ModSample &sample = Samples[smpID];

			sampleHeader.ConvertToMPT(sample);
			m_szNames[smpID] = sample.filename;

			if(sampleHeader.length)
			{
				FileReader sampleChunk = file.ReadChunk(sampleHeader.length);
				if(loadFlags & loadSampleData)
				{
					SampleIO(
						sample.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
						SampleIO::mono,
						SampleIO::littleEndian,
						SampleIO::signedPCM)
						.ReadSample(sample, sampleChunk);
				}
			}
		}
		firstSample += instrumentHeader.smpNum;
	}

	return true;
}


OPENMPT_NAMESPACE_END
