
#include "Precomp.h"
#include "VRHands.h"
#include "VRPlayerInput.h"
#include "Engine.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "UObject/UClient.h" // UViewport, which Engine.h only forward declares
#include "Collision/TopLevel/CollisionSystem.h"
#include "Collision/TopLevel/CollisionHit.h"
#include "VM/ScriptCall.h"
#include "Math/coords.h"

// Roughly a fist. Big enough that the player can hit a wall button without having to thread a point
// through it, small enough that reaching past one trigger does not set off the one beside it.
const float VRHands::HandRadius = 6.0f;

namespace
{
	bool Contains(const Array<UActor*>& actors, UActor* actor)
	{
		for (UActor* a : actors)
		{
			if (a == actor)
				return true;
		}
		return false;
	}
}

void VRHands::Reset()
{
	for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
	{
		Contacts[hand].clear();
		Hands[hand] = HandPose();
		HasPreviousPosition[hand] = false;
	}
}

bool VRHands::IsTriggerLike(UActor* actor)
{
	// "Triggers" is the base class of Trigger and its variants, not Trigger itself - what maps actually
	// place all descends from it.
	//
	// Deliberately not "anything touchable": the hands exist to work the world's switches, and widening
	// this to every actor with a Touch handler would hand the player things they never asked for. A
	// Teleporter teleports whoever touches it, so brushing one with a fingertip would rip the player
	// across the map.
	return actor->IsA("Triggers");
}

bool VRHands::IsBodyTouching(UPlayerPawn* pawn, UActor* actor)
{
	for (UActor* touching : pawn->Touching())
	{
		if (touching == actor)
			return true;
	}
	return false;
}

void VRHands::Tick()
{
	for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
		Hands[hand] = HandPose();

	VRSubsystem* vr = engine->vr.get();
	if (!vr || !vr->IsActive())
	{
		Reset();
		return;
	}

	// While the pause menu is up the world is frozen and the renderer draws the eyes against a captured
	// camera reference instead of the live one, with the pointer hand driving the menu cursor. Hands
	// placed against the live camera would sit somewhere the player cannot see, and a paused game is not
	// a game whose buttons should be pressable.
	if (engine->console && engine->console->bNoDrawWorld())
	{
		Reset();
		return;
	}

	UPlayerPawn* pawn = engine->viewport ? UObject::TryCast<UPlayerPawn>(engine->viewport->Actor()) : nullptr;
	if (!pawn || !pawn->XLevel() || !pawn->Level()->bBegunPlay() || pawn->bDeleteMe())
	{
		Reset();
		return;
	}

	UpdatePoses(vr);

	for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
	{
		UpdateTriggerContacts(hand, pawn);
		BumpMovers(hand, pawn);

		if (Hands[hand].Valid)
		{
			PreviousPosition[hand] = Hands[hand].Position;
			HasPreviousPosition[hand] = true;
		}
		else
		{
			// Nothing to sweep from next frame: wherever the hand comes back, it did not travel there.
			HasPreviousPosition[hand] = false;
		}
	}
}

void VRHands::UpdatePoses(VRSubsystem* vr)
{
	// Controller poses arrive relative to the play space anchor, which is the camera - the same space the
	// eyes are located in. Mapping one into the world is therefore the camera's rotation applied to the
	// pose and the camera's location added on, which is exactly what RenderSubsystem::BuildVREyeView does
	// to an eye. Doing anything else here would let the hands and the view disagree about where the
	// player's arm is.
	Coords cameraRotation = Coords::Rotation(engine->CameraRotation);
	auto localToWorldDir = [&cameraRotation](const vec3& v)
	{
		return cameraRotation.XAxis * v.x + cameraRotation.YAxis * v.y + cameraRotation.ZAxis * v.z;
	};

	for (int hand = 0; hand < VRSubsystem::HandCount; hand++)
	{
		const VRSubsystem::ControllerState& controller = vr->GetController(hand);
		if (!controller.PoseValid)
			continue;

		HandPose& pose = Hands[hand];
		pose.Valid = true;
		// The same room-scale cancellation the eyes get, and for the same reason: room-scale walking has
		// already moved the pawn (and the camera) under the head, so leaving the head's horizontal offset
		// in here as well would apply it twice and slide the hands away from the body as the player walks
		// around their room.
		pose.Position = engine->CameraLocation + localToWorldDir(engine->vrInput->RemoveRoomScaleOffset(controller.Position));
		pose.Forward = localToWorldDir(controller.Forward);
		pose.Right = localToWorldDir(controller.Right);
		pose.Up = localToWorldDir(controller.Up);
	}
}

