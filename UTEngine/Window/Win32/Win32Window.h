#pragma once

#include "Window/Window.h"
#include "RenderDevice/Vulkan/VulkanDevice.h"

struct ReceivedWindowMessage
{
	UINT msg;
	WPARAM wparam;
	LPARAM lparam;
};

class Win32Window : public DisplayWindow
{
public:
	Win32Window(Engine* engine);
	~Win32Window();

	void OpenWindow(int width, int height, bool fullscreen) override;
	void CloseWindow() override;
	void* GetDisplay() override { return nullptr; }
	void* GetWindow() override;
	VulkanDevice* GetVulkanDevice() override { return Device.get(); }
	RenderDevice* GetRenderDevice() override { return RenderDevice.get(); }
	void Tick() override;

	void PauseGame();
	void ResumeGame();

	LRESULT OnWindowMessage(UINT msg, WPARAM wparam, LPARAM lparam);
	static LRESULT CALLBACK WndProc(HWND windowhandle, UINT msg, WPARAM wparam, LPARAM lparam);

	Engine* engine = nullptr;

	HWND WindowHandle = 0;
	bool Fullscreen = false;
	bool Paused = false;

	int MouseMoveX = 0;
	int MouseMoveY = 0;

	std::vector<ReceivedWindowMessage> ReceivedMessages;

	std::unique_ptr<VulkanDevice> Device;
	std::unique_ptr<RenderDevice> RenderDevice;
};
