/*
 * Load_sfx.cpp
 * ------------
 * Purpose: SFX / MMS (SoundFX / MultiMedia Sound) module loader
 * Notes  : Mostly based on the Soundtracker loader, some effect behavior is based on Flod's implementation.
 * Authors: Devin Acker
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "Loaders.h"
#include "Tables.h"

OPENMPT_NAMESPACE_BEGIN

// File Header
struct SFXFileHeader
{
	char     magic[4];
	uint16be speed;
	char     unknown[14];  // Just NUL bytes most of the time, sometimes appears to contain leftovers from other data structures

	bool IsValid(const uint8 expectedNumSamples) const noexcept
	{
		if(expectedNumSamples == 15 && memcmp(magic, "SONG", 4))
			return false;
		if(expectedNumSamples == 31 && memcmp(magic, "SO31", 4))
			return false;
		return speed >= 178;
	}
};

MPT_BINARY_STRUCT(SFXFileHeader, 20)


// Order List
struct SFXOrderHeader
{
	uint8 numOrders;
	uint8 restartPos;
	uint8 orderList[128];

	bool IsValid() const noexcept
	{
		return numOrders <= 128;
	}
};

MPT_BINARY_STRUCT(SFXOrderHeader, 130)


// Sample Header
struct SFXSampleHeader
{
	char     name[22];
	uint16be oneshotLength;  // For unlooped samples, this is quite frequently 2 bytes shorter than the sample data length (and the last two samples would cause a click to be heard)
	uint8be  finetune;
	uint8be  volume;
	uint16be loopStart;
	uint16be loopLength;

	// Convert an SFX sample header to OpenMPT's internal sample header.
	void ConvertToMPT(ModSample &mptSmp, uint32 length) const
	{
		mptSmp.Initialize(MOD_TYPE_MOD);
		mptSmp.nLength = (loopLength > 1) ? length : (oneshotLength * 2u);
		mptSmp.nFineTune = MOD2XMFineTune(finetune);
		mptSmp.nVolume = 4u * std::min(volume.get(), uint8(64));

		SmpLength lStart = loopStart;
		SmpLength lLength = loopLength * 2u;

		if(mptSmp.nLength)
		{
			mptSmp.nLoopStart = lStart;
			mptSmp.nLoopEnd = lStart + lLength;

			if(mptSmp.nLoopStart >= mptSmp.nLength)
			{
				mptSmp.nLoopStart = mptSmp.nLength - 1;
			}
			if(mptSmp.nLoopEnd > mptSmp.nLength)
			{
				mptSmp.nLoopEnd = mptSmp.nLength;
			}
			if(mptSmp.nLoopStart > mptSmp.nLoopEnd || mptSmp.nLoopEnd < 4 || mptSmp.nLoopEnd - mptSmp.nLoopStart < 4)
			{
				mptSmp.nLoopStart = 0;
				mptSmp.nLoopEnd = 0;
			}

			if(mptSmp.nLoopEnd > mptSmp.nLoopStart)
			{
				mptSmp.uFlags.set(CHN_LOOP);
			}
		}
	}
};

MPT_BINARY_STRUCT(SFXSampleHeader, 30)

static uint8 ClampSlideParam(uint8 value, uint8 lowNote, uint8 highNote)
{
	uint16 lowPeriod, highPeriod;

	if(lowNote  < highNote &&
	   lowNote  >= 24 + NOTE_MIN &&
	   highNote >= 24 + NOTE_MIN &&
	   lowNote  < std::size(ProTrackerPeriodTable) + 24 + NOTE_MIN &&
	   highNote < std::size(ProTrackerPeriodTable) + 24 + NOTE_MIN)
	{
		lowPeriod  = ProTrackerPeriodTable[lowNote - 24 - NOTE_MIN];
		highPeriod = ProTrackerPeriodTable[highNote - 24 - NOTE_MIN];

		// with a fixed speed of 6 ticks/row, and excluding the first row,
		// 1xx/2xx param has a max value of (low-high)/5 to avoid sliding too far
		return std::min(value, static_cast<uint8>((lowPeriod - highPeriod) / 5));
	}

	return 0;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderSFX(MemoryFileReader file, const uint64 *pfilesize)
{
	SAMPLEINDEX numSamples = 0;
	SFXFileHeader fileHeader;

	if(!file.LengthIsAtLeast(0x3C + sizeof(SFXFileHeader)))
		return ProbeWantMoreData;
	if(file.Seek(0x3C) && file.ReadStruct(fileHeader) && fileHeader.IsValid(15))
	{
		numSamples = 15;
	} else
	{
		if(!file.LengthIsAtLeast(0x7C + sizeof(SFXFileHeader)))
			return ProbeWantMoreData;
		if(file.Seek(0x7C) && file.ReadStruct(fileHeader) && fileHeader.IsValid(31))
			numSamples = 31;
		else
			return ProbeFailure;
	}
	
	file.Rewind();
	for(SAMPLEINDEX smp = 0; smp < numSamples; smp++)
	{
		if(file.ReadUint32BE() > 131072)
			return ProbeFailure;
	}

	if(!file.Skip(sizeof(SFXFileHeader) + sizeof(SFXSampleHeader) * numSamples))
		return ProbeWantMoreData;

	SFXOrderHeader orderHeader;
	if(!file.ReadStruct(orderHeader))
		return ProbeWantMoreData;
	if(!orderHeader.IsValid())
		return ProbeFailure;

	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadSFX(FileReader &file, ModLoadingFlags loadFlags)
{
	SFXFileHeader fileHeader;
	if(file.Seek(0x3C) && file.ReadStruct(fileHeader) && fileHeader.IsValid(15))
	{
		InitializeGlobals(MOD_TYPE_SFX, 4);
		m_nSamples = 15;
	} else if(file.Seek(0x7C) && file.ReadStruct(fileHeader) && fileHeader.IsValid(31))
	{
		InitializeGlobals(MOD_TYPE_SFX, 4);
		m_nSamples = 31;
	} else
	{
		return false;
	}

	uint32 sampleLen[31];

	file.Rewind();
	for(SAMPLEINDEX smp = 0; smp < m_nSamples; smp++)
	{
		sampleLen[smp] = file.ReadUint32BE();
		if(sampleLen[smp] > 131072)
			return false;
	}
	file.Skip(sizeof(SFXFileHeader));

	m_nInstruments = 0;
	Order().SetDefaultTempo(TEMPO((14565.0 * 122.0) / fileHeader.speed));
	Order().SetDefaultSpeed(6);
	m_nMinPeriod = 14 * 4;
	m_nMaxPeriod = 3424 * 4;
	m_nSamplePreAmp = 64;

	// Setup channel pan positions and volume
	SetupMODPanning(true);

	uint32 invalidChars = 0;
	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		SFXSampleHeader sampleHeader;

		file.ReadStruct(sampleHeader);
		// cppcheck false-positive
		// cppcheck-suppress uninitvar
		sampleHeader.ConvertToMPT(Samples[smp], sampleLen[smp - 1]);

		// Get rid of weird characters in sample names.
		for(char &c : sampleHeader.name)
		{
			if(c > 0 && c < ' ')
			{
				c = ' ';
				invalidChars++;
			}
		}
		if(invalidChars >= 128)
			return false;
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::spacePadded, sampleHeader.name);
	}

	// Broken conversions of the "Operation Stealth" soundtrack (BOND23 / BOND32)
	// There is a converter that shifts all note values except FFFD (empty note) to the left by 1 bit,
	// but it should not do that for FFFE (STP) notes - as a consequence, they turn into pattern breaks (FFFC).
	const bool fixPatternBreaks = (m_szNames[1] == "BASSE2.AMI") || (m_szNames[1] == "PRA1.AMI");

	SFXOrderHeader orderHeader;
	if(!file.ReadStruct(orderHeader) || !orderHeader.IsValid())
		return false;
	else if(loadFlags == onlyVerifyHeader)
		return true;

	PATTERNINDEX numPatterns = 0;
	for(ORDERINDEX ord = 0; ord < orderHeader.numOrders; ord++)
	{
		numPatterns = std::max(numPatterns, static_cast<PATTERNINDEX>(orderHeader.orderList[ord] + 1));
	}

	if(orderHeader.restartPos < orderHeader.numOrders)
		Order().SetRestartPos(orderHeader.restartPos);
	else
		Order().SetRestartPos(0);

	ReadOrderFromArray(Order(), orderHeader.orderList, orderHeader.numOrders);

	// SFX v2 / MMS modules have 4 extra bytes here for some reason
	if(m_nSamples == 31)
		file.Skip(4);

	uint8 lastNote[4] = {0};
	uint8 slideTo[4] = {0};
	uint8 slideRate[4] = {0};
	uint8 version = 0;

	// Reading patterns
	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(numPatterns);
	for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
	{
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
		{
			file.Skip(64 * 4 * 4);
			continue;
		}

		for(ROWINDEX row = 0; row < 64; row++)
		{
			auto rowBase = Patterns[pat].GetRow(row);
			for(CHANNELINDEX chn = 0; chn < 4; chn++)
			{
				ModCommand &m = rowBase[chn];
				auto data = file.ReadArray<uint8, 4>();

				if(data[0] == 0xFF)
				{
					lastNote[chn] = slideRate[chn] = 0;

					if(fixPatternBreaks && data[1] == 0xFC)
						data[1] = 0xFE;

					switch(data[1])
					{
					case 0xFE: // STP (note cut)
						m.command = CMD_VOLUME;
						continue;
					case 0xFD: // PIC (null)
						continue;
					case 0xFC: // BRK (pattern break)
						m.command = CMD_PATTERNBREAK;
						version = 9;
						continue;
					}
				}

				const auto [command, param] = ReadMODPatternEntry(data, m);
				if(m.note != NOTE_NONE)
				{
					lastNote[chn] = m.note;
					slideRate[chn] = 0;
					if(m.note < NOTE_MIDDLEC - 12)
					{
						version = std::max(version, uint8(8));
					}
				}

				if(command || param)
				{
					m.param = param;
					switch(command)
					{
					case 0x1: // Arpeggio
						m.command = CMD_ARPEGGIO;
						break;

					case 0x2: // Portamento (like Ultimate Soundtracker)
						if(m.param & 0xF0)
						{
							m.command = CMD_PORTAMENTODOWN;
							m.param >>= 4;
						} else if(m.param & 0xF)
						{
							m.command = CMD_PORTAMENTOUP;
							m.param &= 0x0F;
						} else
						{
							m.command = CMD_NONE;
						}
						break;

					case 0x3: // Enable LED filter
						// Give precedence to 7xy/8xy slides
						if(slideRate[chn])
						{
							m.command = CMD_NONE;
							break;
						}
						m.SetEffectCommand(CMD_MODCMDEX, 0x00);
						break;

					case 0x4: // Disable LED filter
						// Give precedence to 7xy/8xy slides
						if(slideRate[chn])
						{
							m.command = CMD_NONE;
							break;
						}
						m.SetEffectCommand(CMD_MODCMDEX, 0x01);
						break;

					case 0x5: // Increase volume
						if(m.instr)
						{
							m.SetEffectCommand(CMD_VOLUME, std::min(ModCommand::PARAM(0x3F), static_cast<ModCommand::PARAM>((Samples[m.instr].nVolume / 4u) + m.param)));

							// Give precedence to 7xy/8xy slides (and move this to the volume column)
							if(slideRate[chn])
							{
								m.SetVolumeCommand(VOLCMD_VOLUME, m.param);
								m.command = CMD_NONE;
								break;
							}
						} else
						{
							m.command = CMD_NONE;
						}
						break;

					case 0x6: // Decrease volume
						if(m.instr)
						{
							m.command = CMD_VOLUME;
							if((Samples[m.instr].nVolume / 4u) >= m.param)
								m.param = static_cast<ModCommand::PARAM>(Samples[m.instr].nVolume / 4u) - m.param;
							else
								m.param = 0;

							// Give precedence to 7xy/8xy slides (and move this to the volume column)
							if(slideRate[chn])
							{
								m.SetVolumeCommand(VOLCMD_VOLUME, m.param);
								m.command = CMD_NONE;
								break;
							}
						} else
						{
							m.command = CMD_NONE;
						}
						break;

					case 0x7: // 7xy: Slide down x semitones at speed y
						slideTo[chn] = lastNote[chn] - (m.param >> 4);

						slideRate[chn] = m.param & 0xF;
						m.SetEffectCommand(CMD_PORTAMENTODOWN, ClampSlideParam(slideRate[chn], slideTo[chn], lastNote[chn]));
						break;

					case 0x8: // 8xy: Slide up x semitones at speed y
						slideTo[chn] = lastNote[chn] + (m.param >> 4);

						slideRate[chn] = m.param & 0xF;
						m.SetEffectCommand(CMD_PORTAMENTOUP, ClampSlideParam(slideRate[chn], lastNote[chn], slideTo[chn]));
						break;

					case 0x9: // 9xy: Auto slide
						version = std::max(version, uint8(8));
						[[fallthrough]];
					default:
						m.command = CMD_NONE;
						break;
					}
				}

				// Continue 7xy/8xy slides if needed
				if(m.command == CMD_NONE && slideRate[chn])
				{
					if(slideTo[chn])
					{
						m.note = lastNote[chn] = slideTo[chn];
						m.param = slideRate[chn];
						slideTo[chn] = 0;
					}
					m.command = CMD_TONEPORTAMENTO;
				}
			}
		}
	}

	// Reading samples
	if(loadFlags & loadSampleData)
	{
		for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
		{
			if(!sampleLen[smp - 1])
				continue;

			FileReader chunk = file.ReadChunk(sampleLen[smp - 1]);
			SampleIO(
				SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::signedPCM)
				.ReadSample(Samples[smp], chunk);
		}
	}

	m_modFormat.formatName = m_nSamples == 15 ? MPT_UFORMAT("SoundFX 1.{}")(version) : UL_("SoundFX 2.0 / MultiMedia Sound");
	m_modFormat.type = m_nSamples == 15 ? UL_("sfx") : UL_("sfx2");
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	return true;
}

OPENMPT_NAMESPACE_END
