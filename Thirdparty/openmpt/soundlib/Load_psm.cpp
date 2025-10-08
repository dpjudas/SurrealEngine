/*
 * Load_psm.cpp
 * ------------
 * Purpose: PSM16 and new PSM (ProTracker Studio / Epic MegaGames MASI) module loader
 * Notes  : This is partly based on https://moddingwiki.shikadi.net/wiki/ProTracker_Studio_Module
 *          and partly reverse-engineered. Also thanks to the author of foo_dumb, the source code gave me a few clues. :)
 * Authors: Johannes Schultz
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"

#include "mpt/parse/parse.hpp"

OPENMPT_NAMESPACE_BEGIN

////////////////////////////////////////////////////////////
//
//  New PSM support starts here. PSM16 structs are below.
//

// PSM File Header
struct PSMFileHeader
{
	char     formatID[4];    // "PSM " (new format)
	uint32le fileSize;       // Filesize - 12
	char     fileInfoID[4];  // "FILE"
};

MPT_BINARY_STRUCT(PSMFileHeader, 12)

// RIFF-style Chunk
struct PSMChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idTITL	= MagicLE("TITL"),
		idSDFT	= MagicLE("SDFT"),
		idPBOD	= MagicLE("PBOD"),
		idSONG	= MagicLE("SONG"),
		idDATE	= MagicLE("DATE"),
		idOPLH	= MagicLE("OPLH"),
		idPPAN	= MagicLE("PPAN"),
		idPATT	= MagicLE("PATT"),
		idDSAM	= MagicLE("DSAM"),
		idDSMP	= MagicLE("DSMP"),
	};

	uint32le id;
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

MPT_BINARY_STRUCT(PSMChunk, 8)

// Song Information
struct PSMSongHeader
{
	char  songType[9];  // Mostly "MAINSONG " (But not in Extreme Pinball!)
	uint8 compression;  // 1 - uncompressed
	uint8 numChannels;  // Number of channels

};

MPT_BINARY_STRUCT(PSMSongHeader, 11)

// Regular sample header
struct PSMSampleHeader
{
	uint8le  flags;
	char     fileName[8];    // Filename of the original module (without extension)
	char     sampleID[4];    // Identifier like "INS0" (only last digit of sample ID, i.e. sample 1 and sample 11 are equal) or "I0  "
	char     sampleName[33];
	uint8le  unknown1[6];    // 00 00 00 00 00 FF
	uint16le sampleNumber;
	uint32le sampleLength;
	uint32le loopStart;
	uint32le loopEnd;        // FF FF FF FF = end of sample
	uint8le  unknown3;
	uint8le  finetune;       // unused? always 0
	uint8le  defaultVolume;
	uint32le unknown4;
	uint32le c5Freq;         // MASI ignores the high 16 bits
	char     padding[19];

	// Convert header data to OpenMPT's internal format
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.filename = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileName);

		mptSmp.nC5Speed = c5Freq;
		mptSmp.nLength = sampleLength;
		mptSmp.nLoopStart = loopStart;
		// Note that we shouldn't add + 1 for MTM conversions here (e.g. the OMF 2097 music),
		// but I think there is no way to figure out the original format, and in the case of the OMF 2097 soundtrack
		// it doesn't make a huge audible difference anyway (no chip samples are used).
		// On the other hand, sample 8 of MUSIC_A.PSM from Extreme Pinball will sound detuned if we don't adjust the loop end here.
		if(loopEnd)
			mptSmp.nLoopEnd = loopEnd + 1;
		mptSmp.nVolume = static_cast<uint16>((defaultVolume + 1) * 2);
		mptSmp.uFlags.set(CHN_LOOP, (flags & 0x80) != 0);
		LimitMax(mptSmp.nLoopEnd, mptSmp.nLength);
		LimitMax(mptSmp.nLoopStart, mptSmp.nLoopEnd);
	}
};

MPT_BINARY_STRUCT(PSMSampleHeader, 96)

// Sinaria sample header (and possibly other games)
struct PSMSinariaSampleHeader
{
	uint8le  flags;
	char     fileName[8];  // Filename of the original module (without extension)
	char     sampleID[8];  // INS0...INS99999
	char     sampleName[33];
	uint8le  unknown1[6];  // 00 00 00 00 00 FF
	uint16le sampleNumber;
	uint32le sampleLength;
	uint32le loopStart;
	uint32le loopEnd;
	uint16le unknown3;
	uint8le  finetune;     // Appears to be unused
	uint8le  defaultVolume;
	uint32le unknown4;
	uint16le c5Freq;
	char     padding[16];

	// Convert header data to OpenMPT's internal format
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.filename = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, fileName);

		mptSmp.nC5Speed = c5Freq;
		mptSmp.nLength = sampleLength;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		mptSmp.nVolume = static_cast<uint16>((defaultVolume + 1) * 2);
		mptSmp.uFlags.set(CHN_LOOP, (flags & 0x80) != 0);
		LimitMax(mptSmp.nLoopEnd, mptSmp.nLength);
		LimitMax(mptSmp.nLoopStart, mptSmp.nLoopEnd);
	}
};

MPT_BINARY_STRUCT(PSMSinariaSampleHeader, 96)


struct PSMSubSong // For internal use (pattern conversion)
{
	std::vector<uint8> channelPanning, channelVolume;
	std::vector<bool> channelSurround;
	char songName[10] = {};

	PSMSubSong(CHANNELINDEX numChannels)
	    : channelPanning(numChannels, 128)
	    , channelVolume(numChannels, 64)
	    , channelSurround(numChannels, false)
	{ }

	void SetPanning(CHANNELINDEX chn, uint8 type, int16 pan, bool &subsongPanningDiffers, std::vector<PSMSubSong> &subsongs)
	{
		if(chn >= channelPanning.size())
			return;
		switch(type)
		{
			case 0:  // use panning
				if(pan >= 0)
					channelPanning[chn] = static_cast<uint8>(pan ^ 128);
				channelSurround[chn] = false;
				break;
			case 2:  // surround
				channelPanning[chn] = 128;
				channelSurround[chn] = true;
				break;
			case 4:  // center
				channelPanning[chn] = 128;
				channelSurround[chn] = false;
				break;
		}
		if(!subsongPanningDiffers && !subsongs.empty())
		{
			if(subsongs.back().channelPanning[chn] != channelPanning[chn]
				|| subsongs.back().channelSurround[chn] != channelSurround[chn])
				subsongPanningDiffers = true;
		}
	}
};


// Portamento effect conversion (depending on format version)
static uint8 ConvertPSMPorta(uint8 param, bool sinariaFormat)
{
	if(sinariaFormat)
		return param;
	if(param < 4)
		return (param | 0xF0);
	else
		return (param >> 2);
}


// Read a Pattern ID (something like "P0  " or "P13 ", or "PATT0   " in Sinaria)
static PATTERNINDEX ReadPSMPatternIndex(FileReader &file, bool &sinariaFormat)
{
	char patternID[5];
	uint8 offset = 1;
	if(!file.ReadString<mpt::String::spacePadded>(patternID, 4))
		return 0;
	if(!memcmp(patternID, "PATT", 4))
	{
		file.ReadString<mpt::String::spacePadded>(patternID, 4);
		sinariaFormat = true;
		offset = 0;
	}
	return mpt::parse<uint16>(&patternID[offset]);
}


static bool ValidateHeader(const PSMFileHeader &fileHeader)
{
	if(!std::memcmp(fileHeader.formatID, "PSM ", 4)
		&& !std::memcmp(fileHeader.fileInfoID, "FILE", 4))
	{
		return true;
	}
#ifdef MPT_PSM_DECRYPT
	if(!std::memcmp(fileHeader.formatID, "QUP$", 4)
		&& !std::memcmp(fileHeader.fileInfoID, "OSWQ", 4))
	{
		return true;
	}
#endif
	return false;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderPSM(MemoryFileReader file, const uint64 *pfilesize)
{
	PSMFileHeader fileHeader;
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


bool CSoundFile::ReadPSM(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	PSMFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}

#ifdef MPT_PSM_DECRYPT
	// CONVERT.EXE /K - I don't think any game ever used this.
	std::vector<std::byte> decrypted;
	if(!memcmp(fileHeader.formatID, "QUP$", 4)
		&& !memcmp(fileHeader.fileInfoID, "OSWQ", 4))
	{
		if(loadFlags == onlyVerifyHeader)
			return true;
		file.Rewind();
		decrypted.resize(file.GetLength());
		file.ReadRaw(decrypted.data(), decrypted.size());
		uint8 i = 0;
		for(auto &c : decrypted)
		{
			c -= ++i;
		}
		file = FileReader(mpt::as_span(decrypted));
		file.ReadStruct(fileHeader);
	}
#endif // MPT_PSM_DECRYPT

	// Check header
	if(!ValidateHeader(fileHeader))
	{
		return false;
	}

	ChunkReader chunkFile(file);
	ChunkReader::ChunkList<PSMChunk> chunks;
	if(loadFlags == onlyVerifyHeader)
		chunks = chunkFile.ReadChunksUntil<PSMChunk>(1, PSMChunk::idSDFT);
	else
		chunks = chunkFile.ReadChunks<PSMChunk>(1);

	// "SDFT" - Format info (song data starts here)
	if(!chunks.GetChunk(PSMChunk::idSDFT).ReadMagic("MAINSONG"))
		return false;
	else if(loadFlags == onlyVerifyHeader)
		return true;

	auto songChunks = chunks.GetAllChunks(PSMChunk::idSONG);
	CHANNELINDEX numChannels = 0;
	for(FileReader chunk : songChunks)
	{
		PSMSongHeader songHeader;
		if(!chunk.ReadStruct(songHeader) || songHeader.compression != 0x01)  // No compression for PSM files
			return false;
		// Subsongs *might* have different channel count
		numChannels = Clamp(static_cast<CHANNELINDEX>(songHeader.numChannels), numChannels, MAX_BASECHANNELS);
	}
	if(!numChannels)
		return false;

	// Yep, this seems to be a valid file.
	InitializeGlobals(MOD_TYPE_PSM, numChannels);
	m_SongFlags = SONG_ITOLDEFFECTS | SONG_ITCOMPATGXX;

	// "TITL" - Song Title
	FileReader titleChunk = chunks.GetChunk(PSMChunk::idTITL);
	titleChunk.ReadString<mpt::String::spacePadded>(m_songName, titleChunk.GetLength());

	Order().clear();
	// Subsong setup
	std::vector<PSMSubSong> subsongs;
	bool subsongPanningDiffers = false; // Do we have subsongs with different panning positions?
	bool sinariaFormat = false; // The game "Sinaria" uses a slightly modified PSM structure - in some ways it's more like PSM16 (e.g. effects).

	// "SONG" - Subsong information (channel count etc)
	for(ChunkReader chunk : songChunks)
	{
		PSMSongHeader songHeader;
		chunk.ReadStruct(songHeader);

		PSMSubSong subsong{GetNumChannels()};
		mpt::String::WriteAutoBuf(subsong.songName) = mpt::String::ReadBuf(mpt::String::nullTerminated, songHeader.songType);

		if(!Order().empty())
		{
			// Add a new sequence for this subsong
			if(Order.AddSequence() == SEQUENCEINDEX_INVALID)
				break;
		}
		Order().SetName(mpt::ToUnicode(mpt::Charset::CP437, subsong.songName));

		// Read "Sub chunks"
		auto subChunks = chunk.ReadChunks<PSMChunk>(1);
		for(const auto &subChunkIter : subChunks.chunks)
		{
			FileReader subChunk(subChunkIter.GetData());
			PSMChunk subChunkHead = subChunkIter.GetHeader();
			
			switch(subChunkHead.GetID())
			{
#if 0
			case PSMChunk::idDATE: // "DATE" - Conversion date (YYMMDD)
				if(subChunkHead.GetLength() == 6)
				{
					char cversion[7];
					subChunk.ReadString<mpt::String::maybeNullTerminated>(cversion, 6);
					uint32 version = mpt::parse<uint32>(cversion);
					// Sinaria song dates (just to go sure...)
					if(version == 800211 || version == 940902 || version == 940903 ||
						version == 940906 || version == 940914 || version == 941213)
						sinariaFormat = true;
				}
				break;
#endif

			case PSMChunk::idOPLH: // "OPLH" - Order list, channel + module settings
				if(subChunkHead.GetLength() >= 9)
				{
					// First two bytes = Number of chunks that follow
					//uint16 totalChunks = subChunk.ReadUint16LE();
					subChunk.Skip(2);

					// Now, the interesting part begins!
					uint16 chunkCount = 0, firstOrderChunk = uint16_max;

					// "Sub sub chunks" (grrrr, silly format)
					while(subChunk.CanRead(1))
					{
						uint8 opcode = subChunk.ReadUint8();
						if(!opcode)
						{
							// Last chunk was reached.
							break;
						}

						// Note: This is more like a playlist than a collection of global values.
						// In theory, a tempo item inbetween two order items should modify the
						// tempo when switching patterns. No module uses this feature in practice
						// though, so we can keep our loader simple.
						switch(opcode)
						{
						case 0x01: // Play order list item
							{
								PATTERNINDEX pat = ReadPSMPatternIndex(subChunk, sinariaFormat);
								if(pat == 0xFF)
									pat = PATTERNINDEX_INVALID;
								else if(pat == 0xFE)
									pat = PATTERNINDEX_SKIP;
								Order().push_back(pat);
								// Decide whether this is the first order chunk or not (for finding out the correct restart position)
								if(firstOrderChunk == uint16_max)
									firstOrderChunk = chunkCount;
							}
							break;

						case 0x02: // Play Range (xx from line yy to line zz). Three 16-bit parameters but it seems like the next opcode is parsed at the same position as the third parameter.
							subChunk.Skip(4);
							break;

						case 0x03:  // Jump Loop (like Jump Line, but with a third, unknown byte following - nope, it does not appear to be a loop count)
						case 0x04:  // Jump Line (Restart position)
							{
								uint16 restartChunk = subChunk.ReadUint16LE();
								if(restartChunk >= firstOrderChunk)
									Order().SetRestartPos(static_cast<ORDERINDEX>(restartChunk - firstOrderChunk));  // Close enough - we assume that order list is continuous (like in any real-world PSM)
								if(opcode == 0x03)
									subChunk.Skip(1);
							}
							break;

						case 0x05: // Channel Flip (changes channel type without changing pan position)
							{
								const auto [chn, type] = subChunk.ReadArray<uint8, 2>();
								subsong.SetPanning(chn, type, -1, subsongPanningDiffers, subsongs);
							}
							break;

						case 0x06: // Transpose (appears to be a no-op in MASI)
							subChunk.Skip(1);
							break;

						case 0x07: // Default Speed
							Order().SetDefaultSpeed(subChunk.ReadUint8());
							break;

						case 0x08: // Default Tempo
							Order().SetDefaultTempoInt(subChunk.ReadUint8());
							break;

						case 0x0C: // Sample map table
							// Never seems to be different, so...
							// This is probably a part of the never-implemented "mini programming language" mentioned in the PSM docs.
							// Output of PLAY.EXE: "SMapTabl from pos 0 to pos -1 starting at 0 and adding 1 to it each time"
							// It appears that this maps e.g. what is "I0" in the file to sample 1.
							// If we were being fancy, we could implement this, but in practice it won't matter.
							{
								uint8 mapTable[6];
								if(!subChunk.ReadArray(mapTable)
									|| mapTable[0] != 0x00 || mapTable[1] != 0xFF  // "0 to -1" (does not seem to do anything)
									|| mapTable[2] != 0x00 || mapTable[3] != 0x00  // "at 0" (actually this appears to be the adding part - changing this to 0x01 0x00 offsets all samples by 1)
									|| mapTable[4] != 0x01 || mapTable[5] != 0x00) // "adding 1" (does not seem to do anything)
								{
									return false;
								}
							}
							break;

						case 0x0D: // Channel panning table - can be set using CONVERT.EXE /E
							{
								const auto [chn, pan, type] = subChunk.ReadArray<uint8, 3>();
								subsong.SetPanning(chn, type, pan, subsongPanningDiffers, subsongs);
							}
							break;

						case 0x0E: // Channel volume table (0...255) - can be set using CONVERT.EXE /E, is 255 in all "official" PSMs except for some OMF 2097 tracks
							{
								const auto [chn, vol] = subChunk.ReadArray<uint8, 2>();
								if(chn < subsong.channelVolume.size())
								{
									subsong.channelVolume[chn] = (vol / 4u) + 1;
								}
							}
							break;

						default:
							// Non-existent opcode, MASI would just freeze in this case.
							return false;

						}
						chunkCount++;
					}
				}
				break;

			case PSMChunk::idPPAN: // PPAN - Channel panning table (used in Sinaria)
				// In some Sinaria tunes, this is actually longer than 2 * channels...
				MPT_ASSERT(subChunkHead.GetLength() >= GetNumChannels() * 2u);
				for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
				{
					if(!subChunk.CanRead(2))
						break;

					const auto [type, pan] = subChunk.ReadArray<uint8, 2>();
					subsong.SetPanning(chn, type, pan, subsongPanningDiffers, subsongs);
				}
				break;

			case PSMChunk::idPATT: // PATT - Pattern list
				// We don't really need this.
				break;

			case PSMChunk::idDSAM: // DSAM - Sample list
				// We don't need this either.
				break;

			default:
				break;

			}
		}

		// Attach this subsong to the subsong list - finally, all "sub sub sub ..." chunks are parsed.
		if(!Order().empty())
			subsongs.push_back(subsong);
	}

	Order.SetSequence(0);

	if(subsongs.empty())
		return false;

	// DSMP - Samples
	if(loadFlags & loadSampleData)
	{
		auto sampleChunks = chunks.GetAllChunks(PSMChunk::idDSMP);
		for(auto &chunk : sampleChunks)
		{
			SAMPLEINDEX smp;
			if(!sinariaFormat)
			{
				// Original header
				PSMSampleHeader sampleHeader;
				if(!chunk.ReadStruct(sampleHeader))
					continue;

				smp = static_cast<SAMPLEINDEX>(sampleHeader.sampleNumber + 1);
				if(smp > 0 && smp < MAX_SAMPLES)
				{
					m_nSamples = std::max(m_nSamples, smp);
					sampleHeader.ConvertToMPT(Samples[smp]);
					m_szNames[smp] = mpt::String::ReadBuf(mpt::String::nullTerminated, sampleHeader.sampleName);
				}
			} else
			{
				// Sinaria uses a slightly different sample header
				PSMSinariaSampleHeader sampleHeader;
				if(!chunk.ReadStruct(sampleHeader))
					continue;

				smp = static_cast<SAMPLEINDEX>(sampleHeader.sampleNumber + 1);
				if(smp > 0 && smp < MAX_SAMPLES)
				{
					m_nSamples = std::max(m_nSamples, smp);
					sampleHeader.ConvertToMPT(Samples[smp]);
					m_szNames[smp] = mpt::String::ReadBuf(mpt::String::nullTerminated, sampleHeader.sampleName);
				}
			}
			if(smp > 0 && smp < MAX_SAMPLES)
			{
				SampleIO(
					SampleIO::_8bit,
					SampleIO::mono,
					SampleIO::littleEndian,
					SampleIO::deltaPCM).ReadSample(Samples[smp], chunk);
			}
		}
	}

	// Make the default variables of the first subsong global
	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].nVolume = subsongs[0].channelVolume[chn];
		ChnSettings[chn].nPan = subsongs[0].channelPanning[chn];
		ChnSettings[chn].dwFlags.set(CHN_SURROUND, subsongs[0].channelSurround[chn]);
	}

	m_modFormat.formatName = sinariaFormat ? UL_("Epic MegaGames MASI (New Version / Sinaria)") : UL_("Epic MegaGames MASI (New Version)");
	m_modFormat.type = UL_("psm");
	m_modFormat.charset = mpt::Charset::CP437;

	if(!(loadFlags & loadPatternData))
	{
		return true;
	}

	// "PBOD" - Pattern data of a single pattern
	// Now that we know the number of channels, we can go through all the patterns.
	auto pattChunks = chunks.GetAllChunks(PSMChunk::idPBOD);
	Patterns.ResizeArray(static_cast<PATTERNINDEX>(pattChunks.size()));
	for(auto &chunk : pattChunks)
	{
		if(chunk.GetLength() != chunk.ReadUint32LE()	// Same value twice
			|| !chunk.LengthIsAtLeast(8))
		{
			continue;
		}

		PATTERNINDEX pat = ReadPSMPatternIndex(chunk, sinariaFormat);
		uint16 numRows = chunk.ReadUint16LE();

		if(!Patterns.Insert(pat, numRows))
		{
			continue;
		}

		enum
		{
			noteFlag	= 0x80,
			instrFlag	= 0x40,
			volFlag		= 0x20,
			effectFlag	= 0x10,
		};

		// Read pattern.
		for(ROWINDEX row = 0; row < numRows; row++)
		{
			auto rowBase = Patterns[pat].GetRow(row);
			uint16 rowSize = chunk.ReadUint16LE();
			if(rowSize <= 2)
			{
				continue;
			}

			FileReader rowChunk = chunk.ReadChunk(rowSize - 2);

			while(rowChunk.CanRead(3))
			{
				const auto [flags, channel] = rowChunk.ReadArray<uint8, 2>();
				// Point to the correct channel
				ModCommand &m = rowBase[std::min(static_cast<CHANNELINDEX>(GetNumChannels() - 1), static_cast<CHANNELINDEX>(channel))];

				if(flags & noteFlag)
				{
					// Note present
					uint8 note = rowChunk.ReadUint8();
					if(!sinariaFormat)
					{
						if(note == 0xFF)	// Can be found in a few files but is apparently not supported by MASI
							note = NOTE_NOTECUT;
						else
							if(note < 129) note = static_cast<uint8>((note & 0x0F) + 12 * (note >> 4) + 13);
					} else
					{
						if(note < 85) note += 36;
					}
					m.note = note;
				}

				if(flags & instrFlag)
				{
					// Instrument present
					m.instr = rowChunk.ReadUint8() + 1;
				}

				if(flags & volFlag)
				{
					// Volume present
					uint8 vol = rowChunk.ReadUint8();
					m.SetVolumeCommand(VOLCMD_VOLUME, static_cast<ModCommand::VOL>((std::min(vol, uint8(127)) + 1) / 2));
				}

				if(flags & effectFlag)
				{
					// Effect present - convert
					const auto [command, param] = rowChunk.ReadArray<uint8, 2>();
					m.param = param;

					// This list is annoyingly similar to PSM16, but not quite identical.
					switch(command)
					{
					// Volslides
					case 0x01: // fine volslide up
						m.command = CMD_VOLUMESLIDE;
						if (sinariaFormat) m.param = (m.param << 4) | 0x0F;
						else m.param = ((m.param & 0x1E) << 3) | 0x0F;
						break;
					case 0x02: // volslide up
						m.command = CMD_VOLUMESLIDE;
						if (sinariaFormat) m.param = 0xF0 & (m.param << 4);
						else m.param = 0xF0 & (m.param << 3);
						break;
					case 0x03: // fine volslide down
						m.command = CMD_VOLUMESLIDE;
						if (sinariaFormat) m.param |= 0xF0;
						else m.param = 0xF0 | (m.param >> 1);
						break;
					case 0x04: // volslide down
						m.command = CMD_VOLUMESLIDE;
						if (sinariaFormat) m.param &= 0x0F;
						else if(m.param < 2) m.param |= 0xF0; else m.param = (m.param >> 1) & 0x0F;
						break;

					// Portamento
					case 0x0B: // fine portamento up
						m.SetEffectCommand(CMD_PORTAMENTOUP, 0xF0 | ConvertPSMPorta(m.param, sinariaFormat));
						break;
					case 0x0C: // portamento up
						m.SetEffectCommand(CMD_PORTAMENTOUP, ConvertPSMPorta(m.param, sinariaFormat));
						break;
					case 0x0D: // fine portamento down
						m.SetEffectCommand(CMD_PORTAMENTODOWN, 0xF0 | ConvertPSMPorta(m.param, sinariaFormat));
						break;
					case 0x0E: // portamento down
						m.SetEffectCommand(CMD_PORTAMENTODOWN, ConvertPSMPorta(m.param, sinariaFormat));
						break;
					case 0x0F: // tone portamento
						m.command = CMD_TONEPORTAMENTO;
						if(!sinariaFormat) m.param >>= 2;
						break;
					case 0x11: // glissando control
						m.SetEffectCommand(CMD_S3MCMDEX, 0x10 | (m.param & 0x01));
						break;
					case 0x10: // tone portamento + volslide up
						m.SetEffectCommand(CMD_TONEPORTAVOL, m.param & 0xF0);
						break;
					case 0x12: // tone portamento + volslide down
						m.SetEffectCommand(CMD_TONEPORTAVOL, (m.param >> 4) & 0x0F);
						break;

					case 0x13: // ScreamTracker command S - actually hangs / crashes MASI
						m.command = CMD_S3MCMDEX;
						break;

					// Vibrato
					case 0x15: // vibrato
						m.command = CMD_VIBRATO;
						break;
					case 0x16: // vibrato waveform
						m.SetEffectCommand(CMD_S3MCMDEX, 0x30 | (m.param & 0x0F));
						break;
					case 0x17: // vibrato + volslide up
						m.SetEffectCommand(CMD_VIBRATOVOL, 0xF0 | m.param);
						break;
					case 0x18: // vibrato + volslide down
						m.command = CMD_VIBRATOVOL;
						break;

					// Tremolo
					case 0x1F: // tremolo
						m.command = CMD_TREMOLO;
						break;
					case 0x20: // tremolo waveform
						m.SetEffectCommand(CMD_S3MCMDEX, 0x40 | (m.param & 0x0F));
						break;

					// Sample commands
					case 0x29: // 3-byte offset - we only support the middle byte.
						m.SetEffectCommand(CMD_OFFSET, rowChunk.ReadArray<uint8, 2>()[0]);
						break;
					case 0x2A: // retrigger
						m.command = CMD_RETRIG;
						break;
					case 0x2B: // note cut
						m.SetEffectCommand(CMD_S3MCMDEX, 0xC0 | (m.param & 0x0F));
						break;
					case 0x2C: // note delay
						m.SetEffectCommand(CMD_S3MCMDEX, 0xD0 | (m.param & 0x0F));
						break;

					// Position change
					case 0x33: // position jump - MASI seems to ignore this command, and CONVERT.EXE never writes it
						// Parameter actually is probably just an index into the order table
						m.SetEffectCommand(CMD_POSITIONJUMP, m.param / 2u);
						rowChunk.Skip(1);
						break;
					case 0x34: // pattern break
						// When converting from S3M, the parameter is double-BDC-encoded (wtf!)
						// When converting from MOD, it's in binary.
						// MASI ignores the parameter entirely, and so do we.
						m.SetEffectCommand(CMD_PATTERNBREAK, 0);
						break;
					case 0x35: // loop pattern
						m.SetEffectCommand(CMD_S3MCMDEX, 0xB0 | (m.param & 0x0F));
						break;
					case 0x36: // pattern delay
						m.SetEffectCommand(CMD_S3MCMDEX, 0xE0 | (m.param & 0x0F));
						break;

					// speed change
					case 0x3D: // set speed
						m.command = CMD_SPEED;
						break;
					case 0x3E: // set tempo
						m.command = CMD_TEMPO;
						break;

					// misc commands
					case 0x47: // arpeggio
						m.command = CMD_ARPEGGIO;
						break;
					case 0x48: // set finetune
						m.SetEffectCommand(CMD_S3MCMDEX, 0x20 | (m.param & 0x0F));
						break;
					case 0x49: // set balance
						m.SetEffectCommand(CMD_S3MCMDEX, 0x80 | (m.param & 0x0F));
						break;

					default:
						m.command = CMD_NONE;
						break;
					}
				}
			}
		}
	}

	if(subsongs.size() > 1)
	{
		// Write subsong "configuration" to patterns (only if there are multiple subsongs)
		for(size_t i = 0; i < subsongs.size(); i++)
		{
			const PSMSubSong &subsong = subsongs[i];
			ModSequence &order = Order(static_cast<SEQUENCEINDEX>(i));
			const PATTERNINDEX startPattern = order.EnsureUnique(order.GetFirstValidIndex());
			if(startPattern == PATTERNINDEX_INVALID)
				continue;

			// Subsongs with different panning setup -> write to pattern (MUSIC_C.PSM)
			// Don't write channel volume for now, as there is no real-world module which needs it.
			if(subsongPanningDiffers)
			{
				for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
				{
					if(subsong.channelSurround[chn])
						Patterns[startPattern].WriteEffect(EffectWriter(CMD_S3MCMDEX, 0x91).Row(0).Channel(chn).RetryNextRow());
					else
						Patterns[startPattern].WriteEffect(EffectWriter(CMD_PANNING8, subsong.channelPanning[chn]).Row(0).Channel(chn).RetryNextRow());
				}
			}
		}
	}

	return true;
}

////////////////////////////////
//
//  PSM16 support starts here.
//

struct PSM16FileHeader
{
	char     formatID[4];		// "PSM\xFE" (PSM16)
	char     songName[59];		// Song title, padded with nulls
	uint8le  lineEnd;			// $1A
	uint8le  songType;			// Song Type bitfield
	uint8le  formatVersion;		// $10
	uint8le  patternVersion;	// 0 or 1
	uint8le  songSpeed;			// 1 ... 255
	uint8le  songTempo;			// 32 ... 255
	uint8le  masterVolume;		// 0 ... 255
	uint16le songLength;		// 0 ... 255 (number of patterns to play in the song)
	uint16le songOrders;		// 0 ... 255 (same as previous value as no subsongs are present)
	uint16le numPatterns;		// 1 ... 255
	uint16le numSamples;		// 1 ... 255
	uint16le numChannelsPlay;	// 0 ... 32 (max. number of channels to play)
	uint16le numChannelsReal;	// 0 ... 32 (max. number of channels to process)
	uint32le orderOffset;		// Pointer to order list
	uint32le panOffset;			// Pointer to pan table
	uint32le patOffset;			// Pointer to pattern data
	uint32le smpOffset;			// Pointer to sample headers
	uint32le commentsOffset;	// Pointer to song comment
	uint32le patSize;			// Size of all patterns
	char     filler[40];
};

MPT_BINARY_STRUCT(PSM16FileHeader, 146)

struct PSM16SampleHeader
{
	enum SampleFlags
	{
		smpMask		= 0x7F,
		smp16Bit	= 0x04,
		smpUnsigned	= 0x08,
		smpDelta	= 0x10,
		smpPingPong	= 0x20,
		smpLoop		= 0x80,
	};

	char     filename[13];	// null-terminated
	char     name[24];		// ditto
	uint32le offset;		// in file
	uint32le memoffset;		// not used
	uint16le sampleNumber;	// 1 ... 255
	uint8le  flags;			// sample flag bitfield
	uint32le length;		// in bytes
	uint32le loopStart;		// in samples?
	uint32le loopEnd;		// in samples?
	uint8le  finetune;		// Low nibble = MOD finetune, high nibble = transpose (7 = center)
	uint8le  volume;		// default volume
	uint16le c2freq;		// Middle-C frequency, which has to be combined with the finetune and transpose.

	// Convert sample header to OpenMPT's internal format
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize();
		mptSmp.filename = mpt::String::ReadBuf(mpt::String::nullTerminated, filename);

		mptSmp.nLength = length;
		mptSmp.nLoopStart = loopStart;
		mptSmp.nLoopEnd = loopEnd;
		// It seems like that finetune and transpose are added to the already given c2freq... That's a double WTF!
		// Why on earth would you want to use both systems at the same time?
		mptSmp.nC5Speed = c2freq;
		mptSmp.Transpose(((finetune ^ 0x08) - 0x78) / (12.0 * 16.0));

		mptSmp.nVolume = std::min(volume.get(), uint8(64)) * 4u;

		mptSmp.uFlags.reset();
		if(flags & PSM16SampleHeader::smp16Bit)
		{
			mptSmp.uFlags.set(CHN_16BIT);
			mptSmp.nLength /= 2u;
		}
		if(flags & PSM16SampleHeader::smpPingPong)
		{
			mptSmp.uFlags.set(CHN_PINGPONGLOOP);
		}
		if(flags & PSM16SampleHeader::smpLoop)
		{
			mptSmp.uFlags.set(CHN_LOOP);
		}
	}

	// Retrieve the internal sample format flags for this sample.
	SampleIO GetSampleFormat() const
	{
		SampleIO sampleIO(
			(flags & PSM16SampleHeader::smp16Bit) ? SampleIO::_16bit : SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::signedPCM);

		if(flags & PSM16SampleHeader::smpUnsigned)
		{
			sampleIO |= SampleIO::unsignedPCM;
		} else if((flags & PSM16SampleHeader::smpDelta) || (flags & PSM16SampleHeader::smpMask) == 0)
		{
			sampleIO |= SampleIO::deltaPCM;
		}

		return sampleIO;
	}
};

MPT_BINARY_STRUCT(PSM16SampleHeader, 64)

struct PSM16PatternHeader
{
	uint16le size;		// includes header bytes
	uint8le  numRows;	// 1 ... 64
	uint8le  numChans;	// 1 ... 32
};

MPT_BINARY_STRUCT(PSM16PatternHeader, 4)


static bool ValidateHeader(const PSM16FileHeader &fileHeader)
{
	if(std::memcmp(fileHeader.formatID, "PSM\xFE", 4)
		|| fileHeader.lineEnd != 0x1A
		|| (fileHeader.formatVersion != 0x10 && fileHeader.formatVersion != 0x01) // why is this sometimes 0x01?
		|| fileHeader.patternVersion != 0 // 255ch pattern version not supported (does it actually exist? The pattern format is not documented)
		|| (fileHeader.songType & 3) != 0
		|| fileHeader.numChannelsPlay > 32
		|| fileHeader.numChannelsReal > 32
		|| std::max(fileHeader.numChannelsPlay, fileHeader.numChannelsReal) == 0)
	{
		return false;
	}
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderPSM16(MemoryFileReader file, const uint64 *pfilesize)
{
	PSM16FileHeader fileHeader;
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


bool CSoundFile::ReadPSM16(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();

	// Is it a valid PSM16 file?
	PSM16FileHeader fileHeader;
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

	// Seems to be valid!
	InitializeGlobals(MOD_TYPE_PSM, Clamp(CHANNELINDEX(fileHeader.numChannelsPlay), CHANNELINDEX(fileHeader.numChannelsReal), MAX_BASECHANNELS));
	
	m_modFormat.formatName = UL_("Epic MegaGames MASI (Old Version)");
	m_modFormat.type = UL_("psm");
	m_modFormat.charset = mpt::Charset::CP437;

	m_nSamplePreAmp = fileHeader.masterVolume;
	if(m_nSamplePreAmp == 255)
	{
		// Most of the time, the master volume value makes sense... Just not when it's 255.
		m_nSamplePreAmp = 48;
	}
	Order().SetDefaultSpeed(fileHeader.songSpeed);
	Order().SetDefaultTempoInt(fileHeader.songTempo);

	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.songName);

	// Read orders
	if(fileHeader.orderOffset > 4 && file.Seek(fileHeader.orderOffset - 4) && file.ReadMagic("PORD"))
	{
		ReadOrderFromFile<uint8>(Order(), file, fileHeader.songOrders);
	}

	// Read pan positions
	if(fileHeader.panOffset > 4 && file.Seek(fileHeader.panOffset - 4) && file.ReadMagic("PPAN"))
	{
		for(CHANNELINDEX i = 0; i < GetNumChannels(); i++)
		{
			ChnSettings[i].nPan = static_cast<uint16>(((15 - (file.ReadUint8() & 0x0F)) * 256 + 8) / 15);  // 15 seems to be left and 0 seems to be right...
			// ChnSettings[i].dwFlags = (i >= fileHeader.numChannelsPlay) ? CHN_MUTE : 0; // don't mute channels, as muted channels are completely ignored in S3M
		}
	}

	// Read samples
	if(fileHeader.smpOffset > 4 && file.Seek(fileHeader.smpOffset - 4) && file.ReadMagic("PSAH"))
	{
		FileReader sampleChunk = file.ReadChunk(fileHeader.numSamples * sizeof(PSM16SampleHeader));

		for(SAMPLEINDEX fileSample = 0; fileSample < fileHeader.numSamples; fileSample++)
		{
			PSM16SampleHeader sampleHeader;
			if(!sampleChunk.ReadStruct(sampleHeader))
			{
				break;
			}

			const SAMPLEINDEX smp = sampleHeader.sampleNumber;
			if(smp > 0 && smp < MAX_SAMPLES && !Samples[smp].HasSampleData())
			{
				m_nSamples = std::max(m_nSamples, smp);

				sampleHeader.ConvertToMPT(Samples[smp]);
				m_szNames[smp] = mpt::String::ReadBuf(mpt::String::nullTerminated, sampleHeader.name);

				if(loadFlags & loadSampleData)
				{
					file.Seek(sampleHeader.offset);
					sampleHeader.GetSampleFormat().ReadSample(Samples[smp], file);
				}
			}
		}
	}

	// Read patterns
	if(!(loadFlags & loadPatternData))
	{
		return true;
	}
	if(fileHeader.patOffset > 4 && file.Seek(fileHeader.patOffset - 4) && file.ReadMagic("PPAT"))
	{
		Patterns.ResizeArray(fileHeader.numPatterns);
		for(PATTERNINDEX pat = 0; pat < fileHeader.numPatterns; pat++)
		{
			PSM16PatternHeader patternHeader;
			if(!file.ReadStruct(patternHeader))
			{
				break;
			}

			if(patternHeader.size < sizeof(PSM16PatternHeader))
			{
				continue;
			}

			// Patterns are padded to 16 Bytes
			FileReader patternChunk = file.ReadChunk(((patternHeader.size + 15) & ~15) - sizeof(PSM16PatternHeader));

			if(!Patterns.Insert(pat, patternHeader.numRows))
			{
				continue;
			}

			enum
			{
				channelMask	= 0x1F,
				noteFlag	= 0x80,
				volFlag		= 0x40,
				effectFlag	= 0x20,
			};

			ROWINDEX curRow = 0;

			while(patternChunk.CanRead(1) && curRow < patternHeader.numRows)
			{
				uint8 chnFlag = patternChunk.ReadUint8();
				if(chnFlag == 0)
				{
					curRow++;
					continue;
				}

				ModCommand &m = *Patterns[pat].GetpModCommand(curRow, std::min(static_cast<CHANNELINDEX>(chnFlag & channelMask), static_cast<CHANNELINDEX>(GetNumChannels() - 1)));

				if(chnFlag & noteFlag)
				{
					// note + instr present
					const auto [note, instr] = patternChunk.ReadArray<uint8, 2>();
					m.note = note + 36;
					m.instr = instr;
				}
				if(chnFlag & volFlag)
				{
					// volume present
					m.volcmd = VOLCMD_VOLUME;
					m.vol = std::min(patternChunk.ReadUint8(), uint8(64));
				}
				if(chnFlag & effectFlag)
				{
					// effect present - convert
					const auto [command, param] = patternChunk.ReadArray<uint8, 2>();
					m.param = param;

					switch(command)
					{
					// Volslides
					case 0x01: // fine volslide up
						m.SetEffectCommand(CMD_VOLUMESLIDE, (m.param << 4) | 0x0F);
						break;
					case 0x02: // volslide up
						m.SetEffectCommand(CMD_VOLUMESLIDE, (m.param << 4) & 0xF0);
						break;
					case 0x03: // fine voslide down
						m.SetEffectCommand(CMD_VOLUMESLIDE, 0xF0 | m.param);
						break;
					case 0x04: // volslide down
						m.SetEffectCommand(CMD_VOLUMESLIDE, m.param & 0x0F);
						break;

					// Portamento
					case 0x0A: // fine portamento up
						m.SetEffectCommand(CMD_PORTAMENTOUP, 0xF0 | m.param);
						break;
					case 0x0B: // portamento down
						m.command = CMD_PORTAMENTOUP;
						break;
					case 0x0C: // fine portamento down
						m.SetEffectCommand(CMD_PORTAMENTODOWN, m.param | 0xF0);
						break;
					case 0x0D: // portamento down
						m.command = CMD_PORTAMENTODOWN;
						break;
					case 0x0E: // tone portamento
						m.command = CMD_TONEPORTAMENTO;
						break;
					case 0x0F: // glissando control (note: this can be found in the Odyssey music from Silverball but it seems like it was actually a literal translation from MOD effect F)
						m.SetEffectCommand(CMD_S3MCMDEX, 0x10 | (m.param & 0x0F));
						break;
					case 0x10: // tone portamento + volslide up
						m.SetEffectCommand(CMD_TONEPORTAVOL, m.param << 4);
						break;
					case 0x11: // tone portamento + volslide down
						m.SetEffectCommand(CMD_TONEPORTAVOL, m.param & 0x0F);
						break;

					// Vibrato
					case 0x14: // vibrato
						m.command = CMD_VIBRATO;
						break;
					case 0x15: // vibrato waveform
						m.SetEffectCommand(CMD_S3MCMDEX, 0x30 | (m.param & 0x0F));
						break;
					case 0x16: // vibrato + volslide up
						m.SetEffectCommand(CMD_VIBRATOVOL, m.param << 4);
						break;
					case 0x17: // vibrato + volslide down
						m.SetEffectCommand(CMD_VIBRATOVOL, m.param & 0x0F);
						break;

					// Tremolo
					case 0x1E: // tremolo
						m.command = CMD_TREMOLO;
						break;
					case 0x1F: // tremolo waveform
						m.SetEffectCommand(CMD_S3MCMDEX, 0x40 | (m.param & 0x0F));
						break;

					// Sample commands
					case 0x28: // 3-byte offset - we only support the middle byte.
						m.SetEffectCommand(CMD_OFFSET, patternChunk.ReadArray<uint8, 2>()[0]);
						break;
					case 0x29: // retrigger
						m.SetEffectCommand(CMD_RETRIG, m.param & 0x0F);
						break;
					case 0x2A: // note cut
						m.command = CMD_S3MCMDEX;
#ifdef MODPLUG_TRACKER
						if(m.param == 0)	// in S3M mode, SC0 is ignored, so we convert it to a note cut.
						{
							if(m.note == NOTE_NONE)
							{
								m.note = NOTE_NOTECUT;
								m.command = CMD_NONE;
							} else
							{
								m.param = 1;
							}
						}
#endif // MODPLUG_TRACKER
						m.param |= 0xC0;
						break;
					case 0x2B: // note delay
						m.SetEffectCommand(CMD_S3MCMDEX, 0xD0 | (m.param & 0x0F));
						break;

					// Position change
					case 0x32: // position jump
						m.command = CMD_POSITIONJUMP;
						break;
					case 0x33: // pattern break
						m.command = CMD_PATTERNBREAK;
						break;
					case 0x34: // loop pattern
						m.SetEffectCommand(CMD_S3MCMDEX, 0xB0 | (m.param & 0x0F));
						break;
					case 0x35: // pattern delay
						m.SetEffectCommand(CMD_S3MCMDEX, 0xE0 | (m.param & 0x0F));
						break;

					// speed change
					case 0x3C: // set speed
						m.command = CMD_SPEED;
						break;
					case 0x3D: // set tempo
						m.command = CMD_TEMPO;
						break;

					// misc commands
					case 0x46: // arpeggio
						m.command = CMD_ARPEGGIO;
						break;
					case 0x47: // set finetune
						m.SetEffectCommand(CMD_S3MCMDEX, 0x20 | (m.param & 0x0F));
						break;
					case 0x48: // set balance (panning?)
						m.SetEffectCommand(CMD_S3MCMDEX, 0x80 | (m.param & 0x0F));
						break;

					default:
						m.command = CMD_NONE;
						break;
					}
				}
			}
		}
	}

	if(fileHeader.commentsOffset != 0)
	{
		file.Seek(fileHeader.commentsOffset);
		m_songMessage.Read(file, file.ReadUint16LE(), SongMessage::leAutodetect);
	}

	return true;
}


OPENMPT_NAMESPACE_END
