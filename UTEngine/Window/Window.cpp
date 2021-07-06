
#include "Precomp.h"
#include "Window.h"
#include "Win32/Win32Window.h"

std::unique_ptr<Window> Window::Create(Engine* engine)
{
	return std::make_unique<Win32Window>(engine);
}
