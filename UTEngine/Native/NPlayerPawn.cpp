
#include "Precomp.h"
#include "NPlayerPawn.h"
#include "VM/NativeFunc.h"
#include "UObject/ULevel.h"
#include "UObject/UActor.h"
#include "Engine.h"
#include "Package/PackageManager.h"

void NPlayerPawn::RegisterFunctions()
{
	RegisterVMNativeFunc_3("PlayerPawn", "ClientTravel", &NPlayerPawn::ClientTravel, 0);
	RegisterVMNativeFunc_2("PlayerPawn", "ConsoleCommand", &NPlayerPawn::ConsoleCommand, 0);
	RegisterVMNativeFunc_1("PlayerPawn", "CopyToClipboard", &NPlayerPawn::CopyToClipboard, 0);
	RegisterVMNativeFunc_2("PlayerPawn", "GetDefaultURL", &NPlayerPawn::GetDefaultURL, 0);
	RegisterVMNativeFunc_1("PlayerPawn", "GetEntryLevel", &NPlayerPawn::GetEntryLevel, 0);
	RegisterVMNativeFunc_1("PlayerPawn", "GetPlayerNetworkAddress", &NPlayerPawn::GetPlayerNetworkAddress, 0);
	RegisterVMNativeFunc_1("PlayerPawn", "PasteFromClipboard", &NPlayerPawn::PasteFromClipboard, 0);
	RegisterVMNativeFunc_0("PlayerPawn", "ResetKeyboard", &NPlayerPawn::ResetKeyboard, 544);
	RegisterVMNativeFunc_3("PlayerPawn", "UpdateURL", &NPlayerPawn::UpdateURL, 546);
}

void NPlayerPawn::ClientTravel(UObject* Self, const std::string& URL, uint8_t TravelType, bool bItems)
{
	engine->ClientTravel(URL, TravelType, bItems);
}

void NPlayerPawn::ConsoleCommand(UObject* Self, const std::string& Command, std::string& ReturnValue)
{
	// "Execute a console command in the context of this player, then forward to Actor.ConsoleCommand"

	bool found;
	ReturnValue = engine->ConsoleCommand(Self, Command, found);
}

void NPlayerPawn::CopyToClipboard(UObject* Self, const std::string& Text)
{
	engine->LogUnimplemented("PlayerPawn.CopyToClipboard(" + Text + ")");
}

void NPlayerPawn::GetDefaultURL(UObject* Self, const std::string& Option, std::string& ReturnValue)
{
	ReturnValue = engine->packages->GetIniValue("user", "DefaultPlayer", Option);
}

void NPlayerPawn::GetEntryLevel(UObject* Self, UObject*& ReturnValue)
{
	ReturnValue = engine->EntryLevelInfo;
}

void NPlayerPawn::GetPlayerNetworkAddress(UObject* Self, std::string& ReturnValue)
{
	engine->LogUnimplemented("PlayerPawn.GetPlayerNetworkAddress");
	ReturnValue = "";
}

void NPlayerPawn::PasteFromClipboard(UObject* Self, std::string& ReturnValue)
{
	engine->LogUnimplemented("PlayerPawn.PasteFromClipboard");
	ReturnValue = "";
}

void NPlayerPawn::ResetKeyboard(UObject* Self)
{
	throw std::runtime_error("PlayerPawn.ResetKeyboard not implemented");
}

void NPlayerPawn::UpdateURL(UObject* Self, const std::string& NewOption, const std::string& NewValue, bool bSaveDefault)
{
	UPlayerPawn* SelfPlayerPawn = UObject::Cast<UPlayerPawn>(Self);
	SelfPlayerPawn->Level()->URL.AddOrReplaceOption(NewOption + "=" + NewValue);
	if (bSaveDefault)
		engine->LogUnimplemented("PlayerPawn.UpdateURL save default");
}
