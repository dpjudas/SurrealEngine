#pragma once

#include "Packages/Core/UObject.h"

class UPlayerPawn;
class UConsole;

class UPlayer : public UObject
{
public:
	using UObject::UObject;

	UPlayerPawn*& Actor() { return Value<UPlayerPawn*>(PropOffsets_Player.Actor); }
	int& ConfiguredInternetSpeed() { return Value<int>(PropOffsets_Player.ConfiguredInternetSpeed); }
	int& ConfiguredLanSpeed() { return Value<int>(PropOffsets_Player.ConfiguredLanSpeed); }
	UConsole*& Console() { return Value<UConsole*>(PropOffsets_Player.Console); }
	int& CurrentNetSpeed() { return Value<int>(PropOffsets_Player.CurrentNetSpeed); }
	uint8_t& SelectedCursor() { return Value<uint8_t>(PropOffsets_Player.SelectedCursor); }
	float& WindowsMouseX() { return Value<float>(PropOffsets_Player.WindowsMouseX); }
	float& WindowsMouseY() { return Value<float>(PropOffsets_Player.WindowsMouseY); }
	BitfieldBool bShowWindowsMouse() { return BoolValue(PropOffsets_Player.bShowWindowsMouse); }
	BitfieldBool bSuspendPrecaching() { return BoolValue(PropOffsets_Player.bSuspendPrecaching); }
	BitfieldBool bWindowsMouseAvailable() { return BoolValue(PropOffsets_Player.bWindowsMouseAvailable); }
	int& vfExec() { return Value<int>(PropOffsets_Player.vfExec); } // native
	int& vfOut() { return Value<int>(PropOffsets_Player.vfOut); } // native
};
