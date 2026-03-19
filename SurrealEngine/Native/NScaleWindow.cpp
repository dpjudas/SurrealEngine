#include "Precomp.h"
#include "NScaleWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
#include "Engine.h"

void NScaleWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_0("ScaleWindow", "ClearAllEnumerations", &NScaleWindow::ClearAllEnumerations, 1649);
	RegisterVMNativeFunc_1("ScaleWindow", "EnableStretchedScale", &NScaleWindow::EnableStretchedScale, 1625);
	RegisterVMNativeFunc_1("ScaleWindow", "GetNumTicks", &NScaleWindow::GetNumTicks, 1637);
	RegisterVMNativeFunc_1("ScaleWindow", "GetThumbSpan", &NScaleWindow::GetThumbSpan, 1639);
	RegisterVMNativeFunc_1("ScaleWindow", "GetTickPosition", &NScaleWindow::GetTickPosition, 1641);
	RegisterVMNativeFunc_1("ScaleWindow", "GetValue", &NScaleWindow::GetValue, 1644);
	RegisterVMNativeFunc_1("ScaleWindow", "GetValueString", &NScaleWindow::GetValueString, 1647);
	RegisterVMNativeFunc_2("ScaleWindow", "GetValues", &NScaleWindow::GetValues, 1645);
	RegisterVMNativeFunc_1("ScaleWindow", "MoveThumb", &NScaleWindow::MoveThumb, 1650);
	RegisterVMNativeFunc_3("ScaleWindow", "PlayScaleSound", &NScaleWindow::PlayScaleSound, 1653);
	RegisterVMNativeFunc_1("ScaleWindow", "SetBorderPattern", &NScaleWindow::SetBorderPattern, 1626);
	RegisterVMNativeFunc_2("ScaleWindow", "SetEnumeration", &NScaleWindow::SetEnumeration, 1648);
	RegisterVMNativeFunc_1("ScaleWindow", "SetNumTicks", &NScaleWindow::SetNumTicks, 1636);
	RegisterVMNativeFunc_2("ScaleWindow", "SetScaleBorder", &NScaleWindow::SetScaleBorder, 1627);
	RegisterVMNativeFunc_1("ScaleWindow", "SetScaleColor", &NScaleWindow::SetScaleColor, 1632);
	RegisterVMNativeFunc_2("ScaleWindow", "SetScaleMargins", &NScaleWindow::SetScaleMargins, 1635);
	RegisterVMNativeFunc_1("ScaleWindow", "SetScaleOrientation", &NScaleWindow::SetScaleOrientation, 1620);
	RegisterVMNativeFunc_3("ScaleWindow", "SetScaleSounds", &NScaleWindow::SetScaleSounds, 1652);
	RegisterVMNativeFunc_1("ScaleWindow", "SetScaleStyle", &NScaleWindow::SetScaleStyle, 1629);
	RegisterVMNativeFunc_5("ScaleWindow", "SetScaleTexture", &NScaleWindow::SetScaleTexture, 1621);
	RegisterVMNativeFunc_2("ScaleWindow", "SetThumbBorder", &NScaleWindow::SetThumbBorder, 1628);
	RegisterVMNativeFunc_6("ScaleWindow", "SetThumbCaps", &NScaleWindow::SetThumbCaps, 1624);
	RegisterVMNativeFunc_1("ScaleWindow", "SetThumbColor", &NScaleWindow::SetThumbColor, 1633);
	RegisterVMNativeFunc_1("ScaleWindow", "SetThumbSpan", &NScaleWindow::SetThumbSpan, 1638);
	RegisterVMNativeFunc_1("ScaleWindow", "SetThumbStep", &NScaleWindow::SetThumbStep, 1651);
	RegisterVMNativeFunc_1("ScaleWindow", "SetThumbStyle", &NScaleWindow::SetThumbStyle, 1630);
	RegisterVMNativeFunc_3("ScaleWindow", "SetThumbTexture", &NScaleWindow::SetThumbTexture, 1622);
	RegisterVMNativeFunc_1("ScaleWindow", "SetTickColor", &NScaleWindow::SetTickColor, 1634);
	RegisterVMNativeFunc_1("ScaleWindow", "SetTickPosition", &NScaleWindow::SetTickPosition, 1640);
	RegisterVMNativeFunc_1("ScaleWindow", "SetTickStyle", &NScaleWindow::SetTickStyle, 1631);
	RegisterVMNativeFunc_4("ScaleWindow", "SetTickTexture", &NScaleWindow::SetTickTexture, 1623);
	RegisterVMNativeFunc_1("ScaleWindow", "SetValue", &NScaleWindow::SetValue, 1643);
	RegisterVMNativeFunc_1("ScaleWindow", "SetValueFormat", &NScaleWindow::SetValueFormat, 1646);
	RegisterVMNativeFunc_2("ScaleWindow", "SetValueRange", &NScaleWindow::SetValueRange, 1642);
}

void NScaleWindow::ClearAllEnumerations(UObject* Self)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->ClearAllEnumerations();
}

void NScaleWindow::EnableStretchedScale(UObject* Self, std::optional<bool> bNewStretch)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->EnableStretchedScale(bNewStretch);
}

