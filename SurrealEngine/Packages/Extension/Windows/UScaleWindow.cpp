
#include "Precomp.h"
#include "UScaleWindow.h"
#include "UGC.h"
#include "Engine.h"
#include "Packages/Engine/Resources/USound.h"
#include "Packages/Engine/Resources/Textures/UTexture.h"

void UScaleWindow::ClearAllEnumerations()
{
	ticks.clear();
}

void UScaleWindow::EnableStretchedScale(std::optional<bool> bNewStretch)
{
	bStretchScale() = bNewStretch.has_value() ? bNewStretch.value() : true;
}

void UScaleWindow::GetValues(float& outFromValue, float& outToValue)
{
	outFromValue = fromValue();
	outToValue = toValue();
}

void UScaleWindow::PlayScaleSound(UObject* newsound, std::optional<float> Volume, std::optional<float> Pitch)
{
	PlaySound(newsound, Volume, Pitch, {}, {});
}

void UScaleWindow::SetBorderPattern(UObject* NewTexture)
{
	borderPattern() = UObject::Cast<UTexture>(NewTexture);
}

void UScaleWindow::SetEnumeration(int tickPos, const std::string& newStr)
{
	if (tickPos < 0)
	{
		LogMessage("Negative tickPos value passed to ScaleWindow.SetEnumeration");
		return;
	}
	if (ticks.size() <= (size_t)tickPos)
		ticks.resize(tickPos + 1);
	ticks[tickPos] = newStr;
}

void UScaleWindow::SetNumTicks(int newNumTicks)
{
	ticks.resize(newNumTicks);
}

void UScaleWindow::SetScaleBorder(std::optional<float> newBorderSize, std::optional<Color> NewColor)
{
	if (newBorderSize.has_value())
		scaleBorderSize() = newBorderSize.value();
	if (NewColor.has_value())
		scaleBorderColor() = NewColor.value();
}

void UScaleWindow::SetScaleColor(const Color& NewColor)
{
	scaleColor() = NewColor;
}

void UScaleWindow::SetScaleMargins(std::optional<float> newMarginWidth, std::optional<float> newMarginHeight)
{
	if (newMarginWidth.has_value())
		marginWidth() = newMarginWidth.value();
	if (newMarginHeight.has_value())
		marginHeight() = newMarginHeight.value();
}

void UScaleWindow::SetScaleOrientation(uint8_t newOrientation)
{
	orientation() = newOrientation;
}

void UScaleWindow::SetScaleSounds(std::optional<UObject*> newSetSound, std::optional<UObject*> newClickSound, std::optional<UObject*> newDragSound)
{
	if (newSetSound.has_value())
		setSound() = UObject::Cast<USound>(newSetSound.value());
	if (newClickSound.has_value())
		clickSound() = UObject::Cast<USound>(newClickSound.value());
	if (newDragSound.has_value())
		dragSound() = UObject::Cast<USound>(newDragSound.value());
}

void UScaleWindow::SetScaleStyle(uint8_t NewStyle)
{
	scaleStyle() = NewStyle;
}

void UScaleWindow::SetScaleTexture(UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight, std::optional<float> newStart, std::optional<float> newEnd)
{
	scaleTexture() = UObject::Cast<UTexture>(NewTexture);
	if (newWidth.has_value())
		scaleWidth() = newWidth.value();
	if (NewHeight.has_value())
		scaleHeight() = NewHeight.value();
	if (newStart.has_value())
		startOffset() = newStart.value();
	if (newEnd.has_value())
		endOffset() = newEnd.value();
}

void UScaleWindow::SetThumbBorder(std::optional<float> newBorderSize, std::optional<Color> NewColor)
{
	if (newBorderSize.has_value())
		thumbBorderSize() = newBorderSize.value();
	if (NewColor.has_value())
		thumbBorderColor() = NewColor.value();
}

void UScaleWindow::SetThumbCaps(UObject* preCap, UObject* postCap, std::optional<float> newPreCapWidth, std::optional<float> newPreCapHeight, std::optional<float> newPostCapWidth, std::optional<float> newPostCapHeight)
{
	preCapTexture() = UObject::Cast<UTexture>(preCap);
	postCapTexture() = UObject::Cast<UTexture>(postCap);
	if (newPreCapWidth.has_value())
		preCapWidth() = newPreCapWidth.value();
	if (newPreCapHeight.has_value())
		preCapHeight() = newPreCapHeight.value();
	if (newPostCapWidth.has_value())
		postCapWidth() = newPostCapWidth.value();
	if (newPostCapHeight.has_value())
		postCapHeight() = newPostCapHeight.value();
}

void UScaleWindow::SetThumbColor(const Color& NewColor)
{
	thumbColor() = NewColor;
}

