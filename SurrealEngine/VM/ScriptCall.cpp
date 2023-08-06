
#include "Precomp.h"
#include "ScriptCall.h"
#include "Frame.h"

NameString ToNameString(EventName name)
{
	static const NameString names[] =
	{
		// Probe names:
		"Spawned",      "Destroyed",       "GainedChild",     "LostChild",
		"Probe4",       "Probe5",          "Trigger",         "UnTrigger",
		"Timer",        "HitWall",         "Falling",         "Landed",
		"ZoneChange",   "Touch",           "UnTouch",         "Bump",
		"BeginState",   "EndState",        "BaseChange",      "Attach",
		"Detach",       "ActorEntered",    "ActorLeaving",    "KillCredit",
		"AnimEnd",      "EndedRotation",   "InterpolateEnd",  "EncroachingOn",
		"EncroachedBy", "FootZoneChange",  "HeadZoneChange",  "PainTimer",
		"SpeechTimer",  "MayFall",         "Probe34",         "Die",
		"Tick",         "PlayerTick",      "Expired",         "Probe39",
		"SeePlayer",    "EnemyNotVisible", "HearNoise",       "UpdateEyeHeight",
		"SeeMonster",   "SeeFriend",       "SpecialHandling", "BotDesireability",
		"Probe48",      "Probe49",         "Probe50",         "Probe51",
		"Probe52",      "Probe53",         "Probe54",         "Probe55",
		"Probe56",      "Probe57",         "Probe58",         "Probe59",
		"Probe60",      "Probe61",         "Probe62",         "All",

		// Other events:
		"PlayerCalcView", "Resolved", "ResolveFailed", "PreBeginPlay",
		"BeginPlay", "PostBeginPlay", "SetInitialState", "SpawnNotification",
		"PostTouch", "FellOutOfWorld", "UpdateTactics", "PlayerInput",
		"Reset", "PreRender", "RenderOverlays", "PostRender",
		"NotifyLevelChange", "InitGame", "PreLogin", "Login",
		"Possess", "TravelPreAccept", "AcceptInventory", "TravelPostAccept",
		"PostLogin", "KeyType", "KeyEvent"
	};
	return names[(int)name];
}

ExpressionValue CallEvent(UObject* Context, EventName eventname, std::vector<ExpressionValue> args)
{
	if (!Context->IsEventEnabled(eventname))
		return ExpressionValue::NothingValue();

	UFunction* func = FindEventFunction(Context, ToNameString(eventname));
	if (func)
		return Frame::Call(func, Context, std::move(args));
	else
		return ExpressionValue::NothingValue();
}

ExpressionValue CallEvent(UObject* Context, const NameString& name, std::vector<ExpressionValue> args)
{
	if (!Context->IsEventEnabled(name))
		return ExpressionValue::NothingValue();

	UFunction* func = FindEventFunction(Context, name);
	if (func)
		return Frame::Call(func, Context, std::move(args));
	else
		return ExpressionValue::NothingValue();
}

UFunction* FindEventFunction(UObject* Context, const NameString& name)
{
	// Search states first

	NameString stateName = Context->GetStateName();
	if (!stateName.IsNone())
	{
		for (UClass* cls = Context->Class; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
		{
			UState* state = cls->GetState(stateName);
			if (state)
			{
				UFunction* func = state->GetFunction(name);
				if (func)
				{
					return func;
				}
			}
		}
	}

	// Search normal member functions next

	for (UClass* cls = Context->Class; cls != nullptr; cls = static_cast<UClass*>(cls->BaseStruct))
	{
		UFunction* func = cls->GetFunction(name);
		if (func)
		{
			return func;
		}
	}

	return nullptr;
}
