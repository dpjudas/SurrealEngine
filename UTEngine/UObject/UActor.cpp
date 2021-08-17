
#include "Precomp.h"
#include "UActor.h"
#include "ULevel.h"
#include "UMesh.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "Package/PackageManager.h"
#include "Engine.h"

static std::string tickEventName = "Tick";

UActor* UActor::Spawn(UClass* SpawnClass, UActor* SpawnOwner, std::string SpawnTag, vec3* SpawnLocation, Rotator* SpawnRotation)
{
	// To do: return null if spawn location isn't valid

	if (!SpawnClass) // To do: return null if spawn class is abstract
	{
		return nullptr;
	}

	vec3 location = SpawnLocation ? *SpawnLocation : Location();
	Rotator rotation = SpawnRotation ? *SpawnRotation : Rotation();

	// To do: package needs to be grabbed from outer, or the "transient package" if it is None, a virtual package for runtime objects
	UActor* actor = UObject::Cast<UActor>(engine->packages->GetPackage("Engine")->NewObject("", UObject::Cast<UClass>(SpawnClass), ObjectFlags::Transient, true));

	actor->Outer() = XLevel()->Outer();
	actor->XLevel() = XLevel();
	actor->Level() = Level();
	actor->Tag() = (!SpawnTag.empty() && SpawnTag != "None") ? SpawnTag : SpawnClass->Name;
	actor->bTicked() = bTicked(); // To do: should it tick in the same world tick it was spawned in or wait until the next one?
	actor->Instigator() = Instigator();
	actor->Brush() = nullptr;
	actor->Location() = location;
	actor->OldLocation() = location;
	actor->Rotation() = rotation;

	XLevel()->Actors.push_back(actor);
	XLevel()->AddToCollision(actor);

	actor->SetOwner(SpawnOwner ? SpawnOwner : this);
	actor->InitActorZone();

	if (Level()->bBegunPlay())
	{
		CallEvent(actor, "Spawned");
		CallEvent(actor, "PreBeginPlay");
		CallEvent(actor, "BeginPlay");

		if (actor->bDeleteMe())
			return nullptr;

		// To do: we need to call touch events here

		CallEvent(actor, "PostBeginPlay");
		CallEvent(actor, "SetInitialState");

		actor->InitBase();

		if (engine->packages->GetEngineVersion() >= 400)
		{
			static bool spawnNotificationLocked = false;
			if (!spawnNotificationLocked)
			{
				struct NotificationLockGuard
				{
					NotificationLockGuard() { spawnNotificationLocked = true; }
					~NotificationLockGuard() { spawnNotificationLocked = false; }
				} lockGuard;

				for (USpawnNotify* notifyObj = Level()->SpawnNotify(); notifyObj != nullptr; notifyObj = notifyObj->Next())
				{
					UClass* cls = notifyObj->ActorClass();
					if (cls && actor->IsA(cls->Name))
						actor = UObject::Cast<UGameInfo>(CallEvent(notifyObj, "SpawnNotification", { ExpressionValue::ObjectValue(actor) }).ToObject());
				}
			}
		}
	}

	return actor;
}

void UActor::InitBase()
{
	if (engine->packages->GetEngineVersion() > 219)
	{
		std::string attachTag = AttachTag();
		if (!attachTag.empty() && attachTag != "None")
		{
			for (UActor* levelActor : XLevel()->Actors)
			{
				if (levelActor && levelActor->Tag() == attachTag)
				{
					levelActor->SetBase(this, false);
				}
			}
			return;
		}
	}

	// Find base for certain types
	bool isDecorationInventoryOrPawn = UObject::TryCast<UDecoration>(this) || UObject::TryCast<UInventory>(this) || UObject::TryCast<UPawn>(this);
	if (isDecorationInventoryOrPawn && !ActorBase() && bCollideWorld() && (Physics() == PHYS_None || Physics() == PHYS_Rotating))
	{
#if 0
		UActor* hitActor = nullptr;
		auto onHit = [&](UActor* testActor) -> bool
		{
			for (UActor* cur = testActor; cur; cur = cur->Owner())
			{
				if (cur == hitActor)
					return true;
			}
			hitActor = testActor;
			return false;
		};
		engine->collision->TraceHit(actor->Location(), actor->Location() + vec3(0.0f, 0.0f, -10.0f), onHit);
		actor->SetBase(hitActor);
#else
		UActor* hitActor = Level();
		SetBase(hitActor, true);
#endif
	}
}

