#pragma once

#include "UObject/UObject.h"

class NButtonWindow
{
public:
	static void RegisterFunctions();

	static void ActivateButton(UObject* Self, uint8_t Key);
	static void EnableAutoRepeat(UObject* Self, std::optional<bool> bEnable, std::optional<float> initialDelay, std::optional<float> repeatRate);
	static void EnableRightMouseClick(UObject* Self, std::optional<bool> bEnable);
	static void PressButton(UObject* Self, std::optional<uint8_t> Key);
	static void SetActivateDelay(UObject* Self, std::optional<float> newDelay);
	static void SetButtonColors(UObject* Self, std::optional<Color> Normal, std::optional<Color> pressed, std::optional<Color> normalFocus, std::optional<Color> pressedFocus, std::optional<Color> normalInsensitive, std::optional<Color> pressedInsensitive);
	static void SetButtonSounds(UObject* Self, std::optional<UObject*> pressSound, std::optional<UObject*> clickSound);
	static void SetButtonTextures(UObject* Self, std::optional<UObject*> Normal, std::optional<UObject*> pressed, std::optional<UObject*> normalFocus, std::optional<UObject*> pressedFocus, std::optional<UObject*> normalInsensitive, std::optional<UObject*> pressedInsensitive);
	static void SetTextColors(UObject* Self, std::optional<Color> Normal, std::optional<Color> pressed, std::optional<Color> normalFocus, std::optional<Color> pressedFocus, std::optional<Color> normalInsensitive, std::optional<Color> pressedInsensitive);
};
