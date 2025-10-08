/*
 * Load_xmf.cpp
 * ------------
 * Purpose: Module loader for music files from the DOS game "Imperium Galactica" and various Astroidea demos
 * Notes  : This format has nothing to do with the XMF format by the MIDI foundation.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Loaders.h"
#include "mpt/endian/int24.hpp"

OPENMPT_NAMESPACE_BEGIN

struct XMFSampleHeader
{
	using uint24le = mpt::uint24le;

	enum SampleFlags : uint8
	{
		smp16Bit      = 0x04,
		smpEnableLoop = 0x08,
		smpBidiLoop   = 0x10,
	};
	
	uint24le loopStart;
	uint24le loopEnd;
	uint24le dataStart;
	uint24le dataEnd;
	uint8    defaultVolume;
	uint8    flags;
	uint16le sampleRate;

	bool IsValid(uint8 type) const noexcept
	{
		if(flags & ~(smp16Bit | smpEnableLoop | smpBidiLoop))
			return false;
		if((flags & (smpEnableLoop | smpBidiLoop)) == smpBidiLoop)
			return false;
		if(dataStart.get() > dataEnd.get())
			return false;
		const uint32 length = dataEnd.get() - dataStart.get();
		if(type != 2 && length > 0 && sampleRate < 100)
			return false;
		if(type == 2 && length > 0 && sampleRate >= 0x8000)  // Any values != 0 are not really usable but when they turn negative, playback really goes haywire
			return false;
		if((flags & smp16Bit) && (length % 2u))
			return false;
		if((flags & smpEnableLoop) && !loopEnd.get())
			return false;
		if(loopStart.get() > loopEnd.get() || loopStart.get() > length)
			return false;
		if(loopEnd.get() != 0 && (loopEnd.get() >= length || loopStart.get() >= loopEnd.get()))
			return false;
		return true;
	}

	bool HasSampleData() const noexcept
	{
		return dataEnd.get() > dataStart.get();
	}

	void ConvertToMPT(ModSample &mptSmp, uint8 type) const
	{
		mptSmp.Initialize(MOD_TYPE_MOD);
		mptSmp.nLength = dataEnd.get() - dataStart.get();
		mptSmp.nLoopStart = loopStart.get() + 1u;
		mptSmp.nLoopEnd = loopEnd.get() + 1u;
		mptSmp.uFlags.set(CHN_LOOP, flags & smpEnableLoop);
		mptSmp.uFlags.set(CHN_PINGPONGLOOP, flags & smpBidiLoop);
		if(flags & smp16Bit)
		{
			mptSmp.uFlags.set(CHN_16BIT);
			mptSmp.nLength /= 2;
		}
		mptSmp.nVolume = defaultVolume;
		if(type != 2)
			mptSmp.nC5Speed = sampleRate;
		mptSmp.FrequencyToTranspose();
	}
};

MPT_BINARY_STRUCT(XMFSampleHeader, 16)


static bool TranslateXMFEffect(ModCommand &m, uint8 command, uint8 param, uint8 type)
{
	if(command == 0x0B && param < 0xFF)
	{
		param++;
	} else if(command == 0x10 || command == 0x11)
	{
		param = 0x80 | (command << 4) | (param & 0x0F);
		command = 0x0E;
	} else if(command == 0x12)
	{
		// The ULT to XMF converter uses this to translate ULT command 5 but the player does not handle this command. Thus we will simply ignore it.
		command = param = 0;
	} else if(command > 0x12)
	{
		return false;
	}
	CSoundFile::ConvertModCommand(m, command, param);
	if(type == 4 && m.command == CMD_VOLUME && (!(m.param & 0x03) || m.param == 0xFF))
		m.param = static_cast<ModCommand::PARAM>((m.param + 3u) / 4u);
	else if(m.command == CMD_VOLUME)
		m.command = CMD_VOLUME8;
	if(type != 4 && m.command == CMD_TEMPO && m.param == 0x20)
		m.command = CMD_SPEED;
	return true;
}


CSoundFile::ProbeResult CSoundFile::ProbeFileHeaderXMF(MemoryFileReader file, const uint64 *pfilesize)
{
	if(!file.CanRead(1))
		return ProbeWantMoreData;
	uint8 type = file.ReadUint8();
	if(type < 2 || type > 4)
		return ProbeFailure;
	
	constexpr size_t probeHeaders = std::min(size_t(256), (ProbeRecommendedSize - 1) / sizeof(XMFSampleHeader));
	for(size_t sample = 0; sample < probeHeaders; sample++)
	{
		XMFSampleHeader sampleHeader;
		if(!file.ReadStruct(sampleHeader))
			return ProbeWantMoreData;
		if(!sampleHeader.IsValid(type))
			return ProbeFailure;
	}

	MPT_UNREFERENCED_PARAMETER(pfilesize);
	return ProbeSuccess;
}


bool CSoundFile::ReadXMF(FileReader &file, ModLoadingFlags loadFlags)
{
	file.Rewind();
	uint8 type = file.ReadUint8();
	// Type 2: Old UltraTracker finetune behaviour, automatic tone portamento
	// Type 3: Normal finetune behaviour, automatic tone portamento (like in ULT)
	// Type 4: Normal finetune behaviour, manual tone portamento (like in MOD)
	if(type < 2 || type > 4)
		return false;
	if(!file.CanRead(256 * sizeof(XMFSampleHeader) + 256 + 3))
		return false;
	static_assert(MAX_SAMPLES > 256);
	SAMPLEINDEX numSamples = 0;
	for(SAMPLEINDEX smp = 1; smp <= 256; smp++)
	{
		XMFSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		if(!sampleHeader.IsValid(type))
			return false;
		if(sampleHeader.HasSampleData())
			numSamples = smp;
	}
	if(!numSamples)
		return false;

	file.Skip(256);
	const uint8 lastChannel = file.ReadUint8();
	if(lastChannel > 31)
		return false;
	if(loadFlags == onlyVerifyHeader)
		return true;

	InitializeGlobals(MOD_TYPE_MOD, lastChannel + 1);
	m_SongFlags.set(SONG_IMPORTED);
	m_SongFlags.reset(SONG_ISAMIGA);
	m_SongFlags.set(SONG_AUTO_TONEPORTA | SONG_AUTO_TONEPORTA_CONT, type < 4);
	m_nSamples = numSamples;
	m_nSamplePreAmp = (type == 3) ? 192 : 48;  // Imperium Galactica files are really quiet, no other XMFs appear to use type 3

	file.Seek(1);
	for(SAMPLEINDEX smp = 1; smp <= numSamples; smp++)
	{
		XMFSampleHeader sampleHeader;
		file.ReadStruct(sampleHeader);
		sampleHeader.ConvertToMPT(Samples[smp], type);
		m_szNames[smp] = "";
	}

	file.Seek(1 + 256 * sizeof(XMFSampleHeader));
	ReadOrderFromFile<uint8>(Order(), file, 256, 0xFF);

	file.Skip(1);  // Channel count already read
	const PATTERNINDEX numPatterns  = file.ReadUint8() + 1u;
	if(!file.CanRead(GetNumChannels() + numPatterns * GetNumChannels() * 64 * 6))
		return false;

	for(CHANNELINDEX chn = 0; chn < GetNumChannels(); chn++)
	{
		ChnSettings[chn].nPan = file.ReadUint8() * 0x11;
	}

	Patterns.ResizeArray(numPatterns);
	for(PATTERNINDEX pat = 0; pat < numPatterns; pat++)
	{
		if(!(loadFlags & loadPatternData) || !Patterns.Insert(pat, 64))
		{
			file.Skip(GetNumChannels() * 64 * 6);
			continue;
		}
		ModCommand dummy;
		for(ModCommand &m : Patterns[pat])
		{
			const auto data = file.ReadArray<uint8, 6>();
			if(data[0] > 0 && data[0] <= 77)
				m.note = NOTE_MIN + 35 + data[0];
			m.instr = data[1];
			if(!TranslateXMFEffect(m, data[2], data[5], type) || !TranslateXMFEffect(dummy, data[3], data[4], type))
				return false;
			if(dummy.command != CMD_NONE)
				m.FillInTwoCommands(m.command, m.param, dummy.command, dummy.param);
		}
	}

	if(loadFlags & loadSampleData)
	{
		for(SAMPLEINDEX smp = 1; smp <= numSamples; smp++)
		{
			SampleIO(Samples[smp].uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
				SampleIO::mono,
				SampleIO::littleEndian,
				SampleIO::signedPCM).ReadSample(Samples[smp], file);
		}
	}

	m_modFormat.formatName = UL_("Astroidea XMF");
	m_modFormat.type = UL_("xmf");
	m_modFormat.madeWithTracker.clear();
	m_modFormat.charset = mpt::Charset::CP437;  // No strings in this format...

	return true;
}


OPENMPT_NAMESPACE_END
