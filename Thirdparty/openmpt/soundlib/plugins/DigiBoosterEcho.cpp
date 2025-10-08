/*
 * DigiBoosterEcho.cpp
 * -------------------
 * Purpose: Implementation of the DigiBooster Pro Echo DSP
 * Notes  : (currently none)
 * Authors: OpenMPT Devs, based on original code by Grzegorz Kraszewski (BSD 2-clause)
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#ifndef NO_PLUGINS
#include "DigiBoosterEcho.h"
#include "../Sndfile.h"

OPENMPT_NAMESPACE_BEGIN

IMixPlugin* DigiBoosterEcho::Create(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct)
{
	return new (std::nothrow) DigiBoosterEcho(factory, sndFile, mixStruct);
}


DigiBoosterEcho::DigiBoosterEcho(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct)
	: IMixPlugin(factory, sndFile, mixStruct)
	, m_sampleRate(sndFile.GetSampleRate())
	, m_chunk(PluginChunk::Default())
{
	m_mixBuffer.Initialize(2, 2);
}


void DigiBoosterEcho::Process(float *pOutL, float *pOutR, uint32 numFrames)
{
	if(!m_bufferSize)
		return;
	const float *srcL = m_mixBuffer.GetInputBuffer(0), *srcR = m_mixBuffer.GetInputBuffer(1);
	float *outL = m_mixBuffer.GetOutputBuffer(0), *outR = m_mixBuffer.GetOutputBuffer(1);

	for(uint32 i = numFrames; i != 0; i--)
	{
		int readPos = m_writePos - m_delayTime;
		if(readPos < 0)
			readPos += m_bufferSize;

		float l = *srcL++, r = *srcR++;
		float lDelay = m_delayLine[readPos * 2], rDelay = m_delayLine[readPos * 2 + 1];

		// Calculate the delay
		float al = l * m_NCrossNBack;
		al += r * m_PCrossNBack;
		al += lDelay * m_NCrossPBack;
		al += rDelay * m_PCrossPBack;

		float ar = r * m_NCrossNBack;
		ar += l * m_PCrossNBack;
		ar += rDelay * m_NCrossPBack;
		ar += lDelay * m_PCrossPBack;

		// Prevent denormals
		if(std::abs(al) < 1e-24f)
			al = 0.0f;
		if(std::abs(ar) < 1e-24f)
			ar = 0.0f;

		m_delayLine[m_writePos * 2] = al;
		m_delayLine[m_writePos * 2 + 1] = ar;
		m_writePos++;
		if(m_writePos == m_bufferSize)
			m_writePos = 0;

		// Output samples now
		*outL++ = (l * m_NMix + lDelay * m_PMix);
		*outR++ = (r * m_NMix + rDelay * m_PMix);
	}

	ProcessMixOps(pOutL, pOutR, m_mixBuffer.GetOutputBuffer(0), m_mixBuffer.GetOutputBuffer(1), numFrames);
}


void DigiBoosterEcho::SaveAllParameters()
{
	m_pMixStruct->defaultProgram = -1;
	try
	{
		m_pMixStruct->pluginData.resize(sizeof(m_chunk));
		memcpy(m_pMixStruct->pluginData.data(), &m_chunk, sizeof(m_chunk));
	} catch(mpt::out_of_memory e)
	{
		mpt::delete_out_of_memory(e);
		m_pMixStruct->pluginData.clear();
	}
}


void DigiBoosterEcho::RestoreAllParameters(int32 program)
{
	if(m_pMixStruct->pluginData.size() == sizeof(m_chunk) && !memcmp(m_pMixStruct->pluginData.data(), "Echo", 4))
	{
		memcpy(&m_chunk, m_pMixStruct->pluginData.data(), sizeof(m_chunk));
	} else
	{
		IMixPlugin::RestoreAllParameters(program);
	}
	RecalculateEchoParams();
}


PlugParamValue DigiBoosterEcho::GetParameter(PlugParamIndex index)
{
	if(index < kEchoNumParameters)
	{
		return m_chunk.param[index] / 255.0f;
	}
	return 0;
}


void DigiBoosterEcho::SetParameter(PlugParamIndex index, PlugParamValue value, PlayState *, CHANNELINDEX)
{
	if(index < kEchoNumParameters)
	{
		m_chunk.param[index] = mpt::saturate_round<uint8>(mpt::safe_clamp(value, 0.0f, 1.0f) * 255.0f);
		RecalculateEchoParams();
	}
}


void DigiBoosterEcho::Resume()
{
	m_isResumed = true;
	m_sampleRate = m_SndFile.GetSampleRate();
	RecalculateEchoParams();
	PositionChanged();
}


void DigiBoosterEcho::PositionChanged()
{
	m_bufferSize = (m_sampleRate >> 1) + (m_sampleRate >> 6);
	try
	{
		m_delayLine.assign(m_bufferSize * 2, 0);
	} catch(mpt::out_of_memory e)
	{
		mpt::delete_out_of_memory(e);
		m_bufferSize = 0;
	}
	m_writePos = 0;
}


#ifdef MODPLUG_TRACKER

CString DigiBoosterEcho::GetParamName(PlugParamIndex param)
{
	switch(param)
	{
	case kEchoDelay: return _T("Delay");
	case kEchoFeedback: return _T("Feedback");
	case kEchoMix: return _T("Wet / Dry Ratio");
	case kEchoCross: return _T("Cross Echo");
	}
	return CString();
}


CString DigiBoosterEcho::GetParamLabel(PlugParamIndex param)
{
	if(param == kEchoDelay)
		return _T("ms");
	return CString();
}


CString DigiBoosterEcho::GetParamDisplay(PlugParamIndex param)
{
	CString s;
	if(param == kEchoMix)
	{
		int wet = (m_chunk.param[kEchoMix] * 100) / 255;
		s.Format(_T("%d%% / %d%%"), wet, 100 - wet);
	} else if(param < kEchoNumParameters)
	{
		int val = m_chunk.param[param];
		if(param == kEchoDelay)
		{
			if(val == 0)
				val = 167;
			val *= 2;
		}	
		s.Format(_T("%d"), val);
	}
	return s;
}

#endif // MODPLUG_TRACKER


IMixPlugin::ChunkData DigiBoosterEcho::GetChunk(bool)
{
	auto data = reinterpret_cast<const std::byte *>(&m_chunk);
	return ChunkData(data, sizeof(m_chunk));
}


void DigiBoosterEcho::SetChunk(const ChunkData &chunk, bool)
{
	auto data = chunk.data();
	if(chunk.size() == sizeof(chunk) && !memcmp(data, "Echo", 4))
	{
		memcpy(&m_chunk, data, chunk.size());
		RecalculateEchoParams();
	}
}


void DigiBoosterEcho::RecalculateEchoParams()
{
	// The fallback value when the delay is 0 was determined experimentally from DBPro 2.21 output.
	// The C implementation of libdigibooster3 has no specific handling of this value and thus produces a delay with maximum length.
	m_delayTime = ((m_chunk.param[kEchoDelay] ? m_chunk.param[kEchoDelay] : 167u) * m_sampleRate + 250u) / 500u;
	m_PMix = static_cast<float>(m_chunk.param[kEchoMix]) * (1.0f / 256.0f);
	m_NMix = static_cast<float>(256 - m_chunk.param[kEchoMix]) * (1.0f / 256.0f);
	m_PCrossPBack = static_cast<float>(m_chunk.param[kEchoCross] * m_chunk.param[kEchoFeedback]) * (1.0f / 65536.0f);
	m_PCrossNBack = static_cast<float>(m_chunk.param[kEchoCross] * (256 - m_chunk.param[kEchoFeedback])) * (1.0f / 65536.0f);
	m_NCrossPBack = static_cast<float>((m_chunk.param[kEchoCross] - 256) * m_chunk.param[kEchoFeedback]) * (1.0f / 65536.0f);
	m_NCrossNBack = static_cast<float>((m_chunk.param[kEchoCross] - 256) * (m_chunk.param[kEchoFeedback] - 256)) * (1.0f / 65536.0f);
}

OPENMPT_NAMESPACE_END

#endif // NO_PLUGINS
