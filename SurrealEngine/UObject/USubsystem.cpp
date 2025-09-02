
#include "Precomp.h"
#include "USubsystem.h"
#include "ULevel.h"
#include "Engine.h"
#include "Package/PackageManager.h"

static float square(float x) { return x * x; }

std::string USurrealRenderDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	if (propertyName == "Translucency")
		return IniPropertyConverter<bool>::ToString(Translucency);
	else if (propertyName == "VolumetricLighting")
		return IniPropertyConverter<bool>::ToString(VolumetricLighting);
	else if (propertyName == "ShinySurfaces")
		return IniPropertyConverter<bool>::ToString(ShinySurfaces);
	else if (propertyName == "Coronas")
		return IniPropertyConverter<bool>::ToString(Coronas);
	else if (propertyName == "HighDetailActors")
		return IniPropertyConverter<bool>::ToString(HighDetailActors);

	LogMessage("Queried unknown property for SurrealRenderDevice: " + propertyName.ToString());
	return {};
}

void USurrealRenderDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "Translucency")
		Translucency = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "VolumetricLighting")
		VolumetricLighting = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "ShinySurfaces")
		ShinySurfaces = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "Coronas")
		Coronas = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "HighDetailActors")
		HighDetailActors = IniPropertyConverter<bool>::FromString(value);
	else
		LogMessage("Setting unknown property for SurrealRenderDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}

void USurrealRenderDevice::LoadProperties(const NameString& from)
{	
	NameString name_from = from;
	
	if (from == "")
		name_from = NameString(Class);

	Translucency = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Translucency", Translucency);
	VolumetricLighting = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "VolumetricLighting", VolumetricLighting);
	ShinySurfaces = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "ShinySurfaces", ShinySurfaces);
	Coronas = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Coronas", Coronas);
	HighDetailActors = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "HighDetailActors", HighDetailActors);
}

void USurrealRenderDevice::SaveConfig()
{
	engine->packages->SetIniValue("System", Class, "Translucency", IniPropertyConverter<bool>::ToString(Translucency));
	engine->packages->SetIniValue("System", Class, "VolumetricLighting", IniPropertyConverter<bool>::ToString(VolumetricLighting));
	engine->packages->SetIniValue("System", Class, "ShinySurfaces", IniPropertyConverter<bool>::ToString(ShinySurfaces));
	engine->packages->SetIniValue("System", Class, "Coronas", IniPropertyConverter<bool>::ToString(Coronas));
	engine->packages->SetIniValue("System", Class, "HighDetailActors", IniPropertyConverter<bool>::ToString(HighDetailActors));
}

/////////////////////////////////////////////////////////////////////////////

std::string USurrealAudioDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	else if (propertyName == "UseFilter")
		return IniPropertyConverter<bool>::ToString(UseFilter);
	else if (propertyName == "UseSurround")
		return IniPropertyConverter<bool>::ToString(UseSurround);
	else if (propertyName == "UseStereo")
		return IniPropertyConverter<bool>::ToString(UseStereo);
	else if (propertyName == "UseCDMusic")
		return IniPropertyConverter<bool>::ToString(UseCDMusic);
	else if (propertyName == "UseDigitalMusic")
		return IniPropertyConverter<bool>::ToString(UseDigitalMusic);
	else if (propertyName == "UseSpatial")
		return IniPropertyConverter<bool>::ToString(UseSpatial);
	else if (propertyName == "UseReverb")
		return IniPropertyConverter<bool>::ToString(UseReverb);
	else if (propertyName == "Use3dHardware")
		return IniPropertyConverter<bool>::ToString(Use3dHardware);
	else if (propertyName == "LowSoundQuality")
		return IniPropertyConverter<bool>::ToString(LowSoundQuality);
	else if (propertyName == "ReverseStereo")
		return IniPropertyConverter<bool>::ToString(ReverseStereo);
	else if (propertyName == "Latency")
		return IniPropertyConverter<int>::ToString(Latency);
	else if (propertyName == "OutputRate")
		return IniPropertyConverter<AudioFrequency>::ToString(OutputRate);
	else if (propertyName == "Channels")
		return IniPropertyConverter<int>::ToString(Channels);
	else if (propertyName == "MusicVolume")
		return IniPropertyConverter<uint8_t>::ToString(MusicVolume);
	else if (propertyName == "SoundVolume")
		return IniPropertyConverter<uint8_t>::ToString(SoundVolume);
	else if (propertyName == "AmbientFactor")
		return IniPropertyConverter<float>::ToString(AmbientFactor);

	LogMessage("Queried unknown property for SurrealAudioDevice: " + propertyName.ToString());
	return {};
}

void USurrealAudioDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "UseFilter")
		UseFilter = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseSurround")
		UseSurround = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseStereo")
		UseStereo = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseCDMusic")
		UseCDMusic = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseDigitalMusic")
		UseDigitalMusic = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseSpatial")
		UseSpatial = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseReverb")
		UseReverb = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "Use3dHardware")
		Use3dHardware = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "LowSoundQuality")
		LowSoundQuality = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "ReverseStereo")
		ReverseStereo = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "Latency")
		Latency = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "OutputRate")
		OutputRate = IniPropertyConverter<AudioFrequency>::FromString(value);
	else if (propertyName == "Channels")
		Channels = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "MusicVolume")
		MusicVolume = IniPropertyConverter<uint8_t>::FromString(value);
	else if (propertyName == "SoundVolume")
		SoundVolume = IniPropertyConverter<uint8_t>::FromString(value);
	else if (propertyName == "AmbientFactor")
		AmbientFactor = IniPropertyConverter<float>::FromString(value);
	else
		LogMessage("Setting unknown property for SurrealAudioDevice: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}

void USurrealAudioDevice::LoadProperties(const NameString& from)
{
	NameString name_from = from;

	if (from == "")
		name_from = NameString(Class);

	UseFilter = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseFilter", UseFilter);
	UseSurround = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseSurround", UseSurround);
	UseStereo = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseStereo", UseStereo);
	UseCDMusic = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseCDMusic", UseCDMusic);
	UseDigitalMusic = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseDigitalMusic", UseDigitalMusic);
	UseSpatial = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseSpatial", UseSpatial);
	UseReverb = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseReverb", UseReverb);
	Use3dHardware = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Use3dHardware", Use3dHardware);
	LowSoundQuality = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "LowSoundQuality", LowSoundQuality);
	ReverseStereo = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "ReverseStereo", ReverseStereo);
	Latency = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Latency", Latency);
	OutputRate = IniPropertyConverter<AudioFrequency>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "OutputRate", OutputRate);
	Channels = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Channels", Channels);
	MusicVolume = IniPropertyConverter<uint8_t>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "MusicVolume", MusicVolume);
	SoundVolume = IniPropertyConverter<uint8_t>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "SoundVolume", SoundVolume);
	AmbientFactor = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "AmbientFactor", AmbientFactor);
}

void USurrealAudioDevice::SaveConfig()
{
	engine->packages->SetIniValue("System", Class, "UseFilter", IniPropertyConverter<bool>::ToString(UseFilter));
	engine->packages->SetIniValue("System", Class, "UseSurround", IniPropertyConverter<bool>::ToString(UseSurround));
	engine->packages->SetIniValue("System", Class, "UseStereo", IniPropertyConverter<bool>::ToString(UseStereo));
	engine->packages->SetIniValue("System", Class, "UseCDMusic", IniPropertyConverter<bool>::ToString(UseCDMusic));
	engine->packages->SetIniValue("System", Class, "UseDigitalMusic", IniPropertyConverter<bool>::ToString(UseDigitalMusic));
	engine->packages->SetIniValue("System", Class, "UseSpatial", IniPropertyConverter<bool>::ToString(UseSpatial));
	engine->packages->SetIniValue("System", Class, "UseReverb", IniPropertyConverter<bool>::ToString(UseReverb));
	engine->packages->SetIniValue("System", Class, "Use3dHardware", IniPropertyConverter<bool>::ToString(Use3dHardware));
	engine->packages->SetIniValue("System", Class, "LowSoundQuality", IniPropertyConverter<bool>::ToString(LowSoundQuality));
	engine->packages->SetIniValue("System", Class, "ReverseStereo", IniPropertyConverter<bool>::ToString(ReverseStereo));
	engine->packages->SetIniValue("System", Class, "Latency", IniPropertyConverter<int>::ToString(Latency));
	engine->packages->SetIniValue("System", Class, "OutputRate", IniPropertyConverter<AudioFrequency>::ToString(OutputRate));
	engine->packages->SetIniValue("System", Class, "Channels", IniPropertyConverter<int>::ToString(Channels));
	engine->packages->SetIniValue("System", Class, "MusicVolume", IniPropertyConverter<uint8_t>::ToString(MusicVolume));
	engine->packages->SetIniValue("System", Class, "SoundVolume", IniPropertyConverter<uint8_t>::ToString(SoundVolume));
	engine->packages->SetIniValue("System", Class, "AmbientFactor", IniPropertyConverter<float>::ToString(AmbientFactor));
}

