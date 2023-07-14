
#include "Precomp.h"
#include "USound.h"
#include "Engine.h"
#include "Audio/AudioSource.h"
#include "Audio/AudioMixer.h"
#include "Audio/AudioSubsystem.h"

void USound::Load(ObjectStream* stream)
{
	UObject::Load(stream);

	Format = stream->ReadName();
	if (stream->GetVersion() >= 63)
		stream->ReadUInt32(); // lazy array skip offset
	uint32_t size = stream->ReadIndex();
	Data.resize(size);
	stream->ReadBytes(Data.data(), size);
}

AudioSound* USound::GetSound()
{
	if (Sound)
		return Sound;

	// Search for smpl chunk in wav file to find its loop flags, if any:
	// (yes, this code is very ugly but obviously I don't care anymore)
	AudioLoopInfo loopinfo;
	if (Data.size() > 44 && memcmp(Data.data(), "RIFF", 4) == 0 && memcmp(Data.data() + 8, "WAVE", 4) == 0)
	{
		size_t pos = 12;
		while (pos + 8 < Data.size())
		{
			if (memcmp(Data.data() + pos, "smpl", 4) == 0 && pos + 0x2c <= Data.size())
			{
				uint32_t chunksize = *(uint32_t*)(Data.data() + pos + 4);
				size_t endpos = pos + 8 + (size_t)chunksize;

				uint32_t sampleLoops = *(uint32_t*)(Data.data() + pos + 0x24);
				pos += 0x2c;

				if (sampleLoops != 0 && pos + 0x18 <= Data.size())
				{
					uint32_t type = *(uint32_t*)(Data.data() + pos + 0x04);
					uint32_t start = *(uint32_t*)(Data.data() + pos + 0x08);
					uint32_t end = *(uint32_t*)(Data.data() + pos + 0x0c);

					loopinfo.Looped = true;
					// if (type & 1)
					//	loopinfo.BidiLoop = true;
					loopinfo.LoopStart = start;
					loopinfo.LoopEnd = end;
				}

				pos = endpos;
			}
			else
			{
				uint32_t chunksize = *(uint32_t*)(Data.data() + pos + 4);
				pos += 8 + (size_t)chunksize;
			}
		}
	}

	Sound = engine->audio->GetMixer()->AddSound(AudioSource::CreateWav(Data), loopinfo);
	return Sound;
}
