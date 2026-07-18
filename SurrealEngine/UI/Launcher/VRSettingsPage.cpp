
#include "Precomp.h"
#include "VRSettingsPage.h"
#include "LauncherWindow.h"
#include "LauncherSettings.h"
#include <surrealwidgets/widgets/textlabel/textlabel.h>
#include <surrealwidgets/widgets/checkboxlabel/checkboxlabel.h>
#include <surrealwidgets/widgets/lineedit/lineedit.h>
#include <surrealwidgets/widgets/dropdown/dropdown.h>
#include <surrealwidgets/widgets/pushbutton/pushbutton.h>
#include <surrealwidgets/widgets/layout/vboxlayout.h>
#include <surrealwidgets/widgets/layout/hboxlayout.h>
#include <algorithm>
#include <cmath>

namespace
{
	// Human-readable names for each controller button, in VRSubsystem::Button order. A/B carry their Touch
	// aliases (X/Y on the left hand there) so the label fits both controller families.
	const char* const ButtonDisplayNames[VRSubsystem::ButtonCount] =
	{
		"Trigger", "Grip", "A / X", "B / Y", "Thumbstick click", "Menu", "Trackpad"
	};
}

VRSettingsPage::VRSettingsPage(Widget* parent)
	: Widget(parent)
{
	auto& settings = LauncherSettings::Get();

	GeneralLabel = new TextLabel(this);
	EnableVR = new CheckboxLabel(this);
	RenderScaleLabel = new TextLabel(this);
	RenderScale = new LineEdit(this);

	MovementLabel = new TextLabel(this);
	MovementReferenceLabel = new TextLabel(this);
	MovementReferences = new Dropdown(this);
	MovementHandLabel = new TextLabel(this);
	MovementHand = new Dropdown(this);
	MovementDirectionHandLabel = new TextLabel(this);
	MovementDirectionHand = new Dropdown(this);
	DeadzoneLabel = new TextLabel(this);
	Deadzone = new LineEdit(this);
	RoomScaleMovement = new CheckboxLabel(this);

	TurningLabel = new TextLabel(this);
	TurnModeLabel = new TextLabel(this);
	TurnModes = new Dropdown(this);
	SnapTurnDegreesLabel = new TextLabel(this);
	SnapTurnDegrees = new LineEdit(this);
	SmoothTurnSpeedLabel = new TextLabel(this);
	SmoothTurnSpeed = new LineEdit(this);

	HandsHudLabel = new TextLabel(this);
	HudHandLabel = new TextLabel(this);
	HudHand = new Dropdown(this);
	MenuPointerHandLabel = new TextLabel(this);
	MenuPointerHand = new Dropdown(this);
	HandRadiusLabel = new TextLabel(this);
	HandRadius = new LineEdit(this);
	TabletWidthLabel = new TextLabel(this);
	TabletWidth = new LineEdit(this);
	TabletForearmOffsetLabel = new TextLabel(this);
	TabletForearmOffset = new LineEdit(this);
	TabletWristOffsetLabel = new TextLabel(this);
	TabletWristOffset = new LineEdit(this);

	ControlsLabel = new TextLabel(this);
	ControlsColumnLeft = new TextLabel(this);
	ControlsColumnRight = new TextLabel(this);
	for (int button = 0; button < VRSubsystem::ButtonCount; button++)
	{
		ButtonLabel[button] = new TextLabel(this);
		for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
			ButtonCommand[hand][button] = new LineEdit(this);
	}

	ResetButton = new PushButton(this);

	GeneralLabel->SetText("General:");
#ifdef USE_OPENXR
	EnableVR->SetText("Enable VR (experimental, requires Vulkan + a running SteamVR/OpenXR runtime)");
#else
	EnableVR->SetText("Enable VR (not available - this build was compiled without OpenXR support)");
	EnableVR->SetDisabled(true);
#endif
	RenderScaleLabel->SetText("Render scale (% of headset's recommended resolution)");

	MovementLabel->SetText("Movement:");
	MovementReferenceLabel->SetText("Move in the direction of");
	MovementHandLabel->SetText("Movement thumbstick (the other hand turns)");
	MovementDirectionHandLabel->SetText("Controller that points the way (controller mode only)");
	DeadzoneLabel->SetText("Thumbstick deadzone (%)");
	RoomScaleMovement->SetText("Walk the player around to stay under the headset (room scale)");

	TurningLabel->SetText("Turning:");
	TurnModeLabel->SetText("Turning");
	SnapTurnDegreesLabel->SetText("Snap turn angle (degrees)");
	SmoothTurnSpeedLabel->SetText("Smooth turn speed (degrees/second)");

	HandsHudLabel->SetText("Hands & HUD:");
	HudHandLabel->SetText("HUD tablet hand");
	MenuPointerHandLabel->SetText("Menu pointer hand (points the laser, B toggles the menu)");
	HandRadiusLabel->SetText("Hand collider radius (Unreal units)");
	TabletWidthLabel->SetText("HUD tablet width (cm)");
	TabletForearmOffsetLabel->SetText("HUD tablet offset up the forearm (cm)");
	TabletWristOffsetLabel->SetText("HUD tablet offset off the wrist (cm)");

	ControlsLabel->SetText("Controller buttons (any command or alias; leave blank to unbind):");
	ControlsColumnLeft->SetText("Left hand");
	ControlsColumnRight->SetText("Right hand");
	for (int button = 0; button < VRSubsystem::ButtonCount; button++)
		ButtonLabel[button]->SetText(ButtonDisplayNames[button]);

	ResetButton->SetText("Reset to defaults");

	RenderScale->SetIntrinsicSize(3);
	Deadzone->SetIntrinsicSize(3);
	SnapTurnDegrees->SetIntrinsicSize(3);
	SmoothTurnSpeed->SetIntrinsicSize(3);
	HandRadius->SetIntrinsicSize(3);
	TabletWidth->SetIntrinsicSize(3);
	TabletForearmOffset->SetIntrinsicSize(3);
	TabletWristOffset->SetIntrinsicSize(3);
	for (int button = 0; button < VRSubsystem::ButtonCount; button++)
		for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
			ButtonCommand[hand][button]->SetIntrinsicSize(12);

	// Order must match VRMovementReference/VRHand/VRTurnMode - both directions go through
	// GetSelectedItem()/SetSelectedItem() as a plain index.
	MovementReferences->AddItem("The controller (point where you want to go)");
	MovementReferences->AddItem("The headset (look where you want to go)");

	MovementHand->AddItem("Left");
	MovementHand->AddItem("Right");
	MovementDirectionHand->AddItem("Left");
	MovementDirectionHand->AddItem("Right");
	HudHand->AddItem("Left");
	HudHand->AddItem("Right");
	MenuPointerHand->AddItem("Left");
	MenuPointerHand->AddItem("Right");

	TurnModes->AddItem("Snap");
	TurnModes->AddItem("Smooth");
	TurnModes->AddItem("Off (mouse only)");

	EnableVR->SetChecked(settings.VR.Enabled);
	RenderScale->SetTextInt(settings.VR.RenderScale);
	MovementReferences->SetSelectedItem((int)settings.VR.MovementReference);
	MovementHand->SetSelectedItem((int)settings.VR.MovementHand);
	MovementDirectionHand->SetSelectedItem((int)settings.VR.MovementDirectionHand);
	Deadzone->SetTextInt((int)std::lround(settings.VR.StickDeadzone * 100.0f));
	RoomScaleMovement->SetChecked(settings.VR.RoomScaleMovement);
	TurnModes->SetSelectedItem((int)settings.VR.TurnMode);
	SnapTurnDegrees->SetTextInt(settings.VR.SnapTurnDegrees);
	SmoothTurnSpeed->SetTextInt(settings.VR.SmoothTurnDegreesPerSecond);
	HudHand->SetSelectedItem((int)settings.VR.HudHand);
	MenuPointerHand->SetSelectedItem((int)settings.VR.MenuPointerHand);
	HandRadius->SetTextInt(settings.VR.HandColliderRadius);
	TabletWidth->SetTextInt(settings.VR.HudTabletWidthCm);
	TabletForearmOffset->SetTextInt(settings.VR.HudTabletForearmOffsetCm);
	TabletWristOffset->SetTextInt(settings.VR.HudTabletWristOffsetCm);
	for (int button = 0; button < VRSubsystem::ButtonCount; button++)
		for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
			ButtonCommand[hand][button]->SetText(settings.VR.ButtonCommands[hand][button]);

	ResetButton->OnClick = [this]() { OnResetButtonClicked(); };

	auto labelledRow = [](TextLabel* label, Widget* field)
	{
		auto row = new HBoxLayout();
		row->AddWidget(label);
		row->AddWidget(field);
		row->AddStretch();
		return row;
	};

	auto mainLayout = new VBoxLayout();

	mainLayout->AddWidget(GeneralLabel);
	mainLayout->AddWidget(EnableVR);
	mainLayout->AddLayout(labelledRow(RenderScaleLabel, RenderScale));

	mainLayout->AddWidget(MovementLabel);
	mainLayout->AddLayout(labelledRow(MovementReferenceLabel, MovementReferences));
	mainLayout->AddLayout(labelledRow(MovementHandLabel, MovementHand));
	mainLayout->AddLayout(labelledRow(MovementDirectionHandLabel, MovementDirectionHand));
	mainLayout->AddLayout(labelledRow(DeadzoneLabel, Deadzone));
	mainLayout->AddWidget(RoomScaleMovement);

	mainLayout->AddWidget(TurningLabel);
	mainLayout->AddLayout(labelledRow(TurnModeLabel, TurnModes));
	mainLayout->AddLayout(labelledRow(SnapTurnDegreesLabel, SnapTurnDegrees));
	mainLayout->AddLayout(labelledRow(SmoothTurnSpeedLabel, SmoothTurnSpeed));

	mainLayout->AddWidget(HandsHudLabel);
	mainLayout->AddLayout(labelledRow(HudHandLabel, HudHand));
	mainLayout->AddLayout(labelledRow(MenuPointerHandLabel, MenuPointerHand));
	mainLayout->AddLayout(labelledRow(HandRadiusLabel, HandRadius));
	mainLayout->AddLayout(labelledRow(TabletWidthLabel, TabletWidth));
	mainLayout->AddLayout(labelledRow(TabletForearmOffsetLabel, TabletForearmOffset));
	mainLayout->AddLayout(labelledRow(TabletWristOffsetLabel, TabletWristOffset));

	mainLayout->AddWidget(ControlsLabel);
	// A column-header row (spacer where the per-button name sits, then Left / Right over the two edits),
	// then one row per button: name + its left-hand and right-hand command.
	auto controlsHeader = new HBoxLayout();
	controlsHeader->AddWidget(ControlsColumnLeft);
	controlsHeader->AddWidget(ControlsColumnRight);
	controlsHeader->AddStretch();
	mainLayout->AddLayout(controlsHeader);
	for (int button = 0; button < VRSubsystem::ButtonCount; button++)
	{
		auto row = new HBoxLayout();
		row->AddWidget(ButtonLabel[button]);
		row->AddWidget(ButtonCommand[VRSubsystem::HandLeft][button]);
		row->AddWidget(ButtonCommand[VRSubsystem::HandRight][button]);
		row->AddStretch();
		mainLayout->AddLayout(row);
	}

	auto resetButtonLayout = new HBoxLayout();
	resetButtonLayout->AddWidget(ResetButton);
	resetButtonLayout->AddStretch();
	mainLayout->AddLayout(resetButtonLayout);

	mainLayout->AddStretch();

	SetLayout(mainLayout);
}

