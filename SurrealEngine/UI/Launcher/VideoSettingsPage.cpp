
#include "Precomp.h"
#include "VideoSettingsPage.h"
#include "LauncherWindow.h"
#include "LauncherSettings.h"
#include <surrealwidgets/core/resourcedata.h>
#include <surrealwidgets/widgets/textlabel/textlabel.h>
#include <surrealwidgets/widgets/checkboxlabel/checkboxlabel.h>
#include <surrealwidgets/widgets/lineedit/lineedit.h>
#include <surrealwidgets/widgets/dropdown/dropdown.h>
#include <surrealwidgets/widgets/pushbutton/pushbutton.h>
#include <surrealwidgets/widgets/layout/vboxlayout.h>
#include <surrealwidgets/widgets/layout/hboxlayout.h>
#include <algorithm>

	namespace
	{
		struct CommonResolution
		{
			const char* aspectRatio;
			int width;
			int height;
		};

	const std::vector<CommonResolution>& GetCommonStartupResolutions()
	{
		static const std::vector<CommonResolution> resolutions = {
			{ "16:10", 2560, 1600 },
			{ "16:9", 2560, 1440 },
			{ "4:3", 2048, 1536 },
			{ "16:10", 1920, 1200 },
			{ "16:9", 1920, 1080 },
			{ "4:3", 1600, 1200 },
			{ "16:10", 1680, 1050 },
			{ "16:9", 1600, 900 },
			{ "4:3", 1440, 1080 },
			{ "5:4", 1280, 1024 },
			{ "4:3", 1280, 960 },
			{ "16:10", 1280, 800 },
			{ "16:9", 1366, 768 },
			{ "16:9", 1280, 720 },
			{ "4:3", 1152, 864 },
			{ "4:3", 1024, 768 },
			{ "4:3", 800, 600 },
			{ "4:3", 640, 480 },
		};
		return resolutions;
	}

	const std::vector<int>& GetFallbackRefreshRates()
	{
		static const std::vector<int> refreshRates = { 60, 75, 120, 144, 165, 240 };
		return refreshRates;
	}

}

