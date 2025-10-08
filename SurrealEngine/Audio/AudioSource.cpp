
#include "Precomp.h"
#include "AudioSource.h"
#include "Math/vec.h"
#include "resample/CDSPResampler.h"
#include "Utils/Exception.h"
#include <functional>

#ifdef _MSC_VER
#pragma warning(disable: 4267)
#pragma warning(disable: 4456)
#pragma warning(disable: 4706)
#pragma warning(disable: 4457)
#pragma warning(disable: 4245)
#pragma warning(disable: 4244)
#endif

// Downloaded from https://github.com/lieff/minimp3
#define MINIMP3_IMPLEMENTATION
#include "minimp3_ex.h"

#define DR_FLAC_IMPLEMENTATION
#include "dr_flac.h"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"

#include "stb_vorbis.h"

#include "libopenmpt/libopenmpt.hpp"

class Mp3AudioSource : public AudioSource
{
public:
	Mp3AudioSource(Array<uint8_t> filedata) : input(std::move(filedata))
	{
		int error = mp3dec_ex_open_buf(&decoder, input.data(), input.size(), MP3D_SEEK_TO_SAMPLE);
		if (error)
			Exception::Throw("mp3dec_ex_open_buf failed");

		memset(&frame_info, 0, sizeof(frame_info));
	}

	~Mp3AudioSource()
	{
		mp3dec_ex_close(&decoder);
	}

	int GetFrequency() override
	{
		return decoder.info.hz;
	}

	int GetChannels() override
	{
		return decoder.info.channels;
	}

	int GetSamples() override
	{
		return decoder.samples / decoder.info.channels;
	}

	void SeekToSample(uint64_t position) override
	{
		int error = mp3dec_ex_seek(&decoder, position);
		if (error)
			Exception::Throw("mp3dec_ex_seek failed");
	}

	size_t ReadSamples(float* output, size_t samples) override
	{
		size_t samplesrequested = samples;
		while (samples > 0)
		{
			if (available == 0)
			{
				if (!endofdata)
					ReadFrame(samples);
				else
					break;
			}

			size_t count = std::min(samples, available);

			for (size_t i = 0; i < count; i++)
			{
				output[i] = buffer[i] * (float)(1.0 / 32768.0);
			}

			output += count;
			buffer += count;
			available -= count;
			samples -= count;
		}
		return samplesrequested - samples;
	}

	void ReadFrame(size_t samples)
	{
		buffer = nullptr;
		available = mp3dec_ex_read_frame(&decoder, &buffer, &frame_info, samples);
		endofdata = (available == 0);
	}

	Array<uint8_t> input;
	mp3d_sample_t* buffer = nullptr;
	size_t available = 0;
	bool endofdata = false;
	mp3dec_ex_t decoder;
	mp3dec_frame_info_t frame_info;
};

class FlacAudioSource : public AudioSource
{
public:
	FlacAudioSource(Array<uint8_t> filedata) : filedata(std::move(filedata))
	{
		decoder = drflac_open(&FlacAudioSource::StaticInputRead, &FlacAudioSource::StaticInputSeek, this, nullptr);
		if (!decoder)
			Exception::Throw("Could not open flac file");
	}

	~FlacAudioSource()
	{
		drflac_close(decoder);
	}

	int GetFrequency() override
	{
		return decoder->sampleRate;
	}

	int GetChannels() override
	{
		return decoder->channels;
	}

	int GetSamples() override
	{
		return decoder->totalPCMFrameCount;
	}

	void SeekToSample(uint64_t position) override
	{
		if (drflac_seek_to_pcm_frame(decoder, position))
			eofdata = false;
	}

	size_t ReadSamples(float* output, size_t samples) override
	{
		if (!eofdata)
		{
			size_t samplesread = drflac_read_pcm_frames_f32(decoder, samples / decoder->channels, output) * decoder->channels;
			eofdata = (samplesread != samples);
			return samplesread;
		}
		else
		{
			return 0;
		}
	}

	size_t InputRead(void* pBufferOut, size_t bytesToRead)
	{
		size_t available = filedata.size() - inputpos;
		size_t count = std::min(bytesToRead, available);
		memcpy(pBufferOut, filedata.data() + inputpos, count);
		inputpos += count;
		return count;
	}

