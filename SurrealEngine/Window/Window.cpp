
#include "Precomp.h"
#include "Window.h"

#ifdef WIN32
#include "Win32/Win32Window.h"
#endif

#ifdef USE_SDL2
#include "SDL2/SDL2Window.h"
#endif

#include <cstdio>
#include <cmath>

std::string GameWindow::windowingSystemName;

std::unique_ptr<GameWindow> GameWindow::Create(GameWindowHost* windowHost, std::string& windowingSystemName)
{
	if (windowingSystemName.empty())
		Exception::Throw("Windowing system field is empty.");

#if !defined(WIN32)
	if (windowingSystemName == "Win32")
		Exception::Throw("Win32 windowing system can only work on the Windows version of SurrealEngine");
#endif

#if !defined(USE_SDL2)
	if (windowingSystemName == "SDL2")
		Exception::Throw("SurrealEngine is built without SDL2 support. Windowing system cannot be SDL2");
#endif

	GameWindow::windowingSystemName = windowingSystemName;

#if defined(WIN32)
	if (windowingSystemName == "Win32")
		return std::make_unique<Win32Window>(windowHost);
#endif

#if defined(USE_SDL2)
	if (windowingSystemName == "SDL2")
		return std::make_unique<SDL2Window>(windowHost);
#endif

	Exception::Throw("Invalid Windowing system name: " + windowingSystemName);
}

void GameWindow::ProcessEvents()
{
#if defined(WIN32)
	if (windowingSystemName == "Win32")
	{
		Win32Window::ProcessEvents();
		return;
	}
#endif
#if defined(USE_SDL2)
	if (windowingSystemName == "SDL2")
	{
		SDL2Window::ProcessEvents();
		return;
	}
#endif
}

void GameWindow::RunLoop()
{
#if defined(WIN32)
	if (windowingSystemName == "Win32")
	{
		Win32Window::RunLoop();
		return;
	}
#endif
#if defined(USE_SDL2)
	if (windowingSystemName == "SDL2")
	{
		SDL2Window::RunLoop();
		return;
	}
#endif
}

void GameWindow::ExitLoop()
{
#if defined(WIN32)
	if (windowingSystemName == "Win32")
	{
		Win32Window::ExitLoop();
		return;
	}
#endif
#if defined(USE_SDL2)
	if (windowingSystemName == "SDL2")
	{
		SDL2Window::ExitLoop();
		return;
	}
#endif
}

std::string GameWindow::GetAvailableResolutions() const
{
	std::string result = "";

	auto resolutions = QueryAvailableResolutions();

	// "Flatten" the resolutions list into a single string
	for (int i = 0; i < resolutions.size(); i++)
	{
		auto& res = resolutions[i];
		std::string resString = std::to_string(int(res.width)) + "x" + std::to_string(int(res.height));

		result += resString;
		if (i < resolutions.size() - 1)
			result += " ";
	}

	return result;
}

void GameWindow::AddResolutionIfNotAdded(std::vector<Size>& resList, Size resolution) const
{
	// Skip over the current resolution if it is already inserted
	// (in case of multiple refresh rates being available for the display)
	for (auto& res : resList)
	{
		if (resolution == res)
			return;
	}

	// Add the resolution, as it is not added before
	resList.push_back(resolution);
}

Size GameWindow::ParseResolutionString(std::string& resolutionString) const
{
	if (resolutionString.empty())
		return Size(0, 0);

#ifdef WIN32
	int width, height;
	int parsedDataCount = sscanf_s(resolutionString.c_str(), "%dx%d", &width, &height);
#else
	int width, height;
	int parsedDataCount = sscanf(resolutionString.c_str(), "%dx%d", &width, &height);
#endif

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

Size GameWindow::GetClosestResolution(Size resolution) const
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

void GameWindow::SetResolution(std::string& resolutionString)
{
	Size parsedResolution = ParseResolutionString(resolutionString);
	if (parsedResolution == Size(0, 0))
		return;

	Rect windowRect = GetWindowFrame();

	if (isWindowFullscreen)
	{
		parsedResolution = GetClosestResolution(parsedResolution);
		windowRect.x = 0;
		windowRect.y = 0;
	}
		
	
#ifdef WIN32
	auto dpi = GetDpiScale();
	windowRect.width = parsedResolution.width / dpi;
	windowRect.height = parsedResolution.height / dpi;
#else
	windowRect.width = parsedResolution.width;
	windowRect.height = parsedResolution.height;
#endif

	SetWindowFrame(windowRect);
}