void USurrealAudioDevice::InitDevice()
{
	// TODO: Add configurable option for audio device
	// TODO: Add configurable option for audio output frequency
	// TODO: Add option for number of sound channels
	// TODO: Add option for music buffer count
	// TODO: Add option for music buffer size
	// m_Device = AudioDevice::Create(48000, 256, 16, 256);
	m_Device = AudioDevice::Create(OutputRate.frequency, 256, 16, 256);
}

void USurrealAudioDevice::SetViewport(UViewport* InViewport)
{
	if (m_Viewport != InViewport)
	{
		StopSounds();

		if (m_Viewport)
		{
			m_Device->PlayMusic({});
		}

		m_Viewport = InViewport;

		if (m_Viewport)
		{
			if (m_Viewport->Actor()->Song() && m_Viewport->Actor()->Transition() == MTRAN_None)
				m_Viewport->Actor()->Transition() = MTRAN_Instant;

			PlayingSounds.resize(Channels);
		}
	}
}

void USurrealAudioDevice::Update(const mat4& listener)
{
	StartAmbience();
	UpdateAmbience();
	UpdateSounds(listener);
	UpdateMusic();

	m_Device->SetMusicVolume(MusicVolume / 255.0f);
	m_Device->SetSoundVolume(SoundVolume / 255.0f);
	m_Device->Update();
}

void USurrealAudioDevice::StartAmbience()
{
	if (!m_Viewport->Actor())
		return;

	bool Realtime = m_Viewport->IsRealtime() && m_Viewport->Actor()->Level()->Pauser() == "";
	if (Realtime)
	{
		UActor* ViewActor = m_Viewport->Actor()->ViewTarget() ? m_Viewport->Actor()->ViewTarget() : m_Viewport->Actor();
		int actorIndex = 0;
		for (UActor* Actor : m_Viewport->Actor()->XLevel()->Actors)
		{
			if (Actor && Actor->AmbientSound() && dist_squared(ViewActor->Location(), Actor->Location()) <= square(Actor->WorldSoundRadius()))
			{
				int Id = actorIndex * 16 + SLOT_Ambient * 2;
				bool foundSound = false;
				for (size_t j = 0; j < PlayingSounds.size(); j++)
				{
					if (PlayingSounds[j].Id == Id)
					{
						foundSound = true;
						break;
					}
				}
				if (!foundSound)
					PlaySound(Actor, Id, Actor->AmbientSound(), Actor->Location(), AmbientFactor * Actor->SoundVolume() / 255.0f, Actor->WorldSoundRadius(), Actor->SoundPitch() / 64.0f);
			}
			actorIndex++;
		}
	}
}

void USurrealAudioDevice::UpdateAmbience()
{
	if (!m_Viewport->Actor())
		return;

	UActor* ViewActor = m_Viewport->Actor()->ViewTarget() ? m_Viewport->Actor()->ViewTarget() : m_Viewport->Actor();
	bool Realtime = m_Viewport->IsRealtime() && m_Viewport->Actor()->Level()->Pauser() == "";
	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		PlayingSound& Playing = PlayingSounds[i];
		if ((Playing.Id & 14) == SLOT_Ambient * 2)
		{
			if (Playing.Actor->bDeleteMe() || dist_squared(ViewActor->Location(), Playing.Actor->Location()) > square(Playing.Actor->WorldSoundRadius()) || Playing.Actor->AmbientSound() != Playing.Sound || !Realtime)
			{
				// Ambient sound went out of range
				StopSound(i);
			}
			else
			{
				// Update basic sound properties
				Playing.Volume = 2.0f * (AmbientFactor * Playing.Actor->SoundVolume() / 255.0f);
				Playing.Radius = Playing.Actor->WorldSoundRadius();
				Playing.Pitch = Playing.Actor->SoundPitch() / 64.0f;
			}
		}
	}
}

