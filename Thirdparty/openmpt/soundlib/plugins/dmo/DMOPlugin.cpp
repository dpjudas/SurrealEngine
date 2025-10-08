/*
 * DMOPlugin.h
 * -----------
 * Purpose: DirectX Media Object plugin handling / processing.
 * Notes  : Some default plugins only have the same output characteristics in the floating point code path (compared to integer PCM)
 *          if we feed them input in the range [-32768, +32768] rather than the more usual [-1, +1].
 *          Hence, OpenMPT uses this range for both the floating-point and integer path.
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#include "mpt/base/aligned_array.hpp"
#if defined(MPT_WITH_DMO)
#include "DMOPlugin.h"
#include "mpt/uuid/guid.hpp"
#include "../../Sndfile.h"
#include "../PluginManager.h"
#include <uuids.h>
#include <medparam.h>
#include <mmsystem.h>
#endif // MPT_WITH_DMO

OPENMPT_NAMESPACE_BEGIN


#if defined(MPT_WITH_DMO)


#ifdef MPT_ALL_LOGGING
#define DMO_LOG
#else
#define DMO_LOG
#endif


IMixPlugin* DMOPlugin::Create(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct)
{
	CLSID clsid;
	if(mpt::VerifyStringToCLSID(factory.dllPath.AsNative(), clsid))
	{
		IMediaObject *pMO = nullptr;
		IMediaObjectInPlace *pMOIP = nullptr;
		if ((CoCreateInstance(clsid, nullptr, CLSCTX_INPROC_SERVER, IID_IMediaObject, (VOID **)&pMO) == S_OK) && (pMO))
		{
			if (pMO->QueryInterface(IID_IMediaObjectInPlace, (void **)&pMOIP) != S_OK) pMOIP = nullptr;
		} else pMO = nullptr;
		if ((pMO) && (pMOIP))
		{
			DWORD dwInputs = 0, dwOutputs = 0;
			pMO->GetStreamCount(&dwInputs, &dwOutputs);
			if (dwInputs == 1 && dwOutputs == 1)
			{
				DMOPlugin *p = new (std::nothrow) DMOPlugin(factory, sndFile, mixStruct, pMO, pMOIP, clsid.Data1);
				return p;
			}
#ifdef DMO_LOG
			MPT_LOG_GLOBAL(LogDebug, "DMO", factory.libraryName.ToUnicode() + U_(": Unable to use this DMO"));
#endif
		}
#ifdef DMO_LOG
		else MPT_LOG_GLOBAL(LogDebug, "DMO", factory.libraryName.ToUnicode() + U_(": Failed to get IMediaObject & IMediaObjectInPlace interfaces"));
#endif
		if (pMO) pMO->Release();
		if (pMOIP) pMOIP->Release();
	}
	return nullptr;
}


DMOPlugin::DMOPlugin(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct, IMediaObject *pMO, IMediaObjectInPlace *pMOIP, uint32 uid)
	: IMixPlugin(factory, sndFile, mixStruct)
	, m_pMediaObject(pMO)
	, m_pMediaProcess(pMOIP)
	, m_pParamInfo(nullptr)
	, m_pMediaParams(nullptr)
	, m_nSamplesPerSec(sndFile.GetSampleRate())
	, m_uid(uid)
{
	if(FAILED(m_pMediaObject->QueryInterface(IID_IMediaParamInfo, (void **)&m_pParamInfo)))
		m_pParamInfo = nullptr;
	if (FAILED(m_pMediaObject->QueryInterface(IID_IMediaParams, (void **)&m_pMediaParams)))
		m_pMediaParams = nullptr;
	m_alignedBuffer.f32 = mpt::align_bytes<16, MIXBUFFERSIZE * 2>(m_interleavedBuffer.f32);
	m_mixBuffer.Initialize(2, 2);
}


DMOPlugin::~DMOPlugin()
{
	if(m_pMediaParams)
	{
		m_pMediaParams->Release();
		m_pMediaParams = nullptr;
	}
	if(m_pParamInfo)
	{
		m_pParamInfo->Release();
		m_pParamInfo = nullptr;
	}
	if(m_pMediaProcess)
	{
		m_pMediaProcess->Release();
		m_pMediaProcess = nullptr;
	}
	if(m_pMediaObject)
	{
		m_pMediaObject->Release();
		m_pMediaObject = nullptr;
	}
}


uint32 DMOPlugin::GetLatency() const
{
	REFERENCE_TIME time;	// Unit 100-nanoseconds
	if(m_pMediaProcess->GetLatency(&time) == S_OK)
	{
		return static_cast<uint32>(time * m_nSamplesPerSec / (10 * 1000 * 1000));
	}
	return 0;
}


static constexpr float _f2si = 32768.0f;
static constexpr float _si2f = 1.0f / 32768.0f;


static void InterleaveStereo(const float * MPT_RESTRICT inputL, const float * MPT_RESTRICT inputR, float * MPT_RESTRICT output, uint32 numFrames)
{
	while(numFrames--)
	{
		*(output++) = *(inputL++) * _f2si;
		*(output++) = *(inputR++) * _f2si;
	}
}


static void DeinterleaveStereo(const float * MPT_RESTRICT input, float * MPT_RESTRICT outputL, float * MPT_RESTRICT outputR, uint32 numFrames)
{
	while(numFrames--)
	{
		*(outputL++) = *(input++) * _si2f;
		*(outputR++) = *(input++) * _si2f;
	}
}


// Interleave two float streams into one int16 stereo stream.
static void InterleaveFloatToInt16(const float * MPT_RESTRICT inputL, const float * MPT_RESTRICT inputR, int16 * MPT_RESTRICT output, uint32 numFrames)
{
	while(numFrames--)
	{
		*(output++) = static_cast<int16>(Clamp(*(inputL++) * _f2si, static_cast<float>(int16_min), static_cast<float>(int16_max)));
		*(output++) = static_cast<int16>(Clamp(*(inputR++) * _f2si, static_cast<float>(int16_min), static_cast<float>(int16_max)));
	}
}


// Deinterleave an int16 stereo stream into two float streams.
static void DeinterleaveInt16ToFloat(const int16 * MPT_RESTRICT input, float * MPT_RESTRICT outputL, float * MPT_RESTRICT outputR, uint32 numFrames)
{
	while(numFrames--)
	{
		*outputL++ += _si2f * static_cast<float>(*input++);
		*outputR++ += _si2f * static_cast<float>(*input++);
	}
}


void DMOPlugin::Process(float *pOutL, float *pOutR, uint32 numFrames)
{
	if(!numFrames || !m_mixBuffer.Ok())
		return;
	m_mixBuffer.ClearOutputBuffers(numFrames);
	REFERENCE_TIME startTime = Util::muldiv(m_SndFile.GetTotalSampleCount(), 10000000, m_nSamplesPerSec);
	
	if(m_useFloat)
	{
		InterleaveStereo(m_mixBuffer.GetInputBuffer(0), m_mixBuffer.GetInputBuffer(1), m_alignedBuffer.f32, numFrames);
		m_pMediaProcess->Process(numFrames * 2 * sizeof(float), reinterpret_cast<BYTE *>(m_alignedBuffer.f32), startTime, DMO_INPLACE_NORMAL);
		DeinterleaveStereo(m_alignedBuffer.f32, m_mixBuffer.GetOutputBuffer(0), m_mixBuffer.GetOutputBuffer(1), numFrames);
	} else
	{
		InterleaveFloatToInt16(m_mixBuffer.GetInputBuffer(0), m_mixBuffer.GetInputBuffer(1), m_alignedBuffer.i16, numFrames);
		m_pMediaProcess->Process(numFrames * 2 * sizeof(int16), reinterpret_cast<BYTE *>(m_alignedBuffer.i16), startTime, DMO_INPLACE_NORMAL);
		DeinterleaveInt16ToFloat(m_alignedBuffer.i16, m_mixBuffer.GetOutputBuffer(0), m_mixBuffer.GetOutputBuffer(1), numFrames);
	}

	ProcessMixOps(pOutL, pOutR, m_mixBuffer.GetOutputBuffer(0), m_mixBuffer.GetOutputBuffer(1), numFrames);
}


PlugParamIndex DMOPlugin::GetNumParameters() const
{
	DWORD dwParamCount = 0;
	m_pParamInfo->GetParamCount(&dwParamCount);
	return dwParamCount;
}


PlugParamValue DMOPlugin::GetParameter(PlugParamIndex index)
{
	if(index < GetNumParameters() && m_pParamInfo != nullptr && m_pMediaParams != nullptr)
	{
		MP_PARAMINFO mpi{};
		MP_DATA md = 0;
		if (m_pParamInfo->GetParamInfo(index, &mpi) == S_OK
			&& m_pMediaParams->GetParam(index, &md) == S_OK)
		{
			float fValue = md;
			float fMin = mpi.mpdMinValue;
			float fMax = mpi.mpdMaxValue;
			//float fDefault = mpi.mpdNeutralValue;
			if (mpi.mpType == MPT_BOOL)
			{
				fMin = 0;
				fMax = 1;
			}
			fValue -= fMin;
			if (fMax > fMin) fValue /= (fMax - fMin);
			return fValue;
		}
	}
	return 0;
}


void DMOPlugin::SetParameter(PlugParamIndex index, PlugParamValue value, PlayState *, CHANNELINDEX)
{
	if(index < GetNumParameters() && m_pParamInfo != nullptr && m_pMediaParams != nullptr)
	{
		MP_PARAMINFO mpi{};
		if (m_pParamInfo->GetParamInfo(index, &mpi) == S_OK)
		{
			float fMin = mpi.mpdMinValue;
			float fMax = mpi.mpdMaxValue;

			if (mpi.mpType == MPT_BOOL)
			{
				fMin = 0;
				fMax = 1;
				value = (value > 0.5f) ? 1.0f : 0.0f;
			}
			if (fMax > fMin) value *= (fMax - fMin);
			value += fMin;
			value = mpt::safe_clamp(value, fMin, fMax);
			if (mpi.mpType != MPT_FLOAT) value = mpt::round(value);
			m_pMediaParams->SetParam(index, value);
		}
	}
}


void DMOPlugin::Resume()
{
	m_nSamplesPerSec = m_SndFile.GetSampleRate();
	m_isResumed = true;

	DMO_MEDIA_TYPE mt;
	WAVEFORMATEX wfx;

	mt.majortype = MEDIATYPE_Audio;
	mt.subtype = MEDIASUBTYPE_PCM;
	mt.bFixedSizeSamples = TRUE;
	mt.bTemporalCompression = FALSE;
	mt.formattype = FORMAT_WaveFormatEx;
	mt.pUnk = nullptr;
	mt.pbFormat = (LPBYTE)&wfx;
	mt.cbFormat = sizeof(WAVEFORMATEX);
	mt.lSampleSize = 2 * sizeof(float);
	wfx.wFormatTag = 3; // WAVE_FORMAT_IEEE_FLOAT;
	wfx.nChannels = 2;
	wfx.nSamplesPerSec = m_nSamplesPerSec;
	wfx.wBitsPerSample = sizeof(float) * 8;
	wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample / 8);
	wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
	wfx.cbSize = 0;

	// First try 32-bit float (DirectX 9+)
	m_useFloat = true;
	if(FAILED(m_pMediaObject->SetInputType(0, &mt, 0))
		|| FAILED(m_pMediaObject->SetOutputType(0, &mt, 0)))
	{
		m_useFloat = false;
		// Try again with 16-bit PCM
		mt.lSampleSize = 2 * sizeof(int16);
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.wBitsPerSample = sizeof(int16) * 8;
		wfx.nBlockAlign = wfx.nChannels * (wfx.wBitsPerSample / 8);
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		if(FAILED(m_pMediaObject->SetInputType(0, &mt, 0))
			|| FAILED(m_pMediaObject->SetOutputType(0, &mt, 0)))
		{
#ifdef DMO_LOG
			MPT_LOG_GLOBAL(LogDebug, "DMO", U_("DMO: Failed to set I/O media type"));
#endif
		}
	}
}


void DMOPlugin::PositionChanged()
{
	m_pMediaObject->Discontinuity(0);
	m_pMediaObject->Flush();
}


void DMOPlugin::Suspend()
{
	m_isResumed = false;
	m_pMediaObject->Flush();
	m_pMediaObject->SetInputType(0, nullptr, DMO_SET_TYPEF_CLEAR);
	m_pMediaObject->SetOutputType(0, nullptr, DMO_SET_TYPEF_CLEAR);
}


#ifdef MODPLUG_TRACKER

CString DMOPlugin::GetParamName(PlugParamIndex param)
{
	if(param < GetNumParameters() && m_pParamInfo != nullptr)
	{
		MP_PARAMINFO mpi;
		mpi.mpType = MPT_INT;
		mpi.szUnitText[0] = 0;
		mpi.szLabel[0] = 0;
		if(m_pParamInfo->GetParamInfo(param, &mpi) == S_OK)
		{
			return mpt::ToCString(mpi.szLabel);
		}
	}
	return CString();

}


CString DMOPlugin::GetParamLabel(PlugParamIndex param)
{
	if(param < GetNumParameters() && m_pParamInfo != nullptr)
	{
		MP_PARAMINFO mpi;
		mpi.mpType = MPT_INT;
		mpi.szUnitText[0] = 0;
		mpi.szLabel[0] = 0;
		if(m_pParamInfo->GetParamInfo(param, &mpi) == S_OK)
		{
			return mpt::ToCString(mpi.szUnitText);
		}
	}
	return CString();
}


CString DMOPlugin::GetParamDisplay(PlugParamIndex param)
{
	if(param < GetNumParameters() && m_pParamInfo != nullptr && m_pMediaParams != nullptr)
	{
		MP_PARAMINFO mpi;
		mpi.mpType = MPT_INT;
		mpi.szUnitText[0] = 0;
		mpi.szLabel[0] = 0;
		if (m_pParamInfo->GetParamInfo(param, &mpi) == S_OK)
		{
			MP_DATA md;
			if(m_pMediaParams->GetParam(param, &md) == S_OK)
			{
				switch(mpi.mpType)
				{
				case MPT_FLOAT:
					{
						CString s;
						s.Format(_T("%.2f"), md);
						return s;
					}
					break;

				case MPT_BOOL:
					return ((int)md) ? _T("Yes") : _T("No");
					break;

				case MPT_ENUM:
					{
						WCHAR *text = nullptr;
						m_pParamInfo->GetParamText(param, &text);

						const int nValue = mpt::saturate_round<int>(md * (mpi.mpdMaxValue - mpi.mpdMinValue));
						// Always skip first two strings (param name, unit name)
						for(int i = 0; i < nValue + 2; i++)
						{
							text += wcslen(text) + 1;
						}
						return mpt::ToCString(text);
					}
					break;

				case MPT_INT:
				default:
					{
						CString s;
						s.Format(_T("%d"), mpt::saturate_round<int>(md));
						return s;
					}
					break;
				}
			}
		}
	}
	return CString();
}

#endif // MODPLUG_TRACKER

#else // !MPT_WITH_DMO

MPT_MSVC_WORKAROUND_LNK4221(DMOPlugin)

#endif // MPT_WITH_DMO

OPENMPT_NAMESPACE_END

