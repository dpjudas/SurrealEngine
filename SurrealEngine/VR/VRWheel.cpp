
#include "Precomp.h"
#include "VRWheel.h"
#include "VRHands.h"
#include "Engine.h"
#include "UObject/UActor.h"
#include "UObject/UClient.h" // UViewport, which Engine.h only forward declares
#include "VM/ScriptCall.h"
#include "LauncherSettings.h"
#include <cmath>

void VRWheel::OpenFor(int hand, UPlayerPawn* pawn, bool isWeaponHand)
{
	// Opening one wheel while the other is up replaces it without committing - a single Open+Hand state
	// makes the two mutually exclusive for free.
	Close();

	if (!pawn || !pawn->XLevel() || pawn->bDeleteMe())
		return;
	if (engine->console && engine->console->bNoDrawWorld())
		return; // pause/main menu has the screen

	VRHands* hands = engine->vrHands.get();
	const VRHands::HandPose* pose = hands ? &hands->GetHand(hand) : nullptr;
	if (!pose || !pose->Valid)
		return; // untracked hand: nowhere to centre the wheel on

	Weapon = isWeaponHand;
	Hand = hand;
	Center = pose->GripPosition;
	BuildPlaneBasis();

	RebuildEntries(pawn);
	if (Entries.empty())
	{
		Entries.clear();
		return; // nothing of the relevant kind carried - the press does nothing, same as a centred release
	}

	Highlighted = -1;
	Active = true;

	if (engine->vr)
		engine->vr->Haptic(Hand, 0.5f); // one runtime-minimum tick, "a wheel opened"
}

void VRWheel::Tick(UPlayerPawn* pawn)
{
	if (!Active)
		return;

	if (!pawn || !pawn->XLevel() || pawn->bDeleteMe() || (engine->console && engine->console->bNoDrawWorld()))
	{
		Close();
		return;
	}

	// Rebuild first: a pickup or consume mid-wheel must be reflected in what can be highlighted this
	// frame, before the highlight itself is recomputed below.
	RebuildEntries(pawn);

	VRHands* hands = engine->vrHands.get();
	const VRHands::HandPose* pose = hands ? &hands->GetHand(Hand) : nullptr;
	if (!pose || !pose->Valid)
	{
		// The hand went untracked mid-wheel. Fall back to "centred" rather than holding the last good
		// direction - a stale highlight the player can no longer see or correct would be worse than none.
		Highlighted = -1;
		return;
	}

	UpdateHighlight(pose->GripPosition);
}

void VRWheel::Commit(int hand, UPlayerPawn* pawn)
{
	if (!Active || Hand != hand)
		return; // not this hand's wheel to commit (the other hand's press already replaced it this frame)

	bool wasWeapon = Weapon;
	UInventory* selected = (Highlighted >= 0 && Highlighted < (int)Entries.size()) ? Entries[Highlighted].Item.get() : nullptr;

	Close();

	if (!pawn || !selected || selected->bDeleteMe())
		return; // cancelled (centred release), or the entry died between highlight and release

	if (wasWeapon)
	{
		UWeapon* w = UObject::TryCast<UWeapon>(selected);
		if (!w || w == pawn->Weapon())
			return; // already held weapon selected again - a no-op, not a PutDown of yourself

		// Mirrors the tail of PlayerPawn.SwitchWeapon (UT99's ordering, decompile-verified safe for both
		// Unreal Gold and UT99 - see the phase-6 plan §9), substituting the exact object we hold for its
		// by-InventoryGroup lookup. That substitution is what makes two weapons sharing a group both
		// reachable through the wheel, which SwitchWeapon's own lookup can't tell apart.
		pawn->PendingWeapon() = w;
		if (pawn->Weapon() == nullptr)
		{
			// No weapon out: nothing to put down, bring this one up immediately.
			CallEvent(pawn, "ChangedWeapon");
		}
		else if (!CallEvent(pawn->Weapon(), "PutDown").ToBool())
		{
			// PutDown() declined (mid-animation) - abort the switch rather than leave a pending weapon
			// that nothing will ever pick up (ChangedWeapon only runs off DownWeapon's finished anim).
			pawn->PendingWeapon() = nullptr;
		}
		// Otherwise PutDown() has only *initiated* the animated holster; its DownWeapon state calls
		// Pawn.ChangedWeapon() itself a few ticks later, which reads PendingWeapon and finishes the swap.
		// Nothing here waits for or drives that - the state machine completes it on its own.
	}
	else
	{
		// A bare writable pointer - PlayerPawn.PrevItem/ActivateItem read and assign it the same way.
		// Setting it *is* the selection; no script call, no async, no ambiguity.
		pawn->SelectedItem() = selected;
	}
}

