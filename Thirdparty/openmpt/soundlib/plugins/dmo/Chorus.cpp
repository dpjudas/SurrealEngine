/*
 * Chorus.cpp
 * ----------
 * Purpose: Implementation of the DMO Chorus DSP (for non-Windows platforms)
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */


#include "stdafx.h"

#ifndef NO_PLUGINS
#include "Chorus.h"
#include "../../Sndfile.h"
#include "mpt/base/numbers.hpp"
#endif // !NO_PLUGINS

OPENMPT_NAMESPACE_BEGIN

#ifndef NO_PLUGINS

namespace DMO
{

IMixPlugin* Chorus::Create(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct)
{
	return new (std::nothrow) Chorus(factory, sndFile, mixStruct);
}


Chorus::Chorus(VSTPluginLib &factory, CSoundFile &sndFile, SNDMIXPLUGIN &mixStruct, bool isFlanger)
	: IMixPlugin(factory, sndFile, mixStruct)
	, m_isFlanger(isFlanger)
{
	m_param[kChorusWetDryMix] = 0.5f;
	m_param[kChorusDepth] = 0.1f;
	m_param[kChorusFrequency] = 0.11f;
	m_param[kChorusWaveShape] = 1.0f;
	m_param[kChorusPhase] = 0.75f;
	m_param[kChorusFeedback] = (25.0f + 99.0f) / 198.0f;
	m_param[kChorusDelay] = 0.8f;

	m_mixBuffer.Initialize(2, 2);
}


// Integer part of buffer position
int32 Chorus::GetBufferIntOffset(int32 fpOffset) const
{
	if(fpOffset < 0)
		fpOffset += m_bufSize * 4096;
	MPT_ASSERT(fpOffset >= 0);
	return (m_bufPos + (fpOffset / 4096)) % m_bufSize;
}


void Chorus::Process(float *pOutL, float *pOutR, uint32 numFrames)
{
	if(!m_bufSize || !m_mixBuffer.Ok())
		return;

	const float *in[2] = { m_mixBuffer.GetInputBuffer(0), m_mixBuffer.GetInputBuffer(1) };
	float *out[2] = { m_mixBuffer.GetOutputBuffer(0), m_mixBuffer.GetOutputBuffer(1) };

	const bool isSquare = IsSquare();
	const float feedback = Feedback() / 100.0f;
	const float wetDryMix = WetDryMix();
	const uint32 phase = Phase();
	const auto &bufferR = m_isFlanger ? m_bufferR : m_bufferL;

	for(uint32 i = numFrames; i != 0; i--)
	{
		const float leftIn = *(in[0])++;
		const float rightIn = *(in[1])++;

		const int32 readOffset = GetBufferIntOffset(m_delayOffset);
		const int32 writeOffset = m_bufPos;
		if(m_isFlanger)
		{
			m_DryBufferL[m_dryWritePos] = leftIn;
			m_DryBufferR[m_dryWritePos] = rightIn;
			m_bufferL[writeOffset] = (m_bufferL[readOffset] * feedback) + leftIn;
			m_bufferR[writeOffset] = (m_bufferR[readOffset] * feedback) + rightIn;
		} else
		{
			m_bufferL[writeOffset] = (m_bufferL[readOffset] * feedback) + (leftIn + rightIn) * 0.5f;
		}

		float waveMin;
		float waveMax;
		if(isSquare)
		{
			m_waveShapeMin += m_waveShapeVal;
			m_waveShapeMax += m_waveShapeVal;
			if(m_waveShapeMin > 1)
				m_waveShapeMin -= 2;
			if(m_waveShapeMax > 1)
				m_waveShapeMax -= 2;
			waveMin = std::abs(m_waveShapeMin) * 2 - 1;
			waveMax = std::abs(m_waveShapeMax) * 2 - 1;
		} else
		{
			m_waveShapeMin = m_waveShapeMax * m_waveShapeVal + m_waveShapeMin;
			m_waveShapeMax = m_waveShapeMax - m_waveShapeMin * m_waveShapeVal;
			waveMin = m_waveShapeMin;
			waveMax = m_waveShapeMax;
		}

		const float leftDelayIn = m_isFlanger ? m_DryBufferL[(m_dryWritePos + 2) % 3] : leftIn;
		const float rightDelayIn = m_isFlanger ? m_DryBufferR[(m_dryWritePos + 2) % 3] : rightIn;

		float left1 = m_bufferL[GetBufferIntOffset(m_delayL)];
		float left2 = m_bufferL[GetBufferIntOffset(m_delayL + 4096)];
		float fracPos = static_cast<float>(m_delayL & 0xFFF) * (1.0f / 4096.0f);
		float leftOut = (left2 - left1) * fracPos + left1;
		*(out[0])++ = leftDelayIn + (leftOut - leftDelayIn) * wetDryMix;

		float right1 = bufferR[GetBufferIntOffset(m_delayR)];
		float right2 = bufferR[GetBufferIntOffset(m_delayR + 4096)];
		fracPos = static_cast<float>(m_delayR & 0xFFF) * (1.0f / 4096.0f);
		float rightOut = (right2 - right1) * fracPos + right1;
		*(out[1])++ = rightDelayIn + (rightOut - rightDelayIn) * wetDryMix;

		// Increment delay positions
		if(m_dryWritePos <= 0)
			m_dryWritePos += 3;
		m_dryWritePos--;

		m_delayL = m_delayOffset + (phase < 4 ? 1 : -1) * static_cast<int32>(waveMin * m_depthDelay);
		m_delayR = m_delayOffset + (phase < 2 ? -1 : 1) * static_cast<int32>(((phase % 2u) ? waveMax : waveMin) * m_depthDelay);

		if(m_bufPos <= 0)
			m_bufPos += m_bufSize;
		m_bufPos--;
	}

	ProcessMixOps(pOutL, pOutR, m_mixBuffer.GetOutputBuffer(0), m_mixBuffer.GetOutputBuffer(1), numFrames);
}


PlugParamValue Chorus::GetParameter(PlugParamIndex index)
{
	if(index < kChorusNumParameters)
	{
		return m_param[index];
	}
	return 0;
}


void Chorus::SetParameter(PlugParamIndex index, PlugParamValue value, PlayState *, CHANNELINDEX)
{
	if(index < kChorusNumParameters)
	{
		value = mpt::safe_clamp(value, 0.0f, 1.0f);
		if(index == kChorusWaveShape)
		{
			value = mpt::round(value);
			if(m_param[index] != value)
			{
				m_waveShapeMin = 0.0f;
				m_waveShapeMax = 0.5f + value * 0.5f;
			}
		} else if(index == kChorusPhase)
		{
			value = mpt::round(value * 4.0f) / 4.0f;
		}
		m_param[index] = value;
		RecalculateChorusParams();
	}
}


void Chorus::Resume()
{
	PositionChanged();
	RecalculateChorusParams();

	m_isResumed = true;
	m_waveShapeMin = 0.0f;
	m_waveShapeMax = IsSquare() ? 0.5f : 1.0f;
	m_delayL = m_delayR = m_delayOffset;
	m_dryWritePos = 0;
}


void Chorus::PositionChanged()
{
	m_bufSize = Util::muldiv(m_SndFile.GetSampleRate(), 3840, 1000);
	m_bufPos = 0;
	try
	{
		m_bufferL.assign(m_bufSize, 0.0f);
		if(m_isFlanger)
			m_bufferR.assign(m_bufSize, 0.0f);
		m_DryBufferL.fill(0.0f);
		m_DryBufferR.fill(0.0f);
	} catch(mpt::out_of_memory e)
	{
		mpt::delete_out_of_memory(e);
		m_bufSize = 0;
	}
}


#ifdef MODPLUG_TRACKER

CString Chorus::GetParamName(PlugParamIndex param)
{
	switch(param)
	{
	case kChorusWetDryMix: return _T("WetDryMix");
	case kChorusDepth: return _T("Depth");
	case kChorusFrequency: return _T("Frequency");
	case kChorusWaveShape: return _T("WaveShape");
	case kChorusPhase: return _T("Phase");
	case kChorusFeedback: return _T("Feedback");
	case kChorusDelay: return _T("Delay");
	}
	return CString();
}


CString Chorus::GetParamLabel(PlugParamIndex param)
{
	switch(param)
	{
	case kChorusWetDryMix:
	case kChorusDepth:
	case kChorusFeedback:
		return _T("%");
	case kChorusFrequency:
		return _T("Hz");
	case kChorusPhase:
		return mpt::ToCString(MPT_UTF8("\xC2\xB0"));  // U+00B0 DEGREE SIGN
	case kChorusDelay:
		return _T("ms");
	}
	return CString();
}


CString Chorus::GetParamDisplay(PlugParamIndex param)
{
	CString s;
	float value = m_param[param];
	switch(param)
	{
	case kChorusWetDryMix:
	case kChorusDepth:
		value *= 100.0f;
		break;
	case kChorusFrequency:
		value = FrequencyInHertz();
		break;
	case kChorusWaveShape:
		return (value < 1) ? _T("Square") : _T("Sine");
		break;
	case kChorusPhase:
		switch(Phase())
		{
		case 0: return _T("-180");
		case 1: return _T("-90");
		case 2: return _T("0");
		case 3: return _T("90");
		case 4: return _T("180");
		}
		break;
	case kChorusFeedback:
		value = Feedback();
		break;
	case kChorusDelay:
		value = Delay();
	}
	s.Format(_T("%.2f"), value);
	return s;
}

#endif // MODPLUG_TRACKER


void Chorus::RecalculateChorusParams()
{
	const float sampleRate = static_cast<float>(m_SndFile.GetSampleRate());

	float delaySamples = Delay() * sampleRate / 1000.0f;
	m_depthDelay = Depth() * delaySamples * 2048.0f;
	m_delayOffset = mpt::saturate_round<int32>(4096.0f * (delaySamples + 2.0f));
	m_frequency = FrequencyInHertz();
	const float frequencySamples = m_frequency / sampleRate;
	if(IsSquare())
		m_waveShapeVal = frequencySamples * 2.0f;
	else
		m_waveShapeVal = std::sin(frequencySamples * mpt::numbers::pi_v<float>) * 2.0f;
}

} // namespace DMO

#else
MPT_MSVC_WORKAROUND_LNK4221(Chorus)

#endif // !NO_PLUGINS

OPENMPT_NAMESPACE_END
