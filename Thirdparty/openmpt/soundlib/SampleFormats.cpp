/*
 * SampleFormats.cpp
 * -----------------
 * Purpose: Code for loading various more or less common sample and instrument formats.
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "ITTools.h"
#include "Loaders.h"
#include "mod_specifications.h"
#include "S3MTools.h"
#include "Sndfile.h"
#include "Tagging.h"
#include "tuningcollection.h"
#include "WAVTools.h"
#include "XMTools.h"
#include "../common/FileReader.h"
#include "../common/misc_util.h"
#include "../common/version.h"
#include "../soundlib/AudioCriticalSection.h"
#include "../soundlib/ModSampleCopy.h"
#include "mpt/format/join.hpp"
#include "mpt/string/utility.hpp"
#include "openmpt/base/Endian.hpp"

#ifdef MODPLUG_TRACKER
#include "../mptrack/Moddoc.h"
#include "Dlsbank.h"
#endif // MODPLUG_TRACKER

#ifndef MODPLUG_NO_FILESAVE
#include "mpt/io/base.hpp"
#include "mpt/io/io.hpp"
#include "mpt/io/io_stdstream.hpp"
#include "../common/mptFileIO.h"
#endif // !MODPLUG_NO_FILESAVE

#include <functional>
#include <map>


OPENMPT_NAMESPACE_BEGIN


using namespace mpt::uuid_literals;


bool CSoundFile::ReadSampleFromFile(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize, bool includeInstrumentFormats)
{
	if(!nSample || nSample >= MAX_SAMPLES) return false;
	if(!ReadWAVSample(nSample, file, mayNormalize)
		&& !(includeInstrumentFormats && ReadXISample(nSample, file))
		&& !(includeInstrumentFormats && ReadITISample(nSample, file))
		&& !ReadW64Sample(nSample, file)
		&& !ReadCAFSample(nSample, file)
		&& !ReadAIFFSample(nSample, file, mayNormalize)
		&& !ReadITSSample(nSample, file)
		&& !(includeInstrumentFormats && ReadPATSample(nSample, file))
		&& !ReadIFFSample(nSample, file)
		&& !ReadS3ISample(nSample, file)
		&& !ReadSBISample(nSample, file)
		&& !ReadAUSample(nSample, file, mayNormalize)
		&& !ReadBRRSample(nSample, file)
		&& !ReadFLACSample(nSample, file)
		&& !ReadOpusSample(nSample, file)
		&& !ReadVorbisSample(nSample, file)
		&& !ReadMP3Sample(nSample, file, false)
		&& !ReadMediaFoundationSample(nSample, file)
		)
	{
		return false;
	}

	if(nSample > GetNumSamples())
	{
		m_nSamples = nSample;
	}
	if(Samples[nSample].uFlags[CHN_ADLIB])
	{
		InitOPL();
	}
	return true;
}


bool CSoundFile::ReadInstrumentFromFile(INSTRUMENTINDEX nInstr, FileReader &file, bool mayNormalize)
{
	if ((!nInstr) || (nInstr >= MAX_INSTRUMENTS)) return false;
	if(!ReadITIInstrument(nInstr, file)
		&& !ReadXIInstrument(nInstr, file)
		&& !ReadPATInstrument(nInstr, file)
		&& !ReadSFZInstrument(nInstr, file)
		// Generic read
		&& !ReadSampleAsInstrument(nInstr, file, mayNormalize))
	{
		bool ok = false;
#ifdef MODPLUG_TRACKER
		CDLSBank bank;
		if(bank.Open(file))
		{
			ok = bank.ExtractInstrument(*this, nInstr, 0, 0);
		}
#endif // MODPLUG_TRACKER
		if(!ok) return false;
	}

	if(nInstr > GetNumInstruments()) m_nInstruments = nInstr;
	return true;
}


bool CSoundFile::ReadSampleAsInstrument(INSTRUMENTINDEX nInstr, FileReader &file, bool mayNormalize)
{
	// Scanning free sample
	SAMPLEINDEX nSample = GetNextFreeSample(nInstr); // may also return samples which are only referenced by the current instrument
	if(nSample == SAMPLEINDEX_INVALID)
	{
		return false;
	}

	// Loading Instrument
	ModInstrument *pIns = new (std::nothrow) ModInstrument(nSample);
	if(pIns == nullptr)
	{
		return false;
	}
	if(!ReadSampleFromFile(nSample, file, mayNormalize, false))
	{
		delete pIns;
		return false;
	}

	// Remove all samples which are only referenced by the old instrument, except for the one we just loaded our new sample into.
	RemoveInstrumentSamples(nInstr, nSample);

	// Replace the instrument
	DestroyInstrument(nInstr, doNoDeleteAssociatedSamples);
	Instruments[nInstr] = pIns;

#if defined(MPT_EXTERNAL_SAMPLES)
	SetSamplePath(nSample, file.GetOptionalFileName().value_or(P_("")));
#endif

	return true;
}


bool CSoundFile::DestroyInstrument(INSTRUMENTINDEX nInstr, deleteInstrumentSamples removeSamples)
{
	if(nInstr == 0 || nInstr >= MAX_INSTRUMENTS || !Instruments[nInstr]) return true;

	if(removeSamples == deleteAssociatedSamples)
	{
		RemoveInstrumentSamples(nInstr);
	}

	CriticalSection cs;

	ModInstrument *pIns = Instruments[nInstr];
	Instruments[nInstr] = nullptr;
	for(auto &chn : m_PlayState.Chn)
	{
		if(chn.pModInstrument == pIns)
			chn.pModInstrument = nullptr;
	}
	delete pIns;
	return true;
}


// Remove all unused samples from the given nInstr and keep keepSample if provided
bool CSoundFile::RemoveInstrumentSamples(INSTRUMENTINDEX nInstr, SAMPLEINDEX keepSample)
{
	if(Instruments[nInstr] == nullptr)
	{
		return false;
	}

	std::vector<bool> keepSamples(GetNumSamples() + 1, true);

	// Check which samples are used by the instrument we are going to nuke.
	auto referencedSamples = Instruments[nInstr]->GetSamples();
	for(auto sample : referencedSamples)
	{
		if(sample <= GetNumSamples())
		{
			keepSamples[sample] = false;
		}
	}

	// If we want to keep a specific sample, do so.
	if(keepSample != SAMPLEINDEX_INVALID)
	{
		if(keepSample <= GetNumSamples())
		{
			keepSamples[keepSample] = true;
		}
	}

	// Check if any of those samples are referenced by other instruments as well, in which case we want to keep them of course.
	for(INSTRUMENTINDEX nIns = 1; nIns <= GetNumInstruments(); nIns++) if (Instruments[nIns] != nullptr && nIns != nInstr)
	{
		Instruments[nIns]->GetSamples(keepSamples);
	}

	// Now nuke the selected samples.
	RemoveSelectedSamples(keepSamples);
	return true;
}

////////////////////////////////////////////////////////////////////////////////
//
// I/O From another song
//

bool CSoundFile::ReadInstrumentFromSong(INSTRUMENTINDEX targetInstr, const CSoundFile &srcSong, INSTRUMENTINDEX sourceInstr)
{
	if ((!sourceInstr) || (sourceInstr > srcSong.GetNumInstruments())
		|| (targetInstr >= MAX_INSTRUMENTS) || (!srcSong.Instruments[sourceInstr]))
	{
		return false;
	}
	if (m_nInstruments < targetInstr) m_nInstruments = targetInstr;

	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
	{
		return false;
	}

	DestroyInstrument(targetInstr, deleteAssociatedSamples);

	Instruments[targetInstr] = pIns;
	*pIns = *srcSong.Instruments[sourceInstr];

	std::vector<SAMPLEINDEX> sourceSample;  // Sample index in source song
	std::vector<SAMPLEINDEX> targetSample;  // Sample index in target song
	SAMPLEINDEX targetIndex = 0;            // Next index for inserting sample

	for(auto &sample : pIns->Keyboard)
	{
		const SAMPLEINDEX sourceIndex = sample;
		if(sourceIndex > 0 && sourceIndex <= srcSong.GetNumSamples())
		{
			const auto entry = std::find(sourceSample.cbegin(), sourceSample.cend(), sourceIndex);
			if(entry == sourceSample.end())
			{
				// Didn't consider this sample yet, so add it to our map.
				targetIndex = GetNextFreeSample(targetInstr, targetIndex + 1);
				if(targetIndex <= GetModSpecifications().samplesMax)
				{
					sourceSample.push_back(sourceIndex);
					targetSample.push_back(targetIndex);
					sample = targetIndex;
				} else
				{
					sample = 0;
				}
			} else
			{
				// Sample reference has already been created, so only need to update the sample map.
				sample = *(entry - sourceSample.begin() + targetSample.begin());
			}
		} else
		{
			// Invalid or no source sample
			sample = 0;
		}
	}

#ifdef MODPLUG_TRACKER
	if(pIns->filename.empty() && srcSong.GetpModDoc() != nullptr && &srcSong != this)
	{
		pIns->filename = srcSong.GetpModDoc()->GetPathNameMpt().GetFilename().ToLocale();
	}
#endif
	pIns->Convert(srcSong.GetType(), GetType());

	if(pIns->pTuning && this != &srcSong)
	{
		CTuning *existingTuning = m_pTuningsTuneSpecific->FindIdenticalTuning(*pIns->pTuning);
		if(existingTuning)
			pIns->pTuning = existingTuning;
		else
			pIns->pTuning = m_pTuningsTuneSpecific->AddTuning(std::make_unique<CTuning>(*pIns->pTuning));
	}

	// Copy all referenced samples over
	for(size_t i = 0; i < targetSample.size(); i++)
	{
		ReadSampleFromSong(targetSample[i], srcSong, sourceSample[i]);
	}

	return true;
}


bool CSoundFile::ReadSampleFromSong(SAMPLEINDEX targetSample, const CSoundFile &srcSong, SAMPLEINDEX sourceSample)
{
	if(!sourceSample
		|| sourceSample > srcSong.GetNumSamples()
		|| (targetSample >= GetModSpecifications().samplesMax && targetSample > GetNumSamples()))
	{
		return false;
	}

	if(GetNumSamples() < targetSample)
		m_nSamples = targetSample;

	DestroySampleThreadsafe(targetSample);

	const ModSample &sourceSmp = srcSong.GetSample(sourceSample);
	ModSample &targetSmp = GetSample(targetSample);

	targetSmp = sourceSmp;
	m_szNames[targetSample] = srcSong.m_szNames[sourceSample];

	if(sourceSmp.HasSampleData())
	{
		if(targetSmp.CopyWaveform(sourceSmp))
			targetSmp.PrecomputeLoops(*this, false);
		// Remember on-disk path (for MPTM files), but don't implicitely enable on-disk storage
		// (we really don't want this for e.g. duplicating samples or splitting stereo samples)
#ifdef MPT_EXTERNAL_SAMPLES
		SetSamplePath(targetSample, srcSong.GetSamplePath(sourceSample));
#endif
		targetSmp.uFlags.reset(SMP_KEEPONDISK);
	}

#ifdef MODPLUG_TRACKER
	if((targetSmp.filename.empty()) && srcSong.GetpModDoc() != nullptr && &srcSong != this)
	{
		targetSmp.filename = mpt::ToCharset(GetCharsetInternal(), srcSong.GetpModDoc()->GetTitle());
	}
#endif

	if(targetSmp.uFlags[CHN_ADLIB] && !SupportsOPL())
	{
		AddToLog(LogInformation, U_("OPL instruments are not supported by this format."));
	}
	targetSmp.Convert(srcSong.GetType(), GetType());
	if(targetSmp.uFlags[CHN_ADLIB])
	{
		InitOPL();
	}
	return true;
}


////////////////////////////////////////////////////////////////////////
// IMA ADPCM Support for WAV files


static bool IMAADPCMUnpack16(int16 *target, SmpLength sampleLen, FileReader file, uint16 blockAlign, uint32 numChannels)
{
	static constexpr int8 IMAIndexTab[8] =  { -1, -1, -1, -1, 2, 4, 6, 8 };
	static constexpr int16 IMAUnpackTable[90] =
	{
		7,     8,     9,     10,    11,    12,    13,    14,
		16,    17,    19,    21,    23,    25,    28,    31,
		34,    37,    41,    45,    50,    55,    60,    66,
		73,    80,    88,    97,    107,   118,   130,   143,
		157,   173,   190,   209,   230,   253,   279,   307,
		337,   371,   408,   449,   494,   544,   598,   658,
		724,   796,   876,   963,   1060,  1166,  1282,  1411,
		1552,  1707,  1878,  2066,  2272,  2499,  2749,  3024,
		3327,  3660,  4026,  4428,  4871,  5358,  5894,  6484,
		7132,  7845,  8630,  9493,  10442, 11487, 12635, 13899,
		15289, 16818, 18500, 20350, 22385, 24623, 27086, 29794,
		32767, 0
	};

	if(target == nullptr || blockAlign < 4u * numChannels)
		return false;

	SmpLength samplePos = 0;
	sampleLen *= numChannels;
	while(file.CanRead(4u * numChannels) && samplePos < sampleLen)
	{
		FileReader block = file.ReadChunk(blockAlign);
		FileReader::PinnedView blockView = block.GetPinnedView();
		const std::byte *data = blockView.data();
		const uint32 blockSize = static_cast<uint32>(blockView.size());

		for(uint32 chn = 0; chn < numChannels; chn++)
		{
			// Block header
			int32 value = block.ReadInt16LE();
			int32 nIndex = block.ReadUint8();
			Limit(nIndex, 0, 89);
			block.Skip(1);

			SmpLength smpPos = samplePos + chn;
			uint32 dataPos = (numChannels + chn) * 4;
			// Block data
			while(smpPos <= (sampleLen - 8) && dataPos <= (blockSize - 4))
			{
				for(uint32 i = 0; i < 8; i++)
				{
					uint8 delta = mpt::byte_cast<uint8>(data[dataPos]);
					if(i & 1)
					{
						delta >>= 4;
						dataPos++;
					} else
					{
						delta &= 0x0F;
					}
					int32 v = IMAUnpackTable[nIndex] >> 3;
					if (delta & 1) v += IMAUnpackTable[nIndex] >> 2;
					if (delta & 2) v += IMAUnpackTable[nIndex] >> 1;
					if (delta & 4) v += IMAUnpackTable[nIndex];
					if (delta & 8) value -= v; else value += v;
					nIndex += IMAIndexTab[delta & 7];
					Limit(nIndex, 0, 88);
					Limit(value, -32768, 32767);
					target[smpPos] = static_cast<int16>(value);
					smpPos += numChannels;
				}
				dataPos += (numChannels - 1) * 4u;
			}
		}
		samplePos += ((blockSize - (numChannels * 4u)) * 2u);
	}

	return true;
}


////////////////////////////////////////////////////////////////////////////////
// WAV Open

bool CSoundFile::ReadWAVSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize, FileReader *wsmpChunk)
{
	WAVReader wavFile(file);

	static constexpr WAVFormatChunk::SampleFormats SupportedFormats[] = {WAVFormatChunk::fmtPCM, WAVFormatChunk::fmtFloat, WAVFormatChunk::fmtIMA_ADPCM, WAVFormatChunk::fmtMP3, WAVFormatChunk::fmtALaw, WAVFormatChunk::fmtULaw};
	if(!wavFile.IsValid()
	   || wavFile.GetNumChannels() == 0
	   || wavFile.GetNumChannels() > 2
	   || (wavFile.GetBitsPerSample() == 0 && wavFile.GetSampleFormat() != WAVFormatChunk::fmtMP3)
	   || (wavFile.GetBitsPerSample() < 32 && wavFile.GetSampleFormat() == WAVFormatChunk::fmtFloat)
	   || (wavFile.GetBitsPerSample() > 64)
	   || !mpt::contains(SupportedFormats, wavFile.GetSampleFormat()))
	{
		return false;
	}

	DestroySampleThreadsafe(nSample);
	m_szNames[nSample] = "";
	ModSample &sample = Samples[nSample];
	sample.Initialize();
	sample.nLength = wavFile.GetSampleLength();
	sample.nC5Speed = wavFile.GetSampleRate();
	wavFile.ApplySampleSettings(sample, GetCharsetInternal(), m_szNames[nSample]);

	FileReader sampleChunk = wavFile.GetSampleData();

	SampleIO sampleIO(
		SampleIO::_8bit,
		(wavFile.GetNumChannels() > 1) ? SampleIO::stereoInterleaved : SampleIO::mono,
		SampleIO::littleEndian,
		SampleIO::signedPCM);

	if(wavFile.GetSampleFormat() == WAVFormatChunk::fmtIMA_ADPCM && wavFile.GetNumChannels() <= 2)
	{
		// IMA ADPCM 4:1
		LimitMax(sample.nLength, MAX_SAMPLE_LENGTH);
		sample.uFlags.set(CHN_16BIT);
		sample.uFlags.set(CHN_STEREO, wavFile.GetNumChannels() == 2);
		if(!sample.AllocateSample())
		{
			return false;
		}
		IMAADPCMUnpack16(sample.sample16(), sample.nLength, sampleChunk, wavFile.GetBlockAlign(), wavFile.GetNumChannels());
		sample.PrecomputeLoops(*this, false);
	} else if(wavFile.GetSampleFormat() == WAVFormatChunk::fmtMP3)
	{
		// MP3 in WAV
		bool loadedMP3 = ReadMP3Sample(nSample, sampleChunk, false, true) || ReadMediaFoundationSample(nSample, sampleChunk, true);
		if(!loadedMP3)
		{
			return false;
		}
	} else if(!wavFile.IsExtensibleFormat() && wavFile.MayBeCoolEdit16_8() && wavFile.GetSampleFormat() == WAVFormatChunk::fmtPCM && wavFile.GetBitsPerSample() == 32 && wavFile.GetBlockAlign() == wavFile.GetNumChannels() * 4)
	{
		// Syntrillium Cool Edit hack to store IEEE 32bit floating point
		// Format is described as 32bit integer PCM contained in 32bit blocks and an WAVEFORMATEX extension size of 2 which contains a single 16 bit little endian value of 1.
		//  (This is parsed in WAVTools.cpp and returned via MayBeCoolEdit16_8()).
		// The data actually stored in this case is little endian 32bit floating point PCM with 2**15 full scale.
		// Cool Edit calls this format "16.8 float".
		sampleIO |= SampleIO::_32bit;
		sampleIO |= SampleIO::floatPCM15;
		sampleIO.ReadSample(sample, sampleChunk);
	} else if(!wavFile.IsExtensibleFormat() && wavFile.GetSampleFormat() == WAVFormatChunk::fmtPCM && wavFile.GetBitsPerSample() == 24 && wavFile.GetBlockAlign() == wavFile.GetNumChannels() * 4)
	{
		// Syntrillium Cool Edit hack to store IEEE 32bit floating point
		// Format is described as 24bit integer PCM contained in 32bit blocks.
		// The data actually stored in this case is little endian 32bit floating point PCM with 2**23 full scale.
		// Cool Edit calls this format "24.0 float".
		sampleIO |= SampleIO::_32bit;
		sampleIO |= SampleIO::floatPCM23;
		sampleIO.ReadSample(sample, sampleChunk);
	} else if(wavFile.GetSampleFormat() == WAVFormatChunk::fmtALaw || wavFile.GetSampleFormat() == WAVFormatChunk::fmtULaw)
	{
		// a-law / u-law
		sampleIO |= SampleIO::_16bit;
		sampleIO |= wavFile.GetSampleFormat() == WAVFormatChunk::fmtALaw ? SampleIO::aLaw : SampleIO::uLaw;
		sampleIO.ReadSample(sample, sampleChunk);
	} else
	{
		// PCM / Float
		SampleIO::Bitdepth bitDepth;
		switch((wavFile.GetBitsPerSample() - 1) / 8u)
		{
		default:
		case 0: bitDepth = SampleIO::_8bit; break;
		case 1: bitDepth = SampleIO::_16bit; break;
		case 2: bitDepth = SampleIO::_24bit; break;
		case 3: bitDepth = SampleIO::_32bit; break;
		case 7: bitDepth = SampleIO::_64bit; break;
		}

		sampleIO |= bitDepth;
		if(wavFile.GetBitsPerSample() <= 8)
			sampleIO |= SampleIO::unsignedPCM;

		if(wavFile.GetSampleFormat() == WAVFormatChunk::fmtFloat)
			sampleIO |= SampleIO::floatPCM;

		if(mayNormalize)
			sampleIO.MayNormalize();

		sampleIO.ReadSample(sample, sampleChunk);
	}

	if(wsmpChunk != nullptr)
	{
		// DLS WSMP chunk
		*wsmpChunk = wavFile.GetWsmpChunk();
	}

	sample.Convert(MOD_TYPE_IT, GetType());
	sample.PrecomputeLoops(*this, false);

	return true;
}


///////////////////////////////////////////////////////////////
// Save WAV


#ifndef MODPLUG_NO_FILESAVE
bool CSoundFile::SaveWAVSample(SAMPLEINDEX nSample, std::ostream &f) const
{
	const ModSample &sample = Samples[nSample];
	if(sample.uFlags[CHN_ADLIB] || !sample.HasSampleData())
		return false;

	mpt::IO::OFile<std::ostream> ff(f);
	WAVSampleWriter file(ff);

	file.WriteFormat(sample.GetSampleRate(GetType()), sample.GetElementarySampleSize() * 8, sample.GetNumChannels(), WAVFormatChunk::fmtPCM);

	// Write sample data
	file.StartChunk(RIFFChunk::iddata);
	SampleIO(
		sample.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
		sample.uFlags[CHN_STEREO] ? SampleIO::stereoInterleaved : SampleIO::mono,
		SampleIO::littleEndian,
		sample.uFlags[CHN_16BIT] ? SampleIO::signedPCM : SampleIO::unsignedPCM)
		.WriteSample(f, sample);

	file.WriteLoopInformation(sample);
	file.WriteExtraInformation(sample, GetType());
	if(sample.HasCustomCuePoints())
	{
		file.WriteCueInformation(sample);
	}

	FileTags tags;
	tags.encoder = Version::Current().GetOpenMPTVersionString();
	tags.title = mpt::ToUnicode(GetCharsetInternal(), m_szNames[nSample]);
	file.WriteMetatags(tags);
	file.Finalize();

	return true;
}

#endif // MODPLUG_NO_FILESAVE



/////////////////
// Sony Wave64 //


struct Wave64FileHeader
{
	mpt::GUIDms GuidRIFF;
	uint64le    FileSize;
	mpt::GUIDms GuidWAVE;
};

MPT_BINARY_STRUCT(Wave64FileHeader, 40)


struct Wave64ChunkHeader
{
	mpt::GUIDms GuidChunk;
	uint64le    Size;
};

MPT_BINARY_STRUCT(Wave64ChunkHeader, 24)


struct Wave64Chunk
{
	Wave64ChunkHeader header;

	FileReader::pos_type GetLength() const
	{
		uint64 length = header.Size;
		if(length < sizeof(Wave64ChunkHeader))
		{
			length = 0;
		} else
		{
			length -= sizeof(Wave64ChunkHeader);
		}
		return mpt::saturate_cast<FileReader::pos_type>(length);
	}

	mpt::UUID GetID() const
	{
		return mpt::UUID(header.GuidChunk);
	}
};

MPT_BINARY_STRUCT(Wave64Chunk, 24)


static void Wave64TagFromLISTINFO(mpt::ustring & dst, uint16 codePage, const FileReader::ChunkList<RIFFChunk> & infoChunk, RIFFChunk::ChunkIdentifiers id)
{
	if(!infoChunk.ChunkExists(id))
	{
		return;
	}
	FileReader textChunk = infoChunk.GetChunk(id);
	if(!textChunk.IsValid())
	{
		return;
	}
	std::string str;
	textChunk.ReadString<mpt::String::maybeNullTerminated>(str, mpt::saturate_cast<std::size_t>(textChunk.GetLength()));
	str = mpt::replace(str, std::string("\r\n"), std::string("\n"));
	str = mpt::replace(str, std::string("\r"), std::string("\n"));
	dst = mpt::ToUnicode(codePage, mpt::Charset::Windows1252, str);
}


bool CSoundFile::ReadW64Sample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize)
{
	file.Rewind();

	constexpr mpt::UUID guidRIFF       = "66666972-912E-11CF-A5D6-28DB04C10000"_uuid;
	constexpr mpt::UUID guidWAVE       = "65766177-ACF3-11D3-8CD1-00C04F8EDB8A"_uuid;

	constexpr mpt::UUID guidLIST       = "7473696C-912F-11CF-A5D6-28DB04C10000"_uuid;
	constexpr mpt::UUID guidFMT        = "20746D66-ACF3-11D3-8CD1-00C04F8EDB8A"_uuid;
	//constexpr mpt::UUID guidFACT       = "74636166-ACF3-11D3-8CD1-00C04F8EDB8A"_uuid;
	constexpr mpt::UUID guidDATA       = "61746164-ACF3-11D3-8CD1-00C04F8EDB8A"_uuid;
	//constexpr mpt::UUID guidLEVL       = "6C76656C-ACF3-11D3-8CD1-00C04F8EDB8A"_uuid;
	//constexpr mpt::UUID guidJUNK       = "6b6E756A-ACF3-11D3-8CD1-00C04f8EDB8A"_uuid;
	//constexpr mpt::UUID guidBEXT       = "74786562-ACF3-11D3-8CD1-00C04F8EDB8A"_uuid;
	//constexpr mpt::UUID guiMARKER      = "ABF76256-392D-11D2-86C7-00C04F8EDB8A"_uuid;
	//constexpr mpt::UUID guiSUMMARYLIST = "925F94BC-525A-11D2-86DC-00C04F8EDB8A"_uuid;

	constexpr mpt::UUID guidCSET       = "54455343-ACF3-11D3-8CD1-00C04F8EDB8A"_uuid;

	Wave64FileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(mpt::UUID(fileHeader.GuidRIFF) != guidRIFF)
	{
		return false;
	}
	if(mpt::UUID(fileHeader.GuidWAVE) != guidWAVE)
	{
		return false;
	}
	if(fileHeader.FileSize != file.GetLength())
	{
		return false;
	}

	FileReader chunkFile = file;
	auto chunkList = chunkFile.ReadChunks<Wave64Chunk>(8);

	if(!chunkList.ChunkExists(guidFMT))
	{
		return false;
	}
	FileReader formatChunk = chunkList.GetChunk(guidFMT);
	WAVFormatChunk format;
	if(!formatChunk.ReadStruct(format))
	{
		return false;
	}
	uint16 sampleFormat = format.format;
	if(format.format == WAVFormatChunk::fmtExtensible)
	{
		WAVFormatChunkExtension formatExt;
		if(!formatChunk.ReadStruct(formatExt))
		{
			return false;
		}
		sampleFormat = static_cast<uint16>(mpt::UUID(formatExt.subFormat).GetData1());
	}
	if(format.sampleRate == 0)
	{
		return false;
	}
	if(format.numChannels == 0)
	{
		return false;
	}
	if(format.numChannels > 2)
	{
		return false;
	}
	if(sampleFormat != WAVFormatChunk::fmtPCM && sampleFormat != WAVFormatChunk::fmtFloat)
	{
		return false;
	}
	if(sampleFormat == WAVFormatChunk::fmtFloat && format.bitsPerSample != 32 && format.bitsPerSample != 64)
	{
		return false;
	}
	if(sampleFormat == WAVFormatChunk::fmtPCM && format.bitsPerSample > 64)
	{
		return false;
	}

	SampleIO::Bitdepth bitDepth;
	switch((format.bitsPerSample - 1) / 8u)
	{
	default:
	case 0: bitDepth = SampleIO::_8bit ; break;
	case 1: bitDepth = SampleIO::_16bit; break;
	case 2: bitDepth = SampleIO::_24bit; break;
	case 3: bitDepth = SampleIO::_32bit; break;
	case 7: bitDepth = SampleIO::_64bit; break;
	}
	SampleIO sampleIO(
		bitDepth,
		(format.numChannels > 1) ? SampleIO::stereoInterleaved : SampleIO::mono,
		SampleIO::littleEndian,
		(sampleFormat == WAVFormatChunk::fmtFloat) ? SampleIO::floatPCM : SampleIO::signedPCM);
	if(format.bitsPerSample <= 8)
	{
		sampleIO |= SampleIO::unsignedPCM;
	}
	if(mayNormalize)
	{
		sampleIO.MayNormalize();
	}

	FileTags tags;

	uint16 codePage = 28591; // mpt::Charset::ISO8859_1
	FileReader csetChunk = chunkList.GetChunk(guidCSET);
	if(csetChunk.IsValid())
	{
		if(csetChunk.CanRead(2))
		{
			codePage = csetChunk.ReadUint16LE();
		}
	}

	if(chunkList.ChunkExists(guidLIST))
	{
		FileReader listChunk = chunkList.GetChunk(guidLIST);
		if(listChunk.ReadMagic("INFO"))
		{
			auto infoChunk = listChunk.ReadChunks<RIFFChunk>(2);
			Wave64TagFromLISTINFO(tags.title, codePage, infoChunk, RIFFChunk::idINAM);
			Wave64TagFromLISTINFO(tags.encoder, codePage, infoChunk, RIFFChunk::idISFT);
			//Wave64TagFromLISTINFO(void, codePage, infoChunk, RIFFChunk::idICOP);
			Wave64TagFromLISTINFO(tags.artist, codePage, infoChunk, RIFFChunk::idIART);
			Wave64TagFromLISTINFO(tags.album, codePage, infoChunk, RIFFChunk::idIPRD);
			Wave64TagFromLISTINFO(tags.comments, codePage, infoChunk, RIFFChunk::idICMT);
			//Wave64TagFromLISTINFO(void, codePage, infoChunk, RIFFChunk::idIENG);
			//Wave64TagFromLISTINFO(void, codePage, infoChunk, RIFFChunk::idISBJ);
			Wave64TagFromLISTINFO(tags.genre, codePage, infoChunk, RIFFChunk::idIGNR);
			//Wave64TagFromLISTINFO(void, codePage, infoChunk, RIFFChunk::idICRD);
			Wave64TagFromLISTINFO(tags.year, codePage, infoChunk, RIFFChunk::idYEAR);
			Wave64TagFromLISTINFO(tags.trackno, codePage, infoChunk, RIFFChunk::idTRCK);
			Wave64TagFromLISTINFO(tags.url, codePage, infoChunk, RIFFChunk::idTURL);
			//Wave64TagFromLISTINFO(tags.bpm, codePage, infoChunk, void);
		}
	}

	if(!chunkList.ChunkExists(guidDATA))
	{
		return false;
	}
	FileReader audioData = chunkList.GetChunk(guidDATA);
	
	SmpLength length = mpt::saturate_cast<SmpLength>(audioData.GetLength() / (sampleIO.GetEncodedBitsPerSample()/8));

	ModSample &mptSample = Samples[nSample];
	DestroySampleThreadsafe(nSample);
	mptSample.Initialize();
	mptSample.nLength = length;
	mptSample.nC5Speed = format.sampleRate;

	sampleIO.ReadSample(mptSample, audioData);

	m_szNames[nSample] = mpt::ToCharset(GetCharsetInternal(), GetSampleNameFromTags(tags));

	mptSample.Convert(MOD_TYPE_IT, GetType());
	mptSample.PrecomputeLoops(*this, false);

	return true;

}



#ifndef MODPLUG_NO_FILESAVE

///////////////////////////////////////////////////////////////
// Save RAW

bool CSoundFile::SaveRAWSample(SAMPLEINDEX nSample, std::ostream &f) const
{
	const ModSample &sample = Samples[nSample];
	if(!sample.HasSampleData())
		return false;
	SampleIO(
		sample.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
		sample.uFlags[CHN_STEREO] ? SampleIO::stereoInterleaved : SampleIO::mono,
		SampleIO::littleEndian,
		SampleIO::signedPCM)
		.WriteSample(f, sample);

	return true;
}

#endif // MODPLUG_NO_FILESAVE

/////////////////////////////////////////////////////////////
// GUS Patches

struct GF1PatchFileHeader
{
	char     magic[8];       // "GF1PATCH"
	char     version[4];     // "100", or "110"
	char     id[10];         // "ID#000002"
	char     copyright[60];  // Copyright
	uint8le  numInstr;       // Number of instruments in patch
	uint8le  voices;         // Number of voices, usually 14
	uint8le  channels;       // Number of wav channels that can be played concurently to the patch
	uint16le numSamples;     // Total number of waveforms for all the .PAT
	uint16le volume;         // Master volume
	uint32le dataSize;
	char     reserved2[36];
};

MPT_BINARY_STRUCT(GF1PatchFileHeader, 129)


struct GF1Instrument
{
	uint16le id;        // Instrument id: 0-65535
	char     name[16];  // Name of instrument. Gravis doesn't seem to use it
	uint32le size;      // Number of bytes for the instrument with header. (To skip to next instrument)
	uint8    layers;    // Number of layers in instrument: 1-4
	char     reserved[40];
};

MPT_BINARY_STRUCT(GF1Instrument, 63)


struct GF1SampleHeader
{
	char     name[7];        // null terminated string. name of the wave.
	uint8le  fractions;      // Start loop point fraction in 4 bits + End loop point fraction in the 4 other bits.
	uint32le length;         // total size of wavesample. limited to 65535 now by the drivers, not the card.
	uint32le loopstart;      // start loop position in the wavesample
	uint32le loopend;        // end loop position in the wavesample
	uint16le freq;           // Rate at which the wavesample has been sampled
	uint32le low_freq;       // check note.h for the correspondance.
	uint32le high_freq;      // check note.h for the correspondance.
	uint32le root_freq;      // check note.h for the correspondance.
	int16le  finetune;       // fine tune. -512 to +512, EXCLUDING 0 cause it is a multiplier. 512 is one octave off, and 1 is a neutral value
	uint8le  balance;        // Balance: 0-15. 0=full left, 15 = full right
	uint8le  env_rate[6];    // attack rates
	uint8le  env_volume[6];  // attack volumes
	uint8le  tremolo_sweep, tremolo_rate, tremolo_depth;
	uint8le  vibrato_sweep, vibrato_rate, vibrato_depth;
	uint8le  flags;
	int16le  scale_frequency;  // Note
	uint16le scale_factor;     // 0...2048 (1024 is normal) or 0...2
	char     reserved[36];
};

MPT_BINARY_STRUCT(GF1SampleHeader, 96)

// -- GF1 Envelopes --
//
// It can be represented like this (the envelope is totally bogus, it is
// just to show the concept):
//
//  |
//  |           /----`               | |
//  |   /------/      `\         | | | | |
//  |  /                 \       | | | | |
//  | /                    \     | | | | |
//  |/                       \   | | | | |
//  ---------------------------- | | | | | |
//  <---> attack rate 0          0 1 2 3 4 5 amplitudes
//       <----> attack rate 1
//             <> attack rate 2
//               <--> attack rate 3
//                   <> attack rate 4
//                     <-----> attack rate 5
//
// -- GF1 Flags --
//
// bit 0: 8/16 bit
// bit 1: Signed/Unsigned
// bit 2: off/on looping
// bit 3: off/on bidirectionnal looping
// bit 4: off/on backward looping
// bit 5: off/on sustaining (3rd point in env.)
// bit 6: off/on envelopes
// bit 7: off/on clamped release (6th point, env)


struct GF1Layer
{
	uint8le  previous;  // If !=0 the wavesample to use is from the previous layer. The waveheader is still needed
	uint8le  id;        // Layer id: 0-3
	uint32le size;      // data size in bytes in the layer, without the header. to skip to next layer for example:
	uint8le  samples;   // number of wavesamples
	char     reserved[40];
};

MPT_BINARY_STRUCT(GF1Layer, 47)


static double PatchFreqToNote(uint32 nFreq)
{
	return std::log(nFreq / 2044.0) * (12.0 * 1.44269504088896340736);  // 1.0/std::log(2.0)
}


static int32 PatchFreqToNoteInt(uint32 nFreq)
{
	return mpt::saturate_round<int32>(PatchFreqToNote(nFreq));
}


static void PatchToSample(CSoundFile *that, SAMPLEINDEX nSample, GF1SampleHeader &sampleHeader, FileReader &file)
{
	ModSample &sample = that->GetSample(nSample);

	file.ReadStruct(sampleHeader);

	sample.Initialize();
	if(sampleHeader.flags & 4) sample.uFlags.set(CHN_LOOP);
	if(sampleHeader.flags & 8) sample.uFlags.set(CHN_PINGPONGLOOP);
	if(sampleHeader.flags & 16) sample.uFlags.set(CHN_REVERSE);
	sample.nLength = sampleHeader.length;
	sample.nLoopStart = sampleHeader.loopstart;
	sample.nLoopEnd = sampleHeader.loopend;
	sample.nC5Speed = sampleHeader.freq;
	sample.nPan = static_cast<uint16>((sampleHeader.balance * 256 + 8) / 15);
	if(sample.nPan > 256) sample.nPan = 128;
	else sample.uFlags.set(CHN_PANNING);
	sample.nVibType = VIB_SINE;
	sample.nVibSweep = sampleHeader.vibrato_sweep;
	sample.nVibDepth = sampleHeader.vibrato_depth;
	sample.nVibRate = sampleHeader.vibrato_rate / 4;
	if(sampleHeader.scale_factor)
	{
		sample.Transpose((84.0 - PatchFreqToNote(sampleHeader.root_freq)) / 12.0);
	}

	SampleIO sampleIO(
		SampleIO::_8bit,
		SampleIO::mono,
		SampleIO::littleEndian,
		(sampleHeader.flags & 2) ? SampleIO::unsignedPCM : SampleIO::signedPCM);

	if(sampleHeader.flags & 1)
	{
		sampleIO |= SampleIO::_16bit;
		sample.nLength /= 2;
		sample.nLoopStart /= 2;
		sample.nLoopEnd /= 2;
	}
	sampleIO.ReadSample(sample, file);
	sample.Convert(MOD_TYPE_IT, that->GetType());
	sample.PrecomputeLoops(*that, false);

	that->m_szNames[nSample] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, sampleHeader.name);
}


bool CSoundFile::ReadPATSample(SAMPLEINDEX nSample, FileReader &file)
{
	file.Rewind();
	GF1PatchFileHeader fileHeader;
	GF1Instrument instrHeader;  // We only support one instrument
	GF1Layer layerHeader;
	if(!file.ReadStruct(fileHeader)
		|| memcmp(fileHeader.magic, "GF1PATCH", 8)
		|| (memcmp(fileHeader.version, "110\0", 4) && memcmp(fileHeader.version, "100\0", 4))
		|| memcmp(fileHeader.id, "ID#000002\0", 10)
		|| !fileHeader.numInstr || !fileHeader.numSamples
		|| !file.ReadStruct(instrHeader)
		//|| !instrHeader.layers  // DOO.PAT has 0 layers
		|| !file.ReadStruct(layerHeader)
		|| !layerHeader.samples)
	{
		return false;
	}

	DestroySampleThreadsafe(nSample);
	GF1SampleHeader sampleHeader;
	PatchToSample(this, nSample, sampleHeader, file);

	if(instrHeader.name[0] > ' ')
	{
		m_szNames[nSample] = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, instrHeader.name);
	}
	return true;
}


// PAT Instrument
bool CSoundFile::ReadPATInstrument(INSTRUMENTINDEX nInstr, FileReader &file)
{
	file.Rewind();
	GF1PatchFileHeader fileHeader;
	GF1Instrument instrHeader;  // We only support one instrument
	GF1Layer layerHeader;
	if(!file.ReadStruct(fileHeader)
		|| memcmp(fileHeader.magic, "GF1PATCH", 8)
		|| (memcmp(fileHeader.version, "110\0", 4) && memcmp(fileHeader.version, "100\0", 4))
		|| memcmp(fileHeader.id, "ID#000002\0", 10)
		|| !fileHeader.numInstr || !fileHeader.numSamples
		|| !file.ReadStruct(instrHeader)
		//|| !instrHeader.layers  // DOO.PAT has 0 layers
		|| !file.ReadStruct(layerHeader)
		|| !layerHeader.samples)
	{
		return false;
	}

	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
	{
		return false;
	}

	DestroyInstrument(nInstr, deleteAssociatedSamples);
	if (nInstr > m_nInstruments) m_nInstruments = nInstr;
	Instruments[nInstr] = pIns;

	pIns->name = mpt::String::ReadBuf(mpt::String::maybeNullTerminated, instrHeader.name);
	pIns->nFadeOut = 2048;
	if(GetType() & (MOD_TYPE_IT | MOD_TYPE_MPT))
	{
		pIns->nNNA = NewNoteAction::NoteOff;
		pIns->nDNA = DuplicateNoteAction::NoteFade;
	}

	SAMPLEINDEX nextSample = 0;
	int32 nMinSmpNote = 0xFF;
	SAMPLEINDEX nMinSmp = 0;
	for(uint8 smp = 0; smp < layerHeader.samples; smp++)
	{
		// Find a free sample
		nextSample = GetNextFreeSample(nInstr, nextSample + 1);
		if(nextSample == SAMPLEINDEX_INVALID) break;
		if(m_nSamples < nextSample) m_nSamples = nextSample;
		if(!nMinSmp) nMinSmp = nextSample;
		// Load it
		GF1SampleHeader sampleHeader;
		PatchToSample(this, nextSample, sampleHeader, file);
		int32 nMinNote = (sampleHeader.low_freq > 100) ? PatchFreqToNoteInt(sampleHeader.low_freq) : 0;
		int32 nMaxNote = (sampleHeader.high_freq > 100) ? PatchFreqToNoteInt(sampleHeader.high_freq) : static_cast<uint8>(NOTE_MAX);
		int32 nBaseNote = (sampleHeader.root_freq > 100) ? PatchFreqToNoteInt(sampleHeader.root_freq) : -1;
		if(!sampleHeader.scale_factor && layerHeader.samples == 1) { nMinNote = 0; nMaxNote = NOTE_MAX; }
		// Fill Note Map
		for(int32 k = 0; k < NOTE_MAX; k++)
		{
			if(k == nBaseNote || (!pIns->Keyboard[k] && k >= nMinNote && k <= nMaxNote))
			{
				if(!sampleHeader.scale_factor)
					pIns->NoteMap[k] = NOTE_MIDDLEC;

				pIns->Keyboard[k] = nextSample;
				if(k < nMinSmpNote)
				{
					nMinSmpNote = k;
					nMinSmp = nextSample;
				}
			}
		}
	}
	if(nMinSmp)
	{
		// Fill note map and missing samples
		for(uint8 k = 0; k < NOTE_MAX; k++)
		{
			if(!pIns->NoteMap[k]) pIns->NoteMap[k] = k + 1;
			if(!pIns->Keyboard[k])
			{
				pIns->Keyboard[k] = nMinSmp;
			} else
			{
				nMinSmp = pIns->Keyboard[k];
			}
		}
	}

	pIns->Sanitize(MOD_TYPE_IT);
	pIns->Convert(MOD_TYPE_IT, GetType());
	return true;
}


/////////////////////////////////////////////////////////////
// S3I Samples


bool CSoundFile::ReadS3ISample(SAMPLEINDEX nSample, FileReader &file)
{
	file.Rewind();

	S3MSampleHeader sampleHeader;
	if(!file.ReadStruct(sampleHeader)
		|| (sampleHeader.sampleType != S3MSampleHeader::typePCM && sampleHeader.sampleType != S3MSampleHeader::typeAdMel)
		|| (memcmp(sampleHeader.magic, "SCRS", 4) && memcmp(sampleHeader.magic, "SCRI", 4))
		|| !file.Seek(sampleHeader.GetSampleOffset()))
	{
		return false;
	}

	if(sampleHeader.sampleType >= S3MSampleHeader::typeAdMel)
	{
		if(SupportsOPL())
		{
			InitOPL();
		} else
		{
			AddToLog(LogInformation, U_("OPL instruments are not supported by this format."));
			return true;
		}
	}

	DestroySampleThreadsafe(nSample);

	ModSample &sample = Samples[nSample];
	sampleHeader.ConvertToMPT(sample);
	m_szNames[nSample] = mpt::String::ReadBuf(mpt::String::nullTerminated, sampleHeader.name);

	if(sampleHeader.sampleType < S3MSampleHeader::typeAdMel)
		sampleHeader.GetSampleFormat(false).ReadSample(sample, file);

	sample.Convert(MOD_TYPE_S3M, GetType());
	sample.PrecomputeLoops(*this, false);
	return true;
}

#ifndef MODPLUG_NO_FILESAVE

bool CSoundFile::SaveS3ISample(SAMPLEINDEX smp, std::ostream &f) const
{
	const ModSample &sample = Samples[smp];
	if(!sample.uFlags[CHN_ADLIB] && !sample.HasSampleData())
		return false;
	S3MSampleHeader sampleHeader{};
	SmpLength length = sampleHeader.ConvertToS3M(sample);
	mpt::String::WriteBuf(mpt::String::nullTerminated, sampleHeader.name) = m_szNames[smp];
	mpt::String::WriteBuf(mpt::String::maybeNullTerminated, sampleHeader.reserved2) = mpt::ToCharset(mpt::Charset::UTF8, Version::Current().GetOpenMPTVersionString());
	if(length)
		sampleHeader.dataPointer[1] = sizeof(S3MSampleHeader) >> 4;
	mpt::IO::Write(f, sampleHeader);
	if(length)
		sampleHeader.GetSampleFormat(false).WriteSample(f, sample, length);

	return true;
}

#endif // MODPLUG_NO_FILESAVE


/////////////////////////////////////////////////////////////
// SBI OPL patch files

bool CSoundFile::ReadSBISample(SAMPLEINDEX sample, FileReader &file)
{
	file.Rewind();
	const auto magic = file.ReadArray<char, 4>();
	if((memcmp(magic.data(), "SBI\x1A", 4) && memcmp(magic.data(), "SBI\x1D", 4))  // 1D =  broken JuceOPLVSTi files
	   || !file.CanRead(32 + sizeof(OPLPatch))
	   || file.CanRead(64))  // Arbitrary threshold to reject files that are unlikely to be SBI files
		return false;

	if(!SupportsOPL())
	{
		AddToLog(LogInformation, U_("OPL instruments are not supported by this format."));
		return true;
	}

	DestroySampleThreadsafe(sample);
	InitOPL();

	ModSample &mptSmp = Samples[sample];
	mptSmp.Initialize(MOD_TYPE_S3M);
	file.ReadString<mpt::String::nullTerminated>(m_szNames[sample], 32);
	OPLPatch patch;
	file.ReadArray(patch);
	mptSmp.SetAdlib(true, patch);

	mptSmp.Convert(MOD_TYPE_S3M, GetType());
	return true;
}



/////////////////////////////////////////////////////////////
// XI Instruments


bool CSoundFile::ReadXIInstrument(INSTRUMENTINDEX nInstr, FileReader &file)
{
	file.Rewind();

	XIInstrumentHeader fileHeader;
	if(!file.ReadStruct(fileHeader)
		|| memcmp(fileHeader.signature, "Extended Instrument: ", 21)
		|| fileHeader.version != XIInstrumentHeader::fileVersion
		|| fileHeader.eof != 0x1A)
	{
		return false;
	}

	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
	{
		return false;
	}

	DestroyInstrument(nInstr, deleteAssociatedSamples);
	if(nInstr > m_nInstruments)
	{
		m_nInstruments = nInstr;
	}
	Instruments[nInstr] = pIns;

	fileHeader.ConvertToMPT(*pIns);

	// Translate sample map and find available sample slots
	std::vector<SAMPLEINDEX> sampleMap(fileHeader.numSamples);
	SAMPLEINDEX maxSmp = 0;

	for(size_t i = 0 + 12; i < 96 + 12; i++)
	{
		if(pIns->Keyboard[i] >= fileHeader.numSamples)
		{
			continue;
		}

		if(sampleMap[pIns->Keyboard[i]] == 0)
		{
			// Find slot for this sample
			maxSmp = GetNextFreeSample(nInstr, maxSmp + 1);
			if(maxSmp != SAMPLEINDEX_INVALID)
			{
				sampleMap[pIns->Keyboard[i]] = maxSmp;
			}
		}
		pIns->Keyboard[i] = sampleMap[pIns->Keyboard[i]];
	}

	if(m_nSamples < maxSmp)
	{
		m_nSamples = maxSmp;
	}

	std::vector<SampleIO> sampleFlags(fileHeader.numSamples);

	// Read sample headers
	for(SAMPLEINDEX i = 0; i < fileHeader.numSamples; i++)
	{
		XMSample sampleHeader;
		if(!file.ReadStruct(sampleHeader)
			|| !sampleMap[i])
		{
			continue;
		}

		ModSample &mptSample = Samples[sampleMap[i]];
		sampleHeader.ConvertToMPT(mptSample);
		fileHeader.instrument.ApplyAutoVibratoToMPT(mptSample);
		mptSample.Convert(MOD_TYPE_XM, GetType());
		if(GetType() != MOD_TYPE_XM && fileHeader.numSamples == 1)
		{
			// No need to pan that single sample, thank you...
			mptSample.uFlags &= ~CHN_PANNING;
		}

		mptSample.filename = mpt::String::ReadBuf(mpt::String::spacePadded, sampleHeader.name);
		m_szNames[sampleMap[i]] = mpt::String::ReadBuf(mpt::String::spacePadded, sampleHeader.name);

		sampleFlags[i] = sampleHeader.GetSampleFormat();
	}

	// Read sample data
	for(SAMPLEINDEX i = 0; i < fileHeader.numSamples; i++)
	{
		if(sampleMap[i])
		{
			sampleFlags[i].ReadSample(Samples[sampleMap[i]], file);
			Samples[sampleMap[i]].PrecomputeLoops(*this, false);
		}
	}

	// Read MPT crap
	LoadExtendedInstrumentProperties(mpt::as_span(&Instruments[nInstr], 1), file);
	pIns->Convert(MOD_TYPE_XM, GetType());
	pIns->Sanitize(GetType());
	return true;
}


#ifndef MODPLUG_NO_FILESAVE

bool CSoundFile::SaveXIInstrument(INSTRUMENTINDEX nInstr, std::ostream &f) const
{
	ModInstrument *pIns = Instruments[nInstr];
	if(pIns == nullptr)
	{
		return false;
	}

	// Create file header
	XIInstrumentHeader header;
	const auto sampleList = header.ConvertToXM(*pIns, false);
	const auto &samples = sampleList.samples;
	if(sampleList.tooManySamples)
		AddToLog(LogInformation, MPT_UFORMAT("This instrument references too many samples, only the first {} will be exported.")(samples.size()));

	if(samples.size() > 0 && samples[0] <= GetNumSamples())
	{
		// Copy over auto-vibrato settings of first sample
		header.instrument.ApplyAutoVibratoToXM(Samples[samples[0]], GetType());
	}

	mpt::IO::Write(f, header);

	std::vector<SampleIO> sampleFlags(samples.size());

	// XI Sample Headers
	for(SAMPLEINDEX i = 0; i < samples.size(); i++)
	{
		XMSample xmSample;
		if(samples[i] <= GetNumSamples())
		{
			xmSample.ConvertToXM(Samples[samples[i]], GetType(), false);
		} else
		{
			MemsetZero(xmSample);
		}
		sampleFlags[i] = xmSample.GetSampleFormat();

		mpt::String::WriteBuf(mpt::String::spacePadded, xmSample.name) = m_szNames[samples[i]];

		mpt::IO::Write(f, xmSample);
	}

	// XI Sample Data
	for(SAMPLEINDEX i = 0; i < samples.size(); i++)
	{
		if(samples[i] <= GetNumSamples())
		{
			sampleFlags[i].WriteSample(f, Samples[samples[i]]);
		}
	}

	SaveExtendedInstrumentProperties(nInstr, MOD_TYPE_XM, f);

	return true;
}

#endif // MODPLUG_NO_FILESAVE


// Read first sample from XI file into a sample slot
bool CSoundFile::ReadXISample(SAMPLEINDEX nSample, FileReader &file)
{
	file.Rewind();

	XIInstrumentHeader fileHeader;
	if(!file.ReadStruct(fileHeader)
		|| !file.CanRead(sizeof(XMSample))
		|| memcmp(fileHeader.signature, "Extended Instrument: ", 21)
		|| fileHeader.version != XIInstrumentHeader::fileVersion
		|| fileHeader.eof != 0x1A
		|| fileHeader.numSamples == 0)
	{
		return false;
	}

	if(m_nSamples < nSample)
	{
		m_nSamples = nSample;
	}

	uint16 numSamples = fileHeader.numSamples;
	FileReader::pos_type samplePos = sizeof(XIInstrumentHeader) + numSamples * sizeof(XMSample);
	// Preferably read the middle-C sample
	auto sample = fileHeader.instrument.sampleMap[48];
	if(sample >= fileHeader.numSamples)
		sample = 0;
	XMSample sampleHeader;
	while(sample--)
	{
		file.ReadStruct(sampleHeader);
		samplePos += sampleHeader.length;
	}
	file.ReadStruct(sampleHeader);
	// Gotta skip 'em all!
	file.Seek(samplePos);

	DestroySampleThreadsafe(nSample);

	ModSample &mptSample = Samples[nSample];
	sampleHeader.ConvertToMPT(mptSample);
	if(GetType() != MOD_TYPE_XM)
	{
		// No need to pan that single sample, thank you...
		mptSample.uFlags.reset(CHN_PANNING);
	}
	fileHeader.instrument.ApplyAutoVibratoToMPT(mptSample);
	mptSample.Convert(MOD_TYPE_XM, GetType());

	mptSample.filename = mpt::String::ReadBuf(mpt::String::spacePadded, sampleHeader.name);
	m_szNames[nSample] = mpt::String::ReadBuf(mpt::String::spacePadded, sampleHeader.name);

	// Read sample data
	sampleHeader.GetSampleFormat().ReadSample(Samples[nSample], file);
	Samples[nSample].PrecomputeLoops(*this, false);

	return true;
}


///////////////
// Apple CAF //


struct CAFFileHeader
{
	uint32be mFileType;
	uint16be mFileVersion;
	uint16be mFileFlags;
};

MPT_BINARY_STRUCT(CAFFileHeader, 8)


struct CAFChunkHeader
{
	uint32be mChunkType;
	int64be  mChunkSize;
};

MPT_BINARY_STRUCT(CAFChunkHeader, 12)


struct CAFChunk
{
	enum ChunkIdentifiers
	{
		iddesc = MagicBE("desc"),
		iddata = MagicBE("data"),
		idstrg = MagicBE("strg"),
		idinfo = MagicBE("info")
	};

	CAFChunkHeader header;

	FileReader::pos_type GetLength() const
	{
		int64 length = header.mChunkSize;
		if(length == -1)
		{
			length = std::numeric_limits<int64>::max();  // spec
		}
		if(length < 0)
		{
			length = std::numeric_limits<int64>::max();  // heuristic
		}
		return mpt::saturate_cast<FileReader::pos_type>(length);
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(header.mChunkType.get());
	}
};

MPT_BINARY_STRUCT(CAFChunk, 12)


enum {
	CAFkAudioFormatLinearPCM      = MagicBE("lpcm"),
	CAFkAudioFormatAppleIMA4      = MagicBE("ima4"),
	CAFkAudioFormatMPEG4AAC       = MagicBE("aac "),
	CAFkAudioFormatMACE3          = MagicBE("MAC3"),
	CAFkAudioFormatMACE6          = MagicBE("MAC6"),
	CAFkAudioFormatULaw           = MagicBE("ulaw"),
	CAFkAudioFormatALaw           = MagicBE("alaw"),
	CAFkAudioFormatMPEGLayer1     = MagicBE(".mp1"),
	CAFkAudioFormatMPEGLayer2     = MagicBE(".mp2"),
	CAFkAudioFormatMPEGLayer3     = MagicBE(".mp3"),
	CAFkAudioFormatAppleLossless  = MagicBE("alac")
};


enum {
	CAFkCAFLinearPCMFormatFlagIsFloat         = (1L << 0),
	CAFkCAFLinearPCMFormatFlagIsLittleEndian  = (1L << 1)
};


struct CAFAudioFormat
{
	float64be mSampleRate;
	uint32be  mFormatID;
	uint32be  mFormatFlags;
	uint32be  mBytesPerPacket;
	uint32be  mFramesPerPacket;
	uint32be  mChannelsPerFrame;
	uint32be  mBitsPerChannel;
};

MPT_BINARY_STRUCT(CAFAudioFormat, 32)


static void CAFSetTagFromInfoKey(mpt::ustring & dst, const std::map<std::string,std::string> & infoMap, const std::string & key)
{
	auto item = infoMap.find(key);
	if(item == infoMap.end())
	{
		return;
	}
	if(item->second.empty())
	{
		return;
	}
	dst = mpt::ToUnicode(mpt::Charset::UTF8, item->second);
}


bool CSoundFile::ReadCAFSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize)
{
	file.Rewind();

	CAFFileHeader fileHeader;
	if(!file.ReadStruct(fileHeader))
	{
		return false;
	}
	if(fileHeader.mFileType != MagicBE("caff"))
	{
		return false;
	}
	if(fileHeader.mFileVersion != 1)
	{
		return false;
	}

	auto chunkList = file.ReadChunks<CAFChunk>(0);

	CAFAudioFormat audioFormat;
	if(!chunkList.GetChunk(CAFChunk::iddesc).ReadStruct(audioFormat))
	{
		return false;
	}
	if(audioFormat.mSampleRate <= 0.0)
	{
		return false;
	}
	if(audioFormat.mChannelsPerFrame == 0)
	{
		return false;
	}
	if(audioFormat.mChannelsPerFrame > 2)
	{
		return false;
	}

	if(!mpt::in_range<uint32>(mpt::saturate_round<int64>(audioFormat.mSampleRate.get())))
	{
		return false;
	}
	uint32 sampleRate = static_cast<uint32>(mpt::saturate_round<int64>(audioFormat.mSampleRate.get()));
	if(sampleRate <= 0)
	{
		return false;
	}

	SampleIO sampleIO;
	if(audioFormat.mFormatID == CAFkAudioFormatLinearPCM)
	{
		if(audioFormat.mFramesPerPacket != 1)
		{
			return false;
		}
		if(audioFormat.mBytesPerPacket == 0)
		{
			return false;
		}
		if(audioFormat.mBitsPerChannel == 0)
		{
			return false;
		}
		if(audioFormat.mFormatFlags & CAFkCAFLinearPCMFormatFlagIsFloat)
		{
			if(audioFormat.mBitsPerChannel != 32 && audioFormat.mBitsPerChannel != 64)
			{
				return false;
			}
			if(audioFormat.mBytesPerPacket != audioFormat.mChannelsPerFrame * audioFormat.mBitsPerChannel/8)
			{
				return false;
			}
		}
		if(audioFormat.mBytesPerPacket % audioFormat.mChannelsPerFrame != 0)
		{
			return false;
		}
		if(audioFormat.mBytesPerPacket / audioFormat.mChannelsPerFrame != 1
			&& audioFormat.mBytesPerPacket / audioFormat.mChannelsPerFrame != 2
			&& audioFormat.mBytesPerPacket / audioFormat.mChannelsPerFrame != 3
			&& audioFormat.mBytesPerPacket / audioFormat.mChannelsPerFrame != 4
			&& audioFormat.mBytesPerPacket / audioFormat.mChannelsPerFrame != 8
			)
		{
			return false;
		}
		SampleIO::Channels channels = (audioFormat.mChannelsPerFrame == 2) ? SampleIO::stereoInterleaved : SampleIO::mono;
		SampleIO::Endianness endianness = (audioFormat.mFormatFlags & CAFkCAFLinearPCMFormatFlagIsLittleEndian) ? SampleIO::littleEndian : SampleIO::bigEndian;
		SampleIO::Encoding encoding = (audioFormat.mFormatFlags & CAFkCAFLinearPCMFormatFlagIsFloat) ? SampleIO::floatPCM : SampleIO::signedPCM;
		SampleIO::Bitdepth bitdepth = static_cast<SampleIO::Bitdepth>((audioFormat.mBytesPerPacket / audioFormat.mChannelsPerFrame) * 8);
		sampleIO = SampleIO(bitdepth, channels, endianness, encoding);
	} else
	{
		return false;
	}

	if(mayNormalize)
	{
		sampleIO.MayNormalize();
	}

	/*
	std::map<uint32, std::string> stringMap; // UTF-8
	if(chunkList.ChunkExists(CAFChunk::idstrg))
	{
		FileReader stringsChunk = chunkList.GetChunk(CAFChunk::idstrg);
		uint32 numEntries = stringsChunk.ReadUint32BE();
		if(stringsChunk.Skip(12 * numEntries))
		{
			FileReader stringData = stringsChunk.ReadChunk(stringsChunk.BytesLeft());
			stringsChunk.Seek(4);
			for(uint32 entry = 0; entry < numEntries && stringsChunk.CanRead(12); entry++)
			{
				uint32 stringID = stringsChunk.ReadUint32BE();
				int64 offset = stringsChunk.ReadIntBE<int64>();
				if(offset >= 0 && mpt::in_range<FileReader::pos_type>(offset))
				{
					stringData.Seek(mpt::saturate_cast<FileReader::pos_type>(offset));
					std::string str;
					if(stringData.ReadNullString(str))
					{
						stringMap[stringID] = str;
					}
				}
			}
		}
	}
	*/

	std::map<std::string, std::string> infoMap; // UTF-8
	if(chunkList.ChunkExists(CAFChunk::idinfo))
	{
		FileReader informationChunk = chunkList.GetChunk(CAFChunk::idinfo);
		uint32 numEntries = informationChunk.ReadUint32BE();
		for(uint32 entry = 0; entry < numEntries && informationChunk.CanRead(2); entry++)
		{
			std::string key;
			std::string value;
			if(!informationChunk.ReadNullString(key))
			{
				break;
			}
			if(!informationChunk.ReadNullString(value))
			{
				break;
			}
			if(!key.empty() && !value.empty())
			{
				infoMap[key] = value;
			}
		}
	}
	FileTags tags;
	CAFSetTagFromInfoKey(tags.bpm, infoMap, "tempo");
	//CAFSetTagFromInfoKey(void, infoMap, "key signature");
	//CAFSetTagFromInfoKey(void, infoMap, "time signature");
	CAFSetTagFromInfoKey(tags.artist, infoMap, "artist");
	CAFSetTagFromInfoKey(tags.album, infoMap, "album");
	CAFSetTagFromInfoKey(tags.trackno, infoMap, "track number");
	CAFSetTagFromInfoKey(tags.year, infoMap, "year");
	//CAFSetTagFromInfoKey(void, infoMap, "composer");
	//CAFSetTagFromInfoKey(void, infoMap, "lyricist");
	CAFSetTagFromInfoKey(tags.genre, infoMap, "genre");
	CAFSetTagFromInfoKey(tags.title, infoMap, "title");
	//CAFSetTagFromInfoKey(void, infoMap, "recorded date");
	CAFSetTagFromInfoKey(tags.comments, infoMap, "comments");
	//CAFSetTagFromInfoKey(void, infoMap, "copyright");
	//CAFSetTagFromInfoKey(void, infoMap, "source encoder");
	CAFSetTagFromInfoKey(tags.encoder, infoMap, "encoding application");
	//CAFSetTagFromInfoKey(void, infoMap, "nominal bit rate");
	//CAFSetTagFromInfoKey(void, infoMap, "channel layout");
	//CAFSetTagFromInfoKey(tags.url, infoMap, void);

	if(!chunkList.ChunkExists(CAFChunk::iddata))
	{
		return false;
	}
	FileReader dataChunk = chunkList.GetChunk(CAFChunk::iddata);
	dataChunk.Skip(4);  // edit count
	FileReader audioData = dataChunk.ReadChunk(dataChunk.BytesLeft());
	
	SmpLength length = mpt::saturate_cast<SmpLength>((audioData.GetLength() / audioFormat.mBytesPerPacket) * audioFormat.mFramesPerPacket);

	ModSample &mptSample = Samples[nSample];
	DestroySampleThreadsafe(nSample);
	mptSample.Initialize();
	mptSample.nLength = length;
	mptSample.nC5Speed = sampleRate;

	sampleIO.ReadSample(mptSample, audioData);

	m_szNames[nSample] = mpt::ToCharset(GetCharsetInternal(), GetSampleNameFromTags(tags));

	mptSample.Convert(MOD_TYPE_IT, GetType());
	mptSample.PrecomputeLoops(*this, false);

	return true;

}


