
#include "Precomp.h"
#include "EditorApp.h"
#include "Window/Window.h"
#include "UI/Core/Element/Canvas.h"
#include "UI/Core/Element/Colorf.h"
#include "UI/Core/Element/Rect.h"

bool exiteditor;

class EditorWindowFrame : public DisplayWindowHost
{
public:
	EditorWindowFrame()
	{
		window = DisplayWindow::Create(this);
		window->OpenWindow(1920, 1080, false);

		canvas = Canvas::create(window->GetRenderDevice());
	}

	~EditorWindowFrame()
	{
		window.reset();
	}

	void Key(DisplayWindow* window, std::string key) override
	{
	}

	void InputEvent(DisplayWindow* window, EInputKey key, EInputType type, int delta)
	{
	}

	void FocusChange(bool lost) override
	{
	}

	void MouseMove(float x, float y) override
	{
	}

	bool MouseCursorVisible() override
	{
		return true;
	}

	void WindowClose(DisplayWindow* window) override
	{
		exiteditor = true;
	}

	std::unique_ptr<DisplayWindow> window;
	std::unique_ptr<Canvas> canvas;
};

int EditorApp::main(std::vector<std::string> args)
{
	EditorWindowFrame frame;

	while (!exiteditor)
	{
		frame.canvas->begin(Colorf(80 / 255.0f, 80 / 255.0f, 80 / 255.0f));
		frame.canvas->fillRect(Rect::xywh(0.0, 0.0, 1920.0, 32.0), Colorf(240/255.0f, 240/255.0f, 240/255.0f));
		frame.canvas->drawText(Point(16.0, 21.0), Colorf(0.0f, 0.0f, 0.0f), "File      Edit      View      Tools      Window     Help");

		frame.canvas->fillRect(Rect::xywh(1920.0 * 0.5, 32.0, 8.0, 1080.0 - 32.0), Colorf(240 / 255.0f, 240 / 255.0f, 240 / 255.0f));
		frame.canvas->fillRect(Rect::xywh(0.0, 1080.0 * 0.5 - 30.0f, 1920.0, 8.0 + 30.0f), Colorf(240 / 255.0f, 240 / 255.0f, 240 / 255.0f));
		frame.canvas->fillRect(Rect::xywh(0.0, 1080.0 - 30.0f, 1920.0, 30.0f), Colorf(240 / 255.0f, 240 / 255.0f, 240 / 255.0f));

		frame.canvas->drawText(Point(12.0, 1080.0 * 0.5 - 12.0), Colorf(0.2f, 0.2f, 0.2f), "Viewport 1");
		frame.canvas->drawText(Point(1920.0 * 0.5 + 12.0, 1080.0 * 0.5 - 12.0), Colorf(0.2f, 0.2f, 0.2f), "Viewport 2");
		frame.canvas->drawText(Point(12.0, 1080.0 - 12.0), Colorf(0.2f, 0.2f, 0.2f), "Viewport 3");
		frame.canvas->drawText(Point(1920.0 * 0.5 + 12.0, 1080.0 - 12.0), Colorf(0.2f, 0.2f, 0.2f), "Viewport 4");


		frame.canvas->end();

		frame.window->Tick();
#ifdef WIN32
		Sleep(10); // To do: improve DisplayWindow to support waiting for messages
#endif
	}

	return 0;
}
