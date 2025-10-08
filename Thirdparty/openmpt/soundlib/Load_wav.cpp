/*
 * Load_wav.cpp
 * ------------
 * Purpose: WAV importer
 * Notes  : This loader converts each WAV channel into a separate mono sample.
 * Authors: Olivier Lapicque
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "WAVTools.h"
#include "openmpt/soundbase/SampleConvert.hpp"
#include "openmpt/soundbase/SampleDecode.hpp"
#include "SampleCopy.h"


OPENMPT_NAMESPACE_BEGIN


/////////////////////////////////////////////////////////////
// WAV file support


template <typename SampleConversion>
static bool CopyWavChannel(ModSample &sample, const FileReader &file, size_t channelIndex, size_t numChannels, SampleConversion conv = SampleConversion())
{
	MPT_ASSERT(sample.GetNumChannels() == 1);
	MPT_ASSERT(sample.GetElementarySampleSize() == sizeof(typename SampleConversion::output_t));

	const size_t offset = channelIndex * sizeof(typename SampleConversion::input_t) * SampleConversion::input_inc;

	if(sample.AllocateSample() == 0 || !file.CanRead(offset))
	{
		return false;
	}

	FileReader::PinnedView inData = file.GetPinnedView(file.BytesLeft());
	CopySample<SampleConversion>(sample.template sample<typename SampleConversion::output_t>(), sample.nLength, 1, inData.data() + offset, inData.size() - offset, numChannels, conv);
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderWAV(MemoryFileReader file, const uint64 *pfilesize)
{
	RIFFHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return ProbeWantMoreData;
	}
	if((fileHeader.magic != RIFFHeader::idRIFF && fileHeader.magic != RIFFHeader::idLIST)
		|| (fileHeader.type != RIFFHeader::idWAVE && fileHeader.type != RIFFHeader::idwave))
	{
		return ProbeFailure;
	}
	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadWAV(FileReader &file, ModLoadingFlags loadFlags)
{
	WAVReader wavFile(file);

	if(!wavFile.IsValid()
	   || wavFile.GetNumChannels() == 0
	   || wavFile.GetNumChannels() > MAX_BASECHANNELS
	   || wavFile.GetNumChannels() >= MAX_SAMPLES
	   || wavFile.GetBitsPerSample() == 0
	   || wavFile.GetBitsPerSample() > 64
	   || (wavFile.GetBitsPerSample() < 32 && wavFile.GetSampleFormat() == WAVFormatChunk::fmtFloat)
	   || (wavFile.GetSampleFormat() != WAVFormatChunk::fmtPCM && wavFile.GetSampleFormat() != WAVFormatChunk::fmtFloat))
	{
		return false;
	} else if(loadFlags == onlyVerifyHeader)
	{
		return true;
	}

	InitializeGlobals(MOD_TYPE_MPT, std::max(wavFile.GetNumChannels(), uint16(2)));
	m_ContainerType = ModContainerType::WAV;
	Patterns.ResizeArray(2);
	if(!Patterns.Insert(0, 64) || !Patterns.Insert(1, 64))
	{
		return false;
	}

	m_modFormat.formatName = UL_("RIFF WAVE");
	m_modFormat.type = UL_("wav");
	m_modFormat.charset = mpt::Charset::Windows1252;
	
	const SmpLength sampleLength = wavFile.GetSampleLength();

	// Setting up module length
	// Calculate sample length in ticks at tempo 125
	const uint32 sampleRate = std::max(uint32(1), wavFile.GetSampleRate());
	const uint32 sampleTicks = mpt::saturate_cast<uint32>(((sampleLength * 50) / sampleRate) + 1);
	uint32 ticksPerRow = std::max((sampleTicks + 63u) / 63u, uint32(1));

	Order().assign(1, 0);
	ORDERINDEX numOrders = 1;
	while(ticksPerRow >= 32 && numOrders < MAX_ORDERS)
	{
		numOrders++;
		ticksPerRow = (sampleTicks + (64 * numOrders - 1)) / (64 * numOrders);
	}
	Order().resize(numOrders, 1);

	m_nSamples = wavFile.GetNumChannels();
	m_nInstruments = 0;
	Order().SetDefaultSpeed(ticksPerRow);
	Order().SetDefaultTempoInt(125);
	m_SongFlags = SONG_LINEARSLIDES;

	for(CHANNELINDEX channel = 0; channel < GetNumChannels(); channel++)
	{
		ChnSettings[channel].nPan = (channel % 2u) ? 256 : 0;
	}

	// Setting up pattern
	auto row = Patterns[0].GetRow(0);
	row[0].note = row[1].note = NOTE_MIDDLEC;
	row[0].instr = row[1].instr = 1;

	const FileReader sampleChunk = wavFile.GetSampleData();

	// Read every channel into its own sample lot.
	for(SAMPLEINDEX channel = 0; channel < GetNumSamples(); channel++)
	{
		row[channel].note = row[0].note;
		row[channel].instr = static_cast<ModCommand::INSTR>(channel + 1);

		ModSample &sample = Samples[channel + 1];
		sample.Initialize();
		sample.uFlags = CHN_PANNING;
		sample.nLength =  sampleLength;
		sample.nC5Speed = wavFile.GetSampleRate();
		m_szNames[channel + 1] = "";
		wavFile.ApplySampleSettings(sample, GetCharsetInternal(), m_szNames[channel + 1]);

		if(wavFile.GetNumChannels() > 1)
		{
			// Pan all samples appropriately
			switch(channel)
			{
			case 0:
				sample.nPan = 0;
				break;
			case 1:
				sample.nPan = 256;
				break;
			case 2:
				sample.nPan = (wavFile.GetNumChannels() == 3 ? 128u : 64u);
				row[channel].command = CMD_S3MCMDEX;
				row[channel].param = 0x91;
				break;
			case 3:
				sample.nPan = 192;
				row[channel].command = CMD_S3MCMDEX;
				row[channel].param = 0x91;
				break;
			default:
				sample.nPan = 128;
				break;
			}
		}

		if(wavFile.GetBitsPerSample() > 8)
		{
			sample.uFlags.set(CHN_16BIT);
		}

		if(wavFile.GetSampleFormat() == WAVFormatChunk::fmtFloat)
		{
			if(wavFile.GetBitsPerSample() <= 32)
				CopyWavChannel<SC::ConversionChain<SC::Convert<int16, somefloat32>, SC::DecodeFloat32<littleEndian32>>>(sample, sampleChunk, channel, wavFile.GetNumChannels());
			else
				CopyWavChannel<SC::ConversionChain<SC::Convert<int16, somefloat64>, SC::DecodeFloat64<littleEndian64>>>(sample, sampleChunk, channel, wavFile.GetNumChannels());
		} else
		{
			if(wavFile.GetBitsPerSample() <= 8)
				CopyWavChannel<SC::DecodeUint8>(sample, sampleChunk, channel, wavFile.GetNumChannels());
			else if(wavFile.GetBitsPerSample() <= 16)
				CopyWavChannel<SC::DecodeInt16<0, littleEndian16>>(sample, sampleChunk, channel, wavFile.GetNumChannels());
			else if(wavFile.GetBitsPerSample() <= 24)
				CopyWavChannel<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt24<0, littleEndian24>>>(sample, sampleChunk, channel, wavFile.GetNumChannels());
			else if(wavFile.GetBitsPerSample() <= 32)
				CopyWavChannel<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt32<0, littleEndian32>>>(sample, sampleChunk, channel, wavFile.GetNumChannels());
			else if(wavFile.GetBitsPerSample() <= 64)
				CopyWavChannel<SC::ConversionChain<SC::Convert<int16, int64>, SC::DecodeInt64<0, littleEndian64>>>(sample, sampleChunk, channel, wavFile.GetNumChannels());
		}
		sample.PrecomputeLoops(*this, false);

	}

	return true;
}


OPENMPT_NAMESPACE_END
