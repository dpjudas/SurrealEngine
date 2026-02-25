
#include "Precomp.h"
#include "Package/PackageManager.h"
#include "NPlayerPawnExt.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "UObject/UClient.h"

void NPlayerPawnExt::RegisterFunctions()
{
	RegisterVMNativeFunc_0("PlayerPawnExt", "InitRootWindow", &NPlayerPawnExt::InitRootWindow, 1052);
	RegisterVMNativeFunc_1("PlayerPawnExt", "PostRenderWindows", &NPlayerPawnExt::PostRenderWindows, 1051);
	RegisterVMNativeFunc_1("PlayerPawnExt", "PreRenderWindows", &NPlayerPawnExt::PreRenderWindows, 1050);
}

void NPlayerPawnExt::InitRootWindow(UObject* Self)
{
	UPlayerPawnExt* SelfPawn = UObject::Cast<UPlayerPawnExt>(Self);
	SelfPawn->InitRootWindow();
}

void NPlayerPawnExt::PostRenderWindows(UObject* Self, UObject* Canvas)
{
	UPlayerPawnExt* SelfPawn = UObject::Cast<UPlayerPawnExt>(Self);
	SelfPawn->PostRenderWindows(UObject::Cast<UCanvas>(Canvas));
}

void NPlayerPawnExt::PreRenderWindows(UObject* Self, UObject* Canvas)
{
	UPlayerPawnExt* SelfPawn = UObject::Cast<UPlayerPawnExt>(Self);
	SelfPawn->PreRenderWindows(UObject::Cast<UCanvas>(Canvas));
}
