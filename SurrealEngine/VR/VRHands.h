#pragma once

#include "Math/vec.h"
#include "Utils/Array.h"
#include "GC/GC.h"
#include "VRSubsystem.h"

class UActor;
class UPlayerPawn;

// Gives the motion controllers a physical presence in the world: a ball collider on each hand that
// fires the same Touch/UnTouch/Bump events at Triggers and Movers the player's own body would, so
// proximity triggers fire when you reach into them and bump-buttons work by pushing them by hand.
//
// The events are dispatched with the *player pawn* as the touching actor, not with an actor of the
// hand's own. That is not a shortcut, it is forced by the games' own scripts: Trigger.IsRelevant gates
// the default TT_PlayerProximity on `Pawn(Other) != None && Pawn(Other).bIsPlayer`, and Mover's
// BumpButton/BumpOpenTimed states gate BT_PlayerBump the same way. A hand actor of its own class would
// be rejected by nearly every trigger and mover in both games, and a hand actor that subclassed
// PlayerPawn to get past those checks would then show up in PawnList, AllPawns iterators and bot
// targeting. Dispatching as the pawn keeps every script-visible consequence - Instigator, trigger
// messages, who gets credited - pointing at the player, which is what actually happened.
//
// The contacts are tracked here rather than through UActor::Touch, deliberately: the pawn's own
// Touching[] array is bookkeeping for where the pawn's cylinder is, and the engine untouches anything
// the pawn is not overlapping on its next move (see UActor::TryMove). Registering a hand's contact
// there would have the body untouch it a frame later while the hand was still inside, and the trigger
// would fire again on the next frame, and again, for as long as the player held their hand in it.
class VRHands
{
public:
	// Radius of the hand's collider, in Unreal units - a ball rather than a box, since a controller has
	// no meaningful facing for a box to be aligned to and the player aims it by feel. Also the radius the
	// hand is drawn at, so what the player sees is exactly what touches.
	static const float HandRadius;

	struct HandPose
	{
		// False when the controller is untracked, when VR is off, or whenever the hands are not in play
		// (no level, menu open) - i.e. also the flag for "do not draw this hand".
		bool Valid = false;
		vec3 Position = vec3(0.0f); // world space
		vec3 Forward = vec3(1.0f, 0.0f, 0.0f);
		vec3 Right = vec3(0.0f, 1.0f, 0.0f);
		vec3 Up = vec3(0.0f, 0.0f, 1.0f);
	};

	// Runs from Engine::Run after PlayerCalcView and before rendering, so the hands are placed against
	// the very camera the frame is about to be drawn from. Placing them during the input tick instead
	// would leave the collider a frame away from the ball the player is aiming with.
	void Tick();

	// Forgets every contact without dispatching anything. For map changes, where the actors we were
	// touching are about to stop existing and there is nobody left to tell.
	void Reset();

	const HandPose& GetHand(int hand) const { return Hands[hand]; }

private:
	// One trigger a hand is currently inside, held across frames so that entering it can be told from
	// staying in it, and so leaving it can be paired with the UnTouch that entering it owes.
	struct Contact
	{
		// Rooted because it is held across frames: a trigger can be destroyed while the hand is still
		// inside it (any bTriggerOnceOnly one) and dispatching UnTouch into freed memory would be a
		// crash. Rooting keeps it alive until we notice bDeleteMe and let go.
		GCRoot<UActor> Actor;
	};

	void UpdatePoses(VRSubsystem* vr);
	// Triggers: overlap the hand against them where it is now, and pair Touch with UnTouch.
	void UpdateTriggerContacts(int hand, UPlayerPawn* pawn);
	// Movers: sweep the hand along where it travelled since last frame, and bump what it ran into. A
	// separate path because a mover is a brush, which the overlap test refuses to answer for.
	void BumpMovers(int hand, UPlayerPawn* pawn);

	// Whether a hand should interact with this actor at all.
	static bool IsTriggerLike(UActor* actor);
	// Whether the player's own cylinder is already inside this actor, in which case the body owns the
	// touch and the hand must keep out of it or every trigger the player stands in fires twice.
	static bool IsBodyTouching(UPlayerPawn* pawn, UActor* actor);

	HandPose Hands[VRSubsystem::HandCount];
	Array<Contact> Contacts[VRSubsystem::HandCount];

	// Where each hand was last frame, i.e. what this frame's mover sweep starts from. Invalid on the
	// first frame a hand appears and after tracking drops, where there is no travel to sweep.
	vec3 PreviousPosition[VRSubsystem::HandCount] = {};
	bool HasPreviousPosition[VRSubsystem::HandCount] = {};
};
