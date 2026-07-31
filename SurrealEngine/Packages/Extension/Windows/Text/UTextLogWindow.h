#pragma once

#include "UTextWindow.h"

class UTextLogWindow : public UTextWindow
{
public:
	using UTextWindow::UTextWindow;

	void AddLog(const std::string& NewText, const Color& linecol);
	void ClearLog();
	void PauseLog(bool bNewPauseState);
	void SetTextTimeout(float newTimeout);

	void Tick(float timeElapsed) override;
	void ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight) override;
	void DrawWindow(UGC* gc) override;

	//DynamicArray& Lines() { return Value<DynamicArray>(PropOffsets_TextLogWindow.Lines); }
	BitfieldBool bPaused() { return BoolValue(PropOffsets_TextLogWindow.bPaused); }
	BitfieldBool bTooTall() { return BoolValue(PropOffsets_TextLogWindow.bTooTall); }
	float& textTimeout() { return Value<float>(PropOffsets_TextLogWindow.textTimeout); }

private:
	struct LogEntry
	{
		std::string text;
		Color color;
		float timestamp = 0.0f;
	};
	Array<LogEntry> logs;
	float totalTime = 0.0f;
};
