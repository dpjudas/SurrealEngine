/* SPDX-License-Identifier: BSL-1.0 OR BSD-3-Clause */

#ifndef MPT_AUDIO_SPAN_HPP
#define MPT_AUDIO_SPAN_HPP



#include "mpt/base/macros.hpp"
#include "mpt/base/namespace.hpp"

#include <utility>
#include <variant>

#include <cassert>
#include <cstddef>



namespace mpt {
inline namespace MPT_INLINE_NS {



// LRLRLRLRLR

template <typename SampleType>
struct audio_span_interleaved {
public:
	using sample_type = SampleType;

private:
	sample_type * const m_buffer;
	std::size_t m_channels;
	std::size_t m_frames;

public:
	MPT_CONSTEXPRINLINE audio_span_interleaved(sample_type * buffer, std::size_t channels, std::size_t frames) noexcept
		: m_buffer(buffer)
		, m_channels(channels)
		, m_frames(frames) {
		return;
	}
	MPT_FORCEINLINE sample_type * const * data_planar() const noexcept {
		return nullptr;
	}
	MPT_FORCEINLINE sample_type * data() const noexcept {
		return m_buffer;
	}
	MPT_FORCEINLINE sample_type & operator()(std::size_t channel, std::size_t frame) const {
		return m_buffer[m_channels * frame + channel];
	}
	MPT_FORCEINLINE bool is_contiguous() const noexcept {
		return true;
	}
	MPT_FORCEINLINE bool channels_are_contiguous() const noexcept {
		return false;
	}
	MPT_FORCEINLINE bool frames_are_contiguous() const noexcept {
		return true;
	}
	MPT_FORCEINLINE std::size_t size_channels() const noexcept {
		return m_channels;
	}
	MPT_FORCEINLINE std::size_t size_frames() const noexcept {
		return m_frames;
	}
	MPT_FORCEINLINE std::size_t size_samples() const noexcept {
		return m_channels * m_frames;
	}
	MPT_FORCEINLINE std::ptrdiff_t frame_stride() const noexcept {
		return m_channels;
	}
};

struct audio_span_frames_are_contiguous_t {
};

inline constexpr audio_span_frames_are_contiguous_t audio_span_frames_are_contiguous;



// LLLLLRRRRR

template <typename SampleType>
struct audio_span_contiguous {
public:
	using sample_type = SampleType;

private:
	sample_type * const m_buffer;
	std::size_t m_channels;
	std::size_t m_frames;

public:
	MPT_CONSTEXPRINLINE audio_span_contiguous(sample_type * buffer, std::size_t channels, std::size_t frames) noexcept
		: m_buffer(buffer)
		, m_channels(channels)
		, m_frames(frames) {
		return;
	}
	MPT_FORCEINLINE sample_type * const * data_planar() const noexcept {
		return nullptr;
	}
	MPT_FORCEINLINE sample_type * data() const noexcept {
		return m_buffer;
	}
	MPT_FORCEINLINE sample_type & operator()(std::size_t channel, std::size_t frame) const {
		return m_buffer[(m_frames * channel) + frame];
	}
	MPT_FORCEINLINE bool is_contiguous() const noexcept {
		return true;
	}
	MPT_FORCEINLINE bool channels_are_contiguous() const noexcept {
		return true;
	}
	MPT_FORCEINLINE bool frames_are_contiguous() const noexcept {
		return false;
	}
	MPT_FORCEINLINE std::size_t size_channels() const noexcept {
		return m_channels;
	}
	MPT_FORCEINLINE std::size_t size_frames() const noexcept {
		return m_frames;
	}
	MPT_FORCEINLINE std::size_t size_samples() const noexcept {
		return m_channels * m_frames;
	}
	MPT_FORCEINLINE std::ptrdiff_t frame_stride() const noexcept {
		return 1;
	}
};

struct audio_span_channels_are_contiguous_t {
};

inline constexpr audio_span_channels_are_contiguous_t audio_span_channels_are_contiguous;



// LLLLL RRRRR

template <typename SampleType>
struct audio_span_planar {
public:
	using sample_type = SampleType;

private:
	sample_type * const * m_buffers;
	std::size_t m_channels;
	std::size_t m_frames;

public:
	MPT_CONSTEXPRINLINE audio_span_planar(sample_type * const * buffers, std::size_t channels, std::size_t frames) noexcept
		: m_buffers(buffers)
		, m_channels(channels)
		, m_frames(frames) {
		return;
	}
	MPT_FORCEINLINE sample_type * const * data_planar() const noexcept {
		return m_buffers;
	}
	MPT_FORCEINLINE sample_type * data() const noexcept {
		return nullptr;
	}
	MPT_FORCEINLINE sample_type & operator()(std::size_t channel, std::size_t frame) const {
		return m_buffers[channel][frame];
	}
	MPT_FORCEINLINE bool is_contiguous() const noexcept {
		return false;
	}
	MPT_FORCEINLINE bool channels_are_contiguous() const noexcept {
		return true;
	}
	MPT_FORCEINLINE bool frames_are_contiguous() const noexcept {
		return false;
	}
	MPT_FORCEINLINE std::size_t size_channels() const noexcept {
		return m_channels;
	}
	MPT_FORCEINLINE std::size_t size_frames() const noexcept {
		return m_frames;
	}
	MPT_FORCEINLINE std::size_t size_samples() const noexcept {
		return m_channels * m_frames;
	}
	MPT_FORCEINLINE std::ptrdiff_t frame_stride() const noexcept {
		return 1;
	}
};

struct audio_span_channels_are_planar_t {
};

inline constexpr audio_span_channels_are_planar_t audio_span_channels_are_planar;



// LxxxLxxxLxxxLxxxLxxx xRxxxRxxxRxxxRxxxRxx
template <typename SampleType>
struct audio_span_planar_strided {
public:
	using sample_type = SampleType;

private:
	sample_type * const * m_buffers;
	std::ptrdiff_t m_frame_stride;
	std::size_t m_channels;
	std::size_t m_frames;

public:
	MPT_CONSTEXPRINLINE audio_span_planar_strided(sample_type * const * buffers, std::size_t channels, std::size_t frames, std::ptrdiff_t frame_stride) noexcept
		: m_buffers(buffers)
		, m_frame_stride(frame_stride)
		, m_channels(channels)
		, m_frames(frames) {
		return;
	}
	MPT_FORCEINLINE sample_type * const * data_planar() const noexcept {
		return m_buffers;
	}
	MPT_FORCEINLINE sample_type * data() const noexcept {
		return nullptr;
	}
	MPT_FORCEINLINE sample_type & operator()(std::size_t channel, std::size_t frame) const {
		return m_buffers[channel][static_cast<std::ptrdiff_t>(frame) * m_frame_stride];
	}
	MPT_FORCEINLINE bool is_contiguous() const noexcept {
		return false;
	}
	MPT_FORCEINLINE bool channels_are_contiguous() const noexcept {
		return false;
	}
	MPT_FORCEINLINE bool frames_are_contiguous() const noexcept {
		return false;
	}
	MPT_FORCEINLINE std::size_t size_channels() const noexcept {
		return m_channels;
	}
	MPT_FORCEINLINE std::size_t size_frames() const noexcept {
		return m_frames;
	}
	MPT_FORCEINLINE std::size_t size_samples() const noexcept {
		return m_channels * m_frames;
	}
	MPT_FORCEINLINE std::ptrdiff_t frame_stride() const noexcept {
		return m_frame_stride;
	}
};

struct audio_span_channels_are_planar_and_strided_t {
};

inline constexpr audio_span_channels_are_planar_and_strided_t audio_span_channels_are_planar_and_strided;



template <typename SampleType>
struct audio_span {
public:
	using sample_type = SampleType;

private:
	union {
		sample_type * const contiguous;
		sample_type * const * const planes;
	} m_buffer;
	std::ptrdiff_t m_frame_stride;
	std::ptrdiff_t m_channel_stride;
	std::size_t m_channels;
	std::size_t m_frames;

public:
	MPT_CONSTEXPRINLINE audio_span(audio_span_interleaved<sample_type> buffer) noexcept
		: m_frame_stride(static_cast<std::ptrdiff_t>(buffer.size_channels()))
		, m_channel_stride(1)
		, m_channels(buffer.size_channels())
		, m_frames(buffer.size_frames()) {
		m_buffer.contiguous = buffer.data();
	}
	MPT_CONSTEXPRINLINE audio_span(sample_type * buffer, std::size_t channels, std::size_t frames, audio_span_frames_are_contiguous_t) noexcept
		: m_frame_stride(static_cast<std::ptrdiff_t>(channels))
		, m_channel_stride(1)
		, m_channels(channels)
		, m_frames(frames) {
		m_buffer.contiguous = buffer;
	}
	MPT_CONSTEXPRINLINE audio_span(audio_span_contiguous<sample_type> buffer) noexcept
		: m_frame_stride(1)
		, m_channel_stride(buffer.size_frames())
		, m_channels(buffer.size_channels())
		, m_frames(buffer.size_frames()) {
		m_buffer.contiguous = buffer.data();
	}
	MPT_CONSTEXPRINLINE audio_span(sample_type * buffer, std::size_t channels, std::size_t frames, audio_span_channels_are_contiguous_t) noexcept
		: m_frame_stride(1)
		, m_channel_stride(static_cast<std::ptrdiff_t>(frames))
		, m_channels(channels)
		, m_frames(frames) {
		m_buffer.contiguous = buffer;
	}
	MPT_CONSTEXPRINLINE audio_span(audio_span_planar<sample_type> buffer) noexcept
		: m_frame_stride(1)
		, m_channel_stride(0)
		, m_channels(buffer.size_channels())
		, m_frames(buffer.size_frames()) {
		m_buffer.planes = buffer.data_planar();
	}
	MPT_CONSTEXPRINLINE audio_span(sample_type * const * planes, std::size_t channels, std::size_t frames, audio_span_channels_are_planar_t) noexcept
		: m_frame_stride(1)
		, m_channel_stride(0)
		, m_channels(channels)
		, m_frames(frames) {
		m_buffer.planes = planes;
	}
	MPT_CONSTEXPRINLINE audio_span(audio_span_planar_strided<sample_type> buffer) noexcept
		: m_frame_stride(static_cast<std::ptrdiff_t>(buffer.frame_stride()))
		, m_channel_stride(0)
		, m_channels(buffer.size_channels())
		, m_frames(buffer.size_frames()) {
		m_buffer.planes = buffer.data_planar();
	}
	MPT_CONSTEXPRINLINE audio_span(sample_type * const * planes, std::size_t channels, std::size_t frames, std::ptrdiff_t frame_stride, audio_span_channels_are_planar_and_strided_t) noexcept
		: m_frame_stride(frame_stride)
		, m_channel_stride(0)
		, m_channels(channels)
		, m_frames(frames) {
		m_buffer.planes = planes;
	}
	MPT_FORCEINLINE bool is_contiguous() const noexcept {
		return (m_channel_stride != 0);
	}
	MPT_FORCEINLINE sample_type * const * data_planar() const noexcept {
		return (!is_contiguous()) ? m_buffer.planes : nullptr;
	}
	MPT_FORCEINLINE sample_type * data() const noexcept {
		return is_contiguous() ? m_buffer.contiguous : nullptr;
	}
	MPT_FORCEINLINE sample_type & operator()(std::size_t channel, std::size_t frame) const {
		return is_contiguous() ? m_buffer.contiguous[(m_channel_stride * static_cast<std::ptrdiff_t>(channel)) + (m_frame_stride * static_cast<std::ptrdiff_t>(frame))] : m_buffer.planes[channel][frame * static_cast<std::ptrdiff_t>(m_frame_stride)];
	}
	MPT_FORCEINLINE bool channels_are_contiguous() const noexcept {
		return (m_frame_stride == 1);
	}
	MPT_FORCEINLINE bool frames_are_contiguous() const noexcept {
		return (m_channel_stride == 1);
	}
	MPT_FORCEINLINE std::size_t size_channels() const noexcept {
		return m_channels;
	}
	MPT_FORCEINLINE std::size_t size_frames() const noexcept {
		return m_frames;
	}
	MPT_FORCEINLINE std::size_t size_samples() const noexcept {
		return m_channels * m_frames;
	}
	MPT_FORCEINLINE std::ptrdiff_t frame_stride() const noexcept {
		return m_frame_stride;
	}
};



template <typename Taudio_span>
struct audio_span_with_offset {
public:
	using sample_type = typename Taudio_span::sample_type;

private:
	Taudio_span m_buffer;
	std::size_t m_offset;

public:
	MPT_CONSTEXPRINLINE audio_span_with_offset(Taudio_span buffer, std::size_t offsetFrames) noexcept
		: m_buffer(buffer)
		, m_offset(offsetFrames) {
		return;
	}
	MPT_FORCEINLINE sample_type * const * data_planar() const noexcept {
		return nullptr;
	}
	MPT_FORCEINLINE sample_type * data() const noexcept {
		if (!is_contiguous()) {
			return nullptr;
		}
		return m_buffer.data() + (size_channels() * m_offset);
	}
	MPT_FORCEINLINE sample_type & operator()(std::size_t channel, std::size_t frame) const {
		return m_buffer(channel, m_offset + frame);
	}
	MPT_FORCEINLINE bool is_contiguous() const noexcept {
		return m_buffer.is_contiguous() && m_buffer.frames_are_contiguous();
	}
	MPT_FORCEINLINE bool channels_are_contiguous() const noexcept {
		return m_buffer.channels_are_contiguous();
	}
	MPT_FORCEINLINE bool frames_are_contiguous() const noexcept {
		return m_buffer.frames_are_contiguous();
	}
	MPT_FORCEINLINE std::size_t size_channels() const noexcept {
		return m_buffer.size_channels();
	}
	MPT_FORCEINLINE std::size_t size_frames() const noexcept {
		return m_buffer.size_frames() - m_offset;
	}
	MPT_FORCEINLINE std::size_t size_samples() const noexcept {
		return size_channels() * size_frames();
	}
	MPT_FORCEINLINE std::ptrdiff_t frame_stride() const noexcept {
		return m_buffer.frame_stride();
	}
};



template <typename Tspan, typename Tfunc>
MPT_FORCEINLINE auto audio_span_optimized_visit(Tspan span, Tfunc && f) -> decltype(std::forward<Tfunc>(f)(span)) const {
	using sample_type = typename Tspan::sample_type;
	std::variant<Tspan, audio_span_interleaved<sample_type>, audio_span_contiguous<sample_type>, audio_span_planar<sample_type>, audio_span_planar_strided<sample_type>> v{span};
	if (span.data() && span.is_contiguous() && span.frames_are_contiguous()) {
		v = audio_span_interleaved<sample_type>{span.data(), span.size_channels(), span.size_frames()};
	} else if (span.data() && span.is_contiguous() && span.channels_are_contiguous()) {
		v = audio_span_contiguous<sample_type>{span.data(), span.size_channels(), span.size_frames()};
	} else if (span.data_planar() && span.channels_are_contiguous()) {
		v = audio_span_planar<sample_type>{span.data_planar(), span.size_channels(), span.size_frames()};
	} else if (span.data_planar()) {
		v = audio_span_planar_strided<sample_type>{span.data_planar(), span.size_channels(), span.size_frames(), span.frame_stride()};
	}
	return std::visit(std::forward<Tfunc>(f), v);
}



template <typename BufferType>
inline audio_span_with_offset<BufferType> make_audio_span_with_offset(BufferType buf, std::size_t offsetFrames) noexcept {
	assert(offsetFrames <= buf.size_frames());
	return audio_span_with_offset<BufferType>{buf, offsetFrames};
}



} // namespace MPT_INLINE_NS
} // namespace mpt



#endif // MPT_AUDIO_SPAN_HPP
