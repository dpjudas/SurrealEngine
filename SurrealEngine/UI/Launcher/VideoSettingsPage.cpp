#include "VideoSettingsPage.h"
#include "LauncherWindow.h"
#include "LauncherSettings.h"
#include <zwidget/core/resourcedata.h>
#include <zwidget/widgets/textlabel/textlabel.h>
#include <zwidget/widgets/checkboxlabel/checkboxlabel.h>
#include <zwidget/widgets/lineedit/lineedit.h>
#include <zwidget/widgets/dropdown/dropdown.h>
#include <zwidget/widgets/pushbutton/pushbutton.h>
#include <zwidget/widgets/layout/vboxlayout.h>
#include <zwidget/widgets/layout/hboxlayout.h>

VideoSettingsPage::VideoSettingsPage(Widget* parent)
    : Widget(parent)
{
    RenderDeviceLabel = new TextLabel(this);
	Vulkan = new CheckboxLabel(this);
	Vulkan->SetRadioStyle(true);
#ifdef WIN32
	D3D11 = new CheckboxLabel(this);
	//D3D12 = new CheckboxLabel(this);
	D3D11->SetRadioStyle(true);
	//D3D12->SetRadioStyle(true);
#endif

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
#ifdef WIN32
	D3D11->SetText("Direct3D 11");
	//D3D12->SetText("Direct3D 12");
#endif
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

	HdrScale->SetIntrinsicSize(3);
	BloomAmount->SetIntrinsicSize(3);

	AntialiasModes->AddItem("Off");
	AntialiasModes->AddItem("MSAA 2x");
	AntialiasModes->AddItem("MSAA 4x");

	GammaModes->AddItem("D3D9");
	GammaModes->AddItem("XOpenGL");

	LightModes->AddItem("Normal");
	LightModes->AddItem("1x blending");
	LightModes->AddItem("Brighter actors");


#ifdef WIN32
	Vulkan->FuncChanged = [this](bool on) { if (on) { D3D11->SetChecked(false); /*D3D12->SetChecked(false);*/ }};
	D3D11->FuncChanged = [this](bool on) { if (on) { Vulkan->SetChecked(false); /*D3D12->SetChecked(false);*/ }};
	//D3D12->FuncChanged = [this](bool on) { if (on) { Vulkan->SetChecked(false); D3D11->SetChecked(false); }};
#else
	Vulkan->FuncChanged = [this](bool on) { /* Add checkbox logic here if an OpenGL renderer ever gets added */ };
#endif

	auto& settings = LauncherSettings::Get();
	Vulkan->SetChecked(settings.RenderDevice.Type == RenderDeviceType::Vulkan);
#ifdef WIN32
	D3D11->SetChecked(settings.RenderDevice.Type == RenderDeviceType::D3D11);
	//D3D12->SetChecked(settings.RenderDevice.Type == RenderDeviceType::D3D12);
#endif
	UseVSync->SetChecked(settings.RenderDevice.UseVSync);
	AntialiasModes->SetSelectedItem((int)settings.RenderDevice.Antialias);
	LightModes->SetSelectedItem((int)settings.RenderDevice.Light);
	GammaModes->SetSelectedItem((int)settings.RenderDevice.Gamma);
	GammaCorrectScreenshots->SetChecked(settings.RenderDevice.GammaCorrectScreenshots);
	Hdr->SetChecked(settings.RenderDevice.Hdr);
	HdrScale->SetTextInt(settings.RenderDevice.HdrScale);
	Bloom->SetChecked(settings.RenderDevice.Bloom);
	BloomAmount->SetTextInt(settings.RenderDevice.BloomAmount);
	UseDebugLayer->SetChecked(settings.RenderDevice.UseDebugLayer);

	ResetButton->OnClick = [this]() { OnResetButtonClicked(); };

	auto renderDeviceLayout = new HBoxLayout();

	renderDeviceLayout->AddWidget(Vulkan);
#ifdef WIN32
	renderDeviceLayout->AddWidget(D3D11);
	// renderDeviceLayout->AddWidget(D3D12);
#endif
	renderDeviceLayout->AddStretch();

	auto antialiasModesLayout = new HBoxLayout();
	antialiasModesLayout->AddWidget(AntialiasModesLabel);
	antialiasModesLayout->AddWidget(AntialiasModes);
	antialiasModesLayout->AddStretch();

	auto lightModesLayout = new HBoxLayout();
	lightModesLayout->AddWidget(LightModesLabel);
	lightModesLayout->AddWidget(LightModes);
	lightModesLayout->AddStretch();

	auto gammaModesLayout = new HBoxLayout();
	gammaModesLayout->AddWidget(GammaModesLabel);
	gammaModesLayout->AddWidget(GammaModes);
	gammaModesLayout->AddStretch();

	auto hdrScaleLayout = new HBoxLayout();
	hdrScaleLayout->AddWidget(HdrScaleLabel);
	hdrScaleLayout->AddWidget(HdrScale);
	hdrScaleLayout->AddStretch();

	auto bloomAmountLayout = new HBoxLayout();
	bloomAmountLayout->AddWidget(BloomAmountLabel);
	bloomAmountLayout->AddWidget(BloomAmount);
	bloomAmountLayout->AddStretch();

	auto resetButtonLayout = new HBoxLayout();
	resetButtonLayout->AddWidget(ResetButton);
	resetButtonLayout->AddStretch();

	auto mainLayout = new VBoxLayout();

	mainLayout->AddWidget(RenderDeviceLabel);
	mainLayout->AddLayout(renderDeviceLayout);

	mainLayout->AddWidget(AdvancedLabel);
	mainLayout->AddWidget(UseVSync);

	mainLayout->AddLayout(antialiasModesLayout);

	mainLayout->AddLayout(lightModesLayout);

	mainLayout->AddLayout(gammaModesLayout);

	mainLayout->AddWidget(GammaCorrectScreenshots);

	mainLayout->AddWidget(Hdr);
	mainLayout->AddLayout(hdrScaleLayout);

	mainLayout->AddWidget(Bloom);
	mainLayout->AddLayout(bloomAmountLayout);

	mainLayout->AddWidget(UseDebugLayer);

	mainLayout->AddLayout(resetButtonLayout);

	mainLayout->AddStretch();

	SetLayout(mainLayout);
}

