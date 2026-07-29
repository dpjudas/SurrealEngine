
#include "Precomp.h"
#include "UTileWindow.h"

void UTileWindow::EnableWrapping(bool bWrapOn)
{
	bWrap() = bWrapOn;
}

void UTileWindow::FillParent(bool FillParent)
{
	bFillParent() = FillParent;
}

void UTileWindow::MakeHeightsEqual(bool bEqual)
{
	bEqualHeight() = bEqual;
}

void UTileWindow::MakeWidthsEqual(bool bEqual)
{
	bEqualWidth() = bEqual;
}

void UTileWindow::SetChildAlignments(uint8_t newHAlign, uint8_t newVAlign)
{
	hChildAlign() = newHAlign;
	vChildAlign() = newVAlign;
}

void UTileWindow::SetDirections(uint8_t newHDir, uint8_t newVDir)
{
	hDirection() = newHDir;
	vDirection() = newVDir;
}

void UTileWindow::SetMajorSpacing(float newSpacing)
{
	majorSpacing() = newSpacing;
}

void UTileWindow::SetMargins(float newHMargin, float newVMargin)
{
	hMargin() = newHMargin;
	vMargin() = newVMargin;
}

void UTileWindow::SetMinorSpacing(float newSpacing)
{
	minorSpacing() = newSpacing;
}

void UTileWindow::SetOrder(EOrder newOrder)
{
	bWrap() = true;
	uint8_t horizontal = (uint8_t)EOrientation::Horizontal;
	uint8_t vertical = (uint8_t)EOrientation::Vertical;
	uint8_t leftToRight = (uint8_t)EHDirection::LeftToRight;
	uint8_t rightToLeft = (uint8_t)EHDirection::RightToLeft;
	uint8_t topToBottom = (uint8_t)EVDirection::TopToBottom;
	uint8_t bottomToTop = (uint8_t)EVDirection::BottomToTop;
	switch (newOrder)
	{
		//Offsetreminder:orientation,hdirection,vdirection,bwrap
	case EOrder::Right:
		orientation() = horizontal;
		hDirection() = leftToRight;
		vDirection() = topToBottom;
		bWrap() = false;
		break;
	case EOrder::Left:
		orientation() = horizontal;
		hDirection() = rightToLeft;
		vDirection() = topToBottom;
		bWrap() = false;
		break;
	case EOrder::Down:
		orientation() = vertical;
		hDirection() = leftToRight;
		vDirection() = topToBottom;
		bWrap() = false;
		break;
	case EOrder::Up:
		orientation() = vertical;
		hDirection() = leftToRight;
		vDirection() = topToBottom;
		bWrap() = false;
		break;
	case EOrder::RightThenDown:
		orientation() = horizontal;
		hDirection() = leftToRight;
		vDirection() = topToBottom;
		break;
	case EOrder::RightThenUp:
		orientation() = horizontal;
		hDirection() = leftToRight;
		vDirection() = bottomToTop;
		break;
	case EOrder::LeftThenDown:
		orientation() = horizontal;
		hDirection() = rightToLeft;
		vDirection() = topToBottom;
		break;
	case EOrder::LeftThenUp:
		orientation() = horizontal;
		hDirection() = rightToLeft;
		vDirection() = bottomToTop;
		break;
	case EOrder::DownThenRight:
		orientation() = vertical;
		hDirection() = leftToRight;
		vDirection() = topToBottom;
		break;
	case EOrder::DownThenLeft:
		orientation() = vertical;
		hDirection() = rightToLeft;
		vDirection() = topToBottom;
		break;
	case EOrder::UpThenRight:
		orientation() = vertical;
		hDirection() = leftToRight;
		vDirection() = bottomToTop;
		break;
	case EOrder::UpThenLeft:
		orientation() = vertical;
		hDirection() = rightToLeft;
		vDirection() = topToBottom;
		break;
	}
	AskParentForReconfigure();
}

