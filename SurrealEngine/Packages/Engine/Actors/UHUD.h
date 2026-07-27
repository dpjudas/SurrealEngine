#pragma once

#include "UActor.h"

class UMutator;
class UMenu;
class UPlayerPawn;

class UHUD : public UActor
{
public:
	using UActor::UActor;

	int& Crosshair() { return Value<int>(PropOffsets_HUD.Crosshair); }
	std::string& HUDConfigWindowType() { return Value<std::string>(PropOffsets_HUD.HUDConfigWindowType); }
	UMutator*& HUDMutator() { return Value<UMutator*>(PropOffsets_HUD.HUDMutator); }
	int& HudMode() { return Value<int>(PropOffsets_HUD.HudMode); }
	UMenu*& MainMenu() { return Value<UMenu*>(PropOffsets_HUD.MainMenu); }
	UClass*& MainMenuType() { return Value<UClass*>(PropOffsets_HUD.MainMenuType); }
	UPlayerPawn*& PlayerOwner() { return Value<UPlayerPawn*>(PropOffsets_HUD.PlayerOwner); }
	Color& WhiteColor() { return Value<Color>(PropOffsets_HUD.WhiteColor); }
};
