#pragma once

#include <surrealwidgets/core/widget.h>
#include <surrealwidgets/widgets/radiobuttonlabel/radiobuttonlabel.h>

class LauncherWindow;
class ListView;
class TextLabel;
class PushButton;
class CheckboxLabel;
class LineEdit;
class Dropdown;

class VideoSettingsPage : public Widget
{
public:
	VideoSettingsPage(Widget* parent);

	void Save();

private:
	void OnResetButtonClicked();

	LauncherWindow* Launcher = nullptr;

	TextLabel* RenderDeviceLabel = nullptr;

	RadioGroup RenderDeviceGroup;
	RadioButtonLabel* Vulkan = nullptr;
	RadioButtonLabel* OpenGL = nullptr;

#ifdef WIN32
	RadioButtonLabel* D3D11 = nullptr;
	//RadioButtonLabel* D3D12 = nullptr;
#endif

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