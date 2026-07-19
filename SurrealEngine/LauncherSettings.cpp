
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

// Names used to serialize the per-hand/per-button control bindings. Order must match VRSubsystem::HandLeft
// /HandRight and the VRSubsystem::Button enum - the arrays are indexed by those directly.
static const char* const kVRHandNames[VRSubsystem::HandCount] = { "Left", "Right" };
static const char* const kVRButtonNames[VRSubsystem::ButtonCount] =
	{ "Trigger", "Grip", "A", "B", "ThumbstickClick", "Menu", "Trackpad" };

static std::string VRButtonKey(int hand, int button)
{
	return std::string(kVRHandNames[hand]) + "." + kVRButtonNames[button];
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

		const auto& vrProps = settings["VR"].properties();

		// [0, 1). A missing or out-of-range value keeps the default rather than pinning the stick centred
		// (deadzone >= 1) or letting it drift (< 0).
		if (vrProps.find("StickDeadzone") != vrProps.end())
		{
			float vrDeadzone = settings["VR"]["StickDeadzone"].to_float();
			if (vrDeadzone >= 0.0f && vrDeadzone < 1.0f)
				VR.StickDeadzone = vrDeadzone;
		}

		std::string vrHudHand = settings["VR"]["HudHand"].to_string();
		if (vrHudHand == "Left")
			VR.HudHand = VRHand::Left;
		else if (vrHudHand == "Right")
			VR.HudHand = VRHand::Right;

		std::string vrMenuHand = settings["VR"]["MenuPointerHand"].to_string();
		if (vrMenuHand == "Left")
			VR.MenuPointerHand = VRHand::Left;
		else if (vrMenuHand == "Right")
			VR.MenuPointerHand = VRHand::Right;

		std::string vrWeaponHand = settings["VR"]["WeaponHand"].to_string();
		if (vrWeaponHand == "Left")
			VR.WeaponHand = VRHand::Left;
		else if (vrWeaponHand == "Right")
			VR.WeaponHand = VRHand::Right;

		// Weapon placement offsets: 0 is a meaningful value (weapon on the raw grip pose), so keep the
		// default only when the key is genuinely absent rather than treating a stored 0 as unset.
		if (vrProps.find("WeaponForwardOffsetCm") != vrProps.end())
			VR.WeaponForwardOffsetCm = settings["VR"]["WeaponForwardOffsetCm"].to_int();
		if (vrProps.find("WeaponRightOffsetCm") != vrProps.end())
			VR.WeaponRightOffsetCm = settings["VR"]["WeaponRightOffsetCm"].to_int();
		if (vrProps.find("WeaponUpOffsetCm") != vrProps.end())
			VR.WeaponUpOffsetCm = settings["VR"]["WeaponUpOffsetCm"].to_int();
		if (vrProps.find("WeaponPitchOffsetDegrees") != vrProps.end())
			VR.WeaponPitchOffsetDegrees = settings["VR"]["WeaponPitchOffsetDegrees"].to_int();
		if (vrProps.find("WeaponYawOffsetDegrees") != vrProps.end())
			VR.WeaponYawOffsetDegrees = settings["VR"]["WeaponYawOffsetDegrees"].to_int();
		if (vrProps.find("WeaponRollOffsetDegrees") != vrProps.end())
			VR.WeaponRollOffsetDegrees = settings["VR"]["WeaponRollOffsetDegrees"].to_int();
		if (vrProps.find("WeaponScalePercent") != vrProps.end())
			VR.WeaponScalePercent = settings["VR"]["WeaponScalePercent"].to_int();

		// Wheel radius/deadzone: 0 would make the wheel unreachable or unusable, so only take a positive
		// stored value, same reasoning as HandColliderRadius below. EntryScalePercent has no such floor
		// (it's a plain percentage), so it follows the WeaponScalePercent pattern above instead.
		int vrWheelRadius = settings["VR"]["WheelRadiusCm"].to_int();
		if (vrWheelRadius > 0)
			VR.WheelRadiusCm = vrWheelRadius;
		int vrWheelDeadzone = settings["VR"]["WheelSelectDeadzoneCm"].to_int();
		if (vrWheelDeadzone > 0)
			VR.WheelSelectDeadzoneCm = vrWheelDeadzone;
		if (vrProps.find("WheelEntryScalePercent") != vrProps.end())
			VR.WheelEntryScalePercent = settings["VR"]["WheelEntryScalePercent"].to_int();
		if (vrProps.find("WheelItemScalePercent") != vrProps.end())
			VR.WheelItemScalePercent = settings["VR"]["WheelItemScalePercent"].to_int();
		if (vrProps.find("WheelIconScalePercent") != vrProps.end())
			VR.WheelIconScalePercent = settings["VR"]["WheelIconScalePercent"].to_int();

		// Item placement offsets: 0 is a meaningful value (item on the raw hand pose), same reasoning as
		// the weapon offsets above.
		if (vrProps.find("ItemForwardOffsetCm") != vrProps.end())
			VR.ItemForwardOffsetCm = settings["VR"]["ItemForwardOffsetCm"].to_int();
		if (vrProps.find("ItemRightOffsetCm") != vrProps.end())
			VR.ItemRightOffsetCm = settings["VR"]["ItemRightOffsetCm"].to_int();
		if (vrProps.find("ItemUpOffsetCm") != vrProps.end())
			VR.ItemUpOffsetCm = settings["VR"]["ItemUpOffsetCm"].to_int();
		if (vrProps.find("ItemPitchOffsetDegrees") != vrProps.end())
			VR.ItemPitchOffsetDegrees = settings["VR"]["ItemPitchOffsetDegrees"].to_int();
		if (vrProps.find("ItemYawOffsetDegrees") != vrProps.end())
			VR.ItemYawOffsetDegrees = settings["VR"]["ItemYawOffsetDegrees"].to_int();
		if (vrProps.find("ItemRollOffsetDegrees") != vrProps.end())
			VR.ItemRollOffsetDegrees = settings["VR"]["ItemRollOffsetDegrees"].to_int();
		if (vrProps.find("ItemScalePercent") != vrProps.end())
			VR.ItemScalePercent = settings["VR"]["ItemScalePercent"].to_int();

		// Missing-key-reads-as-0 again: a zero radius/size would mean an invisible, un-collidable hand or a
		// zero-area tablet, so only take stored values that are actually usable.
		int vrHandRadius = settings["VR"]["HandColliderRadius"].to_int();
		if (vrHandRadius > 0)
			VR.HandColliderRadius = vrHandRadius;
		if (vrProps.find("PickupHands") != vrProps.end())
			VR.PickupHands = std::clamp(settings["VR"]["PickupHands"].to_int(), 0, 2);
		// Both default false, so a missing key already reads back as the default - no existence check needed
		// the way RoomScaleMovement's (default true) does.
		VR.Crosshair = settings["VR"]["Crosshair"].to_boolean();
		if (vrProps.find("CrosshairSizePercent") != vrProps.end())
			VR.CrosshairSizePercent = settings["VR"]["CrosshairSizePercent"].to_int();
		VR.AimLaser = settings["VR"]["AimLaser"].to_boolean();
		int vrTabletWidth = settings["VR"]["HudTabletWidthCm"].to_int();
		if (vrTabletWidth > 0)
			VR.HudTabletWidthCm = vrTabletWidth;
		if (vrProps.find("HudTabletForearmOffsetCm") != vrProps.end())
			VR.HudTabletForearmOffsetCm = settings["VR"]["HudTabletForearmOffsetCm"].to_int();
		if (vrProps.find("HudTabletWristOffsetCm") != vrProps.end())
			VR.HudTabletWristOffsetCm = settings["VR"]["HudTabletWristOffsetCm"].to_int();

		// Only touch the bindings if the file actually carries them; otherwise the built-in defaults stand.
		// Within a stored set, an empty string is a real value (a deliberately unbound button), so any key
		// present is taken verbatim and only keys absent from the object keep their default.
		if (vrProps.find("ButtonCommands") != vrProps.end())
		{
			const JsonValue& buttons = settings["VR"]["ButtonCommands"];
			for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
			{
				for (int button = 0; button < VRSubsystem::ButtonCount; button++)
				{
					std::string key = VRButtonKey(hand, button);
					if (buttons.properties().find(key) != buttons.properties().end())
						VR.ButtonCommands[hand][button] = buttons[key].to_string();
				}
			}
		}
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
	vr["StickDeadzone"] = JsonValue::number(VR.StickDeadzone);
	vr["RoomScaleMovement"] = JsonValue::boolean(VR.RoomScaleMovement);

	switch (VR.HudHand)
	{
	default:
	case VRHand::Left: vr["HudHand"] = JsonValue::string("Left"); break;
	case VRHand::Right: vr["HudHand"] = JsonValue::string("Right"); break;
	}

	switch (VR.MenuPointerHand)
	{
	default:
	case VRHand::Left: vr["MenuPointerHand"] = JsonValue::string("Left"); break;
	case VRHand::Right: vr["MenuPointerHand"] = JsonValue::string("Right"); break;
	}

	switch (VR.WeaponHand)
	{
	default:
	case VRHand::Left: vr["WeaponHand"] = JsonValue::string("Left"); break;
	case VRHand::Right: vr["WeaponHand"] = JsonValue::string("Right"); break;
	}

	vr["WeaponForwardOffsetCm"] = JsonValue::number(VR.WeaponForwardOffsetCm);
	vr["WeaponRightOffsetCm"] = JsonValue::number(VR.WeaponRightOffsetCm);
	vr["WeaponUpOffsetCm"] = JsonValue::number(VR.WeaponUpOffsetCm);
	vr["WeaponPitchOffsetDegrees"] = JsonValue::number(VR.WeaponPitchOffsetDegrees);
	vr["WeaponYawOffsetDegrees"] = JsonValue::number(VR.WeaponYawOffsetDegrees);
	vr["WeaponRollOffsetDegrees"] = JsonValue::number(VR.WeaponRollOffsetDegrees);
	vr["WeaponScalePercent"] = JsonValue::number(VR.WeaponScalePercent);

	vr["WheelRadiusCm"] = JsonValue::number(VR.WheelRadiusCm);
	vr["WheelSelectDeadzoneCm"] = JsonValue::number(VR.WheelSelectDeadzoneCm);
	vr["WheelEntryScalePercent"] = JsonValue::number(VR.WheelEntryScalePercent);
	vr["WheelItemScalePercent"] = JsonValue::number(VR.WheelItemScalePercent);
	vr["WheelIconScalePercent"] = JsonValue::number(VR.WheelIconScalePercent);

	vr["ItemForwardOffsetCm"] = JsonValue::number(VR.ItemForwardOffsetCm);
	vr["ItemRightOffsetCm"] = JsonValue::number(VR.ItemRightOffsetCm);
	vr["ItemUpOffsetCm"] = JsonValue::number(VR.ItemUpOffsetCm);
	vr["ItemPitchOffsetDegrees"] = JsonValue::number(VR.ItemPitchOffsetDegrees);
	vr["ItemYawOffsetDegrees"] = JsonValue::number(VR.ItemYawOffsetDegrees);
	vr["ItemRollOffsetDegrees"] = JsonValue::number(VR.ItemRollOffsetDegrees);
	vr["ItemScalePercent"] = JsonValue::number(VR.ItemScalePercent);

	vr["HandColliderRadius"] = JsonValue::number(VR.HandColliderRadius);
	vr["PickupHands"] = JsonValue::number(VR.PickupHands);
	vr["Crosshair"] = JsonValue::boolean(VR.Crosshair);
	vr["CrosshairSizePercent"] = JsonValue::number(VR.CrosshairSizePercent);
	vr["AimLaser"] = JsonValue::boolean(VR.AimLaser);
	vr["HudTabletWidthCm"] = JsonValue::number(VR.HudTabletWidthCm);
	vr["HudTabletForearmOffsetCm"] = JsonValue::number(VR.HudTabletForearmOffsetCm);
	vr["HudTabletWristOffsetCm"] = JsonValue::number(VR.HudTabletWristOffsetCm);

	JsonValue vrButtons = JsonValue::object();
	for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
		for (int button = 0; button < VRSubsystem::ButtonCount; button++)
			vrButtons[VRButtonKey(hand, button)] = JsonValue::string(VR.ButtonCommands[hand][button]);
	vr["ButtonCommands"] = std::move(vrButtons);

	JsonValue settings = JsonValue::object();
	settings["RenderDevice"] = std::move(rendev);
	settings["Games"] = std::move(games);
	settings["VR"] = std::move(vr);

	const std::string filename = GetSettingsFilename();
	Directory::create(fs::path(filename).parent_path().string());
	File::write_all_text(filename, settings.to_json(true));
}