bool UActor::Destroy()
{
	if (bStatic() || bNoDelete())
		return false;
	if (bDeleteMe())
		return true;

	bDeleteMe() = true;

	GotoState({}, {});
	SetBase(nullptr, true);

	ULevel* level = XLevel();

	level->RemoveFromCollision(this);

	CallEvent(this, "Destroyed");

	UActor** TouchingArray = &Touching();
	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray[i])
			UnTouch(TouchingArray[i]);
	}

	if (Owner())
		CallEvent(Owner(), "LostChild", { ExpressionValue::ObjectValue(this) });

	for (size_t i = 0; i < level->Actors.size(); i++)
	{
		UActor* actor = level->Actors[i];
		if (actor && actor->Owner() == this)
		{
			actor->SetOwner(nullptr);
		}
		if (actor == this)
		{
			level->Actors[i] = nullptr;
		}
	}

	return true;
}

PointRegion UActor::FindRegion(const vec3& offset)
{
	PointRegion region;
	region.BspLeaf = 0;
	region.ZoneNumber = 0;

	vec4 location = vec4(Location() + offset, 1.0f);

	// Search the BSP
	BspNode* nodes = XLevel()->Model->Nodes.data();
	BspNode* node = nodes;
	while (true)
	{
		vec4 plane = { node->PlaneX, node->PlaneY, node->PlaneZ, -node->PlaneW };
		float side = dot(location, plane);
		if (node->Front >= 0 && side >= 0.0f)
		{
			node = nodes + node->Front;
		}
		else if (node->Back >= 0 && side <= 0.0f)
		{
			node = nodes + node->Back;
		}
		else
		{
			region.ZoneNumber = node->Zone1;
			region.BspLeaf = side >= 0.0f ? node->Leaf0 : node->Leaf1;
			break;
		}
	}

	region.Zone = UObject::Cast<UZoneInfo>(XLevel()->Model->Zones[region.ZoneNumber].ZoneActor);
	if (!region.Zone)
		region.Zone = Level();

	return region;
}

void UActor::InitActorZone()
{
	Region() = FindRegion();
}

void UActor::UpdateActorZone()
{
	PointRegion oldregion = Region();
	PointRegion newregion = FindRegion();

	if (Region().Zone && oldregion.Zone != newregion.Zone)
		CallEvent(Region().Zone, "ActorLeaving", { ExpressionValue::ObjectValue(this) });

	Region() = newregion;

	if (Region().Zone && oldregion.Zone != newregion.Zone)
		CallEvent(Region().Zone, "ActorEntered", { ExpressionValue::ObjectValue(this) });
}

void UActor::SetOwner(UActor* newOwner)
{
	if (Owner())
		CallEvent(Owner(), "LostChild", { ExpressionValue::ObjectValue(this) });

	Owner() = newOwner;

	if (Owner())
		CallEvent(Owner(), "GainedChild", { ExpressionValue::ObjectValue(this) });
}

void UActor::SetBase(UActor* newBase, bool sendBaseChangeEvent)
{
	if (ActorBase() != newBase)
	{
		for (UActor* cur = newBase; cur; cur = cur->ActorBase())
		{
			if (cur == this)
				return;
		}

		if (ActorBase() && ActorBase() != Level())
		{
			ActorBase()->StandingCount()--;
			CallEvent(ActorBase(), "Detach", { ExpressionValue::ObjectValue(this) });
		}

		ActorBase() = newBase;

		if (ActorBase() && ActorBase() != Level())
		{
			ActorBase()->StandingCount()++;
			CallEvent(ActorBase(), "Attach", { ExpressionValue::ObjectValue(this) });
		}

		if (sendBaseChangeEvent)
			CallEvent(this, "BaseChange");
	}
}

