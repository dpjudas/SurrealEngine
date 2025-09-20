
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
	if (engine->LaunchInfo.engineVersion > 219)
	{
		RegisterVMNativeFunc_2("PlayerPawn", "ConsoleCommand", &NPlayerPawn::ConsoleCommand, 0);
	}
	else
	{
		RegisterVMNativeFunc_1("PlayerPawn", "ClientMessage", &NPlayerPawn::ClientMessage_219, 0);
		RegisterVMNativeFunc_1("PlayerPawn", "ConsoleCommand", &NPlayerPawn::ConsoleCommand_219, 537);
		RegisterVMNativeFunc_2("PlayerPawn", "ConsoleCommandResult", &NPlayerPawn::ConsoleCommandResult_219, 542);
	}
	RegisterVMNativeFunc_1("PlayerPawn", "CopyToClipboard", &NPlayerPawn::CopyToClipboard, 0);
	RegisterVMNativeFunc_2("PlayerPawn", "GetDefaultURL", &NPlayerPawn::GetDefaultURL, 0);
	RegisterVMNativeFunc_1("PlayerPawn", "GetEntryLevel", &NPlayerPawn::GetEntryLevel, 0);
	RegisterVMNativeFunc_1("PlayerPawn", "GetPlayerNetworkAddress", &NPlayerPawn::GetPlayerNetworkAddress, 0);
	RegisterVMNativeFunc_1("PlayerPawn", "PasteFromClipboard", &NPlayerPawn::PasteFromClipboard, 0);
	RegisterVMNativeFunc_0("PlayerPawn", "ResetKeyboard", &NPlayerPawn::ResetKeyboard, 544);
	if (engine->LaunchInfo.engineVersion > 219)
		RegisterVMNativeFunc_3("PlayerPawn", "UpdateURL", &NPlayerPawn::UpdateURL, 546);
	else
		RegisterVMNativeFunc_1("PlayerPawn", "UpdateURL", &NPlayerPawn::UpdateURL_219, 546);
}

void NPlayerPawn::ClientTravel(UObject* Self, const std::string& URL, uint8_t TravelType, bool bItems)
{
	engine->ClientTravel(URL, static_cast<ETravelType>(TravelType), bItems);
}

void NPlayerPawn::ConsoleCommand(UObject* Self, const std::string& Command, std::string& ReturnValue)
{
	// "Execute a console command in the context of this player, then forward to Actor.ConsoleCommand"

	ExpressionValue found = ExpressionValue::BoolValue(false);
	ReturnValue = engine->ConsoleCommand(Self, Command, found.ToType<BitfieldBool&>());
}

void NPlayerPawn::ClientMessage_219(UObject* Self, const std::string& S)
{
	LogUnimplemented("ClientMessage not implemented");
}

void NPlayerPawn::ConsoleCommand_219(UObject* Self, const std::string& Command)
{
	std::string result;
	ConsoleCommand(Self, Command, result);
}

void NPlayerPawn::ConsoleCommandResult_219(UObject* Self, const std::string& Command, std::string& ReturnValue)
{
	ConsoleCommand(Self, Command, ReturnValue);
}

void NPlayerPawn::CopyToClipboard(UObject* Self, const std::string& Text)
{
	engine->window->SetClipboardText(Text);
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
	LogUnimplemented("PlayerPawn.GetPlayerNetworkAddress");
	ReturnValue = "";
}

void NPlayerPawn::PasteFromClipboard(UObject* Self, std::string& ReturnValue)
{
	ReturnValue = engine->window->GetClipboardText();
}

void NPlayerPawn::ResetKeyboard(UObject* Self)
{
	LogUnimplemented("PlayerPawn.ResetKeyboard");
}

void NPlayerPawn::UpdateURL(UObject* Self, const std::string& NewOption, const std::string& NewValue, bool bSaveDefault)
{
	UPlayerPawn* SelfPlayerPawn = UObject::Cast<UPlayerPawn>(Self);
	SelfPlayerPawn->Level()->URL.AddOrReplaceOption(NewOption + "=" + NewValue);
	if (bSaveDefault)
	{
		// Save the setting to DefaultUser section in User.ini
		engine->packages->SetIniValue("User", "DefaultPlayer", NewOption, NewValue);
	}
}

void NPlayerPawn::UpdateURL_219(UObject* Self, const std::string& NewOption)
{
	UPlayerPawn* SelfPlayerPawn = UObject::Cast<UPlayerPawn>(Self);
	SelfPlayerPawn->Level()->URL.AddOrReplaceOption(NewOption);
}
