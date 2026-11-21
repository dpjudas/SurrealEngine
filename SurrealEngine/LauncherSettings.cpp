
#include "Precomp.h"
#include "LauncherSettings.h"
#include "Utils/JsonValue.h"
#include "Utils/File.h"

LauncherSettings& LauncherSettings::Get()
{
	static LauncherSettings settings;
	return settings;
}

static std::string GetSettingsFilename()
{
	return (Directory::localAppData() / "SurrealEngine/Settings.json").string();
}

LauncherSettings::LauncherSettings()
{
	try
	{
		JsonValue settings = JsonValue::parse(File::read_all_text(GetSettingsFilename()));

		std::string rendevtype = settings["RenderDevice"]["Type"].to_string();
		if (rendevtype == "Vulkan")
			RenderDevice.Type = RenderDeviceType::Vulkan;
		else if (rendevtype == "D3D11")
			RenderDevice.Type = RenderDeviceType::D3D11;
		else if (rendevtype == "D3D12")
			RenderDevice.Type = RenderDeviceType::D3D12;
		else if (rendevtype == "OpenGL")
			RenderDevice.Type = RenderDeviceType::OpenGL;

		std::string rendevaa = settings["RenderDevice"]["Antialias"].to_string();
		if (rendevaa == "Off")
			RenderDevice.Antialias = AntialiasMode::Off;
		else if (rendevaa == "MSAA2x")
			RenderDevice.Antialias = AntialiasMode::MSAA2x;
		else if (rendevaa == "MSAA4x")
			RenderDevice.Antialias = AntialiasMode::MSAA4x;

		std::string rendevlight = settings["RenderDevice"]["Light"].to_string();
		if (rendevlight == "Normal")
			RenderDevice.Light = LightMode::Normal;
		else if (rendevlight == "OneX")
			RenderDevice.Light = LightMode::OneX;
		else if (rendevlight == "BrighterActors")
			RenderDevice.Light = LightMode::BrighterActors;

		std::string rendevgamma = settings["RenderDevice"]["Gamma"].to_string();
		if (rendevgamma == "D3D9")
			RenderDevice.Gamma = GammaMode::D3D9;
		else if (rendevgamma == "XOpenGL")
			RenderDevice.Gamma = GammaMode::XOpenGL;

		RenderDevice.GammaCorrectScreenshots = settings["RenderDevice"]["GammaCorrectScreenshots"].to_boolean();
		RenderDevice.UseVSync = settings["RenderDevice"]["UseVSync"].to_boolean();
		RenderDevice.Hdr = settings["RenderDevice"]["Hdr"].to_boolean();
		RenderDevice.HdrScale = settings["RenderDevice"]["HdrScale"].to_int();
		RenderDevice.Bloom = settings["RenderDevice"]["Bloom"].to_boolean();
		RenderDevice.BloomAmount = settings["RenderDevice"]["BloomAmount"].to_int();
		RenderDevice.UseDebugLayer = settings["RenderDevice"]["UseDebugLayer"].to_boolean();

		std::string audiodev = settings["AudioDevice"]["Driver"].to_string();

		if (audiodev == "OpenAL")
			AudioDevice.Type = AudioDeviceType::OpenAL;
		else if (audiodev == "AudioPlayer")
			AudioDevice.Type = AudioDeviceType::AudioPlayer;
		else if (audiodev == "None")
			AudioDevice.Type = AudioDeviceType::NullAudio;

		std::string outputRate = settings["AudioDevice"]["OutputRate"].to_string();

		if (outputRate == "11025Hz")
			AudioDevice.OutputRate = AudioOutputRate::OR_11025Hz;
		else if (outputRate == "22050Hz")
			AudioDevice.OutputRate = AudioOutputRate::OR_22050Hz;
		else if (outputRate == "44100Hz")
			AudioDevice.OutputRate = AudioOutputRate::OR_44100Hz;
		else if (outputRate == "48000Hz")
			AudioDevice.OutputRate = AudioOutputRate::OR_48000Hz;

		AudioDevice.UseFilter = settings["AudioDevice"]["UseFilter"].to_boolean();
		AudioDevice.UseSurround = settings["AudioDevice"]["UseSurround"].to_boolean();
		AudioDevice.UseStereo = settings["AudioDevice"]["UseStereo"].to_boolean();
		AudioDevice.UseSpatial = settings["AudioDevice"]["UseSpatial"].to_boolean();
		AudioDevice.UseReverb = settings["AudioDevice"]["UseReverb"].to_boolean();
		AudioDevice.Use3DHardware = settings["AudioDevice"]["Use3DHardware"].to_boolean();
		AudioDevice.ReverseStereo = settings["AudioDevice"]["ReverseStereo"].to_boolean();

		for (const JsonValue& jsonItem : settings["Games"]["SearchList"].items())
		{
			Games.SearchList.push_back(jsonItem.to_string());
		}
		Games.LastSelected = settings["Games"]["LastSelected"].to_int();
	}
	catch (...)
	{
	}
}

