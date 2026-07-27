#pragma once

#include "Packages/Core/UObject.h"
#include "UWindow.h"

// Represents a drawable text portion
struct TextBlock
{
	std::string text; // Text stripped of any control codes
	Color textColor;  // Text color extracted from a |p or |c control code
	size_t accelPos;  // Accelerator position acquired from the |& control code
};

class UGC : public UObject
{
public:
	using UObject::UObject;

	void ClearZ();
	void CopyGC(UObject* Copy);
	void DrawActor(UObject* Actor, std::optional<bool> bClearZ, std::optional<bool> bConstrain, std::optional<bool> bUnlit, std::optional<float> DrawScale, std::optional<float> ScaleGlow, std::optional<UObject*> Skin);
	void DrawBorders(float DestX, float DestY, float destWidth, float destHeight, float leftMargin, float rightMargin, float TopMargin, float BottomMargin, UObject** borders, std::optional<bool> bStretchHorizontally, std::optional<bool> bStretchVertically);
	void DrawBox(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, float boxThickness, UObject* tX);
	void DrawIcon(float DestX, float DestY, UObject* tX);
	void DrawPattern(float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, UObject* tX);
	void DrawStretchedTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, float srcWidth, float srcHeight, UObject* tX);
	void DrawText(float DestX, float DestY, float destWidth, float destHeight, const std::string& textStr);
	void DrawTexture(float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, UObject* tX);
	void EnableDrawing(bool bDrawEnabled);
	void EnableMasking(bool bNewMasking);
	void EnableModulation(bool bNewModulation);
	void EnableSmoothing(bool bNewSmoothing);
	void EnableSpecialText(bool bNewSpecialText);
	void EnableTranslucency(bool bNewTranslucency);
	void EnableTranslucentText(bool bNewTranslucency);
	void EnableWordWrap(bool bNewWordWrap);
	void GetAlignments(uint8_t& HAlign, uint8_t& VAlign);
	float GetFontHeight(std::optional<bool> bIncludeSpace);
	void GetFonts(UObject*& normalFont, UObject*& boldFont);
	uint8_t GetHorizontalAlignment();
	uint8_t GetStyle();
	void GetTextColor(Color& TextColor);
	void GetTextExtent(float destWidth, float& xExtent, float& yExtent, const std::string& textStr);
	float GetTextVSpacing();
	void GetTileColor(Color& tileColor);
	uint8_t GetVerticalAlignment();
	void Intersect(float ClipX, float ClipY, float clipWidth, float clipHeight);
	bool IsDrawingEnabled();
	bool IsMaskingEnabled();
	bool IsModulationEnabled();
	bool IsSmoothingEnabled();
	bool IsSpecialTextEnabled();
	bool IsTranslucencyEnabled();
	bool IsTranslucentTextEnabled();
	bool IsWordWrapEnabled();
	void PopGC(std::optional<int> gcNum);
	int PushGC();
	void SetAlignments(uint8_t newHAlign, uint8_t newVAlign);
	void SetBaselineData(std::optional<float> newBaselineOffset, std::optional<float> newUnderlineHeight);
	void SetBoldFont(UObject* newBoldFont);
	void SetFont(UObject* NewFont);
	void SetFonts(UObject* newNormalFont, UObject* newBoldFont);
	void SetHorizontalAlignment(uint8_t newHAlign);
	void SetNormalFont(UObject* newNormalFont);
	void SetStyle(EDrawStyle NewStyle);
	void SetTextColor(const Color& newTextColor);
	void SetTextVSpacing(float newVSpacing);
	void SetTileColor(const Color& newTileColor);
	void SetVerticalAlignment(uint8_t newVAlign);

	UCanvas*& Canvas() { return Value<UCanvas*>(PropOffsets_GC.Canvas); }
	uint8_t& HAlign() { return Value<uint8_t>(PropOffsets_GC.HAlign); }
	int& PolyFlags() { return Value<int>(PropOffsets_GC.PolyFlags); }
	uint8_t& Style() { return Value<uint8_t>(PropOffsets_GC.Style); }
	Color& TextColor() { return Value<Color>(PropOffsets_GC.TextColor); }
	uint8_t& VAlign() { return Value<uint8_t>(PropOffsets_GC.VAlign); }
	BitfieldBool bDrawEnabled() { return BoolValue(PropOffsets_GC.bDrawEnabled); }
	BitfieldBool bFree() { return BoolValue(PropOffsets_GC.bFree); }
	BitfieldBool bMasked() { return BoolValue(PropOffsets_GC.bMasked); }
	BitfieldBool bModulated() { return BoolValue(PropOffsets_GC.bModulated); }
	BitfieldBool bParseMetachars() { return BoolValue(PropOffsets_GC.bParseMetachars); }
	BitfieldBool bSmoothed() { return BoolValue(PropOffsets_GC.bSmoothed); }
	BitfieldBool bTextTranslucent() { return BoolValue(PropOffsets_GC.bTextTranslucent); }
	BitfieldBool bTranslucent() { return BoolValue(PropOffsets_GC.bTranslucent); }
	BitfieldBool bWordWrap() { return BoolValue(PropOffsets_GC.bWordWrap); }
	float& baselineOffset() { return Value<float>(PropOffsets_GC.baselineOffset); }
	UFont*& boldFont() { return Value<UFont*>(PropOffsets_GC.boldFont); }
	ClipRect& gcClipRect() { return Value<ClipRect>(PropOffsets_GC.gcClipRect); }
	int& gcCount() { return Value<int>(PropOffsets_GC.gcCount); }
	UGC*& gcFree() { return Value<UGC*>(PropOffsets_GC.gcFree); }
	UGC*& gcOwner() { return Value<UGC*>(PropOffsets_GC.gcOwner); }
	UGC*& gcStack() { return Value<UGC*>(PropOffsets_GC.gcStack); }
	int& hMultiplier() { return Value<int>(PropOffsets_GC.hMultiplier); }
	UFont*& normalFont() { return Value<UFont*>(PropOffsets_GC.normalFont); }
	//Plane& textPlane() { return Value<Plane>(PropOffsets_GC.textPlane); }
	int& textPolyFlags() { return Value<int>(PropOffsets_GC.textPolyFlags); }
	float& textVSpacing() { return Value<float>(PropOffsets_GC.textVSpacing); }
	Color& tileColor() { return Value<Color>(PropOffsets_GC.tileColor); }
	//Plane& tilePlane() { return Value<Plane>(PropOffsets_GC.tilePlane); }
	float& underlineHeight() { return Value<float>(PropOffsets_GC.underlineHeight); }
	UTexture*& underlineTexture() { return Value<UTexture*>(PropOffsets_GC.underlineTexture); }
	int& vMultiplier() { return Value<int>(PropOffsets_GC.vMultiplier); }

	Rectf ScaleRect(const Rectf& box);
	uint32_t EffectivePolyFlags();
	uint32_t EffectiveTextPolyFlags();
	void DrawTile(UTexture* tex, const Rectf& dest, const Rectf& src, const Color& c, uint32_t flags);
	Sizef DrawText(UFont* font, float x, float y, float destWidth, const std::string& text, const Color& c, uint32_t polyflags, bool noDraw = false);
	Array<TextBlock> FindTextBlocks(const std::string& text, const Color& color);
	void DrawTextBlockRange(float x, float y, const Array<TextBlock>& textBlocks, size_t start, size_t end, UFont* font, uint32_t polyflags);
	vec2 GetTextSize(UFont* font, const std::string& text);

	void ResetClip(Rectf box);
	void PushClip(Rectf box);
	void PopClip();

	bool SpecialTextEnabled = false;

	float offsetX = 0.0f;
	float offsetY = 0.0f;
	Rectf clipBox;
	std::vector<Rectf> clipStack;
};
