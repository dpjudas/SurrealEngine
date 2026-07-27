#pragma once

#include "UWindow.h"

class UScaleWindow : public UWindow
{
public:
	using UWindow::UWindow;

	void ClearAllEnumerations();
	void EnableStretchedScale(std::optional<bool> bNewStretch);
	int GetNumTicks();
	int GetThumbSpan();
	int GetTickPosition();
	float GetValue();
	std::string GetValueString();
	void GetValues(float& fromValue, float& toValue);
	void MoveThumb(uint8_t MoveThumb);
	void PlayScaleSound(UObject* newsound, std::optional<float> Volume, std::optional<float> Pitch);
	void SetBorderPattern(UObject* NewTexture);
	void SetEnumeration(int tickPos, const std::string& newStr);
	void SetNumTicks(int newNumTicks);
	void SetScaleBorder(std::optional<float> newBorderSize, std::optional<Color> NewColor);
	void SetScaleColor(const Color& NewColor);
	void SetScaleMargins(std::optional<float> marginWidth, std::optional<float> marginHeight);
	void SetScaleOrientation(uint8_t newOrientation);
	void SetScaleSounds(std::optional<UObject*> setSound, std::optional<UObject*> clickSound, std::optional<UObject*> dragSound);
	void SetScaleStyle(uint8_t NewStyle);
	void SetScaleTexture(UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight, std::optional<float> newStart, std::optional<float> newEnd);
	void SetThumbBorder(std::optional<float> newBorderSize, std::optional<Color> NewColor);
	void SetThumbCaps(UObject* preCap, UObject* postCap, std::optional<float> preCapWidth, std::optional<float> preCapHeight, std::optional<float> postCapWidth, std::optional<float> postCapHeight);
	void SetThumbColor(const Color& NewColor);
	void SetThumbSpan(std::optional<int> newRange);
	void SetThumbStep(int NewStep);
	void SetThumbStyle(uint8_t NewStyle);
	void SetThumbTexture(UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight);
	void SetTickColor(const Color& NewColor);
	void SetTickPosition(int newPosition);
	void SetTickStyle(uint8_t NewStyle);
	void SetTickTexture(UObject* tickTexture, std::optional<bool> bDrawEndTicks, std::optional<float> newWidth, std::optional<float> NewHeight);
	void SetValue(float NewValue);
	void SetValueFormat(const std::string& newFmt);
	void SetValueRange(float newFrom, float newTo);