void VRSettingsPage::Save()
{
	auto& settings = LauncherSettings::Get();

	// Requires Vulkan, which the Video tab owns. This page runs its Save after that one (see
	// LauncherWindow::Save), so settings.RenderDevice.Type is already the freshly-chosen value here.
	settings.VR.Enabled = EnableVR->GetChecked() && settings.RenderDevice.Type == RenderDeviceType::Vulkan;

	// Clamped again on the way into OpenXRSubsystem, but keep the file itself sane.
	int renderScale = RenderScale->GetTextInt();
	if (renderScale > 0)
		settings.VR.RenderScale = std::max(std::min(renderScale, 200), 10);

	settings.VR.MovementReference = (MovementReferences->GetSelectedItem() == 1) ? VRMovementReference::Head : VRMovementReference::Controller;
	settings.VR.MovementHand = (MovementHand->GetSelectedItem() == 1) ? VRHand::Right : VRHand::Left;
	settings.VR.MovementDirectionHand = (MovementDirectionHand->GetSelectedItem() == 1) ? VRHand::Right : VRHand::Left;

	// A deadzone of 100% would pin the stick centred (no movement) while the field still claimed a
	// percentage, so an out-of-range value keeps the previous one rather than being stored.
	int deadzonePercent = Deadzone->GetTextInt();
	if (deadzonePercent >= 0 && deadzonePercent < 100)
		settings.VR.StickDeadzone = deadzonePercent / 100.0f;

	settings.VR.RoomScaleMovement = RoomScaleMovement->GetChecked();

	if (TurnModes->GetSelectedItem() == 0)
		settings.VR.TurnMode = VRTurnMode::Snap;
	else if (TurnModes->GetSelectedItem() == 1)
		settings.VR.TurnMode = VRTurnMode::Smooth;
	else if (TurnModes->GetSelectedItem() == 2)
		settings.VR.TurnMode = VRTurnMode::Off;

	// Zero or negative would silently mean "no turning" while the dropdown still claimed otherwise, so
	// an unusable value keeps the previous one rather than being stored.
	int snapTurnDegrees = SnapTurnDegrees->GetTextInt();
	if (snapTurnDegrees > 0)
		settings.VR.SnapTurnDegrees = std::min(snapTurnDegrees, 180);
	int smoothTurnSpeed = SmoothTurnSpeed->GetTextInt();
	if (smoothTurnSpeed > 0)
		settings.VR.SmoothTurnDegreesPerSecond = std::min(smoothTurnSpeed, 720);

	settings.VR.HudHand = (HudHand->GetSelectedItem() == 1) ? VRHand::Right : VRHand::Left;
	settings.VR.MenuPointerHand = (MenuPointerHand->GetSelectedItem() == 1) ? VRHand::Right : VRHand::Left;

	// Zero or negative would give a hand that touches nothing and draws as a dot, or a zero-area tablet, so
	// an unusable value keeps the previous one.
	int handRadius = HandRadius->GetTextInt();
	if (handRadius > 0)
		settings.VR.HandColliderRadius = handRadius;
	int tabletWidth = TabletWidth->GetTextInt();
	if (tabletWidth > 0)
		settings.VR.HudTabletWidthCm = tabletWidth;
	// The offsets may legitimately be zero (tablet flush to the wrist), so take them as-is.
	settings.VR.HudTabletForearmOffsetCm = TabletForearmOffset->GetTextInt();
	settings.VR.HudTabletWristOffsetCm = TabletWristOffset->GetTextInt();

	for (int button = 0; button < VRSubsystem::ButtonCount; button++)
		for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
			settings.VR.ButtonCommands[hand][button] = ButtonCommand[hand][button]->GetText();
}