void USurrealAudioDevice::UpdateSounds(const mat4& listener)
{
	if (!m_Viewport->Actor())
		return;

	UActor* ViewActor = m_Viewport->Actor()->ViewTarget() ? m_Viewport->Actor()->ViewTarget() : m_Viewport->Actor();
	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		PlayingSound& Playing = PlayingSounds[i];

		if (Playing.Id != 0)
		{
			// Update positioning from actor, if available
			if (Playing.Actor)
				Playing.Location = Playing.Actor->Location();

			// Update the priority
			Playing.Priority = SoundPriority(m_Viewport, Playing.Location, Playing.Volume, Playing.Radius);

			// Update the sound.
			Playing.CurrentVolume = SoundVolume;
			if (Playing.Channel)
			{
				if (m_Device->IsPlaying(Playing.Channel))
				{
					m_Device->UpdateSound(Playing.Channel, Playing.Sound, Playing.Location, SoundVolume * 0.5f, Playing.Radius, Playing.Pitch);
				}
				else
				{
					PlayingSounds[i] = {};
				}
			}
			else
			{
				Playing.Channel = m_Device->PlaySound((int)i, Playing.Sound, Playing.Location, SoundVolume * 0.5f, Playing.Radius, Playing.Pitch);
			}
		}
	}
}

void USurrealAudioDevice::UpdateMusic()
{
	if (m_Viewport->Actor() && m_Viewport->Actor()->Transition() != MTRAN_None)
	{
		// To do: this needs to fade out the old song before switching

		if (CurrentSong)
		{
			m_Device->PlayMusic({});
			CurrentSong = nullptr;
		}

		CurrentSong = m_Viewport->Actor()->Song();
		CurrentSection = m_Viewport->Actor()->SongSection();

		if (CurrentSong && UseDigitalMusic)
		{
			int subsong = CurrentSection != 255 ? CurrentSection : 0;
			m_Device->PlayMusic(AudioSource::CreateMod(CurrentSong->Data, true, 0, subsong));
		}

		m_Viewport->Actor()->Transition() = MTRAN_None;
	}
}

bool USurrealAudioDevice::PlaySound(UActor* Actor, int Id, USound* Sound, vec3 Location, float Volume, float Radius, float Pitch)
{
	if (!m_Viewport || !Sound)
		return false;

	// Allocate a new slot if requested
	if ((Id & 14) == 2 * SLOT_None)
		Id = 16 * --FreeSlot;

	float Priority = SoundPriority(m_Viewport, Location, Volume, Radius);

	// If already playing, stop it
	size_t Index = PlayingSounds.size();
	float BestPriority = Priority;
	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		PlayingSound& Playing = PlayingSounds[i];
		if ((Playing.Id & ~1) == (Id & ~1))
		{
			// Skip if not interruptable.
			if (Id & 1)
				return 0;

			// Stop the sound.
			Index = i;
			break;
		}
		else if (Playing.Priority <= BestPriority)
		{
			Index = i;
			BestPriority = Playing.Priority;
		}
	}

	// If no sound, or its priority is overruled, stop it
	if (Index == PlayingSounds.size())
		return 0;

	Sound->GetSound();

	// Put the sound on the play-list
	StopSound(Index);
	PlayingSounds[Index] = PlayingSound(Actor, Id, Sound, Location, Volume, Radius, Pitch, Priority);

	return true;
}

void USurrealAudioDevice::NoteDestroy(UActor* Actor)
{
	for (size_t i = 0; i < PlayingSounds.size(); i++)
	{
		if (PlayingSounds[i].Actor == Actor)
		{
			if ((PlayingSounds[i].Id & 14) == SLOT_Ambient * 2)
			{
				// Stop ambient sound when actor dies
				StopSound(i);
			}
			else
			{
				// Unbind regular sounds from actors
				PlayingSounds[i].Actor = nullptr;
			}
		}
	}
}

void USurrealAudioDevice::StopSound(size_t index)
{
	PlayingSound& Playing = PlayingSounds[index];

	if (Playing.Channel)
	{
		m_Device->StopSound(Playing.Channel);
	}

	PlayingSounds[index] = {};
}

void USurrealAudioDevice::StopSounds()
{
	for (size_t i = 0; i < PlayingSounds.size(); i++)
		StopSound(i);
}

