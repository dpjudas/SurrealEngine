
#include "Precomp.h"
#include "AudioMixer.h"
#include "AudioSource.h"
#include "AudioPlayer.h"
#include "File.h"
#include "kissfft/kiss_fft.h"
#include <mutex>
#include <stdexcept>

class AudioMixerImpl;

class AudioSound
{
public:
	AudioSound(int mixing_frequency, std::unique_ptr<AudioSource> source)
	{
		duration = samples.size() / 44100.0f;

		if (source->GetChannels() == 1)
		{
			if (source->GetFrequency() != mixing_frequency)
			{
				source = AudioSource::CreateResampler(mixing_frequency, std::move(source));
			}

			samples.resize(source->GetSamples());
			samples.resize(source->ReadSamples(samples.data(), samples.size()));

			// Remove any audio pops at end of the sound that the resampler might have created
			if (samples.size() >= 16)
			{
				for (size_t i = 1; i < 16; i++)
				{
					float t = i / 16.0f;
					size_t pos = samples.size() - i;
					samples[pos] = samples[pos] * t;
				}
			}
		}
		else
		{
			throw std::runtime_error("Only mono sounds are supported");
		}
	}

	std::vector<float> samples;
	float duration = 0.0f;
};

class HRTF_Direction
{
public:
	void load(const std::string& filename)
	{
		auto file = File::open_existing(filename);

		uint32_t ChunkID = file->read_uint32(); // "RIFF"
		uint32_t ChunkSize = file->read_uint32();
		uint32_t Format = file->read_uint32(); // "WAVE"
		uint32_t FormatChunkID = file->read_uint32(); // "fmt "
		uint32_t FormatChunkSize = file->read_uint32();
		uint16_t AudioFormat = file->read_uint16();
		uint16_t NumChannels = file->read_uint16();
		uint32_t SampleRate = file->read_uint32();
		uint32_t ByteRate = file->read_uint32();
		uint16_t BlockAlign = file->read_uint16();
		uint16_t BitsPerSample = file->read_uint16();
		uint32_t DataChunkID = file->read_uint32(); // "data"
		uint32_t DataChunkSize = file->read_uint32();

		if (ChunkID != 0x46464952 || Format != 0x45564157 || FormatChunkID != 0x20746d66)
			throw std::runtime_error("Not a wav file");

		if (FormatChunkSize != 16 || DataChunkID != 0x61746164)
			throw std::runtime_error("Wav file subformat not supported");

		if (NumChannels != 2 || BitsPerSample != 16 || BlockAlign != 4)
			throw std::runtime_error("Wav file must be 16 bit stereo");

		std::vector<int16_t> samples(DataChunkSize / sizeof(int16_t));
		file->read(samples.data(), samples.size() * sizeof(int16_t));
		file.reset();

		size_t size = samples.size() / 2;
		if (size > 128)
			throw std::runtime_error("Invalid HRTF file");

		size_t points = 1024;

		std::vector<kiss_fft_cpx> left(points);
		std::vector<kiss_fft_cpx> right(points);
		memset(left.data(), 0, left.size() * sizeof(kiss_fft_cpx));
		memset(right.data(), 0, right.size() * sizeof(kiss_fft_cpx));

		for (size_t i = 0; i < size; i++)
		{
			left[i].r = samples[i << 1] * (1.0f / 32768.0f);
			right[i].r = samples[(i << 1) + 1] * (1.0f / 32768.0f);
		}

		left_freq = transform_channel(left);
		right_freq = transform_channel(right);
	}

	std::vector<kiss_fft_cpx> transform_channel(const std::vector<kiss_fft_cpx>& samples)
	{
		std::vector<kiss_fft_cpx> freq(samples.size());
		memset(freq.data(), 0, freq.size() * sizeof(kiss_fft_cpx));

		kiss_fft_cfg cfg = kiss_fft_alloc((int)samples.size(), 0, nullptr, nullptr);
		kiss_fft(cfg, samples.data(), freq.data());
		kiss_fft_free(cfg);

		return freq;
	}

	std::vector<kiss_fft_cpx> left_freq;
	std::vector<kiss_fft_cpx> right_freq;
};

