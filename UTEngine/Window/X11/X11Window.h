/*
**  Copyright (c) Mark Page, Chu Chin Kuan, Magnus Norddahl, Harry Storbacka
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
*/

#pragma once

#include "Window/Window.h"
#include "X11Atoms.h"
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>

class VulkanDevice;

class X11Window : public DisplayWindow
{
public:
	X11Window(Engine* engine);
	~X11Window();
	void OpenWindow(int width, int height, bool fullscreen) override;
	void CloseWindow() override;
	void* GetDisplay() override;
	void* GetWindow() override;
	void Tick() override;
	RenderDevice* GetRenderDevice() override { return RendDevice.get(); }

	bool HasFocus() const;
	bool IsMinimized() const;
	bool IsMaximized() const;
	bool IsVisible() const;
	void ShowSystemCursor();
	void HideSystemCursor();
	void SetCursor(StandardCursor type);

	void MapWindow();
	void UnmapWindow();

	void OnKeyboardInput(XKeyEvent &event);
	void OnMouseInput(XButtonEvent &event);
	void OnMouseMove(XMotionEvent &event);

	EInputKey KeySymToInputKey(KeySym keysym);

	Engine* engine = nullptr;
	std::unique_ptr<RenderDevice> RendDevice;

	Display* display = nullptr;
	int screen = 0;
	X11Atoms atoms;
	Window window = {};
	Colormap colormap = {};
	Pixmap cursor_bitmap = {};
	Cursor system_cursor = {};
	Cursor hidden_cursor = {};
	XSizeHints* size_hints = nullptr;
	bool is_window_mapped = false;
	bool is_fullscreen = false;
	EInputKey last_press_id = IK_None;
	Time time_at_last_press = 0;

	int MouseX = -1;
	int MouseY = -1;
	int MouseMoveX = 0;
	int MouseMoveY = 0;
	std::map<KeySym, int> repeat_count;
	bool ctrl_down = false;
	bool shift_down = false;
	bool alt_down = false;

	double dpiscale = 1.0;
};
