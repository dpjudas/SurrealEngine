
#include "Precomp.h"
#include "Window.h"
#ifdef WIN32
#include "Win32/Win32Window.h"
#elif defined(USE_SDL)
#include "SDL2/SDL2Window.h"
#else
#include "X11/X11Window.h"
#endif

#include <cstdio>
#include <cmath>

// TODO: base this off of ini setting, not dependent on OS macro

#ifdef WIN32

std::unique_ptr<DisplayWindow> DisplayWindow::Create(DisplayWindowHost* windowHost)
{
	return std::make_unique<Win32Window>(windowHost);
}

void DisplayWindow::ProcessEvents()
{
	Win32Window::ProcessEvents();
}

void DisplayWindow::RunLoop()
{
	Win32Window::RunLoop();
}

void DisplayWindow::ExitLoop()
{
	Win32Window::ExitLoop();
}

#elif defined(USE_SDL)

std::unique_ptr<DisplayWindow> DisplayWindow::Create(DisplayWindowHost* windowHost)
{
	return std::make_unique<SDL2Window>(windowHost);
}

void DisplayWindow::ProcessEvents()
{
	SDL2Window::ProcessEvents();
}

void DisplayWindow::RunLoop()
{
	SDL2Window::RunLoop();
}

void DisplayWindow::ExitLoop()
{
	SDL2Window::ExitLoop();
}

#else

std::unique_ptr<DisplayWindow> DisplayWindow::Create(DisplayWindowHost* windowHost)
{
	return std::make_unique<X11Window>(windowHost);
}

void DisplayWindow::ProcessEvents()
{
	X11Window::ProcessEvents();
}

void DisplayWindow::RunLoop()
{
	X11Window::RunLoop();
}

void DisplayWindow::ExitLoop()
{
	X11Window::ExitLoop();
}

#endif

Size DisplayWindow::ParseResolutionString(std::string& resolutionString)
{
	if (resolutionString.empty())
		return Size(0, 0);

	int width, height;
	int parsedDataCount = sscanf(resolutionString.c_str(), "%dx%d", &width, &height);

	// Handle incorrect parsings
	// sscanf() returns EOF on input failure, or the amount of "data processed" otherwise
	// Since we want to process width and height of a window, we expect it to return 2
	if (parsedDataCount == EOF || parsedDataCount != 2)
	{
		return Size(0, 0);
	}

	// Resolution shouldn't be smaller than 640x480
	if (width < 640 || height < 480)
	{
		// Maybe produce a log here too?
		return Size(0, 0);
	}

	return Size(width, height);
}

Size DisplayWindow::GetClosestResolution(Size resolution) const
{
	auto resolutions = QueryAvailableResolutions();

	if (resolutions.empty())
		return resolution;

	if (resolutions.size() == 1)
		return resolutions[0];

	int index = 0;
	double minDist = abs(pow(resolutions[0].width - resolution.width, 2) + pow(resolutions[0].height - resolution.height, 2));

	for (int i = 1; i < resolutions.size(); i++)
	{
		auto& currRes = resolutions[i];

		double dist = abs(pow(currRes.width - resolution.width, 2) + pow(currRes.height - resolution.height, 2));

		if (currRes == resolution)
			return resolutions[i];
		
		if (dist < minDist)
		{
			minDist = dist;
			index = i;
		}
	}

	return resolutions[index];
}

void DisplayWindow::SetResolution(std::string& resolutionString)
{
	Size parsedResolution = ParseResolutionString(resolutionString);
	if (parsedResolution.width == 0 || parsedResolution.height == 0)
		return;

	if (isWindowFullscreen)
		parsedResolution = GetClosestResolution(parsedResolution);

	Rect windowRect = GetWindowFrame();

	windowRect.width = parsedResolution.width;
	windowRect.height = parsedResolution.height;

	SetWindowFrame(windowRect);
}