class HRTF_Data
{
public:
	HRTF_Data()
	{
		data.resize(N_ELEV);
		for (int el_index = 0; el_index < N_ELEV; el_index++)
		{
			int nfaz = get_nfaz(el_index);
			data[el_index].resize(nfaz);
			for (int az_index = 0; az_index < nfaz; az_index++)
			{
				data[el_index][az_index].load(hrtf_name(el_index, az_index));
			}
		}

		cfg_forward = kiss_fft_alloc(get_nfft(), 0, nullptr, nullptr);
		cfg_inverse = kiss_fft_alloc(get_nfft(), 1, nullptr, nullptr);
	}

	~HRTF_Data()
	{
		kiss_fft_free(cfg_forward);
		kiss_fft_free(cfg_inverse);
	}

	int get_nfft() const
	{
		return (int)data.front().front().left_freq.size();
	}

	// Get the closest HRTF to the specified elevation and azimuth in degrees
	void get_hrtf(float elev, float azim, kiss_fft_cpx** p_left, kiss_fft_cpx** p_right)
	{
		// Clip angles and convert to indices
		int el_index = 0, az_index = 0;
		bool flip_flag = false;
		get_indices(elev, azim, &el_index, &az_index, &flip_flag);

		// Get data and flip channels if necessary
		auto hd = &data[el_index][az_index];
		if (flip_flag)
		{
			*p_left = hd->right_freq.data();
			*p_right = hd->left_freq.data();
		}
		else
		{
			*p_left = hd->left_freq.data();
			*p_right = hd->right_freq.data();
		}
	}

	kiss_fft_cfg cfg_forward;
	kiss_fft_cfg cfg_inverse;

private:
	// Return the number of azimuths actually stored in file system
	int get_nfaz(int el_index)
	{
		return (elev_data[el_index] / 2) + 1;
	}

	// Get (closest) elevation index for given elevation
	int get_el_index(float elev)
	{
		int el_index = (int)std::round((elev - MIN_ELEV) / ELEV_INC);
		if (el_index < 0)
			el_index = 0;
		else if (el_index >= N_ELEV)
			el_index = N_ELEV - 1;
		return el_index;
	}

	// For a given elevation and azimuth in degrees, return the indices for the proper HRTF
	// *p_flip will be set true if left and right channels need to be flipped
	void get_indices(float elev, float azim, int* p_el, int* p_az, bool* p_flip)
	{
		int el_index = get_el_index(elev);
		int naz = elev_data[el_index];
		int nfaz = get_nfaz(el_index);

		// Coerce azimuth into legal range and calculate flip if any
		azim = std::fmod(azim, 360.0f);
		if (azim < 0) azim += 360.0f;
		if (azim > 180.0f)
		{
			azim = 360.0f - azim;
			*p_flip = true;
		}

		// Now 0 <= azim <= 180. Calculate index and clip to legal range just to be sure
		int az_index = (int)std::round(azim / (360.0 / naz));
		if (az_index < 0)
			az_index = 0;
		else if (az_index >= nfaz)
			az_index = nfaz - 1;

		*p_el = el_index;
		*p_az = az_index;
	}

	// Convert index to elevation
	int index_to_elev(int el_index)
	{
		return MIN_ELEV + el_index * ELEV_INC;
	}

	// Convert index to azimuth
	int index_to_azim(int el_index, int az_index)
	{
		return (int)std::round(az_index * 360.0f / elev_data[el_index]);
	}

	// Return pathname of HRTF specified by indices
	std::string hrtf_name(int el_index, int az_index)
	{
		int elev = index_to_elev(el_index);
		int azim = index_to_azim(el_index, az_index);

		// zero prefix azim number
		std::string azimstr = std::to_string(azim);
		azimstr = std::string(3 - azimstr.size(), '0') + azimstr;

		std::string filename = "elev" + std::to_string(elev) + "/H" + std::to_string(elev) + "e" + azimstr + "a.wav";
		return FilePath::combine("C:\\Development\\UTEngine\\build\\diffuse", filename);
	}