	float& RemainingTime() { return Value<float>(PropOffsets_ScaleWindow.RemainingTime); }
	float& ThumbHeight() { return Value<float>(PropOffsets_ScaleWindow.ThumbHeight); }
	float& ThumbWidth() { return Value<float>(PropOffsets_ScaleWindow.ThumbWidth); }
	float& absEndScale() { return Value<float>(PropOffsets_ScaleWindow.absEndScale); }
	float& absStartScale() { return Value<float>(PropOffsets_ScaleWindow.absStartScale); }
	BitfieldBool bDraggingThumb() { return BoolValue(PropOffsets_ScaleWindow.bDraggingThumb); }
	BitfieldBool bDrawEndTicks() { return BoolValue(PropOffsets_ScaleWindow.bDrawEndTicks); }
	BitfieldBool bRepeatScaleTexture() { return BoolValue(PropOffsets_ScaleWindow.bRepeatScaleTexture); }
	BitfieldBool bRepeatThumbTexture() { return BoolValue(PropOffsets_ScaleWindow.bRepeatThumbTexture); }
	BitfieldBool bSpanThumb() { return BoolValue(PropOffsets_ScaleWindow.bSpanThumb); }
	BitfieldBool bStretchScale() { return BoolValue(PropOffsets_ScaleWindow.bStretchScale); }
	UTexture*& borderPattern() { return Value<UTexture*>(PropOffsets_ScaleWindow.borderPattern); }
	USound*& clickSound() { return Value<USound*>(PropOffsets_ScaleWindow.clickSound); }
	int& currentPos() { return Value<int>(PropOffsets_ScaleWindow.currentPos); }
	USound*& dragSound() { return Value<USound*>(PropOffsets_ScaleWindow.dragSound); }
	float& endOffset() { return Value<float>(PropOffsets_ScaleWindow.endOffset); }
	//DynamicArray& enumStrings() { return Value<DynamicArray>(PropOffsets_ScaleWindow.enumStrings); }
	float& fromValue() { return Value<float>(PropOffsets_ScaleWindow.fromValue); }
	float& initialDelay() { return Value<float>(PropOffsets_ScaleWindow.initialDelay); }
	int& initialPos() { return Value<int>(PropOffsets_ScaleWindow.initialPos); }
	float& marginHeight() { return Value<float>(PropOffsets_ScaleWindow.marginHeight); }
	float& marginWidth() { return Value<float>(PropOffsets_ScaleWindow.marginWidth); }
	float& mousePos() { return Value<float>(PropOffsets_ScaleWindow.mousePos); }
	int& numPositions() { return Value<int>(PropOffsets_ScaleWindow.numPositions); }
	uint8_t& orientation() { return Value<uint8_t>(PropOffsets_ScaleWindow.orientation); }
	float& postCapH() { return Value<float>(PropOffsets_ScaleWindow.postCapH); }
	float& postCapHeight() { return Value<float>(PropOffsets_ScaleWindow.postCapHeight); }
	UTexture*& postCapTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.postCapTexture); }
	float& postCapW() { return Value<float>(PropOffsets_ScaleWindow.postCapW); }
	float& postCapWidth() { return Value<float>(PropOffsets_ScaleWindow.postCapWidth); }
	float& postCapXOff() { return Value<float>(PropOffsets_ScaleWindow.postCapXOff); }
	float& postCapYOff() { return Value<float>(PropOffsets_ScaleWindow.postCapYOff); }
	float& preCapH() { return Value<float>(PropOffsets_ScaleWindow.preCapH); }
	float& preCapHeight() { return Value<float>(PropOffsets_ScaleWindow.preCapHeight); }
	UTexture*& preCapTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.preCapTexture); }
	float& preCapW() { return Value<float>(PropOffsets_ScaleWindow.preCapW); }
	float& preCapWidth() { return Value<float>(PropOffsets_ScaleWindow.preCapWidth); }
	float& preCapXOff() { return Value<float>(PropOffsets_ScaleWindow.preCapXOff); }
	float& preCapYOff() { return Value<float>(PropOffsets_ScaleWindow.preCapYOff); }
	uint8_t& repeatDir() { return Value<uint8_t>(PropOffsets_ScaleWindow.repeatDir); }
	float& repeatRate() { return Value<float>(PropOffsets_ScaleWindow.repeatRate); }
	Color& scaleBorderColor() { return Value<Color>(PropOffsets_ScaleWindow.scaleBorderColor); }
	float& scaleBorderSize() { return Value<float>(PropOffsets_ScaleWindow.scaleBorderSize); }
	Color& scaleColor() { return Value<Color>(PropOffsets_ScaleWindow.scaleColor); }
	float& scaleH() { return Value<float>(PropOffsets_ScaleWindow.scaleH); }
	float& scaleHeight() { return Value<float>(PropOffsets_ScaleWindow.scaleHeight); }
	uint8_t& scaleStyle() { return Value<uint8_t>(PropOffsets_ScaleWindow.scaleStyle); }
	UTexture*& scaleTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.scaleTexture); }
	float& scaleW() { return Value<float>(PropOffsets_ScaleWindow.scaleW); }
	float& scaleWidth() { return Value<float>(PropOffsets_ScaleWindow.scaleWidth); }
	float& scaleX() { return Value<float>(PropOffsets_ScaleWindow.scaleX); }
	float& scaleY() { return Value<float>(PropOffsets_ScaleWindow.scaleY); }
	USound*& setSound() { return Value<USound*>(PropOffsets_ScaleWindow.setSound); }
	int& spanRange() { return Value<int>(PropOffsets_ScaleWindow.spanRange); }
	float& startOffset() { return Value<float>(PropOffsets_ScaleWindow.startOffset); }
	Color& thumbBorderColor() { return Value<Color>(PropOffsets_ScaleWindow.thumbBorderColor); }
	float& thumbBorderSize() { return Value<float>(PropOffsets_ScaleWindow.thumbBorderSize); }
	Color& thumbColor() { return Value<Color>(PropOffsets_ScaleWindow.thumbColor); }
	float& thumbH() { return Value<float>(PropOffsets_ScaleWindow.thumbH); }
	int& thumbStep() { return Value<int>(PropOffsets_ScaleWindow.thumbStep); }
	uint8_t& thumbStyle() { return Value<uint8_t>(PropOffsets_ScaleWindow.thumbStyle); }
	UTexture*& thumbTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.thumbTexture); }
	float& thumbW() { return Value<float>(PropOffsets_ScaleWindow.thumbW); }
	float& thumbX() { return Value<float>(PropOffsets_ScaleWindow.thumbX); }
	float& thumbY() { return Value<float>(PropOffsets_ScaleWindow.thumbY); }
	Color& tickColor() { return Value<Color>(PropOffsets_ScaleWindow.tickColor); }
	float& tickH() { return Value<float>(PropOffsets_ScaleWindow.tickH); }
	float& tickHeight() { return Value<float>(PropOffsets_ScaleWindow.tickHeight); }
	uint8_t& tickStyle() { return Value<uint8_t>(PropOffsets_ScaleWindow.tickStyle); }
	UTexture*& tickTexture() { return Value<UTexture*>(PropOffsets_ScaleWindow.tickTexture); }
	float& tickW() { return Value<float>(PropOffsets_ScaleWindow.tickW); }
	float& tickWidth() { return Value<float>(PropOffsets_ScaleWindow.tickWidth); }
	float& tickX() { return Value<float>(PropOffsets_ScaleWindow.tickX); }
	float& tickY() { return Value<float>(PropOffsets_ScaleWindow.tickY); }
	float& toValue() { return Value<float>(PropOffsets_ScaleWindow.toValue); }
	std::string& valueFmt() { return Value<std::string>(PropOffsets_ScaleWindow.valueFmt); }
};