void UActor::Tick(float elapsed, bool tickedFlag)
{
	bTicked() = tickedFlag;

	TickAnimation(elapsed);

	if (Role() >= ROLE_SimulatedProxy && IsEventEnabled(tickEventName))
	{
		CallEvent(this, tickEventName, { ExpressionValue::FloatValue(elapsed) });
	}

	if (Role() >= ROLE_SimulatedProxy && StateFrame && StateFrame->LatentState == LatentRunState::Continue)
	{
		auto curStateFrame = StateFrame; // pin frame as GotoFrame may otherwise destroy it
		curStateFrame->Tick();
	}

	TickPhysics(elapsed);

	if (TimerRate() > 0.0f) // Role() == ROLE_Authority && RemoteRole() == ROLE_AutonomousProxy
	{
		TimerCounter() += elapsed;
		while (TimerRate() > 0.0f && TimerCounter() > TimerRate())
		{
			TimerCounter() -= TimerRate();
			if (!bTimerLoop())
				TimerRate() = 0.0f;
			CallEvent(this, "Timer");
		}
	}
}

void UActor::TickPhysics(float elapsed)
{
	int mode = Physics();
	if (mode != PHYS_None)
	{
		// To do: do all that cylinder based physics here!

		switch (mode)
		{
		case PHYS_Walking: TickWalking(elapsed); break;
		case PHYS_Falling: TickFalling(elapsed); break;
		case PHYS_Swimming: TickSwimming(elapsed); break;
		case PHYS_Flying: TickFlying(elapsed); break;
		case PHYS_Rotating: TickRotating(elapsed); break;
		case PHYS_Projectile: TickProjectile(elapsed); break;
		case PHYS_Rolling: TickRolling(elapsed); break;
		case PHYS_Interpolating: TickInterpolating(elapsed); break;
		case PHYS_MovingBrush: TickMovingBrush(elapsed); break;
		case PHYS_Spider: TickSpider(elapsed); break;
		case PHYS_Trailer: TickTrailer(elapsed); break;
		}
	}
}

void UActor::TickWalking(float elapsed)
{
}

void UActor::TickFalling(float elapsed)
{
}

void UActor::TickSwimming(float elapsed)
{
}

void UActor::TickFlying(float elapsed)
{
}

void UActor::TickRotating(float elapsed)
{
	if (bRotateToDesired() && Rotation() != DesiredRotation())
	{
		// To do: rotate by RotationRate until we reach the rotation. Then fire "EndedRotation" event.
	}
	else if (bFixedRotationDir())
	{
		Rotation() += RotationRate() * elapsed;
	}
}

void UActor::TickProjectile(float elapsed)
{
}

void UActor::TickRolling(float elapsed)
{
}

