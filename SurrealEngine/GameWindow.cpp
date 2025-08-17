
#include "Precomp.h"
#include "GameWindow.h"
#include "RenderDevice/RenderDevice.h"
#include <zvulkan/vulkansurface.h>
#include <zvulkan/vulkancompatibledevice.h>
#include <zvulkan/vulkanbuilders.h>

GameWindow::GameWindow(GameWindowHost* windowHost, RenderAPI renderAPI) : Widget(nullptr, WidgetType::Window, renderAPI), windowHost(windowHost)
{
	if (renderAPI == RenderAPI::D3D11)
	{
		device = RenderDevice::CreateD3D11(this);
	}
	else if (renderAPI == RenderAPI::Vulkan)
	{
		std::shared_ptr<VulkanInstance> instance = VulkanInstanceBuilder()
			.RequireExtensions(GetVulkanInstanceExtensions())
			.OptionalSwapchainColorspace()
			.DebugLayer(false)
			.Create();

		auto surface = std::make_shared<VulkanSurface>(instance, CreateVulkanSurface(instance->Instance));
		if (!surface)
			Exception::Throw("No vulkan surface found");

		device = RenderDevice::CreateVulkan(this, surface);
	}
	else
	{
		Exception::Throw("Unsupported render API");
	}

	SetCanvas(std::make_unique<RenderDeviceCanvas>(device.get()));
	SetFocus();
}

RenderDevice* GameWindow::GetRenderDevice()
{
	return device.get();
}

int GameWindow::GetPixelWidth()
{
	return GetNativePixelWidth();
}

int GameWindow::GetPixelHeight()
{
	return GetNativePixelHeight();
}

void GameWindow::ToggleWindowFullscreen(Size newResolution)
{
	if (this->IsFullscreen())
	{
		// First switch to normal
		this->ShowNormal();

		// Then resize the window
		auto geometry = this->GetFrameGeometry();
		geometry.width = newResolution.width;
		geometry.height = newResolution.height;
		this->SetFrameGeometry(geometry);
	}
	else
	{
		// Get the nearest valid fullscreen resolution first
		auto selectedResolution = GetClosestResolution(newResolution);

		// Then resize the window to the resolution
		auto geometry = this->GetFrameGeometry();
		geometry.width = selectedResolution.width;
		geometry.height = selectedResolution.height;
		this->SetFrameGeometry(geometry);

		// Finally switch to fullscreen
		this->ShowFullscreen();
	}
}

bool GameWindow::GetKeyState(EInputKey key)
{
	return Widget::GetKeyState((InputKey)key);
}

Array<Size> GameWindow::QueryAvailableResolutions() const
{
	return { DisplayBackend::Get()->GetScreenSize() };
}

void GameWindow::OnPaint(Canvas* canvas)
{
	windowHost->OnWindowPaint();
}

bool GameWindow::OnMouseDown(const Point& pos, InputKey key)
{
	windowHost->OnWindowMouseDown(pos, (EInputKey)key);
	return true;
}

bool GameWindow::OnMouseDoubleclick(const Point& pos, InputKey key)
{
	windowHost->OnWindowMouseDoubleclick(pos, (EInputKey)key);
	return true;
}

bool GameWindow::OnMouseUp(const Point& pos, InputKey key)
{
	windowHost->OnWindowMouseUp(pos, (EInputKey)key);
	return true;
}

bool GameWindow::OnMouseWheel(const Point& pos, InputKey key)
{
	windowHost->OnWindowMouseWheel(pos, (EInputKey)key);
	return true;
}

void GameWindow::OnMouseMove(const Point& pos)
{
	windowHost->OnWindowMouseMove(pos);
}

void GameWindow::OnRawMouseMove(int dx, int dy)
{
	windowHost->OnWindowRawMouseMove(dx, dy);
}

void GameWindow::OnKeyChar(std::string chars)
{
	windowHost->OnWindowKeyChar(chars);
}

void GameWindow::OnKeyDown(InputKey key)
{
	windowHost->OnWindowKeyDown((EInputKey)key);
}

void GameWindow::OnKeyUp(InputKey key)
{
	windowHost->OnWindowKeyUp((EInputKey)key);
}

void GameWindow::OnGeometryChanged()
{
	windowHost->OnWindowGeometryChanged();
}

void GameWindow::OnClose()
{
	windowHost->OnWindowClose();
}

void GameWindow::OnSetFocus()
{
	windowHost->OnWindowActivated();
}

void GameWindow::OnLostFocus()
{
	windowHost->OnWindowDeactivated();
}

std::unique_ptr<GameWindow> GameWindow::Create(GameWindowHost* windowHost)
{
#ifdef WIN32
	return std::make_unique<GameWindow>(windowHost, RenderAPI::D3D11);
#else
	return std::make_unique<GameWindow>(windowHost, RenderAPI::Vulkan);
#endif
}

void GameWindow::ProcessEvents()
{
	DisplayBackend::Get()->ProcessEvents();
}

void GameWindow::RunLoop()
{
	DisplayBackend::Get()->RunLoop();
}

void GameWindow::ExitLoop()
{
	DisplayBackend::Get()->ExitLoop();
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

void GameWindow::AddResolutionIfNotAdded(Array<Size>& resList, Size resolution) const
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

Size GameWindow::ParseResolutionString(const std::string& resolutionString) const
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

void GameWindow::SetResolution(const std::string& resolutionString)
{
	Size parsedResolution = ParseResolutionString(resolutionString);
	if (parsedResolution == Size(0, 0))
		return;

	Rect windowRect = GetFrameGeometry();

	if (IsFullscreen())
	{
		parsedResolution = GetClosestResolution(parsedResolution);
		windowRect.x = 0;
		windowRect.y = 0;
	}
		
	auto dpi = GetDpiScale();
	windowRect.width = parsedResolution.width / dpi;
	windowRect.height = parsedResolution.height / dpi;

	SetFrameGeometry(windowRect);
}
