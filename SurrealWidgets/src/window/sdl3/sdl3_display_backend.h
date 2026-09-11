#pragma once

#include "surrealwidgets/window/window.h"

class SDL3DisplayBackend : public DisplayBackend
{
public:
	SDL3DisplayBackend();

	std::unique_ptr<DisplayWindow> Create(DisplayWindowHost* windowHost, WidgetType type, DisplayWindow* owner, RenderAPI renderAPI) override;
	void ProcessEvents() override;
	void RunLoop() override;
	void ExitLoop() override;

	void* StartTimer(int timeoutMilliseconds, std::function<void()> onTimer) override;
	void StopTimer(void* timerID) override;

	Size GetScreenSize() override;
	std::vector<Size> GetAvailableResolutions() const override;
	std::vector<DisplayMode> GetAvailableDisplayModes() const override;

	bool IsSDL3() override { return true; }

private:
	double UIScale = 1.0;
};
