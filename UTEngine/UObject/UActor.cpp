
#include "Precomp.h"
#include "UActor.h"
#include "VM/ScriptCall.h"

static std::string tickEventName = "Tick";

void UActor::Tick(float elapsed, bool tickedFlag)
{
	bTicked() = tickedFlag;

	TickAnimation(elapsed);

	if (Role() >= ROLE_SimulatedProxy && IsEventEnabled(tickEventName))
	{
		CallEvent(this, tickEventName, { ExpressionValue::FloatValue(elapsed) });
	}

	if (Role() == ROLE_Authority && RemoteRole() == ROLE_AutonomousProxy)
	{
		TimerCounter() += elapsed;
		while (TimerRate() > 0.0f && TimerCounter() > TimerRate())
		{
			TimerCounter() -= TimerRate();
			if (bTimerLoop())
				TimerRate() = 0.0f;
			CallEvent(this, "Timer");
		}
	}
}

void UActor::TickAnimation(float elapsed)
{
	// To do: play active animation here. Call MeshAnimNotify functions as required. Set bAnimFinished() and release any latent FinishAnim function. Also fire "AnimEnd" event if it finishes. 

	// float animRate = (AnimRate() >= 0) ? AnimRate() * elapsed : std::max(AnimMinRate(), -AnimRate() * length(Velocity()));
	// AnimFrame() += animRate * elapsed;
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
