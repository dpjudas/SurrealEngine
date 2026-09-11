#include "sdl3_display_backend.h"
#include "sdl3_display_window.h"
#include <stdexcept>
#include <vector>
#include <SDL3/SDL_video.h>
#include <SDL3/SDL_joystick.h>
#ifndef WIN32
#include <dlfcn.h>
#endif

SDL3DisplayBackend::SDL3DisplayBackend()
{
	if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_EVENTS | SDL_INIT_JOYSTICK | SDL_INIT_GAMEPAD))
		throw std::runtime_error(std::string("Unable to initialize SDL:") + SDL_GetError());

	SDL3DisplayWindow::PaintEventNumber = SDL_RegisterEvents(1);

	int joystickCount;
	const SDL_JoystickID* joystickIDs = SDL_GetJoysticks(&joystickCount);

	for (auto i = 0; i < joystickCount ; i++)
	{
		SDL_OpenJoystick(joystickIDs[i]);
	}
}

std::unique_ptr<DisplayWindow> SDL3DisplayBackend::Create(DisplayWindowHost* windowHost, WidgetType type, DisplayWindow* owner, RenderAPI renderAPI)
{
	return std::make_unique<SDL3DisplayWindow>(windowHost, type, static_cast<SDL3DisplayWindow*>(owner), renderAPI, UIScale);
}

void SDL3DisplayBackend::ProcessEvents()
{
	SDL3DisplayWindow::ProcessEvents();
}

void SDL3DisplayBackend::RunLoop()
{
	SDL3DisplayWindow::RunLoop();
}

void SDL3DisplayBackend::ExitLoop()
{
	SDL3DisplayWindow::ExitLoop();
}

Size SDL3DisplayBackend::GetScreenSize()
{
	SDL_Rect rect = {};
	SDL_DisplayID *displays = SDL_GetDisplays(nullptr);
	if (!displays || !SDL_GetDisplayBounds(displays[0], &rect))
		throw std::runtime_error(std::string("Unable to get screen size:") + SDL_GetError());
	SDL_free(displays);

	return {rect.w / UIScale, rect.h / UIScale};
}

std::vector<Size> SDL3DisplayBackend::GetAvailableResolutions() const
{
	std::vector<Size> result;
	int displayCount = 0;
	SDL_DisplayID* displays = SDL_GetDisplays(&displayCount);
	if (!displays || displayCount < 1)
		return result;

	SDL_DisplayID displayID = displays[0];
	int modeCount = 0;
	SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(displayID, &modeCount);
	if (modes)
	{
		for (int i = 0; i < modeCount; ++i)
		{
			const SDL_DisplayMode* mode = modes[i];
			if (!mode || mode->w <= 0 || mode->h <= 0)
				continue;

			Size resolution(mode->w / UIScale, mode->h / UIScale);
			bool alreadyAdded = false;
			for (const Size& existing : result)
			{
				if (existing == resolution)
				{
					alreadyAdded = true;
					break;
				}
			}
			if (!alreadyAdded)
				result.push_back(resolution);
		}
		SDL_free(modes);
	}
	SDL_free(displays);

	if (result.empty())
	{
		SDL_Rect rect = {};
		SDL_DisplayID* fallbackDisplays = SDL_GetDisplays(&displayCount);
		if (fallbackDisplays && displayCount > 0 && SDL_GetDisplayBounds(fallbackDisplays[0], &rect))
			result.push_back(Size(rect.w / UIScale, rect.h / UIScale));
		SDL_free(fallbackDisplays);
	}

	return result;
}

std::vector<DisplayMode> SDL3DisplayBackend::GetAvailableDisplayModes() const
{
	std::vector<DisplayMode> result;
	int displayCount = 0;
	SDL_DisplayID* displays = SDL_GetDisplays(&displayCount);
	if (!displays || displayCount < 1)
		return result;

	SDL_DisplayID displayID = displays[0];
	int modeCount = 0;
	SDL_DisplayMode** modes = SDL_GetFullscreenDisplayModes(displayID, &modeCount);
	if (modes)
	{
		for (int i = 0; i < modeCount; ++i)
		{
			const SDL_DisplayMode* mode = modes[i];
			if (!mode || mode->w <= 0 || mode->h <= 0)
				continue;

			DisplayMode displayMode;
			displayMode.resolution = Size(mode->w / UIScale, mode->h / UIScale);
			displayMode.refreshRate = mode->refresh_rate > 0 ? mode->refresh_rate : 0;
			bool alreadyAdded = false;
			for (const DisplayMode& existing : result)
			{
				if (existing == displayMode)
				{
					alreadyAdded = true;
					break;
				}
			}
			if (!alreadyAdded)
				result.push_back(displayMode);
		}
		SDL_free(modes);
	}
	SDL_free(displays);

	return result;
}

void* SDL3DisplayBackend::StartTimer(int timeoutMilliseconds, std::function<void()> onTimer)
{
	return SDL3DisplayWindow::StartTimer(timeoutMilliseconds, std::move(onTimer));
}

void SDL3DisplayBackend::StopTimer(void* timerID)
{
	SDL3DisplayWindow::StopTimer(timerID);
}
