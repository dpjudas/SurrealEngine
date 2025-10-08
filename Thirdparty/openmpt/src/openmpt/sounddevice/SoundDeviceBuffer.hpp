/* SPDX-License-Identifier: BSD-3-Clause */
/* SPDX-FileCopyrightText: OpenMPT Project Developers and Contributors */


#pragma once

#include "openmpt/all/BuildSettings.hpp"

#include "SoundDeviceCallback.hpp"

#include "mpt/audio/span.hpp"
#include "openmpt/base/Types.hpp"
#include "openmpt/soundbase/Dither.hpp"
#include "openmpt/soundbase/CopyMix.hpp"

#include <variant>

#include <cassert>
#include <cstddef>


OPENMPT_NAMESPACE_BEGIN


namespace SoundDevice
{


template <typename Tsample>
class BufferIO
{

private:
	const mpt::audio_span_interleaved<const Tsample> m_src;
	const mpt::audio_span_interleaved<Tsample> m_dst;
	std::size_t m_countFramesReadProcessed;
	std::size_t m_countFramesWriteProcessed;
	const BufferFormat m_bufferFormat;

public:
	inline BufferIO(Tsample *dst, const Tsample *src, std::size_t numFrames, BufferFormat bufferFormat)
		: m_src(src, bufferFormat.InputChannels, numFrames)
		, m_dst(dst, bufferFormat.Channels, numFrames)
		, m_countFramesReadProcessed(0)
		, m_countFramesWriteProcessed(0)
		, m_bufferFormat(bufferFormat)
	{
		return;
	}

	template <typename audio_span_dst>
	inline void Read(audio_span_dst dst)
	{
		assert(m_countFramesReadProcessed + dst.size_frames() <= m_src.size_frames());
		ConvertBufferToBufferMixInternal(dst, mpt::make_audio_span_with_offset(m_src, m_countFramesReadProcessed), m_bufferFormat.InputChannels, dst.size_frames());
		m_countFramesReadProcessed += dst.size_frames();
	}

	template <int fractionalBits, typename audio_span_dst>
	inline void ReadFixedPoint(audio_span_dst dst)
	{
		assert(m_countFramesReadProcessed + dst.size_frames() <= m_src.size_frames());
		ConvertBufferToBufferMixInternalFixed<fractionalBits>(dst, mpt::make_audio_span_with_offset(m_src, m_countFramesReadProcessed), m_bufferFormat.InputChannels, dst.size_frames());
		m_countFramesReadProcessed += dst.size_frames();
	}

	template <typename audio_span_src, typename TDither>
	inline void Write(audio_span_src src, TDither &dither)
	{
		assert(m_countFramesWriteProcessed + src.size_frames() <= m_dst.size_frames());
		if(m_bufferFormat.WantsClippedOutput)
		{
			ConvertBufferMixInternalToBuffer<true>(mpt::make_audio_span_with_offset(m_dst, m_countFramesWriteProcessed), src, dither, m_bufferFormat.Channels, src.size_frames());
		} else
		{
			ConvertBufferMixInternalToBuffer<false>(mpt::make_audio_span_with_offset(m_dst, m_countFramesWriteProcessed), src, dither, m_bufferFormat.Channels, src.size_frames());
		}
		m_countFramesWriteProcessed += src.size_frames();
	}

	template <int fractionalBits, typename audio_span_src, typename TDither>
	inline void WriteFixedPoint(audio_span_src src, TDither &dither)
	{
		assert(m_countFramesWriteProcessed + src.size_frames() <= m_dst.size_frames());
		if(m_bufferFormat.WantsClippedOutput)
		{
			ConvertBufferMixInternalFixedToBuffer<fractionalBits, true>(mpt::make_audio_span_with_offset(m_dst, m_countFramesWriteProcessed), src, dither, m_bufferFormat.Channels, src.size_frames());
		} else
		{
			ConvertBufferMixInternalFixedToBuffer<fractionalBits, false>(mpt::make_audio_span_with_offset(m_dst, m_countFramesWriteProcessed), src, dither, m_bufferFormat.Channels, src.size_frames());
		}
		m_countFramesWriteProcessed += src.size_frames();
	}

	inline ~BufferIO()
	{
		// fill remaining buffer with silence
		while(m_countFramesWriteProcessed < m_dst.size_frames())
		{
			for(std::size_t channel = 0; channel < m_dst.size_channels(); ++channel)
			{
				m_dst(channel, m_countFramesWriteProcessed) = SC::sample_cast<Tsample>(static_cast<int16>(0));
			}
			m_countFramesWriteProcessed += 1;
		}
	}
};


template <typename TDithers>
class CallbackBuffer
{
private:
	std::variant<
		BufferIO<uint8>,
		BufferIO<int8>,
		BufferIO<int16>,
		BufferIO<int24>,
		BufferIO<int32>,
		BufferIO<float>,
		BufferIO<double>>
		m_BufferIO;
	TDithers &m_Dithers;
	std::size_t m_NumFrames;

public:
	template <typename Tsample>
	explicit inline CallbackBuffer(Tsample *dst, const Tsample *src, std::size_t numFrames, TDithers &dithers, BufferFormat bufferFormat)
		: m_BufferIO(BufferIO<Tsample>{dst, src, numFrames, bufferFormat})
		, m_Dithers(dithers)
		, m_NumFrames(numFrames)
	{
		return;
	}

