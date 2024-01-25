#pragma once

#include "ExpressionValue.h"

class UObject;

enum class EventName
{
	// Probe names:
	Spawned, Destroyed, GainedChild, LostChild,
	Probe4, Probe5, Trigger, UnTrigger,
	Timer, HitWall, Falling, Landed,
	ZoneChange, Touch, UnTouch, Bump,
	BeginState, EndState, BaseChange, Attach,
	Detach, ActorEntered, ActorLeaving, KillCredit,
	AnimEnd, EndedRotation, InterpolateEnd, EncroachingOn,
	EncroachedBy, FootZoneChange, HeadZoneChange, PainTimer,
	SpeechTimer, MayFall, Probe34, Die,
	Tick, PlayerTick, Expired, Probe39,
	SeePlayer, EnemyNotVisible, HearNoise, UpdateEyeHeight,
	SeeMonster, SeeFriend, SpecialHandling, BotDesireability,
	Probe48, Probe49, Probe50, Probe51,
	Probe52, Probe53, Probe54, Probe55,
	Probe56, Probe57, Probe58, Probe59,
	Probe60, Probe61, Probe62, All,

	// Other events:
	PlayerCalcView, Resolved, ResolveFailed, PreBeginPlay,
	BeginPlay, PostBeginPlay, SetInitialState, SpawnNotification,
	PostTouch, FellOutOfWorld, UpdateTactics, PlayerInput,
	Reset, PreRender, RenderOverlays, PostRender,
	NotifyLevelChange, InitGame, PreLogin, Login,
	Possess, TravelPreAccept, AcceptInventory, TravelPostAccept,
	PostLogin, KeyType, KeyEvent,

	MaxEventNameValue // Why isn't this part of C++ after 40+ years of people doing this in both C and C++?
};

ExpressionValue CallEvent(UObject* Context, EventName name, std::vector<ExpressionValue> args = {});
ExpressionValue CallEvent(UObject* Context, const NameString& name, std::vector<ExpressionValue> args = {});

UFunction* FindEventFunction(UObject* Context, const NameString& name);

NameString ToNameString(EventName name);
bool NameStringToEventName(const NameString& name, EventName& eventName);