	drflac_bool32 InputSeek(int offset, drflac_seek_origin origin)
	{
		if (origin == drflac_seek_origin_start)
		{
			if (offset < 0 || offset >(int)filedata.size())
				return DRFLAC_FALSE;
			inputpos = offset;
			return DRFLAC_TRUE;
		}
		else if (origin == drflac_seek_origin_current)
		{
			if ((offset < 0 && (size_t)-offset > inputpos) || inputpos + offset > filedata.size())
				return DRFLAC_FALSE;
			inputpos = (size_t)((int64_t)inputpos + offset);
			return DRFLAC_TRUE;
		}
		else
		{
			return DRFLAC_FALSE;
		}
	}

	static size_t StaticInputRead(void* pUserData, void* pBufferOut, size_t bytesToRead)
	{
		return static_cast<FlacAudioSource*>(pUserData)->InputRead(pBufferOut, bytesToRead);
	}

	static drflac_bool32 StaticInputSeek(void* pUserData, int offset, drflac_seek_origin origin)
	{
		return static_cast<FlacAudioSource*>(pUserData)->InputSeek(offset, origin);
	}

	drflac* decoder = nullptr;
	Array<uint8_t> filedata;
	size_t inputpos = 0;
	bool eofdata = false;
};

class WavAudioSource : public AudioSource
{
public:
	WavAudioSource(Array<uint8_t> filedata) : filedata(std::move(filedata))
	{
		drwav_bool32 result = drwav_init_ex(&decoder, &WavAudioSource::StaticInputRead, &WavAudioSource::StaticInputSeek, nullptr, this, nullptr, 0, nullptr);
		if (!result)
			Exception::Throw("Could not open wav file");

		// drwav only supports 1 loop
		if (decoder.smpl.numSampleLoops == 1)
		{
			bIsLooped = true;
			loopStart = decoder.smpl.loops[0].start;
			loopEnd = decoder.smpl.loops[0].end;
		}
	}

	~WavAudioSource()
	{
		drwav_uninit(&decoder);
	}

	int GetFrequency() override
	{
		return decoder.sampleRate;
	}

	int GetChannels() override
	{
		return decoder.channels;
	}

	int GetSamples() override
	{
		return decoder.totalPCMFrameCount;
	}

	void SeekToSample(uint64_t position) override
	{
		if (drwav_seek_to_pcm_frame(&decoder, position))
			eofdata = false;
	}

	size_t ReadSamples(float* output, size_t samples) override
	{
		if (!eofdata)
		{
			size_t samplesread = drwav_read_pcm_frames_f32(&decoder, samples / decoder.channels, output) * decoder.channels;
			eofdata = (samplesread != samples);
			return samplesread;
		}
		else
		{
			return 0;
		}
	}

	size_t InputRead(void* pBufferOut, size_t bytesToRead)
	{
		size_t available = filedata.size() - inputpos;
		size_t count = std::min(bytesToRead, available);
		memcpy(pBufferOut, filedata.data() + inputpos, count);
		inputpos += count;
		return count;
	}

	drwav_bool32 InputSeek(int offset, drwav_seek_origin origin)
	{
		if (origin == drwav_seek_origin_start)
		{
			if (offset < 0 || offset >(int)filedata.size())
				return DRWAV_FALSE;
			inputpos = offset;
			return DRWAV_TRUE;
		}
		else if (origin == drwav_seek_origin_current)
		{
			if ((offset < 0 && (size_t)-offset > inputpos) || inputpos + offset > filedata.size())
				return DRWAV_FALSE;
			inputpos = (size_t)((int64_t)inputpos + offset);
			return DRWAV_TRUE;
		}
		else
		{
			return DRWAV_FALSE;
		}
	}

	static size_t StaticInputRead(void* pUserData, void* pBufferOut, size_t bytesToRead)
	{
		return static_cast<WavAudioSource*>(pUserData)->InputRead(pBufferOut, bytesToRead);
	}

	static drwav_bool32 StaticInputSeek(void* pUserData, int offset, drwav_seek_origin origin)
	{
		return static_cast<WavAudioSource*>(pUserData)->InputSeek(offset, origin);
	}

	drwav decoder = {};
	Array<uint8_t> filedata;
	size_t inputpos = 0;
	bool eofdata = false;
};

class OggAudioSource : public AudioSource
{
public:
	OggAudioSource(Array<uint8_t> input) : filedata(std::move(input))
	{
		int error = 0;
		handle = stb_vorbis_open_pushdata(filedata.data(), (int)filedata.size(), &stream_byte_offset, &error, nullptr);
		if (!handle)
			Exception::Throw("Unable to read ogg file");

		stream_info = stb_vorbis_get_info(handle);
	}