	static const int MIN_ELEV = -40;
	static const int MAX_ELEV = 90;
	static const int ELEV_INC = 10;
	static const int N_ELEV = (((MAX_ELEV - MIN_ELEV) / ELEV_INC) + 1);

	// This array gives the total number of azimuths measured per elevation, and hence the AZIMUTH INCREMENT.
	// Note that only azimuths up to and including 180 degrees actually exist in file system (because the data is symmetrical).
	static int elev_data[N_ELEV];

	std::vector<std::vector<HRTF_Direction>> data;
};

int HRTF_Data::elev_data[N_ELEV] = { 56, 60, 72, 72, 72, 72, 72, 60, 56, 45, 36, 24, 12, 1 };

class ActiveSound
{
public:
	void* owner = nullptr;
	int slot = 0;
	AudioSound* sound = nullptr;
	vec3 location = vec3(0.0f);
	float volume = 0.0f;
	float radius = 0.0f;
	float pitch = 0.0f;

	double pos = 0;
};

class HRTFAudioSound
{
public:
	HRTFAudioSound(HRTF_Data* hrtf, const ActiveSound& sound) : hrtf(hrtf), sound(sound)
	{
		int nfft = hrtf->get_nfft();
		playPos = nfft;
		left.resize(nfft);
		right.resize(nfft);
		time_buf.resize(nfft);
		freq_buf.resize(nfft);
		workspace_buf.resize(nfft);
	}

	bool MixSounds(float* output, size_t samples, const mat4& viewport)
	{
		vec3 location = (viewport * vec4(sound.location, 1.0f)).xyz();
		float attenuation = sound.volume;
		if (sound.radius > 0.0f)
		{
			float distanceAttenuation = std::max(1.0f - length(location) / sound.radius, 0.0f);
			attenuation *= distanceAttenuation * distanceAttenuation;
		}
		attenuation = clamp(attenuation, 0.0f, 1.0f);

		while (samples > 0)
		{
			size_t available = std::min(left.size() - playPos, samples);
			if (available > 0)
			{
				const float* l = left.data() + playPos;
				const float* r = right.data() + playPos;
				for (size_t i = 0; i < available; i++)
				{
					*(output++) += *(l++) * attenuation;
					*(output++) += *(r++) * attenuation;
				}
				playPos += available;
				samples -= available;
			}
			else if (lastFrame)
			{
				break;
			}
			else
			{
				FillFrame();
				PositionFrame(location);
				playPos = 0;
			}
		}

		if (samples > 0)
		{
			for (size_t i = 0; i < samples * 2; i++)
			{
				output[i] = 0.0f;
			}
			return false;
		}

		return true;
	}

	ActiveSound sound;

private:
	HRTFAudioSound(const HRTFAudioSound&) = delete;
	HRTFAudioSound& operator=(const HRTFAudioSound&) = delete;

	std::vector<kiss_fft_cpx> time_buf, freq_buf, workspace_buf;
	std::vector<float> left, right;

	kiss_fft_cpx CMul(const kiss_fft_cpx& a, const kiss_fft_cpx& b)
	{
		kiss_fft_cpx c;
		c.r = a.r * b.r - a.i * b.i;
		c.i = a.i * b.r + a.r * b.i;
		return c;
	}

	// Optimized multiply of two conjugate symmetric arrays (c = a * b)
	void COptMul(const kiss_fft_cpx* a, const kiss_fft_cpx* b, kiss_fft_cpx* c, size_t count)
	{
		size_t half = count >> 1;
		for (size_t i = 0; i < half; i++)
		{
			c[i] = CMul(a[i], b[i]);
		}
		for (size_t i = 1; i < half; i++)
		{
			c[half + i].r = c[half - i].r;
			c[half + i].i = -c[half - i].i;
		}
	}

