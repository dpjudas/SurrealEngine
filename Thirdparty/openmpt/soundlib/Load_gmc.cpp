/*
 * Load_gmc.cpp
 * ------------
 * Purpose: GMC (Game Music Creator) module loader
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#include "stdafx.h"
#include "Loaders.h"

OPENMPT_NAMESPACE_BEGIN

// Sample Header
struct GMCSampleHeader
{
	uint32be offset;
	uint16be length;
	uint8    zero;
	uint8    volume;
	uint32be address;
	uint16be loopLength;  // Loop start is implicit
	uint16be dataStart;   // Non-zero if sample was shortened in editor (amount of bytes trimmed from sample start)

	// Convert a GMC sample header to OpenMPT's internal sample header.
	void ConvertToMPT(ModSample &mptSmp) const
	{
		mptSmp.Initialize(MOD_TYPE_MOD);
		mptSmp.nLength = length * 2u;
		mptSmp.nVolume = 4u * std::min(volume, uint8(64));

		if(loopLength > 2)
		{
			mptSmp.nLoopStart = mptSmp.nLength - loopLength * 2u;
			mptSmp.nLoopEnd = mptSmp.nLength;
			mptSmp.uFlags.set(CHN_LOOP);
		}
	}

	bool IsValid() const
	{
		if(offset > 0x7F'FFFF || (offset & 1) || address > 0x7F'FFFF || (address & 1))
			return false;
		if(length > 0x7FFF || dataStart > 0x7FFF || (dataStart & 1))
			return false;
		if(loopLength > 2 && loopLength > length)
			return false;
		if(volume > 64)
			return false;
		if(zero != 0)
			return false;
		return true;
	}
};

MPT_BINARY_STRUCT(GMCSampleHeader, 16)


// File Header
struct GMCFileHeader
{
	GMCSampleHeader samples[15];
	uint8    zero[3];
	uint8    numOrders;
	uint16be orders[100];

	bool IsValid() const noexcept
	{
		for(const auto &sample : samples)
		{
			if(!sample.IsValid())
				return false;
		}
		if(zero[0] != 0 || zero[1] != 0 || zero[2] != 0)
			return false;
		if(!numOrders || numOrders > std::size(orders))
			return false;
		for(uint16 ord : orders)
		{
			if(ord % 1024u)
				return false;
		}
		return true;
	}
};

MPT_BINARY_STRUCT(GMCFileHeader, 444)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderGMC(MemoryFileReader file, const uint64 *pfilesize)
{
	GMCFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;
	if(!fileHeader.IsValid())
			return ProbeFailure;
	
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadGMC(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	GMCFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader) || !fileHeader.IsValid())
		return false;
	else if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_MOD, 4);
	m_nSamples = 15;
	m_nMinPeriod = 113 * 4;
	m_nMaxPeriod = 856 * 4;
	m_nSamplePreAmp = 64;
	m_SongFlags.set(SONG_FASTPORTAS | SONG_IMPORTED | SONG_FORMAT_NO_VOLCOL);
	Order().SetDefaultTempoInt(125);
	Order().SetDefaultSpeed(6);

	// Setup channel pan positions and volume
	SetupMODPanning(true);

	// Note: The Brides of Dracula modules contain a "hidden" pattern past fileHeader.numOrders.
	// This pattern would need to be read in order to align the samples correctly, however this would mean that the file size no longer matches
	// the expected size (the last sample would be missing 1024 samples at the end).
	// Looking at some gameplay footage, the playback is apparently broken ingame as well, and fixing it would break other modules (e.g. Jet Set Willy 2 Title).
	Order().resize(fileHeader.numOrders);
	PATTERNINDEX numPatterns = 0;
	for(ORDERINDEX ord = 0; ord < fileHeader.numOrders; ord++)
	{
		PATTERNINDEX pat = Order()[ord] = fileHeader.orders[ord] / 1024u;
		// Fix for Covert Action theme music (there appears to be a general bug in GMC export when pattern 63 is used)
		if(pat != 63)
			numPatterns = std::max(numPatterns, static_cast<PATTERNINDEX>(pat + 1));
	}

	for(SAMPLEINDEX smp = 1; smp <= 15; smp++)
	{
		fileHeader.samples[smp - 1].ConvertToMPT(Samples[smp]);
	}

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
				// Probably bad conversion from SoundFX Import? (Stryx title music)
				const bool noteCut = (data[0] == 0xFF && data[1] == 0xFE);
				if(noteCut)
					data[0] = 0;
				else if(data[0] & 0xF0)
					return false;

				uint8 command = data[2] & 0x0F, param = data[3];
				switch(command)
				{
				case 0x00:  // Nothing
					param = 0;
					break;
				case 0x01:  // Portamento up
				case 0x02:  // Portamento down
					break;
				case 0x03:  // Volume
					command = 0x0C;
					param &= 0x7F;
					break;
				case 0x04:  // Pattern break
					if(param > 0x63)
						return false;
					command = 0x0D;
					break;
				case 0x05:  // Position jump
					if(param > fileHeader.numOrders + 1)
						return false;
					command = 0x0B;
					break;
				case 0x06:  // LED filter on
				case 0x07:  // LED filter off
					param = command - 0x06;
					command = 0x0E;
					break;
				case 0x08:  // Set speed
					command = 0x0F;
					break;
				default:
					command = param = 0;
					break;
				}
				ReadMODPatternEntry(data, m);
				ConvertModCommand(m, command, param);
				if(noteCut)
					m.note = NOTE_NOTECUT;
				if(m.command == CMD_PORTAMENTOUP)
					m.command = CMD_AUTO_PORTAUP;
				else if(m.command == CMD_PORTAMENTODOWN)
					m.command = CMD_AUTO_PORTADOWN;
				else if(m.command == CMD_TEMPO)
					m.command = CMD_SPEED;
			}
		}
	}

	if(loadFlags & loadSampleData)
	{
		for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
		{
			if(!Samples[smp].nLength)
				continue;
			SampleIO(
				SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::signedPCM)
				.ReadSample(Samples[smp], file);
		}
	}

	m_modFormat.madeWithTracker = UL_("Game Music Creator");
	m_modFormat.formatName = UL_("Game Music Creator");
	m_modFormat.type = UL_("GMC");
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;  // No strings in this format...

	return true;
}

OPENMPT_NAMESPACE_END