void UActor::TickInterpolating(float elapsed)
{
	OldLocation() = Location();

	float timeLeft = elapsed;
	while (timeLeft > 0.0f)
	{
		if (PhysRate() == 0.0f || !bInterpolating())
			break;

		UInterpolationPoint* target = UObject::Cast<UInterpolationPoint>(Target());
		UInterpolationPoint* next = target ? target->Next() : nullptr;
		if (!target || !next)
			break;

		float physAlpha = PhysAlpha();

		if (dynamic_cast<UPlayerPawn*>(this))
		{
			UPlayerPawn* pawn = static_cast<UPlayerPawn*>(this);
			pawn->DesiredFlashScale() = mix(target->ScreenFlashScale(), next->ScreenFlashScale(), physAlpha);
			pawn->DesiredFlashFog() = mix(target->ScreenFlashFog(), next->ScreenFlashFog(), physAlpha);
			pawn->FovAngle() = mix(target->FovModifier(), next->FovModifier(), physAlpha) * Base->GetDefaultObject()->GetFloat("FovAngle");
			pawn->FlashScale() = vec3(pawn->DesiredFlashScale());
			pawn->FlashFog() = pawn->DesiredFlashFog();
		}

		Level()->TimeDilation() = mix(target->GameSpeedModifier(), next->GameSpeedModifier(), physAlpha);

		float rateModifier = mix(target->RateModifier(), next->RateModifier(), physAlpha);
		float physRate = PhysRate() * rateModifier;
		if (physRate == 0.0f)
			break;

		bool interpolateStart = false, interpolateEnd = false;
		physAlpha += physRate * timeLeft;
		if (physRate < 0.0f && physAlpha < 0.0f)
		{
			timeLeft = physAlpha / physRate;
			physAlpha = 0.0f;
			interpolateStart = true;
		}
		else if (physRate > 0.0f && physAlpha > 1.0f)
		{
			timeLeft = (physAlpha - 1.0f) / physRate;
			physAlpha = 1.0f;
			interpolateEnd = true;
		}
		else
		{
			timeLeft = 0.0f;
		}

		UInterpolationPoint* prev = target->Position() > 0 ? target->Prev() : target;
		UInterpolationPoint* nextnext = next->Next();
		if (!nextnext) nextnext = next;
		if (next->bEndOfPath()) nextnext = next;
		if (target->bEndOfPath()) { next = target; nextnext = target; }
		vec3 location = spline(prev->Location(), target->Location(), next->Location(), nextnext->Location(), physAlpha);
		Rotator rotation = spline(prev->Rotation(), target->Rotation(), next->Rotation(), nextnext->Rotation(), physAlpha);

		PhysAlpha() = physAlpha;
		TryMove(location - Location());
		Rotation() = rotation;

		if (dynamic_cast<UPawn*>(this))
		{
			static_cast<UPawn*>(this)->ViewRotation() = Rotation();
		}

		if (interpolateStart)
		{
			CallEvent(target, "InterpolateEnd", { ExpressionValue::ObjectValue(this) });
			CallEvent(this, "InterpolateEnd", { ExpressionValue::ObjectValue(target) });

			target = target->Prev();
			while (target && target->bSkipNextPath())
				target = target->Prev();

			Target() = target;
			PhysAlpha() = 1.0f;
		}
		else if (interpolateEnd)
		{
			CallEvent(target, "InterpolateEnd", { ExpressionValue::ObjectValue(this) });
			CallEvent(this, "InterpolateEnd", { ExpressionValue::ObjectValue(target) });

			target = target->Next();
			while (target && target->bSkipNextPath())
				target = target->Next();

			Target() = target;
			PhysAlpha() = 0.0f;
		}
	}

	if (elapsed > 0.0f)
		Velocity() = (Location() - OldLocation()) / elapsed;
}

void UActor::TickMovingBrush(float elapsed)
{
}

void UActor::TickSpider(float elapsed)
{
}

void UActor::TickTrailer(float elapsed)
{
}

void UActor::SetPhysics(uint8_t newPhysics)
{
	Physics() = newPhysics;
}

void UActor::SetCollision(bool newColActors, bool newBlockActors, bool newBlockPlayers)
{
	XLevel()->RemoveFromCollision(this);
	bCollideActors() = newColActors;
	bBlockActors() = newBlockActors;
	bBlockPlayers() = newBlockPlayers;
	XLevel()->AddToCollision(this);
}

bool UActor::SetLocation(const vec3& newLocation)
{
	// To do: do overlap test and return false if the object cannot be moved to this location

	XLevel()->RemoveFromCollision(this);
	Location() = newLocation;
	XLevel()->AddToCollision(this);
	return true;
}

bool UActor::SetRotation(const Rotator& newRotation)
{
	// To do: do overlap test and return false if the object cannot be changed to this rotation
	
	Rotation() = newRotation;
	return true;
}

bool UActor::SetCollisionSize(float newRadius, float newHeight)
{
	// To do: do overlap test and return false if the object cannot be changed to this new size

	XLevel()->RemoveFromCollision(this);
	CollisionRadius() = newRadius;
	CollisionHeight() = newHeight;
	XLevel()->AddToCollision(this);
	return true;
}

UObject* UActor::Trace(vec3& hitLocation, vec3& hitNormal, const vec3& traceEnd, const vec3& traceStart, bool bTraceActors, const vec3& extent)
{
	// To do: this needs to do an AABB sweep (hmm, does UE1 treat all actor cylinders as AABB?)

	std::vector<SweepHit> hits = XLevel()->Sweep(traceStart, traceEnd, extent.z, extent.x, bTraceActors, true);
	for (SweepHit& hit : hits)
	{
		if (hit.Actor && hit.Actor != this)
		{
			hitNormal = hit.Normal;
			hitLocation = traceStart + (traceEnd - traceStart) * hit.Fraction;
			return hit.Actor;
		}
		else
		{
			hitNormal = hit.Normal;
			hitLocation = traceStart + (traceEnd - traceStart) * hit.Fraction;
			return Level();
		}
	}

	return nullptr;
}

