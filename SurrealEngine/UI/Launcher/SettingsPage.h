#pragma once

#include <zwidget/core/widget.h>

class LauncherWindow;
class ListView;
class TextLabel;
class PushButton;
class CheckboxLabel;
class LineEdit;
class Dropdown;

class SettingsPage : public Widget
{
public:
	SettingsPage(LauncherWindow* launcher);

	void Save();

private:
	void OnGeometryChanged() override;

	void OnResetButtonClicked();

	LauncherWindow* Launcher = nullptr;

	TextLabel* RenderDeviceLabel = nullptr;

	CheckboxLabel* Vulkan = nullptr;
	CheckboxLabel* D3D11 = nullptr;
	//CheckboxLabel* D3D12 = nullptr;

	TextLabel* AdvancedLabel = nullptr;

	CheckboxLabel* UseVSync = nullptr;

	TextLabel* AntialiasModesLabel = nullptr;
	Dropdown* AntialiasModes = nullptr;

	TextLabel* LightModesLabel = nullptr;
	Dropdown* LightModes = nullptr;

	TextLabel* GammaModesLabel = nullptr;
	Dropdown* GammaModes = nullptr;
	CheckboxLabel* GammaCorrectScreenshots = nullptr;

	CheckboxLabel* Hdr = nullptr;
	TextLabel* HdrScaleLabel = nullptr;
	LineEdit* HdrScale = nullptr;

	CheckboxLabel* Bloom = nullptr;
	TextLabel* BloomAmountLabel = nullptr;
	LineEdit* BloomAmount = nullptr;

	CheckboxLabel* UseDebugLayer = nullptr;
	PushButton* ResetButton = nullptr;
};
