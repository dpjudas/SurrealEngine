/*
 * SampleFormatBRR.cpp
 * -------------------
 * Purpose: BRR (SNES Bit Rate Reduction) sample format import.
 * Notes  : This format has no magic bytes, so frame headers are thoroughly validated.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Sndfile.h"
#include "../common/FileReader.h"


OPENMPT_NAMESPACE_BEGIN


static void ProcessBRRSample(int32 sample, int16 *output, uint8 range, uint8 filter)
{
	if(sample >= 8)
		sample -= 16;

	if(range <= 12)
		sample = mpt::rshift_signed(mpt::lshift_signed(sample, range), 1);
	else
		sample = (sample < 0) ? -2048 : 0;  // Implementations do not fully agree on what to do in this case. This is what bsnes does.

	// Apply prediction filter
	// Note 1: It is okay that we may access data before the first sample point because this memory is reserved for interpolation
	// Note 2: The use of signed shift arithmetic is crucial for some samples (e.g. killer lead.brr, Mac2.brr)
	// Note 3: Divisors are twice of what is written in the respective comments, as all sample data is divided by 2 (again crucial for accuracy)
	static_assert(InterpolationLookaheadBufferSize >= 2);
	switch(filter)
	{
	case 1:  // y(n) = x(n) + x(n-1) * 15/16
		sample += mpt::rshift_signed(output[-1] * 15, 5);
		break;
	case 2:  // y(n) = x(n) + x(n-1) * 61/32 - x(n-2) * 15/16
		sample += mpt::rshift_signed(output[-1] * 61, 6) + mpt::rshift_signed(output[-2] * -15, 5);
		break;
	case 3:  // y(n) = x(n) + x(n-1) * 115/64 - x(n-2) * 13/16
		sample += mpt::rshift_signed(output[-1] * 115, 7) + mpt::rshift_signed(output[-2] * -13, 5);
		break;
	}

	sample = std::clamp(sample, int32(-32768), int32(32767)) * 2;
	if(sample > 32767)
		sample -= 65536;
	else if(sample < -32768)
		sample += 65536;
	output[0] = static_cast<int16>(sample);
}


bool CSoundFile::ReadBRRSample(SAMPLEINDEX sample, FileReader &file)
{
	if(!file.LengthIsAtLeast(9) || file.LengthIsAtLeast(65536))
		return false;
	const auto fileSize = file.GetLength();
	const bool hasLoopInfo = (fileSize % 9) == 2;
	if((fileSize % 9) != 0 && !hasLoopInfo)
		return false;
	
	file.Rewind();

	SmpLength loopStart = 0;
	if(hasLoopInfo)
	{
		loopStart = file.ReadUint16LE();
		if(loopStart >= fileSize)
			return false;
		if((loopStart % 9) != 0)
			return false;
	}

	// First scan the file for validity and consistency
	// Note: There are some files with loop start set but ultimately the loop is never enabled. Cannot use this as a consistency check.
	// Very few files also have a filter set on the first block, so we cannot reject those either.
	bool enableLoop = false, first = true;
	while(!file.EndOfFile())
	{
		const auto block = file.ReadArray<uint8, 9>();
		const bool isLast = (block[0] & 0x01) != 0;
		const bool isLoop = (block[0] & 0x02) != 0;
		const uint8 range = block[0] >> 4u;
		if(isLast != file.EndOfFile())
			return false;
		if(!first && enableLoop != isLoop)
		{
			if(!hasLoopInfo)
				return false;
			// In some files, the loop flag is only set for the blocks within the loop (except for the first block?)
			const bool inLoop = file.GetPosition() > loopStart + 11u;
			if(enableLoop != inLoop)
				return false;
		}
		// While a range of 13 is technically invalid as well, it can be found in the wild.
		if(range > 13)
			return false;
		enableLoop = isLoop;
		first = false;
	}

	file.Seek(hasLoopInfo ? 2 : 0);

	DestroySampleThreadsafe(sample);
	ModSample &mptSmp = Samples[sample];
	mptSmp.Initialize();
	mptSmp.uFlags = CHN_16BIT;
	mptSmp.nLength = mpt::saturate_cast<SmpLength>((fileSize - (hasLoopInfo ? 2 : 0)) * 16 / 9);
	if(enableLoop)
		mptSmp.SetLoop(loopStart * 16 / 9, mptSmp.nLength, true, false, *this);
	mptSmp.nC5Speed = 32000;
	m_szNames[sample] = "";

	if(!mptSmp.AllocateSample())
		return false;

	int16 *output = mptSmp.sample16();
	while(!file.EndOfFile())
	{
		const auto block = file.ReadArray<uint8, 9>();
		const uint8 range = block[0] >> 4u;
		const uint8 filter = (block[0] >> 2) & 0x03;

		for(int i = 0; i < 8; i++)
		{
			ProcessBRRSample(block[i + 1] >> 4u, output, range, filter);
			ProcessBRRSample(block[i + 1] & 0x0F, output + 1, range, filter);
			output += 2;
		}
	}
	mptSmp.Convert(MOD_TYPE_IT, GetType());
	mptSmp.PrecomputeLoops(*this, false);

	return true;
}


OPENMPT_NAMESPACE_END
