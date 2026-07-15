
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

		for (const JsonValue& jsonItem : settings["Games"]["SearchList"].items())
		{
			Games.SearchList.push_back(jsonItem.to_string());
		}
		Games.LastSelected = settings["Games"]["LastSelected"].to_int();

		VR.Enabled = settings["VR"]["Enabled"].to_boolean();
		// A missing key reads back as 0, which would mean a zero-sized eye swapchain, so keep the
		// default unless the file actually holds something usable.
		int vrRenderScale = settings["VR"]["RenderScale"].to_int();
		if (vrRenderScale > 0)
			VR.RenderScale = vrRenderScale;

		std::string vrMoveRef = settings["VR"]["MovementReference"].to_string();
		if (vrMoveRef == "Controller")
			VR.MovementReference = VRMovementReference::Controller;
		else if (vrMoveRef == "Head")
			VR.MovementReference = VRMovementReference::Head;

		std::string vrMoveHand = settings["VR"]["MovementHand"].to_string();
		if (vrMoveHand == "Left")
			VR.MovementHand = VRHand::Left;
		else if (vrMoveHand == "Right")
			VR.MovementHand = VRHand::Right;

		// Defaults to whatever hand the stick is on, so a settings file written before this key existed
		// keeps behaving the way it did.
		VR.MovementDirectionHand = VR.MovementHand;
		std::string vrMoveDirHand = settings["VR"]["MovementDirectionHand"].to_string();
		if (vrMoveDirHand == "Left")
			VR.MovementDirectionHand = VRHand::Left;
		else if (vrMoveDirHand == "Right")
			VR.MovementDirectionHand = VRHand::Right;

		std::string vrTurnMode = settings["VR"]["TurnMode"].to_string();
		if (vrTurnMode == "Snap")
			VR.TurnMode = VRTurnMode::Snap;
		else if (vrTurnMode == "Smooth")
			VR.TurnMode = VRTurnMode::Smooth;
		else if (vrTurnMode == "Off")
			VR.TurnMode = VRTurnMode::Off;

		// Same missing-key-reads-as-0 caveat as RenderScale: a 0 degree snap or turn rate would silently
		// disable turning rather than fall back to something usable.
		int vrSnapTurn = settings["VR"]["SnapTurnDegrees"].to_int();
		if (vrSnapTurn > 0)
			VR.SnapTurnDegrees = vrSnapTurn;
		int vrSmoothTurn = settings["VR"]["SmoothTurnDegreesPerSecond"].to_int();
		if (vrSmoothTurn > 0)
			VR.SmoothTurnDegreesPerSecond = vrSmoothTurn;

		// Defaults to true, so an explicit false has to survive a round trip - but so does a settings file
		// written before this key existed, where a missing key reads back as false. Only take the stored
		// value when the key is actually present.
		if (settings["VR"].properties().find("RoomScaleMovement") != settings["VR"].properties().end())
			VR.RoomScaleMovement = settings["VR"]["RoomScaleMovement"].to_boolean();
	}
	catch (...)
	{
	}
}

void LauncherSettings::Save()
{
	JsonValue rendev = JsonValue::object();

	switch (RenderDevice.Type)
	{
	default:
	case RenderDeviceType::Vulkan: rendev["Type"] = JsonValue::string("Vulkan"); break;
	case RenderDeviceType::D3D11: rendev["Type"] = JsonValue::string("D3D11"); break;
	case RenderDeviceType::D3D12: rendev["Type"] = JsonValue::string("D3D12"); break;
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

	JsonValue games = JsonValue::object();
	games["SearchList"] = JsonValue::array(Games.SearchList);
	games["LastSelected"] = JsonValue::number(Games.LastSelected);

	JsonValue vr = JsonValue::object();
	vr["Enabled"] = JsonValue::boolean(VR.Enabled);
	vr["RenderScale"] = JsonValue::number(VR.RenderScale);

	switch (VR.MovementReference)
	{
	default:
	case VRMovementReference::Controller: vr["MovementReference"] = JsonValue::string("Controller"); break;
	case VRMovementReference::Head: vr["MovementReference"] = JsonValue::string("Head"); break;
	}

	switch (VR.MovementHand)
	{
	default:
	case VRHand::Left: vr["MovementHand"] = JsonValue::string("Left"); break;
	case VRHand::Right: vr["MovementHand"] = JsonValue::string("Right"); break;
	}

	switch (VR.MovementDirectionHand)
	{
	default:
	case VRHand::Left: vr["MovementDirectionHand"] = JsonValue::string("Left"); break;
	case VRHand::Right: vr["MovementDirectionHand"] = JsonValue::string("Right"); break;
	}

	switch (VR.TurnMode)
	{
	default:
	case VRTurnMode::Snap: vr["TurnMode"] = JsonValue::string("Snap"); break;
	case VRTurnMode::Smooth: vr["TurnMode"] = JsonValue::string("Smooth"); break;
	case VRTurnMode::Off: vr["TurnMode"] = JsonValue::string("Off"); break;
	}

	vr["SnapTurnDegrees"] = JsonValue::number(VR.SnapTurnDegrees);
	vr["SmoothTurnDegreesPerSecond"] = JsonValue::number(VR.SmoothTurnDegreesPerSecond);
	vr["RoomScaleMovement"] = JsonValue::boolean(VR.RoomScaleMovement);

	JsonValue settings = JsonValue::object();
	settings["RenderDevice"] = std::move(rendev);
	settings["Games"] = std::move(games);
	settings["VR"] = std::move(vr);

	const std::string filename = GetSettingsFilename();
	Directory::create(fs::path(filename).parent_path().string());
	File::write_all_text(filename, settings.to_json(true));
}
