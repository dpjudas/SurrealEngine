
#include "SettingsPage.h"
#include "LauncherWindow.h"
#include <zwidget/core/resourcedata.h>
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/widgets/checkboxlabel/checkboxlabel.h>
#include <zwidget/widgets/lineedit/lineedit.h>
#include <zwidget/widgets/dropdown/dropdown.h>
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/systemdialogs/open_folder_dialog.h>

SettingsPage::SettingsPage(LauncherWindow* launcher) : Widget(nullptr), Launcher(launcher)
{
	RenderDeviceLabel = new TextLabel(this);
	Vulkan = new CheckboxLabel(this);
	D3D11 = new CheckboxLabel(this);
	//D3D12 = new CheckboxLabel(this);
	Vulkan->SetRadioStyle(true);
	D3D11->SetRadioStyle(true);
	//D3D12->SetRadioStyle(true);

	AdvancedLabel = new TextLabel(this);
	UseVSync = new CheckboxLabel(this);
	AntialiasModesLabel = new TextLabel(this);
	AntialiasModes = new Dropdown(this);
	LightModesLabel = new TextLabel(this);
	LightModes = new Dropdown(this);
	GammaModesLabel = new TextLabel(this);
	GammaModes = new Dropdown(this);
	GammaCorrectScreenshots = new CheckboxLabel(this);
	Hdr = new CheckboxLabel(this);
	HdrScaleLabel = new TextLabel(this);
	HdrScale = new LineEdit(this);
	Bloom = new CheckboxLabel(this);
	BloomAmountLabel = new TextLabel(this);
	BloomAmount = new LineEdit(this);
	UseDebugLayer = new CheckboxLabel(this);
	ResetButton = new PushButton(this);

	RenderDeviceLabel->SetText("Render device:");
	Vulkan->SetText("Vulkan");
	D3D11->SetText("Direct3D 11");
	//D3D12->SetText("Direct3D 12");
	AdvancedLabel->SetText("Render settings:");
	UseVSync->SetText("Use vertical sync");
	AntialiasModesLabel->SetText("Anti aliasing");
	LightModesLabel->SetText("Light mode");
	GammaModesLabel->SetText("Gamma mode");
	GammaCorrectScreenshots->SetText("Gamma correct screenshots");
	Hdr->SetText("HDR mode");
	HdrScaleLabel->SetText("HDR scale");
	Bloom->SetText("Bloom effect");
	BloomAmountLabel->SetText("Bloom amount");
	UseDebugLayer->SetText("Enable debug layer (slow)");
	ResetButton->SetText("Reset to defaults");

	AntialiasModes->AddItem("Off");
	AntialiasModes->AddItem("MSAA 2x");
	AntialiasModes->AddItem("MSAA 4x");

	GammaModes->AddItem("D3D9");
	GammaModes->AddItem("XOpenGL");

	LightModes->AddItem("Normal");
	LightModes->AddItem("1x blending");
	LightModes->AddItem("Brighter actors");

	Vulkan->FuncChanged = [this](bool on) { if (on) { D3D11->SetChecked(false); /*D3D12->SetChecked(false);*/ }};
	D3D11->FuncChanged = [this](bool on) { if (on) { Vulkan->SetChecked(false); /*D3D12->SetChecked(false);*/ }};
	//D3D12->FuncChanged = [this](bool on) { if (on) { Vulkan->SetChecked(false); D3D11->SetChecked(false); }};

	Vulkan->SetChecked(true);

	HdrScale->SetText("128");
	BloomAmount->SetText("128");

	ResetButton->OnClick = [this]() { OnResetButtonClicked(); };
}

void SettingsPage::Save()
{
}

void SettingsPage::OnResetButtonClicked()
{
}

void SettingsPage::OnGeometryChanged()
{
	double y = 10.0;
	double gap = 2.0;
	double groupEndGap = 7.0;
	double width = GetWidth();
	double labelWidth = 150.0;
	double lineHeight = 23.0;
	double dropdownWidth = 140.0;
	double byteLineEditWidth = 50.0;
	double rendevWidth = 100.0;

	RenderDeviceLabel->SetFrameGeometry(0.0, y, width, lineHeight);
	y += lineHeight + gap;

	Vulkan->SetFrameGeometry(0.0, y, rendevWidth, lineHeight);
	D3D11->SetFrameGeometry(rendevWidth, y, rendevWidth, lineHeight);
	//D3D12->SetFrameGeometry(rendevWidth * 2.0, y, rendevWidth, lineHeight);
	y += lineHeight + groupEndGap;

	AdvancedLabel->SetFrameGeometry(0.0, y, width, lineHeight);
	y += lineHeight + gap;

	UseVSync->SetFrameGeometry(0.0, y, width, lineHeight);
	y += lineHeight + gap;

	AntialiasModesLabel->SetFrameGeometry(0.0, y, labelWidth, lineHeight);
	AntialiasModes->SetFrameGeometry(labelWidth, y, dropdownWidth, lineHeight);
	y += lineHeight + gap;

	LightModesLabel->SetFrameGeometry(0.0, y, labelWidth, lineHeight);
	LightModes->SetFrameGeometry(labelWidth, y, dropdownWidth, lineHeight);
	y += lineHeight + gap;

	GammaModesLabel->SetFrameGeometry(0.0, y, labelWidth, lineHeight);
	GammaModes->SetFrameGeometry(labelWidth, y, dropdownWidth, lineHeight);
	y += lineHeight + gap;

	GammaCorrectScreenshots->SetFrameGeometry(0.0, y, width, lineHeight);
	y += lineHeight + gap;

	Hdr->SetFrameGeometry(0.0, y, width, lineHeight);
	y += lineHeight + gap;

	HdrScaleLabel->SetFrameGeometry(0.0, y, labelWidth, lineHeight);
	HdrScale->SetFrameGeometry(labelWidth, y, byteLineEditWidth, lineHeight);
	y += lineHeight + gap;

	Bloom->SetFrameGeometry(0.0, y, width, lineHeight);
	y += lineHeight + gap;

	BloomAmountLabel->SetFrameGeometry(0.0, y, labelWidth, lineHeight);
	BloomAmount->SetFrameGeometry(labelWidth, y, byteLineEditWidth, lineHeight);
	y += lineHeight + groupEndGap;

	UseDebugLayer->SetFrameGeometry(0.0, y, width, lineHeight);
	y += lineHeight + groupEndGap;

	ResetButton->SetFrameGeometry(0.0, y, 150.0, ResetButton->GetPreferredHeight());
	y += ResetButton->GetPreferredHeight() + gap;
}