	void ApplyHRTF(float* samples, kiss_fft_cpx* hrtf)
	{
		size_t nfft = time_buf.size();

		COptMul(freq_buf.data(), hrtf, workspace_buf.data(), freq_buf.size());
		kiss_fft(this->hrtf->cfg_inverse, workspace_buf.data(), time_buf.data());

		for (size_t i = 0; i < nfft; i++)
			samples[i] = time_buf[i].r * (1.0f / nfft);
	}

#if 1
	void PositionFrame(const vec3& location)
	{
		size_t nfft = time_buf.size();

		if (sound.radius > 0.0f)
		{
			float elev = clamp(std::atan2(location.y, std::abs(location.z)) * 180.0f / 3.14159265359f, -90.0f, 90.0f);
			float azim = clamp(std::atan2(location.x, location.z) * 180.0f / 3.14159265359f, -180.0f, 180.0f);

			kiss_fft_cpx* leftHRTF = nullptr;
			kiss_fft_cpx* rightHRTF = nullptr;
			hrtf->get_hrtf(elev, azim, &leftHRTF, &rightHRTF);

			kiss_fft(hrtf->cfg_forward, time_buf.data(), freq_buf.data());

			ApplyHRTF(left.data(), leftHRTF);
			ApplyHRTF(right.data(), rightHRTF);
		}
		else
		{
			for (size_t i = 0; i < nfft; i++)
			{
				left[i] = time_buf[i].r;
				right[i] = time_buf[i].r;
			}
		}
	}
#else
	void PositionFrame(const vec3& location)
	{
		size_t nfft = time_buf.size();

		if (sound.radius > 0.0f)
		{
			float angle = std::atan2(location.x, std::abs(location.z));

			// Reduce spatialization effect when getting close to the center
			float despatializedRadius = sound.radius * 0.1f;
			float dist2 = dot(location, location);
			if (dist2 < despatializedRadius * despatializedRadius)
				angle *= std::sqrt(dist2) / despatializedRadius;

			float pan = clamp(angle * 2.0f / 3.14f, -1.0f, 1.0f);

			float leftVolume = clamp(std::min(1.0f - pan, 1.0f), 0.0f, 1.0f);
			float rightVolume = clamp(std::min(1.0f + pan, 1.0f), 0.0f, 1.0f);

			for (size_t i = 0; i < nfft; i++)
			{
				left[i] = time_buf[i].r * leftVolume;
				right[i] = time_buf[i].r * rightVolume;
			}
		}
		else
		{
			for (size_t i = 0; i < nfft; i++)
			{
				left[i] = time_buf[i].r;
				right[i] = time_buf[i].r;
			}
		}
	}
#endif

	void FillFrame()
	{
		const float* src = sound.sound->samples.data();
		double srcpos = sound.pos;
		double srcpitch = sound.pitch;
		int srcsize = (int)sound.sound->samples.size();
		int srcmax = srcsize - 1;
		if (srcmax >= 0)
		{
			size_t samples = time_buf.size();
			for (size_t i = 0; i < samples; i++)
			{
				int pos = (int)srcpos;
				int pos2 = pos + 1;
				float t = (float)(srcpos - pos);

				pos = std::min(pos, srcmax);
				pos2 = std::min(pos2, srcmax);

				time_buf[i].r = (src[pos] * t + src[pos2] * (1.0f - t));
				time_buf[i].i = 0.0f;

				srcpos += srcpitch;
			}

			sound.pos = srcpos;
			lastFrame = srcpos >= sound.sound->samples.size();
		}
		else
		{
			memset(time_buf.data(), 0, time_buf.size() * sizeof(kiss_fft_cpx));
		}
	}

	HRTF_Data* hrtf = nullptr;
	size_t playPos = 0;
	bool lastFrame = false;
};

class AudioMixerSource : public AudioSource
{
public:
	AudioMixerSource(AudioMixerImpl* mixer) : mixer(mixer) { }

	int GetFrequency() override;
	int GetChannels() override { return 2; }
	int GetSamples() override { return -1; }
	void SeekToSample(uint64_t position) override { }
	size_t ReadSamples(float* output, size_t samples) override;

	void TransferFromClient();
	void CopyMusic(float* output, size_t samples);
	void MixSounds(float* output, size_t samples);

	AudioMixerImpl* mixer = nullptr;
	HRTF_Data hrtf;
	std::map<std::pair<void*, int>, std::unique_ptr<HRTFAudioSound>> sounds;
	mat4 viewport = mat4::identity();
	std::unique_ptr<AudioSource> music;
};

