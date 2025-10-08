/*
 * ITCompression.h
 * ---------------
 * Purpose: Code for IT sample compression and decompression.
 * Notes  : The original Python compression code was written by GreaseMonkey and has been released into the public domain.
 * Authors: OpenMPT Devs
 *          Ben "GreaseMonkey" Russell
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include <vector>
#include <iosfwd>
#include "Snd_defs.h"
#include "BitReader.h"


OPENMPT_NAMESPACE_BEGIN

struct ModSample;

class ITCompression
{
public:
	ITCompression(const ModSample &sample, bool it215, std::ostream *f, SmpLength maxLength = 0);
	size_t GetCompressedSize() const { return packedTotalLength; }

	static constexpr size_t bufferSize = 2 + 0xFFFF;  // Our output buffer can't be longer than this.
	static constexpr size_t blockSize = 0x8000;       // Block size (in bytes) in which samples are being processed

protected:
	std::vector<int8> bwt;           // Bit width table for each sampling point
	std::vector<uint8> packedData;   // Compressed data for current sample block
	std::ostream *file = nullptr;    // File to which compressed data will be written (can be nullptr if you only want to find out the sample size)
	const ModSample &mptSample;      // Sample that is being processed
	size_t packedLength = 0;         // Size of currently compressed sample block
	size_t packedTotalLength = 0;    // Size of all compressed data so far
	SmpLength baseLength = 0;        // Length of the currently compressed sample block (in samples)

	// Bit writer
	int8 bitPos = 0;    // Current bit position in this byte
	int8 remBits = 0;   // Remaining bits in this byte
	uint8 byteVal = 0;  // Current byte value to be written

	const bool is215;  // Use IT2.15 compression (double deltas)

	template<typename Properties>
	void Compress(const typename Properties::sample_t *mptSampleData, SmpLength maxLength);

	template<typename T>
	static void CopySample(T *target, const T *source, SmpLength offset, SmpLength length, SmpLength skip);

	template<typename T>
	void Deltafy(T *sampleData);

	template<typename Properties>
	void CompressBlock(const typename Properties::sample_t *data, SmpLength offset, SmpLength actualLength, typename Properties::sample_t *sampleData);

	static int8 GetWidthChangeSize(int8 w, bool is16);

	template<typename Properties>
	void SquishRecurse(int8 sWidth, int8 lWidth, int8 rWidth, int8 width, SmpLength offset, SmpLength length, const typename Properties::sample_t *sampleData);

	static int8 ConvertWidth(int8 curWidth, int8 newWidth);
	void WriteBits(int8 width, int v);

	void WriteByte(uint8 v);
};


class ITDecompression
{
public:
	ITDecompression(FileReader &file, ModSample &sample, bool it215);

protected:
	BitReader bitFile;
	ModSample &mptSample;  // Sample that is being processed

	SmpLength writtenSamples = 0;     // Number of samples so far written on this channel
	SmpLength writePos = 0;           // Absolut write position in sample (for stereo samples)
	SmpLength curLength = 0;          // Length of currently processed block
	unsigned int mem1 = 0, mem2 = 0;  // Integrator memory
	
	const bool is215;  // Use IT2.15 compression (double deltas)

	template<typename Properties>
	void Uncompress(typename Properties::sample_t *target);
	static void ChangeWidth(int &curWidth, int width);

	template<typename Properties>
	void Write(int v, int topbit, typename Properties::sample_t *target);
};


OPENMPT_NAMESPACE_END