void UScaleWindow::SetThumbSpan(std::optional<int> newRange)
{
	// Script calls this setting ThumbSpan to zero when it wants it to be a slider it seems
	if (newRange.has_value())
	{
		spanRange() = newRange.value();
		bSpanThumb() = spanRange() > 0;
	}
}

void UScaleWindow::SetThumbStep(int NewStep)
{
	thumbStep() = NewStep;
}

void UScaleWindow::SetThumbStyle(uint8_t NewStyle)
{
	thumbStyle() = NewStyle;
}

void UScaleWindow::SetThumbTexture(UObject* NewTexture, std::optional<float> newWidth, std::optional<float> NewHeight)
{
	thumbTexture() = UObject::Cast<UTexture>(NewTexture);
	if (newWidth.has_value())
		ThumbWidth() = newWidth.value();
	if (NewHeight.has_value())
		ThumbHeight() = NewHeight.value();
}

void UScaleWindow::SetTickColor(const Color& NewColor)
{
	tickColor() = NewColor;
}

void UScaleWindow::SetTickStyle(uint8_t NewStyle)
{
	tickStyle() = NewStyle;
}

void UScaleWindow::SetTickTexture(UObject* newTickTexture, std::optional<bool> newDrawEndTicks, std::optional<float> newWidth, std::optional<float> NewHeight)
{
	// Note: this is never called by script

	tickTexture() = UObject::Cast<UTexture>(newTickTexture);
	if (newDrawEndTicks.has_value())
		bDrawEndTicks() = newDrawEndTicks.value();
	if (newWidth.has_value())
		tickWidth() = newWidth.value();
	if (NewHeight.has_value())
		tickHeight() = NewHeight.value();
}

void UScaleWindow::SetValueFormat(const std::string& newFmt)
{
	valueFmt() = newFmt;
}

void UScaleWindow::SetValueRange(float newFrom, float newTo)
{
	fromValue() = newFrom;
	toValue() = newTo;
}

void UScaleWindow::SetTickPosition(int newPosition)
{
	float t = !ticks.empty() ? newPosition / (float)(ticks.size() - 1) : 0.0f;
	SetValue(mix(fromValue(), toValue(), t));
}

void UScaleWindow::SetValue(float NewValue)
{
	value = std::clamp(NewValue, fromValue(), toValue());
	DispatchScalePositionChanged(GetTickPosition(), GetValue(), true);
}

void UScaleWindow::MoveThumb(uint8_t MoveThumb)
{
	switch ((EMoveThumb)MoveThumb)
	{
	case EMoveThumb::Home:
	case EMoveThumb::PageUp:
		SetTickPosition(0);
		break;
	case EMoveThumb::End:
	case EMoveThumb::PageDown:
		SetTickPosition(std::max(GetNumTicks() - 1, 0));
		break;
	case EMoveThumb::StepUp:
	case EMoveThumb::Prev:
		SetTickPosition(std::max(GetTickPosition() - 1, 0));
		break;
	case EMoveThumb::StepDown:
	case EMoveThumb::Next:
		SetTickPosition(std::min(GetTickPosition() + 1, std::max(GetNumTicks() - 1, 0)));
		break;
	}
}

int UScaleWindow::GetThumbSpan()
{
	return spanRange();
}

int UScaleWindow::GetNumTicks()
{
	return (int)ticks.size();
}

int UScaleWindow::GetTickPosition()
{
	if (ticks.empty())
		return 0;
	float t = (value - fromValue()) / (toValue() - fromValue());
	return (int)std::round(t * (ticks.size() - 1.0f));
}

float UScaleWindow::GetValue()
{
	return value;
}

std::string UScaleWindow::GetValueString()
{
	int tickPosition = GetTickPosition();
	if ((size_t)tickPosition >= ticks.size())
		return "";
	return ticks[tickPosition];
}

void UScaleWindow::InitWindow()
{
	bSpanThumb() = true; // Default to a scrollbar
	scaleStyle() = (uint8_t)EDrawStyle::Masked;
	tickStyle() = (uint8_t)EDrawStyle::Masked;
	thumbStyle() = (uint8_t)EDrawStyle::Masked;
	UWindow::InitWindow();
}

void UScaleWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	UWindow::ParentRequestedPreferredSize(bWidthSpecified, preferredWidth, bHeightSpecified, preferredHeight);
}

