
#include "window/window.h"
#include "window/stub/stub_open_folder_dialog.h"
#include "window/stub/stub_open_file_dialog.h"
#include "window/stub/stub_save_file_dialog.h"
#include "core/widget.h"
#include <stdexcept>

std::unique_ptr<DisplayWindow> DisplayWindow::Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner)
{
	return DisplayBackend::Get()->Create(windowHost, popupWindow, owner);
}

void DisplayWindow::ProcessEvents()
{
	DisplayBackend::Get()->ProcessEvents();
}

void DisplayWindow::RunLoop()
{
	DisplayBackend::Get()->RunLoop();
}

void DisplayWindow::ExitLoop()
{
	DisplayBackend::Get()->ExitLoop();
}

void* DisplayWindow::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	return DisplayBackend::Get()->StartTimer(timeoutMilliseconds, onTimer);
}

void DisplayWindow::StopTimer(void* timerID)
{
	DisplayBackend::Get()->StopTimer(timerID);
}

Size DisplayWindow::GetScreenSize()
{
	return DisplayBackend::Get()->GetScreenSize();
}

/////////////////////////////////////////////////////////////////////////////

static std::unique_ptr<DisplayBackend>& GetBackendVar()
{
	// In C++, static variables in functions are constructed on first encounter and is destructed in the reverse order when main() ends.
	static std::unique_ptr<DisplayBackend> p;
	return p;
}

DisplayBackend* DisplayBackend::Get()
{
	return GetBackendVar().get();
}

void DisplayBackend::Set(std::unique_ptr<DisplayBackend> instance)
{
	GetBackendVar() = std::move(instance);
}

std::unique_ptr<OpenFileDialog> DisplayBackend::CreateOpenFileDialog(DisplayWindow* owner)
{
	return std::make_unique<StubOpenFileDialog>(owner);
}

std::unique_ptr<SaveFileDialog> DisplayBackend::CreateSaveFileDialog(DisplayWindow* owner)
{
	return std::make_unique<StubSaveFileDialog>(owner);
}

std::unique_ptr<OpenFolderDialog> DisplayBackend::CreateOpenFolderDialog(DisplayWindow* owner)
{
	return std::make_unique<StubOpenFolderDialog>(owner);
}

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateBackend()
{
	auto backend = TryCreateWin32();
	if (!backend)
	{
		// Check if there is an environment variable specified for the desired backend
		const char* backendSelectionEnv = std::getenv("ZWIDGET_DISPLAY_BACKEND");
		if (backendSelectionEnv)
		{
			std::string backendSelectionStr(backendSelectionEnv);

			if (backendSelectionStr == "X11")
			{
				backend = TryCreateX11();
				if (!backend) TryCreateWayland();
				if (!backend) TryCreateSDL2();
			}
			else if (backendSelectionStr == "SDL2")
			{
				backend = TryCreateSDL2();
				if (!backend) TryCreateWayland();
				if (!backend) TryCreateX11();
			}
			// Wayland is already first priority by default
			// so no need to handle that case here
			else if (backendSelectionStr != "Wayland")
				std::runtime_error("ZWidget: Unrecognized backend: " + backendSelectionStr);
		}
	}
	if (!backend)
	{
		backend = TryCreateWayland();
		if (!backend) TryCreateX11();
		if (!backend) TryCreateSDL2();
	}

	return backend;
}

#ifdef WIN32

#include "win32/win32_display_backend.h"

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateWin32()
{
	return std::make_unique<Win32DisplayBackend>();
}

#else

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateWin32()
{
	return nullptr;
}

#endif

#ifdef USE_SDL2

#include "sdl2/sdl2_display_backend.h"

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateSDL2()
{
	return std::make_unique<SDL2DisplayBackend>();
}

#else

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateSDL2()
{
	return nullptr;
}

#endif

#ifdef USE_X11

#include "x11/x11_display_backend.h"

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateX11()
{
	return std::make_unique<X11DisplayBackend>();
}

#else

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateX11()
{
	return nullptr;
}

#endif

#ifdef USE_WAYLAND

#include "wayland/wayland_display_backend.h"

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateWayland()
{
	return std::make_unique<WaylandDisplayBackend>();
}

#else

std::unique_ptr<DisplayBackend> DisplayBackend::TryCreateWayland()
{
	return nullptr;
}

#endif
