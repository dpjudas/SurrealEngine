/*
 * SampleFormatMediaSoundation.cpp
 * -------------------------------
 * Purpose: MediaFoundation sample import.
 * Notes  :
 * Authors: Joern Heusipp
 *          OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"
#include "Sndfile.h"
#ifndef MODPLUG_NO_FILESAVE
#include "../common/mptFileIO.h"
#endif
#include "../common/misc_util.h"
#include "Tagging.h"
#include "Loaders.h"
#include "../common/FileReader.h"
#include "modsmp_ctrl.h"
#include "openmpt/soundbase/Copy.hpp"
#include "../soundlib/ModSampleCopy.h"
#include "../common/ComponentManager.h"
#if defined(MPT_WITH_MEDIAFOUNDATION)
#include "mpt/io_file_adapter/fileadapter.hpp"
#include "../common/FileReader.h"
#include "../common/mptFileTemporary.h"
#include <windows.h>
#include <atlbase.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <mferror.h>
#include <Propvarutil.h>
#endif // MPT_WITH_MEDIAFOUNDATION
#include "mpt/string/utility.hpp"


OPENMPT_NAMESPACE_BEGIN


#if defined(MPT_WITH_MEDIAFOUNDATION)

struct PropVariant : PROPVARIANT
{
	PropVariant() { PropVariantInit(this); }
	~PropVariant() { PropVariantClear(this); }
};

// Implementing IMFByteStream is apparently not enough to stream raw bytes
// data to MediaFoundation.
// Additionally, one has to also implement a custom IMFAsyncResult for the
// BeginRead/EndRead interface which allows transferring the number of read
// bytes around.
// To make things even worse, MediaFoundation fails to detect some AAC and MPEG
// files if a non-file-based or read-only stream is used for opening.
// The only sane option which remains if we do not have an on-disk filename
// available:
//  1 - write a temporary file
//  2 - close it
//  3 - open it using MediaFoundation.
// We use FILE_ATTRIBUTE_TEMPORARY which will try to keep the file data in
// memory just like regular allocated memory and reduce the overhead basically
// to memcpy.

static FileTags ReadMFMetadata(IMFMediaSource *mediaSource)
{

	FileTags tags;

	CComPtr<IMFPresentationDescriptor> presentationDescriptor;
	if(!SUCCEEDED(mediaSource->CreatePresentationDescriptor(&presentationDescriptor)))
	{
		return tags;
	}
	DWORD streams = 0;
	if(!SUCCEEDED(presentationDescriptor->GetStreamDescriptorCount(&streams)))
	{
		return tags;
	}
	CComPtr<IMFMetadataProvider> metadataProvider;
	if(!SUCCEEDED(MFGetService(mediaSource, MF_METADATA_PROVIDER_SERVICE, IID_IMFMetadataProvider, (void**)&metadataProvider)))
	{
		return tags;
	}
	CComPtr<IMFMetadata> metadata;
	if(!SUCCEEDED(metadataProvider->GetMFMetadata(presentationDescriptor, 0, 0, &metadata)))
	{
		return tags;
	}

	PropVariant varPropNames;
	if(!SUCCEEDED(metadata->GetAllPropertyNames(&varPropNames)))
	{
		return tags;
	}
	for(DWORD propIndex = 0; propIndex < varPropNames.calpwstr.cElems; ++propIndex)
	{
		PropVariant propVal;
		LPWSTR propName = varPropNames.calpwstr.pElems[propIndex];
		if(S_OK != metadata->GetProperty(propName, &propVal))
		{
			break;
		}
		std::wstring stringVal;
#if !MPT_OS_WINDOWS_WINRT
		// WTF, no PropVariantToString() in WinRT 
		std::vector<WCHAR> wcharVal(256);
		for(;;)
		{
			HRESULT hrToString = PropVariantToString(propVal, wcharVal.data(), mpt::saturate_cast<UINT>(wcharVal.size()));
			if(hrToString == S_OK)
			{
				stringVal = wcharVal.data();
				break;
			} else if(hrToString == ERROR_INSUFFICIENT_BUFFER)
			{
				wcharVal.resize(mpt::exponential_grow(wcharVal.size()));
			} else
			{
				break;
			}
		}
#endif // !MPT_OS_WINDOWS_WINRT
		if(stringVal.length() > 0)
		{
			if(propName == std::wstring(L"Author")) tags.artist = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"Title")) tags.title = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"WM/AlbumTitle")) tags.album = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"WM/Track")) tags.trackno = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"WM/Year")) tags.year = mpt::ToUnicode(stringVal);
			if(propName == std::wstring(L"WM/Genre")) tags.genre = mpt::ToUnicode(stringVal);
		}
	}

	return tags;

}


class ComponentMediaFoundation : public ComponentLibrary
{
	MPT_DECLARE_COMPONENT_MEMBERS(ComponentMediaFoundation, "MediaFoundation")
public:
	ComponentMediaFoundation()
		: ComponentLibrary(ComponentTypeSystem)
	{
		return;
	}
	bool DoInitialize() override
	{
#if !MPT_OS_WINDOWS_WINRT
		if(!(true
			&& AddLibrary("mf", mpt::LibraryPath::System(P_("mf")))
			&& AddLibrary("mfplat", mpt::LibraryPath::System(P_("mfplat")))
			&& AddLibrary("mfreadwrite", mpt::LibraryPath::System(P_("mfreadwrite")))
			&& AddLibrary("propsys", mpt::LibraryPath::System(P_("propsys")))
			))
		{
			return false;
		}
#endif // !MPT_OS_WINDOWS_WINRT
		if(!SUCCEEDED(MFStartup(MF_VERSION)))
		{
			return false;
		}
		return true;
	}
	virtual ~ComponentMediaFoundation()
	{
		if(IsAvailable())
		{
			MFShutdown();
		}
	}
};

#endif // MPT_WITH_MEDIAFOUNDATION


#ifdef MODPLUG_TRACKER
std::vector<FileType> CSoundFile::GetMediaFoundationFileTypes()
{
	std::vector<FileType> result;

#if defined(MPT_WITH_MEDIAFOUNDATION)

	ComponentHandle<ComponentMediaFoundation> mf;
	if(!IsComponentAvailable(mf))
	{
		return result;
	}

	std::map<std::wstring, FileType> guidMap;

	HKEY hkHandlers = NULL;
	LSTATUS regResult = RegOpenKeyExW(HKEY_LOCAL_MACHINE, L"SOFTWARE\\Microsoft\\Windows Media Foundation\\ByteStreamHandlers", 0, KEY_READ, &hkHandlers);
	if(regResult != ERROR_SUCCESS)
	{
		return result;
	}

	for(DWORD handlerIndex = 0; ; ++handlerIndex)
	{

		WCHAR handlerTypeBuf[256];
		MemsetZero(handlerTypeBuf);
		regResult = RegEnumKeyW(hkHandlers, handlerIndex, handlerTypeBuf, 256);
		if(regResult != ERROR_SUCCESS)
		{
			break;
		}

		std::wstring handlerType = handlerTypeBuf;

		if(handlerType.length() < 1)
		{
			continue;
		}

		HKEY hkHandler = NULL;
		regResult = RegOpenKeyExW(hkHandlers, handlerTypeBuf, 0, KEY_READ, &hkHandler);
		if(regResult != ERROR_SUCCESS)
		{
			continue;
		}

		std::vector<WCHAR> valueNameBuf(16384);
		std::vector<BYTE> valueData(16384);
		for(DWORD valueIndex = 0; ; ++valueIndex)
		{
			std::fill(valueNameBuf.begin(), valueNameBuf.end(), WCHAR{0});
			DWORD valueNameBufLen = 16384;
			DWORD valueType = 0;
			std::fill(valueData.begin(), valueData.end(), BYTE{0});
			DWORD valueDataLen = 16384;
			regResult = RegEnumValueW(hkHandler, valueIndex, valueNameBuf.data(), &valueNameBufLen, NULL, &valueType, valueData.data(), &valueDataLen);
			if(regResult != ERROR_SUCCESS)
			{
				break;
			}
			if(valueNameBufLen <= 0 || valueType != REG_SZ || valueDataLen <= 0)
			{
				continue;
			}

			std::wstring guid = std::wstring(valueNameBuf.data());

			mpt::ustring description = mpt::ToUnicode(ParseMaybeNullTerminatedStringFromBufferWithSizeInBytes<std::wstring>(valueData.data(), valueDataLen));
			description = mpt::replace(description, U_("Byte Stream Handler"), U_("Files"));
			description = mpt::replace(description, U_("ByteStreamHandler"), U_("Files"));

			guidMap[guid]
				.ShortName(U_("mf"))
				.Description(description)
				;

			if(handlerType[0] == L'.')
			{
				guidMap[guid].AddExtension(mpt::PathString::FromWide(handlerType.substr(1)));
			} else
			{
				guidMap[guid].AddMimeType(mpt::ToCharset(mpt::Charset::ASCII, handlerType));
			}

		}

		RegCloseKey(hkHandler);
		hkHandler = NULL;

	}

	RegCloseKey(hkHandlers);
	hkHandlers = NULL;

	for(const auto &it : guidMap)
	{
		result.push_back(it.second);
	}

#endif // MPT_WITH_MEDIAFOUNDATION

	return result;
}
#endif // MODPLUG_TRACKER


bool CSoundFile::ReadMediaFoundationSample(SAMPLEINDEX sample, FileReader &file, bool mo3Decode)
{

#if !defined(MPT_WITH_MEDIAFOUNDATION)

	MPT_UNREFERENCED_PARAMETER(sample);
	MPT_UNREFERENCED_PARAMETER(file);
	MPT_UNREFERENCED_PARAMETER(mo3Decode);
	return false;

#else

	ComponentHandle<ComponentMediaFoundation> mf;
	if(!IsComponentAvailable(mf))
	{
		return false;
	}

	file.Rewind();
	// When using MF to decode MP3 samples in MO3 files, we need the mp3 file extension
	// for some of them or otherwise MF refuses to recognize them.
	mpt::PathString tmpfileExtension = (mo3Decode ? P_("mp3") : P_("tmp"));
	mpt::IO::FileAdapter<FileCursor> diskfile(file, mpt::TemporaryPathname{tmpfileExtension}.GetPathname());
	if(!diskfile.IsValid())
	{
		return false;
	}

	#define MPT_MF_CHECKED(x) do { \
		HRESULT hr = (x); \
		if(!SUCCEEDED(hr)) \
		{ \
			return false; \
		} \
	} while(0)

	CComPtr<IMFSourceResolver> sourceResolver;
	MPT_MF_CHECKED(MFCreateSourceResolver(&sourceResolver));
	MF_OBJECT_TYPE objectType = MF_OBJECT_INVALID;
	CComPtr<IUnknown> unknownMediaSource;
	MPT_MF_CHECKED(sourceResolver->CreateObjectFromURL(mpt::ToWide(diskfile.GetFilename()).c_str(), MF_RESOLUTION_MEDIASOURCE | MF_RESOLUTION_CONTENT_DOES_NOT_HAVE_TO_MATCH_EXTENSION_OR_MIME_TYPE | MF_RESOLUTION_READ, NULL, &objectType, &unknownMediaSource));
	if(objectType != MF_OBJECT_MEDIASOURCE)
	{
		return false;
	}
	CComPtr<IMFMediaSource> mediaSource;
	MPT_MF_CHECKED(unknownMediaSource->QueryInterface(&mediaSource));

	FileTags tags = ReadMFMetadata(mediaSource);

	CComPtr<IMFSourceReader> sourceReader;
	MPT_MF_CHECKED(MFCreateSourceReaderFromMediaSource(mediaSource, NULL, &sourceReader));
	CComPtr<IMFMediaType> partialType;
	MPT_MF_CHECKED(MFCreateMediaType(&partialType));
	MPT_MF_CHECKED(partialType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio));
	MPT_MF_CHECKED(partialType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM));
	MPT_MF_CHECKED(sourceReader->SetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, NULL, partialType));
	CComPtr<IMFMediaType> uncompressedAudioType;
	MPT_MF_CHECKED(sourceReader->GetCurrentMediaType((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, &uncompressedAudioType));
	MPT_MF_CHECKED(sourceReader->SetStreamSelection((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE));
	UINT32 numChannels = 0;
	MPT_MF_CHECKED(uncompressedAudioType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &numChannels));
	UINT32 samplesPerSecond = 0;
	MPT_MF_CHECKED(uncompressedAudioType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &samplesPerSecond));
	UINT32 bitsPerSample = 0;
	MPT_MF_CHECKED(uncompressedAudioType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample));
	if(numChannels <= 0 || numChannels > 2)
	{
		return false;
	}
	if(samplesPerSecond <= 0)
	{
		return false;
	}
	if(bitsPerSample != 8 && bitsPerSample != 16 && bitsPerSample != 24 && bitsPerSample != 32)
	{
		return false;
	}

	std::vector<char> rawData;
	for(;;)
	{
		CComPtr<IMFSample> mfSample;
		DWORD mfSampleFlags = 0;
		CComPtr<IMFMediaBuffer> buffer;
		MPT_MF_CHECKED(sourceReader->ReadSample((DWORD)MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, NULL, &mfSampleFlags, NULL, &mfSample));
		if(mfSampleFlags & MF_SOURCE_READERF_CURRENTMEDIATYPECHANGED)
		{
			break;
		}
		if(mfSampleFlags & MF_SOURCE_READERF_ENDOFSTREAM)
		{
			break;
		}
		MPT_MF_CHECKED(mfSample->ConvertToContiguousBuffer(&buffer));
		{
			BYTE *data = NULL;
			DWORD dataSize = 0;
			MPT_MF_CHECKED(buffer->Lock(&data, NULL, &dataSize));
			mpt::append(rawData, mpt::byte_cast<char*>(data), mpt::byte_cast<char*>(data + dataSize));
			MPT_MF_CHECKED(buffer->Unlock());
			if(rawData.size() / numChannels / (bitsPerSample / 8) > MAX_SAMPLE_LENGTH)
			{
				break;
			}
		}
	}

	std::string sampleName = mpt::ToCharset(GetCharsetInternal(), GetSampleNameFromTags(tags));

	const size_t length = rawData.size() / numChannels / (bitsPerSample / 8);
	if(length < 1 || length > MAX_SAMPLE_LENGTH)
	{
		return false;
	}

	DestroySampleThreadsafe(sample);
	if(!mo3Decode)
	{
		Samples[sample].Initialize();
		Samples[sample].nC5Speed = samplesPerSecond;
		m_szNames[sample] = sampleName;
	}
	Samples[sample].nLength = static_cast<SmpLength>(length);
	Samples[sample].uFlags.set(CHN_16BIT, bitsPerSample >= 16);
	Samples[sample].uFlags.set(CHN_STEREO, numChannels == 2);
	Samples[sample].AllocateSample();
	if(!Samples[sample].HasSampleData())
	{
		return false;
	}

	if(bitsPerSample == 24)
	{
		if(numChannels == 2)
		{
			CopyStereoInterleavedSample<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt24<0, littleEndian24>>>(Samples[sample], rawData.data(), rawData.size());
		} else
		{
			CopyMonoSample<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt24<0, littleEndian24>>>(Samples[sample], rawData.data(), rawData.size());
		}
	} else if(bitsPerSample == 32)
	{
		if(numChannels == 2)
		{
			CopyStereoInterleavedSample<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt32<0, littleEndian32>>>(Samples[sample], rawData.data(), rawData.size());
		} else
		{
			CopyMonoSample<SC::ConversionChain<SC::Convert<int16, int32>, SC::DecodeInt32<0, littleEndian32>>>(Samples[sample], rawData.data(), rawData.size());
		}
	} else
	{
		// just copy
		std::copy(rawData.begin(), rawData.end(), mpt::byte_cast<char *>(Samples[sample].sampleb()));
	}

	#undef MPT_MF_CHECKED

	if(!mo3Decode)
	{
		Samples[sample].Convert(MOD_TYPE_IT, GetType());
		Samples[sample].PrecomputeLoops(*this, false);
	}

	return true;

#endif

}


OPENMPT_NAMESPACE_END
