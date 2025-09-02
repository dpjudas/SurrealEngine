#pragma once

#include "UObject/UObject.h"

class NPlayerPawn
{
public:
	static void RegisterFunctions();

	static void ClientTravel(UObject* Self, const std::string& URL, uint8_t TravelType, bool bItems);
	static void ClientMessage_219(UObject* Self, const std::string& S);
	static void ConsoleCommand(UObject* Self, const std::string& Command, std::string& ReturnValue);
	static void ConsoleCommand_219(UObject* Self, const std::string& Command);
	static void ConsoleCommandResult_219(UObject* Self, const std::string& Command, std::string& ReturnValue);
	static void CopyToClipboard(UObject* Self, const std::string& Text);
	static void GetDefaultURL(UObject* Self, const std::string& Option, std::string& ReturnValue);
	static void GetEntryLevel(UObject* Self, UObject*& ReturnValue);
	static void GetPlayerNetworkAddress(UObject* Self, std::string& ReturnValue);
	static void PasteFromClipboard(UObject* Self, std::string& ReturnValue);
	static void ResetKeyboard(UObject* Self);
	static void UpdateURL(UObject* Self, const std::string& NewOption, const std::string& NewValue, bool bSaveDefault);
	static void UpdateURL_219(UObject* Self, const std::string& NewOption);
};
