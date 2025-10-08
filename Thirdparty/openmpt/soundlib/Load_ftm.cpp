/*
 * Load_ftm.cpp
 * ------------
 * Purpose: FTM (Face The Music) loader
 * Notes  : I actually bought a copy of Face The Music with its manual in order to write this loader.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

struct FTMSampleHeader
{
	char  name[30];   // FTM cannot load samples with a filename longer than 29 characters
	uint8 unknown;    // Probably padding. Usually contains null or space (probably overflowing from name)
	uint8 iffOctave;  // Only relevant in song mode
};

MPT_BINARY_STRUCT(FTMSampleHeader, 32)


struct FTMFileHeader
{
	char     magic[4];    // "FTMN"
	uint8    version;     // ...I guess?
	uint8    numSamples;  // 0...63
	uint16be numMeasures;
	uint16be tempo;         // Smaller values = faster, the default of 14209 is ~125.098 BPM (though it seems to fluctuate and *sometimes* it drops to 124.307 BPM?)
	uint8    tonality;      // Not relevant for playback (0 = C/a, 1 = Db/bb, etc.)
	uint8    muteStatus;
	uint8    globalVolume;  // 0...63
	uint8    flags;         // 1 = module (embedded samples), 2 = enable LED filter
	uint8    ticksPerRow;
	uint8    rowsPerMeasure;
	char     title[32];
	char     artist[32];
	uint8    numEffects;
	uint8    padding;

	bool IsValid() const
	{
		return !memcmp(magic, "FTMN", 4)
			&& version == 3
			&& numSamples < 64
			&& tempo >= 0x1000 && tempo <= 0x4FFF
			&& tonality < 12
			&& globalVolume < 64
#if !defined(MPT_EXTERNAL_SAMPLES) && !defined(MPT_BUILD_FUZZER)
			&& (flags & 0x01)
#endif
			&& !(flags & 0xFC)
			&& ticksPerRow >= 1 && ticksPerRow <= 24
			&& rowsPerMeasure >= 4 && rowsPerMeasure <= 96
			&& rowsPerMeasure == 96 / ticksPerRow
			&& numEffects <= 64
			&& padding == 0;
	}

	uint32 GetHeaderMinimumAdditionalSize() const
	{
		return static_cast<uint32>(numSamples * sizeof(FTMSampleHeader) + numEffects * 4);
	}
};

MPT_BINARY_STRUCT(FTMFileHeader, 82)


struct FTMScriptItem
{
	uint8 type;
	std::array<uint8, 3> data;
};

MPT_BINARY_STRUCT(FTMScriptItem, 4)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderFTM(MemoryFileReader file, const uint64 *pfilesize)
{
	FTMFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadFTM(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	FTMFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return false;
	if(!fileHeader.IsValid())
		return false;
	if(!file.CanRead(fileHeader.GetHeaderMinimumAdditionalSize()))
		return false;
	else if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_MOD, 8);
	for(CHANNELINDEX chn = 0; chn < 8; chn++)
	{
		ChnSettings[chn].nPan = (chn < 2 || chn > 5) ? 64 : 192;
		ChnSettings[chn].dwFlags.set(CHN_MUTE, !(fileHeader.muteStatus & (1 << chn)));
	}
	m_SongFlags.set(SONG_LINEARSLIDES | SONG_ISAMIGA | SONG_IMPORTED | SONG_FORMAT_NO_VOLCOL);
	m_playBehaviour.set(kContinueSampleWithoutInstr);
	m_playBehaviour.set(kST3NoMutedChannels);
	m_playBehaviour.set(kApplyUpperPeriodLimit);
	Order().SetDefaultSpeed(fileHeader.ticksPerRow);
	Order().SetDefaultTempo(TEMPO(1777517.482 / fileHeader.tempo));
	m_nDefaultRowsPerMeasure = fileHeader.rowsPerMeasure;
	if(fileHeader.ticksPerRow == 2 || fileHeader.ticksPerRow == 4 || fileHeader.ticksPerRow == 8 || fileHeader.ticksPerRow == 16)
		m_nDefaultRowsPerBeat = m_nDefaultRowsPerMeasure / 3;
	else
		m_nDefaultRowsPerBeat = m_nDefaultRowsPerMeasure / 4;
	m_nDefaultGlobalVolume = Util::muldivr_unsigned(fileHeader.globalVolume, MAX_GLOBAL_VOLUME, 63);
	m_nMinPeriod = 3208;
	m_nMaxPeriod = 5376;
	const bool moduleWithSamples = (fileHeader.flags & 0x01);
	
	m_modFormat.formatName = UL_("Face The Music");
	m_modFormat.type = UL_("ftm");
	m_modFormat.madeWithTracker = UL_("Face The Music");
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;
	
	m_songName = mpt::String::ReadBuf(mpt::String::nullTerminated, fileHeader.title);
	m_songArtist = mpt::ToUnicode(mpt::Charset::Amiga_no_C1, mpt::String::ReadBuf(mpt::String::nullTerminated, fileHeader.artist));

	m_nSamples = fileHeader.numSamples;
	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		Samples[smp].Initialize(MOD_TYPE_MOD);
		FTMSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		const auto name = mpt::String::ReadBuf(mpt::String::nullTerminated, sampleHeader.name);
#if defined(MPT_EXTERNAL_SAMPLES)
		if(!moduleWithSamples && !name.empty() && (loadFlags & loadSampleData))
		{
			const auto filename = mpt::PathString::FromUnicode(mpt::ToUnicode(mpt::Charset::Amiga_no_C1, name));
			bool ok = false;
			if(const auto moduleFilename = file.GetOptionalFileName())
			{
				// Try to load the sample directly, so that we can retain IFF loop points, octave selection and raw samples
				mpt::IO::InputFile f(moduleFilename->GetDirectoryWithDrive() + filename, SettingCacheCompleteFileBeforeLoading());
				if(f.IsValid())
				{
					FileReader sampleFile = GetFileReader(f);
					if(!ReadIFFSample(smp, sampleFile, false, sampleHeader.iffOctave))
					{
						SampleIO(
							SampleIO::_8bit,
							SampleIO::mono,
							SampleIO::littleEndian,
							SampleIO::signedPCM)
							.ReadSample(Samples[smp], sampleFile);
					}
					ok = true;
				}
			}
			if(!ok)
			{
				// Fall back to external sample mechanism. TODO: external sample loading ignores loop data on purpose, and does not support raw samples or IFF octave selection. How do we get it back?
				Samples[smp].uFlags.set(SMP_KEEPONDISK);
				SetSamplePath(smp, filename);
			}
		}
#endif // MPT_EXTERNAL_SAMPLES
		m_szNames[smp] = name;
		Samples[smp].nC5Speed = 8287;
		Samples[smp].RelativeTone = Samples[smp].nFineTune = 0;
	}

	auto &events = m_globalScript;
	constexpr uint16 STOP_INDEX = uint16_max - 1;
	std::map<uint16, uint16> offsetToIndex;
	std::bitset<64> effectDefined;
	for(uint8 effect = 0; effect < fileHeader.numEffects; effect++)
	{
		const auto [numLines, index] = file.ReadArray<uint16be, 2>();
		if(numLines > 0x200 || index > 63 || offsetToIndex.count(index << 10))
			return false;
		effectDefined.set(index);
		events.reserve(events.size() + 4 + numLines);
		events.push_back(InstrumentSynth::Event::Jump(STOP_INDEX));
		events.push_back(InstrumentSynth::Event::JumpMarker(index));
		events.push_back(InstrumentSynth::Event::FTM_SetWorkTrack(uint8_max, false));
		events.push_back(InstrumentSynth::Event::FTM_SetInterrupt(uint16_max, uint8_max));
		
		std::vector<FTMScriptItem> items;
		file.ReadVector(items, numLines);
		for(uint16 i = 0; i < numLines; i++)
		{
			const auto &item = items[i];
			const uint32 u24 = (item.data[0] << 16) | (item.data[1] << 8) | item.data[2];
			const uint16 u16 = static_cast<uint16>(u24 & 0xFFFF);
			const uint16 u12lo = u16 & 0xFFF, u12hi = static_cast<uint16>(u24 >> 12);
			const uint8 u8 = item.data[2];
			const uint16 jumpTarget = static_cast<uint16>((index << 10) | std::min(u12lo, uint16(0x200)));
			offsetToIndex[(index << 10u) | i] = static_cast<uint16>(events.size());
			switch(item.type)
			{
			case 0:  // NOP [......]
				break;
			case 1:  // WAIT t TICKS [..tttt]
				if(u16)
					events.push_back(InstrumentSynth::Event::Delay(u16 - 1));
				else
					events.push_back(InstrumentSynth::Event::StopScript());
				break;
			case 2:  // GOTO LINE l [...lll]
				events.push_back(InstrumentSynth::Event::Jump(jumpTarget));
				break;
			case 3:  // LOOP x TIMES TO l [xxxlll]
				events.push_back(InstrumentSynth::Event::SetLoopCounter(u12hi, false));
				events.push_back(InstrumentSynth::Event::EvaluateLoopCounter(jumpTarget));
				break;
			case 4:  // GOTO EFF e / LINE l [eeelll]
				events.push_back(InstrumentSynth::Event::Jump(static_cast<uint16>((u12hi << 10) | (jumpTarget & 0x3FF))));
				break;
			case 5:  // END OF EFFECT [......]
				events.push_back(InstrumentSynth::Event::Jump(STOP_INDEX));
				break;
			case 6:   // IF PITCH=v GOTO l [ppplll]
			case 7:   // IF PITCH<v GOTO l [ppplll]
			case 8:   // IF PITCH>v GOTO l [ppplll]
			case 9:   // IF VOLUM=v GOTO l [vvvlll]
			case 10:  // IF VOLUM<v GOTO l [vvvlll]
			case 11:  // IF VOLUM>v GOTO l [vvvlll]
				events.push_back(InstrumentSynth::Event::FTM_SetCondition(u12hi, item.type - 6));
				events.push_back(InstrumentSynth::Event::JumpIfTrue(jumpTarget));
				break;
			case 12:  // ON NEWPIT. GOTO l [...lll]
			case 13:  // ON NEWVOL. GOTO l [...lll]
			case 14:  // ON NEWSAM. GOTO l [...lll]
			case 15:  // ON RELEASE GOTO l [...lll]
			case 16:  // ON PB GOTO l [...lll]
			case 17:  // ON VD GOTO l [...lll]
				events.push_back(InstrumentSynth::Event::FTM_SetInterrupt(jumpTarget, 1 << (item.type - 12)));
				break;
			case 18:  // PLAY CURR. SAMPLE [......]
				events.push_back(InstrumentSynth::Event::FTM_PlaySample());
				break;
			case 19:  // PLAY QUIET SAMPLE [......]
				events.push_back(InstrumentSynth::Event::NoteCut());
				break;
			case 20:  // PLAYPOSITION = [.ppppp]
				events.push_back(InstrumentSynth::Event::SampleOffset((u24 & 0xFFFFF) << 1));
				break;
			case 21:  // PLAYPOSITION ADD [.ppppp]
				events.push_back(InstrumentSynth::Event::SampleOffsetAdd((u24 & 0xFFFFF) << 1));
				break;
			case 22:  // PLAYPOSITION SUB [.ppppp]
				events.push_back(InstrumentSynth::Event::SampleOffsetSub((u24 & 0xFFFFF) << 1));
				break;
			case 23:  // PITCH = [...ppp]
				events.push_back(InstrumentSynth::Event::FTM_SetPitch(std::min(u12lo, uint16(0x10F))));
				break;
			case 24:  // DETUNE = [...ddd]
				events.push_back(InstrumentSynth::Event::FTM_SetDetune(u12lo));
				break;
			case 25:  // DETUNE/PITCH ADD [dddppp]
			case 26:  // DETUNE/PITCH SUB [dddppp]
				events.push_back(InstrumentSynth::Event::FTM_AddDetune((item.type == 26) ? -static_cast<int16>(u12hi) : u12hi));
				events.push_back(InstrumentSynth::Event::FTM_AddPitch((item.type == 26) ? -static_cast<int16>(u12lo) : u12lo));
				break;
			case 27:  // VOLUME = [....vv]
				events.push_back(InstrumentSynth::Event::FTM_SetVolume(u8));
				break;
			case 28:  // VOLUME ADD [....vv]
			case 29:  // VOLUME SUB [....vv]
				events.push_back(InstrumentSynth::Event::FTM_AddVolume((item.type == 29) ? -static_cast<int16>(u8) : u8));
				break;
			case 30:  // CURRENT SAMPLE = [....ss]
				events.push_back(InstrumentSynth::Event::FTM_SetSample(u8));
				break;
			case 31:  // SAMPLESTART = [.sssss]
			case 32:  // SAMPLESTART ADD [.sssss]
			case 33:  // SAMPLESTART SUB [.sssss]
				events.push_back(InstrumentSynth::Event::FTM_SetSampleStart(mpt::saturate_cast<uint16>((u24 & 0xFFFFF) >> 1), item.type - 31));
				break;
			case 34:  // ONESHOTLENGTH = [..oooo]
			case 35:  // ONESHOTLENGTH ADD [..oooo]
			case 36:  // ONESHOTLENGTH SUB [..oooo]
				events.push_back(InstrumentSynth::Event::FTM_SetOneshotLength(u16, item.type - 34));
				break;
			case 37:  // REPEATLENGTH = [..rrrr]
			case 38:  // REPEATLENGTH ADD [..rrrr]
			case 39:  // REPEATLENGTH SUB [..rrrr]
				events.push_back(InstrumentSynth::Event::FTM_SetRepeatLength(u16, item.type - 37));
				break;
			case 40:  // GET PIT.OF TRACK [.....t]
			case 41:  // GET VOL.OF TRACK [.....t]
			case 42:  // GET SAM.OF TRACK [.....t]
			case 43:  // CLONE TRACK [.....t]
				events.push_back(InstrumentSynth::Event::FTM_CloneTrack(u8 & 0x07, 1 << (item.type - 40)));
				break;
			case 44:  // 1ST LFO START [mfssdd]
			case 47:  // 2ND LFO START [mfssdd]
			case 50:  // 3RD LFO START [mfssdd]
			case 53:  // 4TH LFO START [mfssdd]
				events.push_back(InstrumentSynth::Event::FTM_StartLFO(static_cast<uint8>((item.type - 44u) / 3u), item.data[0]));
				[[fallthrough]];
			case 45:  // 1ST LFO SP/DE ADD [..ssdd]
			case 46:  // 1ST LFO SP/DE SUB [..ssdd]
			case 48:  // 2ND LFO SP/DE ADD [..ssdd]
			case 49:  // 2ND LFO SP/DE SUB [..ssdd]
			case 51:  // 3RD LFO SP/DE ADD [..ssdd]
			case 52:  // 3RD LFO SP/DE SUB [..ssdd]
			case 54:  // 4TH LFO SP/DE ADD [..ssdd]
			case 55:  // 4TH LFO SP/DE SUB [..ssdd]
				events.push_back(InstrumentSynth::Event::FTM_LFOAddSub(static_cast<uint8>(((item.type - 44u) / 3u) | (((item.type - 44u) % 3u == 2) ? 4 : 0)), item.data[1], item.data[2]));
				break;
			case 56:  // WORK ON TRACK t [.....t]
			case 57:  // WORKTRACK ADD [.....t]
				events.push_back(InstrumentSynth::Event::FTM_SetWorkTrack(u8 & 0x07, item.type == 57));
				break;
			case 58:  // GLOBAL VOLUME = [....vv]
				events.push_back(InstrumentSynth::Event::FTM_SetGlobalVolume(mpt::saturate_cast<uint16>(Util::muldivr_unsigned(u8, MAX_GLOBAL_VOLUME, 64))));
				break;
			case 59:  // GLOBAL SPEED = [..ssss]
				events.push_back(InstrumentSynth::Event::FTM_SetTempo(u16));
				break;
			case 60:  // TICKS PER LINE = [....tt]
				events.push_back(InstrumentSynth::Event::FTM_SetSpeed(u8));
				break;
			case 61:  // JUMP TO SONGLINE [..llll]
				events.push_back(InstrumentSynth::Event::FTM_SetPlayPosition(u16 / fileHeader.rowsPerMeasure, static_cast<uint8>(u16 % fileHeader.rowsPerMeasure)));
				break;
			}
		}
	}
	if(fileHeader.numEffects)
	{
		events.push_back(InstrumentSynth::Event::JumpMarker(64));
		offsetToIndex[STOP_INDEX] = static_cast<uint16>(events.size());
		events.push_back(InstrumentSynth::Event::FTM_SetInterrupt(uint16_max, uint8_max));
	}
	for(auto &event : events)
	{
		event.FixupJumpTarget(offsetToIndex);
	}

	if(fileHeader.numMeasures)
	{
		Patterns.ResizeArray(fileHeader.numMeasures);
		Order().resize(fileHeader.numMeasures);
		for(uint16 measure = 0; measure < fileHeader.numMeasures; measure++)
		{
			Order()[measure] = measure;
			if(!Patterns.Insert(measure, fileHeader.rowsPerMeasure))
				return false;
		}

		struct PatternLoopPoint
		{
			ORDERINDEX order;
			uint16 param : 6, channel : 3, row : 7;
			bool operator<(const PatternLoopPoint other) const noexcept { return std::tie(order, row, channel) < std::tie(other.order, other.row, other.channel); }
		};
		std::vector<PatternLoopPoint> loopPoints;
		for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
		{
			const uint16 defaultSpacing = file.ReadUint16BE();
			FileReader channelChunk = file.ReadChunk(file.ReadUint32BE());
			if(!(loadFlags & loadPatternData))
				continue;

			uint32 globalRow = 0;
			uint16 spacing = defaultSpacing;
			while(channelChunk.CanRead(2))
			{
				const auto data = channelChunk.ReadArray<uint8, 2>();
				if((data[0] & 0xF0) == 0xF0)
				{
					spacing = data[1] | ((data[0] & 0x0F) << 8);
					continue;
				}

				const auto position = std::div(globalRow + spacing, fileHeader.rowsPerMeasure);
				if(position.quot >= fileHeader.numMeasures)
					break;
				const PATTERNINDEX pat = static_cast<PATTERNINDEX>(position.quot);

				ModCommand &m = *Patterns[pat].GetpModCommand(position.rem, chn);
				const uint8 param = ((data[0] & 0x0F) << 2) | (data[1] >> 6);  // 0...63
				switch(data[0] & 0xF0)
				{
				case 0x00:  // Set instrument, no effect
					m.instr = param;
					break;
				case 0xB0:  // SEL effect
					m.SetEffectCommand(CMD_MED_SYNTH_JUMP, (param < effectDefined.size() && effectDefined[param]) ? param : 64);
					break;
				case 0xC0:  // Pitch bend
					m.SetEffectCommand(CMD_TONEPORTA_DURATION, param);
					break;
				case 0xD0:  // Volume down
					m.SetEffectCommand(CMD_VOLUMEDOWN_DURATION, param);
					break;
				case 0xE0:  // Loop
					loopPoints.push_back({pat, static_cast<uint8>(param & 0x3F), static_cast<uint8>(chn & 0x07), static_cast<uint8>(position.rem & 0x7F)});
					break;
				case 0xF0:  // Already handled
					break;
				default:
					m.SetEffectCommand(CMD_CHANNELVOLUME, static_cast<uint8>(Util::muldivr_unsigned((data[0] >> 4) - 1, 64, 9)));
					m.instr = param;
					break;
				}
				if(uint8 note = data[1] & 0x3F; note >= 35)
					m.note = NOTE_KEYOFF;
				else if(note != 0)
					m.note = NOTE_MIDDLEC - 13 + note;
				globalRow += 1 + spacing;
				spacing = defaultSpacing;
			}
		}

		if(Patterns.IsValidPat(0) && (fileHeader.flags & 0x02))
		{
			Patterns[0].WriteEffect(EffectWriter(CMD_MODCMDEX, 0).Row(0).RetryNextRow());
		}

		// Evaluate pattern loops (there can be 16 nested loops at most)
		std::sort(loopPoints.begin(), loopPoints.end());
		std::array<std::pair<ORDERINDEX, uint8>, 16> loopStart;
		ORDERINDEX ordersInserted = 0;
		uint8 activeLoops = 0;
		bool canAddMore = true;
		for(const auto &loop : loopPoints)
		{
			ORDERINDEX ord = loop.order + ordersInserted;
			if(loop.param && activeLoops < loopStart.size())
			{
				loopStart[activeLoops++] = std::make_pair(ord, static_cast<uint8>(loop.param));
			} else if(!loop.param && activeLoops > 0)
			{
				activeLoops--;
				const auto start = loopStart[activeLoops].first;
				const std::vector<PATTERNINDEX> ordersToCopy(Order().begin() + start, Order().begin() + ord + 1);
				for(uint8 rep = 1; rep < loopStart[activeLoops].second && canAddMore; rep++)
				{
					if(ORDERINDEX inserted = Order().insert(ord + 1, mpt::as_span(ordersToCopy), false); inserted == ordersToCopy.size())
						ordersInserted += inserted;
					else
						canAddMore = false;
				}
				if(!canAddMore)
					break;
			}
		}
	}

	if((loadFlags & loadSampleData) && moduleWithSamples)
	{
		for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
		{
			if(!m_szNames[smp][0])
				continue;
			ModSample &mptSmp = Samples[smp];
			SmpLength loopStart = file.ReadUint16BE() * 2u;
			SmpLength loopLength = file.ReadUint16BE() * 2u;
			mptSmp.nLength = loopStart + loopLength;
			if(!mptSmp.nLength)
				continue;
			if(loopLength)
			{
				mptSmp.nLoopStart = loopStart;
				mptSmp.nLoopEnd = loopStart + loopLength;
				mptSmp.uFlags.set(CHN_LOOP);
			}
			SampleIO(
				SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::signedPCM)
				.ReadSample(mptSmp, file);
		}
	}

	return true;
}

OPENMPT_NAMESPACE_END
