
#include "Precomp.h"
#include "NPlayerPawn.h"
#include "VM/NativeFunc.h"
#include "UObject/ULevel.h"
#include "Engine.h"

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
	throw std::runtime_error("PlayerPawn.ClientTravel not implemented");
}

void NPlayerPawn::ConsoleCommand(UObject* Self, const std::string& Command, std::string& ReturnValue)
{
	throw std::runtime_error("PlayerPawn.ConsoleCommand not implemented");
}

void NPlayerPawn::CopyToClipboard(UObject* Self, const std::string& Text)
{
	throw std::runtime_error("PlayerPawn.CopyToClipboard not implemented");
}

void NPlayerPawn::GetDefaultURL(UObject* Self, const std::string& Option, std::string& ReturnValue)
{
	throw std::runtime_error("PlayerPawn.GetDefaultURL not implemented");
}

void NPlayerPawn::GetEntryLevel(UObject* Self, UObject*& ReturnValue)
{
	ReturnValue = Engine::Instance->level;
}

void NPlayerPawn::GetPlayerNetworkAddress(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("PlayerPawn.GetPlayerNetworkAddress not implemented");
}

void NPlayerPawn::PasteFromClipboard(UObject* Self, std::string& ReturnValue)
{
	throw std::runtime_error("PlayerPawn.PasteFromClipboard not implemented");
}

void NPlayerPawn::ResetKeyboard(UObject* Self)
{
	throw std::runtime_error("PlayerPawn.ResetKeyboard not implemented");
}

void NPlayerPawn::UpdateURL(UObject* Self, const std::string& NewOption, const std::string& NewValue, bool bSaveDefault)
{
	throw std::runtime_error("PlayerPawn.UpdateURL not implemented");
}