/////////////////////////////////////////////////////////////////////////////////////////
// AIFF File I/O

// AIFF header
struct AIFFHeader
{
	char     magic[4];  // FORM
	uint32be length;    // Size of the file, not including magic and length
	char     type[4];   // AIFF or AIFC
};

MPT_BINARY_STRUCT(AIFFHeader, 12)


// General IFF Chunk header
struct AIFFChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		idCOMM	= MagicBE("COMM"),
		idSSND	= MagicBE("SSND"),
		idINST	= MagicBE("INST"),
		idMARK	= MagicBE("MARK"),
		idNAME	= MagicBE("NAME"),
	};

	uint32be id;      // See ChunkIdentifiers
	uint32be length;  // Chunk size without header

	size_t GetLength() const
	{
		return length;
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(id.get());
	}
};

MPT_BINARY_STRUCT(AIFFChunk, 8)


// "Common" chunk (in AIFC, a compression ID and compression name follows this header, but apart from that it's identical)
struct AIFFCommonChunk
{
	uint16be numChannels;
	uint32be numSampleFrames;
	uint16be sampleSize;
	uint8be  sampleRate[10];  // Sample rate in 80-Bit floating point

	// Convert sample rate to integer
	uint32 GetSampleRate() const
	{
		uint32 mantissa = (sampleRate[2] << 24) | (sampleRate[3] << 16) | (sampleRate[4] << 8) | (sampleRate[5] << 0);
		uint32 last = 0;
		uint8 exp = 30 - sampleRate[1];

		while(exp--)
		{
			last = mantissa;
			mantissa >>= 1;
		}
		if(last & 1) mantissa++;
		return mantissa;
	}
};

