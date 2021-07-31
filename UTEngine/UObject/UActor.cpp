
#include "Precomp.h"
#include "UActor.h"
#include "ULevel.h"
#include "UMesh.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"
#include "Package/PackageManager.h"
#include "Engine.h"
#include "Collision.h"

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

	actor->Outer() = engine->Level->Outer();
	actor->XLevel() = engine->Level;
	actor->Level() = engine->LevelInfo;
	actor->Tag() = (!SpawnTag.empty() && SpawnTag != "None") ? SpawnTag : SpawnClass->Name;
	actor->bTicked() = bTicked(); // To do: should it tick in the same world tick it was spawned in or wait until the next one?
	actor->Instigator() = Instigator();
	actor->Brush() = nullptr;
	actor->Location() = location;
	actor->OldLocation() = location;
	actor->Rotation() = rotation;

	engine->Level->Actors.push_back(actor);

	actor->SetOwner(SpawnOwner ? SpawnOwner : this);

	actor->CreateDefaultState();

	// To do: find the correct zone and BSP leaf for the actor
	PointRegion region = {};
	for (size_t i = 0; i < engine->Level->Model->Zones.size(); i++)
	{
		auto& zone = engine->Level->Model->Zones[i];
		if (zone.ZoneActor)
		{
			region.Zone = UObject::Cast<UZoneInfo>(zone.ZoneActor);
			region.ZoneNumber = (uint8_t)i;
			region.BspLeaf = 0;
			break;
		}
	}
	actor->Region() = region;
	UPawn* pawn = UObject::TryCast<UPawn>(actor);
	if (pawn)
	{
		pawn->HeadRegion() = region;
		pawn->FootRegion() = region;
	}

	if (engine->LevelInfo->bBegunPlay())
	{
		CallEvent(actor, "Spawned");
		CallEvent(actor, "PreBeginPlay");
		CallEvent(actor, "BeginPlay");

		if (actor->bDeleteMe())
			return nullptr;

		// To do: we need to call touch events here

		CallEvent(actor, "PostBeginPlay");
		CallEvent(actor, "SetInitialState");

		// Find base for certain types
		bool isDecorationInventoryOrPawn = UObject::TryCast<UDecoration>(actor) || UObject::TryCast<UInventory>(actor) || UObject::TryCast<UPawn>(actor);
		if (isDecorationInventoryOrPawn && !actor->ActorBase() && actor->bCollideWorld() && (actor->Physics() == PHYS_None || actor->Physics() == PHYS_Rotating))
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
			UActor* hitActor = engine->LevelInfo;
			actor->SetBase(hitActor, true);
#endif
		}

		std::string attachTag = actor->AttachTag();
		if (!attachTag.empty() && attachTag != "None")
		{
			for (UActor* levelActor : engine->Level->Actors)
			{
				if (levelActor && levelActor->Tag() == attachTag)
				{
					levelActor->SetBase(actor, false);
				}
			}
		}

		static bool spawnNotificationLocked = false;
		if (!spawnNotificationLocked)
		{
			struct NotificationLockGuard
			{
				NotificationLockGuard() { spawnNotificationLocked = true; }
				~NotificationLockGuard() { spawnNotificationLocked = false; }
			} lockGuard;

			if (!engine->packages->IsUnreal1())
			{
				for (USpawnNotify* notifyObj = engine->LevelInfo->SpawnNotify(); notifyObj != nullptr; notifyObj = notifyObj->Next())
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

		if (ActorBase() && ActorBase() != engine->LevelInfo)
		{
			ActorBase()->StandingCount()--;
			CallEvent(ActorBase(), "Detach", { ExpressionValue::ObjectValue(this) });
		}

		ActorBase() = newBase;

		if (ActorBase() && ActorBase() != engine->LevelInfo)
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
			AnimRate() = 1.0f / seq->NumFrames * (seq->Rate * (rate ? *rate : 1.0f));
			bAnimLoop() = false;
		}
	}
}

void UActor::LoopAnim(const std::string& sequence, float* rate, float* tweenTime, float* minRate)
{
	// To do: TweenTime = Amount of Time to "tween" into the first frame of this animation sequence if in a different sequence
	// To do: what does minRate do?

	if (Mesh())
	{
		MeshAnimSeq* seq = Mesh()->GetSequence(sequence);
		if (seq)
		{
			AnimSequence() = sequence;
			AnimFrame() = 0.0f;
			AnimRate() = 1.0f / seq->NumFrames * (seq->Rate * (rate ? *rate : 1.0f));
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
			float fromFrame = AnimFrame();
			float animRate = (AnimRate() >= 0) ? AnimRate() : std::max(AnimMinRate(), -AnimRate() * length(Velocity()));
			float toFrame = fromFrame + animRate * elapsed;

			if (toFrame > 1.0f)
			{
				elapsed -= (toFrame - fromFrame) / animRate;
				toFrame = 1.0f;
			}

			MeshAnimSeq* seq = Mesh()->GetSequence(AnimSequence());
			if (seq)
			{
				for (const MeshAnimNotify& n : seq->Notifys)
				{
					if (n.Time > fromFrame && n.Time <= toFrame)
					{
						if (FindEventFunction(this, n.Function))
						{
							AnimFrame() = n.Time;
							CallEvent(this, n.Function);
						}
					}
				}
			}

			if (toFrame == 1.0f)
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
				AnimFrame() = toFrame;
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////

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
		Weapon()->Location() = Location();

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