void VideoSettingsPage::Save()
{
	auto& settings = LauncherSettings::Get();

	if (Vulkan->GetChecked())
		settings.RenderDevice.Type = RenderDeviceType::Vulkan;
#ifdef WIN32
	if (D3D11->GetChecked())
		settings.RenderDevice.Type = RenderDeviceType::D3D11;
	//if (D3D12->GetChecked())
	//	settings.RenderDevice.Type = RenderDeviceType::D3D12;
#endif

	settings.RenderDevice.UseVSync = UseVSync->GetChecked();

	if (AntialiasModes->GetSelectedItem() == 0)
		settings.RenderDevice.Antialias = AntialiasMode::Off;
	else if (AntialiasModes->GetSelectedItem() == 1)
		settings.RenderDevice.Antialias = AntialiasMode::MSAA2x;
	else if (AntialiasModes->GetSelectedItem() == 2)
		settings.RenderDevice.Antialias = AntialiasMode::MSAA4x;

	if (LightModes->GetSelectedItem() == 0)
		settings.RenderDevice.Light = LightMode::Normal;
	else if (LightModes->GetSelectedItem() == 1)
		settings.RenderDevice.Light = LightMode::OneX;
	else if (LightModes->GetSelectedItem() == 2)
		settings.RenderDevice.Light = LightMode::BrighterActors;

	if (GammaModes->GetSelectedItem() == 0)
		settings.RenderDevice.Gamma = GammaMode::D3D9;
	else if (GammaModes->GetSelectedItem() == 1)
		settings.RenderDevice.Gamma = GammaMode::XOpenGL;

	settings.RenderDevice.GammaCorrectScreenshots = GammaCorrectScreenshots->GetChecked();
	settings.RenderDevice.Hdr = Hdr->GetChecked();
	settings.RenderDevice.HdrScale = HdrScale->GetTextInt();
	settings.RenderDevice.Bloom = Bloom->GetChecked();
	settings.RenderDevice.BloomAmount = BloomAmount->GetTextInt();
	settings.RenderDevice.UseDebugLayer = UseDebugLayer->GetChecked();
}

void VideoSettingsPage::OnResetButtonClicked()
{
	Vulkan->SetChecked(true);
#ifdef WIN32
	D3D11->SetChecked(false);
	//D3D12->SetChecked(false);
#endif
	UseVSync->SetChecked(true);
	AntialiasModes->SetSelectedItem(0);
	LightModes->SetSelectedItem(0);
	GammaModes->SetSelectedItem(0);
	Hdr->SetChecked(false);
	HdrScale->SetTextInt(128);
	Bloom->SetChecked(false);
	BloomAmount->SetTextInt(128);
	UseDebugLayer->SetChecked(false);
}
