
#include "Precomp.h"
#include "VRSettingsPage.h"
#include "LauncherWindow.h"
#include "LauncherSettings.h"
#include "ScrollWidget.h"
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

	// Every control lives inside the scroll viewport, not on the page itself - the page is a fixed window
	// height and this content is taller than it. OnGeometryChanged keeps ScrollArea filling the page.
	ScrollArea = new ScrollWidget(this);
	Widget* container = ScrollArea->GetContainer();

	GeneralLabel = new TextLabel(container);
	EnableVR = new CheckboxLabel(container);
	RenderScaleLabel = new TextLabel(container);
	RenderScale = new LineEdit(container);

	MovementLabel = new TextLabel(container);
	MovementReferenceLabel = new TextLabel(container);
	MovementReferences = new Dropdown(container);
	MovementHandLabel = new TextLabel(container);
	MovementHand = new Dropdown(container);
	MovementDirectionHandLabel = new TextLabel(container);
	MovementDirectionHand = new Dropdown(container);
	DeadzoneLabel = new TextLabel(container);
	Deadzone = new LineEdit(container);
	RoomScaleMovement = new CheckboxLabel(container);

	TurningLabel = new TextLabel(container);
	TurnModeLabel = new TextLabel(container);
	TurnModes = new Dropdown(container);
	SnapTurnDegreesLabel = new TextLabel(container);
	SnapTurnDegrees = new LineEdit(container);
	SmoothTurnSpeedLabel = new TextLabel(container);
	SmoothTurnSpeed = new LineEdit(container);

	HandsHudLabel = new TextLabel(container);
	HudHandLabel = new TextLabel(container);
	HudHand = new Dropdown(container);
	MenuPointerHandLabel = new TextLabel(container);
	MenuPointerHand = new Dropdown(container);
	HandRadiusLabel = new TextLabel(container);
	HandRadius = new LineEdit(container);
	PickupHandsLabel = new TextLabel(container);
	PickupHands = new Dropdown(container);
	TabletWidthLabel = new TextLabel(container);
	TabletWidth = new LineEdit(container);
	TabletForearmOffsetLabel = new TextLabel(container);
	TabletForearmOffset = new LineEdit(container);
	TabletWristOffsetLabel = new TextLabel(container);
	TabletWristOffset = new LineEdit(container);

	WeaponLabel = new TextLabel(container);
	WeaponHandLabel = new TextLabel(container);
	WeaponHand = new Dropdown(container);
	WeaponPositionOffsetLabel = new TextLabel(container);
	WeaponForwardOffset = new LineEdit(container);
	WeaponRightOffset = new LineEdit(container);
	WeaponUpOffset = new LineEdit(container);
	WeaponRotationOffsetLabel = new TextLabel(container);
	WeaponPitchOffset = new LineEdit(container);
	WeaponYawOffset = new LineEdit(container);
	WeaponRollOffset = new LineEdit(container);
	WeaponScaleLabel = new TextLabel(container);
	WeaponScale = new LineEdit(container);

	WheelLabel = new TextLabel(container);
	WheelRadiusLabel = new TextLabel(container);
	WheelRadius = new LineEdit(container);
	WheelDeadzoneLabel = new TextLabel(container);
	WheelDeadzone = new LineEdit(container);
	WheelEntryScaleLabel = new TextLabel(container);
	WheelEntryScale = new LineEdit(container);
	WheelIconScaleLabel = new TextLabel(container);
	WheelIconScale = new LineEdit(container);

	ItemLabel = new TextLabel(container);
	ItemPositionOffsetLabel = new TextLabel(container);
	ItemForwardOffset = new LineEdit(container);
	ItemRightOffset = new LineEdit(container);
	ItemUpOffset = new LineEdit(container);
	ItemRotationOffsetLabel = new TextLabel(container);
	ItemPitchOffset = new LineEdit(container);
	ItemYawOffset = new LineEdit(container);
	ItemRollOffset = new LineEdit(container);
	ItemScaleLabel = new TextLabel(container);
	ItemScale = new LineEdit(container);

	ControlsLabel = new TextLabel(container);
	ControlsColumnLeft = new TextLabel(container);
	ControlsColumnRight = new TextLabel(container);
	for (int button = 0; button < VRSubsystem::ButtonCount; button++)
	{
		ButtonLabel[button] = new TextLabel(container);
		for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
			ButtonCommand[hand][button] = new LineEdit(container);
	}

	ResetButton = new PushButton(container);

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
	PickupHandsLabel->SetText("Hand(s) that can grab pickups by touch");
	TabletWidthLabel->SetText("HUD tablet width (cm)");
	TabletForearmOffsetLabel->SetText("HUD tablet offset up the forearm (cm)");
	TabletWristOffsetLabel->SetText("HUD tablet offset off the wrist (cm)");

	WeaponLabel->SetText("Weapon in hand:");
	WeaponHandLabel->SetText("Weapon hand (holds and aims the gun)");
	WeaponPositionOffsetLabel->SetText("Position offset forward / right / up (cm)");
	WeaponRotationOffsetLabel->SetText("Rotation trim pitch / yaw / roll (degrees)");
	WeaponScaleLabel->SetText("Weapon scale (% of the mesh's own size)");

	WheelLabel->SetText("Weapon/item wheel (hold A on a hand to open):");
	WheelRadiusLabel->SetText("Wheel radius (cm)");
	WheelDeadzoneLabel->SetText("Wheel select deadzone (cm of hand travel before a slot is chosen)");
	WheelEntryScaleLabel->SetText("Wheel weapon scale (% of the pickup mesh's own size)");
	WheelIconScaleLabel->SetText("Wheel item icon scale (% of a fixed reference size)");

	ItemLabel->SetText("Active item on the off hand:");
	ItemPositionOffsetLabel->SetText("Position offset forward / right / up (cm)");
	ItemRotationOffsetLabel->SetText("Rotation trim pitch / yaw / roll (degrees)");
	ItemScaleLabel->SetText("Item scale (% of the mesh's own size)");

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
	WeaponForwardOffset->SetIntrinsicSize(3);
	WeaponRightOffset->SetIntrinsicSize(3);
	WeaponUpOffset->SetIntrinsicSize(3);
	WeaponPitchOffset->SetIntrinsicSize(3);
	WeaponYawOffset->SetIntrinsicSize(3);
	WeaponRollOffset->SetIntrinsicSize(3);
	WeaponScale->SetIntrinsicSize(3);
	WheelRadius->SetIntrinsicSize(3);
	WheelDeadzone->SetIntrinsicSize(3);
	WheelEntryScale->SetIntrinsicSize(3);
	WheelIconScale->SetIntrinsicSize(3);
	ItemForwardOffset->SetIntrinsicSize(3);
	ItemRightOffset->SetIntrinsicSize(3);
	ItemUpOffset->SetIntrinsicSize(3);
	ItemPitchOffset->SetIntrinsicSize(3);
	ItemYawOffset->SetIntrinsicSize(3);
	ItemRollOffset->SetIntrinsicSize(3);
	ItemScale->SetIntrinsicSize(3);
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
	WeaponHand->AddItem("Left");
	WeaponHand->AddItem("Right");

	PickupHands->AddItem("Off");
	PickupHands->AddItem("Off-hand only");
	PickupHands->AddItem("Both hands");

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
	PickupHands->SetSelectedItem(settings.VR.PickupHands);
	TabletWidth->SetTextInt(settings.VR.HudTabletWidthCm);
	TabletForearmOffset->SetTextInt(settings.VR.HudTabletForearmOffsetCm);
	TabletWristOffset->SetTextInt(settings.VR.HudTabletWristOffsetCm);
	WeaponHand->SetSelectedItem((int)settings.VR.WeaponHand);
	WeaponForwardOffset->SetTextInt(settings.VR.WeaponForwardOffsetCm);
	WeaponRightOffset->SetTextInt(settings.VR.WeaponRightOffsetCm);
	WeaponUpOffset->SetTextInt(settings.VR.WeaponUpOffsetCm);
	WeaponPitchOffset->SetTextInt(settings.VR.WeaponPitchOffsetDegrees);
	WeaponYawOffset->SetTextInt(settings.VR.WeaponYawOffsetDegrees);
	WeaponRollOffset->SetTextInt(settings.VR.WeaponRollOffsetDegrees);
	WeaponScale->SetTextInt(settings.VR.WeaponScalePercent);
	WheelRadius->SetTextInt(settings.VR.WheelRadiusCm);
	WheelDeadzone->SetTextInt(settings.VR.WheelSelectDeadzoneCm);
	WheelEntryScale->SetTextInt(settings.VR.WheelEntryScalePercent);
	WheelIconScale->SetTextInt(settings.VR.WheelIconScalePercent);
	ItemForwardOffset->SetTextInt(settings.VR.ItemForwardOffsetCm);
	ItemRightOffset->SetTextInt(settings.VR.ItemRightOffsetCm);
	ItemUpOffset->SetTextInt(settings.VR.ItemUpOffsetCm);
	ItemPitchOffset->SetTextInt(settings.VR.ItemPitchOffsetDegrees);
	ItemYawOffset->SetTextInt(settings.VR.ItemYawOffsetDegrees);
	ItemRollOffset->SetTextInt(settings.VR.ItemRollOffsetDegrees);
	ItemScale->SetTextInt(settings.VR.ItemScalePercent);
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
	mainLayout->AddLayout(labelledRow(PickupHandsLabel, PickupHands));
	mainLayout->AddLayout(labelledRow(TabletWidthLabel, TabletWidth));
	mainLayout->AddLayout(labelledRow(TabletForearmOffsetLabel, TabletForearmOffset));
	mainLayout->AddLayout(labelledRow(TabletWristOffsetLabel, TabletWristOffset));

	mainLayout->AddWidget(WeaponLabel);
	mainLayout->AddLayout(labelledRow(WeaponHandLabel, WeaponHand));
	// Three edits per row (forward/right/up, then pitch/yaw/roll), the same compact multi-field layout the
	// controller-binding rows use, so the six offsets take two rows instead of six.
	auto weaponPositionRow = new HBoxLayout();
	weaponPositionRow->AddWidget(WeaponPositionOffsetLabel);
	weaponPositionRow->AddWidget(WeaponForwardOffset);
	weaponPositionRow->AddWidget(WeaponRightOffset);
	weaponPositionRow->AddWidget(WeaponUpOffset);
	weaponPositionRow->AddStretch();
	mainLayout->AddLayout(weaponPositionRow);
	auto weaponRotationRow = new HBoxLayout();
	weaponRotationRow->AddWidget(WeaponRotationOffsetLabel);
	weaponRotationRow->AddWidget(WeaponPitchOffset);
	weaponRotationRow->AddWidget(WeaponYawOffset);
	weaponRotationRow->AddWidget(WeaponRollOffset);
	weaponRotationRow->AddStretch();
	mainLayout->AddLayout(weaponRotationRow);
	mainLayout->AddLayout(labelledRow(WeaponScaleLabel, WeaponScale));

	mainLayout->AddWidget(WheelLabel);
	mainLayout->AddLayout(labelledRow(WheelRadiusLabel, WheelRadius));
	mainLayout->AddLayout(labelledRow(WheelDeadzoneLabel, WheelDeadzone));
	mainLayout->AddLayout(labelledRow(WheelEntryScaleLabel, WheelEntryScale));
	mainLayout->AddLayout(labelledRow(WheelIconScaleLabel, WheelIconScale));

	mainLayout->AddWidget(ItemLabel);
	auto itemPositionRow = new HBoxLayout();
	itemPositionRow->AddWidget(ItemPositionOffsetLabel);
	itemPositionRow->AddWidget(ItemForwardOffset);
	itemPositionRow->AddWidget(ItemRightOffset);
	itemPositionRow->AddWidget(ItemUpOffset);
	itemPositionRow->AddStretch();
	mainLayout->AddLayout(itemPositionRow);
	auto itemRotationRow = new HBoxLayout();
	itemRotationRow->AddWidget(ItemRotationOffsetLabel);
	itemRotationRow->AddWidget(ItemPitchOffset);
	itemRotationRow->AddWidget(ItemYawOffset);
	itemRotationRow->AddWidget(ItemRollOffset);
	itemRotationRow->AddStretch();
	mainLayout->AddLayout(itemRotationRow);
	mainLayout->AddLayout(labelledRow(ItemScaleLabel, ItemScale));

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

	container->SetLayout(mainLayout);
}

