
#include "Precomp.h"
#include "Viewport.h"
#include "Win32/Win32Viewport.h"

std::unique_ptr<Viewport> Viewport::Create(Engine* engine)
{
	return std::make_unique<Win32Viewport>(engine);
}