bool UActor::FastTrace(const vec3& traceEnd, const vec3& traceStart)
{
	return XLevel()->TraceAnyHit(traceStart, traceEnd, this, false, true);
}

bool UActor::IsBasedOn(UActor* other)
{
	for (UActor* cur = other; cur; cur = cur->ActorBase())
	{
		if (cur == this)
		{
			return true;
		}
	}
	return false;
}

bool UActor::IsOverlapping(UActor* other)
{
	vec3 dist = Location() - other->Location();
	float h = CollisionHeight() + other->CollisionHeight();
	float r = CollisionRadius() + other->CollisionRadius();
	return dist.z < h && dot(dist.xy(), dist.xy()) < r * r;
}

SweepHit UActor::TryMove(const vec3& delta)
{
	// Static and non-movable objects can't move
	if (bStatic() || !bMovable())
	{
		SweepHit hit;
		hit.Fraction = 0.0f;
		return hit;
	}

	// Avoid moving if movement is too small as the physics code doesn't like very small numbers
	if (dot(delta, delta) < 0.0001f)
		return {};

	// Analyze what we will hit if we move as requested and stop if it is the level or a blocking actor
	bool useBlockPlayers = UObject::TryCast<UPlayerPawn>(this) || UObject::TryCast<UProjectile>(this);
	SweepHit blockingHit;
	std::vector<SweepHit> hits = XLevel()->Sweep(Location(), Location() + delta, CollisionHeight(), CollisionRadius(), bCollideActors(), bCollideWorld());
	for (auto& hit : hits)
	{
		if (hit.Actor)
		{
			bool isBlocking;
			if (useBlockPlayers || UObject::TryCast<UPlayerPawn>(hit.Actor) || UObject::TryCast<UProjectile>(hit.Actor))
				isBlocking = hit.Actor->bBlockPlayers();
			else
				isBlocking = hit.Actor->bBlockActors();

			// We never hit ourselves or anything moving along with us
			if (isBlocking && !hit.Actor->IsBasedOn(this) && !IsBasedOn(hit.Actor))
			{
				blockingHit = hit;
				break;
			}
		}
		else
		{
			blockingHit = hit;
			break;
		}
	}

	vec3 actuallyMoved = delta * blockingHit.Fraction;

	XLevel()->RemoveFromCollision(this);
	Location() += actuallyMoved;
	XLevel()->AddToCollision(this);

	// Based actors needs to move with us
	if (StandingCount() > 0)
	{
		ULevel* level = XLevel();
		for (size_t i = 0; i < level->Actors.size(); i++)
		{
			UActor* actor = level->Actors[i];
			if (actor && actor->ActorBase() == this)
			{
				actor->TryMove(actuallyMoved);
			}
		}
	}

	// Send bump notification if we hit an actor
	if (blockingHit.Actor)
	{
		if (!blockingHit.Actor->IsBasedOn(this))
		{
			CallEvent(blockingHit.Actor, "Bump", { ExpressionValue::ObjectValue(this) });
			CallEvent(this, "Bump", { ExpressionValue::ObjectValue(blockingHit.Actor) });
		}
	}

	// Send touch notifications for anything we crossed while moving
	for (auto& hit : hits)
	{
		if (hit.Fraction >= blockingHit.Fraction)
			break;

		if (hit.Actor && !hit.Actor->IsBasedOn(this) && !IsBasedOn(hit.Actor))
		{
			Touch(hit.Actor);
		}
	}

	// Untouch everything we aren't overlapping anymore
	UActor** TouchingArray = &Touching();
	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray[i] && !IsOverlapping(TouchingArray[i]))
		{
			UnTouch(TouchingArray[i]);
		}
	}

	UpdateActorZone();

	return blockingHit;
}

