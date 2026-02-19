#include "Precomp.h"
#include "NScaleWindow.h"
#include "VM/NativeFunc.h"
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
	LogUnimplemented("NScaleWindow::ClearAllEnumerations not implemented");
}

void NScaleWindow::EnableStretchedScale(UObject* Self, BitfieldBool* bNewStretch)
{
	LogUnimplemented("NScaleWindow::EnableStretchedScale not implemented");
}

void NScaleWindow::GetNumTicks(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NScaleWindow::GetNumTicks not implemented");
	ReturnValue = 0;
}

void NScaleWindow::GetThumbSpan(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NScaleWindow::GetThumbSpan not implemented");
	ReturnValue = 0;
}

void NScaleWindow::GetTickPosition(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("NScaleWindow::GetTickPosition not implemented");
	ReturnValue = 0;
}

void NScaleWindow::GetValue(UObject* Self, float& ReturnValue)
{
	LogUnimplemented("NScaleWindow::GetValue not implemented");
	ReturnValue = 0.0f;
}

void NScaleWindow::GetValueString(UObject* Self, std::string& ReturnValue)
{
	LogUnimplemented("NScaleWindow::GetValueString not implemented");
	ReturnValue = "";
}

void NScaleWindow::GetValues(UObject* Self, float& fromValue, float& toValue)
{
	LogUnimplemented("NScaleWindow::GetValues not implemented");
}

void NScaleWindow::MoveThumb(UObject* Self, uint8_t MoveThumb)
{
	LogUnimplemented("NScaleWindow::MoveThumb not implemented");
}

void NScaleWindow::PlayScaleSound(UObject* Self, UObject* newsound, float* Volume, float* Pitch)
{
	LogUnimplemented("NScaleWindow::PlayScaleSound not implemented");
}

void NScaleWindow::SetBorderPattern(UObject* Self, UObject* NewTexture)
{
	LogUnimplemented("NScaleWindow::SetBorderPattern not implemented");
}

void NScaleWindow::SetEnumeration(UObject* Self, int tickPos, const std::string& newStr)
{
	LogUnimplemented("NScaleWindow::SetEnumeration not implemented");
}

void NScaleWindow::SetNumTicks(UObject* Self, int newNumTicks)
{
	LogUnimplemented("NScaleWindow::SetNumTicks not implemented");
}

void NScaleWindow::SetScaleBorder(UObject* Self, float* newBorderSize, Color* NewColor)
{
	LogUnimplemented("NScaleWindow::SetScaleBorder not implemented");
}

void NScaleWindow::SetScaleColor(UObject* Self, const Color& NewColor)
{
	LogUnimplemented("NScaleWindow::SetScaleColor not implemented");
}

void NScaleWindow::SetScaleMargins(UObject* Self, float* marginWidth, float* marginHeight)
{
	LogUnimplemented("NScaleWindow::SetScaleMargins not implemented");
}

void NScaleWindow::SetScaleOrientation(UObject* Self, uint8_t newOrientation)
{
	LogUnimplemented("NScaleWindow::SetScaleOrientation not implemented");
}

void NScaleWindow::SetScaleSounds(UObject* Self, UObject** setSound, UObject** clickSound, UObject** dragSound)
{
	LogUnimplemented("NScaleWindow::SetScaleSounds not implemented");
}

void NScaleWindow::SetScaleStyle(UObject* Self, uint8_t NewStyle)
{
	LogUnimplemented("NScaleWindow::SetScaleStyle not implemented");
}

void NScaleWindow::SetScaleTexture(UObject* Self, UObject* NewTexture, float* newWidth, float* NewHeight, float* newStart, float* newEnd)
{
	LogUnimplemented("NScaleWindow::SetScaleTexture not implemented");
}

void NScaleWindow::SetThumbBorder(UObject* Self, float* newBorderSize, Color* NewColor)
{
	LogUnimplemented("NScaleWindow::SetThumbBorder not implemented");
}

void NScaleWindow::SetThumbCaps(UObject* Self, UObject* preCap, UObject* postCap, float* preCapWidth, float* preCapHeight, float* postCapWidth, float* postCapHeight)
{
	LogUnimplemented("NScaleWindow::SetThumbCaps not implemented");
}

void NScaleWindow::SetThumbColor(UObject* Self, const Color& NewColor)
{
	LogUnimplemented("NScaleWindow::SetThumbColor not implemented");
}

void NScaleWindow::SetThumbSpan(UObject* Self, int* newRange)
{
	LogUnimplemented("NScaleWindow::SetThumbSpan not implemented");
}

void NScaleWindow::SetThumbStep(UObject* Self, int NewStep)
{
	LogUnimplemented("NScaleWindow::SetThumbStep not implemented");
}

void NScaleWindow::SetThumbStyle(UObject* Self, uint8_t NewStyle)
{
	LogUnimplemented("NScaleWindow::SetThumbStyle not implemented");
}

void NScaleWindow::SetThumbTexture(UObject* Self, UObject* NewTexture, float* newWidth, float* NewHeight)
{
	LogUnimplemented("NScaleWindow::SetThumbTexture not implemented");
}

void NScaleWindow::SetTickColor(UObject* Self, const Color& NewColor)
{
	LogUnimplemented("NScaleWindow::SetTickColor not implemented");
}

void NScaleWindow::SetTickPosition(UObject* Self, int newPosition)
{
	LogUnimplemented("NScaleWindow::SetTickPosition not implemented");
}

void NScaleWindow::SetTickStyle(UObject* Self, uint8_t NewStyle)
{
	LogUnimplemented("NScaleWindow::SetTickStyle not implemented");
}

void NScaleWindow::SetTickTexture(UObject* Self, UObject* tickTexture, BitfieldBool* bDrawEndTicks, float* newWidth, float* NewHeight)
{
	LogUnimplemented("NScaleWindow::SetTickTexture not implemented");
}

void NScaleWindow::SetValue(UObject* Self, float NewValue)
{
	LogUnimplemented("NScaleWindow::SetValue not implemented");
}

void NScaleWindow::SetValueFormat(UObject* Self, const std::string& newFmt)
{
	LogUnimplemented("NScaleWindow::SetValueFormat not implemented");
}

void NScaleWindow::SetValueRange(UObject* Self, float newFrom, float newTo)
{
	LogUnimplemented("NScaleWindow::SetValueRange not implemented");
}