MPT_BINARY_STRUCT(AIFFCommonChunk, 18)


// Sound chunk
struct AIFFSoundChunk
{
	uint32be offset;
	uint32be blockSize;
};

MPT_BINARY_STRUCT(AIFFSoundChunk, 8)


// Marker
struct AIFFMarker
{
	uint16be id;
	uint32be position;    // Position in sample
	uint8be  nameLength;  // Not counting eventually existing padding byte in name string
};

MPT_BINARY_STRUCT(AIFFMarker, 7)


// Instrument loop
struct AIFFInstrumentLoop
{
	enum PlayModes
	{
		noLoop     = 0,
		loopNormal = 1,
		loopBidi   = 2,
	};

	uint16be playMode;
	uint16be beginLoop;  // Marker index
	uint16be endLoop;    // Marker index
};

MPT_BINARY_STRUCT(AIFFInstrumentLoop, 6)


struct AIFFInstrumentChunk
{
	uint8be  baseNote;
	uint8be  detune;
	uint8be  lowNote;
	uint8be  highNote;
	uint8be  lowVelocity;
	uint8be  highVelocity;
	uint16be gain;
	AIFFInstrumentLoop sustainLoop;
	AIFFInstrumentLoop releaseLoop;
};

MPT_BINARY_STRUCT(AIFFInstrumentChunk, 20)


