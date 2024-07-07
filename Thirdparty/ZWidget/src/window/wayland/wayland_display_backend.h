#pragma once

#include "window/window.h"

class WaylandDisplayBackend : public DisplayBackend
{
public:
	std::unique_ptr<DisplayWindow> Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner) override;
	void ProcessEvents() override;
	void RunLoop() override;
	void ExitLoop() override;

	void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer) override;
	void StopTimer(void* timerID) override;

	Size GetScreenSize() override;

	bool IsWayland() override { return true; }

#ifdef USE_DBUS
	std::unique_ptr<OpenFileDialog> CreateOpenFileDialog(DisplayWindow* owner) override;
	std::unique_ptr<SaveFileDialog> CreateSaveFileDialog(DisplayWindow* owner) override;
	std::unique_ptr<OpenFolderDialog> CreateOpenFolderDialog(DisplayWindow* owner) override;
#endif
};
