/*
 * Load_gt2.cpp
 * ------------
 * Purpose: Graoumf Tracker 1/2 module loader (GTK and GT2)
 * Notes  : DSPs (delay, filter) are currently not supported.
 *          They are kinda tricky because DSP to track assignments are established using pattern commands and can change at any time.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

#include "mpt/parse/parse.hpp"

OPENMPT_NAMESPACE_BEGIN

// GTK File Header
struct GTKFileHeader
{
	char     signature[3];
	uint8be  fileVersion;
	char     songName[32];
	char     smallComment[160];
	uint16be numSamples;
	uint16be numRows;
	uint16be numChannels;
	uint16be numOrders;
	uint16be restartPos;

	bool Validate() const
	{
		return !std::memcmp(signature, "GTK", 3)
			&& fileVersion >= 1
			&& fileVersion <= 4
			&& numSamples <= 255
			&& numRows > 0 && numRows <= 256
			&& numChannels > 0 && numChannels <= 32
			&& numOrders <= 256
			&& restartPos < numOrders;
	}

	uint64 GetHeaderMinimumAdditionalSize() const
	{
		return ((fileVersion < 3) ? 48 : 64) * numSamples
			+ 512                                                   // Order list
			+ ((fileVersion < 4) ? 4 : 5) * numRows * numChannels;  // One pattern minimum
	}
};

MPT_BINARY_STRUCT(GTKFileHeader, 206)


static uint16 GT2LogToLinearVolume(uint16 logVolume)
{
	LimitMax(logVolume, uint16(0xFFF));
	return mpt::saturate_round<uint16>(std::pow(2.0, (logVolume & 0xFF) / 256.0) * 32768.0) >> (15 - (logVolume >> 8));
}


static void TranslateGraoumfEffect(CSoundFile &sndFile, ModCommand &m, const uint8 effect, const uint8 param, const int16 fileVersion, const ModCommand::NOTE lastNote, const ModCommand::INSTR lastInstr, TEMPO &currentTempo, uint32 &currentSpeed)
{
	if(!effect)
		return;
	if(effect >= 0xB0 && fileVersion < 0)
		return;

	m.param = param;

	const bool isFalcon = fileVersion <= 5;
	const uint16 param12bit = ((effect & 0x0F) << 8) | param;
	const uint8 param4bit = std::min(param, uint8(0x0F));
	const uint8 param4bitSlide = std::min(param, uint8(0x0E));
	const uint8 param4bitDiv32 = static_cast<uint8>(std::min((param + 31) / 32, 0x0E));
	const uint8 param4bitDiv128 = static_cast<uint8>(std::min((param + 127) / 128, 0x0E));
	switch(effect >> 4)
	{
	case 0x02:  // 2xxx: Set Volume
		m.volcmd = VOLCMD_VOLUME;
		m.vol = static_cast<ModCommand::VOL>(std::min(param12bit / 4, 64));
		return;

	case 0x03:  // 3xxx: Set Logarithmic Volume
		m.volcmd = VOLCMD_VOLUME;
		if(param12bit == 0)
			m.vol = 0;
		else
			m.vol = static_cast<ModCommand::VOL>(std::min(GT2LogToLinearVolume(param12bit) / 4, 64));
		return;

	case 0x04:  // 4xxx: Panning
		m.command = CMD_PANNING8;
		m.param = static_cast<ModCommand::PARAM>(param12bit / 16);
		return;

	case 0x05:  // 5xxx: Linear global volume
		m.command = CMD_GLOBALVOLUME;
		m.param = static_cast<ModCommand::PARAM>((param12bit + 1) / 32);
		return;

	case 0x06:  // 6xxx: Logarithmic global volume
		m.command = CMD_GLOBALVOLUME;
		if(param12bit == 0)
			m.param = 0;
		else
			m.param = mpt::saturate_cast<ModCommand::PARAM>(GT2LogToLinearVolume(param12bit) / 32);
		return;

	case 0x07:  // 7xyy: Roll (x = speed, yy = maximum number of repetitions, 0 = unlimited)
		m.command = CMD_RETRIG;
		m.param = effect & 0x0F;
		return;

	case 0x08:
		m.command = CMD_RETRIG;
		if(fileVersion >= 0)
		{
			// GT2: 8xyz: Roll + Volume slide + Set balance
			if(m.volcmd == VOLCMD_NONE)
			{
				m.volcmd = VOLCMD_PANNING;
				m.vol = static_cast<ModCommand::VOL>(Util::muldivr(effect & 0x0F, 64, 15));
			}
		} else
		{
			// GTK: 8xyy: Roll + Set Linear Volume
			m.param = effect & 0x0F;
			m.volcmd = VOLCMD_VOLUME;
			m.vol = static_cast<ModCommand::VOL>((param + 3u) / 4u);
		}
		return;

	case 0x09:  // 9xxx: Offset
		m.command = CMD_OFFSET;
		{
			SmpLength fullOffset = param12bit;
			if(isFalcon)
			{
				if(!fullOffset)
				{
					// 9000 didn't recall last offset param
					fullOffset = 1;
				} else
				{
					SAMPLEINDEX sample = sndFile.GetSampleIndex(lastNote, lastInstr);
					if(sample != 0)
					{
						const ModSample &smp = sndFile.GetSample(sample);
						if(smp.uFlags[CHN_16BIT])
							fullOffset /= 2;
						// Offset beyond sample loop restarted from sample loop rather than cutting sample
						// We could probably also use a compatibility flag for this, but this is what newer GT2 versions do
						// And it avoids writing VOLCMD_OFFSET in case the parameter is very large
						if(smp.uFlags[CHN_LOOP] && fullOffset * 256u > smp.nLength)
						{
							fullOffset = (smp.nLoopStart + 8u) / 256u;
							LimitMax(fullOffset, (smp.nLength - 1u) / 256u);
						}
					}
				}
			}
			m.param = static_cast<ModCommand::PARAM>(fullOffset);
			if(fullOffset > 0xFF && m.volcmd == VOLCMD_NONE)
			{
				m.volcmd = VOLCMD_OFFSET;
				m.vol = static_cast<ModCommand::VOL>(std::min(fullOffset >> 8, static_cast<SmpLength>(std::size(ModSample().cues))));
			}
		}
		return;
	}

	switch(effect)
	{
	case 0x01:
	case 0x02:
	case 0x03:
	case 0x04:
	case 0x07:
	case 0x0B:
	case 0x0F:
		CSoundFile::ConvertModCommand(m, effect, param);
		if(m.command == CMD_TEMPO)
			currentTempo.Set(m.param, 0);
		else if(m.command == CMD_SPEED && m.param > 0)
			currentSpeed = m.param;
		break;
	case 0x05:  // 05xx: Tone portamento + vibrato
		m.command = CMD_TONEPORTAMENTO;
		if(m.volcmd == VOLCMD_NONE)
		{
			m.volcmd = VOLCMD_VIBRATODEPTH;
			m.vol = 0;
		}
		break;
	case 0x06:  // 06xx: Vibrato + tone portamento
		m.command = CMD_VIBRATO;
		if(m.volcmd == VOLCMD_NONE)
		{
			m.volcmd = VOLCMD_TONEPORTAMENTO;
			m.vol = 0;
		}
		break;
	case 0x08:  // 08xy: Detune
		if(!m.IsNote() || !param)
			break;
		m.command = CMD_FINETUNE;
		m.param = static_cast<ModCommand::PARAM>(0x80 + ((param & 0xF0) >> 1) - ((param & 0x0F) << 3));
		if(!sndFile.GetNumInstruments())
		{
			// Need instruments for correct pitch wheel depth
			sndFile.m_nInstruments = std::min(sndFile.m_nSamples, static_cast<SAMPLEINDEX>(MAX_INSTRUMENTS - 1u));
			for(SAMPLEINDEX smp = 1; smp <= sndFile.m_nInstruments; smp++)
			{
				if(auto ins = sndFile.AllocateInstrument(smp, smp); ins != nullptr)
				{
					ins->midiPWD = 2;
					ins->name = sndFile.m_szNames[smp];
					ins->SetCutoff(0x7F, true);
					ins->SetResonance(0, true);
				}
			}
		}
		break;
	case 0x09:  // 09xx: Note Delay
		m.command = CMD_MODCMDEX;
		m.param = 0xD0 | param4bit;
		break;
	case 0x0A:  // 0Axx: Cut Note / Key Off
		m.command = CMD_KEYOFF;
		break;
	case 0x0C:  // 0C0x: Vibrato waveform
		if(param <= 2)
		{
			m.command = CMD_MODCMDEX;
			m.param = 0x40 | param;
		}
		break;
	case 0x0D:  // 0Dxx: Pattern break
		m.command = CMD_PATTERNBREAK;
		break;
	case 0x0E:  // 0E0x: Tremolo waveform
		if(param <= 2)
		{
			m.command = CMD_MODCMDEX;
			m.param = 0x70 | param;
		}
		break;
	case 0x10:  // 10xy: Arpeggio
		m.command = CMD_ARPEGGIO;
		break;
	case 0x11:  // 11xx: Fine portamento up
	case 0x12:  // 12xx: Fine portamento down
		m.command = CMD_MODCMDEX;
		m.param = (effect << 4) | param4bitSlide;
		break;
	case 0x13:  // 13xy: Roll + Volume Slide
		m.command = CMD_RETRIG;
		break;
	case 0x14:  // 14xx: Linear volume slide up
		m.command = CMD_VOLUMESLIDE;
		m.param = param4bitSlide << 4;
		break;
	case 0x15:  // 15xx Linear volume slide down
		m.command = CMD_VOLUMESLIDE;
		m.param = param4bitSlide;
		break;
	case 0x16:  // 16xx: Logarithmic volume slide up
	case 0x17:  // 17xx: Logarithmic volume slide down
		break;
	case 0x18:  // 18xx: Linear volume slide up + Tone portamento
		m.command = CMD_TONEPORTAVOL;
		m.param = param4bitSlide << 4;
		break;
	case 0x19:  // 19xx: Linear volume slide down + Tone portamento
		m.command = CMD_TONEPORTAVOL;
		m.param = param4bitSlide;
		break;
	case 0x1A:  // 1Axx: Logarithmic volume slide up + Tone portamento
	case 0x1B:  // 1Bxx: Logarithmic volume slide down + Tone portamento
		break;
	case 0x1C:  // 1Cxx: Linear volume slide up + Vibrato
		m.command = CMD_VIBRATOVOL;
		m.param = param4bitSlide << 4;
		break;
	case 0x1D:  // 1Dxx: Linear volume slide down + Vibrato
		m.command = CMD_VIBRATOVOL;
		m.param = param4bitSlide;
		break;
	case 0x1E:  // 1Exx: Logarithmic volume slide up + Vibrato
	case 0x1F:  // 1Fxx: Logarithmic volume slide down + Vibrato
		break;
	case 0xA0:  // A0xx: Linear master volume slide up
		if(param)
		{
			m.command = CMD_GLOBALVOLSLIDE;
			m.param = param4bitDiv32 << 4;
		}
		break;
	case 0xA1:  // A1xx: Linear master volume slide down
		if(param)
		{
			m.command = CMD_GLOBALVOLSLIDE;
			m.param = param4bitDiv32;
		}
		break;
	case 0xA4:  // A4xx: Fine linear volume slide up
		m.command = CMD_VOLUMESLIDE;
		m.param = (param4bitSlide << 4) | 0x0F;
		break;
	case 0xA5:  // A5xx: Fine linear volume slide down
		m.command = CMD_VOLUMESLIDE;
		m.param = param4bitSlide | 0xF0;
		break;
	case 0xA6:  // A6xx: Fine linear master volume slide up
		if(param)
		{
			m.command = CMD_GLOBALVOLSLIDE;
			m.param = (param4bitDiv32 << 4) | 0x0F;
		}
		break;
	case 0xA7:  // A7xx: Fine linear master volume slide down
		if(param)
		{
			m.command = CMD_GLOBALVOLSLIDE;
			m.param = param4bitDiv32 | 0xF0;
		}
		break;
	case 0xA8:  // A8xx: Set number of ticks
		if(param)
		{
			m.command = CMD_SPEED;
			currentSpeed = m.param;
		}
		break;
	case 0xA9:  // A9xx: Set fine tempo (fractional)
		if(isFalcon && currentTempo.GetInt() > 0)
		{
			// GTK / GT2 Falcon version (assuming 49170 Hz sample rate, the best possible. bigbigbt.gtk and twenty five-twenty.gtk require this sample rate)
			double samplesPerTick = (49170 * 2.5) / currentTempo.ToDouble();
			samplesPerTick += param * 2 - 256;
			m.command = CMD_TEMPO;
			m.param = mpt::saturate_round<ModCommand::PARAM>((49170 * 5) / (samplesPerTick * 2));
		} else
		{
			// GT2 Windows version - Add xx/256 to tempo
		}
		break;
	case 0xAA:  // AAxx: Pattern delay
		m.command = CMD_MODCMDEX;
		m.param = 0xE0 | param4bit;
		break;
	case 0xAC:  // ACxx: Extra fine portamento up
		m.command = CMD_XFINEPORTAUPDOWN;
		m.param = 0x10 | param4bit;
		break;
	case 0xAD:  // ADxx: Extra fine portamento down
		m.command = CMD_XFINEPORTAUPDOWN;
		m.param = 0x20 | param4bit;
		break;
	case 0xAE:  // AExx: Pan slide left
		m.command = CMD_PANNINGSLIDE;
		m.param = (param4bitSlide / 2u) << 4;  // approximation
		break;
	case 0xAF:  // AFxx: Pan slide right
		m.command = CMD_PANNINGSLIDE;
		m.param = param4bitSlide / 2u;  // approximation
		break;
	case 0xB0:  // B0xy: Tremor
		m.command = CMD_TREMOR;
		break;
	case 0xB1:  // B1xx: Pattern Loop
		m.command = CMD_MODCMDEX;
		m.param = 0xB0 | param4bit;
		break;
	case 0xB2:  // B2xx: Per-channel interpolation flags
	case 0xB3:  // B3xx: Set Volume Envelope (we have no way to use another envelope)
		m.command = CMD_S3MCMDEX;
		m.param = 0x77 + (param ? 1 : 0);
		break;
	case 0xB4:  // B4xx: Set Tone Envelope (ditto)
		m.command = CMD_S3MCMDEX;
		m.param = 0x7B + (param ? 2 : 0);
		break;
	case 0xB5:  // B5xx: Set Panning Envelope (ditto)
		m.command = CMD_S3MCMDEX;
		m.param = 0x79 + (param ? 1 : 0);
		break;
	case 0xB6:  // B6xx: Set Cutoff Envelope (ditto)
		m.command = CMD_S3MCMDEX;
		m.param = 0x7B + (param ? 3 : 0);
		break;
	case 0xB7:  // B7xx: Set Resonance Envelope (ditto)
		break;
	case 0xBA:  // BAxx: Fine Offset
		m.command = CMD_OFFSET;
		m.param = static_cast<ModCommand::PARAM>((m.param + 15u) / 16u);
		break;
	case 0xBB:  // BBxx: Very Fine Offset
		m.command = CMD_OFFSET;
		m.param = 1;
		break;
	case 0xBE:  // BExx: Auto Tempo (adjusts tempo so that a C-2 played by the current sample will last exactly xx rows)
	case 0xBF:  // BFxx: Auto Period (adjusts note frequency so that the note will last exactly xx rows)
		// Auto Tempo crashes GT2 for Windows. Differences from GT2 for Falcon are based on the source code (assuming no crash)
		if(param != 0 && lastNote != NOTE_NONE && lastInstr != 0 && currentTempo.GetInt() > 0 && currentSpeed > 0)
		{
			SAMPLEINDEX sample = sndFile.GetSampleIndex(lastNote, lastInstr);
			if(!sample)
				break;
			const ModSample &smp = sndFile.GetSample(sample);
			SmpLength length = smp.nLength;
			if(smp.uFlags[CHN_LOOP])
			{
				length = smp.nLoopEnd;
				if(isFalcon)
					length -= smp.nLoopStart;
				if(smp.uFlags[CHN_PINGPONGLOOP] && !isFalcon)
					length *= 2;
			}
			if(isFalcon && length < 1024)
				break;
			if(!length || !smp.nC5Speed)
				break;

			int32 transpose = 0, finetune = 0;
			if(lastInstr <= sndFile.GetNumInstruments() && sndFile.Instruments[lastInstr] != nullptr)
				transpose = static_cast<int32>(sndFile.Instruments[lastInstr]->NoteMap[lastNote - NOTE_MIN]) - lastNote;
			if(isFalcon && effect == 0xBE)
				finetune = smp.nFineTune;

			const double sampleRate = smp.nC5Speed * std::pow(2.0, finetune / (-128.0 * 12.0) + transpose / 12.0);

			if(effect == 0xBE)
			{
				// Note: This assumes that the middle-C frequency was doubled when converting the samples
				const double tempo = 2.5 * (currentSpeed * param) * (sampleRate * 0.5) / length;
				// GT2 would divide the tempo by 2 while it's greater than 999 but we cannot store tempos above 255 here anyway
				m.command = CMD_TEMPO;
				m.param = mpt::saturate_round<ModCommand::PARAM>(tempo);
				currentTempo = TEMPO(m.param, 0);
			} else
			{
				const double sampleDurationMiddleC = length / sampleRate;
				const double wantedDuration = (param * currentSpeed * 2.5) / currentTempo.ToDouble();
				const double ratio = sampleDurationMiddleC / wantedDuration;
				const double note = Clamp(double(NOTE_MIDDLEC) + mpt::log2(ratio) * 12.0, double(NOTE_MIN), double(NOTE_MAX)), frac = note - std::floor(note);

				const bool hadNote = m.IsNote();
				m.note = static_cast<ModCommand::NOTE>(note);
				m.param = static_cast<ModCommand::PARAM>(0x80 + (frac * 64.0));
				if(m.param != 0x80)
					m.command = CMD_FINETUNE;

				if(!hadNote)
				{
					if(m.volcmd == VOLCMD_NONE)
					{
						m.volcmd = VOLCMD_TONEPORTAMENTO;
						m.vol = 9;
					} else
					{
						m.command = CMD_TONEPORTA_DURATION;
						m.param = 0;
					}
				}
			}
		}
		break;
	case 0xC0:  // C0xy: Change sample direction after y ticks (x: 0 = invert current direction, 1 = forward, 2 = backward)
		if(const uint8 direction = (param >> 4); direction == 1 || direction == 2)
		{
			m.command = CMD_S3MCMDEX;
			m.param = static_cast<ModCommand::PARAM>(0x9D + direction);
		}
		break;
	case 0xC2:  // C2xx: Set linear track volume (0x10 = 0dB)
		m.command = CMD_CHANNELVOLUME;
		m.param *= 4;
		break;
	case 0xC3:  // C2xx: Set logarithmic track volume (0xC0 = 0dB)
		m.command = CMD_CHANNELVOLUME;
		m.param = mpt::saturate_cast<ModCommand::PARAM>(GT2LogToLinearVolume(param << 4) / 64);
		break;
	case 0xC4:  // C4xx: Linear track volume slide up
	case 0xC6:  // C6xx: Logarithmic track volume slide up
		m.command = CMD_CHANNELVOLSLIDE;
		m.param = param4bitDiv128 << 4;
		break;
	case 0xC5:  // C5xx: Linear track volume slide down
	case 0xC7:  // C7xx: Logarithmic track volume slide down
		m.command = CMD_CHANNELVOLSLIDE;
		m.param = param4bitDiv128;
		break;
	case 0xC8:  // C8xx: Fine logarithmic track volume slide up
		m.command = CMD_CHANNELVOLSLIDE;
		m.param = (param4bitSlide << 4) | 0x0F;
		break;
	case 0xC9:  // C9xx: Fine logarithmic track volume slide down
		m.command = CMD_CHANNELVOLSLIDE;
		m.param = param4bitSlide | 0xF0;
		break;
	default:
		break;
	}
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderGTK(MemoryFileReader file, const uint64 *pfilesize)
{
	GTKFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.Validate())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadGTK(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	GTKFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return false;
	if(!fileHeader.Validate())
		return false;
	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(fileHeader.GetHeaderMinimumAdditionalSize())))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	// Globals
	InitializeGlobals(MOD_TYPE_MPT, fileHeader.numChannels);
	m_SongFlags.set(SONG_IMPORTED);
	m_playBehaviour = GetDefaultPlaybackBehaviour(MOD_TYPE_IT);
	m_playBehaviour.set(kApplyOffsetWithoutNote);
	SetupMODPanning(true);

	m_modFormat.madeWithTracker = UL_("Graoumf Tracker");
	m_modFormat.formatName = MPT_UFORMAT("Graoumf Tracker v{}")(fileHeader.fileVersion);
	m_modFormat.type = UL_("gtk");
	m_modFormat.charset = mpt::Charset::ISO8859_1_no_C1;

	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.songName);
	size_t msgLength = sizeof(fileHeader.smallComment);
	while(msgLength > 0 && fileHeader.smallComment[msgLength - 1] == ' ')
		msgLength--;
	m_songMessage.Read(mpt::byte_cast<const std::byte *>(fileHeader.smallComment), msgLength, SongMessage::leAutodetect);

	m_nSamples = fileHeader.numSamples;
	for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
	{
		ModSample &mptSmp = Samples[smp];
		mptSmp.Initialize(MOD_TYPE_MOD);
		mptSmp.Set16BitCuePoints();
		file.ReadString<mpt::String::spacePadded>(m_szNames[smp], (fileHeader.fileVersion == 1) ? 32 : 28);
		if(fileHeader.fileVersion >= 3)
		{
			file.Skip(14);                          // reserved
			int16 defaultPan = file.ReadInt16BE();  // -1 = use track panning, 0...4095 otherwise
			if(defaultPan >= 0)
			{
				mptSmp.uFlags.set(CHN_PANNING);
				mptSmp.nPan = static_cast<uint16>(Util::muldivr_unsigned(defaultPan, 256, 4095));
			}
		}
		uint16 bytesPerSample = 1;
		if(fileHeader.fileVersion >= 2)
		{
			bytesPerSample = file.ReadUint16BE();
			mptSmp.nC5Speed = file.ReadUint16BE();
		}
		const auto [length, loopStart, loopLength] = file.ReadArray<uint32be, 3>();
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = mptSmp.nLoopStart + loopLength;
		mptSmp.uFlags.set(CHN_LOOP, mptSmp.nLoopStart > 0 || loopLength > 2);
		if(bytesPerSample == 2)
		{
			mptSmp.uFlags.set(CHN_16BIT);
			mptSmp.nLength /= 2u;
			mptSmp.nLoopStart /= 2u;
			mptSmp.nLoopEnd /= 2u;
		}
		mptSmp.nVolume = file.ReadUint16BE();
		int16 finetune = file.ReadInt16BE();  // -8...7
		if(finetune != 0)
			mptSmp.Transpose(finetune / (12.0 * 8.0));
		mptSmp.nFineTune = MOD2XMFineTune(finetune);  // We store this because we need to undo the finetune for the auto-tempo command
	}

	Order().SetRestartPos(fileHeader.restartPos);
	FileReader orderChunk = file.ReadChunk(512);
	ReadOrderFromFile<uint16be>(Order(), orderChunk, fileHeader.numOrders);
	PATTERNINDEX numPatterns = *std::max_element(Order().begin(), Order().end()) + 1u;

	Patterns.ResizeArray(numPatterns);
	const uint8 eventSize = fileHeader.fileVersion < 4 ? 4 : 5;
	TEMPO currentTempo = Order().GetDefaultTempo();
	uint32 currentSpeed = Order().GetDefaultSpeed();
	for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
	{
		if(!(loadFlags & loadPatternData) || !file.CanRead(fileHeader.numRows * GetNumChannels() * eventSize) || !Patterns.Insert(pat, fileHeader.numRows))
			continue;

		std::vector<std::pair<ModCommand::NOTE, ModCommand::INSTR>> lastNoteInstr(GetNumChannels(), {NOTE_NONE, {}});
		for(ROWINDEX row = 0; row < fileHeader.numRows; row++)
		{
			auto rowData = Patterns[pat].GetRow(row);
			for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
			{
				ModCommand &m = rowData[chn];
				uint8 data[5]{};
				file.ReadStructPartial(data, eventSize);
				if(data[0] >= 24 && data[0] < 84)
				{
					m.note = data[0] + NOTE_MIN + 12;
					lastNoteInstr[chn].first = m.note;
				}
				if(data[1])
				{
					m.instr = data[1];
					lastNoteInstr[chn].second = m.instr;
				}
				TranslateGraoumfEffect(*this, m, data[2], data[3], -static_cast<int16>(fileHeader.fileVersion), lastNoteInstr[chn].first, lastNoteInstr[chn].second, currentTempo, currentSpeed);
				if(data[4])
				{
					m.volcmd = VOLCMD_VOLUME;
					m.vol = static_cast<ModCommand::VOL>((data[4] + uint32(1)) / 4u);
				}
			}
		}
	}

	if(!(loadFlags & loadSampleData))
		return true;
	for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
	{
		SampleIO(
			Samples[smp].uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::bigEndian,
			SampleIO::signedPCM).ReadSample(Samples[smp], file);
	}

	return true;
}


// GT2 File Header
struct GT2FileHeader
{
	char     signature[3];
	uint8be  fileVersion;
	uint32be headerSize;
	char     songName[32];
	char     smallComment[160];
	uint8be  day;
	uint8be  month;
	uint16be year;
	char     trackerName[24];
	// The following fields are only present in version 0-5.
	uint16be speed;
	uint16be tempo;
	uint16be masterVol;
	uint16be numPannedTracks;

	bool Validate() const
	{
		if(std::memcmp(signature, "GT2", 3) || fileVersion > 9 || year < 1980 || year > 9999)
			return false;
		if(fileVersion > 5)
			return true;
		return speed > 0 && tempo > 0 && masterVol <= 0xFFF && numPannedTracks <= 99;
	}

	uint64 GetHeaderMinimumAdditionalSize() const
	{
		return std::max(static_cast<size_t>(headerSize), sizeof(GT2FileHeader)) - sizeof(GT2FileHeader) + 20u;  // 10 bytes each for required PATS and SONG chunks
	}
};

MPT_BINARY_STRUCT(GT2FileHeader, 236)


// GT2 File Header
struct GT2Chunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers : uint32
	{
		idXCOM = MagicBE("XCOM"),
		idTCN1 = MagicBE("TCN1"),
		idTCN2 = MagicBE("TCN2"),
		idTVOL = MagicBE("TVOL"),
		idMIXP = MagicBE("MIXP"),
		idSONG = MagicBE("SONG"),
		idPATS = MagicBE("PATS"),
		idPATD = MagicBE("PATD"),
		idTNAM = MagicBE("TNAM"),
		idINST = MagicBE("INST"),
		idVENV = MagicBE("VENV"),
		idTENV = MagicBE("TENV"),
		idPENV = MagicBE("PENV"),
		idSAMP = MagicBE("SAMP"),
		idSAM2 = MagicBE("SAM2"),
		idENDC = MagicBE("ENDC"),
	};

	uint32be id;
	uint32be length;

	size_t GetLength() const
	{
		return std::max(length.get(), uint32(8)) - 8u;
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(id.get());
	}
};

MPT_BINARY_STRUCT(GT2Chunk, 8)


// GT2 Pattern Data
struct GT2PatternCell
{
	uint8 note;
	uint8 instr;
	uint8 effect;
	uint8 param;
	uint8 volume;
};

MPT_BINARY_STRUCT(GT2PatternCell, 5)


// GT2 Track Name
struct GT2TrackName
{
	uint16be type;
	uint16be trackNumber;
	char     name[32];
};

MPT_BINARY_STRUCT(GT2TrackName, 36)


// GT2 Instrument
struct GT2Instrument
{
	struct SampleInfo
	{
		uint8 num;
		int8 transpose;
	};

	uint16be insNum;
	char name[28];
	uint16be type;             // 0 = sample-based
	uint16be defaultVelocity;  // 0...256
	int16be  defaultPan;       // -1 = use track panning, 0...4095 otherwise
	uint16be volEnv;           // 0 = no envelope
	uint16be toneEnv;          // 0 = no envelope
	uint16be panEnv;           // 0 = no envelope
	uint16be cutoffEnv;        // 0 = no envelope
	uint16be resoEnv;          // 0 = no envelope
	char     reserved[4];
	uint16be version;  // 0...2
	SampleInfo samples[128];
};

MPT_BINARY_STRUCT(GT2Instrument, 308)


struct GT2InstrumentExt
{
	enum FilterFlags : uint16
	{
		fltEnabled     = 0x01,
		fltVelToCutoff = 0x02,
		fltVelToReso   = 0x04,
		fltTypeMask    = 0x18,

		fltTypeLowpass  = 0x00,
		fltTypeHighpass = 0x08,
		fltTypeBandpass = 0x10,
		fltTypeNotch    = 0x18,
	};

	uint16be filterFlags;     // See FilterFlags
	char     maxVelFreq[24];  // Cutoff frequency for the minimum velocity (256); <= 20: Multiple of the note frequency, > 20: Fixed frequency in Hz
	char     minVelFreq[24];  // Cutoff frequency for the maximum velocity (0); ditto
	char     maxVelReso[24];  // Q (resonance) value for the minimum velocity (256)
	char     minVelReso[24];  // Q (resonance) value for the maximum velocity (0)
	uint8    pitchPanCenter;  // Unused
	uint8    pitchPanSep;     // Unused
	uint8    accent;          // Unused
	uint8    numEnvelopes;
};

MPT_BINARY_STRUCT(GT2InstrumentExt, 102)


struct GT2Envelope
{
	enum EnvelopeFlags : uint16
	{
		envFadeOut = 0x01,
		envLFO     = 0x02,
		envLoop    = 0x10,
		envSustain = 0x20,
	};

	struct EnvPoint
	{
		uint16be duration;
		int16be  value;
	};

	uint16be numPoints;
	uint16be lfoDepth;
	uint16be lfoSpeed;
	uint16be lfoSweep;
	uint16be lfoWaveform;
	uint16be fadeOut;
	uint16be flags;  // See EnvelopeFlags
	uint16be loopStart;
	uint16be loopEnd;
	uint16be loopRepCount;
	uint16be sustainStart;
	uint16be sustainEnd;
	uint16be sustainRepCount;
	EnvPoint data[64];

	void ConvertToMPT(ModInstrument &mptIns, EnvelopeType type) const
	{
		InstrumentEnvelope &mptEnv = mptIns.GetEnvelope(type);
		mptEnv.resize(std::min(numPoints.get(), static_cast<uint16>(std::size(data))));
		mptEnv.nLoopStart = static_cast<uint8>(loopStart);
		mptEnv.nLoopEnd = static_cast<uint8>(loopEnd);
		mptEnv.nSustainStart = static_cast<uint8>(sustainStart);
		mptEnv.nSustainEnd = static_cast<uint8>(sustainEnd);
		mptEnv.dwFlags.set(ENV_ENABLED, !mptEnv.empty());
		mptEnv.dwFlags.set(ENV_LOOP, (flags & envLoop) != 0);
		mptEnv.dwFlags.set(ENV_SUSTAIN, (flags & envSustain) != 0);

		int16 minValue, maxValue;
		switch(type)
		{
		case ENV_VOLUME:
		default:
			minValue = 0;
			maxValue = 4096;
			break;
		case ENV_PANNING:
			minValue = -128;
			maxValue = 127;
			break;
		case ENV_PITCH:
			minValue = -9999 * 16 / 99;
			maxValue = 9999 * 16 / 99;
			break;
		}

		uint16 tick = 0;
		for(uint32 i = 0; i < mptEnv.size(); i++)
		{
			mptEnv[i].tick = tick;
			int32 value = data[i].value + minValue;
			value = Util::muldivr(value, ENVELOPE_MAX, maxValue - minValue);
			mptEnv[i].value = mpt::saturate_cast<uint8>(value);
			tick += std::max(data[i].duration.get(), uint16(1));
		}
	}
};

MPT_BINARY_STRUCT(GT2Envelope, 282)


struct GT2SampleV2
{
	uint16be smpNum;
	char     name[28];
	uint16be type;  // 0 = memory-based, 1 = disk-based
	uint16be bits;
	uint16be endian;  // 0 = big, 1 = little
	uint16be numChannels;
	int16be  defaultPan;     // -1 = use track panning, 0...4095 otherwise
	uint16be volume;         // 0...4095, 256 = 0dB
	int16be  finetune;       // -8...7
	uint16be loopType;       // 0 = no, 1 = forward, 2 = ping-pong
	int16be  midiNote;       // 48 = default (no transpose)
	uint16be sampleCoding;   // 0 = PCM in module, 1 = PCM in separate file
	uint16be filenameLen;
	int16be  yPanning;    // Unused
	uint32be sampleFreq;  // Sample frequency of reference note
	uint32be length;
	uint32be loopStart;
	uint32be loopLength;
	uint32be loopBufLength;
	uint32be dataOffset;

	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize(MOD_TYPE_IT);
		mptSmp.Set16BitCuePoints();

		mptSmp.nGlobalVol = volume / 4u;
		if(defaultPan > 0)
		{
			mptSmp.uFlags.set(CHN_PANNING);
			mptSmp.nPan = static_cast<uint16>(Util::muldivr_unsigned(defaultPan, 256, 4095));
		}
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopStart + loopLength;
		mptSmp.uFlags.set(CHN_LOOP, loopType != 0);
		mptSmp.uFlags.set(CHN_PINGPONGLOOP, loopType & 2);
		mptSmp.nC5Speed = sampleFreq * 2u;
		if(finetune != 0)
			mptSmp.Transpose(finetune / (12.0 * 8.0));
		mptSmp.nFineTune = MOD2XMFineTune(finetune);  // We store this because we need to undo the finetune for the auto-tempo command
	}

	SampleIO GetSampleFormat() const
	{
		return SampleIO(
			bits == 8 ? SampleIO::_8bit : SampleIO::_16bit,
			numChannels == 1 ? SampleIO::mono : SampleIO::stereoInterleaved,
			endian == 0 ? SampleIO::bigEndian : SampleIO::littleEndian,
			SampleIO::signedPCM);
	}
};

MPT_BINARY_STRUCT(GT2SampleV2, 78)


struct GT2SampleV1
{
	enum SampleFlags : uint16
	{
		smpStereo   = 0x01,
		smpPingPong = 0x02,
	};

	uint16be smpNum;
	char     name[28];
	uint16be flags;       // See SampleFlags
	int16be  defaultPan;  // -1 = use track panning, 0...4095 otherwise
	uint16be bits;
	uint16be sampleFreq;
	uint32be length;
	uint32be loopStart;
	uint32be loopLength;    // Loop is deactivated if loopStart = 0 and loopLength = 2
	int16be  volume;        // 0...255
	int16be  finetune;      // -8...7
	uint16be sampleCoding;  // 0

	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize(MOD_TYPE_IT);
		mptSmp.Set16BitCuePoints();

		mptSmp.nGlobalVol = static_cast<uint16>(volume / 4u);
		if(defaultPan > 0)
		{
			mptSmp.uFlags.set(CHN_PANNING);
			mptSmp.nPan = static_cast<uint16>(Util::muldivr_unsigned(defaultPan, 256, 4095));
		}
		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopStart + loopLength;
		if(bits == 16)
		{
			mptSmp.nLength /= 2u;
			mptSmp.nLoopStart /= 2u;
			mptSmp.nLoopEnd /= 2u;
		}
		mptSmp.uFlags.set(CHN_LOOP, loopStart > 0 || loopLength > 2);
		mptSmp.uFlags.set(CHN_PINGPONGLOOP, (flags & GT2SampleV1::smpPingPong) != 0);
		mptSmp.nC5Speed = sampleFreq * 2u;
		if(finetune != 0)
			mptSmp.Transpose(finetune / (12.0 * 8.0));
		mptSmp.nFineTune = MOD2XMFineTune(finetune);  // We store this because we need to undo the finetune for the auto-tempo command
	}

	SampleIO GetSampleFormat() const
	{
		return SampleIO(
			bits == 8 ? SampleIO::_8bit : SampleIO::_16bit,
			(flags & GT2SampleV1::smpStereo) ? SampleIO::stereoInterleaved : SampleIO::mono,
			SampleIO::bigEndian,
			SampleIO::signedPCM);
	}
};

MPT_BINARY_STRUCT(GT2SampleV1, 56)


struct GT2MixPreset
{
	uint16be number;
	char     name[32];
	uint16be trackType;
	uint16be trackIndex;
	uint16be numTracks;
	uint16be volWet;  // 0..1000..FFFF
	uint16be panWet;  // 0...FFF
	uint16be volDry;
	uint16be panDry;
	uint16be version;  // 0x101
	uint16be stereo;
	uint32be reserved;
};

MPT_BINARY_STRUCT(GT2MixPreset, 56)


struct GT2MixPresetTrack
{
	uint8    trackType;
	uint8    dryFlag;  // Effects tracks: 0 = wet, 1 = dry
	uint16be trackIndex;
	uint16be volume;   // 0..1000..FFFF
	uint16be balance;  // 0..800..FFF
};

MPT_BINARY_STRUCT(GT2MixPresetTrack, 8)


static void ConvertGT2Envelope(InstrumentSynth &synth, const uint16 envNum, std::vector<FileReader> &envChunks)
{
	if(!envNum)
		return;
	for(auto &chunk : envChunks)
	{
		chunk.Rewind();
		if(chunk.ReadUint16BE() != envNum)
			continue;
		chunk.Skip(20);  // Name
		const uint16 keyoffOffset = chunk.ReadUint16BE();
		uint16 jumpOffset = 0;

		std::map<uint16, uint16> offsetToIndex;
		auto &events = synth.m_scripts.emplace_back();
		events.push_back(InstrumentSynth::Event::GTK_KeyOff(keyoffOffset));
		while(chunk.CanRead(1))
		{
			const uint16 chunkPos = static_cast<uint16>(chunk.GetPosition() - 24);
			if(!jumpOffset && chunkPos >= keyoffOffset)
			{
				events.push_back(InstrumentSynth::Event::StopScript());
				jumpOffset = keyoffOffset;
			}

			offsetToIndex[chunkPos] = static_cast<uint16>(events.size());
			const uint8 command = chunk.ReadUint8();
			switch(command)
			{
			case 0x00:  // 00: End
				events.push_back(InstrumentSynth::Event::StopScript());
				break;
			case 0x01:  // 01: Unconditional Jump
				events.push_back(InstrumentSynth::Event::Jump(jumpOffset + chunk.ReadUint16BE()));
				break;
			case 0x02:  // 02: Wait
				events.push_back(InstrumentSynth::Event::Delay(std::max(chunk.ReadUint16BE(), uint16(1)) - 1));
				break;
			case 0x03:  // 03: Set Counter
				events.push_back(InstrumentSynth::Event::SetLoopCounter(std::max(chunk.ReadUint8(), uint8(1)) - 1, true));
				break;
			case 0x04:  // 04: Loop
				events.push_back(InstrumentSynth::Event::EvaluateLoopCounter(jumpOffset + chunk.ReadUint16BE()));
				break;
			case 0x05:  // 05: Key Off
				events.push_back(InstrumentSynth::Event::Jump(keyoffOffset));
				break;

			case 0x80:  // 80: Set Volume (16384 = 100%, max = 32767)
				events.push_back(InstrumentSynth::Event::GTK_SetVolume(chunk.ReadUint16BE()));
				break;
			case 0xA0:  // A0: Set Tone (4096 = normal period)
				events.push_back(InstrumentSynth::Event::GTK_SetPitch(chunk.ReadUint16BE()));
				break;
			case 0xC0:  // C0: Set Panning (2048 = normal position)
				events.push_back(InstrumentSynth::Event::GTK_SetPanning(chunk.ReadUint16BE()));
				break;

			case 0x81:  // 81: Set Volume step
				events.push_back(InstrumentSynth::Event::GTK_SetVolumeStep(chunk.ReadInt16BE()));
				break;
			case 0xA1:  // A1: Set Tone step
				events.push_back(InstrumentSynth::Event::GTK_SetPitchStep(chunk.ReadInt16BE()));
				break;
			case 0xC1:  // C1: Set Pan step
				events.push_back(InstrumentSynth::Event::GTK_SetPanningStep(chunk.ReadInt16BE()));
				break;
			case 0x82:  // 82: Set Volume speed
			case 0xA2:  // A2: Set Tone speed
			case 0xC2:  // C2: Set Pan speed
				events.push_back(InstrumentSynth::Event::GTK_SetSpeed(std::max(chunk.ReadUint8(), uint8(1))));
				break;

			case 0x87:  // 87: Tremor On
				events.push_back(InstrumentSynth::Event::GTK_EnableTremor(true));
				break;
			case 0x88:  // 88: Tremor Off
				events.push_back(InstrumentSynth::Event::GTK_EnableTremor(false));
				break;
			case 0x89:  // 89: Set Tremor Time 1
				events.push_back(InstrumentSynth::Event::GTK_SetTremorTime(std::max(chunk.ReadUint8(), uint8(1)), 0));
				break;
			case 0x8A:  // 8A: Set Tremor Time 2
				events.push_back(InstrumentSynth::Event::GTK_SetTremorTime(0, std::max(chunk.ReadUint8(), uint8(1))));
				break;

			case 0x83:  // 83: Tremolo On
				events.push_back(InstrumentSynth::Event::GTK_EnableTremolo(true));
				break;
			case 0x84:  // 84: Tremolo Off
				events.push_back(InstrumentSynth::Event::GTK_EnableTremolo(false));
				break;
			case 0x85:  // 85: Set Tremolo Width
				events.push_back(InstrumentSynth::Event::GTK_SetVibratoParams(std::max(chunk.ReadUint8(), uint8(1)), 0));
				break;
			case 0x86:  // 86: Set Tremolo Speed
				events.push_back(InstrumentSynth::Event::GTK_SetVibratoParams(0, std::max(chunk.ReadUint8(), uint8(1))));
				break;

			case 0xA3:  // A3: Vibrato On
				events.push_back(InstrumentSynth::Event::GTK_EnableVibrato(true));
				break;
			case 0xA4:  // A4: Vibrato Off
				events.push_back(InstrumentSynth::Event::GTK_EnableVibrato(false));
				break;
			case 0xA5:  // A5: Set Vibrato Width
				events.push_back(InstrumentSynth::Event::GTK_SetVibratoParams(std::max(chunk.ReadUint8(), uint8(1)), 0));
				break;
			case 0xA6:  // A6: Set Vibrato Speed
				events.push_back(InstrumentSynth::Event::GTK_SetVibratoParams(0, std::max(chunk.ReadUint8(), uint8(1))));
				break;
			}
		}
		for(auto &event : events)
		{
			event.FixupJumpTarget(offsetToIndex);
		}
		break;
	}
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderGT2(MemoryFileReader file, const uint64 *pfilesize)
{
	GT2FileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.Validate())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadGT2(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	GT2FileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return false;
	if(!fileHeader.Validate())
		return false;
	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(fileHeader.GetHeaderMinimumAdditionalSize())))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	std::vector<uint16be> pannedTracks;
	file.ReadVector(pannedTracks, fileHeader.numPannedTracks);

	ChunkReader chunkFile(file);
	auto chunks = chunkFile.ReadChunksUntil<GT2Chunk>(1, GT2Chunk::idENDC);

	if(auto chunk = chunks.GetChunk(GT2Chunk::idPATS); chunk.CanRead(2))
	{
		if(uint16 channels = chunk.ReadUint16BE(); channels >= 1 && channels <= MAX_BASECHANNELS)
			InitializeGlobals(MOD_TYPE_MPT, channels);
		else
			return false;
	} else
	{
		return false;
	}

	// Globals
	m_SongFlags.set(SONG_IMPORTED | SONG_EXFILTERRANGE);
	m_playBehaviour = GetDefaultPlaybackBehaviour(MOD_TYPE_IT);
	m_playBehaviour.set(kFT2ST3OffsetOutOfRange, fileHeader.fileVersion >= 6);
	m_playBehaviour.set(kApplyOffsetWithoutNote);

	FileHistory mptHistory;
	mptHistory.loadDate.day = Clamp<uint8, uint8>(fileHeader.day, 1, 31);
	mptHistory.loadDate.month = Clamp<uint8, uint8>(fileHeader.month, 1, 12);
	mptHistory.loadDate.year = fileHeader.year;
	m_FileHistory.push_back(mptHistory);

	m_modFormat.madeWithTracker = mpt::ToUnicode(mpt::Charset::ASCII, mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.trackerName));
	m_modFormat.formatName = (fileHeader.fileVersion <= 5 ? MPT_UFORMAT("Graoumf Tracker v{}") : MPT_UFORMAT("Graoumf Tracker 2 v{}"))(fileHeader.fileVersion);
	m_modFormat.type = UL_("gt2");
	m_modFormat.charset = mpt::Charset::ISO8859_1_no_C1;

	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.songName);

	m_nSamplePreAmp = 256;
	m_nDefaultGlobalVolume = 384 / (3 + GetNumChannels());  // See documentation on command 5xxx: Default linear master volume is 12288 / (3 + number of channels)

	if(fileHeader.fileVersion <= 5)
	{
		Order().SetDefaultSpeed(std::max(fileHeader.speed.get(), uint16(1)));
		Order().SetDefaultTempoInt(std::max(fileHeader.tempo.get(), uint16(1)));
		m_nDefaultGlobalVolume = std::min(Util::muldivr_unsigned(fileHeader.masterVol, MAX_GLOBAL_VOLUME, 4095), uint32(MAX_GLOBAL_VOLUME));
		const CHANNELINDEX tracks = std::min(static_cast<CHANNELINDEX>(pannedTracks.size()), GetNumChannels());
		for(CHANNELINDEX chn = 0; chn < tracks; chn++)
		{
			ChnSettings[chn].nPan = std::min(static_cast<uint16>(Util::muldivr_unsigned(pannedTracks[chn], 256, 4095)), uint16(256));
		}
	}
	file.Seek(fileHeader.headerSize);

	if(auto chunk = chunks.GetChunk(GT2Chunk::idSONG); chunk.CanRead(2))
	{
		ORDERINDEX numOrders = chunk.ReadUint16BE();
		Order().SetRestartPos(chunk.ReadUint16BE());
		LimitMax(numOrders, mpt::saturate_cast<uint16>(chunk.BytesLeft() / 2u));
		ReadOrderFromFile<uint16be>(Order(), chunk, numOrders);
	} else
	{
		return false;
	}

	if(auto chunk = chunks.GetChunk(GT2Chunk::idXCOM); chunk.CanRead(3))
	{
		const uint16 length = chunk.ReadUint16BE();
		m_songMessage.Read(chunk, length, SongMessage::leAutodetect);
	} else
	{
		size_t msgLength = sizeof(fileHeader.smallComment);
		while(msgLength > 0 && fileHeader.smallComment[msgLength - 1] == ' ')
			msgLength--;
		m_songMessage.Read(mpt::byte_cast<const std::byte *>(fileHeader.smallComment), msgLength, SongMessage::leAutodetect);
	}

	if(auto chunk = chunks.GetChunk(GT2Chunk::idTCN1); chunk.ReadUint8() == 0 && chunk.Seek(2648))
	{
		// Only channel mute status is of interest here
		uint32 muteStatus = chunk.ReadUint32BE();
		const CHANNELINDEX numChannels = std::min(GetNumChannels(), CHANNELINDEX(32));
		for(CHANNELINDEX i = 0; i < numChannels; i++)
		{
			ChnSettings[i].dwFlags.set(CHN_MUTE, !(muteStatus & (1u << i)));
		}
	}

	if(auto chunk = chunks.GetChunk(GT2Chunk::idTCN2); chunk.CanRead(12))
	{
		const auto [chunkVersion, bpmInt, bpmFract, speed, timeSigNum, timeSigDenum] = chunk.ReadArray<uint16be, 6>();
		Order().SetDefaultTempo(TEMPO(Clamp<uint16, uint16>(bpmInt, 32, 999), Util::muldivr_unsigned(bpmFract, TEMPO::fractFact, 65536)));
		Order().SetDefaultSpeed(Clamp<uint16, uint16>(speed, 1, 255));
		m_nDefaultRowsPerBeat = 16 / Clamp<uint16, uint16>(timeSigDenum, 1, 16);
		m_nDefaultRowsPerMeasure = m_nDefaultRowsPerBeat * Clamp<uint16, uint16>(timeSigNum, 1, 16);
		if(chunkVersion >= 1)
		{
			uint16 periodMode = chunk.ReadUint16BE();
			m_SongFlags.set(SONG_LINEARSLIDES, !periodMode);
		}
	}

	if(auto chunk = chunks.GetChunk(GT2Chunk::idTVOL); chunk.CanRead(2))
	{
		CHANNELINDEX numChannels = std::min(static_cast<CHANNELINDEX>(chunk.ReadUint16BE()), GetNumChannels());
		for(CHANNELINDEX chn = 0; chn < numChannels; chn++)
		{
			ChnSettings[chn].nVolume = static_cast<uint8>(std::min(chunk.ReadUint16BE(), uint16(4096)) / 64u);
		}
	}

	// There can be more than one mix preset, but we only care about the first one (which appears to be the one that's always active when loading a file)
	if(auto chunk = chunks.GetChunk(GT2Chunk::idMIXP); chunk.CanRead(sizeof(GT2MixPreset)))
	{
		GT2MixPreset mixPreset;
		chunk.ReadStruct(mixPreset);
		if(mixPreset.trackType == 4 && mixPreset.version == 0x101)
		{
			m_nDefaultGlobalVolume = Util::muldivr_unsigned(std::min(mixPreset.volWet.get(), uint16(0x4000)), MAX_GLOBAL_VOLUME, 0x1000);
			for(uint16 i = 0 ; i < mixPreset.numTracks; i++)
			{
				GT2MixPresetTrack mixTrack;
				chunk.ReadStruct(mixTrack);
				if(mixTrack.trackType == 0 && mixTrack.trackIndex < GetNumChannels())
				{
					auto &chnSetting = ChnSettings[mixTrack.trackIndex];
					chnSetting.nPan = static_cast<uint16>(Util::muldivr_unsigned(mixTrack.balance, 256, 0xFFF));
					chnSetting.nVolume = static_cast<uint8>(Util::muldivr_unsigned(mixTrack.volume, 64, 0x1000));
				}
			}
		}
	}

	for(auto &smpChunk : chunks.GetAllChunks(GT2Chunk::idSAM2))
	{
		GT2SampleV2 sample;
		if(!smpChunk.ReadStruct(sample)
			|| !sample.smpNum
			|| sample.smpNum >= MAX_SAMPLES
			|| sample.sampleCoding > 1
			|| sample.type > 1)
			continue;

		if(sample.smpNum > m_nSamples)
			m_nSamples = sample.smpNum;

		ModSample &mptSmp = Samples[sample.smpNum];
		sample.ConvertToMPT(mptSmp);
		m_szNames[sample.smpNum] = mpt::String::ReadBuf(mpt::String::spacePadded, sample.name);

		if((loadFlags & loadSampleData) && smpChunk.Seek(sample.dataOffset - 8))
		{
			if(sample.type == 0)
				sample.GetSampleFormat().ReadSample(mptSmp, smpChunk);

			std::string filenameU8;
			smpChunk.ReadString<mpt::String::maybeNullTerminated>(filenameU8, sample.filenameLen);
			mptSmp.filename = filenameU8;

			if(sample.type == 1)
			{
#ifdef MPT_EXTERNAL_SAMPLES
#if MPT_OS_WINDOWS
				std::transform(filenameU8.begin(), filenameU8.end(), filenameU8.begin(), [] (char c) { if(c == '/') return '\\'; else return c; });
#endif // MPT_OS_WINDOWS
				SetSamplePath(sample.smpNum, mpt::PathString::FromUTF8(filenameU8));
				mptSmp.uFlags.set(SMP_KEEPONDISK);
#else
				AddToLog(LogWarning, MPT_UFORMAT("Loading external sample {} ('{}') failed: External samples are not supported.")(sample.smpNum, mpt::ToUnicode(mpt::Charset::UTF8, filenameU8)));
#endif // MPT_EXTERNAL_SAMPLES
			}
		}

#if 0
		if(fileHeader.fileVersion >= 9)
		{
			if(!(loadFlags & loadSampleData))
			{
				smpChunk.Seek(sample.dataOffset - 8 + sample.filenameLen);
				if(sample.type == 0)
					smpChunk.Skip(sample.GetSampleFormat().CalculateEncodedSize(mptSmp.nLength));
			}
			if(smpChunk.Seek((smpChunk.GetPosition() + 3u) & ~3u))
			{
				const uint16 version = smpChunk.ReadUint16BE();
				if(version <= 1)
				{
					const uint16 sampleGroup = smpChunk.ReadUint16BE();
				}
			}
		}
#endif
	}

	for(auto &smpChunk : chunks.GetAllChunks(GT2Chunk::idSAMP))
	{
		GT2SampleV1 sample;
		if(!smpChunk.ReadStruct(sample)
			|| !sample.smpNum
			|| sample.smpNum >= MAX_SAMPLES
			|| sample.sampleCoding != 0)
			continue;

		if(sample.smpNum > m_nSamples)
			m_nSamples = sample.smpNum;

		ModSample &mptSmp = Samples[sample.smpNum];
		sample.ConvertToMPT(mptSmp);
		m_szNames[sample.smpNum] = mpt::String::ReadBuf(mpt::String::spacePadded, sample.name);
		if(loadFlags & loadSampleData)
			sample.GetSampleFormat().ReadSample(mptSmp, smpChunk);
	}

	auto volEnvChunks = chunks.GetAllChunks(GT2Chunk::idVENV), pitchEnvChunks = chunks.GetAllChunks(GT2Chunk::idTENV), panEnvChunks = chunks.GetAllChunks(GT2Chunk::idPENV);
	for(auto &insChunk : chunks.GetAllChunks(GT2Chunk::idINST))
	{
		GT2Instrument instr;
		ModInstrument *mptIns;
		if(!insChunk.ReadStruct(instr)
			|| instr.type != 0
			|| (mptIns = AllocateInstrument(instr.insNum)) == nullptr)
			continue;

		if(instr.insNum > m_nInstruments)
			m_nInstruments = instr.insNum;
		mptIns->name = mpt::String::ReadBuf(mpt::String::spacePadded, instr.name);
		mptIns->midiPWD = 2;
		if(instr.defaultPan > 0)
		{
			mptIns->dwFlags.set(INS_SETPANNING);
			mptIns->nPan = Util::muldivr_unsigned(instr.defaultPan, 256, 4095);
		}
		for(uint8 i = 0; i < std::min(std::size(mptIns->Keyboard), std::size(instr.samples)); i++)
		{
			mptIns->Keyboard[i] = instr.samples[i].num;
			mptIns->NoteMap[i] = static_cast<uint8>(Clamp(mptIns->NoteMap[i] + instr.samples[i].transpose, NOTE_MIN, NOTE_MAX));
		}
		for(SAMPLEINDEX smp : mptIns->GetSamples())
		{
			if(smp > 0 && smp <= GetNumSamples())
				Samples[smp].nVolume = instr.defaultVelocity;
		}

		mptIns->SetCutoff(0x7F, true);
		mptIns->SetResonance(0, true);

		if(instr.version == 0)
		{
			// Old "envelopes"
			mptIns->nFadeOut = instr.volEnv ? 0 : 32768;
			ConvertGT2Envelope(mptIns->synth, instr.volEnv, volEnvChunks);
			ConvertGT2Envelope(mptIns->synth, instr.panEnv, panEnvChunks);
			ConvertGT2Envelope(mptIns->synth, instr.toneEnv, pitchEnvChunks);
		} else
		{
			GT2InstrumentExt instrExt;
			insChunk.ReadStructPartial(instrExt, (instr.version == 1) ? offsetof(GT2InstrumentExt, pitchPanCenter) : sizeof(GT2InstrumentExt));

			const bool filterEnabled = (instrExt.filterFlags & GT2InstrumentExt::fltEnabled);
			if(filterEnabled)
			{
				if(auto filterType = (instrExt.filterFlags & GT2InstrumentExt::fltTypeMask); filterType == GT2InstrumentExt::fltTypeLowpass)
					mptIns->filterMode = FilterMode::LowPass;
				else if(filterType == GT2InstrumentExt::fltTypeHighpass)
					mptIns->filterMode = FilterMode::HighPass;
				if(instrExt.filterFlags & GT2InstrumentExt::fltVelToCutoff)
				{
					// Note: we're missing the velocity to cutoff modulation here, and also the envelope is added on top of this in GT2, while we multiply it.
					const double maxCutoff = mpt::parse<double>(std::string{instrExt.maxVelFreq, sizeof(instrExt.maxVelFreq)});
					if(maxCutoff > 20)
						mptIns->SetCutoff(FrequencyToCutOff(maxCutoff), true);
				}
				if(instrExt.filterFlags & GT2InstrumentExt::fltVelToReso)
				{
					const double maxReso = mpt::parse<double>(std::string{instrExt.maxVelReso, sizeof(instrExt.maxVelReso)});
					mptIns->SetResonance(mpt::saturate_round<uint8>(maxReso * 127.0 / 24.0), true);
				}
			}

			for(uint8 env = 0; env < instrExt.numEnvelopes; env++)
			{
				GT2Envelope envelope;
				insChunk.ReadStruct(envelope);

				const EnvelopeType envType[] = {ENV_VOLUME, ENV_PITCH, ENV_PANNING, ENV_PITCH};
				const bool envEnabled[] = {instr.volEnv != 0, instr.toneEnv != 0, instr.panEnv != 0, instr.cutoffEnv != 0 && filterEnabled};
				if(env >= std::size(envType) || !envEnabled[env])
					continue;
				// Only import cutoff envelope if it wouldn't overwrite pitch envelope
				if(env == 3 && mptIns->PitchEnv.dwFlags[ENV_ENABLED])
					continue;
				envelope.ConvertToMPT(*mptIns, envType[env]);
				if(env == 3)
					mptIns->PitchEnv.dwFlags.set(ENV_FILTER);

				if(envType[env] == ENV_VOLUME)
				{
					if(envelope.flags & GT2Envelope::envFadeOut)
						mptIns->nFadeOut = envelope.fadeOut & 0x7FFF;  // High bit = logarithmic flag
					else
						mptIns->nFadeOut = 0;
				} else if(envType[env] == ENV_PITCH && (envelope.flags & GT2Envelope::envLFO))
				{
					static constexpr VibratoType vibTyes[] = {VIB_SINE, VIB_SQUARE, VIB_SINE, VIB_RAMP_UP, VIB_RAMP_DOWN, VIB_RANDOM};
					uint8 sweep = static_cast<uint8>(255 / std::max(envelope.lfoSweep.get(), uint16(1)));
					uint8 depth = mpt::saturate_cast<uint8>(Util::muldivr_unsigned(envelope.lfoDepth, 64, 100));
					uint8 speed = mpt::saturate_cast<uint8>(256 / std::max(envelope.lfoSpeed.get(), uint16(1)));
					PropagateXMAutoVibrato(instr.insNum, vibTyes[envelope.lfoWaveform < std::size(vibTyes) ? envelope.lfoWaveform : 0], sweep, depth, speed);
				}
			}
		}
	}

	auto patterns = chunks.GetAllChunks(GT2Chunk::idPATD);
	Patterns.ResizeArray(static_cast<PATTERNINDEX>(patterns.size()));
	TEMPO currentTempo = Order().GetDefaultTempo();
	uint32 currentSpeed = Order().GetDefaultSpeed();
	for(auto &patChunk : patterns)
	{
		if(!(loadFlags & loadPatternData) || !patChunk.CanRead(24))
			continue;

		const uint16 patNum = patChunk.ReadUint16BE();
		char name[17]{};
		patChunk.ReadString<mpt::String::spacePadded>(name, 16);
		const uint16 codingVersion = patChunk.ReadUint16BE();
		const ROWINDEX numRows = patChunk.ReadUint16BE();
		const CHANNELINDEX numTracks = patChunk.ReadUint16BE();

		if(!patChunk.CanRead(sizeof(GT2PatternCell) * numRows * numTracks))
			continue;
		if(codingVersion > 1 || !Patterns.Insert(patNum, numRows))
			continue;

		Patterns[patNum].SetName(name);
		std::vector<std::pair<ModCommand::NOTE, ModCommand::INSTR>> lastNoteInstr(numTracks, {NOTE_NONE, {}});
		for(ROWINDEX row = 0; row < numRows; row++)
		{
			auto rowBase = Patterns[patNum].GetRow(row);
			for(CHANNELINDEX chn = 0; chn < numTracks; chn++)
			{
				ModCommand dummy;
				ModCommand &m = (chn < GetNumChannels()) ? rowBase[chn] : dummy;

				GT2PatternCell data;
				patChunk.ReadStruct(data);

				if(data.note > 0 && data.note <= NOTE_MAX - NOTE_MIN + 1)
					lastNoteInstr[chn].first = m.note = data.note + NOTE_MIN;
				if(data.instr)
					lastNoteInstr[chn].second = m.instr = data.instr;

				if(data.volume)
				{
					m.volcmd = VOLCMD_VOLUME;
					if(codingVersion == 0)
						m.vol = data.volume / 4u;
					else
						m.vol = (data.volume - 0x10);
				}

				TranslateGraoumfEffect(*this, m, data.effect, data.param, fileHeader.fileVersion, lastNoteInstr[chn].first, lastNoteInstr[chn].second, currentTempo, currentSpeed);
			}
		}
	}

	if(auto chunk = chunks.GetChunk(GT2Chunk::idTNAM); chunk.CanRead(2 + sizeof(GT2TrackName)))
	{
		uint16 numNames = chunk.ReadUint16BE();
		for (uint16 i = 0; i < numNames && chunk.CanRead(sizeof(GT2TrackName)); i++)
		{
			GT2TrackName trackName;
			chunk.ReadStruct(trackName);
			if(trackName.type == 0 && trackName.trackNumber < GetNumChannels())
			{
				ChnSettings[trackName.trackNumber].szName = mpt::String::ReadBuf(mpt::String::spacePadded, trackName.name);
			}
		}
	}

	return true;
}


OPENMPT_NAMESPACE_END