	~OggAudioSource()
	{
		if (handle)
			stb_vorbis_close(handle);
	}

	int GetFrequency() override
	{
		return stream_info.sample_rate;
	}

	int GetChannels() override
	{
		return stream_info.channels;
	}

	int GetSamples() override
	{
		return 0; // To do: how to get this
	}

	void SeekToSample(uint64_t pos) override
	{
		// Currently only support seeking to beginning of stream.
		if (pos != 0) return;

		if (handle)
			stb_vorbis_close(handle);
		handle = nullptr;
		stream_byte_offset = 0;

		int error = 0;
		handle = stb_vorbis_open_pushdata(filedata.data(), (int)filedata.size(), &stream_byte_offset, &error, nullptr);
		if (handle == nullptr)
			Exception::Throw("Unable to read ogg file");

		stream_info = stb_vorbis_get_info(handle);
		stream_eof = false;
	}

	size_t ReadSamples(float* output, size_t requestedsamples) override
	{
		int data_requested = requestedsamples / stream_info.channels;
		int data_left = data_requested;
		while (!stream_eof && data_left > 0)
		{
			while (pcm_position == pcm_samples)
			{
				pcm = nullptr;
				pcm_position = 0;
				pcm_samples = 0;
				int bytes_used = stb_vorbis_decode_frame_pushdata(handle, filedata.data() + stream_byte_offset, filedata.size() - stream_byte_offset, nullptr, &pcm, &pcm_samples);
				stream_byte_offset += bytes_used;
				if (bytes_used == 0 || stream_byte_offset == filedata.size())
				{
					stream_eof = true;
					break;
				}
			}

			int samples = pcm_samples - pcm_position;
			if (samples > data_left) samples = data_left;

			int buffer_pos = (data_requested - data_left) * stream_info.channels;
			int c = stream_info.channels;
			for (int j = 0; j < c; j++)
			{
				const float* src = &pcm[j][pcm_position];
				float* dst = &output[buffer_pos + j];
				for (int i = 0; i < samples; i++)
				{
					*dst = src[i];
					dst += c;
				}
			}

			pcm_position += samples;
			data_left -= samples;
		}

		return (data_requested - data_left) * stream_info.channels;
	}

	Array<uint8_t> filedata;
	bool stream_eof = false;

	stb_vorbis* handle = nullptr;
	stb_vorbis_info stream_info = {};
	int stream_byte_offset = 0;