bool CSoundFile::ReadAIFFSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize)
{
	file.Rewind();

	// Verify header
	AIFFHeader fileHeader;
	if(!file.ReadStruct(fileHeader)
		|| memcmp(fileHeader.magic, "FORM", 4)
		|| (memcmp(fileHeader.type, "AIFF", 4) && memcmp(fileHeader.type, "AIFC", 4)))
	{
		return false;
	}

	auto chunks = file.ReadChunks<AIFFChunk>(2);

	// Read COMM chunk
	FileReader commChunk(chunks.GetChunk(AIFFChunk::idCOMM));
	AIFFCommonChunk sampleInfo;
	if(!commChunk.ReadStruct(sampleInfo))
	{
		return false;
	}

	// Is this a proper sample?
	if(sampleInfo.numSampleFrames == 0
		|| sampleInfo.numChannels < 1 || sampleInfo.numChannels > 2
		|| sampleInfo.sampleSize < 1 || sampleInfo.sampleSize > 64)
	{
		return false;
	}

	// Read compression type in AIFF-C files.
	uint8 compression[4] = { 'N', 'O', 'N', 'E' };
	SampleIO::Endianness endian = SampleIO::bigEndian;
	if(!memcmp(fileHeader.type, "AIFC", 4))
	{
		if(!commChunk.ReadArray(compression))
		{
			return false;
		}
		if(!memcmp(compression, "twos", 4))
		{
			endian = SampleIO::littleEndian;
		}
	}

	// Read SSND chunk
	FileReader soundChunk(chunks.GetChunk(AIFFChunk::idSSND));
	AIFFSoundChunk sampleHeader;
	if(!soundChunk.ReadStruct(sampleHeader))
	{
		return false;
	}

	SampleIO::Bitdepth bitDepth;
	switch((sampleInfo.sampleSize - 1) / 8)
	{
	default:
	case 0: bitDepth = SampleIO::_8bit; break;
	case 1: bitDepth = SampleIO::_16bit; break;
	case 2: bitDepth = SampleIO::_24bit; break;
	case 3: bitDepth = SampleIO::_32bit; break;
	case 7: bitDepth = SampleIO::_64bit; break;
	}

	SampleIO sampleIO(bitDepth,
		(sampleInfo.numChannels == 2) ? SampleIO::stereoInterleaved : SampleIO::mono,
		endian,
		SampleIO::signedPCM);

	if(!memcmp(compression, "fl32", 4) || !memcmp(compression, "FL32", 4) || !memcmp(compression, "fl64", 4) || !memcmp(compression, "FL64", 4))
	{
		sampleIO |= SampleIO::floatPCM;
	} else if(!memcmp(compression, "alaw", 4) || !memcmp(compression, "ALAW", 4))
	{
		sampleIO |= SampleIO::aLaw;
		sampleIO |= SampleIO::_16bit;
	} else if(!memcmp(compression, "ulaw", 4) || !memcmp(compression, "ULAW", 4))
	{
		sampleIO |= SampleIO::uLaw;
		sampleIO |= SampleIO::_16bit;
	} else if(!memcmp(compression, "raw ", 4))
	{
		sampleIO |= SampleIO::unsignedPCM;
	}

	if(mayNormalize)
	{
		sampleIO.MayNormalize();
	}

	if(soundChunk.CanRead(sampleHeader.offset))
	{
		soundChunk.Skip(sampleHeader.offset);
	}

	ModSample &mptSample = Samples[nSample];
	DestroySampleThreadsafe(nSample);
	mptSample.Initialize();
	mptSample.nLength = sampleInfo.numSampleFrames;
	mptSample.nC5Speed = sampleInfo.GetSampleRate();

	sampleIO.ReadSample(mptSample, soundChunk);

	// Read MARK and INST chunk to extract sample loops
	FileReader markerChunk(chunks.GetChunk(AIFFChunk::idMARK));
	AIFFInstrumentChunk instrHeader;
	if(markerChunk.IsValid() && chunks.GetChunk(AIFFChunk::idINST).ReadStruct(instrHeader))
	{
		uint16 numMarkers = markerChunk.ReadUint16BE();

		std::vector<AIFFMarker> markers;
		markers.reserve(numMarkers);
		for(size_t i = 0; i < numMarkers; i++)
		{
			AIFFMarker marker;
			if(!markerChunk.ReadStruct(marker))
			{
				break;
			}
			markers.push_back(marker);
			markerChunk.Skip(marker.nameLength + ((marker.nameLength % 2u) == 0 ? 1 : 0));
		}

		if(instrHeader.sustainLoop.playMode != AIFFInstrumentLoop::noLoop)
		{
			mptSample.uFlags.set(CHN_SUSTAINLOOP);
			mptSample.uFlags.set(CHN_PINGPONGSUSTAIN, instrHeader.sustainLoop.playMode == AIFFInstrumentLoop::loopBidi);
		}

		if(instrHeader.releaseLoop.playMode != AIFFInstrumentLoop::noLoop)
		{
			mptSample.uFlags.set(CHN_LOOP);
			mptSample.uFlags.set(CHN_PINGPONGLOOP, instrHeader.releaseLoop.playMode == AIFFInstrumentLoop::loopBidi);
		}

		// Read markers
		for(const auto &m : markers)
		{
			if(m.id == instrHeader.sustainLoop.beginLoop)
				mptSample.nSustainStart = m.position;
			if(m.id == instrHeader.sustainLoop.endLoop)
				mptSample.nSustainEnd = m.position;
			if(m.id == instrHeader.releaseLoop.beginLoop)
				mptSample.nLoopStart = m.position;
			if(m.id == instrHeader.releaseLoop.endLoop)
				mptSample.nLoopEnd = m.position;
		}
		mptSample.SanitizeLoops();
	}

	// Extract sample name
	FileReader nameChunk(chunks.GetChunk(AIFFChunk::idNAME));
	if(nameChunk.IsValid())
	{
		nameChunk.ReadString<mpt::String::spacePadded>(m_szNames[nSample], mpt::saturate_cast<std::size_t>(nameChunk.GetLength()));
	} else
	{
		m_szNames[nSample] = "";
	}

	mptSample.Convert(MOD_TYPE_IT, GetType());
	mptSample.PrecomputeLoops(*this, false);
	return true;
}