void USurrealAudioDevice::BreakpointTriggered()
{
	if (m_Device)
	{
		m_Device->SetSoundVolume(0.0f);
		m_Device->Update();
	}
}

void USurrealAudioDevice::AddStats(Array<std::string>& lines)
{
	const int bufsize = 1024;
	char buffer[bufsize];
	int index = 0;
	for (const PlayingSound& sound : PlayingSounds)
	{
		if (sound.Channel)
		{
			std::snprintf(buffer, bufsize - 1, "Channel %2i: Vol: %05.2f %s", index, sound.CurrentVolume, sound.Sound->Name.ToString().c_str());
		}
		else
		{
			if (index >= 10)
				std::snprintf(buffer, bufsize - 1, "Channel %i:  None", index);
			else
				std::snprintf(buffer, bufsize - 1, "Channel %i: None", index);
		}
		buffer[bufsize - 1] = 0;
		lines.push_back(buffer);
		index++;
	}
}

float USurrealAudioDevice::SoundPriority(UViewport* Viewport, vec3 Location, float Volume, float Radius)
{
	UActor* target = Viewport->Actor();
	if (target && Viewport->Actor()->ViewTarget())
		target = Viewport->Actor()->ViewTarget();
	return target ? std::max(Volume * (1.0f - length(Location - target->Location()) / Radius), 0.0f) : 0.0f;
}

/////////////////////////////////////////////////////////////////////////////

std::string USurrealNetworkDevice::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";

	LogMessage("Queried unknown property for SurrealNetworkDevice: " + propertyName.ToString());
	return {};
}

void USurrealNetworkDevice::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	LogMessage("Setting unknown property for SurrealNetworkDevice: " + propertyName.ToString());
	engine->packages->SetIniValue("System", Class, propertyName, value);
}

/////////////////////////////////////////////////////////////////////////////

void USurrealClient::LoadProperties(const NameString& from)
{
	NameString name_from = from;

	if (from == "")
		name_from = NameString(Class);

	if (engine->LaunchInfo.engineVersion > 219)
	{
		StartupFullscreen = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "StartupFullscreen", StartupFullscreen);
		WindowedViewportX = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "WindowedViewportX", WindowedViewportX);
		WindowedViewportY = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "WindowedViewportY", WindowedViewportY);
		WindowedColorBits = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "WindowedColorBits", WindowedColorBits);
		FullscreenViewportX = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "FullscreenViewportX", FullscreenViewportX);
		FullscreenViewportY = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "FullscreenViewportY", FullscreenViewportY);
		FullscreenColorBits = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "FullscreenColorBits", FullscreenColorBits);
		Brightness = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Brightness", Brightness);
		UseJoystick = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseJoystick", UseJoystick);
		UseDirectInput = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "UseDirectInput", UseDirectInput);
		MinDesiredFrameRate = IniPropertyConverter<int>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "MinDesiredFrameRate", MinDesiredFrameRate);
		Decals = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "Decals", Decals);
		NoDynamicLights = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "NoDynamicLights", NoDynamicLights);
		TextureDetail = IniPropertyConverter<std::string>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "TextureDetail", TextureDetail);
		SkinDetail = IniPropertyConverter<std::string>::FromIniFile(*engine->packages->GetIniFile("System"), name_from, "SkinDetail", SkinDetail);
	}
}