void UActor::Touch(UActor* actor)
{
	UActor** TouchingArray = &Touching();
	UActor** TouchingArray2 = &actor->Touching();

	// Do nothing if actors are already touching
	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray[i] == actor)
			return;
	}

	// Only attempt to touch actors if there's a free slot in both
	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (!TouchingArray[i])
		{
			for (int j = 0; j < TouchingArraySize; j++)
			{
				if (!TouchingArray2[j])
				{
					TouchingArray[i] = actor;
					CallEvent(this, "Touch", { ExpressionValue::ObjectValue(actor) });

					if (!TouchingArray2[j])
					{
						TouchingArray2[j] = this;
						CallEvent(actor, "Touch", { ExpressionValue::ObjectValue(this) });
					}
					return;
				}
			}
		}
	}
}

void UActor::UnTouch(UActor* actor)
{
	UActor** TouchingArray = &Touching();
	UActor** TouchingArray2 = &actor->Touching();

	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray[i] == actor)
		{
			TouchingArray[i] = nullptr;
			CallEvent(this, "UnTouch", { ExpressionValue::ObjectValue(actor) });
		}
	}

	for (int i = 0; i < TouchingArraySize; i++)
	{
		if (TouchingArray2[i] == this)
		{
			TouchingArray2[i] = nullptr;
			CallEvent(actor, "UnTouch", { ExpressionValue::ObjectValue(this) });
		}
	}
}

bool UActor::Move(const vec3& delta)
{
	return TryMove(delta).Fraction == 1.0f;
}

bool UActor::MoveSmooth(const vec3& delta)
{
	SweepHit hit = TryMove(delta);
	if (hit.Fraction != 1.0f)
	{
		// We hit a slope. Try to follow it.
		vec3 alignedDelta = (delta - hit.Normal * dot(delta, hit.Normal)) * (1.0f - hit.Fraction);
		if (dot(delta, alignedDelta) >= 0.0f) // Don't end up going backwards
		{
			SweepHit hit2 = TryMove(alignedDelta);
		}
	}

	return hit.Fraction != 1.0f;
}

bool UActor::HasAnim(const std::string& sequence)
{
	return Mesh() && Mesh()->GetSequence(sequence);
}

bool UActor::IsAnimating()
{
	return AnimRate() != 0.0f;
}

std::string UActor::GetAnimGroup(const std::string& sequence)
{
	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
			return seq->Group;
	}
	return {};
}

void UActor::PlayAnim(const std::string& sequence, float* rate, float* tweenTime)
{
	// To do: TweenTime = Amount of Time to "tween" into the first frame of this animation sequence if in a different sequence

	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
		{
			AnimSequence() = sequence;
			AnimFrame() = 0.0f;
			AnimRate() = (rate ? *rate : 1.0f) * seq->Rate / seq->NumFrames;
			AnimMinRate() = 0.0f;
			bAnimLoop() = false;
		}
	}
}

void UActor::LoopAnim(const std::string& sequence, float* rate, float* tweenTime, float* minRate)
{
	// To do: TweenTime = Amount of Time to "tween" into the first frame of this animation sequence if in a different sequence

	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
		{
			AnimSequence() = sequence;
			AnimFrame() = 0.0f;
			AnimRate() = (rate ? *rate : 1.0f) * seq->Rate / seq->NumFrames;
			AnimMinRate() = (minRate ? *minRate : 0.0f) * seq->Rate / seq->NumFrames;
			bAnimLoop() = true;
		}
	}
}

void UActor::TweenAnim(const std::string& sequence, float tweenTime)
{
	// "Tween into a new animation" this means what? it keeps the current rate multiplier that was passed into an early Play/LoopAnim call?

	PlayAnim(sequence, nullptr, &tweenTime);
}

