#pragma once

#include <surrealwidgets/core/widget.h>

class LauncherWindow;
class CheckboxLabel;
class RadioButtonLabel;
class RadioGroup;
class PushButton;
class Dropdown;
class TextLabel;

class AudioSettingsPage : public Widget
{
public:
    AudioSettingsPage(Widget* parent);

    void Save() const;

private:
    void OnResetButtonClicked();

    LauncherWindow* launcher = nullptr;

    RadioGroup* AudioDeviceGroup = nullptr;

    RadioButtonLabel* OpenALDevice = nullptr;
    RadioButtonLabel* AudioPlayerDevice = nullptr;
    RadioButtonLabel* NullAudioDevice = nullptr;

    CheckboxLabel* UseFilter = nullptr;
    CheckboxLabel* UseSurround = nullptr;
    CheckboxLabel* UseStereo = nullptr;
    CheckboxLabel* UseSpatial = nullptr;
    CheckboxLabel* UseReverb = nullptr;
    CheckboxLabel* Use3DHardware = nullptr;
    CheckboxLabel* ReverseStereo = nullptr;

    TextLabel* OutputRateLabel = nullptr;
    Dropdown* OutputRate = nullptr;

    PushButton* ResetButton = nullptr;
};