void UTileWindow::SetOrientation(uint8_t newOrientation)
{
	orientation() = newOrientation;
}

void UTileWindow::ParentRequestedPreferredSize(bool bWidthSpecified, float& preferredWidth, bool bHeightSpecified, float& preferredHeight)
{
	EOrientation orient = (EOrientation)orientation();
	EHDirection hdir = (EHDirection)hDirection();
	EVDirection vdir = (EVDirection)vDirection();
	bool wrap = bWrap();

	// To do: implement more of this

	if (orient == EOrientation::Horizontal)
	{
		preferredWidth = 0.0f;
		if (!bHeightSpecified)
			preferredHeight = 0.0f;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			float w = 0.0f, h = 0.0f;
			if (bHeightSpecified)
				w = cur->QueryPreferredWidth(preferredHeight);
			else
				cur->QueryPreferredSize(w, h);
			preferredHeight = std::max(preferredHeight, h);
			preferredWidth += w;
		}
	}
	else
	{
		if (!bWidthSpecified)
			preferredWidth = 0.0f;
		preferredHeight = 0.0f;
		for (auto cur = firstChild(); cur; cur = cur->nextSibling())
		{
			float w = 0.0f, h = 0.0f;
			if (bWidthSpecified)
				h = cur->QueryPreferredHeight(preferredWidth);
			else
				cur->QueryPreferredSize(w, h);
			preferredWidth = std::max(preferredWidth, w);
			preferredHeight += h;
		}
	}
}

void UTileWindow::ConfigurationChanged()
{
	EOrientation orient = (EOrientation)orientation();
	EHDirection hdir = (EHDirection)hDirection();
	EVDirection vdir = (EVDirection)vDirection();
	bool wrap = bWrap();

	// To do: implement more of this

	if (orient == EOrientation::Horizontal)
	{
		if (hdir == EHDirection::LeftToRight)
		{
			float x = 0.0f;
			for (auto cur = firstChild(); cur; cur = cur->nextSibling())
			{
				float w = cur->QueryPreferredWidth(Height());
				float h = cur->QueryPreferredHeight(w);
				cur->ConfigureChild(x, 0.0f, w, std::min(h, Height()));
				x += w;
			}
		}
		else // if (hdir == EHDirection::RightToLeft)
		{
			float x = Width();
			for (auto cur = firstChild(); cur; cur = cur->nextSibling())
			{
				float w = cur->QueryPreferredWidth(Height());
				float h = cur->QueryPreferredHeight(w);
				x -= w;
				cur->ConfigureChild(x, 0.0f, w, std::min(h, Height()));
			}
		}
	}
	else
	{
		if (vdir == EVDirection::TopToBottom)
		{
			float y = 0.0f;
			for (auto cur = firstChild(); cur; cur = cur->nextSibling())
			{
				float h = cur->QueryPreferredHeight(Width());
				float w = cur->QueryPreferredWidth(h);
				cur->ConfigureChild(0.0f, y, std::min(w, Width()), h);
				y += h;
			}
		}
		else // if (vdir == EVDirection::BottomToTop)
		{
			float y = Height();
			for (auto cur = firstChild(); cur; cur = cur->nextSibling())
			{
				float h = cur->QueryPreferredHeight(Width());
				float w = cur->QueryPreferredWidth(h);
				y -= h;
				cur->ConfigureChild(0.0f, y, std::min(w, Width()), h);
			}
		}
	}

	UWindow::ConfigurationChanged();
}

void UTileWindow::DrawWindow(UGC* gc)
{
	UWindow::DrawWindow(gc);
	// DrawDebugBox(gc);
}

bool UTileWindow::ChildRequestedReconfiguration(UWindow* childWin)
{
	AskParentForReconfigure();
	return false;
}

void UTileWindow::ChildAdded(UWindow* child)
{
	bConfigured() = false;
}

void UTileWindow::ChildRemoved(UWindow* child)
{
	bConfigured() = false;
}
