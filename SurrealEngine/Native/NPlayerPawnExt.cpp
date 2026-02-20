
#include "Precomp.h"
#include "Package/PackageManager.h"
#include "NPlayerPawnExt.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NPlayerPawnExt::RegisterFunctions()
{
	RegisterVMNativeFunc_0("PlayerPawnExt", "InitRootWindow", &NPlayerPawnExt::InitRootWindow, 1052);
	RegisterVMNativeFunc_1("PlayerPawnExt", "PostRenderWindows", &NPlayerPawnExt::PostRenderWindows, 1051);
	RegisterVMNativeFunc_1("PlayerPawnExt", "PreRenderWindows", &NPlayerPawnExt::PreRenderWindows, 1050);
}

void NPlayerPawnExt::InitRootWindow(UObject* Self)
{
    NPlayerPawnExt::ConstructRootWindow(Self);
}

void NPlayerPawnExt::ConstructRootWindow(UObject* Self)
{
    auto dxIni = engine->packages->GetIniFile("System");
    NameString dxRootClassName = dxIni->GetValue("Engine.Engine", "Root", "");
    UClass* cls = engine->packages->FindClass(dxRootClassName);
    if (cls)
    {
        engine->dxRootWindow = UObject::Cast<URootWindow>(engine->packages->GetTransientPackage()->NewObject("dxRootWindow", cls, ObjectFlags::Transient));
    }
}

void NPlayerPawnExt::PostRenderWindows(UObject* Self, UObject* Canvas)
{
	LogUnimplemented("PlayerPawnExt.PostRenderWindows");
}

void NPlayerPawnExt::PreRenderWindows(UObject* Self, UObject* Canvas)
{
	LogUnimplemented("PlayerPawnExt.PreRenderWindows");
}