static bool AUIsAnnotationLineWithField(const std::string &line)
{
	std::size_t pos = line.find('=');
	if(pos == std::string::npos)
	{
		return false;
	}
	if(pos == 0)
	{
		return false;
	}
	const auto field = std::string_view(line).substr(0, pos);
	// Scan for invalid chars
	for(auto c : field)
	{
		if(!mpt::is_in_range(c, 'a', 'z') && !mpt::is_in_range(c, 'A', 'Z') && !mpt::is_in_range(c, '0', '9') && c != '-' && c != '_')
		{
			return false;
		}
	}
	return true;
}

static std::string AUTrimFieldFromAnnotationLine(const std::string &line)
{
	if(!AUIsAnnotationLineWithField(line))
	{
		return line;
	}
	std::size_t pos = line.find('=');
	return line.substr(pos + 1);
}

static std::string AUGetAnnotationFieldFromLine(const std::string &line)
{
	if(!AUIsAnnotationLineWithField(line))
	{
		return std::string();
	}
	std::size_t pos = line.find('=');
	return line.substr(0, pos);
}

bool CSoundFile::ReadAUSample(SAMPLEINDEX nSample, FileReader &file, bool mayNormalize)
{
	file.Rewind();

	// Verify header
	const auto magic = file.ReadArray<char, 4>();
	const bool bigEndian = !std::memcmp(magic.data(), ".snd", 4);
	const bool littleEndian = !std::memcmp(magic.data(), "dns.", 4);
	if(!bigEndian && !littleEndian)
		return false;

	auto readUint32 = std::bind(bigEndian ? &FileReader::ReadUint32BE : &FileReader::ReadUint32LE, file);

	uint32 dataOffset = readUint32();  // must be divisible by 8 according to spec, however, there are files that ignore this requirement
	uint32 dataSize = readUint32();
	uint32 encoding = readUint32();
	uint32 sampleRate = readUint32();
	uint32 channels = readUint32();

	// According to spec, a minimum 8 byte annotation field after the header fields is required,
	// however, there are files in the wild that violate this requirement.
	// Thus, check for 24 instead of 32 here.
	if(dataOffset < 24) // data offset points inside header
	{
		return false;
	}

	if(channels < 1 || channels > 2)
		return false;

	SampleIO sampleIO(SampleIO::_8bit, channels == 1 ? SampleIO::mono : SampleIO::stereoInterleaved, bigEndian ? SampleIO::bigEndian : SampleIO::littleEndian, SampleIO::signedPCM);
	switch(encoding)
	{
	case 1: sampleIO |= SampleIO::_16bit;         // u-law
		sampleIO |= SampleIO::uLaw; break;
	case 2: break;                                // 8-bit linear PCM
	case 3: sampleIO |= SampleIO::_16bit; break;  // 16-bit linear PCM
	case 4: sampleIO |= SampleIO::_24bit; break;  // 24-bit linear PCM
	case 5: sampleIO |= SampleIO::_32bit; break;  // 32-bit linear PCM
	case 6: sampleIO |= SampleIO::_32bit;         // 32-bit IEEE floating point
		sampleIO |= SampleIO::floatPCM;
		break;
	case 7: sampleIO |= SampleIO::_64bit;         // 64-bit IEEE floating point
		sampleIO |= SampleIO::floatPCM;
		break;
	case 27: sampleIO |= SampleIO::_16bit;        // a-law
		sampleIO |= SampleIO::aLaw; break;
	default: return false;
	}

	if(!file.LengthIsAtLeast(dataOffset))
	{
		return false;
	}

	FileTags tags;

	// This reads annotation metadata as written by OpenMPT, sox, ffmpeg.
	// Additionally, we fall back to just reading the whole field as a single comment.
	// We only read up to the first \0 byte.
	file.Seek(24);
	std::string annotation;
	file.ReadString<mpt::String::maybeNullTerminated>(annotation, dataOffset - 24);
	annotation = mpt::replace(annotation, std::string("\r\n"), std::string("\n"));
	annotation = mpt::replace(annotation, std::string("\r"), std::string("\n"));
	mpt::Charset charset = mpt::IsUTF8(annotation) ? mpt::Charset::UTF8 : mpt::Charset::ISO8859_1;
	const auto lines = mpt::split(annotation, std::string("\n"));
	bool hasFields = false;
	for(const auto &line : lines)
	{
		if(AUIsAnnotationLineWithField(line))
		{
			hasFields = true;
			break;
		}
	}
	if(hasFields)
	{
		std::map<std::string, std::vector<std::string>> linesPerField;
		std::string lastField = "comment";
		for(const auto &line : lines)
		{
			if(AUIsAnnotationLineWithField(line))
			{
				lastField = mpt::ToLowerCaseAscii(mpt::trim(AUGetAnnotationFieldFromLine(line)));
			}
			linesPerField[lastField].push_back(AUTrimFieldFromAnnotationLine(line));
		}
		tags.title    = mpt::ToUnicode(charset, mpt::join_format(linesPerField["title"  ], std::string("\n")));
		tags.artist   = mpt::ToUnicode(charset, mpt::join_format(linesPerField["artist" ], std::string("\n")));
		tags.album    = mpt::ToUnicode(charset, mpt::join_format(linesPerField["album"  ], std::string("\n")));
		tags.trackno  = mpt::ToUnicode(charset, mpt::join_format(linesPerField["track"  ], std::string("\n")));
		tags.genre    = mpt::ToUnicode(charset, mpt::join_format(linesPerField["genre"  ], std::string("\n")));
		tags.comments = mpt::ToUnicode(charset, mpt::join_format(linesPerField["comment"], std::string("\n")));
	} else
	{
		// Most applications tend to write their own name here,
		// thus there is little use in interpreting the string as a title.
		annotation = mpt::trim_right(annotation, std::string("\r\n"));
		tags.comments = mpt::ToUnicode(charset, annotation);
	}

	file.Seek(dataOffset);

	ModSample &mptSample = Samples[nSample];
	DestroySampleThreadsafe(nSample);
	mptSample.Initialize();
	SmpLength length = mpt::saturate_cast<SmpLength>(file.BytesLeft());
	if(dataSize != 0xFFFFFFFF)
		LimitMax(length, dataSize);
	mptSample.nLength = (length * 8u) / (sampleIO.GetEncodedBitsPerSample() * channels);
	mptSample.nC5Speed = sampleRate;
	m_szNames[nSample] = mpt::ToCharset(GetCharsetInternal(), GetSampleNameFromTags(tags));

	if(mayNormalize)
	{
		sampleIO.MayNormalize();
	}

	sampleIO.ReadSample(mptSample, file);

	mptSample.Convert(MOD_TYPE_IT, GetType());
	mptSample.PrecomputeLoops(*this, false);
	return true;
}