class AudioMixerImpl : public AudioMixer
{
public:
	AudioMixerImpl()
	{
		player = AudioPlayer::Create(std::make_unique<AudioMixerSource>(this));
	}

	~AudioMixerImpl()
	{
		player.reset();
	}

	AudioSound* AddSound(std::unique_ptr<AudioSource> source) override
	{
		sounds.push_back(std::make_unique<AudioSound>(mixing_frequency, std::move(source)));
		return sounds.back().get();
	}

	float GetSoundDuration(AudioSound* sound) override
	{
		return sound->duration;
	}

	void PlaySound(void* owner, int slot, AudioSound* sound, const vec3& location, float volume, float radius, float pitch) override
	{
		ActiveSound s;
		s.owner = owner;
		s.slot = slot;
		s.sound = sound;
		s.location = location;
		s.volume = volume;
		s.radius = radius;
		s.pitch = pitch;
		client.sounds.push_back(std::move(s));
	}

	void PlayMusic(std::unique_ptr<AudioSource> source) override
	{
		if (source && source->GetChannels() != 2)
			throw std::runtime_error("Only stereo music is supported");

		if (source && source->GetFrequency() != mixing_frequency)
		{
			source = AudioSource::CreateResampler(mixing_frequency, std::move(source));
		}
		client.music = std::move(source);
		client.musicupdate = true;
	}

	void SetViewport(const mat4& worldToView) override
	{
		client.viewport = worldToView;
	}

	void Update() override
	{
		std::unique_lock<std::mutex> lock(mutex);
		for (ActiveSound& s : client.sounds)
			transfer.sounds.push_back(s);
		client.sounds.clear();
		transfer.viewport = client.viewport;
		if (client.musicupdate)
		{
			transfer.music = std::move(client.music);
			transfer.musicupdate = true;
		}
		client.music = {};
		client.musicupdate = false;
	}

	int mixing_frequency = 44100;

	std::vector<std::unique_ptr<AudioSound>> sounds;
	std::unique_ptr<AudioPlayer> player;

	struct
	{
		std::vector<ActiveSound> sounds;
		mat4 viewport;
		std::unique_ptr<AudioSource> music;
		bool musicupdate = false;
	} client, transfer;

	std::mutex mutex;
};

/////////////////////////////////////////////////////////////////////////////

std::unique_ptr<AudioMixer> AudioMixer::Create()
{
	return std::make_unique<AudioMixerImpl>();
}

/////////////////////////////////////////////////////////////////////////////

int AudioMixerSource::GetFrequency()
{
	return mixer->mixing_frequency;
}

size_t AudioMixerSource::ReadSamples(float* output, size_t samples)
{
	TransferFromClient();
	CopyMusic(output, samples);
	MixSounds(output, samples);
	return samples;
}

void AudioMixerSource::TransferFromClient()
{
	std::unique_lock<std::mutex> lock(mixer->mutex);
	for (ActiveSound& s : mixer->transfer.sounds)
		sounds[{ s.owner, s.slot }] = std::make_unique<HRTFAudioSound>(&hrtf, s);
	mixer->transfer.sounds.clear();
	viewport = mixer->transfer.viewport;
	if (mixer->transfer.musicupdate)
		music = std::move(mixer->transfer.music);
	mixer->transfer.musicupdate = false;
}

void AudioMixerSource::CopyMusic(float* output, size_t samples)
{
	size_t pos = 0;

	if (music && music->GetChannels() == 2) // Only support stereo music for now
	{
		pos = music->ReadSamples(output, samples);
	}

	for (size_t i = pos; i < samples; i++)
	{
		output[i] = 0.0f;
	}
}

void AudioMixerSource::MixSounds(float* output, size_t samples)
{
	samples /= 2;

	for (auto it = sounds.begin(); it != sounds.end();)
	{
		HRTFAudioSound& hrtf = *(it->second);
		if (hrtf.MixSounds(output, samples, viewport))
		{
			++it;
		}
		else
		{
			it = sounds.erase(it);
		}
	}
}
