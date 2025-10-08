/*
 * S3MTools.cpp
 * ------------
 * Purpose: Definition of S3M file structures and helper functions
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "S3MTools.h"
#include "Loaders.h"
#include "../common/mptStringBuffer.h"


OPENMPT_NAMESPACE_BEGIN

// Convert an S3M sample header to OpenMPT's internal sample header.
void S3MSampleHeader::ConvertToMPT(ModSample &mptSmp, bool isST3) const
{
	mptSmp.Initialize(MOD_TYPE_S3M);
	mptSmp.filename = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, filename);

	if(sampleType == typePCM || sampleType == typeNone)
	{
		// Sample Length and Loops
		if(sampleType == typePCM)
		{
			mptSmp.nLength = length;
			mptSmp.nLoopStart = std::min(static_cast<SmpLength>(loopStart), mptSmp.nLength - 1);
			mptSmp.nLoopEnd = std::min(static_cast<SmpLength>(loopEnd), mptSmp.nLength);
			mptSmp.uFlags.set(CHN_LOOP, (flags & smpLoop) != 0);
		}

		if(mptSmp.nLoopEnd < 2 || mptSmp.nLoopStart >= mptSmp.nLoopEnd || mptSmp.nLoopEnd - mptSmp.nLoopStart < 1)
		{
			mptSmp.nLoopStart = mptSmp.nLoopEnd = 0;
			mptSmp.uFlags.reset();
		}
	} else if(sampleType == typeAdMel)
	{
		OPLPatch patch;
		std::memcpy(patch.data() + 0, mpt::as_raw_memory(length).data(), 4);
		std::memcpy(patch.data() + 4, mpt::as_raw_memory(loopStart).data(), 4);
		std::memcpy(patch.data() + 8, mpt::as_raw_memory(loopEnd).data(), 4);
		mptSmp.SetAdlib(true, patch);
	}

	// Volume / Panning
	mptSmp.nVolume = std::min(defaultVolume.get(), uint8(64)) * 4;

	// C-5 frequency
	mptSmp.nC5Speed = c5speed;
	if(isST3)
	{
		// ST3 ignores or clamps the high 16 bits depending on the instrument type
		if(sampleType == typeAdMel)
			mptSmp.nC5Speed &= 0xFFFF;
		else
			LimitMax(mptSmp.nC5Speed, uint16_max);
	}

	if(mptSmp.nC5Speed == 0)
		mptSmp.nC5Speed = 8363;
	else if(mptSmp.nC5Speed < 1024)
		mptSmp.nC5Speed = 1024;

}


// Convert OpenMPT's internal sample header to an S3M sample header.
SmpLength S3MSampleHeader::ConvertToS3M(const ModSample &mptSmp)
{
	SmpLength smpLength = 0;
	mpt::String::WriteBuf(mpt::String::maybeNullTerminated, filename) = mptSmp.filename;
	memcpy(magic, "SCRS", 4);

	if(mptSmp.uFlags[CHN_ADLIB])
	{
		memcpy(magic, "SCRI", 4);
		sampleType = typeAdMel;
		std::memcpy(mpt::as_raw_memory(length   ).data(), mptSmp.adlib.data() + 0, 4);
		std::memcpy(mpt::as_raw_memory(loopStart).data(), mptSmp.adlib.data() + 4, 4);
		std::memcpy(mpt::as_raw_memory(loopEnd  ).data(), mptSmp.adlib.data() + 8, 4);
	} else if(mptSmp.HasSampleData())
	{
		sampleType = typePCM;
		length = mpt::saturate_cast<uint32>(mptSmp.nLength);
		loopStart = mpt::saturate_cast<uint32>(mptSmp.nLoopStart);
		loopEnd = mpt::saturate_cast<uint32>(mptSmp.nLoopEnd);

		smpLength = length;

		flags = (mptSmp.uFlags[CHN_LOOP] ? smpLoop : 0);
		if(mptSmp.uFlags[CHN_16BIT])
		{
			flags |= smp16Bit;
		}
		if(mptSmp.uFlags[CHN_STEREO])
		{
			flags |= smpStereo;
		}
	} else
	{
		sampleType = typeNone;
	}

	defaultVolume = static_cast<uint8>(std::min(static_cast<uint16>(mptSmp.nVolume / 4), uint16(64)));
	if(mptSmp.nC5Speed != 0)
	{
		c5speed = mptSmp.nC5Speed;
	} else
	{
		c5speed = ModSample::TransposeToFrequency(mptSmp.RelativeTone, mptSmp.nFineTune);
	}

	return smpLength;
}


// Retrieve the internal sample format flags for this sample.
SampleIO S3MSampleHeader::GetSampleFormat(bool signedSamples) const
{
	if(pack == S3MSampleHeader::pADPCM && !(flags & S3MSampleHeader::smp16Bit) && !(flags & S3MSampleHeader::smpStereo))
	{
		// MODPlugin :(
		return SampleIO(SampleIO::_8bit, SampleIO::mono, SampleIO::littleEndian, SampleIO::ADPCM);
	} else
	{
		return SampleIO(
			(flags & S3MSampleHeader::smp16Bit) ? SampleIO::_16bit : SampleIO::_8bit,
			(flags & S3MSampleHeader::smpStereo) ?  SampleIO::stereoSplit : SampleIO::mono,
			SampleIO::littleEndian,
			signedSamples ? SampleIO::signedPCM : SampleIO::unsignedPCM);
	}
}


// Calculate the sample position in file
uint32 S3MSampleHeader::GetSampleOffset() const
{
	return (dataPointer[1] << 4) | (dataPointer[2] << 12) | (dataPointer[0] << 20);
}


OPENMPT_NAMESPACE_END