void VRWheel::Close()
{
	Active = false;
	Entries.clear();
	Highlighted = -1;
}

void VRWheel::BuildPlaneBasis()
{
	// engine->CameraLocation, not vr->GetHead().Position: the head pose is documented as relative to the
	// play space anchor (VRSubsystem::HeadPose), while Center (a VRHands grip position) is already world
	// space (VRHands::UpdatePoses adds CameraLocation on) - subtracting one from the other mixed two
	// different origins and pointed the disc in a near-arbitrary direction instead of at the player. This
	// is exactly what VRHands/RenderSubsystem already treat as "the player's eye in world space".
	vec3 toHead = engine->CameraLocation - Center;
	float lenSq = dot(toHead, toHead);
	// Degenerate only if the hand is essentially at the eye, which a real arm's length never is - a fixed
	// fallback facing is fine since nothing will be visibly wrong about it.
	PlaneNormal = (lenSq > 0.01f) ? toHead / std::sqrt(lenSq) : vec3(-1.0f, 0.0f, 0.0f);

	// Right/up spanning the disc, referenced off world-up so entries lay out level regardless of how the
	// hand happened to be rolled when the wheel opened.
	vec3 worldUp(0.0f, 0.0f, 1.0f);
	if (std::fabs(dot(PlaneNormal, worldUp)) > 0.99f)
		worldUp = vec3(1.0f, 0.0f, 0.0f); // looking straight up/down: fall back to world X as the reference
	PlaneRight = normalize(cross(worldUp, PlaneNormal));
	PlaneUp = cross(PlaneNormal, PlaneRight);
}

void VRWheel::RebuildEntries(UPlayerPawn* pawn)
{
	Entries.clear();

	// Defensive cap so a pathological inventory can't blow the layout; no UE1 game's inventory chain gets
	// remotely close to this.
	const size_t MaxEntries = 16;
	for (UInventory* item = pawn->Inventory(); item && Entries.size() < MaxEntries; item = item->Inventory())
	{
		if (item->bDeleteMe())
			continue;

		bool isWeapon = UObject::TryCast<UWeapon>(item) != nullptr;
		// Weapon wheel: exactly the weapons. Item wheel: non-weapon inventory that is bActivatable -
		// decompile-verified as the predicate Inventory.SelectNext()/Activate() themselves use, not
		// bDisplayableInv (that flag only means "show in the HUD bar"). See the phase-6 plan §9.
		bool keep = Weapon ? isWeapon : (!isWeapon && item->bActivatable());
		if (!keep)
			continue;

		Entry entry;
		entry.Item.set(item);
		Entries.push_back(std::move(entry));
	}

	LayoutSlots();
}

void VRWheel::LayoutSlots()
{
	size_t count = Entries.size();
	for (size_t i = 0; i < count; i++)
	{
		// Top-centre (+PlaneUp) is the first entry, going clockwise (toward +PlaneRight) as the index
		// increases.
		float angle = radians(360.0f * (float)i / (float)count);
		Entries[i].SlotForward = PlaneUp * std::cos(angle) + PlaneRight * std::sin(angle);
	}
}

void VRWheel::UpdateHighlight(const vec3& currentGripPosition)
{
	vec3 delta3 = currentGripPosition - Center;
	vec3 delta = delta3 - PlaneNormal * dot(delta3, PlaneNormal); // projected onto the wheel plane

	const float deadzoneUU = LauncherSettings::Get().VR.WheelSelectDeadzoneCm * 0.01f * MetersToUnrealUnits;

	int newHighlighted = -1;
	if (!Entries.empty() && dot(delta, delta) >= deadzoneUU * deadzoneUU)
	{
		vec3 dir = normalize(delta);
		float bestDot = -2.0f;
		for (size_t i = 0; i < Entries.size(); i++)
		{
			float d = dot(dir, Entries[i].SlotForward);
			if (d > bestDot)
			{
				bestDot = d;
				newHighlighted = (int)i;
			}
		}
	}

	if (newHighlighted != Highlighted)
	{
		if (newHighlighted >= 0 && engine->vr)
			engine->vr->Haptic(Hand, 0.5f); // one tick per crossing, the UpdateFireHaptics pattern
		Highlighted = newHighlighted;
	}
}
