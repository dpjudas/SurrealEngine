/*
 * Load_ice.cpp
 * ------------
 * Purpose: ST26 (SoundTracker 2.6 / Ice Tracker) loader
 * Notes  : The only real difference to other SoundTracker formats is the way patterns are stored:
 *          Every pattern consists of four independent, re-usable tracks.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "MODTools.h"

OPENMPT_NAMESPACE_BEGIN

CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderICE(MemoryFileReader file, const uint64 *pfilesize)
{
	if(!file.CanRead(1464 + 4))
		return ProbeWantMoreData;
	
	file.Seek(1464);
	char magic[4];
	file.ReadArray(magic);
	if(!IsMagic(magic, "MTN\0") && !IsMagic(magic, "IT10"))
		return ProbeFailure;

	file.Seek(20);
	uint32 invalidBytes = 0;
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		MODSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		invalidBytes += sampleHeader.GetInvalidByteScore();
		if(invalidBytes > MODSampleHeader::INVALID_BYTE_THRESHOLD)
			return ProbeFailure;
	}
	const auto [numOrders, numTracks] = file.ReadArray<uint8, 2>();
	if(numOrders > 128)
	{
		return ProbeFailure;
	}
	std::array<uint8, 128 * 4> tracks;
	file.ReadArray(tracks);
	for(auto track : tracks)
	{
		if(track > numTracks)
			return ProbeFailure;
	}
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadICE(FileReader &file, ModLoadingFlags loadFlags)
{
	char magic[4];
	if(!file.Seek(1464) || !file.ReadArray(magic))
	{
		return false;
	}

	if(IsMagic(magic, "MTN\0"))
	{
		InitializeGlobals(MOD_TYPE_MOD, 4);
		m_modFormat.formatName = UL_("MnemoTroN SoundTracker");
		m_modFormat.type = UL_("st26");
		m_modFormat.madeWithTracker = UL_("SoundTracker 2.6");
		m_modFormat.charset = mpt::Charset::Amiga_no_C1;
	} else if(IsMagic(magic, "IT10"))
	{
		InitializeGlobals(MOD_TYPE_MOD, 4);
		m_modFormat.formatName = UL_("Ice Tracker");
		m_modFormat.type = UL_("ice");
		m_modFormat.madeWithTracker = UL_("Ice Tracker 1.0 / 1.1");
		m_modFormat.charset = mpt::Charset::Amiga_no_C1;
	} else
	{
		return false;
	}

	// Reading song title
	file.Seek(0);
	file.ReadString<mpt::String::spacePadded>(m_songName, 20);

	// Load Samples
	m_nSamples = 31;
	uint32 invalidBytes = 0;
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		MODSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		invalidBytes += ReadMODSample(sampleHeader, Samples[smp], m_szNames[smp], true);
		if(invalidBytes > MODSampleHeader::INVALID_BYTE_THRESHOLD)
			return false;
	}

	const auto [numOrders, numTracks] = file.ReadArray<uint8, 2>();
	if(numOrders > 128)
		return false;

	std::array<uint8, 128 * 4> tracks;
	file.ReadArray(tracks);
	for(auto track : tracks)
	{
		if(track > numTracks)
			return false;
	}

	if(loadFlags == onlyVerifyHeader)
		return true;

	// Now we can be pretty sure that this is a valid ICE file. Set up default song settings.
	SetupMODPanning(true);
	Order().SetDefaultSpeed(6);
	Order().SetDefaultTempoInt(125);
	m_nMinPeriod = 14 * 4;
	m_nMaxPeriod = 3424 * 4;
	m_nSamplePreAmp = 64;
	m_SongFlags.set(SONG_PT_MODE | SONG_IMPORTED | SONG_FORMAT_NO_VOLCOL);
	m_playBehaviour.reset(kMODOneShotLoops);
	m_playBehaviour.set(kMODIgnorePanning);
	m_playBehaviour.set(kMODSampleSwap);  // untested

	// Reading patterns
	Order().resize(numOrders);
	uint8 speed = 0;
	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(numOrders);
	for(PATTERNINDEX pat = 0; pat < numOrders; pat++)
	{
		Order()[pat] = pat;
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
			break;

		for(CHANNELINDEX chn = 0; chn < 4; chn++)
		{
			file.Seek(1468 + tracks[pat * 4 + chn] * 64u * 4u);
			ModCommand *m = Patterns[pat].GetpModCommand(0, chn);

			for(ROWINDEX row = 0; row < 64; row++, m += 4)
			{
				const auto [command, param] = ReadMODPatternEntry(file, *m);

				if((command || param)
				   && !(command == 0x0E && param >= 0x10)     // Exx only sets filter
				   && !(command >= 0x05 && command <= 0x09))  // These don't exist in ST2.6
				{
					ConvertModCommand(*m, command, param);
				} else
				{
					m->command = CMD_NONE;
				}
			}
		}

		// Handle speed command with both nibbles set - this enables auto-swing (alternates between the two nibbles)
		auto m = Patterns[pat].begin();
		for(ROWINDEX row = 0; row < 64; row++)
		{
			for(CHANNELINDEX chn = 0; chn < 4; chn++, m++)
			{
				if(m->command == CMD_SPEED || m->command == CMD_TEMPO)
				{
					m->command = CMD_SPEED;
					if(m->param & 0xF0)
					{
						if((m->param >> 4) != (m->param & 0x0F) && (m->param & 0x0F) != 0)
						{
							// Both nibbles set
							speed = m->param;
						}
						m->param >>= 4;
					}
				}
			}
			if(speed)
			{
				speed = mpt::rotr(speed, 4);
				Patterns[pat].WriteEffect(EffectWriter(CMD_SPEED, speed & 0x0F).Row(row));
			}
		}
	}

	// Reading samples
	if(loadFlags & loadSampleData)
	{
		file.Seek(1468 + numTracks * 64u * 4u);
		for(SAMPLEINDEX smp = 1; smp <= 31; smp++) if(Samples[smp].nLength)
		{
			SampleIO(
				SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::signedPCM)
				.ReadSample(Samples[smp], file);
		}
	}

	return true;
}

OPENMPT_NAMESPACE_END
