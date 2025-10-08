/*
 * Load_dsym.cpp
 * -------------
 * Purpose: Digital Symphony module loader
 * Notes  : Based on information from the DSym_Info file and sigma-delta decompression code from TimPlayer.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "BitReader.h"
#include "mpt/endian/int24.hpp"

OPENMPT_NAMESPACE_BEGIN

struct DSymFileHeader
{
	using uint24le = mpt::uint24le;

	char     magic[8];
	uint8le  version;      // 0 / 1
	uint8le  numChannels;  // 1...8
	uint16le numOrders;    // 0...4096
	uint16le numTracks;    // 0...4096
	uint24le infoLen;

	bool Validate() const
	{
		return !std::memcmp(magic, "\x02\x01\x13\x13\x14\x12\x01\x0B", 8)
			&& version <= 1
			&& numChannels >= 1 && numChannels <= 8
			&& numOrders <= 4096
			&& numTracks <= 4096;
	}

	uint64 GetHeaderMinimumAdditionalSize() const
	{
		return 72u;
	}
};

MPT_BINARY_STRUCT(DSymFileHeader, 17)


static std::vector<std::byte> DecompressDSymLZW(FileReader &file, uint32 size)
{
	BitReader bitFile(file);
	const auto startPos = bitFile.GetPosition();

	// In the best case, 13 bits decode 8192 bytes, a ratio of approximately 1:5042.
	// Too much for reserving memory in case of malformed files, just choose an arbitrary but realistic upper limit.
	std::vector<std::byte> output;
	output.reserve(std::min(size, std::min(mpt::saturate_cast<uint32>(file.BytesLeft()), Util::MaxValueOfType(size) / 50u) * 50u));

	static constexpr uint16 lzwBits = 13, MaxNodes = 1 << lzwBits;
	static constexpr uint16 ResetDict = 256, EndOfStream = 257;

	struct LZWEntry
	{
		uint16 prev;
		std::byte value;
	};
	std::vector<LZWEntry> dictionary(MaxNodes);
	std::vector<std::byte> match(MaxNodes);

	// Initialize dictionary
	for(int i = 0; i < 256; i++)
	{
		dictionary[i].prev = MaxNodes;
		dictionary[i].value = static_cast<std::byte>(i);
	}
	uint8 codeSize = 9;
	uint16 prevCode = 0;
	uint16 nextIndex = 257;
	while(true)
	{
		// Read next code
		const auto newCode = static_cast<uint16>(bitFile.ReadBits(codeSize));
		if(newCode == EndOfStream || newCode > nextIndex || output.size() >= size)
			break;

		// Reset dictionary
		if(newCode == ResetDict)
		{
			codeSize = 9;
			prevCode = 0;
			nextIndex = 257;
			continue;
		}

		// Output
		auto code = (newCode < nextIndex) ? newCode : prevCode;
		auto writeOffset = MaxNodes;
		do
		{
			match[--writeOffset] = dictionary[code].value;
			code = dictionary[code].prev;
		} while(code < MaxNodes);
		output.insert(output.end(), match.begin() + writeOffset, match.end());

		// Handling for KwKwK problem
		if(newCode == nextIndex)
			output.push_back(match[writeOffset]);

		// Add to dictionary
		if(nextIndex < MaxNodes)
		{
			// Special case for FULLEFFECT, NARCOSIS and NEWDANCE, which end with a dictionary size of 512
			// right before the end-of-stream token, but the code size is expected to be 9
			if(output.size() >= size)
				continue;

			dictionary[nextIndex].value = match[writeOffset];
			dictionary[nextIndex].prev = prevCode;

			nextIndex++;
			if(nextIndex != MaxNodes && nextIndex == (1u << codeSize))
				codeSize++;
		}

		prevCode = newCode;
	}
	MPT_ASSERT(output.size() == size);

	// Align length to 4 bytes
	file.Seek(startPos + ((bitFile.GetPosition() - startPos + 3u) & ~FileReader::pos_type(3)));
	// cppcheck false-positive
	// cppcheck-suppress returnDanglingLifetime
	return output;
}


static std::vector<std::byte> DecompressDSymSigmaDelta(FileReader &file, uint32 size)
{
	const uint8 maxRunLength = std::max(file.ReadUint8(), uint8(1));

	BitReader bitFile(file);
	const auto startPos = bitFile.GetPosition();

	// In the best case, sigma-delta compression represents each sample point as one bit.
	// As a result, if we have a file length of n, we know that the sample can be at most n*8 sample points long.
	LimitMax(size, std::min(mpt::saturate_cast<uint32>(file.BytesLeft()), Util::MaxValueOfType(size) / 8u) * 8u);
	std::vector<std::byte> output(size);

	uint32 pos = 0;
	uint8 runLength = maxRunLength;
	uint8 numBits = 8;
	uint8 accum = static_cast<uint8>(bitFile.ReadBits(numBits));
	output[pos++] = mpt::byte_cast<std::byte>(accum);

	while(pos < size)
	{
		const uint32 value = bitFile.ReadBits(numBits);
		// Increase bit width
		if(value == 0)
		{
			if(numBits >= 9)
				break;
			numBits++;
			runLength = maxRunLength;
			continue;
		}

		if(value & 1)
			accum -= static_cast<uint8>(value >> 1);
		else
			accum += static_cast<uint8>(value >> 1);
		output[pos++] = mpt::byte_cast<std::byte>(accum);

		// Reset run length if high bit is set
		if((value >> (numBits - 1u)) != 0)
		{
			runLength = maxRunLength;
			continue;
		}
		// Decrease bit width
		if(--runLength == 0)
		{
			if(numBits > 1)
				numBits--;
			runLength = maxRunLength;
		}
	}

	// Align length to 4 bytes
	file.Seek(startPos + ((bitFile.GetPosition() - startPos + 3u) & ~FileReader::pos_type(3)));
	return output;
}


static bool ReadDSymChunk(FileReader &file, std::vector<std::byte> &data, uint32 size)
{
	const uint8 packingType = file.ReadUint8();
	if(packingType > 1)
		return false;
	if(packingType)
	{
		try
		{
			data = DecompressDSymLZW(file, size);
		} catch(const BitReader::eof &)
		{
			return false;
		}
	} else
	{
		if(!file.CanRead(size))
			return false;
		file.ReadVector(data, size);
	}
	return data.size() >= size;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderDSym(MemoryFileReader file, const uint64 *pfilesize)
{
	DSymFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.Validate())
		return ProbeFailure;
	return ProbeAdditionalSize(file, pfilesize, fileHeader.GetHeaderMinimumAdditionalSize());
}


bool CSoundFile::ReadDSym(FileReader &file, ModLoadingFlags loadFlags)
{
	DSymFileHeader fileHeader;

	file.Rewind();
	if(!file.ReadStruct(fileHeader) || !fileHeader.Validate())
		return false;
	if(!file.CanRead(mpt::saturate_cast<FileReader::pos_type>(fileHeader.GetHeaderMinimumAdditionalSize())))
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_MOD, fileHeader.numChannels);
	m_SongFlags.set(SONG_IMPORTED | SONG_AMIGALIMITS);
	m_SongFlags.reset(SONG_ISAMIGA);
	m_nSamples = 63;

	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].nPan = (((chn & 3) == 1) || ((chn & 3) == 2)) ? 64 : 192;
	}

	uint8 sampleNameLength[64] = {};
	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		Samples[smp].Initialize(MOD_TYPE_MOD);
		sampleNameLength[smp] = file.ReadUint8();
		if(!(sampleNameLength[smp] & 0x80))
			Samples[smp].nLength = file.ReadUint24LE() << 1;
	}

	file.ReadSizedString<uint8le, mpt::String::spacePadded>(m_songName);

	const auto allowedCommands = file.ReadArray<uint8, 8>();

	std::vector<uint16le> sequence;
	if(fileHeader.numOrders)
	{
		std::vector<std::byte> sequenceData;
		const uint32 sequenceSize = fileHeader.numOrders * fileHeader.numChannels * 2u;
		if(!ReadDSymChunk(file, sequenceData, sequenceSize))
			return false;
		FileReader sequenceChunk = FileReader(mpt::as_span(sequenceData));
		sequenceChunk.ReadVector(sequence, sequenceData.size() / 2u);
	}

	std::vector<std::byte> trackData;
	trackData.reserve(fileHeader.numTracks * 256u);
	// For some reason, patterns are stored in 512K chunks
	for(uint16 offset = 0; offset < fileHeader.numTracks; offset += 2000)
	{
		const uint32 chunkSize = std::min(fileHeader.numTracks - offset, 2000) * 256;
		std::vector<std::byte> chunk;
		if(!ReadDSymChunk(file, chunk, chunkSize))
			return false;
		trackData.insert(trackData.end(), chunk.begin(), chunk.end());
	}
	const auto tracks = mpt::byte_cast<mpt::span<uint8>>(mpt::as_span(trackData));

	Order().resize(fileHeader.numOrders);
	for(ORDERINDEX pat = 0; pat < fileHeader.numOrders; pat++)
	{
		Order()[pat] = pat;
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
			continue;

		for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
		{
			const uint16 track = sequence[pat * GetNumChannels() + chn];
			if(track >= fileHeader.numTracks)
				continue;

			ModCommand *m = Patterns[pat].GetpModCommand(0, chn);
			for(ROWINDEX row = 0; row < 64; row++, m += GetNumChannels())
			{
				const auto data = tracks.subspan(track * 256 + row * 4, 4);
				m->note = data[0] & 0x3F;
				if(m->note)
					m->note += 47 + NOTE_MIN;
				else
					m->note = NOTE_NONE;

				m->instr = (data[0] >> 6) | ((data[1] & 0x0F) << 2);
				const uint8 command = (data[1] >> 6) | ((data[2] & 0x0F) << 2);
				const uint16 param = (data[2] >> 4) | (data[3] << 4);

				if(!(allowedCommands[command >> 3u] & (1u << (command & 7u))))
					continue;
				if(command == 0 && param == 0)
					continue;

				m->param = static_cast<uint8>(param);
				m->vol = static_cast<ModCommand::VOL>(param >> 8);

				switch(command)
				{
					case 0x00:  // 00 xyz Normal play or Arpeggio + Volume Slide Up
					case 0x01:  // 01 xyy Slide Up + Volume Slide Up
					case 0x02:  // 01 xyy Slide Up + Volume Slide Up
					case 0x20:  // 20 xyz Normal play or Arpeggio + Volume Slide Down
					case 0x21:  // 21 xyy Slide Up + Volume Slide Down
					case 0x22:  // 22 xyy Slide Down + Volume Slide Down
						ConvertModCommand(*m, command & 0x0F, m->param);
						if(m->vol)
							m->volcmd = (command < 0x20) ? VOLCMD_VOLSLIDEUP : VOLCMD_VOLSLIDEDOWN;
						break;
					case 0x03:  // 03 xyy Tone Portamento
					case 0x04:  // 04 xyz Vibrato
					case 0x05:  // 05 xyz Tone Portamento + Volume Slide
					case 0x06:  // 06 xyz Vibrato + Volume Slide
					case 0x07:  // 07 xyz Tremolo
					case 0x0C:  // 0C xyy Set Volume
						ConvertModCommand(*m, command, m->param);
						break;
					case 0x09:  // 09 xxx Set Sample Offset
						m->command = CMD_OFFSET;
						m->param = static_cast<ModCommand::PARAM>(param >> 1);
						if(param >= 0x200)
						{
							m->volcmd = VOLCMD_OFFSET;
							m->vol >>= 1;
						}
						break;
					case 0x0A:  // 0A xyz Volume Slide + Fine Slide Up
					case 0x2A:  // 2A xyz Volume Slide + Fine Slide Down
						if(param < 0xFF)
						{
							ConvertModCommand(*m, command & 0x0F, m->param);
						} else
						{
							m->command = CMD_MODCMDEX;
							m->param = static_cast<ModCommand::PARAM>(((command < 0x20) ? 0x10 : 0x20) | (param >> 8));
							if(param & 0xF0)
							{
								m->volcmd = VOLCMD_VOLSLIDEUP;
								m->vol = static_cast<ModCommand::VOL>((param >> 4) & 0x0F);
							} else
							{
								m->volcmd = VOLCMD_VOLSLIDEDOWN;
								m->vol = static_cast<ModCommand::VOL>(param & 0x0F);
							}
						}
						break;
					case 0x0B:  // 0B xxx Position Jump
					case 0x0F:  // 0F xxx Set Speed
						m->command = (command == 0x0B) ? CMD_POSITIONJUMP : CMD_SPEED;
						m->param = mpt::saturate_cast<ModCommand::PARAM>(param);
						break;
					case 0x0D:  // 0D xyy Pattern Break (not BCD-encoded like in MOD)
						m->command = CMD_PATTERNBREAK;
						if(m->param > 63)
							m->param = 0;
						break;
					case 0x10:  // 10 xxy Filter Control (not implemented in Digital Symphony)
					case 0x13:  // 13 xxy Glissando Control
					case 0x14:  // 14 xxy Set Vibrato Waveform
					case 0x15:  // 15 xxy Set Fine Tune
					case 0x17:  // 17 xxy Set Tremolo Waveform
					case 0x1F:  // 1F xxy Invert Loop
						m->command = CMD_MODCMDEX;
						m->param = (command << 4) | (m->param & 0x0F);
						break;
					case 0x16:  // 16 xxx Jump to Loop
					case 0x19:  // 19 xxx Retrig Note
					case 0x1C:  // 1C xxx Note Cut
					case 0x1D:  // 1D xxx Note Delay
					case 0x1E:  // 1E xxx Pattern Delay
						m->command = CMD_MODCMDEX;
						m->param = (command << 4) | static_cast<ModCommand::PARAM>(std::min(param, uint16(0x0F)));
						break;
					case 0x11:  // 11 xyy Fine Slide Up + Fine Volume Slide Up
					case 0x12:  // 12 xyy Fine Slide Down + Fine Volume Slide Up
					case 0x1A:  // 1A xyy Fine Slide Up + Fine Volume Slide Down
					case 0x1B:  // 1B xyy Fine Slide Down + Fine Volume Slide Down
						m->command = CMD_MODCMDEX;
						if(m->param & 0xFF)
						{
							m->param = static_cast<ModCommand::PARAM>(((command == 0x11 || command == 0x1A) ? 0x10 : 0x20) | (param & 0x0F));
							if(param & 0xF00)
								m->volcmd = (command >= 0x1A) ? VOLCMD_FINEVOLDOWN : VOLCMD_FINEVOLUP;
						} else
						{
							m->param = static_cast<ModCommand::PARAM>(((command >= 0x1A) ? 0xB0 : 0xA0) | (param >> 8));
						}
						break;
					case 0x2F:  // 2F xxx Set Tempo
						if(param > 0)
						{
							m->command = CMD_TEMPO;
							m->param = mpt::saturate_cast<ModCommand::PARAM>(std::max(8, param + 4) / 8);
#ifdef MODPLUG_TRACKER
							m->param = std::max(m->param, ModCommand::PARAM(0x20));
#endif
						} else
						{
							m->command = CMD_NONE;
						}
						break;
					case 0x2B:  // 2B xyy Line Jump
						m->command = CMD_PATTERNBREAK;
						for(CHANNELINDEX brkChn = 0; brkChn < GetNumChannels(); brkChn++)
						{
							ModCommand &cmd = *(m - chn + brkChn);
							if(cmd.command != CMD_NONE)
								continue;
							cmd.command = CMD_POSITIONJUMP;
							cmd.param = mpt::saturate_cast<ModCommand::PARAM>(pat);
						}
						break;
					case 0x30:  // 30 xxy Set Stereo
						m->command = CMD_PANNING8;
						if(param & 7)
						{
							static constexpr uint8 panning[8] = {0x00, 0x00, 0x2B, 0x56, 0x80, 0xAA, 0xD4, 0xFF};
							m->param = panning[param & 7];
						} else if((param >> 4) != 0x80)
						{
							m->param = static_cast<ModCommand::PARAM>(param >> 4);
							if(m->param < 0x80)
								m->param += 0x80;
							else
								m->param = 0xFF - m->param;
						} else
						{
							m->command = CMD_NONE;
						}
						break;
					case 0x32:  // 32 xxx Unset Sample Repeat
						m->command = CMD_NONE;
						m->param = 0;
						if(m->note == NOTE_NONE)
							m->note = NOTE_KEYOFF;
						else
							m->command = CMD_KEYOFF;
						break;
					case 0x31:  // 31 xxx Song Upcall
					default:
						m->command = CMD_NONE;
						break;
				}
			}
		}
	}

	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		file.ReadString<mpt::String::maybeNullTerminated>(m_szNames[smp], sampleNameLength[smp] & 0x3F);

		if(sampleNameLength[smp] & 0x80)
			continue;

		ModSample &mptSmp = Samples[smp];
		mptSmp.nSustainStart = file.ReadUint24LE() << 1;
		if(const auto loopLen = file.ReadUint24LE() << 1; loopLen > 2)
		{
			mptSmp.nSustainEnd = mptSmp.nSustainStart + loopLen;
			mptSmp.uFlags.set(CHN_SUSTAINLOOP);
		}
		mptSmp.nVolume = std::min(file.ReadUint8(), uint8(64)) * 4u;
		mptSmp.nFineTune = MOD2XMFineTune(file.ReadUint8());
		mptSmp.Set16BitCuePoints();

		if(!mptSmp.nLength)
			continue;

		const uint8 packingType = file.ReadUint8();
		switch(packingType)
		{
		case 0:  // Modified u-Law
			if(loadFlags & loadSampleData)
			{
				std::vector<std::byte> sampleData;
				if(!file.CanRead(mptSmp.nLength))
					return false;
				file.ReadVector(sampleData, mptSmp.nLength);
				for(auto &b : sampleData)
				{
					uint8 v = mpt::byte_cast<uint8>(b);
					v = (v << 7) | (static_cast<uint8>(~v) >> 1);
					b = mpt::byte_cast<std::byte>(v);
				}

				FileReader sampleDataFile = FileReader(mpt::as_span(sampleData));
				SampleIO(
					SampleIO::_16bit,
					SampleIO::mono,
					SampleIO::littleEndian,
					SampleIO::uLaw)
					.ReadSample(mptSmp, sampleDataFile);
			} else
			{
				file.Skip(mptSmp.nLength);
			}
			break;
		case 1:  // 13-bit LZW applied to linear sample data differences
			{
				std::vector<std::byte> sampleData;
				try
				{
					sampleData = DecompressDSymLZW(file, mptSmp.nLength);
				} catch(const BitReader::eof &)
				{
					return false;
				}
				if(!(loadFlags & loadSampleData))
					break;
				FileReader sampleDataFile = FileReader(mpt::as_span(sampleData));
				SampleIO(
					SampleIO::_8bit,
					SampleIO::mono,
					SampleIO::littleEndian,
					SampleIO::deltaPCM)
					.ReadSample(mptSmp, sampleDataFile);
			}
			break;
		case 2:  // 8-bit signed
		case 3:  // 16-bit signed
			if(loadFlags & loadSampleData)
			{
				SampleIO(
					(packingType == 2) ? SampleIO::_8bit : SampleIO::_16bit,
					SampleIO::mono,
					SampleIO::littleEndian,
					SampleIO::signedPCM)
					.ReadSample(mptSmp, file);
			} else
			{
				file.Skip(mptSmp.nLength * (packingType - 1));
			}
			break;
		case 4:  // Sigma-Delta compression applied to linear sample differences
		case 5:  // Sigma-Delta compression applied to logarithmic sample differences
			{
				std::vector<std::byte> sampleData;
				try
				{
					sampleData = DecompressDSymSigmaDelta(file, mptSmp.nLength);
				} catch(const BitReader::eof &)
				{
					return false;
				}
				if(!(loadFlags & loadSampleData))
					break;
				if(packingType == 5)
				{
					static constexpr uint8 xorMask[] = {0x00, 0x7F};
					for(auto &b : sampleData)
					{
						uint8 v = mpt::byte_cast<uint8>(b);
						v ^= xorMask[v >> 7];
						b = mpt::byte_cast<std::byte>(v);
					}
				}

				FileReader sampleDataFile = FileReader(mpt::as_span(sampleData));
				SampleIO(
					(packingType == 5) ? SampleIO::_16bit : SampleIO::_8bit,
					SampleIO::mono,
					SampleIO::littleEndian,
					(packingType == 5) ? SampleIO::uLaw : SampleIO::unsignedPCM)
					.ReadSample(mptSmp, sampleDataFile);
			}
			break;
		default:
			return false;
		}
	}

	if(const uint32 infoLen = fileHeader.infoLen.get(); infoLen > 0)
	{
		std::vector<std::byte> infoData;
		if(!ReadDSymChunk(file, infoData, infoLen))
			return false;
		FileReader infoChunk = FileReader(mpt::as_span(infoData));
		m_songMessage.Read(infoChunk, infoLen, SongMessage::leLF);
	}

	m_modFormat.formatName = MPT_UFORMAT("Digital Symphony v{}")(fileHeader.version);
	m_modFormat.type = UL_("dsym");  // RISC OS doesn't use file extensions but this is a common abbreviation used for this tracker
	m_modFormat.madeWithTracker = UL_("Digital Symphony");
	m_modFormat.charset = mpt::Charset::RISC_OS;

	return true;
}


OPENMPT_NAMESPACE_END

