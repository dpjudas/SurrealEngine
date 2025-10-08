/*
 * Load_kris.cpp
 * -------------
 * Purpose: ChipTracker loader
 * Notes  : Another NoiseTracker variant, storing tracks instead of patterns (like ICE / ST26)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "MODTools.h"

OPENMPT_NAMESPACE_BEGIN

CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderKRIS(MemoryFileReader file, const uint64 *pfilesize)
{
	if(!file.CanRead(952 + 4))
		return ProbeWantMoreData;

	file.Seek(952);
	if(!file.ReadMagic("KRIS"))
		return ProbeFailure;
	
	const auto [numOrders, restartPos] = file.ReadArray<uint8, 2>();
	if(numOrders > 128 || restartPos > 127)
		return ProbeFailure;

	file.Seek(22);
	uint32 invalidBytes = 0;
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		MODSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		if(sampleHeader.name[0] != 0)
			invalidBytes += sampleHeader.GetInvalidByteScore();
		if(invalidBytes > MODSampleHeader::INVALID_BYTE_THRESHOLD)
			return ProbeFailure;
	}
	
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadKRIS(FileReader &file, ModLoadingFlags loadFlags)
{
	if(!file.Seek(952) || !file.ReadMagic("KRIS"))
		return false;

	const auto [numOrders, restartPos] = file.ReadArray<uint8, 2>();
	if(numOrders > 128 || restartPos > 127)
		return false;

	std::array<std::array<uint8, 2>, 128 * 4> tracks;
	if(!file.ReadArray(tracks))
		return false;
	uint32 tracksOffset = 1984;

	InitializeGlobals(MOD_TYPE_MOD, 4);

	file.Seek(0);
	file.ReadString<mpt::String::spacePadded>(m_songName, 22);

	m_nSamples = 31;
	uint32 invalidBytes = 0;
	uint8 numSynthWaveforms = 0;
	for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
	{
		MODSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		if(sampleHeader.name[0] != 0)
		{
			invalidBytes += ReadMODSample(sampleHeader, Samples[smp], m_szNames[smp], true);
		} else
		{
			// Unfinished feature. Synth parameters are stored in the module, but loading and saving of synth waveforms
			// (which I'd assume the extra space before the track data is reserved for) is completely absent, making the feature useless.
			Samples[smp].Initialize(MOD_TYPE_MOD);
			m_szNames[smp] = "Synthetic";
			const uint8 maxWaveform = std::max({ sampleHeader.name[1], sampleHeader.name[5], sampleHeader.name[10], sampleHeader.name[19] });
			if(maxWaveform)
				numSynthWaveforms = std::max(numSynthWaveforms, static_cast<uint8>(maxWaveform + 1));
		}
		if(invalidBytes > MODSampleHeader::INVALID_BYTE_THRESHOLD)
			return false;
	}
	tracksOffset += numSynthWaveforms * 64u;

	if(loadFlags == onlyVerifyHeader)
		return true;

	SetupMODPanning(true);
	Order().SetDefaultSpeed(6);
	Order().SetDefaultTempoInt(125);
	Order().SetRestartPos(restartPos);
	m_nMinPeriod = 113 * 4;
	m_nMaxPeriod = 856 * 4;
	m_nSamplePreAmp = 64;
	m_SongFlags.set(SONG_PT_MODE | SONG_IMPORTED | SONG_FORMAT_NO_VOLCOL);
	m_playBehaviour.set(kMODIgnorePanning);
	m_playBehaviour.set(kMODSampleSwap);
	
	Order().resize(numOrders);
	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(numOrders);
	for(PATTERNINDEX pat = 0; pat < numOrders; pat++)
	{
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
			break;
		Order()[pat] = pat;
		for(CHANNELINDEX chn = 0; chn < 4; chn++)
		{
			const uint8 track = tracks[pat * 4u + chn][0];
			const int8 transpose = tracks[pat * 4u + chn][1];
			if(!file.Seek(tracksOffset + track * 256u))
				return false;
			ModCommand *m = Patterns[pat].GetpModCommand(0, chn);
			for(ROWINDEX row = 0; row < 64; row++, m += 4)
			{
				const auto data = file.ReadArray<uint8, 4>();
				if(data[0] & 1)
					return false;
				if(data[0] >= 0x18 && data[0] <= 0x9E)
					m->note = static_cast<ModCommand::NOTE>(Clamp(NOTE_MIDDLEC - 36 + (data[0] - 0x18) / 2 + transpose, NOTE_MIDDLEC - 12, NOTE_MIDDLEC + 23));
				else if(data[0] != 0xA8)
					return false;
				if(data[2] >> 4)
					return false;
				m->instr = data[1];
				ConvertModCommand(*m, data[2] & 0x0F, data[3]);
			}
		}
	}

	m_modFormat.formatName = UL_("ChipTracker");
	m_modFormat.type = UL_("mod");
	m_modFormat.madeWithTracker = UL_("ChipTracker");
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	if(loadFlags & loadSampleData)
	{
		uint8 maxTrack = 0;
		for(uint32 ord = 0; ord < numOrders * 4u; ord++)
		{
			maxTrack = std::max(tracks[ord][0], maxTrack);
		}
		file.Seek(tracksOffset + (maxTrack + 1u) * 256u);
		for(SAMPLEINDEX smp = 1; smp <= m_nSamples; smp++)
			if(Samples[smp].nLength)
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
