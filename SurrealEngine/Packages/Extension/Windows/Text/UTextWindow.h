#pragma once

#include "Packages/Extension/Windows/UWindow.h"

class UTextWindow : public UWindow
{
public:
	using UWindow::UWindow;

	void AppendText(const std::string& NewText);
	void EnableTextAsAccelerator(std::optional<bool> bEnable);
	std::string GetText();
	int GetTextLength();
	int GetTextPart(int startPos, int Count, std::string& OutText);
	void ResetLines();
	void ResetMinWidth();
	void SetLines(int newMinLines, int newMaxLines);
	void SetMaxLines(int newMaxLines);
	void SetMinLines(int newMinLines);
	void SetMinWidth(float newMinWidth);
	void SetText(const std::string& NewText);
	void SetTextAlignments(uint8_t newHAlign, uint8_t newVAlign);
	void SetTextMargins(float newHMargin, float newVMargin);
	void SetWordWrap(bool bNewWordWrap);

	void InitWindow() override;
	void ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight) override;
	void DrawWindow(UGC* gc) override;

	uint8_t& HAlign() { return Value<uint8_t>(PropOffsets_TextWindow.HAlign); }
	int& MaxLines() { return Value<int>(PropOffsets_TextWindow.MaxLines); }
	float& MinWidth() { return Value<float>(PropOffsets_TextWindow.MinWidth); }
	std::string& Text() { return Value<std::string>(PropOffsets_TextWindow.Text); }
	uint8_t& VAlign() { return Value<uint8_t>(PropOffsets_TextWindow.VAlign); }
	BitfieldBool bTextIsAccelerator() { return BoolValue(PropOffsets_TextWindow.bTextIsAccelerator); }
	BitfieldBool bWordWrap() { return BoolValue(PropOffsets_TextWindow.bWordWrap); }
	float& hMargin() { return Value<float>(PropOffsets_TextWindow.hMargin); }
	int& minLines() { return Value<int>(PropOffsets_TextWindow.minLines); }
	float& vMargin() { return Value<float>(PropOffsets_TextWindow.vMargin); }
};