void VRSettingsPage::OnResetButtonClicked()
{
	EnableVR->SetChecked(false);
	RenderScale->SetTextInt(60);
	MovementReferences->SetSelectedItem((int)VRMovementReference::Controller);
	MovementHand->SetSelectedItem((int)VRHand::Left);
	MovementDirectionHand->SetSelectedItem((int)VRHand::Left);
	Deadzone->SetTextInt(15);
	RoomScaleMovement->SetChecked(true);
	TurnModes->SetSelectedItem((int)VRTurnMode::Snap);
	SnapTurnDegrees->SetTextInt(45);
	SmoothTurnSpeed->SetTextInt(90);
	HudHand->SetSelectedItem((int)VRHand::Left);
	MenuPointerHand->SetSelectedItem((int)VRHand::Right);
	HandRadius->SetTextInt(6);
	TabletWidth->SetTextInt(18);
	TabletForearmOffset->SetTextInt(18);
	TabletWristOffset->SetTextInt(4);

	// The stock hand layout: fire on both triggers, weapon-cycle on both thumbstick clicks, alt-fire on
	// both trackpads, everything else unbound. Mirrors LauncherSettings' built-in default.
	const char* const defaults[VRSubsystem::ButtonCount] = { "Fire", "", "", "", "NextWeapon", "", "AltFire" };
	for (int button = 0; button < VRSubsystem::ButtonCount; button++)
		for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
			ButtonCommand[hand][button]->SetText(defaults[button]);
}
