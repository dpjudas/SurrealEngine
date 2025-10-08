/*
 * Load_stk.cpp
 * ------------
 * Purpose: M15 / STK (Ultimate Soundtracker / Soundtracker) loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "MODTools.h"

OPENMPT_NAMESPACE_BEGIN

// We'll have to do some heuristic checks to find out whether this is an old Ultimate Soundtracker module
// or if it was made with the newer Soundtracker versions.
// Thanks for Fraggie for this information! (https://www.un4seen.com/forum/?topic=14471.msg100829#msg100829)
enum STVersions
{
	UST1_00,              // Ultimate Soundtracker 1.0-1.21 (K. Obarski)
	UST1_80,              // Ultimate Soundtracker 1.8-2.0 (K. Obarski)
	ST2_00_Exterminator,  // SoundTracker 2.0 (The Exterminator), D.O.C. Sountracker II (Unknown/D.O.C.)
	ST_III,               // Defjam Soundtracker III (Il Scuro/Defjam), Alpha Flight SoundTracker IV (Alpha Flight), D.O.C. SoundTracker IV (Unknown/D.O.C.), D.O.C. SoundTracker VI (Unknown/D.O.C.)
	ST_IX,                // D.O.C. SoundTracker IX (Unknown/D.O.C.)
	MST1_00,              // Master Soundtracker 1.0 (Tip/The New Masters)
	ST2_00,               // SoundTracker 2.0, 2.1, 2.2 (Unknown/D.O.C.)
};


struct STKFileHeaders
{
	char            songname[20];
	MODSampleHeader sampleHeaders[15];
	MODFileHeader   fileHeader;
};

MPT_BINARY_STRUCT(STKFileHeaders, 20 + 15 * 30 + 130)


static bool ValidateHeader(const STKFileHeaders &fileHeaders)
{
	// In theory, sample and song names should only ever contain printable ASCII chars and null.
	// However, there are quite a few SoundTracker modules in the wild with random
	// characters. To still be able to distguish them from other formats, we just reject
	// files with *too* many bogus characters. Arbitrary threshold: 48 bogus characters in total
	// or more than 5 invalid characters just in the title alone
	uint32 invalidCharsInTitle = CountInvalidChars(fileHeaders.songname);
	uint32 invalidChars = invalidCharsInTitle;

	SmpLength totalSampleLen = 0;
	uint8 allVolumes = 0;
	uint8 validNameCount = 0;
	bool invalidNames = false;

	for(SAMPLEINDEX smp = 0; smp < 15; smp++)
	{
		const MODSampleHeader &sampleHeader = fileHeaders.sampleHeaders[smp];

		invalidChars += CountInvalidChars(sampleHeader.name);

		// schmokk.mod has a non-zero value here but it should not be treated as finetune
		if(sampleHeader.finetune != 0)
			invalidChars += 16;
		if(const auto nameType = ClassifyName(sampleHeader.name); nameType == NameClassification::ValidASCII)
			validNameCount++;
		else if(nameType == NameClassification::Invalid)
			invalidNames = true;

		// Sanity checks - invalid character count adjusted for ata.mod (MD5 937b79b54026fa73a1a4d3597c26eace, SHA1 3322ca62258adb9e0ae8e9afe6e0c29d39add874)
		// Sample length adjusted for romantic.stk which has a (valid) sample of length 72222
		if(invalidChars > 48
		   || sampleHeader.volume > 64
		   || sampleHeader.length > 37000)
		{
			return false;
		}

		totalSampleLen += sampleHeader.length;
		allVolumes |= sampleHeader.volume;
	}

	// scramble_2.mod has a lot of garbage in the song title, but it has lots of properly-formatted sample names, so we consider those to be more important than the garbage bytes.
	if(invalidCharsInTitle > 5 && (validNameCount < 4 || invalidNames))
		return false;

	// Reject any files with no (or only silent) samples at all, as this might just be a random binary file (e.g. ID3 tags with tons of padding)
	if(totalSampleLen == 0 || allVolumes == 0)
		return false;

	// Sanity check: No more than 128 positions. ST's GUI limits tempo to [1, 220].
	// There are some mods with a tempo of 0 (explora3-death.mod) though, so ignore the lower limit.
	if(fileHeaders.fileHeader.numOrders > 128 || fileHeaders.fileHeader.restartPos > 220)
		return false;

	uint8 maxPattern = *std::max_element(std::begin(fileHeaders.fileHeader.orderList), std::end(fileHeaders.fileHeader.orderList));
	// Sanity check: 64 patterns max.
	if(maxPattern > 63)
		return false;

	// No playable song, and lots of null values => most likely a sparse binary file but not a module
	if(fileHeaders.fileHeader.restartPos == 0 && fileHeaders.fileHeader.numOrders == 0 && maxPattern == 0)
		return false;

	return true;
}


template <typename TFileReader>
static bool ValidateFirstSTKPattern(TFileReader &file)
{
	// threshold is chosen as: [threshold for all patterns combined] / [max patterns] * [margin, do not reject too much]
	return ValidateMODPatternData(file, 512 / 64 * 2, false);
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderSTK(MemoryFileReader file, const uint64 *pfilesize)
{
	STKFileHeaders fileHeaders;
	if(!file.ReadStruct(fileHeaders))
		return ProbeWantMoreData;
	if(!ValidateHeader(fileHeaders))
		return ProbeFailure;
	if(!file.CanRead(sizeof(MODPatternData)))
		return ProbeWantMoreData;
	if(!ValidateFirstSTKPattern(file))
		return ProbeFailure;
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadSTK(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	STKFileHeaders fileHeaders;
	if(!file.ReadStruct(fileHeaders))
		return false;
	if(!ValidateHeader(fileHeaders))
		return false;
	if(!ValidateFirstSTKPattern(file))
		return false;
	file.Seek(sizeof(STKFileHeaders));

	InitializeGlobals(MOD_TYPE_MOD, 4);
	m_playBehaviour.reset(kMODOneShotLoops);
	m_playBehaviour.set(kMODIgnorePanning);
	m_playBehaviour.set(kMODSampleSwap);  // untested

	STVersions minVersion = UST1_00;

	bool hasDiskNames = true;
	SmpLength totalSampleLen = 0;
	m_nSamples = 15;

	for(SAMPLEINDEX smp = 1; smp <= 15; smp++)
	{
		ModSample &mptSmp = Samples[smp];
		const MODSampleHeader &sampleHeader = fileHeaders.sampleHeaders[smp - 1];
		ReadMODSample(sampleHeader, Samples[smp], m_szNames[smp], true);
		mptSmp.nFineTune = 0;

		totalSampleLen += mptSmp.nLength;

		if(sampleHeader.HasDiskName())
		{
			// Ultimate Soundtracker 1.8 and D.O.C. SoundTracker IX always have sample names containing disk names.
			hasDiskNames = false;
		}

		// Loop start is always in bytes, not words, so don't trust the auto-fix magic in the sample header conversion (fixes loop of "st-01:asia" in mod.drag 10)
		if(sampleHeader.loopLength > 1)
		{
			mptSmp.nLoopStart = sampleHeader.loopStart;
			mptSmp.nLoopEnd = sampleHeader.loopStart + sampleHeader.loopLength * 2;
			mptSmp.SanitizeLoops();
		}

		// UST only handles samples up to 9999 bytes. Master Soundtracker 1.0 and SoundTracker 2.0 introduce 32KB samples.
		if(sampleHeader.length > 4999 || sampleHeader.loopStart > 9999)
			minVersion = std::max(minVersion, MST1_00);
	}

	MODFileHeader &fileHeader = fileHeaders.fileHeader;
	ReadOrderFromArray(Order(), fileHeader.orderList);
	PATTERNINDEX numPatterns = GetNumPatterns(file, *this, fileHeader.numOrders, totalSampleLen, 0, true);

	// Most likely just a file with lots of NULs at the start
	if(fileHeader.restartPos == 0 && fileHeader.numOrders == 0 && numPatterns <= 1)
	{
		return false;
	}

	// Let's see if the file is too small (including some overhead for broken files like sll7.mod or ghostbus.mod)
	std::size_t requiredRemainingDataSize = numPatterns * 64u * 4u * 4u + totalSampleLen;
	if(!file.CanRead(requiredRemainingDataSize - std::min<std::size_t>(requiredRemainingDataSize, 65536u)))
		return false;

	if(loadFlags == onlyVerifyHeader)
		return true;

	// Now we can be pretty sure that this is a valid Soundtracker file. Set up default song settings.
	// explora3-death.mod has a tempo of 0
	if(!fileHeader.restartPos)
		fileHeader.restartPos = 0x78;
	// jjk55 by Jesper Kyd has a weird tempo set, but it needs to be ignored.
	if(!memcmp(fileHeaders.songname, "jjk55", 6))
		fileHeader.restartPos = 0x78;
	// Sample 7 in echoing.mod won't "loop" correctly if we don't convert the VBlank tempo.
	Order().SetDefaultTempoInt(125);
	if(fileHeader.restartPos != 0x78)
	{
		// Convert to CIA timing
		Order().SetDefaultTempo(TEMPO((709379.0 * 125.0 / 50.0) / ((240 - fileHeader.restartPos) * 122.0)));
		if(minVersion > UST1_80)
		{
			// D.O.C. SoundTracker IX re-introduced the variable tempo after some other versions dropped it.
			minVersion = std::max(minVersion, hasDiskNames ? ST_IX : MST1_00);
		} else
		{
			// Ultimate Soundtracker 1.8 adds variable tempo
			minVersion = std::max(minVersion, hasDiskNames ? UST1_80 : ST2_00_Exterminator);
		}
	}
	m_nMinPeriod = 113 * 4;
	m_nMaxPeriod = 856 * 4;
	m_nSamplePreAmp = 64;
	m_SongFlags.set(SONG_PT_MODE | SONG_FORMAT_NO_VOLCOL | SONG_AUTO_VOLSLIDE_STK);
	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeaders.songname);

	// Setup channel pan positions and volume
	SetupMODPanning();

	FileReader::pos_type patOffset = file.GetPosition();

	// Scan patterns to identify Soundtracker versions and reject garbage.
	uint32 illegalBytes = 0, totalNumDxx = 0;
	bool useAutoSlides = false;
	for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
	{
		const bool patternInUse = mpt::contains(Order(), pat);
		uint8 numDxx = 0, autoSlides = 0;
		uint8 emptyCmds = 0;
		MODPatternData patternData;
		file.ReadArray(patternData);
		if(patternInUse)
		{
			illegalBytes += CountMalformedMODPatternData(patternData, false);
			// Reject files that contain a lot of illegal pattern data.
			// STK.the final remix (MD5 5ff13cdbd77211d1103be7051a7d89c9, SHA1 e94dba82a5da00a4758ba0c207eb17e3a89c3aa3)
			// has one illegal byte, so we only reject after an arbitrary threshold has been passed.
			// This also allows to play some rather damaged files like
			// crockets.mod (MD5 995ed9f44cab995a0eeb19deb52e2a8b, SHA1 6c79983c3b7d55c9bc110b625eaa07ce9d75f369)
			// but naturally we cannot recover the broken data.

			// We only check patterns that are actually being used in the order list, because some bad rips of the
			// "operation wolf" soundtrack have 15 patterns for several songs, but the last few patterns are just garbage.
			// Apart from those hidden patterns, the files play fine.
			// Example: operation wolf - wolf1.mod (MD5 739acdbdacd247fbefcac7bc2d8abe6b, SHA1 e6b4813daacbf95f41ce9ec3b22520a2ae07eed8)
			if(illegalBytes > std::max(512u, numPatterns * 128u))
				return false;
		}
		for(ROWINDEX row = 0; row < 64; row++)
		{
			for(CHANNELINDEX chn = 0; chn < 4; chn++)
			{
				const auto &data = patternData[row][chn];
				const uint8 eff = data[2] & 0x0F, param = data[3];
				// Check for empty space between the last Dxx command and the beginning of another pattern
				if(emptyCmds != 0 && !memcmp(data.data(), "\0\0\0\0", 4))
				{
					emptyCmds++;
					if(emptyCmds > 32)
					{
						// Since there is a lot of empty space after the last Dxx command,
						// we assume it's supposed to be a pattern break effect.
						minVersion = ST2_00;
					}
				} else
				{
					emptyCmds = 0;
				}

				switch(eff)
				{
				case 1:
				case 2:
					if(param > 0x1F && minVersion == UST1_80)
					{
						// If a 1xx / 2xx effect has a parameter greater than 0x20, it is assumed to be UST.
						minVersion = hasDiskNames ? UST1_80 : UST1_00;
					} else if(eff == 1 && param > 0 && param < 0x03)
					{
						// This doesn't look like an arpeggio.
						minVersion = std::max(minVersion, ST2_00_Exterminator);
					} else if(eff == 1 && (param == 0x37 || param == 0x47) && minVersion <= ST2_00_Exterminator)
					{
						// This suspiciously looks like an arpeggio.
						// Catch sleepwalk.mod by Karsten Obarski, which has a default tempo of 125 rather than 120 in the header, so gets mis-identified as a later tracker version.
						minVersion = hasDiskNames ? UST1_80 : UST1_00;
					}
					break;
				case 0x0B:
					minVersion = ST2_00;
					break;
				case 0x0C:
				case 0x0D:
				case 0x0E:
					minVersion = std::max(minVersion, ST2_00_Exterminator);
					if(eff == 0x0D)
					{
						emptyCmds = 1;
						if(param == 0 && row == 0)
						{
							// Fix a possible tracking mistake in Blood Money title - who wants to do a pattern break on the first row anyway?
							break;
						}
						numDxx++;
					} else if(eff == 0x0E)
					{
						if(param > 1 || ++autoSlides > 1)
							useAutoSlides = true;
					}
					break;
				case 0x0F:
					minVersion = std::max(minVersion, ST_III);
					break;
				}
			}
		}

		if(numDxx > 0 && numDxx < 3)
		{
			// Not many Dxx commands in one pattern means they were probably pattern breaks
			minVersion = ST2_00;
		}
		totalNumDxx += numDxx;
	}

	// If there is a huge number of Dxx commands, this is extremely unlikely to be a  SoundTracker 2.0 module
	if(totalNumDxx > numPatterns + 32u && minVersion == ST2_00)
		minVersion = MST1_00;

	file.Seek(patOffset);

	// Reading patterns
	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(numPatterns);
	for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
	{
		MODPatternData patternData;
		file.ReadArray(patternData);

		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
		{
			continue;
		}

		for(ROWINDEX row = 0; row < 64; row++)
		{
			auto rowBase = Patterns[pat].GetRow(row);
			for(CHANNELINDEX chn = 0; chn < 4; chn++)
			{
				ModCommand &m = rowBase[chn];
				auto [command, param] = ReadMODPatternEntry(patternData[row][chn], m);

				if(command || param)
				{
					if(command == 0x0D)
					{
						if(minVersion != ST2_00)
						{
							// Dxy is volume slide in some Soundtracker versions, D00 is a pattern break in the latest versions.
							command = 0x0A;
						} else
						{
							param = 0;
						}
					} else if(command == 0x0C)
					{
						// Volume is sent as-is to the chip, which ignores the highest bit.
						param &= 0x7F;
					} else if(command == 0x0E && (param > 0x01 || minVersion < ST_IX) && useAutoSlides)
					{
						m.command = CMD_AUTO_VOLUMESLIDE;
						m.param = param;
						continue;
					} else if(command == 0x0F)
					{
						// Only the low nibble is evaluated in Soundtracker.
						param &= 0x0F;
					}

					if(minVersion <= UST1_80)
					{
						// UST effects
						m.param = param;
						switch(command)
						{
						case 0:
							// jackdance.mod by Karsten Obarski has 0xy arpeggios...
							if(param < 0x03)
							{
								m.command = CMD_NONE;
							} else
							{
								m.command = CMD_ARPEGGIO;
							}
							break;
						case 1:
							m.command = CMD_ARPEGGIO;
							break;
						case 2:
							if(m.param & 0x0F)
							{
								m.command = CMD_PORTAMENTOUP;
								m.param &= 0x0F;
							} else if(m.param >> 4)
							{
								m.command = CMD_PORTAMENTODOWN;
								m.param >>= 4;
							}
							break;
						default:
							m.command = CMD_NONE;
							break;
						}
					} else
					{
						ConvertModCommand(m, command, param);
					}
				}
			}
		}
	}

	[[maybe_unused]] /* silence clang-tidy deadcode.DeadStores */ const mpt::uchar *madeWithTracker = UL_("");
	switch(minVersion)
	{
	case UST1_00:
		madeWithTracker = UL_("Ultimate Soundtracker 1.0-1.21");
		break;
	case UST1_80:
		madeWithTracker = UL_("Ultimate Soundtracker 1.8-2.0");
		break;
	case ST2_00_Exterminator:
		madeWithTracker = UL_("SoundTracker 2.0 / D.O.C. SoundTracker II");
		break;
	case ST_III:
		madeWithTracker = UL_("Defjam Soundtracker III / Alpha Flight SoundTracker IV / D.O.C. SoundTracker IV / VI");
		break;
	case ST_IX:
		madeWithTracker = UL_("D.O.C. SoundTracker IX");
		break;
	case MST1_00:
		madeWithTracker = UL_("Master Soundtracker 1.0");
		break;
	case ST2_00:
		madeWithTracker = UL_("SoundTracker 2.0 / 2.1 / 2.2");
		break;
	}

	m_modFormat.formatName = UL_("Soundtracker");
	m_modFormat.type = UL_("stk");
	m_modFormat.madeWithTracker = madeWithTracker;
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	// Reading samples
	if(loadFlags & loadSampleData)
	{
		for(SAMPLEINDEX smp = 1; smp <= 15; smp++)
		{
			// Looped samples in (Ultimate) Soundtracker seem to ignore all sample data before the actual loop start.
			// This avoids the clicks in the first sample of pretend.mod by Karsten Obarski.
			file.Skip(Samples[smp].nLoopStart);
			Samples[smp].nLength -= Samples[smp].nLoopStart;
			Samples[smp].nLoopEnd -= Samples[smp].nLoopStart;
			Samples[smp].nLoopStart = 0;
			MODSampleHeader::GetSampleFormat().ReadSample(Samples[smp], file);
		}
	}

	return true;
}

OPENMPT_NAMESPACE_END
