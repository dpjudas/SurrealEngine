#include "Precomp.h"
#include "NViewportWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NViewportWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ViewportWindow", "ClearZBuffer", &NViewportWindow::ClearZBuffer, 1956);
	RegisterVMNativeFunc_1("ViewportWindow", "EnableViewport", &NViewportWindow::EnableViewport, 1945);
	RegisterVMNativeFunc_2("ViewportWindow", "SetDefaultTexture", &NViewportWindow::SetDefaultTexture, 1955);
	RegisterVMNativeFunc_1("ViewportWindow", "SetFOVAngle", &NViewportWindow::SetFOVAngle, 1946);
	RegisterVMNativeFunc_1("ViewportWindow", "SetRelativeLocation", &NViewportWindow::SetRelativeLocation, 1950);
	RegisterVMNativeFunc_1("ViewportWindow", "SetRelativeRotation", &NViewportWindow::SetRelativeRotation, 1951);
	RegisterVMNativeFunc_1("ViewportWindow", "SetRotation", &NViewportWindow::SetRotation, 1943);
	RegisterVMNativeFunc_3("ViewportWindow", "SetViewportActor", &NViewportWindow::SetViewportActor, 1940);
	RegisterVMNativeFunc_2("ViewportWindow", "SetViewportLocation", &NViewportWindow::SetViewportLocation, 1941);
	RegisterVMNativeFunc_2("ViewportWindow", "SetWatchActor", &NViewportWindow::SetWatchActor, 1942);
	RegisterVMNativeFunc_1("ViewportWindow", "ShowViewportActor", &NViewportWindow::ShowViewportActor, 1947);
	RegisterVMNativeFunc_1("ViewportWindow", "ShowWeapons", &NViewportWindow::ShowWeapons, 1948);
}

void NViewportWindow::ClearZBuffer(UObject* Self, BitfieldBool* bClear)
{
	LogUnimplemented("ViewportWindow.ClearZBuffer");
}

void NViewportWindow::EnableViewport(UObject* Self, BitfieldBool* bEnable)
{
	LogUnimplemented("ViewportWindow.EnableViewport");
}

void NViewportWindow::SetDefaultTexture(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	LogUnimplemented("ViewportWindow.SetDefaultTexture");
}

void NViewportWindow::SetFOVAngle(UObject* Self, float* newAngle)
{
	LogUnimplemented("ViewportWindow.SetFOVAngle");
}

void NViewportWindow::SetRelativeLocation(UObject* Self, vec3* relLoc)
{
	LogUnimplemented("ViewportWindow.SetRelativeLocation");
}

void NViewportWindow::SetRelativeRotation(UObject* Self, Rotator* relRot)
{
	LogUnimplemented("ViewportWindow.SetRelativeRotation");
}

void NViewportWindow::SetRotation(UObject* Self, Rotator* NewRotation)
{
	LogUnimplemented("ViewportWindow.SetRotation");
}

void NViewportWindow::SetViewportActor(UObject* Self, UObject** newOriginActor, BitfieldBool* bEyeLevel, BitfieldBool* bEnable)
{
	LogUnimplemented("ViewportWindow.SetViewportActor");
}

void NViewportWindow::SetViewportLocation(UObject* Self, const vec3& NewLocation, BitfieldBool* bEnable)
{
	LogUnimplemented("ViewportWindow.SetViewportLocation");
}

void NViewportWindow::SetWatchActor(UObject* Self, UObject** newWatchActor, BitfieldBool* bEyeLevel)
{
	LogUnimplemented("ViewportWindow.SetWatchActor");
}

void NViewportWindow::ShowViewportActor(UObject* Self, BitfieldBool* bShow)
{
	LogUnimplemented("ViewportWindow.ShowViewportActor");
}

void NViewportWindow::ShowWeapons(UObject* Self, BitfieldBool* bShow)
{
	LogUnimplemented("ViewportWindow.ShowWeapons");
}
