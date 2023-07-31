
#include "Precomp.h"
#include "Window.h"
#ifdef WIN32
#include "Win32/Win32Window.h"
#else
#include "X11/X11Window.h"
#endif

// TODO: base this off of ini setting, not dependent on OS macro

#ifdef WIN32

std::unique_ptr<DisplayWindow> DisplayWindow::Create(Engine* engine)
{
	return std::make_unique<Win32Window>(engine);
}

#else

std::unique_ptr<DisplayWindow> DisplayWindow::Create(Engine* engine)
{
	return std::make_unique<X11Window>(engine);
}

#endif
