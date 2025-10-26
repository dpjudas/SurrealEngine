
#include "LauncherBanner.h"
#include <zwidget/widgets/imagebox/imagebox.h>
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/core/image.h>

LauncherBanner::LauncherBanner(Widget* parent) : Widget(parent)
{
	Logo = new ImageBox(this);
	VersionLabel = new TextLabel(this);
	VersionLabel->SetTextAlignment(TextLabelAlignment::Right);
	// VersionLabel->SetText("1.0");

	Logo->SetImage(Image::LoadResource("banner.png"));
}

double LauncherBanner::GetPreferredHeight()
{
	return Logo->GetPreferredHeight();
}

double LauncherBanner::GetPreferredWidth()
{
	return Logo->GetPreferredWidth();
}


void LauncherBanner::OnGeometryChanged()
{
	Logo->SetFrameGeometry(0.0, 0.0, GetWidth(), Logo->GetPreferredHeight());
	VersionLabel->SetFrameGeometry(20.0, GetHeight() - 10.0 - VersionLabel->GetPreferredHeight(), GetWidth() - 40.0, VersionLabel->GetPreferredHeight());
}