	float** pcm = nullptr;
	int pcm_position = 0;
	int pcm_samples = 0;
};
/*
class DumbAudioSource : public AudioSource
{
public:
	DumbAudioSource(Array<uint8_t> initfiledata, bool loop, std::function<DUH*(DUMBFILE*)> readCallback) : filedata(std::move(initfiledata))
	{
		dfs.open = &DumbAudioSource::DfsOpen;
		dfs.skip = &DumbAudioSource::DfsSkip;
		dfs.getc = &DumbAudioSource::DfsGetc;
		dfs.getnc = &DumbAudioSource::DfsGetnc;
		dfs.close = &DumbAudioSource::DfsClose;
		dfs.seek = &DumbAudioSource::DfsSeek;
		dfs.get_size = &DumbAudioSource::DfsGetSize;

		handle = dumbfile_open_ex(this, &dfs);
		if (!handle)
			Exception::Throw("Could not open tracker file");

		duh = readCallback(handle);
		if (duh == nullptr)
		{
			dumbfile_close(handle);
			Exception::Throw("Could not read tracker file");
		}

		renderer = duh_start_sigrenderer(duh, 0, 2, 0);
		if (renderer == nullptr)
		{
			unload_duh(duh);
			dumbfile_close(handle);
			Exception::Throw("Could not tracker rendering");
		}

		DUMB_IT_SIGRENDERER* itsr = duh_get_it_sigrenderer(renderer);
		if (!loop)
		{
			dumb_it_set_loop_callback(itsr, &dumb_it_callback_terminate, nullptr);
			dumb_it_set_xm_speed_zero_callback(itsr, &dumb_it_callback_terminate, nullptr);
		}
		dumb_it_set_resampling_quality(itsr, DUMB_RQ_CUBIC);
	}

	~DumbAudioSource()
	{
		if (samplebuffers) destroy_sample_buffer(samplebuffers);
		duh_end_sigrenderer(renderer);
		unload_duh(duh);
		dumbfile_close(handle);
	}

	int GetFrequency() override
	{
		return 44100;
	}

	int GetChannels() override
	{
		return 2;
	}

	int GetSamples() override
	{
		return -1;
	}

	void SeekToSample(uint64_t position) override
	{
	}

	size_t ReadSamples(float* output, size_t samples) override
	{
		float volume = 1.0f;
		long result = duh_render_float(renderer, &samplebuffers, &samplebuffer_size, 32, volume, 65536.0f / GetFrequency(), samples / GetChannels(), output);
		if (result < 0) return 0;
		return result * GetChannels();
	}

	static void* DfsOpen(const char* filename)
	{
		return nullptr;
	}

	static void DfsClose(void* f)
	{
	}

	static int DfsSkip(void* f, dumb_off_t n)
	{
		DumbAudioSource* self = (DumbAudioSource*)f;
		if (self->filepointer + n > self->filedata.size() || self->seekerror)
			return -1;
		self->filepointer += n;
		return 0;
	}

	static int DfsGetc(void* f)
	{
		DumbAudioSource* self = (DumbAudioSource*)f;
		if (self->filepointer >= self->filedata.size() || self->seekerror)
			return -1;
		return self->filedata[self->filepointer++];
	}

	static dumb_ssize_t DfsGetnc(char* ptr, size_t n, void* f)
	{
		DumbAudioSource* self = (DumbAudioSource*)f;
		size_t bytes = std::min(self->filedata.size() - self->filepointer, n);
		if ((n > 0 && bytes == 0) || self->seekerror)
			return -1;
		memcpy(ptr, self->filedata.data() + self->filepointer, bytes);
		self->filepointer += n;
		return bytes;
	}

	static int DfsSeek(void* f, dumb_off_t offset)
	{
		DumbAudioSource* self = (DumbAudioSource*)f;
		if (offset < 0 || (size_t)offset > self->filedata.size())
		{
			self->seekerror = true; // "A value of offset < 0 shall set the file into an erroneous state from which no bytes can be read"
			return -1;
		}
		self->filepointer = offset;
		return 0;
	}

	static dumb_off_t DfsGetSize(void* f)
	{
		DumbAudioSource* self = (DumbAudioSource*)f;
		return self->filedata.size();
	}

	Array<uint8_t> filedata;
	size_t filepointer = 0;
	bool seekerror = false;

	sample_t** samplebuffers = nullptr;
	long samplebuffer_size = 0;

	DUMBFILE_SYSTEM dfs = {};
	DUMBFILE* handle = nullptr;
	DUH* duh = nullptr;
	DUH_SIGRENDERER* renderer = nullptr;
};*/

class OpenMPTAudioSource : public AudioSource
{
public:
	OpenMPTAudioSource(Array<uint8_t> initfiledata, bool loop, int subsong)
	{
		uint8_t* data = initfiledata.begin();
		size_t size = initfiledata.size();
		int probe_result = openmpt::probe_file_header(openmpt::probe_file_header_flags_default2, data, size);

		// TODO: should we handle probe_file_header_result_wantmoredata?
		if (probe_result != openmpt::probe_file_header_result_success)
			Exception::Throw("Module file unsupported by OpenMPT");

		// TODO: log param defaults to std::clog, do we want to keep it that way?
		module = new openmpt::module(data, size);

		subsong = clamp(subsong, 0, module->get_num_subsongs() - 1);
		if (subsong != 0)
			module->select_subsong(subsong);

		// TODO: mimic galaxy and look for the next non-empty pattern

		module->set_repeat_count(-1);
	}

	~OpenMPTAudioSource()
	{
		if (module)
			delete module;
	}

	int GetFrequency() override
	{
		return 44100;
	}

	int GetChannels() override
	{
		return 2;
	}

	int GetSamples() override
	{
		return -1;
	}

	void SeekToSample(uint64_t position) override
	{
	}

	size_t ReadSamples(float* output, size_t samples) override
	{
		if (!module)
			return 0;

		return module->read_interleaved_stereo(GetFrequency(), samples / GetChannels(), output);
	}

	openmpt::module* module;
};

std::unique_ptr<AudioSource> AudioSource::CreateMp3(Array<uint8_t> filedata)
{
	return std::make_unique<Mp3AudioSource>(std::move(filedata));
}

