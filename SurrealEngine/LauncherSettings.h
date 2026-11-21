#pragma once

enum class RenderDeviceType
{
	Vulkan,
	D3D11,
	D3D12,
	OpenGL
};

enum class AntialiasMode
{
	Off,
	MSAA2x,
	MSAA4x
};

enum class LightMode
{
	Normal,
	OneX,
	BrighterActors
};

enum class GammaMode
{
	D3D9,
	XOpenGL
};

enum class AudioDeviceType
{
	NullAudio,
	OpenAL,
	AudioPlayer,
};

enum class AudioOutputRate
{
	OR_11025Hz,
	OR_22050Hz,
	OR_44100Hz,
	OR_48000Hz
};

class LauncherSettings
{
public:
	static LauncherSettings& Get();
	void Save();

	struct
	{
		RenderDeviceType Type = RenderDeviceType::Vulkan;
		bool UseVSync = true;
		AntialiasMode Antialias = AntialiasMode::MSAA4x;
		LightMode Light = LightMode::Normal;
		GammaMode Gamma = GammaMode::D3D9;
		bool GammaCorrectScreenshots = false;
		bool Hdr = false;
		int HdrScale = 128;
		bool Bloom = false;
		int BloomAmount = 128;
		bool UseDebugLayer = false;
	} RenderDevice;

	struct
	{
		AudioDeviceType Type = AudioDeviceType::OpenAL;
		AudioOutputRate OutputRate = AudioOutputRate::OR_44100Hz;
		bool UseFilter = true;
		bool UseSurround = true;
		bool UseStereo = true;
		bool UseSpatial = true;
		bool UseReverb = true;
		bool Use3DHardware = true;
		bool ReverseStereo = false;
	} AudioDevice;

	struct
	{
		Array<std::string> SearchList;
		int LastSelected = -1;
	} Games;

private:
	LauncherSettings();
};