/////////////////////////////////////////////////////////////////////////////////////////
// ITS Samples


bool CSoundFile::ReadITSSample(SAMPLEINDEX nSample, FileReader &file, bool rewind)
{
	if(rewind)
	{
		file.Rewind();
	}

	ITSample sampleHeader;
	if(!file.ReadStruct(sampleHeader)
		|| memcmp(sampleHeader.id, "IMPS", 4))
	{
		return false;
	}
	DestroySampleThreadsafe(nSample);

	ModSample &sample = Samples[nSample];
	file.Seek(sampleHeader.ConvertToMPT(sample));
	m_szNames[nSample] = mpt::String::ReadBuf(mpt::String::spacePaddedNull, sampleHeader.name);

	if(sample.uFlags[CHN_ADLIB])
	{
		OPLPatch patch;
		file.ReadArray(patch);
		sample.SetAdlib(true, patch);
		InitOPL();
		if(!SupportsOPL())
		{
			AddToLog(LogInformation, U_("OPL instruments are not supported by this format."));
		}
	} else if(!sample.uFlags[SMP_KEEPONDISK])
	{
		sampleHeader.GetSampleFormat().ReadSample(sample, file);
	} else
	{
		// External sample
		size_t strLen;
		file.ReadVarInt(strLen);
#ifdef MPT_EXTERNAL_SAMPLES
		std::string filenameU8;
		file.ReadString<mpt::String::maybeNullTerminated>(filenameU8, strLen);
		mpt::PathString filename = mpt::PathString::FromUTF8(filenameU8);

		if(!filename.empty())
		{
			if(file.GetOptionalFileName())
			{
				filename = mpt::RelativePathToAbsolute(filename, file.GetOptionalFileName()->GetDirectoryWithDrive());
			}
			if(!LoadExternalSample(nSample, filename))
			{
				AddToLog(LogWarning, U_("Unable to load sample: ") + filename.ToUnicode());
			}
		} else
		{
			sample.uFlags.reset(SMP_KEEPONDISK);
		}
#else
		file.Skip(strLen);
#endif // MPT_EXTERNAL_SAMPLES
	}

	sample.Convert(MOD_TYPE_IT, GetType());
	sample.PrecomputeLoops(*this, false);
	return true;
}