void USurrealClient::SaveConfig()
{
	if (engine->LaunchInfo.engineVersion > 219)
	{
		engine->packages->SetIniValue("System", Class, "StartupFullscreen", IniPropertyConverter<bool>::ToString(StartupFullscreen));
		engine->packages->SetIniValue("System", Class, "WindowedViewportX", IniPropertyConverter<int>::ToString(WindowedViewportX));
		engine->packages->SetIniValue("System", Class, "WindowedViewportY", IniPropertyConverter<int>::ToString(WindowedViewportY));
		engine->packages->SetIniValue("System", Class, "WindowedColorBits", IniPropertyConverter<int>::ToString(WindowedColorBits));
		engine->packages->SetIniValue("System", Class, "FullscreenViewportX", IniPropertyConverter<int>::ToString(FullscreenViewportX));
		engine->packages->SetIniValue("System", Class, "FullscreenViewportY", IniPropertyConverter<int>::ToString(FullscreenViewportY));
		engine->packages->SetIniValue("System", Class, "FullscreenColorBits", IniPropertyConverter<int>::ToString(FullscreenColorBits));
		engine->packages->SetIniValue("System", Class, "Brightness", IniPropertyConverter<float>::ToString(Brightness));
		engine->packages->SetIniValue("System", Class, "UseJoystick", IniPropertyConverter<bool>::ToString(UseJoystick));
		engine->packages->SetIniValue("System", Class, "UseDirectInput", IniPropertyConverter<bool>::ToString(UseDirectInput));
		engine->packages->SetIniValue("System", Class, "MinDesiredFrameRate", IniPropertyConverter<int>::ToString(MinDesiredFrameRate));
		engine->packages->SetIniValue("System", Class, "Decals", IniPropertyConverter<bool>::ToString(Decals));
		engine->packages->SetIniValue("System", Class, "NoDynamicLights", IniPropertyConverter<bool>::ToString(NoDynamicLights));
		engine->packages->SetIniValue("System", Class, "TextureDetail", TextureDetail);
		engine->packages->SetIniValue("System", Class, "SkinDetail", SkinDetail);
	}
}

std::string USurrealClient::GetPropertyAsString(const NameString& propertyName) const
{
	if (propertyName == "Class")
		return "class'" + Class + "'";
	else if (propertyName == "StartupFullscreen")
		return IniPropertyConverter<bool>::ToString(StartupFullscreen);
	else if (propertyName == "WindowedViewportX")
		return IniPropertyConverter<int>::ToString(WindowedViewportX);
	else if (propertyName == "WindowedViewportY")
		return IniPropertyConverter<int>::ToString(WindowedViewportY);
	else if (propertyName == "WindowedColorBits")
		return IniPropertyConverter<int>::ToString(WindowedColorBits);
	else if (propertyName == "FullscreenViewportX")
		return IniPropertyConverter<int>::ToString(FullscreenViewportX);
	else if (propertyName == "FullscreenViewportY")
		return IniPropertyConverter<int>::ToString(FullscreenViewportY);
	else if (propertyName == "FullscreenColorBits")
		return IniPropertyConverter<int>::ToString(FullscreenColorBits);
	else if (propertyName == "Brightness")
		return IniPropertyConverter<float>::ToString(Brightness);
	else if (propertyName == "UseJoystick")
		return IniPropertyConverter<bool>::ToString(UseJoystick);
	else if (propertyName == "UseDirectInput")
		return IniPropertyConverter<bool>::ToString(UseDirectInput);
	else if (propertyName == "MinDesiredFrameRate")
		return IniPropertyConverter<int>::ToString(MinDesiredFrameRate);
	else if (propertyName == "Decals")
		return IniPropertyConverter<bool>::ToString(Decals);
	else if (propertyName == "NoDynamicLights")
		return IniPropertyConverter<bool>::ToString(NoDynamicLights);
	else if (propertyName == "TextureDetail")
		return TextureDetail;
	else if (propertyName == "SkinDetail")
		return SkinDetail;

	LogMessage("Queried unknown property for Surreal.ViewportManager: " + propertyName.ToString());
	return {};
}

void USurrealClient::SetPropertyFromString(const NameString& propertyName, const std::string& value)
{
	if (propertyName == "WindowedViewportX")
		WindowedViewportX = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "WindowedViewportY")
		WindowedViewportY = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "WindowedColorBits")
		WindowedColorBits = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "FullscreenViewportX")
		FullscreenViewportX = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "FullscreenViewportY")
		FullscreenViewportY = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "FullscreenColorBits")
		FullscreenColorBits = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "Brightness")
		Brightness = IniPropertyConverter<float>::FromString(value);
	else if (propertyName == "UseJoystick")
		UseJoystick = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "UseDirectInput")
		UseDirectInput = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "MinDesiredFrameRate")
		MinDesiredFrameRate = IniPropertyConverter<int>::FromString(value);
	else if (propertyName == "Decals")
		Decals = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "NoDynamicLights")
		NoDynamicLights = IniPropertyConverter<bool>::FromString(value);
	else if (propertyName == "TextureDetail")
		TextureDetail = value;
	else if (propertyName == "SkinDetail")
		SkinDetail = value;
	else
		LogMessage("Setting unknown property for Surreal.ViewportManager: " + propertyName.ToString());

	engine->packages->SetIniValue("System", Class, propertyName, value);
}
