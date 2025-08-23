#pragma once

#include <zwidget/core/widget.h>
#include <zwidget/core/span_layout.h>
#include <list>

class LogViewer;
class PushButton;
class Scrollbar;
struct LogMessageLine;

class ErrorWindow : public Widget
{
public:
	static bool CheckCrashReporter();
	static void ExecModal(const std::string& text, const std::list<LogMessageLine>& log, Array<uint8_t> minidump = {});

	ErrorWindow(Array<uint8_t> minidump);

protected:
	void OnClose() override;
	void OnGeometryChanged() override;

private:
	void SetText(const std::string& text, const std::list<LogMessageLine>& log);
	static std::string ToFixed(float time);

	void OnClipboardButtonClicked();
	void OnCloseButtonClicked();
	void OnSaveReportButtonClicked();

	LogViewer* LogView = nullptr;
	PushButton* ClipboardButton = nullptr;
	PushButton* CloseButton = nullptr;
	PushButton* SaveReportButton = nullptr;

	Array<uint8_t> minidump;
	std::string clipboardtext;
};

class LogViewer : public Widget
{
public:
	LogViewer(Widget* parent);

	void SetText(const std::string& text, const std::list<LogMessageLine>& log);

protected:
	void OnPaintFrame(Canvas* canvas) override;
	void OnPaint(Canvas* canvas) override;
	bool OnMouseWheel(const Point& pos, InputKey key) override;
	void OnKeyDown(InputKey key) override;
	void OnGeometryChanged() override;

private:
	void OnScrollbarScroll();
	void ScrollUp(int lines);
	void ScrollDown(int lines);

	struct LogLine
	{
		SpanLayout time;
		SpanLayout source;
		SpanLayout text;
	};

	LogLine CreateLineLayout(const LogMessageLine& line);
	static std::string ToFixed(float time);

	Scrollbar* scrollbar = nullptr;

	std::shared_ptr<Font> largefont = Font::Create("Poppins", 16.0);
	std::shared_ptr<Font> font = Font::Create("Poppins", 12.0);
	Array<LogLine> loglines;
	Array<SpanLayout> errorlines;
};
