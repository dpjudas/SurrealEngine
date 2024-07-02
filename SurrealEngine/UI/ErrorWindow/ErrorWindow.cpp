
#include "ErrorWindow.h"
#include "Utils/File.h"
#include "Engine.h"
#include <zwidget/core/image.h>
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/scrollbar/scrollbar.h>
#include <zwidget/systemdialogs/save_file_dialog.h>
#include <miniz.h>

void ErrorWindow::ExecModal(const std::string& text, const std::list<LogMessageLine>& log, std::vector<uint8_t> minidump)
{
	Size screenSize = GetScreenSize();
	double windowWidth = 1200.0;
	double windowHeight = 700.0;

	auto window = std::make_unique<ErrorWindow>(std::move(minidump));
	window->SetText(text, log);
	window->SetFrameGeometry((screenSize.width - windowWidth) * 0.5, (screenSize.height - windowHeight) * 0.5, windowWidth, windowHeight);
	window->Show();

	DisplayWindow::RunLoop();
}

ErrorWindow::ErrorWindow(std::vector<uint8_t> initminidump) : Widget(nullptr, WidgetType::Window), minidump(std::move(initminidump))
{
	SetWindowTitle("Fatal Error - Surreal Engine");
	SetWindowBackground(Colorf::fromRgba8(51, 51, 51));
	SetWindowBorderColor(Colorf::fromRgba8(51, 51, 51));
	SetWindowCaptionColor(Colorf::fromRgba8(33, 33, 33));
	SetWindowCaptionTextColor(Colorf::fromRgba8(226, 223, 219));

	LogView = new LogViewer(this);
	ClipboardButton = new PushButton(this);
	ClipboardButton->OnClick = [=]() { OnClipboardButtonClicked(); };
	ClipboardButton->SetText("Copy to clipboard");

	if (minidump.empty())
	{
		CloseButton = new PushButton(this);
		CloseButton->OnClick = [=]() { OnCloseButtonClicked(); };
		CloseButton->SetText("Close");
	}
	else
	{
		SaveReportButton = new PushButton(this);
		SaveReportButton->OnClick = [=]() { OnSaveReportButtonClicked(); };
		SaveReportButton->SetText("Save Report");
	}

	LogView->SetFocus();
}

void ErrorWindow::SetText(const std::string& text, const std::list<LogMessageLine>& log)
{
	LogView->SetText(text, log);

	clipboardtext.clear();
	for (const LogMessageLine& line : log)
	{
		std::string timeText = ToFixed(line.Time);
		if (timeText.size() < 10)
			timeText.resize(10, ' ');

		std::string sourceText = line.Source;
		if (sourceText.size() < 50)
			sourceText.resize(50, ' ');

		clipboardtext += timeText;
		clipboardtext += " ";
		clipboardtext += sourceText;
		clipboardtext += " ";
		clipboardtext += line.Text;
		clipboardtext += "\n";
	}
	clipboardtext += "\nExecution could not continue.\n";
	clipboardtext += text;
	clipboardtext += "\n";
}

std::string ErrorWindow::ToFixed(float time)
{
	std::string fixedTime = std::to_string((int64_t)(time * 1000.0));
	if (fixedTime.size() < 4)
		fixedTime.resize(4, '0');
	return fixedTime.substr(0, fixedTime.size() - 3) + "." + fixedTime.substr(fixedTime.size() - 3);
}

void ErrorWindow::OnClipboardButtonClicked()
{
	SetClipboardText(clipboardtext);
}

void ErrorWindow::OnCloseButtonClicked()
{
	DisplayWindow::ExitLoop();
}

