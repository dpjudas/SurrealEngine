/*
 * MODTools.h
 * ----------
 * Purpose: Definition of MOD file structures (shared between several SoundTracker-/ProTracker-like formats) and helper functions
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#pragma once

#include "openmpt/all/BuildSettings.hpp"
#include "openmpt/base/Endian.hpp"
#include "SampleIO.h"
#include "Snd_defs.h"

OPENMPT_NAMESPACE_BEGIN

class CSoundFile;
struct ModSample;

// File header following the sample headers
struct MODFileHeader
{
	uint8be numOrders;
	uint8be restartPos;  // Tempo (early SoundTracker) or restart position (only PC trackers?)
	uint8be orderList[128];
};

MPT_BINARY_STRUCT(MODFileHeader, 130)


// Sample Header
struct MODSampleHeader
{
	char     name[22];
	uint16be length;
	uint8be  finetune;
	uint8be  volume;
	uint16be loopStart;
	uint16be loopLength;

	// Convert an MOD sample header to OpenMPT's internal sample header.
	void ConvertToMPT(ModSample &mptSmp, bool is4Chn) const;
	
	// Convert OpenMPT's internal sample header to a MOD sample header.
	SmpLength ConvertToMOD(const ModSample &mptSmp);
	
	// Compute a "rating" of this sample header by counting invalid header data to ultimately reject garbage files.
	uint32 GetInvalidByteScore() const;

	bool HasDiskName() const;
	
	// Suggested threshold for rejecting invalid files based on cumulated score returned by GetInvalidByteScore
	static constexpr uint32 INVALID_BYTE_THRESHOLD = 40;

	// This threshold is used for files where the file magic only gives a
	// fragile result which alone would lead to too many false positives.
	// In particular, the files from Inconexia demo by Iguana
	// (https://www.pouet.net/prod.php?which=830) which have 3 \0 bytes in
	// the file magic tend to cause misdetection of random files.
	static constexpr uint32 INVALID_BYTE_FRAGILE_THRESHOLD = 1;

	// Retrieve the internal sample format flags for this sample.
	static SampleIO GetSampleFormat()
	{
		return SampleIO(
			SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::bigEndian,
			SampleIO::signedPCM);
	}
};

MPT_BINARY_STRUCT(MODSampleHeader, 30)

// Pattern data of a 4-channel MOD file
using MODPatternData = std::array<std::array<std::array<uint8, 4>, 4>, 64>;

// Check if header magic equals a given string.
inline bool IsMagic(const char *magic1, const char (&magic2)[5]) noexcept
{
	return std::memcmp(magic1, magic2, 4) == 0;
}


// For .DTM files from Apocalypse Abyss, where the first 2108 bytes are swapped
template<typename T, typename TFileReader>
inline T ReadAndSwap(TFileReader &file, const bool swapBytes)
{
	T value{};
	if(file.Read(value) && swapBytes)
	{
		static_assert(sizeof(value) % 2u == 0);
		auto byteView = mpt::as_raw_memory(value);
		for(size_t i = 0; i < sizeof(T); i += 2)
		{
			std::swap(byteView[i], byteView[i + 1]);
		}
	}
	return value;
}


// Convert MOD sample header and validate
uint32 ReadMODSample(const MODSampleHeader &sampleHeader, ModSample &sample, mpt::charbuf<MAX_SAMPLENAME> &sampleName, bool is4Chn);


// Check if a name string is valid (i.e. doesn't contain binary garbage data)
uint32 CountInvalidChars(const mpt::span<const char> name) noexcept;


enum class NameClassification
{
	Empty,
	ValidASCII,
	Invalid,
};

// Check if a name is a valid null-terminated ASCII string with no garbage after the null terminator, or if it's empty
NameClassification ClassifyName(const mpt::span<const char> name) noexcept;


// Count malformed bytes in MOD pattern data
uint32 CountMalformedMODPatternData(const MODPatternData &patternData, const bool extendedFormat);


// Check if number of malformed bytes in MOD pattern data exceeds some threshold
template <typename TFileReader>
inline bool ValidateMODPatternData(TFileReader &file, const uint32 threshold, const bool extendedFormat)
{
	MODPatternData patternData;
	if(!file.Read(patternData))
		return false;
	return CountMalformedMODPatternData(patternData, extendedFormat) <= threshold;
}


// Parse the order list to determine how many patterns are used in the file.
PATTERNINDEX GetNumPatterns(FileReader &file, CSoundFile &sndFile, ORDERINDEX numOrders, SmpLength totalSampleLen, SmpLength wowSampleLen, bool validateHiddenPatterns);


OPENMPT_NAMESPACE_END