void VRSettingsPage::OnGeometryChanged()
{
	// Fill the page with the scroll viewport; it clips and scrolls the content within.
	if (ScrollArea)
		ScrollArea->SetFrameGeometry(Rect::xywh(0.0, 0.0, GetWidth(), GetHeight()));
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
	settings.VR.PickupHands = PickupHands->GetSelectedItem();
	int tabletWidth = TabletWidth->GetTextInt();
	if (tabletWidth > 0)
		settings.VR.HudTabletWidthCm = tabletWidth;
	// The offsets may legitimately be zero (tablet flush to the wrist), so take them as-is.
	settings.VR.HudTabletForearmOffsetCm = TabletForearmOffset->GetTextInt();
	settings.VR.HudTabletWristOffsetCm = TabletWristOffset->GetTextInt();

	settings.VR.WeaponHand = (WeaponHand->GetSelectedItem() == 1) ? VRHand::Right : VRHand::Left;
	// Every weapon offset is legitimately zero (weapon on the raw grip pose) and any of them can be
	// negative (trim the other way), so all six are taken exactly as typed.
	settings.VR.WeaponForwardOffsetCm = WeaponForwardOffset->GetTextInt();
	settings.VR.WeaponRightOffsetCm = WeaponRightOffset->GetTextInt();
	settings.VR.WeaponUpOffsetCm = WeaponUpOffset->GetTextInt();
	settings.VR.WeaponPitchOffsetDegrees = WeaponPitchOffset->GetTextInt();
	settings.VR.WeaponYawOffsetDegrees = WeaponYawOffset->GetTextInt();
	settings.VR.WeaponRollOffsetDegrees = WeaponRollOffset->GetTextInt();
	settings.VR.WeaponScalePercent = WeaponScale->GetTextInt();

	// Zero or negative would make the wheel unreachable (no radius) or unusable (a slot chosen the
	// instant the hand leaves centre), so unusable values keep the previous ones.
	int wheelRadius = WheelRadius->GetTextInt();
	if (wheelRadius > 0)
		settings.VR.WheelRadiusCm = wheelRadius;
	int wheelDeadzone = WheelDeadzone->GetTextInt();
	if (wheelDeadzone > 0)
		settings.VR.WheelSelectDeadzoneCm = wheelDeadzone;
	settings.VR.WheelEntryScalePercent = WheelEntryScale->GetTextInt();
	settings.VR.WheelIconScalePercent = WheelIconScale->GetTextInt();

	// Every item offset is legitimately zero and any of them can be negative, same as the weapon ones.
	settings.VR.ItemForwardOffsetCm = ItemForwardOffset->GetTextInt();
	settings.VR.ItemRightOffsetCm = ItemRightOffset->GetTextInt();
	settings.VR.ItemUpOffsetCm = ItemUpOffset->GetTextInt();
	settings.VR.ItemPitchOffsetDegrees = ItemPitchOffset->GetTextInt();
	settings.VR.ItemYawOffsetDegrees = ItemYawOffset->GetTextInt();
	settings.VR.ItemRollOffsetDegrees = ItemRollOffset->GetTextInt();
	settings.VR.ItemScalePercent = ItemScale->GetTextInt();

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
	PickupHands->SetSelectedItem(2);
	TabletWidth->SetTextInt(18);
	TabletForearmOffset->SetTextInt(18);
	TabletWristOffset->SetTextInt(4);
	WeaponHand->SetSelectedItem((int)VRHand::Right);
	WeaponForwardOffset->SetTextInt(0);
	WeaponRightOffset->SetTextInt(0);
	WeaponUpOffset->SetTextInt(0);
	WeaponPitchOffset->SetTextInt(0);
	WeaponYawOffset->SetTextInt(0);
	WeaponRollOffset->SetTextInt(0);
	WeaponScale->SetTextInt(500);
	WheelRadius->SetTextInt(20);
	WheelDeadzone->SetTextInt(4);
	WheelEntryScale->SetTextInt(6); // was left at a stale 500 through several in-headset scale corrections
	WheelIconScale->SetTextInt(36);
	ItemForwardOffset->SetTextInt(0);
	ItemRightOffset->SetTextInt(0);
	ItemUpOffset->SetTextInt(0);
	ItemPitchOffset->SetTextInt(0);
	ItemYawOffset->SetTextInt(0);
	ItemRollOffset->SetTextInt(0);
	ItemScale->SetTextInt(500);

	// The stock hand layout: fire on both triggers, weapon-cycle on both thumbstick clicks, alt-fire on
	// both trackpads, everything else unbound. Mirrors LauncherSettings' built-in default.
	const char* const defaults[VRSubsystem::ButtonCount] = { "Fire", "", "", "", "NextWeapon", "", "AltFire" };
	for (int button = 0; button < VRSubsystem::ButtonCount; button++)
		for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
			ButtonCommand[hand][button]->SetText(defaults[button]);
}
