#pragma once

#include "window/window.h"

class Win32DisplayBackend : public DisplayBackend
{
public:
	std::unique_ptr<DisplayWindow> Create(DisplayWindowHost* windowHost, bool popupWindow, DisplayWindow* owner) override;
	void ProcessEvents() override;
	void RunLoop() override;
	void ExitLoop() override;

	void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer) override;
	void StopTimer(void* timerID) override;

	Size GetScreenSize() override;

	std::unique_ptr<OpenFileDialog> CreateOpenFileDialog(Widget* owner) override;
	std::unique_ptr<SaveFileDialog> CreateSaveFileDialog(Widget* owner) override;
	std::unique_ptr<OpenFolderDialog> CreateOpenFolderDialog(Widget* owner) override;
};
