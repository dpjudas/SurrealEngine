/*
 * Load_unic.cpp
 * -------------
 * Purpose: UNIC Tracker v1 loader
 * Notes  : UNIC Tracker is actually a module packer, not a stand-alone tracker software.
 *          Support is mostly included to avoid such modules being recognized as regular M.K. MODs.
 *          UNIC files without file signature are not supported.
 * Authors: OpenMPT Devs
 *          Based on ProWizard by Asle
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "MODTools.h"

OPENMPT_NAMESPACE_BEGIN


static bool ValidateUNICSampleHeader(const MODSampleHeader &sampleHeader)
{
	if(CountInvalidChars(mpt::as_span(sampleHeader.name).subspan(0, 20)))
		return false;
	int16be finetune;
	memcpy(&finetune, &sampleHeader.name[20], sizeof(int16be));
	if(finetune < -42 || finetune > 8)  // African Dreams.unic has finetune = -42
		return false;
	if(sampleHeader.finetune != 0 || sampleHeader.volume > 64)
		return false;
	if(sampleHeader.length >= 0x8000 || sampleHeader.loopStart >= 0x8000 || sampleHeader.loopLength >= 0x8000)
		return false;
	if(!sampleHeader.length && (sampleHeader.loopStart > 0 || sampleHeader.loopLength > 1 || finetune != 0))
		return false;
	if(sampleHeader.length && sampleHeader.length < sampleHeader.loopStart + sampleHeader.loopLength)
		return false;
	return true;
}


static bool ValidateUNICPatternEntry(const std::array<uint8, 3> data, SAMPLEINDEX lastSample)
{
	if(data[0] > 0x74)
		return false;
	if((data[0] & 0x3F) > 0x24)
		return false;
	const uint8 command = (data[1] & 0x0F), param = data[2];
	if(command == 0x0C && param > 80)  // Mastercoma.unic has values > 64
		return false;
	if(command == 0x0B && param > 0x7F)
		return false;
	if(command == 0x0D && param > 0x40)
		return false;
	if(uint8 instr = ((data[0] >> 2) & 0x30) | ((data[1] >> 4) & 0x0F); instr > lastSample)
		return false;
	return true;
}


struct UNICFileHeader
{
	using PatternData = std::array<std::array<uint8, 3>, 64 * 4>;

	std::array<char, 20> title;
	MODSampleHeader sampleHeaders[31];
	MODFileHeader fileHeader;
	std::array<char, 4> magic;
	PatternData firstPattern;

	struct ValidationResult
	{
		uint32 totalSampleSize = 0;
		SAMPLEINDEX lastSample = 0;
		uint8 numPatterns = 0;
	};

	ValidationResult IsValid() const noexcept
	{
		if(!IsMagic(magic.data(), "M.K.") && !IsMagic(magic.data(), "UNIC") && !IsMagic(magic.data(), "\0\0\0\0"))
			return {};

		if(CountInvalidChars(title))
			return {};

		uint32 totalSampleSize = 0;
		SAMPLEINDEX lastSample = 0;
		for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
		{
			const MODSampleHeader &sampleHeader = sampleHeaders[smp - 1];
			if(!ValidateUNICSampleHeader(sampleHeader))
				return {};
			totalSampleSize += sampleHeader.length * 2;
			if(sampleHeader.length)
				lastSample = smp;
		}
		if(totalSampleSize < 256)
			return {};

		if(!fileHeader.numOrders || fileHeader.numOrders >= 128)
			return {};

		uint8 numPatterns = 0;
		for(uint8 pat = 0; pat < 128; pat++)
		{
			if(fileHeader.orderList[pat] >= 128 || (pat > fileHeader.numOrders + 1 && fileHeader.orderList[pat] != 0))
				return {};
			numPatterns = std::max(numPatterns, fileHeader.orderList[pat].get());
		}
		numPatterns++;

		for(const auto data : firstPattern)
		{
			if(!ValidateUNICPatternEntry(data, lastSample))
				return {};
		}

		return {totalSampleSize, lastSample, numPatterns};
	}

};

MPT_BINARY_STRUCT(UNICFileHeader, 1084 + 768)


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderUNIC(MemoryFileReader file, const uint64 *pfilesize)
{
	UNICFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
		return ProbeWantMoreData;

	const auto headerValidationResult = fileHeader.IsValid();
	if(!headerValidationResult.totalSampleSize)
		return ProbeFailure;

	if(pfilesize && *pfilesize < 1084 + headerValidationResult.numPatterns * 64u * 4u * 3u + headerValidationResult.totalSampleSize)
		return ProbeFailure;
	
	return ProbeSuccess;
}


bool CSoundFile::ReadUNIC(FileReader &file, ModLoadingFlags loadFlags)
{
	UNICFileHeader fileHeader;
	file.Rewind();
	if(!file.ReadStruct(fileHeader))
		return false;

	const auto headerValidationResult = fileHeader.IsValid();
	if(!headerValidationResult.totalSampleSize)
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_MOD, 4);

	// Reading patterns (done first to avoid doing unnecessary work if this is a real ProTracker M.K. file)
	file.Seek(1084);
	if(!file.CanRead(headerValidationResult.numPatterns * 64u * 4u * 3u))
		return false;
	if(loadFlags & loadPatternData)
		Patterns.ResizeArray(headerValidationResult.numPatterns);
	uint16 numNotes = 0;
	ModCommand::INSTR allInstrs = 0;
	for(PATTERNINDEX pat = 0; pat < headerValidationResult.numPatterns; pat++)
	{
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
		{
			UNICFileHeader::PatternData pattern;
			if(!file.ReadArray(pattern))
				return false;

			for(const auto data : pattern)
			{
				if(!ValidateUNICPatternEntry(data, headerValidationResult.lastSample))
					return false;
			}
			continue;
		}

		for(ModCommand &m : Patterns[pat])
		{
			const auto data = file.ReadArray<uint8, 3>();
			if(!ValidateUNICPatternEntry(data, headerValidationResult.lastSample))
				return false;

			if(data[0] & 0x3F)
			{
				m.note = NOTE_MIDDLEC - 13 + (data[0] & 0x3F);
				numNotes++;
			}
			m.instr = ((data[0] >> 2) & 0x30) | ((data[1] >> 4) & 0x0F);
			allInstrs |= m.instr;
			ConvertModCommand(m, data[1] & 0x0F, data[2]);
		}
	}
	if(numNotes < 16 || !allInstrs)
		return false;

	// Reading samples
	if(!file.CanRead(headerValidationResult.totalSampleSize))
		return false;
	m_nSamples = 31;
	for(SAMPLEINDEX smp = 1; smp <= 31; smp++)
	{
		const MODSampleHeader &sampleHeader = fileHeader.sampleHeaders[smp - 1];
		ModSample &mptSmp = Samples[smp];
		sampleHeader.ConvertToMPT(mptSmp, true);
		int16be finetune;
		memcpy(&finetune, &sampleHeader.name[20], sizeof(int16be));
		mptSmp.nFineTune = MOD2XMFineTune(-finetune);
		// Metal Jumpover.unic (and various other files) has incorrect loop starts expressed as DWORDs
		// But for the flute sample African Dreams.unic this fix doesn't seem to be quite right
		if(mptSmp.uFlags[CHN_LOOP] && mptSmp.nLoopStart > 0
		   && mptSmp.nLoopStart + mptSmp.nLoopEnd >= mptSmp.nLength - 2
		   && mptSmp.nLoopStart + mptSmp.nLoopEnd <= mptSmp.nLength)
		{
			mptSmp.nLoopEnd += mptSmp.nLoopStart;
			mptSmp.nLoopStart += mptSmp.nLoopStart;
		}

		m_szNames[smp] = mpt::String::ReadBuf(mpt::String::spacePadded, sampleHeader.name, 20);
	
		if(!(loadFlags & loadSampleData))
			continue;
		SampleIO(
			SampleIO::_8bit,
			SampleIO::mono,
			SampleIO::littleEndian,
			SampleIO::signedPCM).ReadSample(Samples[smp], file);
	}

	SetupMODPanning(true);
	Order().SetDefaultSpeed(6);
	Order().SetDefaultTempoInt(125);
	ReadOrderFromArray(Order(), fileHeader.fileHeader.orderList, headerValidationResult.numPatterns);
	m_nMinPeriod = 113 * 4;
	m_nMaxPeriod = 856 * 4;
	m_nSamplePreAmp = 64;
	m_SongFlags.set(SONG_PT_MODE | SONG_IMPORTED | SONG_FORMAT_NO_VOLCOL);
	m_playBehaviour.reset(kMODOneShotLoops);
	m_playBehaviour.set(kMODIgnorePanning);
	m_playBehaviour.set(kMODSampleSwap);  // untested

	m_songName = mpt::String::ReadBuf(mpt::String::spacePadded, fileHeader.title);

	m_modFormat.formatName = UL_("UNIC Tracker");
	m_modFormat.type = UL_("unic");
	m_modFormat.charset = mpt::Charset::Amiga_no_C1;

	return true;
}

OPENMPT_NAMESPACE_END