bool CSoundFile::ReadITISample(SAMPLEINDEX nSample, FileReader &file)
{
	ITInstrument instrumentHeader;

	file.Rewind();
	if(!file.ReadStruct(instrumentHeader)
		|| memcmp(instrumentHeader.id, "IMPI", 4))
	{
		return false;
	}
	file.Rewind();
	ModInstrument dummy;
	ITInstrToMPT(file, dummy, instrumentHeader.trkvers);
	// Old SchismTracker versions set nos=0
	const SAMPLEINDEX nsamples = std::max(static_cast<SAMPLEINDEX>(instrumentHeader.nos), *std::max_element(std::begin(dummy.Keyboard), std::end(dummy.Keyboard)));
	if(!nsamples)
		return false;

	// Preferably read the middle-C sample
	auto sample = dummy.Keyboard[NOTE_MIDDLEC - NOTE_MIN];
	if(sample > 0)
		sample--;
	else
		sample = 0;
	file.Seek(file.GetPosition() + sample * sizeof(ITSample));
	return ReadITSSample(nSample, file, false);
}


bool CSoundFile::ReadITIInstrument(INSTRUMENTINDEX nInstr, FileReader &file)
{
	ITInstrument instrumentHeader;
	SAMPLEINDEX smp = 0;

	file.Rewind();
	if(!file.ReadStruct(instrumentHeader)
		|| memcmp(instrumentHeader.id, "IMPI", 4))
	{
		return false;
	}
	if(nInstr > GetNumInstruments()) m_nInstruments = nInstr;

	ModInstrument *pIns = new (std::nothrow) ModInstrument();
	if(pIns == nullptr)
	{
		return false;
	}

	DestroyInstrument(nInstr, deleteAssociatedSamples);

	Instruments[nInstr] = pIns;
	file.Rewind();
	ITInstrToMPT(file, *pIns, instrumentHeader.trkvers);
	// Old SchismTracker versions set nos=0
	const SAMPLEINDEX nsamples = std::max(static_cast<SAMPLEINDEX>(instrumentHeader.nos), *std::max_element(std::begin(pIns->Keyboard), std::end(pIns->Keyboard)));

	// In order to properly compute the position, in file, of eventual extended settings
	// such as "attack" we need to keep the "real" size of the last sample as those extra
	// setting will follow this sample in the file
	FileReader::pos_type extraOffset = file.GetPosition();

	// Reading Samples
	std::vector<SAMPLEINDEX> samplemap(nsamples, 0);
	for(SAMPLEINDEX i = 0; i < nsamples; i++)
	{
		smp = GetNextFreeSample(nInstr, smp + 1);
		if(smp == SAMPLEINDEX_INVALID) break;
		samplemap[i] = smp;
		const FileReader::pos_type offset = file.GetPosition();
		if(!ReadITSSample(smp, file, false))
			smp--;
		extraOffset = std::max(extraOffset, file.GetPosition());
		file.Seek(offset + sizeof(ITSample));
	}
	if(GetNumSamples() < smp) m_nSamples = smp;

	// Adjust sample assignment
	for(auto &sample : pIns->Keyboard)
	{
		if(sample > 0 && sample <= nsamples)
		{
			sample = samplemap[sample - 1];
		}
	}

	if(file.Seek(extraOffset))
	{
		// Read MPT crap
		LoadExtendedInstrumentProperties(mpt::as_span(&Instruments[nInstr], 1), file);
	}

	pIns->Convert(MOD_TYPE_IT, GetType());
	pIns->Sanitize(GetType());

	return true;
}


#ifndef MODPLUG_NO_FILESAVE

bool CSoundFile::SaveITIInstrument(INSTRUMENTINDEX nInstr, std::ostream &f, const mpt::PathString &filename, bool compress, bool allowExternal) const
{
	ITInstrument iti;
	ModInstrument *pIns = Instruments[nInstr];

	if((!pIns) || (filename.empty() && allowExternal)) return false;

	auto instSize = iti.ConvertToIT(*pIns, false, *this);

	// Create sample assignment table
	std::vector<SAMPLEINDEX> smptable;
	std::vector<uint8> smpmap(GetNumSamples(), 0);
	static_assert(NOTE_MAX >= 120);
	for(size_t i = 0; i < 120; i++)
	{
		const SAMPLEINDEX smp = pIns->Keyboard[i];
		if(smp && smp <= GetNumSamples())
		{
			if(!smpmap[smp - 1])
			{
				// We haven't considered this sample yet.
				smptable.push_back(smp);
				smpmap[smp - 1] = static_cast<uint8>(smptable.size());
			}
			iti.keyboard[i * 2 + 1] = smpmap[smp - 1];
		} else
		{
			iti.keyboard[i * 2 + 1] = 0;
		}
	}
	iti.nos = static_cast<uint8>(smptable.size());
	smpmap.clear();

	uint32 filePos = instSize;
	mpt::IO::WritePartial(f, iti, instSize);

	filePos += mpt::saturate_cast<uint32>(smptable.size() * sizeof(ITSample));

	// Writing sample headers + data
	std::vector<SampleIO> sampleFlags;
	for(auto smp : smptable)
	{
		ITSample itss;
		itss.ConvertToIT(Samples[smp], GetType(), compress, compress, allowExternal);
		const bool isExternal = itss.cvt == ITSample::cvtExternalSample;

		mpt::String::WriteBuf(mpt::String::nullTerminated, itss.name) = m_szNames[smp];

		itss.samplepointer = filePos;
		mpt::IO::Write(f, itss);

		// Write sample
		auto curPos = mpt::IO::TellWrite(f);
		mpt::IO::SeekAbsolute(f, filePos);
		if(!isExternal)
		{
			filePos += mpt::saturate_cast<uint32>(itss.GetSampleFormat(0x0214).WriteSample(f, Samples[smp]));
		} else
		{
#ifdef MPT_EXTERNAL_SAMPLES
			const std::string filenameU8 = mpt::AbsolutePathToRelative(GetSamplePath(smp), filename.GetDirectoryWithDrive()).ToUTF8();
			const size_t strSize = filenameU8.size();
			size_t intBytes = 0;
			if(mpt::IO::WriteVarInt(f, strSize, &intBytes))
			{
				filePos += mpt::saturate_cast<uint32>(intBytes + strSize);
				mpt::IO::WriteRaw(f, filenameU8.data(), strSize);
			}
#endif // MPT_EXTERNAL_SAMPLES
		}
		mpt::IO::SeekAbsolute(f, curPos);
	}

	mpt::IO::SeekEnd(f);
	SaveExtendedInstrumentProperties(nInstr, MOD_TYPE_MPT, f);

	return true;
}

#endif // MODPLUG_NO_FILESAVE


///////////////////////////////////////////////////////////////////////////////////////////////////
// 8SVX / 16SVX / MAUD Samples

// IFF File Header
struct IFFHeader
{
	char     form[4];   // "FORM"
	uint32be size;
	char     magic[4];  // "8SVX", "16SV", "MAUD"
};

MPT_BINARY_STRUCT(IFFHeader, 12)


// General IFF Chunk header
struct IFFChunk
{
	// 32-Bit chunk identifiers
	enum ChunkIdentifiers
	{
		// 8SVX / 16SV
		idVHDR = MagicBE("VHDR"),
		idBODY = MagicBE("BODY"),
		idCHAN = MagicBE("CHAN"),

		// MAUD
		idMHDR = MagicBE("MHDR"),
		idMDAT = MagicBE("MDAT"),

		idNAME = MagicBE("NAME"),
		idANNO = MagicBE("ANNO"),
	};

	uint32be id;      // See ChunkIdentifiers
	uint32be length;  // Chunk size without header

	size_t GetLength() const
	{
		if(length == 0 && id == idBODY)  // Broken files
			return std::numeric_limits<size_t>::max();
		return length;
	}

	ChunkIdentifiers GetID() const
	{
		return static_cast<ChunkIdentifiers>(id.get());
	}
};

MPT_BINARY_STRUCT(IFFChunk, 8)


struct IFFSampleHeader
{
	uint32be oneShotHiSamples;   // Samples in the high octave 1-shot part
	uint32be repeatHiSamples;    // Samples in the high octave repeat part
	uint32be samplesPerHiCycle;  // Samples/cycle in high octave, else 0
	uint16be samplesPerSec;      // Data sampling rate
	uint8be  octave;             // Octaves of waveforms
	uint8be  compression;        // Data compression technique used
	uint32be volume;
};

MPT_BINARY_STRUCT(IFFSampleHeader, 20)