void UActor::TickAnimation(float elapsed)
{
	if (Mesh())
	{
		for (int i = 0; AnimRate() != 0.0f && elapsed > 0.0f && i < 10; i++)
		{
			float fromAnimTime = AnimFrame();
			float animRate = (AnimRate() >= 0) ? AnimRate() : std::max(AnimMinRate(), -AnimRate() * length(Velocity()));
			float toAnimTime = fromAnimTime + animRate * elapsed;

			if (toAnimTime >= 1.0f)
			{
				elapsed -= (toAnimTime - fromAnimTime) / animRate;
				toAnimTime = 1.0f;
			}
			else
			{
				elapsed = 0.0f;
			}

			MeshAnimSeq* seq = Mesh()->GetSequence(AnimSequence());
			if (seq)
			{
				for (const MeshAnimNotify& n : seq->Notifys)
				{
					if (n.Time > fromAnimTime && n.Time <= toAnimTime)
					{
						if (FindEventFunction(this, n.Function))
						{
							AnimFrame() = n.Time;
							CallEvent(this, n.Function);
						}
					}
				}
			}

			if (toAnimTime == 1.0f)
			{
				bAnimFinished() = true;

				if (StateFrame && StateFrame->LatentState == LatentRunState::FinishAnim)
					StateFrame->LatentState = LatentRunState::Continue;

				AnimFrame() = 0.0f;
				if (!bAnimLoop())
					AnimRate() = 0.0f;
				CallEvent(this, "AnimEnd");
			}
			else
			{
				AnimFrame() = toAnimTime;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void UPawn::InitActorZone()
{
	UActor::InitActorZone();

	FootRegion() = FindRegion({ 0.0f, 0.0f, -CollisionHeight() });
	HeadRegion() = FindRegion({ 0.0f, 0.0f, EyeHeight() });

	if (PlayerReplicationInfo())
		PlayerReplicationInfo()->PlayerZone() = Region().Zone;
}

void UPawn::UpdateActorZone()
{
	UActor::UpdateActorZone();

	PointRegion oldfootregion = FootRegion();
	PointRegion newfootregion = FindRegion({ 0.0f, 0.0f, -CollisionHeight() });
	if (FootRegion().Zone && oldfootregion.Zone != newfootregion.Zone)
		CallEvent(FootRegion().Zone, "FootZoneChange", { ExpressionValue::ObjectValue(this) });
	FootRegion() = newfootregion;

	PointRegion oldheadregion = HeadRegion();
	PointRegion newheadregion = FindRegion({ 0.0f, 0.0f, EyeHeight() });
	if (HeadRegion().Zone && oldheadregion.Zone != newheadregion.Zone)
		CallEvent(HeadRegion().Zone, "HeadZoneChange", { ExpressionValue::ObjectValue(this) });
	HeadRegion() = newheadregion;

	if (PlayerReplicationInfo())
		PlayerReplicationInfo()->PlayerZone() = Region().Zone;
}

void UPawn::Tick(float elapsed, bool tickedFlag)
{
	UActor::Tick(elapsed, tickedFlag);

	if (bIsPlayer() && Role() >= ROLE_AutonomousProxy)
	{
		if (bViewTarget())
			CallEvent(this, "UpdateEyeHeight", { ExpressionValue::FloatValue(elapsed) });
		else
			ViewRotation() = Rotation();
	}

	if (Weapon())
	{
		Weapon()->Location() = Location();
		Weapon()->UpdateActorZone();
	}

	if (Role() == ROLE_Authority)
	{
		if (PainTime() > 0.0f)
		{
			PainTime() = std::max(PainTime() - elapsed, 0.0f);
			if (PainTime() == 0.0f)
				CallEvent(this, "PainTimer");
		}
		if (SpeechTime() > 0.0f)
		{
			SpeechTime() = std::max(SpeechTime() - elapsed, 0.0f);
			if (SpeechTime() == 0.0f)
				CallEvent(this, "SpeechTimer");
		}
		if (bAdvancedTactics())
			CallEvent(this, "UpdateTactics", { ExpressionValue::FloatValue(elapsed) });
	}
}

/////////////////////////////////////////////////////////////////////////////

void UPlayerPawn::Tick(float elapsed, bool tickedFlag)
{
	UPawn::Tick(elapsed, tickedFlag);

	if (Role() >= ROLE_SimulatedProxy)
	{
		if (Player() && !UObject::TryCast<UCamera>(this))
		{
			CallEvent(this, "PlayerInput", { ExpressionValue::FloatValue(elapsed) });
			CallEvent(this, "PlayerTick", { ExpressionValue::FloatValue(elapsed) });
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

void ULevelInfo::UpdateActorZone()
{
	// No zone events are sent by LevelInfo actors
	Region() = FindRegion();
}
