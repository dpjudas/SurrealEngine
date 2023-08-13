
#include "Precomp.h"
#include "EditorApp.h"
#include "CommandLine.h"
#include "GameFolder.h"
#include "Engine.h"
#include "Render/RenderSubsystem.h"
#include "Package/PackageManager.h"
#include "Window/Window.h"
#include "UI/Core/Canvas.h"
#include "UI/Core/Colorf.h"
#include "UI/Core/Rect.h"

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

	CommandLine cmd(args);
	commandline = &cmd;
	GameLaunchInfo launchinfo = GameFolderSelection::GetLaunchInfo();

	Engine editorengine(launchinfo);
	engine->render = std::make_unique<RenderSubsystem>(frame.window->GetRenderDevice());

	engine->LevelPackage = engine->packages->GetPackage("CTF-Face");
	engine->LevelInfo = UObject::Cast<ULevelInfo>(engine->LevelPackage->GetUObject("LevelInfo", "LevelInfo0"));
	engine->Level = UObject::Cast<ULevel>(engine->LevelPackage->GetUObject("Level", "MyLevel"));
	engine->CameraActor = UObject::Cast<UActor>(engine->packages->NewObject("camera", "Engine", "Camera"));

	while (!exiteditor)
	{
		frame.canvas->begin(Colorf(20 / 255.0f, 20 / 255.0f, 20 / 255.0f));
		frame.canvas->fillRect(Rect::xywh(0.0, 0.0, 1920.0, 32.0), Colorf(240/255.0f, 240/255.0f, 240/255.0f));
		frame.canvas->drawText(Point(16.0, 21.0), Colorf(0.0f, 0.0f, 0.0f), "File      Edit      View      Tools      Window     Help");

		frame.canvas->fillRect(Rect::xywh(1920.0 * 0.5, 32.0, 8.0, 1080.0 - 32.0), Colorf(240 / 255.0f, 240 / 255.0f, 240 / 255.0f));
		frame.canvas->fillRect(Rect::xywh(0.0, 1080.0 * 0.5 - 30.0f, 1920.0, 8.0 + 30.0f), Colorf(240 / 255.0f, 240 / 255.0f, 240 / 255.0f));
		frame.canvas->fillRect(Rect::xywh(0.0, 1080.0 - 30.0f, 1920.0, 30.0f), Colorf(240 / 255.0f, 240 / 255.0f, 240 / 255.0f));

		frame.canvas->drawText(Point(12.0, 1080.0 * 0.5 - 12.0), Colorf(0.2f, 0.2f, 0.2f), "Viewport 1");
		frame.canvas->drawText(Point(1920.0 * 0.5 + 12.0, 1080.0 * 0.5 - 12.0), Colorf(0.2f, 0.2f, 0.2f), "Viewport 2");
		frame.canvas->drawText(Point(12.0, 1080.0 - 12.0), Colorf(0.2f, 0.2f, 0.2f), "Viewport 3");
		frame.canvas->drawText(Point(1920.0 * 0.5 + 12.0, 1080.0 - 12.0), Colorf(0.2f, 0.2f, 0.2f), "Viewport 4");

		engine->CameraLocation = vec3(0.0f, 0.0f, 600.0f);
		engine->CameraRotation = Rotator(0, 0, 0);
		engine->ViewportX = 0;
		engine->ViewportY = 32;
		engine->ViewportWidth = 1920 / 2;
		engine->ViewportHeight = 1080 / 2 - 30 - 32;
		engine->render->DrawEditorViewport();

		engine->CameraRotation = Rotator(-1000, 0, 0);
		engine->ViewportX = 1920 / 2 + 8;
		engine->ViewportY = 32;
		engine->ViewportWidth = 1920 / 2 - 8;
		engine->ViewportHeight = 1080 / 2 - 30 - 32;
		engine->render->DrawEditorViewport();

		engine->CameraRotation = Rotator(-2000, 0, 0);
		engine->ViewportX = 0;
		engine->ViewportY = 1080 / 2 + 8;
		engine->ViewportWidth = 1920 / 2;
		engine->ViewportHeight = 1080 / 2 - 30 - 8;
		engine->render->DrawEditorViewport();

		engine->CameraRotation = Rotator(-3000, 0, 0);
		engine->ViewportX = 1920 / 2 + 8;
		engine->ViewportY = 1080 / 2 + 8;
		engine->ViewportWidth = 1920 / 2 - 8;
		engine->ViewportHeight = 1080 / 2 - 30 - 8;
		engine->render->DrawEditorViewport();

		frame.canvas->end();

		frame.window->Tick();
#ifdef WIN32
		Sleep(10); // To do: improve DisplayWindow to support waiting for messages
#endif
	}

	return 0;
}
