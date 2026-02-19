#pragma once

#include "UObject/UObject.h"

class NButtonWindow
{
public:
	static void RegisterFunctions();

	static void ActivateButton(UObject* Self, uint8_t Key);
	static void EnableAutoRepeat(UObject* Self, BitfieldBool* bEnable, float* initialDelay, float* repeatRate);
	static void EnableRightMouseClick(UObject* Self, BitfieldBool* bEnable);
	static void PressButton(UObject* Self, uint8_t* Key);
	static void SetActivateDelay(UObject* Self, float* newDelay);
	static void SetButtonColors(UObject* Self, Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive);
	static void SetButtonSounds(UObject* Self, UObject** pressSound, UObject** clickSound);
	static void SetButtonTextures(UObject* Self, UObject** Normal, UObject** pressed, UObject** normalFocus, UObject** pressedFocus, UObject** normalInsensitive, UObject** pressedInsensitive);
	static void SetTextColors(UObject* Self, Color* Normal, Color* pressed, Color* normalFocus, Color* pressedFocus, Color* normalInsensitive, Color* pressedInsensitive);
};
