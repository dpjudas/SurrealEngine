#pragma once

#include <map>

#include <SDL2/SDL.h>
#include <SDL2/SDL_vulkan.h>

#include "Window/Window.h"

class SDL2Window : public DisplayWindow
{
	SDL_Window* m_SDLWindow;
public:
	SDL2Window(DisplayWindowHost *windowHost);
	~SDL2Window();

	static void ProcessEvents();
	static void RunLoop();
	static void ExitLoop();

	void SDLWindowError(const std::string&& message) const;
	void OnSDLEvent(SDL_Event& event);

	void SetWindowTitle(const std::string& text) override;
	void SetWindowFrame(const Rect& box) override;
	void SetClientFrame(const Rect& box) override;
	void Show() override;
	void ShowFullscreen() override;
	void ShowMaximized() override;
	void ShowMinimized() override;
	void ShowNormal() override;
	void Hide() override;
	void Activate() override;
	void ShowCursor(bool enable) override;
	void LockCursor() override;
	void UnlockCursor() override;
	void Update() override;
	bool GetKeyState(EInputKey key) override;

	RenderDevice* GetRenderDevice() override { return rendDevice.get(); }

	void OnKeyboardInput(SDL_KeyboardEvent& event);
	void OnKeyboardTextInput(SDL_TextInputEvent& event);
	void OnMouseInput(SDL_MouseButtonEvent& event);
	void OnMouseWheel(SDL_MouseWheelEvent& event);
	void OnMouseMove(SDL_MouseMotionEvent& event);

	Rect GetWindowFrame() const override;
	Size GetClientSize() const override;
	int GetPixelWidth() const override;
	int GetPixelHeight() const override;
	double GetDpiScale() const override;
	std::vector<Size> QueryAvailableResolutions() const override;

	EInputKey SDLScancodeToInputKey(SDL_Scancode keycode);
	SDL_Scancode InputKeyToSDLScancode(EInputKey inputkey);

	DisplayWindowHost* windowHost = nullptr;
	std::unique_ptr<RenderDevice> rendDevice;

	static std::map<int, SDL2Window*> windows;
};