void VRHands::UpdateTriggerContacts(int hand, UPlayerPawn* pawn)
{
	Array<Contact>& contacts = Contacts[hand];

	// Drop anything that died while we were inside it. No UnTouch for these: as far as the game is
	// concerned the actor is already gone, and this is the point the root stops holding it alive.
	for (size_t i = contacts.size(); i > 0; i--)
	{
		UActor* actor = contacts[i - 1].Actor.get();
		if (!actor || actor->bDeleteMe())
			contacts.erase(contacts.begin() + (i - 1));
	}

	// A ball, expressed as the cylinder query the collision system offers: equal height and radius.
	Array<UActor*> overlapping;
	const HandPose& pose = Hands[hand];
	if (pose.Valid)
	{
		for (UActor* actor : pawn->XLevel()->Collision.CollidingActors(pose.Position, HandRadius, HandRadius))
		{
			if (actor == pawn || actor->bDeleteMe() || !actor->bCollideActors() || !IsTriggerLike(actor))
				continue;
			overlapping.push_back(actor);
		}
	}

	// Leaving: anything we were inside and no longer are.
	for (size_t i = contacts.size(); i > 0; i--)
	{
		Contact& contact = contacts[i - 1];
		UActor* actor = contact.Actor.get();
		if (Contains(overlapping, actor))
			continue;

		// Unless the player then walked into what their hand was already in, in which case the body has
		// its own touch registered and untouching now would tell the trigger the player left while they
		// are still standing in it. The body's own UnTouch is coming when it steps back out.
		// Dispatched before the erase, which is what still roots the actor.
		if (!IsBodyTouching(pawn, actor))
			CallEvent(actor, EventName::UnTouch, { ExpressionValue::ObjectValue(pawn) });
		contacts.erase(contacts.begin() + (i - 1));
	}

	// Entering. Recorded first and dispatched second, so a script reacting to one of these events cannot
	// resize the array the rest of them are still being read out of.
	size_t firstNew = contacts.size();
	for (UActor* actor : overlapping)
	{
		bool known = false;
		for (size_t i = 0; i < contacts.size(); i++)
			known = known || contacts[i].Actor.get() == actor;
		if (known)
			continue;

		// Leave anything the player's own body is already inside to the body: the engine has registered
		// that touch and fired it, and a second one from the hand would fire every trigger the player
		// stands in twice.
		if (IsBodyTouching(pawn, actor))
			continue;

		Contact contact;
		contact.Actor.set(actor);
		contacts.push_back(std::move(contact));
	}

	for (size_t i = firstNew; i < contacts.size(); i++)
	{
		UActor* actor = contacts[i].Actor.get();
		// An earlier event in this same batch may have destroyed this one.
		if (actor->bDeleteMe())
			continue;
		CallEvent(actor, EventName::Touch, { ExpressionValue::ObjectValue(pawn) });
	}
}

void VRHands::BumpMovers(int hand, UPlayerPawn* pawn)
{
	const HandPose& pose = Hands[hand];
	if (!pose.Valid || !HasPreviousPosition[hand])
		return;

	vec3 from = PreviousPosition[hand];
	vec3 to = pose.Position;
	if (from == to)
		return; // A hand that did not move did not bump anything.

	// Movers have to be swept for, not overlap-tested: they are brush actors, and the overlap path
	// discards those outright (OverlapTester::CylinderActorOverlap bails on actor->Brush()). Only the
	// trace path brush-traces a mover, and it does so under traceWorld rather than traceActors - hence
	// world tracing on here, whose hits (Actor == null) are then skipped below.
	//
	// Sweeping is also the honest model for a hand: an arm swings quickly enough to cross a thin button
	// brush entirely within one 90Hz frame, which a test of where the hand ended up would miss.
	CollisionHitList hits = pawn->XLevel()->Collision.Trace(from, to, HandRadius, HandRadius, false, true, false);

	Array<UActor*> bumped;
	for (auto& hit : hits)
	{
		UActor* actor = hit.Actor;
		if (!actor || actor->bDeleteMe() || !actor->IsA("Mover") || Contains(bumped, actor))
			continue;
		// The same test UActor::TryMove applies before bumping something as the player: only what would
		// have stopped the player bumps, and these events go out as the player.
		if (!actor->bBlockPlayers() || !pawn->bBlockPlayers())
			continue;
		bumped.push_back(actor);
	}

	// Re-sent every frame the hand keeps pushing into it, rather than once when contact starts. That is
	// what the engine does to a mover the player walks into (UActor::TryMove bumps the blocking hit on
	// every move), and what movers are written against: Mover's BumpButton state disables its own Bump
	// handler once it has begun opening.
	for (UActor* actor : bumped)
	{
		if (actor->bDeleteMe()) // an earlier bump in this batch may have destroyed it
			continue;
		CallEvent(actor, EventName::Bump, { ExpressionValue::ObjectValue(pawn) });
	}
}