	inline std::size_t GetNumFrames() const
	{
		return m_NumFrames;
	}

	template <typename audio_span_dst>
	inline void Read(audio_span_dst dst)
	{
		std::visit(
			[&](auto &bufferIO)
			{
			bufferIO.Read(dst);
			},
			m_BufferIO);
	}

	template <int fractionalBits, typename audio_span_dst>
	inline void ReadFixedPoint(audio_span_dst dst)
	{
		std::visit(
			[&](auto &bufferIO)
			{
			bufferIO.template ReadFixedPoint<fractionalBits>(dst);
			},
			m_BufferIO);
	}

	template <typename audio_span_src>
	inline void Write(audio_span_src src)
	{
		std::visit(
			[&](auto &bufferIO)
			{
			std::visit(
				[&](auto &ditherInstance)
				{
				bufferIO.Write(src, ditherInstance);
				},
				m_Dithers.Variant());
			},
			m_BufferIO);
	}

	template <int fractionalBits, typename audio_span_src>
	inline void WriteFixedPoint(audio_span_src src)
	{
		std::visit(
			[&](auto &bufferIO)
			{
			std::visit(
				[&](auto &ditherInstance)
				{
				bufferIO.template WriteFixedPoint<fractionalBits>(src, ditherInstance);
				},
				m_Dithers.Variant());
			},
			m_BufferIO);
	}
};


template <typename TDithers>
class CallbackBufferHandler
	: public ICallback
{
private:
	TDithers m_Dithers;

protected:
	template <typename Trd>
	explicit CallbackBufferHandler(Trd &rd)
		: m_Dithers(rd)
	{
		return;
	}

protected:
	inline TDithers &Dithers()
	{
		return m_Dithers;
	}

private:
	template <typename Tsample>
	inline void SoundCallbackLockedProcessImpl(BufferFormat bufferFormat, std::size_t numFrames, Tsample *buffer, const Tsample *inputBuffer)
	{
		CallbackBuffer<TDithers> callbackBuffer{buffer, inputBuffer, numFrames, m_Dithers, bufferFormat};
		SoundCallbackLockedCallback(callbackBuffer);
	}

public:
	inline void SoundCallbackLockedProcess(BufferFormat bufferFormat, std::size_t numFrames, uint8 *buffer, const uint8 *inputBuffer) final
	{
		SoundCallbackLockedProcessImpl(bufferFormat, numFrames, buffer, inputBuffer);
	}
	inline void SoundCallbackLockedProcess(BufferFormat bufferFormat, std::size_t numFrames, int8 *buffer, const int8 *inputBuffer) final
	{
		SoundCallbackLockedProcessImpl(bufferFormat, numFrames, buffer, inputBuffer);
	}
	inline void SoundCallbackLockedProcess(BufferFormat bufferFormat, std::size_t numFrames, int16 *buffer, const int16 *inputBuffer) final
	{
		SoundCallbackLockedProcessImpl(bufferFormat, numFrames, buffer, inputBuffer);
	}
	inline void SoundCallbackLockedProcess(BufferFormat bufferFormat, std::size_t numFrames, int24 *buffer, const int24 *inputBuffer) final
	{
		SoundCallbackLockedProcessImpl(bufferFormat, numFrames, buffer, inputBuffer);
	}
	inline void SoundCallbackLockedProcess(BufferFormat bufferFormat, std::size_t numFrames, int32 *buffer, const int32 *inputBuffer) final
	{
		SoundCallbackLockedProcessImpl(bufferFormat, numFrames, buffer, inputBuffer);
	}
	inline void SoundCallbackLockedProcess(BufferFormat bufferFormat, std::size_t numFrames, float *buffer, const float *inputBuffer) final
	{
		SoundCallbackLockedProcessImpl(bufferFormat, numFrames, buffer, inputBuffer);
	}
	inline void SoundCallbackLockedProcess(BufferFormat bufferFormat, std::size_t numFrames, double *buffer, const double *inputBuffer) final
	{
		SoundCallbackLockedProcessImpl(bufferFormat, numFrames, buffer, inputBuffer);
	}
	virtual void SoundCallbackLockedCallback(CallbackBuffer<TDithers> &buffer) = 0;
};


}  // namespace SoundDevice


OPENMPT_NAMESPACE_END