bool CSoundFile::ReadIFFSample(SAMPLEINDEX nSample, FileReader &file, bool allowLittleEndian, uint8 octave)
{
	file.Rewind();

	IFFHeader fileHeader;
	if(!file.ReadStruct(fileHeader)
	   || memcmp(fileHeader.form, "FORM", 4)
	   || (memcmp(fileHeader.magic, "8SVX", 4) && memcmp(fileHeader.magic, "16SV", 4) && memcmp(fileHeader.magic, "MAUD", 4)))
	{
		return false;
	}

	const auto chunks = file.ReadChunks<IFFChunk>(2);
	FileReader sampleData;

	SampleIO sampleIO(SampleIO::_8bit, SampleIO::mono, SampleIO::bigEndian, SampleIO::signedPCM);
	uint32 numSamples = 0, sampleRate = 0, loopStart = 0, loopLength = 0, volume = 0;

	if(!memcmp(fileHeader.magic, "MAUD", 4))
	{
		FileReader mhdrChunk = chunks.GetChunk(IFFChunk::idMHDR);
		sampleData = chunks.GetChunk(IFFChunk::idMDAT);
		if(!mhdrChunk.LengthIs(32)
		   || !sampleData.IsValid())
		{
			return false;
		}

		numSamples = mhdrChunk.ReadUint32BE();
		const uint16 bitsPerSample = mhdrChunk.ReadUint16BE();
		mhdrChunk.Skip(2);  // bits per sample after decompression
		sampleRate = mhdrChunk.ReadUint32BE();
		const auto [clockDivide, channelInformation, numChannels, compressionType] = mhdrChunk.ReadArray<uint16be, 4>();
		if(!clockDivide)
			return false;
		else
			sampleRate /= clockDivide;

		if(numChannels != (channelInformation + 1))
			return false;
		if(numChannels == 2)
			sampleIO |= SampleIO::stereoInterleaved;

		if(bitsPerSample == 8 && compressionType == 0)
			sampleIO |= SampleIO::unsignedPCM;
		else if(bitsPerSample == 8 && compressionType == 2)
			sampleIO |= SampleIO::aLaw;
		else if(bitsPerSample == 8 && compressionType == 3)
			sampleIO |= SampleIO::uLaw;
		else if(bitsPerSample == 16 && compressionType == 0)
			sampleIO |= SampleIO::_16bit;
		else
			return false;
	} else
	{
		FileReader vhdrChunk = chunks.GetChunk(IFFChunk::idVHDR);
		FileReader chanChunk = chunks.GetChunk(IFFChunk::idCHAN);
		sampleData           = chunks.GetChunk(IFFChunk::idBODY);
		IFFSampleHeader sampleHeader;
		if(!sampleData.IsValid()
			|| !vhdrChunk.IsValid()
			|| !vhdrChunk.ReadStruct(sampleHeader))
		{
			return false;
		}

		const uint8 bytesPerSample = memcmp(fileHeader.magic, "8SVX", 4) ? 2 : 1;
		const uint8 numChannels    = chanChunk.ReadUint32BE() == 6 ? 2 : 1;
		const uint8 bytesPerFrame  = bytesPerSample * numChannels;

		if(bytesPerSample == 2)
			sampleIO |= SampleIO::_16bit;
		if(numChannels == 2)
			sampleIO |= SampleIO::stereoSplit;

		loopStart  = sampleHeader.oneShotHiSamples / bytesPerFrame;
		// Loops are a complicated mess in IFF samples.
		// Some samples (e.g. those from the Ensoniq Mirage for Amiga collection available at https://archive.org/details/mirage4amiga)
		// have a repeatHiSamples portion that includes garbage. However, these samples also have an appropriate samplesPerHiCycle value set
		// which indicates the length of one cycle of the repeating waveform. If we just take that one cycle into account, the samples loop cleanly.
		// However, some other, non-musical 8SVX samples use bogus samplesPerHiCycle values. The following conditions help us spot this sort of samples:
		// - If samplesPerHiCycle is 32 or lower, we simply ignore it.
		// - According to the documentation, repeatHiSamples is intended to be a multiple of samplesPerHiCycle if the latter is set (otherwise we wouldn't get a clean loop).
		//   So if this is not the case, we ignore samplesPerHiCycle and only use repeatHiSamples.
		if(sampleHeader.samplesPerHiCycle > 32 && sampleHeader.samplesPerHiCycle < sampleHeader.repeatHiSamples && (sampleHeader.repeatHiSamples % sampleHeader.samplesPerHiCycle) == 0)
			loopLength = sampleHeader.samplesPerHiCycle / bytesPerFrame;
		else
			loopLength = sampleHeader.repeatHiSamples / bytesPerFrame;
		sampleRate = sampleHeader.samplesPerSec;
		volume     = sampleHeader.volume;
		numSamples = mpt::saturate_cast<SmpLength>(sampleData.GetLength() / bytesPerFrame);

		if(octave < sampleHeader.octave)
		{
			numSamples = sampleHeader.oneShotHiSamples + sampleHeader.repeatHiSamples;
			for(uint8 o = 0; o < octave; o++)
			{
				sampleData.Skip(numSamples * bytesPerSample * numChannels);
				numSamples *= 2;
				loopStart *= 2;
				loopLength *= 2;
			}
		}
	}

	DestroySampleThreadsafe(nSample);
	ModSample &sample = Samples[nSample];
	sample.Initialize();
	sample.nLength    = numSamples;
	sample.nLoopStart = loopStart;
	sample.nLoopEnd   = sample.nLoopStart + loopLength;
	if((sample.nLoopStart + 4 < sample.nLoopEnd) && (sample.nLoopEnd <= sample.nLength))
		sample.uFlags.set(CHN_LOOP);

	sample.nC5Speed = sampleRate;
	if(sample.nC5Speed <= 1)
		sample.nC5Speed = 22050;

	sample.nVolume = static_cast<uint16>((volume + 128) / 256);
	if(!sample.nVolume || sample.nVolume > 256)
		sample.nVolume = 256;

	sample.Convert(MOD_TYPE_IT, GetType());

	FileReader nameChunk = chunks.GetChunk(IFFChunk::idNAME);
	if(nameChunk.IsValid())
		nameChunk.ReadString<mpt::String::maybeNullTerminated>(m_szNames[nSample], mpt::saturate_cast<std::size_t>(nameChunk.GetLength()));
	else
		m_szNames[nSample] = "";

	sampleIO.ReadSample(sample, sampleData);

	if(allowLittleEndian && !memcmp(fileHeader.magic, "16SV", 4))
	{
		// Fasttracker 2 (and also Awave Studio up to version 11.7) writes little-endian 16-bit data. Great...
		// It is relatively safe to assume (see raw sample import dialog) that "proper" sample data usually has some smoothness to it,
		// i.e. its first derivative mostly consists of small-ish values.
		// When interpreting the sample data with incorrect endianness, however, the first derivative is usually a lot more jumpy.
		// So we compare the two derivatives when interpreting the data as little-endian and big-endian,
		// and choose the waveform that has less jumps in it.
		// Sample data is normalized for those comparisons, otherwise 8-bit data converted to 16-bit will almost always be more optimal
		// when byte-swapped (as the upper byte is always 0).
		const uint8 numChannels = sample.GetNumChannels();
		auto sample16 = mpt::as_span(sample.sample16(), sample.nLength * numChannels);
		int32 minNative = int16_max, maxNative = int16_min, minSwapped = int16_max, maxSwapped = int16_min;
		for(const auto vNative : sample16)
		{
			const int16 vSwapped = mpt::byteswap(vNative);
			if(vNative < minNative)
				minNative = vNative;
			if(vNative > maxNative)
				maxNative = vNative;
			if(vSwapped < minSwapped)
				minSwapped = vSwapped;
			if(vSwapped > maxSwapped)
				maxSwapped = vSwapped;
		}

		const int32 minMaxNative = std::max({int32(1), -minNative, maxNative});
		const int32 minMaxSwapped = std::max({int32(1), -minSwapped, maxSwapped});
		const double factorNative = 1.0 / minMaxNative, factorSwapped = 1.0 / minMaxSwapped;
		double errorNative = 0.0, errorSwapped = 0.0;
		for(uint8 chn = 0; chn < numChannels; chn++)
		{
			const int16 *vNative = sample.sample16() + chn;
			int32 prev = 0;
			for(SmpLength i = sample.nLength; i != 0; i--, vNative += numChannels)
			{
				const double diffNative = (*vNative - prev) * factorNative;
				errorNative += diffNative * diffNative;
				const double diffSwapped = (mpt::byteswap(*vNative) - mpt::byteswap(static_cast<int16>(prev))) * factorSwapped;
				errorSwapped += diffSwapped * diffSwapped;
				prev = *vNative;
			}
		}
		if(errorNative > errorSwapped)
		{
			for(auto &v : sample16)
			{
				v = mpt::byteswap(v);
			}
		}
	}

	sample.PrecomputeLoops(*this, false);

	return true;
}


#ifndef MODPLUG_NO_FILESAVE

static uint32 WriteIFFStringChunk(std::ostream &f, IFFChunk::ChunkIdentifiers id, const std::string &str)
{
	if(str.empty())
		return 0;
	IFFChunk chunk{};
	chunk.id = id;
	chunk.length = static_cast<uint32>(str.size());
	mpt::IO::Write(f, chunk);
	mpt::IO::WriteText(f, str);
	uint32 totalSize = sizeof(IFFChunk) + chunk.length;
	if(totalSize % 2u)
	{
		mpt::IO::WriteIntBE<uint8>(f, 0);
		totalSize++;
	}
	return totalSize;
}


bool CSoundFile::SaveIFFSample(SAMPLEINDEX smp, std::ostream &f) const
{
	const ModSample &sample = Samples[smp];
	if(sample.uFlags[CHN_ADLIB] || !sample.HasSampleData())
		return false;

	mpt::IO::OFile<std::ostream> ff(f);
	IFFHeader fileHeader{};
	memcpy(fileHeader.form, "FORM", 4);
	if(sample.uFlags[CHN_16BIT])
		memcpy(fileHeader.magic, "16SV", 4);
	else
		memcpy(fileHeader.magic, "8SVX", 4);
	mpt::IO::Write(f, fileHeader);

	uint32 totalSize = 4 + sizeof(IFFChunk) + sizeof(IFFSampleHeader);

	IFFChunk chunk{};
	chunk.id = IFFChunk::idVHDR;
	chunk.length = sizeof(IFFSampleHeader);
	mpt::IO::Write(f, chunk);

	IFFSampleHeader sampleHeader{};
	uint32 loopStart = sample.nLength, loopEnd = sample.nLength;
	if(sample.uFlags[CHN_LOOP])
	{
		loopStart = sample.nLoopStart;
		loopEnd = sample.nLoopEnd;
	} else if(sample.uFlags[CHN_SUSTAINLOOP])
	{
		loopStart = sample.nSustainStart;
		loopEnd = sample.nSustainEnd;
	}
	const uint8 bps = sample.GetBytesPerSample();
	sampleHeader.oneShotHiSamples = loopStart * bps;
	sampleHeader.repeatHiSamples = (loopEnd - loopStart) * bps;

	sampleHeader.samplesPerHiCycle = 0;
	sampleHeader.samplesPerSec = mpt::saturate_cast<uint16>(sample.GetSampleRate(m_nType));
	sampleHeader.octave = 1;
	sampleHeader.compression = 0;
	sampleHeader.volume = mpt::saturate_cast<uint16>(sample.nVolume * 256u);
	mpt::IO::Write(f, sampleHeader);

	if(sample.uFlags[CHN_STEREO])
	{
		chunk.id = IFFChunk::idCHAN;
		chunk.length = 4;
		mpt::IO::Write(f, chunk);
		mpt::IO::WriteIntBE<uint32>(f, 6);
		totalSize += mpt::saturate_cast<uint32>(sizeof(chunk) + chunk.length);
	}

	totalSize += WriteIFFStringChunk(f, IFFChunk::idNAME, mpt::ToCharset(mpt::Charset::Amiga, GetCharsetInternal(), m_szNames[smp]));
	totalSize += WriteIFFStringChunk(f, IFFChunk::idANNO, mpt::ToCharset(mpt::Charset::Amiga, Version::Current().GetOpenMPTVersionString()));

	SampleIO sampleIO(
		sample.uFlags[CHN_16BIT] ? SampleIO::_16bit : SampleIO::_8bit,
		sample.uFlags[CHN_STEREO] ? SampleIO::stereoSplit : SampleIO::mono,
		SampleIO::bigEndian,
		SampleIO::signedPCM);

	chunk.id = IFFChunk::idBODY;
	chunk.length = mpt::saturate_cast<uint32>(sampleIO.CalculateEncodedSize(sample.nLength));
	mpt::IO::Write(f, chunk);
	sampleIO.WriteSample(f, sample);
	totalSize += mpt::saturate_cast<uint32>(sizeof(chunk) + chunk.length);
	if(totalSize % 2u)
	{
		mpt::IO::WriteIntBE<uint8>(f, 0);
		totalSize++;
	}

	fileHeader.size = totalSize;
	mpt::IO::SeekAbsolute(f, 0);
	mpt::IO::Write(f, fileHeader);

	return true;
}

#endif  // MODPLUG_NO_FILESAVE


OPENMPT_NAMESPACE_END
