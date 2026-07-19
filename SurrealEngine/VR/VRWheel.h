#pragma once

#include "Math/vec.h"
#include "Utils/Array.h"
#include "GC/GC.h"

class UInventory;
class UPlayerPawn;

// The weapon/item wheel (Half-Life: Alyx style): hold a hand's A button to fan out a circle of carried
// weapons (weapon hand) or activatable items (off hand) centred on that controller, move the hand toward
// one to highlight it, release to select it or release centred to cancel.
//
// A peer of VRHands, not a part of VRPlayerInput or RenderSubsystem: it is *driven* from
// VRPlayerInput::Tick (open/close on the A button, per-frame rebuild and highlight tracking) and *read* by
// RenderSubsystem (which draws it and owns no wheel state of its own). Folding it into either of those
// would give the input file a rendering-geometry responsibility or the render file an
// inventory-walking/script-call one.
class VRWheel
{
public:
	struct Entry
	{
		// Rooted because it is held across frames the wheel is open: a carried item can be destroyed
		// mid-wheel (picked-up-then-consumed, a level scripting edge case), and selecting into freed
		// memory would crash. Dropped on the next rebuild (RebuildEntries only re-adds live items) or on
		// Close().
		GCRoot<UInventory> Item;
		// Unit-length direction on the wheel plane this entry's slot sits along, i.e. Center + SlotForward
		// * radius is the entry's world position. Recomputed by RebuildEntries every open frame.
		vec3 SlotForward = vec3(1.0f, 0.0f, 0.0f);
	};

	bool IsOpen() const { return Active; }
	// Which controller opened the wheel - HandLeft/HandRight (VRSubsystem). Only that hand's A release
	// can commit or cancel it.
	int GetHand() const { return Hand; }
	// True = weapon wheel (weapon hand), false = item wheel (off hand). Selects both the entry predicate
	// (RebuildEntries) and the render style (meshes vs icon billboards).
	bool IsWeaponWheel() const { return Weapon; }
	const Array<Entry>& GetEntries() const { return Entries; }
	// Index into GetEntries(), or -1 for "hand centred - release cancels".
	int GetHighlighted() const { return Highlighted; }
	const vec3& GetCenter() const { return Center; }
	const vec3& GetPlaneNormal() const { return PlaneNormal; }
	const vec3& GetPlaneRight() const { return PlaneRight; }
	const vec3& GetPlaneUp() const { return PlaneUp; }

	// Called on A press for `hand`. Closes whatever wheel was already open (without committing it - a
	// single Open+Hand state makes the two wheels mutually exclusive for free), then opens the requested
	// one if the pawn actually carries anything of the relevant kind and the hand is tracked. Silently
	// does nothing otherwise (no pawn, menu up, untracked hand, empty inventory of that kind) - the press
	// just has no effect, the same as every other gate in this class.
	void OpenFor(int hand, UPlayerPawn* pawn, bool isWeaponHand);

	// Called every frame a wheel is open (see VRPlayerInput::Tick), whether or not this is the frame it
	// opened or is about to close on: rebuilds the entry list (a pickup/consume mid-wheel must be
	// reflected) and re-derives the highlighted entry from the hand's current displacement from Center.
	// Force-closes if the pawn/level/menu state stops being valid for a wheel to be up.
	void Tick(UPlayerPawn* pawn);

	// Called on A release for `hand`. A no-op unless a wheel opened by that same hand is currently open
	// (guards the case where a same-frame OpenFor on the other hand already replaced it). Commits the
	// highlighted entry - direct UWeapon*/UInventory* object, never the InventoryGroup byte - or cancels
	// if nothing is highlighted (centred release), then closes either way.
	void Commit(int hand, UPlayerPawn* pawn);

	// Force-closes without committing: menu opened, pawn/level gone, map change. Same effect as Commit()
	// on a centred wheel, just without touching PendingWeapon/SelectedItem at all.
	void Close();
	// Alias of Close(), named to match the vrHands->Reset() call sites this is wired in beside - a map
	// change must drop these GCRoots before the level they point into is torn down.
	void Reset() { Close(); }

private:
	void RebuildEntries(UPlayerPawn* pawn);
	void LayoutSlots();
	void BuildPlaneBasis();
	void UpdateHighlight(const vec3& currentGripPosition);

	bool Active = false;
	bool Weapon = false;
	int Hand = 0;

	// The disc's world transform, captured once at OpenFor and held for the whole session - re-deriving
	// PlaneNormal from the head every frame would spin the layout under the player's hand as they turned
	// their head to look at it.
	vec3 Center = vec3(0.0f);
	vec3 PlaneNormal = vec3(1.0f, 0.0f, 0.0f);
	vec3 PlaneRight = vec3(0.0f, 1.0f, 0.0f);
	vec3 PlaneUp = vec3(0.0f, 0.0f, 1.0f);

	Array<Entry> Entries;
	int Highlighted = -1;
};