void NScaleWindow::GetNumTicks(UObject* Self, int& ReturnValue)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	ReturnValue = scalewindow->GetNumTicks();
}

void NScaleWindow::GetThumbSpan(UObject* Self, int& ReturnValue)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	ReturnValue = scalewindow->GetThumbSpan();
}

void NScaleWindow::GetTickPosition(UObject* Self, int& ReturnValue)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	ReturnValue = scalewindow->GetTickPosition();
}

void NScaleWindow::GetValue(UObject* Self, float& ReturnValue)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	ReturnValue = scalewindow->GetValue();
}

void NScaleWindow::GetValueString(UObject* Self, std::string& ReturnValue)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	ReturnValue = scalewindow->GetValueString();
}

void NScaleWindow::GetValues(UObject* Self, float& fromValue, float& toValue)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->GetValues(fromValue, toValue);
}

void NScaleWindow::MoveThumb(UObject* Self, uint8_t MoveThumb)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->MoveThumb(MoveThumb);
}

void NScaleWindow::PlayScaleSound(UObject* Self, UObject* newsound, std::optional<float> Volume, std::optional<float> Pitch)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->PlayScaleSound(newsound, Volume, Pitch);
}

void NScaleWindow::SetBorderPattern(UObject* Self, UObject* NewTexture)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetBorderPattern(NewTexture);
}

void NScaleWindow::SetEnumeration(UObject* Self, int tickPos, const std::string& newStr)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetEnumeration(tickPos, newStr);
}

void NScaleWindow::SetNumTicks(UObject* Self, int newNumTicks)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetNumTicks(newNumTicks);
}

void NScaleWindow::SetScaleBorder(UObject* Self, std::optional<float> newBorderSize, std::optional<Color> NewColor)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetScaleBorder(newBorderSize, NewColor);
}

void NScaleWindow::SetScaleColor(UObject* Self, const Color& NewColor)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetScaleColor(NewColor);
}

void NScaleWindow::SetScaleMargins(UObject* Self, std::optional<float> marginWidth, std::optional<float> marginHeight)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetScaleMargins(marginWidth, marginHeight);
}

void NScaleWindow::SetScaleOrientation(UObject* Self, uint8_t newOrientation)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetScaleOrientation(newOrientation);
}

void NScaleWindow::SetScaleSounds(UObject* Self, std::optional<UObject*> setSound, std::optional<UObject*> clickSound, std::optional<UObject*> dragSound)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetScaleSounds(setSound, clickSound, dragSound);
}

void NScaleWindow::SetScaleStyle(UObject* Self, uint8_t NewStyle)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetScaleStyle(NewStyle);
}

void NScaleWindow::SetScaleTexture(UObject* Self, UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight, std::optional<float> newStart, std::optional<float> newEnd)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetScaleTexture(NewTexture, newWidth, NewHeight, newStart, newEnd);
}

void NScaleWindow::SetThumbBorder(UObject* Self, std::optional<float> newBorderSize, std::optional<Color> NewColor)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetThumbBorder(newBorderSize, NewColor);
}

void NScaleWindow::SetThumbCaps(UObject* Self, UObject* preCap, UObject* postCap, std::optional<float> preCapWidth, std::optional<float> preCapHeight, std::optional<float> postCapWidth, std::optional<float> postCapHeight)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetThumbCaps(preCap, postCap, preCapWidth, preCapHeight, postCapWidth, postCapHeight);
}

void NScaleWindow::SetThumbColor(UObject* Self, const Color& NewColor)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetThumbColor(NewColor);
}

void NScaleWindow::SetThumbSpan(UObject* Self, std::optional<int> newRange)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetThumbSpan(newRange);
}

void NScaleWindow::SetThumbStep(UObject* Self, int NewStep)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetThumbStep(NewStep);
}

void NScaleWindow::SetThumbStyle(UObject* Self, uint8_t NewStyle)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetThumbStyle(NewStyle);
}

void NScaleWindow::SetThumbTexture(UObject* Self, UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetThumbTexture(NewTexture, newWidth, NewHeight);
}

void NScaleWindow::SetTickColor(UObject* Self, const Color& NewColor)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetTickColor(NewColor);
}

void NScaleWindow::SetTickPosition(UObject* Self, int newPosition)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetTickPosition(newPosition);
}

void NScaleWindow::SetTickStyle(UObject* Self, uint8_t NewStyle)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetTickStyle(NewStyle);
}

void NScaleWindow::SetTickTexture(UObject* Self, UObject* tickTexture, std::optional<bool> bDrawEndTicks, std::optional<float> newWidth, std::optional<float> NewHeight)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetTickTexture(tickTexture, bDrawEndTicks, newWidth, NewHeight);
}

void NScaleWindow::SetValue(UObject* Self, float NewValue)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetValue(NewValue);
}

void NScaleWindow::SetValueFormat(UObject* Self, const std::string& newFmt)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetValueFormat(newFmt);
}

void NScaleWindow::SetValueRange(UObject* Self, float newFrom, float newTo)
{
	UScaleWindow* scalewindow = UObject::Cast<UScaleWindow>(Self);
	scalewindow->SetValueRange(newFrom, newTo);
}