VideoSettingsPage::VideoSettingsPage(Widget* parent)
	: Widget(parent)
{
	RenderDeviceLabel = new TextLabel(this);
	Vulkan = new RadioButtonLabel(&RenderDeviceGroup, this);
#ifdef WIN32
	D3D11 = new RadioButtonLabel(&RenderDeviceGroup, this);
	//D3D12 = new RadioButtonLabel(&RenderDeviceGroup, this);
#endif
	OpenGL = new RadioButtonLabel(&RenderDeviceGroup, this);

	AdvancedLabel = new TextLabel(this);
	PresetLabel = new TextLabel(this);
	Preset = new Dropdown(this);
	QualityLabel = new TextLabel(this);
	StartupFullscreen = new CheckboxLabel(this);
	StartupResolutionLabel = new TextLabel(this);
	StartupResolution = new Dropdown(this);
	StartupRefreshRateLabel = new TextLabel(this);
	StartupRefreshRate = new Dropdown(this);
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
	OpenGL->SetText("OpenGL");
	AdvancedLabel->SetText("Render settings:");
	PresetLabel->SetText("Render preset:");
	Preset->AddItem("Custom");
	Preset->AddItem("Compatibility");
	Preset->AddItem("Balanced");
	Preset->AddItem("Quality");
	Preset->SetSelectedItem(0);
	Preset->OnChanged = [this](int index) { ApplyRenderPreset(index); };
	QualityLabel->SetText("Quality options:");
	StartupFullscreen->SetText("Start game in fullscreen");
	StartupResolutionLabel->SetText("Startup resolution");
	StartupRefreshRateLabel->SetText("Startup refresh rate");
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

	auto& settings = LauncherSettings::Get();
	Vulkan->SetChecked(settings.RenderDevice.Type == RenderDeviceType::Vulkan);
#ifdef WIN32
	D3D11->SetChecked(settings.RenderDevice.Type == RenderDeviceType::D3D11);
	//D3D12->SetChecked(settings.RenderDevice.Type == RenderDeviceType::D3D12);
#endif
	OpenGL->SetChecked(settings.RenderDevice.Type == RenderDeviceType::OpenGL);
	StartupFullscreen->SetChecked(settings.RenderDevice.StartupFullscreen);

	StartupResolutions.clear();
	StartupRefreshRates.clear();
	for (const CommonResolution& commonResolution : GetCommonStartupResolutions())
	{
		StartupResolutions.push_back(Size((double)commonResolution.width, (double)commonResolution.height));
		StartupRefreshRates.push_back({});
	}
	if (StartupResolutions.empty())
	{
		StartupResolutions.push_back(DisplayWindow::GetScreenSize());
		StartupRefreshRates.push_back({});
	}
	int startupResolutionIndex = 0;
	int startupRefreshIndex = 0;
	Size desiredStartupResolution(settings.RenderDevice.StartupViewportX, settings.RenderDevice.StartupViewportY);
	if (settings.RenderDevice.StartupViewportX > 0 && settings.RenderDevice.StartupViewportY > 0)
	{
		for (size_t i = 0; i < StartupResolutions.size(); ++i)
		{
			if (StartupResolutions[i] == desiredStartupResolution)
			{
				startupResolutionIndex = (int)i;
				break;
			}
		}
	}
	auto backend = DisplayBackend::Get();
	if (backend)
	{
		auto modes = backend->GetAvailableDisplayModes();
		for (const DisplayMode& mode : modes)
		{
			auto it = std::find(StartupResolutions.begin(), StartupResolutions.end(), mode.resolution);
			if (it == StartupResolutions.end())
				continue;

			size_t index = (size_t)std::distance(StartupResolutions.begin(), it);
			if (mode.refreshRate > 0)
			{
				auto& rates = StartupRefreshRates[index];
				if (std::find(rates.begin(), rates.end(), mode.refreshRate) == rates.end())
					rates.push_back(mode.refreshRate);
			}
		}
	}
	for (auto& rates : StartupRefreshRates)
		std::sort(rates.begin(), rates.end(), std::greater<int>());
	if (startupResolutionIndex >= 0 && (size_t)startupResolutionIndex < StartupRefreshRates.size())
	{
		const auto& rates = StartupRefreshRates[(size_t)startupResolutionIndex];
		if (settings.RenderDevice.StartupRefreshRate > 0)
		{
			auto it = std::find(rates.begin(), rates.end(), settings.RenderDevice.StartupRefreshRate);
			if (it != rates.end())
				startupRefreshIndex = (int)std::distance(rates.begin(), it) + 1;
		}
	}
	if (std::all_of(StartupRefreshRates.begin(), StartupRefreshRates.end(), [](const std::vector<int>& rates) { return rates.empty(); }))
	{
		for (auto& rates : StartupRefreshRates)
			rates = GetFallbackRefreshRates();
		if (settings.RenderDevice.StartupRefreshRate > 0)
		{
			const auto& rates = StartupRefreshRates[(size_t)startupResolutionIndex];
			auto it = std::find(rates.begin(), rates.end(), settings.RenderDevice.StartupRefreshRate);
			if (it != rates.end())
				startupRefreshIndex = (int)std::distance(rates.begin(), it) + 1;
		}
	}
	for (const Size& res : StartupResolutions)
	{
		const CommonResolution* commonResolution = nullptr;
		for (const CommonResolution& candidate : GetCommonStartupResolutions())
		{
			if (candidate.width == (int)res.width && candidate.height == (int)res.height)
			{
				commonResolution = &candidate;
				break;
			}
		}
		if (commonResolution)
			StartupResolution->AddItem(std::string(commonResolution->aspectRatio) + " " + std::to_string((int)res.width) + "x" + std::to_string((int)res.height));
		else
			StartupResolution->AddItem(std::to_string((int)res.width) + "x" + std::to_string((int)res.height));
	}
	StartupResolution->OnChanged = [this](int) { UpdateStartupRefreshRates(); };
	StartupResolution->SetSelectedItem(startupResolutionIndex);
	StartupRefreshRate->AddItem("Default");
	StartupRefreshRate->SetSelectedItem(startupRefreshIndex);
	UpdateStartupRefreshRates();
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
	renderDeviceLayout->AddWidget(OpenGL);
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
	auto presetLayout = new HBoxLayout();
	presetLayout->AddWidget(PresetLabel);
	presetLayout->AddWidget(Preset);
	presetLayout->AddStretch();
	mainLayout->AddLayout(presetLayout);
	mainLayout->AddWidget(QualityLabel);
	mainLayout->AddWidget(StartupFullscreen);
	auto resolutionLayout = new HBoxLayout();
	resolutionLayout->AddWidget(StartupResolutionLabel);
	resolutionLayout->AddWidget(StartupResolution);
	resolutionLayout->AddWidget(StartupRefreshRateLabel);
	resolutionLayout->AddWidget(StartupRefreshRate);
	resolutionLayout->AddStretch();
	mainLayout->AddLayout(resolutionLayout);
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
	if (OpenGL->GetChecked())
	settings.RenderDevice.Type = RenderDeviceType::OpenGL;

	settings.RenderDevice.StartupFullscreen = StartupFullscreen->GetChecked();
	auto selectedResolution = StartupResolution->GetSelectedItem();
	settings.RenderDevice.StartupViewportX = 0;
	settings.RenderDevice.StartupViewportY = 0;
	settings.RenderDevice.StartupRefreshRate = 0;
	if (selectedResolution >= 0)
	{
		const Size& res = StartupResolutions[std::min<size_t>((size_t)selectedResolution, StartupResolutions.size() - 1)];
		settings.RenderDevice.StartupViewportX = (int)res.width;
		settings.RenderDevice.StartupViewportY = (int)res.height;
		int selectedRefresh = StartupRefreshRate->GetSelectedItem();
		if (selectedRefresh > 0)
			settings.RenderDevice.StartupRefreshRate = std::atoi(StartupRefreshRate->GetItem(selectedRefresh).c_str());
	}
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
	if (Preset->GetSelectedItem() == 0)
		settings.RenderDevice.UseVSync = UseVSync->GetChecked();
}

