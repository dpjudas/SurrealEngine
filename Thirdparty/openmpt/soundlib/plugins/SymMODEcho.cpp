/*
 * SymMODEcho.cpp
 * --------------
 * Purpose: Implementation of the SymMOD Echo DSP
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#ifndef NO_PLUGINS
#include "SymMODEcho.h"
#include "../Sndfile.h"

OPENMPT_NAMESPACE_BEGIN

IMixPlugin *SymMODEcho::Create(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct)
{
	return new (std::nothrow) SymMODEcho(factory, sndFile, mixStruct);
}


SymMODEcho::SymMODEcho(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct)
	: IMixPlugin(factory, sndFile, mixStruct)
	, m_chunk(PluginChunk::Default())
{
	m_mixBuffer.Initialize(2, 2);
	RecalculateEchoParams();
}


void SymMODEcho::Process(float* pOutL, float* pOutR, uint32 numFrames)
{
	const float *srcL = m_mixBuffer.GetInputBuffer(0), *srcR = m_mixBuffer.GetInputBuffer(1);
	float *outL = m_mixBuffer.GetOutputBuffer(0), *outR = m_mixBuffer.GetOutputBuffer(1);

	const uint32 delayTime = m_SndFile.m_PlayState.m_nSamplesPerTick * m_chunk.param[kEchoDelay];
	// SymMODs don't have a variable tempo so the tick duration should never change... but if someone loads an instance into an MPTM file we have to account for this.
	if(m_delayLine.size() < delayTime * 2)
		m_delayLine.resize(delayTime * 2);

	const auto dspType = GetDSPType();
	if(dspType == DSPType::Off)
	{
		// Toggling the echo while it's running keeps its delay line untouched
		std::copy(srcL, srcL + numFrames, outL);
		std::copy(srcR, srcR + numFrames, outR);
	} else
	{
		for(uint32 i = 0; i < numFrames; i++)
		{
			if(m_writePos >= delayTime)
				m_writePos = 0;
			int readPos = m_writePos - delayTime;
			if(readPos < 0)
				readPos += delayTime;

			const float lDry = *srcL++, rDry = *srcR++;
			const float lDelay = m_delayLine[readPos * 2], rDelay = m_delayLine[readPos * 2 + 1];

			// Output samples
			*outL++ = (lDry + lDelay);
			*outR++ = (rDry + rDelay);

			// Compute new delay line values
			float lOut = 0.0f, rOut = 0.0f;
			switch(dspType)
			{
				case DSPType::Off:
					break;
				case DSPType::Normal:
					lOut = (lDelay + lDry) * m_feedback;
					rOut = (rDelay + rDry) * m_feedback;
					break;
				case DSPType::Cross:
				case DSPType::Cross2:
					lOut = (rDelay + rDry) * m_feedback;
					rOut = (lDelay + lDry) * m_feedback;
					break;
				case DSPType::Center:
					lOut = (lDelay + (lDry + rDry) * 0.5f) * m_feedback;
					rOut = lOut;
					break;
				case DSPType::NumTypes:
					break;
			}

			// Prevent denormals
			if(std::abs(lOut) < 1e-24f)
				lOut = 0.0f;
			if(std::abs(rOut) < 1e-24f)
				rOut = 0.0f;

			m_delayLine[m_writePos * 2 + 0] = lOut;
			m_delayLine[m_writePos * 2 + 1] = rOut;
			m_writePos++;
		}
	}

	ProcessMixOps(pOutL, pOutR, m_mixBuffer.GetOutputBuffer(0), m_mixBuffer.GetOutputBuffer(1), numFrames);
}


void SymMODEcho::SaveAllParameters()
{
	m_pMixStruct->defaultProgram = -1;
	try
	{
		const auto pluginData = mpt::as_raw_memory(m_chunk);
		m_pMixStruct->pluginData.assign(pluginData.begin(), pluginData.end());
	} catch(mpt::out_of_memory e)
	{
		mpt::delete_out_of_memory(e);
		m_pMixStruct->pluginData.clear();
	}
}


void SymMODEcho::RestoreAllParameters(int32 program)
{
	if(m_pMixStruct->pluginData.size() == sizeof(m_chunk) && !memcmp(m_pMixStruct->pluginData.data(), "Echo", 4))
	{
		std::copy(m_pMixStruct->pluginData.begin(), m_pMixStruct->pluginData.end(), mpt::as_raw_memory(m_chunk).begin());
	} else
	{
		IMixPlugin::RestoreAllParameters(program);
	}
	RecalculateEchoParams();
}


PlugParamValue SymMODEcho::GetParameter(PlugParamIndex index)
{
	if(index < kEchoNumParameters)
	{
		return m_chunk.param[index] / 127.0f;
	}
	return 0;
}


void SymMODEcho::SetParameter(PlugParamIndex index, PlugParamValue value, PlayState *, CHANNELINDEX)
{
	if(index < kEchoNumParameters)
	{
		m_chunk.param[index] = mpt::saturate_round<uint8>(mpt::safe_clamp(value, 0.0f, 1.0f) * 127.0f);
		RecalculateEchoParams();
	}
}


void SymMODEcho::Resume()
{
	m_isResumed = true;
	PositionChanged();
}


void SymMODEcho::PositionChanged()
{
	try
	{
		m_delayLine.assign(127 * 2 * m_SndFile.m_PlayState.m_nSamplesPerTick, 0.0f);
	} catch(mpt::out_of_memory e)
	{
		mpt::delete_out_of_memory(e);
	}
	m_writePos = 0;
}


#ifdef MODPLUG_TRACKER

std::pair<PlugParamValue, PlugParamValue> SymMODEcho::GetParamUIRange(PlugParamIndex param)
{
	if(param == kEchoType)
		return {0.0f, (static_cast<uint8>(DSPType::NumTypes) - 1) / 127.0f};
	else
		return {0.0f, 1.0f};
}

CString SymMODEcho::GetParamName(PlugParamIndex param)
{
	switch (param)
	{
	case kEchoType: return _T("Type");
	case kEchoDelay: return _T("Delay");
	case kEchoFeedback: return _T("Feedback");
	case kEchoNumParameters: break;
	}
	return {};
}


CString SymMODEcho::GetParamLabel(PlugParamIndex param)
{
	if(param == kEchoDelay)
		return _T("Ticks");
	if(param == kEchoFeedback)
		return _T("%");
	return {};
}


CString SymMODEcho::GetParamDisplay(PlugParamIndex param)
{
	switch(static_cast<Parameters>(param))
	{
	case kEchoType:
		switch(GetDSPType())
		{
			case DSPType::Off: return _T("Off");
			case DSPType::Normal: return _T("Normal");
			case DSPType::Cross: return _T("Cross");
			case DSPType::Cross2: return _T("Cross 2");
			case DSPType::Center: return _T("Center");
			case DSPType::NumTypes: break;
		}
		break;
	case kEchoDelay:
		return mpt::cfmt::val(m_chunk.param[kEchoDelay]);
	case kEchoFeedback:
		return mpt::cfmt::flt(m_feedback * 100.0f, 4);
	case kEchoNumParameters:
		break;
	}
	return {};
}

#endif // MODPLUG_TRACKER


IMixPlugin::ChunkData SymMODEcho::GetChunk(bool)
{
	auto data = reinterpret_cast<const std::byte *>(&m_chunk);
	return ChunkData(data, sizeof(m_chunk));
}


void SymMODEcho::SetChunk(const ChunkData& chunk, bool)
{
	auto data = chunk.data();
	if(chunk.size() == sizeof(chunk) && !memcmp(data, "Echo", 4))
	{
		memcpy(&m_chunk, data, chunk.size());
		RecalculateEchoParams();
	}
}


void SymMODEcho::RecalculateEchoParams()
{
	if(m_chunk.param[kEchoType] >= static_cast<uint8>(DSPType::NumTypes))
		m_chunk.param[kEchoType] = 0;
	if(m_chunk.param[kEchoDelay] > 127)
		m_chunk.param[kEchoDelay] = 127;
	if(m_chunk.param[kEchoFeedback] > 127)
		m_chunk.param[kEchoFeedback] = 127;

	if(GetDSPType() == DSPType::Cross2)
		m_feedback = 1.0f - std::pow(2.0f, -static_cast<float>(m_chunk.param[kEchoFeedback] + 1));
	else
		m_feedback = std::pow(2.0f, -static_cast<float>(m_chunk.param[kEchoFeedback]));
}

OPENMPT_NAMESPACE_END

#endif // NO_PLUGINS
