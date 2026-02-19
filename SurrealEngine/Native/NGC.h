#pragma once

#include "UObject/UObject.h"

class NGC
{
public:
	static void RegisterFunctions();

	static void ClearZ(UObject* Self);
	static void CopyGC(UObject* Self, UObject* Copy);
	static void DrawActor(UObject* Self, UObject* Actor, BitfieldBool* bClearZ, BitfieldBool* bConstrain, BitfieldBool* bUnlit, float* DrawScale, float* ScaleGlow, UObject** Skin);
	static void DrawBorders(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float leftMargin, float rightMargin, float TopMargin, float BottomMargin, UObject* borders, BitfieldBool* bStretchHorizontally, BitfieldBool* bStretchVertically);
	static void DrawBox(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, float boxThickness, UObject* tX);
	static void DrawIcon(UObject* Self, float DestX, float DestY, UObject* tX);
	static void DrawPattern(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, UObject* tX);
	static void DrawStretchedTexture(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, float srcWidth, float srcHeight, UObject* tX);
	static void DrawText(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, const std::string& textStr);
	static void DrawTexture(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, UObject* tX);
	static void EnableDrawing(UObject* Self, bool bDrawEnabled);
	static void EnableMasking(UObject* Self, bool bNewMasking);
	static void EnableModulation(UObject* Self, bool bNewModulation);
	static void EnableSmoothing(UObject* Self, bool bNewSmoothing);
	static void EnableSpecialText(UObject* Self, bool bNewSpecialText);
	static void EnableTranslucency(UObject* Self, bool bNewTranslucency);
	static void EnableTranslucentText(UObject* Self, bool bNewTranslucency);
	static void EnableWordWrap(UObject* Self, bool bNewWordWrap);
	static void GetAlignments(UObject* Self, uint8_t& HAlign, uint8_t& VAlign);
	static void GetFontHeight(UObject* Self, BitfieldBool* bIncludeSpace, float& ReturnValue);
	static void GetFonts(UObject* Self, UObject*& normalFont, UObject*& boldFont);
	static void GetHorizontalAlignment(UObject* Self, uint8_t& ReturnValue);
	static void GetStyle(UObject* Self, uint8_t& ReturnValue);
	static void GetTextColor(UObject* Self, Color& TextColor);
	static void GetTextExtent(UObject* Self, float destWidth, float& xExtent, float& yExtent, const std::string& textStr);
	static void GetTextVSpacing(UObject* Self, float& ReturnValue);
	static void GetTileColor(UObject* Self, Color& tileColor);
	static void GetVerticalAlignment(UObject* Self, uint8_t& ReturnValue);
	static void Intersect(UObject* Self, float ClipX, float ClipY, float clipWidth, float clipHeight);
	static void IsDrawingEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsMaskingEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsModulationEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsSmoothingEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsSpecialTextEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsTranslucencyEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsTranslucentTextEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void IsWordWrapEnabled(UObject* Self, BitfieldBool& ReturnValue);
	static void PopGC(UObject* Self, int* gcNum);
	static void PushGC(UObject* Self, int& ReturnValue);
	static void SetAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign);
	static void SetBaselineData(UObject* Self, float* newBaselineOffset, float* newUnderlineHeight);
	static void SetBoldFont(UObject* Self, UObject* newBoldFont);
	static void SetFont(UObject* Self, UObject* NewFont);
	static void SetFonts(UObject* Self, UObject* newNormalFont, UObject* newBoldFont);
	static void SetHorizontalAlignment(UObject* Self, uint8_t newHAlign);
	static void SetNormalFont(UObject* Self, UObject* newNormalFont);
	static void SetStyle(UObject* Self, uint8_t NewStyle);
	static void SetTextColor(UObject* Self, const Color& newTextColor);
	static void SetTextVSpacing(UObject* Self, float newVSpacing);
	static void SetTileColor(UObject* Self, const Color& newTileColor);
	static void SetVerticalAlignment(UObject* Self, uint8_t newVAlign);
};
