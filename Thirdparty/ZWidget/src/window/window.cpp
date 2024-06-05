
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
