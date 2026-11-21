
#include "Precomp.h"
#include "AudioSettingsPage.h"
#include "LauncherWindow.h"
#include "LauncherSettings.h"
#include <surrealwidgets/widgets/textlabel/textlabel.h>
#include <surrealwidgets/widgets/checkboxlabel/checkboxlabel.h>
#include <surrealwidgets/widgets/radiobuttonlabel/radiobuttonlabel.h>
#include <surrealwidgets/widgets/pushbutton/pushbutton.h>
#include <surrealwidgets/widgets/dropdown/dropdown.h>
#include <surrealwidgets/widgets/layout/vboxlayout.h>
#include <surrealwidgets/widgets/layout/hboxlayout.h>

AudioSettingsPage::AudioSettingsPage(Widget* parent)
	: Widget(parent)
{
	AudioDeviceGroup = new RadioGroup();

	OpenALDevice = new RadioButtonLabel(AudioDeviceGroup, this);
	AudioPlayerDevice = new RadioButtonLabel(AudioDeviceGroup, this);
	NullAudioDevice = new RadioButtonLabel(AudioDeviceGroup, this);

	UseFilter = new CheckboxLabel(this);
	UseSurround = new CheckboxLabel(this);
	UseStereo = new CheckboxLabel(this);
	UseSpatial = new CheckboxLabel(this);
	UseReverb = new CheckboxLabel(this);
	Use3DHardware = new CheckboxLabel(this);
	ReverseStereo = new CheckboxLabel(this);

	OutputRateLabel = new TextLabel(this);
	OutputRate = new Dropdown(this);

	ResetButton = new PushButton(this);

	auto audioDeviceLabel = new TextLabel(this);

	audioDeviceLabel->SetText("Audio Driver:");

	OpenALDevice->SetText("OpenAL");
	AudioPlayerDevice->SetText("Audio Player");
	NullAudioDevice->SetText("Disable Audio");

	auto settingsLabel = new TextLabel(this);
	settingsLabel->SetText("Audio Settings:");

	UseFilter->SetText("Use Filter");
	UseSurround->SetText("Use Surround");
	UseStereo->SetText("Use Stereo");
	UseSpatial->SetText("Use Spatial");
	UseReverb->SetText("Use Reverb");
	Use3DHardware->SetText("Use 3D Hardware");
	ReverseStereo->SetText("Reverse Stereo");

	OutputRateLabel->SetText("Output Rate:");
	OutputRate->AddItem("11025Hz");
	OutputRate->AddItem("22050Hz");
	OutputRate->AddItem("44100Hz");
	OutputRate->AddItem("48000Hz");

	ResetButton->SetText("Reset to defaults");

	ResetButton->OnClick = [this] () { OnResetButtonClicked(); };

	const auto& settings = LauncherSettings::Get();

	OpenALDevice->SetChecked(settings.AudioDevice.Type == AudioDeviceType::OpenAL);
	AudioPlayerDevice->SetChecked(settings.AudioDevice.Type == AudioDeviceType::AudioPlayer);
	NullAudioDevice->SetChecked(settings.AudioDevice.Type == AudioDeviceType::NullAudio);

	UseFilter->SetChecked(settings.AudioDevice.UseFilter);
	UseSurround->SetChecked(settings.AudioDevice.UseSurround);
	UseStereo->SetChecked(settings.AudioDevice.UseStereo);
	UseSpatial->SetChecked(settings.AudioDevice.UseSpatial);
	UseReverb->SetChecked(settings.AudioDevice.UseReverb);
	Use3DHardware->SetChecked(settings.AudioDevice.Use3DHardware);
	ReverseStereo->SetChecked(settings.AudioDevice.ReverseStereo);

	switch (settings.AudioDevice.OutputRate)
	{
		case AudioOutputRate::OR_11025Hz:
			OutputRate->SetSelectedItem(0);
			break;
		case AudioOutputRate::OR_22050Hz:
			OutputRate->SetSelectedItem(1);
			break;
		case AudioOutputRate::OR_44100Hz:
			OutputRate->SetSelectedItem(2);
			break;
		case AudioOutputRate::OR_48000Hz:
			OutputRate->SetSelectedItem(3);
			break;
	}

	auto audioDeviceLayout = new HBoxLayout();

	audioDeviceLayout->AddWidget(OpenALDevice);
	audioDeviceLayout->AddWidget(AudioPlayerDevice);
	audioDeviceLayout->AddWidget(NullAudioDevice);
	audioDeviceLayout->AddStretch();

	auto outputRateLayout = new HBoxLayout();

	outputRateLayout->AddWidget(OutputRateLabel);
	outputRateLayout->AddWidget(OutputRate);
	outputRateLayout->AddStretch();

	auto resetButtonLayout = new HBoxLayout();
	resetButtonLayout->AddWidget(ResetButton);
	resetButtonLayout->AddStretch();

	auto mainLayout = new VBoxLayout();

	mainLayout->AddWidget(audioDeviceLabel);
	mainLayout->AddLayout(audioDeviceLayout);
	mainLayout->AddWidget(settingsLabel);
	mainLayout->AddWidget(UseFilter);
	mainLayout->AddWidget(UseSurround);
	mainLayout->AddWidget(UseStereo);
	mainLayout->AddWidget(UseSpatial);
	mainLayout->AddWidget(Use3DHardware);
	mainLayout->AddWidget(ReverseStereo);
	mainLayout->AddLayout(outputRateLayout);
	mainLayout->AddStretch();
	mainLayout->AddLayout(resetButtonLayout);

	SetLayout(mainLayout);
}

void AudioSettingsPage::Save() const
{
	auto& settings = LauncherSettings::Get();

	if (OpenALDevice->GetChecked())
		settings.AudioDevice.Type = AudioDeviceType::OpenAL;
	else if (AudioPlayerDevice->GetChecked())
		settings.AudioDevice.Type = AudioDeviceType::AudioPlayer;
	else if (NullAudioDevice->GetChecked())
		settings.AudioDevice.Type = AudioDeviceType::NullAudio;

	settings.AudioDevice.UseFilter     = UseFilter->GetChecked();
	settings.AudioDevice.UseSurround   = UseSurround->GetChecked();
	settings.AudioDevice.UseStereo     = UseStereo->GetChecked();
	settings.AudioDevice.UseSpatial    = UseSpatial->GetChecked();
	settings.AudioDevice.UseReverb     = UseReverb->GetChecked();
	settings.AudioDevice.Use3DHardware = Use3DHardware->GetChecked();
	settings.AudioDevice.ReverseStereo = ReverseStereo->GetChecked();

	switch (OutputRate->GetSelectedItem())
	{
		case 0:
			settings.AudioDevice.OutputRate = AudioOutputRate::OR_11025Hz;
			break;
		case 1:
			settings.AudioDevice.OutputRate = AudioOutputRate::OR_22050Hz;
			break;
		case 2:
			settings.AudioDevice.OutputRate = AudioOutputRate::OR_44100Hz;
			break;
		case 3:
			settings.AudioDevice.OutputRate = AudioOutputRate::OR_48000Hz;
			break;
	}
}

void AudioSettingsPage::OnResetButtonClicked()
{
	UseFilter->SetChecked(true);
	UseSurround->SetChecked(true);
	UseStereo->SetChecked(true);
	UseSpatial->SetChecked(true);
	UseReverb->SetChecked(true);
	Use3DHardware->SetChecked(true);
	ReverseStereo->SetChecked(false);
	OutputRate->SetSelectedItem(3); // Selects 44100Hz
}
