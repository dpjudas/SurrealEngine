#pragma once

#include "Math/vec.h"

class Engine;
class RenderDevice;

class Window
{
public:
	static std::unique_ptr<Window> Create(Engine* engine);

	virtual ~Window() = default;
	virtual void OpenWindow(int width, int height, bool fullscreen) = 0;
	virtual void CloseWindow() = 0;
	virtual void* GetWindow() = 0;
	virtual void Tick() = 0;
	virtual RenderDevice* GetRenderDevice() = 0;

	int SizeX = 0;
	int SizeY = 0;

	float Brightness = 0.808333f;
};