void LauncherSettings::Save()
{
	JsonValue rendev = JsonValue::object();
	JsonValue audiodev = JsonValue::object();

	switch (RenderDevice.Type)
	{
	default:
	case RenderDeviceType::Vulkan: rendev["Type"] = JsonValue::string("Vulkan"); break;
	case RenderDeviceType::D3D11: rendev["Type"] = JsonValue::string("D3D11"); break;
	case RenderDeviceType::D3D12: rendev["Type"] = JsonValue::string("D3D12"); break;
	case RenderDeviceType::OpenGL: rendev["Type"] = JsonValue::string("OpenGL"); break;
	}

	switch (RenderDevice.Antialias)
	{
	default:
	case AntialiasMode::Off: rendev["Antialias"] = JsonValue::string("Off"); break;
	case AntialiasMode::MSAA2x: rendev["Antialias"] = JsonValue::string("MSAA2x"); break;
	case AntialiasMode::MSAA4x: rendev["Antialias"] = JsonValue::string("MSAA4x"); break;
	}

	switch (RenderDevice.Light)
	{
	default:
	case LightMode::Normal: rendev["Light"] = JsonValue::string("Normal"); break;
	case LightMode::OneX: rendev["Light"] = JsonValue::string("OneX"); break;
	case LightMode::BrighterActors: rendev["Light"] = JsonValue::string("BrighterActors"); break;
	}

	switch (RenderDevice.Gamma)
	{
	default:
	case GammaMode::D3D9: rendev["Gamma"] = JsonValue::string("D3D9"); break;
	case GammaMode::XOpenGL: rendev["Gamma"] = JsonValue::string("XOpenGL"); break;
	}

	rendev["GammaCorrectScreenshots"] = JsonValue::boolean(RenderDevice.GammaCorrectScreenshots);
	rendev["UseVSync"] = JsonValue::boolean(RenderDevice.UseVSync);
	rendev["Hdr"] = JsonValue::boolean(RenderDevice.Hdr);
	rendev["HdrScale"] = JsonValue::number(RenderDevice.HdrScale);
	rendev["Bloom"] = JsonValue::boolean(RenderDevice.Bloom);
	rendev["BloomAmount"] = JsonValue::number(RenderDevice.BloomAmount);
	rendev["UseDebugLayer"] = JsonValue::boolean(RenderDevice.UseDebugLayer);

	switch (AudioDevice.Type)
	{
		case AudioDeviceType::NullAudio: audiodev["Driver"] = JsonValue::string("None"); break;
		case AudioDeviceType::OpenAL: audiodev["Driver"] = JsonValue::string("OpenAl"); break;
		case AudioDeviceType::AudioPlayer: audiodev["Driver"] = JsonValue::string("AudioPlayer"); break;
	}

	switch (AudioDevice.OutputRate)
	{
	case AudioOutputRate::OR_11025Hz: audiodev["OutputRate"] = JsonValue::string("11025Hz"); break;
	case AudioOutputRate::OR_22050Hz: audiodev["OutputRate"] = JsonValue::string("22050Hz"); break;
	case AudioOutputRate::OR_44100Hz: audiodev["OutputRate"] = JsonValue::string("44100Hz"); break;
	case AudioOutputRate::OR_48000Hz: audiodev["OutputRate"] = JsonValue::string("48000Hz"); break;
	}

	audiodev["UseFilter"] = JsonValue::boolean(AudioDevice.UseFilter);
	audiodev["UseSurround"] = JsonValue::boolean(AudioDevice.UseSurround);
	audiodev["UseStereo"] = JsonValue::boolean(AudioDevice.UseStereo);
	audiodev["UseSpatial"] = JsonValue::boolean(AudioDevice.UseSpatial);
	audiodev["UseReverb"] = JsonValue::boolean(AudioDevice.UseReverb);
	audiodev["Use3DHardware"] = JsonValue::boolean(AudioDevice.Use3DHardware);
	audiodev["ReverseStereo"] = JsonValue::boolean(AudioDevice.ReverseStereo);

	JsonValue games = JsonValue::object();
	games["SearchList"] = JsonValue::array(Games.SearchList);
	games["LastSelected"] = JsonValue::number(Games.LastSelected);

	JsonValue settings = JsonValue::object();
	settings["RenderDevice"] = std::move(rendev);
	settings["AudioDevice"] = std::move(audiodev);
	settings["Games"] = std::move(games);

	const std::string filename = GetSettingsFilename();
	Directory::create(fs::path(filename).parent_path().string());
	File::write_all_text(filename, settings.to_json(true));
}
