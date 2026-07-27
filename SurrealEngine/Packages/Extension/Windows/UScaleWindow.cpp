
#include "Precomp.h"
#include "UScaleWindow.h"
#include "Engine.h"

void UScaleWindow::ClearAllEnumerations()
{
	// UNUSED from scripts.
	LogUnimplemented("ScaleWindow.ClearAllEnumerations");
}

void UScaleWindow::EnableStretchedScale(std::optional<bool> bNewStretch)
{
	// UNUSED from scripts.
	LogUnimplemented("ScaleWindow.EnableStretchedScale");
}

int UScaleWindow::GetNumTicks()
{
	LogUnimplemented("ScaleWindow.GetNumTicks");
	return 0;
}

int UScaleWindow::GetThumbSpan()
{
	// UNUSED from scripts.
	LogUnimplemented("ScaleWindow.GetThumbSpan");
	return 0;
}

int UScaleWindow::GetTickPosition()
{
	LogUnimplemented("ScaleWindow.GetTickPosition");
	return 0;
}

float UScaleWindow::GetValue()
{
	LogUnimplemented("ScaleWindow.GetValue");
	return 0.0f;
}

std::string UScaleWindow::GetValueString()
{
	LogUnimplemented("ScaleWindow.GetValueString");
	return "";
}

void UScaleWindow::GetValues(float& fromValue, float& toValue)
{
	// UNUSED from scripts.
	LogUnimplemented("ScaleWindow.GetValues");
}

void UScaleWindow::MoveThumb(uint8_t MoveThumb)
{
	LogUnimplemented("ScaleWindow.MoveThumb");
}

void UScaleWindow::PlayScaleSound(UObject* newsound, std::optional<float> Volume, std::optional<float> Pitch)
{
	LogUnimplemented("ScaleWindow.PlayScaleSound");
}

void UScaleWindow::SetBorderPattern(UObject* NewTexture)
{
	// UNUSED from scripts.
	LogUnimplemented("ScaleWindow.SetBorderPattern");
}

void UScaleWindow::SetEnumeration(int tickPos, const std::string& newStr)
{
	LogUnimplemented("ScaleWindow.SetEnumeration");
}

void UScaleWindow::SetNumTicks(int newNumTicks)
{
	LogUnimplemented("ScaleWindow.SetNumTicks");
}

void UScaleWindow::SetScaleBorder(std::optional<float> newBorderSize, std::optional<Color> NewColor)
{
	LogUnimplemented("ScaleWindow.SetScaleBorder");
}

void UScaleWindow::SetScaleColor(const Color& NewColor)
{
	LogUnimplemented("ScaleWindow.SetScaleColor");
}

void UScaleWindow::SetScaleMargins(std::optional<float> marginWidth, std::optional<float> marginHeight)
{
	LogUnimplemented("ScaleWindow.SetScaleMargins");
}

void UScaleWindow::SetScaleOrientation(uint8_t newOrientation)
{
	LogUnimplemented("ScaleWindow.SetScaleOrientation");
}

void UScaleWindow::SetScaleSounds(std::optional<UObject*> setSound, std::optional<UObject*> clickSound, std::optional<UObject*> dragSound)
{
	LogUnimplemented("ScaleWindow.SetScaleSounds");
}

void UScaleWindow::SetScaleStyle(uint8_t NewStyle)
{
	LogUnimplemented("ScaleWindow.SetScaleStyle");
}

void UScaleWindow::SetScaleTexture(UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight, std::optional<float> newStart, std::optional<float> newEnd)
{
	LogUnimplemented("ScaleWindow.SetScaleTexture");
}

void UScaleWindow::SetThumbBorder(std::optional<float> newBorderSize, std::optional<Color> NewColor)
{
	LogUnimplemented("ScaleWindow.SetThumbBorder");
}

void UScaleWindow::SetThumbCaps(UObject* preCap, UObject* postCap, std::optional<float> preCapWidth, std::optional<float> preCapHeight, std::optional<float> postCapWidth, std::optional<float> postCapHeight)
{
	LogUnimplemented("ScaleWindow.SetThumbCaps");
}

void UScaleWindow::SetThumbColor(const Color& NewColor)
{
	LogUnimplemented("ScaleWindow.SetThumbColor");
}

void UScaleWindow::SetThumbSpan(std::optional<int> newRange)
{
	LogUnimplemented("ScaleWindow.SetThumbSpan");
}

void UScaleWindow::SetThumbStep(int NewStep)
{
	LogUnimplemented("ScaleWindow.SetThumbStep");
}

void UScaleWindow::SetThumbStyle(uint8_t NewStyle)
{
	LogUnimplemented("ScaleWindow.SetThumbStyle");
}

void UScaleWindow::SetThumbTexture(UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight)
{
	LogUnimplemented("ScaleWindow.SetThumbTexture");
}

void UScaleWindow::SetTickColor(const Color& NewColor)
{
	LogUnimplemented("ScaleWindow.SetTickColor");
}

void UScaleWindow::SetTickPosition(int newPosition)
{
	LogUnimplemented("ScaleWindow.SetTickPosition");
}

void UScaleWindow::SetTickStyle(uint8_t NewStyle)
{
	// UNUSED from scripts.
	LogUnimplemented("ScaleWindow.SetTickStyle");
}

void UScaleWindow::SetTickTexture(UObject* tickTexture, std::optional<bool> bDrawEndTicks, std::optional<float> newWidth, std::optional<float> NewHeight)
{
	// UNUSED from scripts.
	LogUnimplemented("ScaleWindow.SetTickTexture");
}

void UScaleWindow::SetValue(float NewValue)
{
	LogUnimplemented("ScaleWindow.SetValue");
}

void UScaleWindow::SetValueFormat(const std::string& newFmt)
{
	LogUnimplemented("ScaleWindow.SetValueFormat");
}

void UScaleWindow::SetValueRange(float newFrom, float newTo)
{
	LogUnimplemented("ScaleWindow.SetValueRange");
}
