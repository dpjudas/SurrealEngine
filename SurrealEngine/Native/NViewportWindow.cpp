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
	Exception::Throw("NViewportWindow::ClearZBuffer not implemented");
}

void NViewportWindow::EnableViewport(UObject* Self, BitfieldBool* bEnable)
{
	Exception::Throw("NViewportWindow::EnableViewport not implemented");
}

void NViewportWindow::SetDefaultTexture(UObject* Self, UObject** NewTexture, Color* NewColor)
{
	Exception::Throw("NViewportWindow::SetDefaultTexture not implemented");
}

void NViewportWindow::SetFOVAngle(UObject* Self, float* newAngle)
{
	Exception::Throw("NViewportWindow::SetFOVAngle not implemented");
}

void NViewportWindow::SetRelativeLocation(UObject* Self, vec3* relLoc)
{
	Exception::Throw("NViewportWindow::SetRelativeLocation not implemented");
}

void NViewportWindow::SetRelativeRotation(UObject* Self, Rotator* relRot)
{
	Exception::Throw("NViewportWindow::SetRelativeRotation not implemented");
}

void NViewportWindow::SetRotation(UObject* Self, Rotator* NewRotation)
{
	Exception::Throw("NViewportWindow::SetRotation not implemented");
}

void NViewportWindow::SetViewportActor(UObject* Self, UObject** newOriginActor, BitfieldBool* bEyeLevel, BitfieldBool* bEnable)
{
	Exception::Throw("NViewportWindow::SetViewportActor not implemented");
}

void NViewportWindow::SetViewportLocation(UObject* Self, const vec3& NewLocation, BitfieldBool* bEnable)
{
	Exception::Throw("NViewportWindow::SetViewportLocation not implemented");
}

void NViewportWindow::SetWatchActor(UObject* Self, UObject** newWatchActor, BitfieldBool* bEyeLevel)
{
	Exception::Throw("NViewportWindow::SetWatchActor not implemented");
}

void NViewportWindow::ShowViewportActor(UObject* Self, BitfieldBool* bShow)
{
	Exception::Throw("NViewportWindow::ShowViewportActor not implemented");
}

void NViewportWindow::ShowWeapons(UObject* Self, BitfieldBool* bShow)
{
	Exception::Throw("NViewportWindow::ShowWeapons not implemented");
}
