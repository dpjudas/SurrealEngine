
#include "Precomp.h"
#include "UPlayerPawn.h"
#include "UCamera.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "Package/IniProperty.h"
#include "VM/ScriptCall.h"

bool UPlayerPawn::IsPressing(uint8_t KeyNum)
{
	for (auto& activeButtons : engine->activeInputButtons)
	{
		if (activeButtons.second == KeyNum)
			return true;
	}

	return false;
}

void UPlayerPawn::PausedInput(float elapsed)
{
	if (Role() >= ROLE_SimulatedProxy && Player() && !UObject::TryCast<UCamera>(this))
	{
		CallEvent(this, EventName::PlayerInput, { ExpressionValue::FloatValue(elapsed) });
	}
}

void UPlayerPawn::Tick(float elapsed)
{
	UPawn::Tick(elapsed);

	if (Role() >= ROLE_SimulatedProxy)
	{
		if (Player() && !UObject::TryCast<UCamera>(this))
		{
			CallEvent(this, EventName::PlayerInput, { ExpressionValue::FloatValue(elapsed) });
			CallEvent(this, EventName::PlayerTick, { ExpressionValue::FloatValue(elapsed) });
		}
	}

	// TODO: is this the correct place to set this?
	aForward() = 0.0f;

	// XXX: we reset this here to prevent infinite runaway, which eventually breaks mouselook
	// however, this might break looking with the controller?
	aTurn() = 0.0f;
	aLookUp() = 0.0f;
}

void UPlayerPawn::TickRotating(float elapsed)
{
	if (Physics() == PHYS_Spider)
		return;

	Rotator rot = Rotation();

	// To do: apply RotationRate().Roll

	Rotation() = rot;
}

void UPlayerPawn::LoadProperties()
{
	bInvertMouse() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bInvertMouse", true);
	MouseSensitivity() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "MouseSensitivity", 5.0f);
	// TODO: Handle the array property this class has (WeaponPriority)
	DodgeClickTime() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "DodgeClickTime", 0.25f);
	Bob() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "Bob", 0.016f);
	MyAutoAim() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "MyAutoAim", 1.0f);
	if (!engine->LaunchInfo.IsRune())
		Handedness() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "Handedness", -1.0f);
	bLookUpStairs() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bLookUpStairs", false);
	bSnapToLevel() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bSnapToLevel", false);
	bAlwaysMouseLook() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bAlwaysMouseLook", true);
	bKeyboardLook() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bKeyboardLook", false);
	if (engine->LaunchInfo.ue1Version > 219)
	{
		bMaxMouseSmoothing() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bMaxMouseSmoothing", false);
		bNoFlash() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bNoFlash", false);
		bNoVoices() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bNoVoices", false);
		bMessageBeep() = IniPropertyConverter<bool>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "bMessageBeep", true);
		// NetSpeed is missing
		// LanSpeed is missing
		MouseSmoothThreshold() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "MouseSmoothThreshold", 0.16f);
		ngWorldSecret() = IniPropertyConverter<std::string>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "ngWorldSecret", "");

		// SE addition: Store/Load the DefaultFOV setting into/from the user.ini file as well
		DefaultFOV() = IniPropertyConverter<float>::FromIniFile(*engine->packages->GetIniFile("user"), "Engine.PlayerPawn", "MainFOV", 90.0f);
	}
}

void UPlayerPawn::SaveConfig()
{
	UPawn::SaveConfig();

	// Note: the code below may no longer be needed. SaveConfig() on UObject saves all unrealscript variables marked as config or globalconfig

	// Not sure why are PlayerPawn's config fields not saved with the base SaveConfig(), but whatever. 
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bInvertMouse", IniPropertyConverter<bool>::ToString(bInvertMouse()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "MouseSensitivity", IniPropertyConverter<float>::ToString(MouseSensitivity()));
	// TODO: Handle the array property this class has (WeaponPriority)
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "DodgeClickTime", IniPropertyConverter<float>::ToString(DodgeClickTime()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "Bob", IniPropertyConverter<float>::ToString(Bob()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "MyAutoAim", IniPropertyConverter<float>::ToString(MyAutoAim()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "Handedness", IniPropertyConverter<float>::ToString(Handedness()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bLookUpStairs", IniPropertyConverter<bool>::ToString(bLookUpStairs()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bSnapToLevel", IniPropertyConverter<bool>::ToString(bSnapToLevel()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bAlwaysMouseLook", IniPropertyConverter<bool>::ToString(bAlwaysMouseLook()));
	engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bKeyboardLook", IniPropertyConverter<bool>::ToString(bKeyboardLook()));
	if (engine->LaunchInfo.ue1Version > 219)
	{
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bMaxMouseSmoothing", IniPropertyConverter<bool>::ToString(bMaxMouseSmoothing()));
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bNoFlash", IniPropertyConverter<bool>::ToString(bNoFlash()));
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bNoVoices", IniPropertyConverter<bool>::ToString(bNoVoices()));
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "bMessageBeep", IniPropertyConverter<bool>::ToString(bMessageBeep()));
		// NetSpeed is missing
		// LanSpeed is missing
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "MouseSmoothThreshold", IniPropertyConverter<float>::ToString(MouseSmoothThreshold()));
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "ngWorldSecret", ngWorldSecret());

		// SE addition: Store/Load the DefaultFOV setting into/from the user.ini file as well
		engine->packages->SetIniValue("user", "Engine.PlayerPawn", "MainFOV", IniPropertyConverter<float>::ToString(DefaultFOV()));
	}
}
