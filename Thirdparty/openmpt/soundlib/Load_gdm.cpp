/*
 * Load_gdm.cpp
 * ------------
 * Purpose: GDM (BWSB Soundsystem) module loader
 * Notes  : This code is partly based on zilym's original code / specs (which are utterly wrong :P).
 *          Thanks to the MenTaLguY for gdm.txt and ajs for gdm2s3m and some hints.
 *
 *          Hint 1: Most (all?) of the unsupported features were not supported in 2GDM / BWSB either.
 *          Hint 2: Files will be played like their original formats would be played in MPT, so no
 *          BWSB quirks including crashes and freezes are supported. :-P
 * Authors: Johannes Schultz
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "mod_specifications.h"


OPENMPT_NAMESPACE_BEGIN


// GDM File Header
struct GDMFileHeader
{
	char     magic[4];          // ID: 'GDM\xFE'
	char     songTitle[32];     // Music's title
	char     songMusician[32];  // Name of music's composer
	char     dosEOF[3];         // 13, 10, 26
	char     magic2[4];         // ID: 'GMFS'
	uint8le  formatMajorVer;    // Format major version
	uint8le  formatMinorVer;    // Format minor version
	uint16le trackerID;         // Composing Tracker ID code (00 = 2GDM)
	uint8le  trackerMajorVer;   // Tracker's major version
	uint8le  trackerMinorVer;   // Tracker's minor version
	uint8le  panMap[32];        // 0-Left to 15-Right, 16=Surround, 255-N/U
	uint8le  masterVol;         // Range: 0...64
	uint8le  tempo;             // Initial music tempo (6)
	uint8le  bpm;               // Initial music BPM (125)
	uint16le originalFormat;    // Original format ID:
		// 1-MOD, 2-MTM, 3-S3M, 4-669, 5-FAR, 6-ULT, 7-STM, 8-MED, 9-PSM
		// (versions of 2GDM prior to v1.15 won't set this correctly)
		// 2GDM v1.17 will only spit out 0-byte files when trying to convert a PSM16 file,
		// and fail outright when trying to convert a new PSM file.

	uint32le orderOffset;
	uint8le  lastOrder;            // Number of orders in module - 1
	uint32le patternOffset;
	uint8le  lastPattern;          // Number of patterns in module - 1
	uint32le sampleHeaderOffset;
	uint32le sampleDataOffset;
	uint8le  lastSample;           // Number of samples in module - 1
	uint32le messageTextOffset;    // Offset of song message
	uint32le messageTextLength;
	uint32le scrollyScriptOffset;  // Offset of scrolly script (huh?)
	uint16le scrollyScriptLength;
	uint32le textGraphicOffset;    // Offset of text graphic (huh?)
	uint16le textGraphicLength;

	uint8 GetNumChannels() const
	{
		return static_cast<uint8>(std::distance(std::begin(panMap), std::find(std::begin(panMap), std::end(panMap), uint8_max)));
	}
};

MPT_BINARY_STRUCT(GDMFileHeader, 157)


// GDM Sample Header
struct GDMSampleHeader
{
	enum SampleFlags
	{
		smpLoop    = 0x01,
		smp16Bit   = 0x02,  // 16-Bit samples are not handled correctly by 2GDM (not implemented)
		smpVolume  = 0x04,  // Use default volume
		smpPanning = 0x08,
		smpLZW     = 0x10,  // LZW-compressed samples are not implemented in 2GDM
		smpStereo  = 0x20,  // Stereo samples are not handled correctly by 2GDM (not implemented)
	};

	char     name[32];      // sample's name
	char     fileName[12];  // sample's filename
	uint8le  emsHandle;     // useless
	uint32le length;        // length in bytes
	uint32le loopBegin;     // loop start in samples
	uint32le loopEnd;       // loop end in samples
	uint8le  flags;         // see SampleFlags
	uint16le c4Hertz;       // frequency
	uint8le  volume;        // default volume
	uint8le  panning;       // default pan
};

MPT_BINARY_STRUCT(GDMSampleHeader, 62)


static constexpr MODTYPE gdmFormatOrigin[] =
{
	MOD_TYPE_NONE, MOD_TYPE_MOD, MOD_TYPE_MTM, MOD_TYPE_S3M, MOD_TYPE_669, MOD_TYPE_FAR, MOD_TYPE_ULT, MOD_TYPE_STM, MOD_TYPE_MED, MOD_TYPE_PSM
};
static constexpr mpt::uchar gdmFormatOriginType[][4] =
{
	UL_(""), UL_("mod"), UL_("mtm"), UL_("s3m"), UL_("669"), UL_("far"), UL_("ult"), UL_("stm"), UL_("med"), UL_("psm")
};
static constexpr const mpt::uchar * gdmFormatOriginFormat[] =
{
	UL_(""),
	UL_("Generic MOD"),
	UL_("MultiTracker"),
	UL_("Scream Tracker 3"),
	UL_("Composer 669 / UNIS 669"),
	UL_("Farandole Composer"),
	UL_("UltraTracker"),
	UL_("Scream Tracker 2"),
	UL_("OctaMED"),
	UL_("Epic Megagames MASI")
};


static bool ValidateHeader(const GDMFileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.magic, "GDM\xFE", 4)
		|| fileHeader.dosEOF[0] != 13 || fileHeader.dosEOF[1] != 10 || fileHeader.dosEOF[2] != 26
		|| std::memcmp(fileHeader.magic2, "GMFS", 4)
		|| fileHeader.formatMajorVer != 1 || fileHeader.formatMinorVer != 0
		|| fileHeader.originalFormat >= std::size(gdmFormatOrigin)
		|| fileHeader.originalFormat == 0
		|| !fileHeader.GetNumChannels())
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderGDM(MemoryFileReader file, const uint64 *pfilesize)
{
	GDMFileHeader fileHeader;
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


bool CSoundFile::ReadGDM(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	GDMFileHeader fileHeader;
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

	InitializeGlobals(gdmFormatOrigin[fileHeader.originalFormat], fileHeader.GetNumChannels());
	m_SongFlags.set(SONG_IMPORTED);

	m_modFormat.formatName = UL_("General Digital Music");
	m_modFormat.type = UL_("gdm");
	m_modFormat.madeWithTracker = MPT_UFORMAT("BWSB 2GDM {}.{}")(fileHeader.trackerMajorVer, fileHeader.formatMinorVer);
	m_modFormat.originalType = gdmFormatOriginType[fileHeader.originalFormat];
	m_modFormat.originalFormatName = gdmFormatOriginFormat[fileHeader.originalFormat];
	m_modFormat.charset = mpt::Charset::CP437;

	// Song name
	m_songName = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.songTitle);

	// Artist name
	{
		std::string artist = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileHeader.songMusician);
		if(artist != "Unknown")
		{
			m_songArtist = mpt::ToUnicode(mpt::Charset::CP437, artist);
		}
	}

	// Read channel pan map... 0...15 = channel panning, 16 = surround channel, 255 = channel does not exist
	for(CHANNELINDEX i = 0; i < GetNumChannels(); i++)
	{
		if(fileHeader.panMap[i] < 16)
		{
			ChnSettings[i].nPan = static_cast<uint16>(std::min((fileHeader.panMap[i] * 16) + 8, 256));
		} else if(fileHeader.panMap[i] == 16)
		{
			ChnSettings[i].nPan = 128;
			ChnSettings[i].dwFlags = CHN_SURROUND;
		}
	}

	m_nDefaultGlobalVolume = std::min(fileHeader.masterVol * 4u, 256u);
	Order().SetDefaultSpeed(fileHeader.tempo);
	Order().SetDefaultTempoInt(fileHeader.bpm);

	// Read orders
	if(file.Seek(fileHeader.orderOffset))
	{
		ReadOrderFromFile<uint8>(Order(), file, fileHeader.lastOrder + 1, 0xFF, 0xFE);
	}

	// Read samples
	if(!file.Seek(fileHeader.sampleHeaderOffset))
	{
		return false;
	}

	m_nSamples = fileHeader.lastSample + 1;

	// Sample headers
	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		GDMSampleHeader gdmSample;
		if(!file.ReadStruct(gdmSample))
		{
			break;
		}

		ModSample &sample = Samples[smp];
		sample.Initialize();
		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, gdmSample.name);
		sample.filename = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, gdmSample.fileName);

		sample.nC5Speed = gdmSample.c4Hertz;
		if(UseFinetuneAndTranspose())
		{
			// Use the same inaccurate table as 2GDM for translating back to finetune, as our own routines
			// give slightly different results for the provided sample rates that may result in transpose != 0.
			static constexpr uint16 rate2finetune[] = { 8363, 8424, 8485, 8547, 8608, 8671, 8734, 8797, 7894, 7951, 8009, 8067, 8125, 8184, 8244, 8303 };
			for(uint8 i = 0; i < 16; i++)
			{
				if(sample.nC5Speed == rate2finetune[i])
				{
					sample.nFineTune = MOD2XMFineTune(i);
					break;
				}
			}
		}

		sample.nGlobalVol = 64;  // Not supported in this format
		
		sample.nLength = gdmSample.length;  // in bytes

		// Sample format
		if(gdmSample.flags & GDMSampleHeader::smp16Bit)
		{
			sample.uFlags.set(CHN_16BIT);
			sample.nLength /= 2;
		}

		sample.nLoopStart = gdmSample.loopBegin;
		sample.nLoopEnd = gdmSample.loopEnd - 1;

		if(gdmSample.flags & GDMSampleHeader::smpLoop)
			sample.uFlags.set(CHN_LOOP);

		if((gdmSample.flags & GDMSampleHeader::smpVolume) && gdmSample.volume != 0xFF)
			sample.nVolume = std::min(static_cast<uint8>(gdmSample.volume), uint8(64)) * 4;
		else
			sample.uFlags.set(SMP_NODEFAULTVOLUME);

		if(gdmSample.flags & GDMSampleHeader::smpPanning)
		{
			// Default panning is used
			sample.uFlags.set(CHN_PANNING);
			// 0...15, 16 = surround (not supported), 255 = no default panning
			sample.nPan = static_cast<uint16>((gdmSample.panning > 15) ? 128 : std::min((gdmSample.panning * 16) + 8, 256));
			sample.uFlags.set(CHN_SURROUND, gdmSample.panning == 16);
		} else
		{
			sample.nPan = 128;
		}
	}

	// Read sample data
	if((loadFlags & loadSampleData) && file.Seek(fileHeader.sampleDataOffset))
	{
		for(SAMPLEINDEX smp = 1; smp <= GetNumSamples(); smp++)
		{
			SampleIO(
				Samples[smp].uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::unsignedPCM)
				.ReadSample(Samples[smp], file);
		}
	}

	// Read patterns
	Patterns.ResizeArray(fileHeader.lastPattern + 1);

	const CModSpecifications &modSpecs = GetModSpecifications(GetBestSaveFormat());
	bool onlyAmigaNotes = true;

	// We'll start at position patternsOffset and decode all patterns
	file.Seek(fileHeader.patternOffset);
	for(PATTERNINDEX pat = 0; pat <= fileHeader.lastPattern; pat++)
	{
		// Read pattern length *including* the two "length" bytes
		uint16 patternLength = file.ReadUint16LE();

		if(patternLength <= 2)
		{
			// Huh, no pattern data present?
			continue;
		}
		FileReader chunk = file.ReadChunk(patternLength - 2);

		if(!(loadFlags & loadPatternData) || !chunk.IsValid() || !Patterns.Insert(pat, 64))
		{
			continue;
		}

		enum
		{
			rowDone     = 0x00,  // Advance to next row
			channelMask = 0x1F,  // Mask for retrieving channel information
			noteFlag    = 0x20,  // Note / instrument information present
			effectFlag  = 0x40,  // Effect information present
			effectMask  = 0x1F,  // Mask for retrieving effect command
			effectMore  = 0x20,  // Another effect follows
		};

		for(ROWINDEX row = 0; row < 64; row++)
		{
			auto rowBase = Patterns[pat].GetRow(row);

			uint8 channelByte;
			// If channel byte is zero, advance to next row.
			while((channelByte = chunk.ReadUint8()) != rowDone)
			{
				CHANNELINDEX channel = channelByte & channelMask;
				if(channel >= GetNumChannels()) break; // Better safe than sorry!

				ModCommand &m = rowBase[channel];

				if(channelByte & noteFlag)
				{
					// Note and sample follows
					auto [note, instr] = chunk.ReadArray<uint8, 2>();

					if(note)
					{
						note = (note & 0x7F) - 1;  // High bit = no-retrig flag (notes with portamento have this set)
						m.note = static_cast<ModCommand::NOTE>((note & 0x0F) + 12 * (note >> 4) + 12 + NOTE_MIN);
						if(!m.IsAmigaNote())
						{
							onlyAmigaNotes = false;
						}
					}
					m.instr = instr;
				}

				if(channelByte & effectFlag)
				{
					// Effect(s) follow(s)
					m.command = CMD_NONE;
					m.volcmd = VOLCMD_NONE;

					while(chunk.CanRead(2))
					{
						// We may want to restore the old command in some cases.
						const ModCommand oldCmd = m;

						const auto [effByte, param] = chunk.ReadArray<uint8, 2>();
						m.param = param;

						// Effect translation LUT
						static constexpr EffectCommand gdmEffTrans[] =
						{
							CMD_NONE, CMD_PORTAMENTOUP, CMD_PORTAMENTODOWN, CMD_TONEPORTAMENTO,
							CMD_VIBRATO, CMD_TONEPORTAVOL, CMD_VIBRATOVOL, CMD_TREMOLO,
							CMD_TREMOR, CMD_OFFSET, CMD_VOLUMESLIDE, CMD_POSITIONJUMP,
							CMD_VOLUME, CMD_PATTERNBREAK, CMD_MODCMDEX, CMD_SPEED,
							CMD_ARPEGGIO, CMD_NONE /* set internal flag */, CMD_RETRIG, CMD_GLOBALVOLUME,
							CMD_FINEVIBRATO, CMD_NONE, CMD_NONE, CMD_NONE,
							CMD_NONE, CMD_NONE, CMD_NONE, CMD_NONE,
							CMD_NONE, CMD_NONE, CMD_S3MCMDEX, CMD_TEMPO,
						};

						// Translate effect
						uint8 command = effByte & effectMask;
						if(command < std::size(gdmEffTrans))
							m.command = gdmEffTrans[command];
						else
							m.command = CMD_NONE;

						// Fix some effects
						switch(m.command)
						{
						case CMD_PORTAMENTOUP:
						case CMD_PORTAMENTODOWN:
							if(m.param >= 0xE0 && m_nType != MOD_TYPE_MOD)
								m.param = 0xDF;  // Don't spill into fine slide territory
							break;

						case CMD_TONEPORTAVOL:
						case CMD_VIBRATOVOL:
							if(m.param & 0xF0)
								m.param &= 0xF0;
							break;

						case CMD_VOLUME:
							m.param = std::min(m.param, uint8(64));
							if(modSpecs.HasVolCommand(VOLCMD_VOLUME))
							{
								m.volcmd = VOLCMD_VOLUME;
								m.vol = m.param;
								// Don't destroy old command, if there was one.
								m.command = oldCmd.command;
								m.param = oldCmd.param;
							}
							break;

						case CMD_MODCMDEX:
							switch(m.param >> 4)
							{
							case 0x8:
								m.command = CMD_PORTAMENTOUP;
								m.param = 0xE0 | (m.param & 0x0F);
								break;
							case 0x9:
								m.command = CMD_PORTAMENTODOWN;
								m.param = 0xE0 | (m.param & 0x0F);
								break;
							default:
								if(!modSpecs.HasCommand(CMD_MODCMDEX))
									m.ExtendedMODtoS3MEffect();
								break;
							}
							break;

						case CMD_RETRIG:
							if(!modSpecs.HasCommand(CMD_RETRIG) && modSpecs.HasCommand(CMD_MODCMDEX))
							{
								// Retrig in "MOD style"
								m.command = CMD_MODCMDEX;
								m.param = 0x90 | (m.param & 0x0F);
							}
							break;

						case CMD_S3MCMDEX:
							// Some really special commands
							if(m.param == 0x01)
							{
								// Surround (implemented in 2GDM but not in BWSB itself)
								m.param = 0x91;
							} else if((m.param & 0xF0) == 0x80)
							{
								// 4-Bit Panning
								if (!modSpecs.HasCommand(CMD_S3MCMDEX))
									m.command = CMD_MODCMDEX;
							} else
							{
								// All other effects are implemented neither in 2GDM nor in BWSB.
								m.command = CMD_NONE;
							}
							break;

						default:
							break;
						}

						// Move pannings to volume column - should never happen
						if(m.command == CMD_S3MCMDEX && ((m.param >> 4) == 0x8) && m.volcmd == VOLCMD_NONE)
						{
							m.SetVolumeCommand(VOLCMD_PANNING, static_cast<ModCommand::VOL>(((m.param & 0x0F) * 64 + 8) / 15));
							m.SetEffectCommand(oldCmd);
						}

						if(!(effByte & effectMore))
							break;
					}
				}
			}
		}
	}

	m_SongFlags.set(SONG_AMIGALIMITS | SONG_ISAMIGA, GetType() == MOD_TYPE_MOD && GetNumChannels() == 4 && onlyAmigaNotes);

	// Read song comments
	if(fileHeader.messageTextLength > 0 && file.Seek(fileHeader.messageTextOffset))
	{
		m_songMessage.Read(file, fileHeader.messageTextLength, SongMessage::leAutodetect);
	}

	return true;

}


OPENMPT_NAMESPACE_END
