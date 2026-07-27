#pragma once

#include "UActor.h"

class UPlayerPawn;

class UMenu : public UActor
{
public:
	using UActor::UActor;

	std::string& CenterString() { return Value<std::string>(PropOffsets_Menu.CenterString); }
	std::string& DisabledString() { return Value<std::string>(PropOffsets_Menu.DisabledString); }
	std::string& EnabledString() { return Value<std::string>(PropOffsets_Menu.EnabledString); }
	FixedArrayView<std::optional<std::string>, 24> HelpMessage() { return FixedArray<std::optional<std::string>, 24>(PropOffsets_Menu.HelpMessage); }
	std::string& LeftString() { return Value<std::string>(PropOffsets_Menu.LeftString); }
	int& MenuLength() { return Value<int>(PropOffsets_Menu.MenuLength); }
	FixedArrayView<std::optional<std::string>, 24> MenuList() { return FixedArray<std::optional<std::string>, 24>(PropOffsets_Menu.MenuList); }
	std::string& MenuTitle() { return Value<std::string>(PropOffsets_Menu.MenuTitle); }
	std::string& NoString() { return Value<std::string>(PropOffsets_Menu.NoString); }
	UMenu*& ParentMenu() { return Value<UMenu*>(PropOffsets_Menu.ParentMenu); }
	UPlayerPawn*& PlayerOwner() { return Value<UPlayerPawn*>(PropOffsets_Menu.PlayerOwner); }
	std::string& RightString() { return Value<std::string>(PropOffsets_Menu.RightString); }
	int& Selection() { return Value<int>(PropOffsets_Menu.Selection); }
	std::string& YesString() { return Value<std::string>(PropOffsets_Menu.YesString); }
	BitfieldBool bConfigChanged() { return BoolValue(PropOffsets_Menu.bConfigChanged); }
	BitfieldBool bExitAllMenus() { return BoolValue(PropOffsets_Menu.bExitAllMenus); }
};