std::unique_ptr<AudioSource> AudioSource::CreateFlac(Array<uint8_t> filedata)
{
	return std::make_unique<FlacAudioSource>(std::move(filedata));
}

std::unique_ptr<AudioSource> AudioSource::CreateWav(Array<uint8_t> filedata)
{
	return std::make_unique<WavAudioSource>(std::move(filedata));
}

std::unique_ptr<AudioSource> AudioSource::CreateOgg(Array<uint8_t> filedata)
{
	return std::make_unique<OggAudioSource>(std::move(filedata));
}

std::unique_ptr<AudioSource> AudioSource::CreateMod(Array<uint8_t> filedata, bool loop, int subsong)
{
	return std::make_unique<OpenMPTAudioSource>(filedata, loop, subsong);
}

class ResampleAudioSource : public AudioSource
{
public:
	ResampleAudioSource(int targetFrequency, std::unique_ptr<AudioSource> audiosource) : targetFrequency(targetFrequency), source(std::move(audiosource))
	{
		channels.resize(source->GetChannels());
		for (auto& channel : channels)
		{
			channel.resampler = std::make_unique<r8b::CDSPResampler24>(source->GetFrequency(), targetFrequency, maxInSize);
			channel.inputbuffer.resize(maxInSize);
		}
		srcbuffer.resize(maxInSize * channels.size());
	}

	int GetFrequency()
	{
		return targetFrequency;
	}

	int GetChannels()
	{
		return source->GetChannels();
	}

	int GetSamples()
	{
		return (int)(source->GetSamples() * (int64_t)targetFrequency / source->GetFrequency());
	}

	void SeekToSample(uint64_t position)
	{
		source->SeekToSample(position * source->GetFrequency() / targetFrequency);
		for (auto& channel : channels)
			channel.resampler->clear();
	}

	size_t ReadSamples(float* output, size_t samples)
	{
		size_t samplesrequested = samples;
		while (samples > 0)
		{
			ProcessInput();

			size_t channelcount = channels.size();
			size_t count = GetOutputCount(samples / channelcount);
			if (count == 0)
				break;

			for (size_t i = 0; i < count; i++)
			{
				for (size_t c = 0; c < channelcount; c++)
				{
					*(output++) = (float)channels[c].outputbuffer[i];
				}
			}

			for (size_t c = 0; c < channelcount; c++)
			{
				channels[c].outputbuffer += count;
				channels[c].outputavailable -= count;
			}
			samples -= count * channels.size();
		}

		return samplesrequested - samples;
	}

	void ProcessInput()
	{
		size_t needed = srcbuffer.size();
		for (Channel& channel : channels)
			needed = std::min(needed, channel.inputbuffer.size() - channel.inputavailable);

		if (needed > 0 && !endofdata)
		{
			size_t available = source->ReadSamples(srcbuffer.data(), needed);
			endofdata = (available == 0);
			size_t channelcount = channels.size();
			auto src = srcbuffer.data();
			for (size_t c = 0; c < channelcount; c++)
			{
				double* inputbuffer = channels[c].inputbuffer.data();
				int pos = channels[c].inputavailable;
				for (size_t i = c; i < available; i += channelcount)
					inputbuffer[pos++] = src[i];
				channels[c].inputavailable = pos;
			}
		}

		for (Channel& channel : channels)
		{
			if (channel.outputavailable == 0 && channel.inputavailable > 0)
			{
				channel.outputavailable = channel.resampler->process(channel.inputbuffer.data(), channel.inputavailable, channel.outputbuffer);
				channel.inputavailable = 0;
			}
		}
	}

	size_t GetOutputCount(size_t samples)
	{
		size_t count = samples;
		for (auto& channel : channels)
			count = std::min((size_t)channel.outputavailable, count);
		return count;
	}

	struct Channel
	{
		Array<double> inputbuffer;
		int inputavailable = 0;
		double* outputbuffer = nullptr;
		int outputavailable = 0;
		std::unique_ptr<r8b::CDSPResampler> resampler;
	};

	Array<float> srcbuffer;
	Array<Channel> channels;
	int maxInSize = 4096;
	bool endofdata = false;

	int targetFrequency = 0;
	std::unique_ptr<AudioSource> source;
};

std::unique_ptr<AudioSource> AudioSource::CreateResampler(int targetFrequency, std::unique_ptr<AudioSource> source)
{
	return std::make_unique<ResampleAudioSource>(targetFrequency, std::move(source));
}
