#pragma once

#include <zwidget/core/widget.h>

class ImageBox;
class TextLabel;

class LauncherBanner : public Widget
{
public:
	LauncherBanner(Widget* parent);

	double GetPreferredHeight() override;
	double GetPreferredWidth() override;

private:
	void OnGeometryChanged() override;

	ImageBox* Logo = nullptr;
	TextLabel* VersionLabel = nullptr;
};