void ErrorWindow::OnSaveReportButtonClicked()
{
	auto dialog = SaveFileDialog::Create(this);
	dialog->AddFilter("Crash Report Zip Files", "*.zip");
	dialog->AddFilter("All Files", "*.*");
	dialog->SetFilename("CrashReport.zip");
	dialog->SetDefaultExtension("zip");
	if (dialog->Show())
	{
		std::string filename = dialog->Filename();

		mz_zip_archive zip = {};
		if (mz_zip_writer_init_heap(&zip, 0, 16 * 1024 * 1024))
		{
			mz_zip_writer_add_mem(&zip, "minidump.dmp", minidump.data(), minidump.size(), MZ_DEFAULT_COMPRESSION);
			mz_zip_writer_add_mem(&zip, "log.txt", clipboardtext.data(), clipboardtext.size(), MZ_DEFAULT_COMPRESSION);
		}
		void* buffer = nullptr;
		size_t buffersize = 0;
		mz_zip_writer_finalize_heap_archive(&zip, &buffer, &buffersize);
		mz_zip_writer_end(&zip);

		try
		{
			File::write_all_bytes(filename, buffer, buffersize);
		}
		catch (...)
		{
		}
	}
}

void ErrorWindow::OnClose()
{
	DisplayWindow::ExitLoop();
}

void ErrorWindow::OnGeometryChanged()
{
	double w = GetWidth();
	double h = GetHeight();

	double y = GetHeight() - 15.0 - ClipboardButton->GetPreferredHeight();
	ClipboardButton->SetFrameGeometry(20.0, y, 170.0, ClipboardButton->GetPreferredHeight());
	if (CloseButton)
		CloseButton->SetFrameGeometry(GetWidth() - 20.0 - 100.0, y, 100.0, CloseButton->GetPreferredHeight());
	else if (SaveReportButton)
		SaveReportButton->SetFrameGeometry(GetWidth() - 20.0 - 100.0, y, 100.0, SaveReportButton->GetPreferredHeight());
	y -= 20.0;

	LogView->SetFrameGeometry(Rect::xywh(0.0, 0.0, w, y));
}

/////////////////////////////////////////////////////////////////////////////

LogViewer::LogViewer(Widget* parent) : Widget(parent)
{
	SetNoncontentSizes(8.0, 8.0, 3.0, 8.0);

	scrollbar = new Scrollbar(this);
	scrollbar->FuncScroll = [=]() { OnScrollbarScroll(); };
}

void LogViewer::SetText(const std::string& text, const std::list<LogMessageLine>&log)
{
	errorlines.clear();
	loglines.clear();

	for (const LogMessageLine& line : log)
	{
		loglines.push_back(CreateLineLayout(line));
	}

	SpanLayout layout;

	// Add an empty line as a bit of spacing
	layout.AddText(" ", largefont, Colorf::fromRgba8(255, 255, 170));
	errorlines.push_back(layout);

	layout.Clear();
	//layout.AddImage(Image::LoadResource("widgets/erroricon.svg"), -8.0);
	layout.AddText("Execution could not continue.", largefont, Colorf::fromRgba8(255, 170, 170));
	errorlines.push_back(layout);

	size_t pos = 0;
	size_t end = text.size();
	while (pos < end)
	{
		size_t linestart = pos;
		size_t lineend = std::min(text.find('\n', linestart), end);
		std::string line = text.substr(linestart, lineend - linestart);
		pos = lineend + 1;
		if (!line.empty() && line.back() == '\r')
			line.pop_back();
		if (line.empty())
			line.push_back(' ');

		layout.Clear();
		layout.AddText(line, largefont, Colorf::fromRgba8(255, 255, 170));
		errorlines.push_back(layout);
	}

	scrollbar->SetRanges(0.0, (double)(loglines.size() + errorlines.size()), 1.0, 100.0);
	scrollbar->SetPosition((double)(loglines.size() + errorlines.size()) - 1.0);

	Update();
}

std::string LogViewer::ToFixed(float time)
{
	std::string fixedTime = std::to_string((int64_t)(time * 1000.0));
	if (fixedTime.size() < 4)
		fixedTime.resize(4, '0');
	return fixedTime.substr(0, fixedTime.size() - 3) + "." + fixedTime.substr(fixedTime.size() - 3);
}

LogViewer::LogLine LogViewer::CreateLineLayout(const LogMessageLine& line)
{
	LogLine logline;
	logline.time.AddText(ToFixed(line.Time), font, Colorf::fromRgba8(200, 200, 200));
	logline.source.AddText(line.Source, font, Colorf::fromRgba8(170, 170, 255));
	logline.text.AddText(line.Text, font, Colorf::fromRgba8(255, 255, 255));
	return logline;
}

