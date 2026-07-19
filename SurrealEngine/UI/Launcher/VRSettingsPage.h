#pragma once

#include <surrealwidgets/core/widget.h>
#include "VR/VRSubsystem.h" // VRSubsystem::HandCount / ButtonCount, for the per-button binding grid

class LauncherWindow;
class TextLabel;
class PushButton;
class CheckboxLabel;
class LineEdit;
class Dropdown;
class ScrollWidget;

// All the virtual-reality settings, split off from the Video tab into their own tab so neither page is a
// wall of controls. Everything here maps onto LauncherSettings::VR; the engine reads those live, so a
// change takes effect on the next launch. See VRPlayerInput / RenderSubsystem / VRHands for the consumers.
class VRSettingsPage : public Widget
{
public:
	VRSettingsPage(Widget* parent);

	void Save();

protected:
	// The page is too tall for the window, so its whole content lives inside ScrollArea; keep that filling
	// the page as it resizes.
	void OnGeometryChanged() override;

private:
	void OnResetButtonClicked();

	LauncherWindow* Launcher = nullptr;

	// Scroll viewport holding every control below. They are parented to ScrollArea->GetContainer(), not to
	// the page directly, so the page just sizes this to fill itself.
	ScrollWidget* ScrollArea = nullptr;

	TextLabel* GeneralLabel = nullptr;
	CheckboxLabel* EnableVR = nullptr;
	TextLabel* RenderScaleLabel = nullptr;
	LineEdit* RenderScale = nullptr;

	TextLabel* MovementLabel = nullptr;
	TextLabel* MovementReferenceLabel = nullptr;
	Dropdown* MovementReferences = nullptr;
	TextLabel* MovementHandLabel = nullptr;
	Dropdown* MovementHand = nullptr;
	TextLabel* MovementDirectionHandLabel = nullptr;
	Dropdown* MovementDirectionHand = nullptr;
	TextLabel* DeadzoneLabel = nullptr;
	LineEdit* Deadzone = nullptr;
	CheckboxLabel* RoomScaleMovement = nullptr;

	TextLabel* TurningLabel = nullptr;
	TextLabel* TurnModeLabel = nullptr;
	Dropdown* TurnModes = nullptr;
	TextLabel* SnapTurnDegreesLabel = nullptr;
	LineEdit* SnapTurnDegrees = nullptr;
	TextLabel* SmoothTurnSpeedLabel = nullptr;
	LineEdit* SmoothTurnSpeed = nullptr;

	TextLabel* HandsHudLabel = nullptr;
	TextLabel* HudHandLabel = nullptr;
	Dropdown* HudHand = nullptr;
	TextLabel* MenuPointerHandLabel = nullptr;
	Dropdown* MenuPointerHand = nullptr;
	TextLabel* HandRadiusLabel = nullptr;
	LineEdit* HandRadius = nullptr;
	TextLabel* TabletWidthLabel = nullptr;
	LineEdit* TabletWidth = nullptr;
	TextLabel* TabletForearmOffsetLabel = nullptr;
	LineEdit* TabletForearmOffset = nullptr;
	TextLabel* TabletWristOffsetLabel = nullptr;
	LineEdit* TabletWristOffset = nullptr;

	TextLabel* WeaponLabel = nullptr;
	TextLabel* WeaponHandLabel = nullptr;
	Dropdown* WeaponHand = nullptr;
	// The six placement offsets sit on two compact rows (position F/R/U, rotation P/Y/R) rather than six
	// labelled rows, to keep the already-long VR page from overflowing the window.
	TextLabel* WeaponPositionOffsetLabel = nullptr;
	LineEdit* WeaponForwardOffset = nullptr;
	LineEdit* WeaponRightOffset = nullptr;
	LineEdit* WeaponUpOffset = nullptr;
	TextLabel* WeaponRotationOffsetLabel = nullptr;
	LineEdit* WeaponPitchOffset = nullptr;
	LineEdit* WeaponYawOffset = nullptr;
	LineEdit* WeaponRollOffset = nullptr;
	TextLabel* WeaponScaleLabel = nullptr;
	LineEdit* WeaponScale = nullptr;

	TextLabel* WheelLabel = nullptr;
	TextLabel* WheelRadiusLabel = nullptr;
	LineEdit* WheelRadius = nullptr;
	TextLabel* WheelDeadzoneLabel = nullptr;
	LineEdit* WheelDeadzone = nullptr;
	TextLabel* WheelEntryScaleLabel = nullptr;
	LineEdit* WheelEntryScale = nullptr;

	TextLabel* ItemLabel = nullptr;
	TextLabel* ItemPositionOffsetLabel = nullptr;
	LineEdit* ItemForwardOffset = nullptr;
	LineEdit* ItemRightOffset = nullptr;
	LineEdit* ItemUpOffset = nullptr;
	TextLabel* ItemRotationOffsetLabel = nullptr;
	LineEdit* ItemPitchOffset = nullptr;
	LineEdit* ItemYawOffset = nullptr;
	LineEdit* ItemRollOffset = nullptr;
	TextLabel* ItemScaleLabel = nullptr;
	LineEdit* ItemScale = nullptr;

	TextLabel* ControlsLabel = nullptr;
	TextLabel* ControlsColumnLeft = nullptr;
	TextLabel* ControlsColumnRight = nullptr;
	TextLabel* ButtonLabel[VRSubsystem::ButtonCount] = {};
	// [hand][button] in VRSubsystem::Button order; free-text commands, empty means unbound.
	LineEdit* ButtonCommand[VRSubsystem::HandCount][VRSubsystem::ButtonCount] = {};

	PushButton* ResetButton = nullptr;
};
