#include "Precomp.h"
#include "NViewportWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
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
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->ClearZBuffer(bClear);
}

void NViewportWindow::EnableViewport(UObject* Self, BitfieldBool* bEnable)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->EnableViewport(bEnable);
}

void NViewportWindow::SetDefaultTexture(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->SetDefaultTexture(NewTexture, NewColor);
}

void NViewportWindow::SetFOVAngle(UObject* Self, float* newAngle)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->SetFOVAngle(newAngle);
}

void NViewportWindow::SetRelativeLocation(UObject* Self, vec3* relLoc)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->SetRelativeLocation(relLoc);
}

void NViewportWindow::SetRelativeRotation(UObject* Self, Rotator* relRot)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->SetRelativeRotation(relRot);
}

void NViewportWindow::SetRotation(UObject* Self, Rotator* NewRotation)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->SetRotation(NewRotation);
}

void NViewportWindow::SetViewportActor(UObject* Self, UObject** newOriginActor, BitfieldBool* bEyeLevel, BitfieldBool* bEnable)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->SetViewportActor(newOriginActor, bEyeLevel, bEnable);
}

void NViewportWindow::SetViewportLocation(UObject* Self, const vec3& NewLocation, BitfieldBool* bEnable)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->SetViewportLocation(NewLocation, bEnable);
}

void NViewportWindow::SetWatchActor(UObject* Self, UObject** newWatchActor, BitfieldBool* bEyeLevel)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->SetWatchActor(newWatchActor, bEyeLevel);
}

void NViewportWindow::ShowViewportActor(UObject* Self, BitfieldBool* bShow)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->ShowViewportActor(bShow);
}

void NViewportWindow::ShowWeapons(UObject* Self, BitfieldBool* bShow)
{
	UViewportWindow* selfViewport = UObject::Cast<UViewportWindow>(Self);
	selfViewport->ShowWeapons(bShow);
}