void LogViewer::OnPaintFrame(Canvas* canvas)
{
	double w = GetFrameGeometry().width;
	double h = GetFrameGeometry().height;
	Colorf bordercolor = Colorf::fromRgba8(100, 100, 100);
	canvas->fillRect(Rect::xywh(0.0, 0.0, w, h), Colorf::fromRgba8(38, 38, 38));
	//canvas->fillRect(Rect::xywh(0.0, 0.0, w, 1.0), bordercolor);
	//canvas->fillRect(Rect::xywh(0.0, h - 1.0, w, 1.0), bordercolor);
	//canvas->fillRect(Rect::xywh(0.0, 0.0, 1.0, h - 0.0), bordercolor);
	//canvas->fillRect(Rect::xywh(w - 1.0, 0.0, 1.0, h - 0.0), bordercolor);
}

void LogViewer::OnPaint(Canvas* canvas)
{
	double width = GetWidth() - scrollbar->GetFrameGeometry().width;
	double y = GetHeight();
	size_t start = std::min((size_t)std::round(scrollbar->GetPosition() + 1.0), errorlines.size() + loglines.size());
	for (size_t i = start; i > 0 && y > 0.0; i--)
	{
		if (i <= loglines.size())
		{
			LogLine& line = loglines[i - 1];

			line.text.Layout(canvas, width - 400.0);
			line.time.Layout(canvas, 100.0);
			line.source.Layout(canvas, 300.0);

			double lineheight = line.time.GetSize().height;
			lineheight = std::max(lineheight, line.source.GetSize().height);
			lineheight = std::max(lineheight, line.text.GetSize().height);
			y -= lineheight;

			line.time.SetPosition(Point(0.0, y));
			line.source.SetPosition(Point(100.0, y));
			line.text.SetPosition(Point(400.0, y));

			line.time.DrawLayout(canvas);
			line.source.DrawLayout(canvas);
			line.text.DrawLayout(canvas);
		}
		else
		{
			SpanLayout& layout = errorlines[i - loglines.size() - 1];
			layout.Layout(canvas, width);
			layout.SetPosition(Point(0.0, y - layout.GetSize().height));
			layout.DrawLayout(canvas);
			y -= layout.GetSize().height;
		}
	}
}

bool LogViewer::OnMouseWheel(const Point& pos, InputKey key)
{
	if (key == InputKey::MouseWheelUp)
	{
		ScrollUp(4);
	}
	else if (key == InputKey::MouseWheelDown)
	{
		ScrollDown(4);
	}
	return true;
}

void LogViewer::OnKeyDown(InputKey key)
{
	if (key == InputKey::Home)
	{
		scrollbar->SetPosition(0.0f);
		Update();
	}
	if (key == InputKey::End)
	{
		scrollbar->SetPosition(scrollbar->GetMax());
		Update();
	}
	else if (key == InputKey::PageUp)
	{
		ScrollUp(20);
	}
	else if (key == InputKey::PageDown)
	{
		ScrollDown(20);
	}
	else if (key == InputKey::Up)
	{
		ScrollUp(4);
	}
	else if (key == InputKey::Down)
	{
		ScrollDown(4);
	}
}

void LogViewer::OnScrollbarScroll()
{
	Update();
}

void LogViewer::OnGeometryChanged()
{
	double w = GetWidth();
	double h = GetHeight();
	double sw = scrollbar->GetPreferredWidth();
	scrollbar->SetFrameGeometry(Rect::xywh(w - sw, 0.0, sw, h));
}

void LogViewer::ScrollUp(int lines)
{
	scrollbar->SetPosition(std::max(scrollbar->GetPosition() - (double)lines, 0.0));
	Update();
}

void LogViewer::ScrollDown(int lines)
{
	scrollbar->SetPosition(std::min(scrollbar->GetPosition() + (double)lines, scrollbar->GetMax()));
	Update();
}