void VideoSettingsPage::OnResetButtonClicked()
{
	Vulkan->SetChecked(true);
	#ifdef WIN32
	D3D11->SetChecked(false);
	//D3D12->SetChecked(false);
	#endif
	StartupFullscreen->SetChecked(false);
	StartupResolution->SetSelectedItem(0);
	StartupRefreshRate->SetSelectedItem(0);
	OpenGL->SetChecked(false);
	Preset->SetSelectedItem(0);
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

void VideoSettingsPage::UpdateStartupRefreshRates()
{
	StartupRefreshRate->ClearItems();
	StartupRefreshRate->AddItem("Default");

	int selectedResolution = StartupResolution->GetSelectedItem();
	if (selectedResolution < 0 || (size_t)selectedResolution >= StartupRefreshRates.size())
	{
		StartupRefreshRate->SetSelectedItem(0);
		return;
	}

	const auto& refreshRates = StartupRefreshRates[(size_t)selectedResolution];
	const auto& effectiveRefreshRates = refreshRates.empty() ? GetFallbackRefreshRates() : refreshRates;
	for (int refreshRate : effectiveRefreshRates)
		StartupRefreshRate->AddItem(std::to_string(refreshRate) + " Hz");

	StartupRefreshRate->SetSelectedItem(0);
}

void VideoSettingsPage::ApplyRenderPreset(int index)
{
	if (bUpdatingPreset)
		return;

	bUpdatingPreset = true;
	switch (index)
	{
	default:
	case 0: // Custom
		break;
	case 1: // Compatibility
		UseVSync->SetChecked(true);
		GammaCorrectScreenshots->SetChecked(false);
		Hdr->SetChecked(false);
		Bloom->SetChecked(false);
		UseDebugLayer->SetChecked(false);
		break;
	case 2: // Balanced
		UseVSync->SetChecked(true);
		GammaCorrectScreenshots->SetChecked(true);
		Hdr->SetChecked(false);
		Bloom->SetChecked(true);
		BloomAmount->SetTextInt(128);
		UseDebugLayer->SetChecked(false);
		break;
	case 3: // Quality
		UseVSync->SetChecked(true);
		GammaCorrectScreenshots->SetChecked(true);
		Hdr->SetChecked(true);
		HdrScale->SetTextInt(128);
		Bloom->SetChecked(true);
		BloomAmount->SetTextInt(160);
		UseDebugLayer->SetChecked(false);
		break;
	}
	bUpdatingPreset = false;
}