void UScaleWindow::DrawWindow(UGC* gc)
{
	UWindow::DrawWindow(gc);

	if (auto tex = scaleTexture())
	{
		gc->SetTileColor(scaleColor());
		gc->SetStyle((EDrawStyle)scaleStyle());
		gc->DrawPattern(0.0f, 0.0f, Width(), Height(), 0.0f, 0.0f, tex);
	}

	if (bSpanThumb())
	{
		if ((EOrientation)orientation() == EOrientation::Horizontal)
			DrawHorzScrollbar(gc);
		else
			DrawVertScrollbar(gc);
	}
	else
	{
		if ((EOrientation)orientation() == EOrientation::Horizontal)
			DrawHorzSlider(gc);
		else
			DrawVertSlider(gc);
	}
}

void UScaleWindow::DrawHorzScrollbar(UGC* gc)
{
	gc->SetTileColor(thumbColor());
	gc->SetStyle((EDrawStyle)thumbStyle());
	float x = 10.0f;
	if (auto tex = preCapTexture())
	{
		gc->DrawStretchedTexture(x, 0.0f, preCapWidth(), Height(), 0.0f, 0.0f, preCapWidth(), preCapHeight(), tex);
		x += preCapWidth();
	}
	if (auto tex = thumbTexture())
	{
		gc->DrawStretchedTexture(x, 0.0f, ThumbWidth(), Height(), 0.0f, 0.0f, ThumbWidth(), ThumbHeight(), tex);
		x += ThumbWidth();
	}
	if (auto tex = postCapTexture())
	{
		gc->DrawStretchedTexture(x, 0.0f, postCapWidth(), Height(), 0.0f, 0.0f, postCapWidth(), postCapHeight(), tex);
	}
}

void UScaleWindow::DrawVertScrollbar(UGC* gc)
{
	gc->SetTileColor(thumbColor());
	gc->SetStyle((EDrawStyle)thumbStyle());
	float y = 100.0f;
	if (auto tex = preCapTexture())
	{
		gc->DrawStretchedTexture(0.0f, y, Width(), preCapHeight(), 0.0f, 0.0f, preCapWidth(), preCapHeight(), tex);
		y += preCapHeight();
	}
	if (auto tex = thumbTexture())
	{
		gc->DrawStretchedTexture(0.0f, y, Width(), ThumbHeight(), 0.0f, 0.0f, ThumbWidth(), ThumbHeight(), tex);
		y += ThumbHeight();
	}
	if (auto tex = postCapTexture())
	{
		gc->DrawStretchedTexture(0.0f, y, Width(), postCapHeight(), 0.0f, 0.0f, postCapWidth(), postCapHeight(), tex);
	}
}

void UScaleWindow::DrawHorzSlider(UGC* gc)
{
	float t = GetNumTicks() > 0 ? GetTickPosition() / (float)(GetNumTicks() - 1) : 0.0f;
	float x = startOffset() + t * (Width() - startOffset() - endOffset());
	if (auto tex = thumbTexture())
	{
		gc->SetTileColor(thumbColor());
		gc->SetStyle((EDrawStyle)thumbStyle());
		gc->DrawStretchedTexture(x - ThumbWidth() * 0.5f, (Height() - ThumbHeight()) * 0.5f, ThumbWidth(), ThumbHeight(), 0.0f, 0.0f, ThumbWidth(), ThumbHeight(), tex);
	}
}

void UScaleWindow::DrawVertSlider(UGC* gc)
{
	float t = GetNumTicks() > 0 ? GetTickPosition() / (float)(GetNumTicks() - 1) : 0.0f;
	float y = startOffset() + t * (Height() - startOffset() - endOffset());
	if (auto tex = thumbTexture())
	{
		gc->SetTileColor(thumbColor());
		gc->SetStyle((EDrawStyle)thumbStyle());
		gc->DrawStretchedTexture((Width() - ThumbWidth()) * 0.5f, y - ThumbHeight() * 0.5f, ThumbWidth(), ThumbHeight(), 0.0f, 0.0f, ThumbWidth(), ThumbHeight(), tex);
	}
}

bool UScaleWindow::MouseButtonPressed(float pointX, float pointY, EInputKey button, int numClicks)
{
	SetFocusWindow(this);
	UWindow::MouseButtonPressed(pointX, pointY, button, numClicks);
	return true;
}

bool UScaleWindow::MouseButtonReleased(float pointX, float pointY, EInputKey button, int numClicks)
{
	UWindow::MouseButtonPressed(pointX, pointY, button, numClicks);
	return true;
}

void UScaleWindow::MouseMoved(float newX, float newY)
{
	UWindow::MouseMoved(newX, newY);
}

void UScaleWindow::DispatchScalePositionChanged(int newTickPosition, float newValue, bool bFinal)
{
	for (UWindow* cur = this; cur != nullptr; cur = cur->parentOwner())
	{
		if (cur->ScalePositionChanged(this, newTickPosition, newValue, bFinal))
			break;
	}
}
