
#include "Precomp.h"
#include "NPlayerPawnExt.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NPlayerPawnExt::RegisterFunctions()
{
	RegisterVMNativeFunc_0("PlayerPawnExt", "InitRootWindow", &NPlayerPawnExt::InitRootWindow, 1052);
	RegisterVMNativeFunc_1("PlayerPawnExt", "PostRenderWindows", &NPlayerPawnExt::PostRenderWindows, 1051);
	RegisterVMNativeFunc_1("PlayerPawnExt", "PreRenderWindows", &NPlayerPawnExt::PreRenderWindows, 1050);
}

void NPlayerPawnExt::InitRootWindow(UObject* Self)
{
	engine->LogUnimplemented("PlayerPawnExt.InitRootWindow not implemented");
}

void NPlayerPawnExt::PostRenderWindows(UObject* Self, UObject* Canvas)
{
	engine->LogUnimplemented("PlayerPawnExt.PostRenderWindows not implemented");
}

void NPlayerPawnExt::PreRenderWindows(UObject* Self, UObject* Canvas)
{
	engine->LogUnimplemented("PlayerPawnExt.PreRenderWindows not implemented");
}
