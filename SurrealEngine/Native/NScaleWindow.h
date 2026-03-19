#pragma once

#include "UObject/UObject.h"

class NScaleWindow
{
public:
	static void RegisterFunctions();

	static void ClearAllEnumerations(UObject* Self);
	static void EnableStretchedScale(UObject* Self, std::optional<bool> bNewStretch);
	static void GetNumTicks(UObject* Self, int& ReturnValue);
	static void GetThumbSpan(UObject* Self, int& ReturnValue);
	static void GetTickPosition(UObject* Self, int& ReturnValue);
	static void GetValue(UObject* Self, float& ReturnValue);
	static void GetValueString(UObject* Self, std::string& ReturnValue);
	static void GetValues(UObject* Self, float& fromValue, float& toValue);
	static void MoveThumb(UObject* Self, uint8_t MoveThumb);
	static void PlayScaleSound(UObject* Self, UObject* newsound, std::optional<float> Volume, std::optional<float> Pitch);
	static void SetBorderPattern(UObject* Self, UObject* NewTexture);
	static void SetEnumeration(UObject* Self, int tickPos, const std::string& newStr);
	static void SetNumTicks(UObject* Self, int newNumTicks);
	static void SetScaleBorder(UObject* Self, std::optional<float> newBorderSize, std::optional<Color> NewColor);
	static void SetScaleColor(UObject* Self, const Color& NewColor);
	static void SetScaleMargins(UObject* Self, std::optional<float> marginWidth, std::optional<float> marginHeight);
	static void SetScaleOrientation(UObject* Self, uint8_t newOrientation);
	static void SetScaleSounds(UObject* Self, std::optional<UObject*> setSound, std::optional<UObject*> clickSound, std::optional<UObject*> dragSound);
	static void SetScaleStyle(UObject* Self, uint8_t NewStyle);
	static void SetScaleTexture(UObject* Self, UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight, std::optional<float> newStart, std::optional<float> newEnd);
	static void SetThumbBorder(UObject* Self, std::optional<float> newBorderSize, std::optional<Color> NewColor);
	static void SetThumbCaps(UObject* Self, UObject* preCap, UObject* postCap, std::optional<float> preCapWidth, std::optional<float> preCapHeight, std::optional<float> postCapWidth, std::optional<float> postCapHeight);
	static void SetThumbColor(UObject* Self, const Color& NewColor);
	static void SetThumbSpan(UObject* Self, std::optional<int> newRange);
	static void SetThumbStep(UObject* Self, int NewStep);
	static void SetThumbStyle(UObject* Self, uint8_t NewStyle);
	static void SetThumbTexture(UObject* Self, UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight);
	static void SetTickColor(UObject* Self, const Color& NewColor);
	static void SetTickPosition(UObject* Self, int newPosition);
	static void SetTickStyle(UObject* Self, uint8_t NewStyle);
	static void SetTickTexture(UObject* Self, UObject* tickTexture, std::optional<bool> bDrawEndTicks, std::optional<float> newWidth, std::optional<float> NewHeight);
	static void SetValue(UObject* Self, float NewValue);
	static void SetValueFormat(UObject* Self, const std::string& newFmt);
	static void SetValueRange(UObject* Self, float newFrom, float newTo);
};
