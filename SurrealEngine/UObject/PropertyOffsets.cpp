
#include "Precomp.h"
#include "PropertyOffsets.h"
#include "Package/PackageManager.h"
#include "UClass.h"
#include "UProperty.h"

PropertyOffsets_Object PropOffsets_Object;

static void InitPropertyOffsets_Object(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Core")->GetUObject("Class", "Object"));
	if (!cls)
	{
		memset(&PropOffsets_Object, 0xff, sizeof(PropOffsets_Object));
		return;
	}
	PropOffsets_Object.Class = cls->GetPropertyDataOffset("Class");
	PropOffsets_Object.Name = cls->GetPropertyDataOffset("Name");
	PropOffsets_Object.ObjectFlags = cls->GetPropertyDataOffset("ObjectFlags");
	PropOffsets_Object.ObjectInternal = cls->GetPropertyDataOffset("ObjectInternal");
	PropOffsets_Object.Outer = cls->GetPropertyDataOffset("Outer");
}

PropertyOffsets_Commandlet PropOffsets_Commandlet;

static void InitPropertyOffsets_Commandlet(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Core")->GetUObject("Class", "Commandlet"));
	if (!cls)
	{
		memset(&PropOffsets_Commandlet, 0xff, sizeof(PropOffsets_Commandlet));
		return;
	}
	PropOffsets_Commandlet.HelpCmd = cls->GetPropertyDataOffset("HelpCmd");
	PropOffsets_Commandlet.HelpDesc = cls->GetPropertyDataOffset("HelpDesc");
	PropOffsets_Commandlet.HelpOneLiner = cls->GetPropertyDataOffset("HelpOneLiner");
	PropOffsets_Commandlet.HelpParm = cls->GetPropertyDataOffset("HelpParm");
	PropOffsets_Commandlet.HelpUsage = cls->GetPropertyDataOffset("HelpUsage");
	PropOffsets_Commandlet.HelpWebLink = cls->GetPropertyDataOffset("HelpWebLink");
	PropOffsets_Commandlet.IsClient = cls->GetPropertyDataOffset("IsClient");
	PropOffsets_Commandlet.IsEditor = cls->GetPropertyDataOffset("IsEditor");
	PropOffsets_Commandlet.IsServer = cls->GetPropertyDataOffset("IsServer");
	PropOffsets_Commandlet.LazyLoad = cls->GetPropertyDataOffset("LazyLoad");
	PropOffsets_Commandlet.LogToStdout = cls->GetPropertyDataOffset("LogToStdout");
	PropOffsets_Commandlet.ShowBanner = cls->GetPropertyDataOffset("ShowBanner");
	PropOffsets_Commandlet.ShowErrorCount = cls->GetPropertyDataOffset("ShowErrorCount");
}

PropertyOffsets_Subsystem PropOffsets_Subsystem;

static void InitPropertyOffsets_Subsystem(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Core")->GetUObject("Class", "Subsystem"));
	if (!cls)
	{
		memset(&PropOffsets_Subsystem, 0xff, sizeof(PropOffsets_Subsystem));
		return;
	}
	PropOffsets_Subsystem.ExecVtbl = cls->GetPropertyDataOffset("ExecVtbl");
}

PropertyOffsets_HelloWorldCommandlet PropOffsets_HelloWorldCommandlet;

static void InitPropertyOffsets_HelloWorldCommandlet(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Core")->GetUObject("Class", "HelloWorldCommandlet"));
	if (!cls)
	{
		memset(&PropOffsets_HelloWorldCommandlet, 0xff, sizeof(PropOffsets_HelloWorldCommandlet));
		return;
	}
	PropOffsets_HelloWorldCommandlet.intparm = cls->GetPropertyDataOffset("intparm");
	PropOffsets_HelloWorldCommandlet.strparm = cls->GetPropertyDataOffset("strparm");
}

PropertyOffsets_SimpleCommandlet PropOffsets_SimpleCommandlet;

static void InitPropertyOffsets_SimpleCommandlet(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Core")->GetUObject("Class", "SimpleCommandlet"));
	if (!cls)
	{
		memset(&PropOffsets_SimpleCommandlet, 0xff, sizeof(PropOffsets_SimpleCommandlet));
		return;
	}
	PropOffsets_SimpleCommandlet.intparm = cls->GetPropertyDataOffset("intparm");
}

PropertyOffsets_Pawn PropOffsets_Pawn;

static void InitPropertyOffsets_Pawn(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Pawn"));
	if (!cls)
	{
		memset(&PropOffsets_Pawn, 0xff, sizeof(PropOffsets_Pawn));
		return;
	}
	PropOffsets_Pawn.AccelRate = cls->GetPropertyDataOffset("AccelRate");
	PropOffsets_Pawn.AirControl = cls->GetPropertyDataOffset("AirControl");
	PropOffsets_Pawn.AirSpeed = cls->GetPropertyDataOffset("AirSpeed");
	PropOffsets_Pawn.AlarmTag = cls->GetPropertyDataOffset("AlarmTag");
	PropOffsets_Pawn.Alertness = cls->GetPropertyDataOffset("Alertness");
	PropOffsets_Pawn.AttitudeToPlayer = cls->GetPropertyDataOffset("AttitudeToPlayer");
	PropOffsets_Pawn.AvgPhysicsTime = cls->GetPropertyDataOffset("AvgPhysicsTime");
	PropOffsets_Pawn.BaseEyeHeight = cls->GetPropertyDataOffset("BaseEyeHeight");
	PropOffsets_Pawn.CombatStyle = cls->GetPropertyDataOffset("CombatStyle");
	PropOffsets_Pawn.DamageScaling = cls->GetPropertyDataOffset("DamageScaling");
	PropOffsets_Pawn.DesiredSpeed = cls->GetPropertyDataOffset("DesiredSpeed");
	PropOffsets_Pawn.Destination = cls->GetPropertyDataOffset("Destination");
	PropOffsets_Pawn.Die = cls->GetPropertyDataOffset("Die");
	PropOffsets_Pawn.DieCount = cls->GetPropertyDataOffset("DieCount");
	PropOffsets_Pawn.DropWhenKilled = cls->GetPropertyDataOffset("DropWhenKilled");
	PropOffsets_Pawn.Enemy = cls->GetPropertyDataOffset("Enemy");
	PropOffsets_Pawn.EyeHeight = cls->GetPropertyDataOffset("EyeHeight");
	PropOffsets_Pawn.FaceTarget = cls->GetPropertyDataOffset("FaceTarget");
	PropOffsets_Pawn.Floor = cls->GetPropertyDataOffset("Floor");
	PropOffsets_Pawn.Focus = cls->GetPropertyDataOffset("Focus");
	PropOffsets_Pawn.FootRegion = cls->GetPropertyDataOffset("FootRegion");
	PropOffsets_Pawn.FovAngle = cls->GetPropertyDataOffset("FovAngle");
	PropOffsets_Pawn.GroundSpeed = cls->GetPropertyDataOffset("GroundSpeed");
	PropOffsets_Pawn.HeadRegion = cls->GetPropertyDataOffset("HeadRegion");
	PropOffsets_Pawn.Health = cls->GetPropertyDataOffset("Health");
	PropOffsets_Pawn.HearingThreshold = cls->GetPropertyDataOffset("HearingThreshold");
	PropOffsets_Pawn.HitSound1 = cls->GetPropertyDataOffset("HitSound1");
	PropOffsets_Pawn.HitSound2 = cls->GetPropertyDataOffset("HitSound2");
	PropOffsets_Pawn.Intelligence = cls->GetPropertyDataOffset("Intelligence");
	PropOffsets_Pawn.ItemCount = cls->GetPropertyDataOffset("ItemCount");
	PropOffsets_Pawn.JumpZ = cls->GetPropertyDataOffset("JumpZ");
	PropOffsets_Pawn.KillCount = cls->GetPropertyDataOffset("KillCount");
	PropOffsets_Pawn.Land = cls->GetPropertyDataOffset("Land");
	PropOffsets_Pawn.LastPainSound = cls->GetPropertyDataOffset("LastPainSound");
	PropOffsets_Pawn.LastSeeingPos = cls->GetPropertyDataOffset("LastSeeingPos");
	PropOffsets_Pawn.LastSeenPos = cls->GetPropertyDataOffset("LastSeenPos");
	PropOffsets_Pawn.LastSeenTime = cls->GetPropertyDataOffset("LastSeenTime");
	PropOffsets_Pawn.MaxDesiredSpeed = cls->GetPropertyDataOffset("MaxDesiredSpeed");
	PropOffsets_Pawn.MaxStepHeight = cls->GetPropertyDataOffset("MaxStepHeight");
	PropOffsets_Pawn.MeleeRange = cls->GetPropertyDataOffset("MeleeRange");
	PropOffsets_Pawn.MenuName = cls->GetPropertyDataOffset("MenuName");
	PropOffsets_Pawn.MinHitWall = cls->GetPropertyDataOffset("MinHitWall");
	PropOffsets_Pawn.MoveTarget = cls->GetPropertyDataOffset("MoveTarget");
	PropOffsets_Pawn.MoveTimer = cls->GetPropertyDataOffset("MoveTimer");
	PropOffsets_Pawn.NameArticle = cls->GetPropertyDataOffset("NameArticle");
	PropOffsets_Pawn.NextLabel = cls->GetPropertyDataOffset("NextLabel");
	PropOffsets_Pawn.NextState = cls->GetPropertyDataOffset("NextState");
	PropOffsets_Pawn.OldMessageTime = cls->GetPropertyDataOffset("OldMessageTime");
	PropOffsets_Pawn.OrthoZoom = cls->GetPropertyDataOffset("OrthoZoom");
	PropOffsets_Pawn.PainTime = cls->GetPropertyDataOffset("PainTime");
	PropOffsets_Pawn.PendingWeapon = cls->GetPropertyDataOffset("PendingWeapon");
	PropOffsets_Pawn.PeripheralVision = cls->GetPropertyDataOffset("PeripheralVision");
	PropOffsets_Pawn.PlayerReStartState = cls->GetPropertyDataOffset("PlayerReStartState");
	PropOffsets_Pawn.PlayerReplicationInfo = cls->GetPropertyDataOffset("PlayerReplicationInfo");
	PropOffsets_Pawn.PlayerReplicationInfoClass = cls->GetPropertyDataOffset("PlayerReplicationInfoClass");
	PropOffsets_Pawn.ReducedDamagePct = cls->GetPropertyDataOffset("ReducedDamagePct");
	PropOffsets_Pawn.ReducedDamageType = cls->GetPropertyDataOffset("ReducedDamageType");
	PropOffsets_Pawn.RouteCache = cls->GetPropertyDataOffset("RouteCache");
	PropOffsets_Pawn.SecretCount = cls->GetPropertyDataOffset("SecretCount");
	PropOffsets_Pawn.SelectedItem = cls->GetPropertyDataOffset("SelectedItem");
	PropOffsets_Pawn.SelectionMesh = cls->GetPropertyDataOffset("SelectionMesh");
	PropOffsets_Pawn.Shadow = cls->GetPropertyDataOffset("Shadow");
	PropOffsets_Pawn.SharedAlarmTag = cls->GetPropertyDataOffset("SharedAlarmTag");
	PropOffsets_Pawn.SightCounter = cls->GetPropertyDataOffset("SightCounter");
	PropOffsets_Pawn.SightRadius = cls->GetPropertyDataOffset("SightRadius");
	PropOffsets_Pawn.Skill = cls->GetPropertyDataOffset("Skill");
	PropOffsets_Pawn.SoundDampening = cls->GetPropertyDataOffset("SoundDampening");
	PropOffsets_Pawn.SpecialGoal = cls->GetPropertyDataOffset("SpecialGoal");
	PropOffsets_Pawn.SpecialMesh = cls->GetPropertyDataOffset("SpecialMesh");
	PropOffsets_Pawn.SpecialPause = cls->GetPropertyDataOffset("SpecialPause");
	PropOffsets_Pawn.SpeechTime = cls->GetPropertyDataOffset("SpeechTime");
	PropOffsets_Pawn.SplashTime = cls->GetPropertyDataOffset("SplashTime");
	PropOffsets_Pawn.Spree = cls->GetPropertyDataOffset("Spree");
	PropOffsets_Pawn.Stimulus = cls->GetPropertyDataOffset("Stimulus");
	PropOffsets_Pawn.UnderWaterTime = cls->GetPropertyDataOffset("UnderWaterTime");
	PropOffsets_Pawn.ViewRotation = cls->GetPropertyDataOffset("ViewRotation");
	PropOffsets_Pawn.Visibility = cls->GetPropertyDataOffset("Visibility");
	PropOffsets_Pawn.VoicePitch = cls->GetPropertyDataOffset("VoicePitch");
	PropOffsets_Pawn.VoiceType = cls->GetPropertyDataOffset("VoiceType");
	PropOffsets_Pawn.WalkBob = cls->GetPropertyDataOffset("WalkBob");
	PropOffsets_Pawn.WaterSpeed = cls->GetPropertyDataOffset("WaterSpeed");
	PropOffsets_Pawn.WaterStep = cls->GetPropertyDataOffset("WaterStep");
	PropOffsets_Pawn.Weapon = cls->GetPropertyDataOffset("Weapon");
	PropOffsets_Pawn.bAdvancedTactics = cls->GetPropertyDataOffset("bAdvancedTactics");
	PropOffsets_Pawn.bAltFire = cls->GetPropertyDataOffset("bAltFire");
	PropOffsets_Pawn.bAutoActivate = cls->GetPropertyDataOffset("bAutoActivate");
	PropOffsets_Pawn.bAvoidLedges = cls->GetPropertyDataOffset("bAvoidLedges");
	PropOffsets_Pawn.bBehindView = cls->GetPropertyDataOffset("bBehindView");
	PropOffsets_Pawn.bCanDoSpecial = cls->GetPropertyDataOffset("bCanDoSpecial");
	PropOffsets_Pawn.bCanFly = cls->GetPropertyDataOffset("bCanFly");
	PropOffsets_Pawn.bCanJump = cls->GetPropertyDataOffset("bCanJump");
	PropOffsets_Pawn.bCanOpenDoors = cls->GetPropertyDataOffset("bCanOpenDoors");
	PropOffsets_Pawn.bCanStrafe = cls->GetPropertyDataOffset("bCanStrafe");
	PropOffsets_Pawn.bCanSwim = cls->GetPropertyDataOffset("bCanSwim");
	PropOffsets_Pawn.bCanWalk = cls->GetPropertyDataOffset("bCanWalk");
	PropOffsets_Pawn.bCountJumps = cls->GetPropertyDataOffset("bCountJumps");
	PropOffsets_Pawn.bDrowning = cls->GetPropertyDataOffset("bDrowning");
	PropOffsets_Pawn.bDuck = cls->GetPropertyDataOffset("bDuck");
	PropOffsets_Pawn.bExtra0 = cls->GetPropertyDataOffset("bExtra0");
	PropOffsets_Pawn.bExtra1 = cls->GetPropertyDataOffset("bExtra1");
	PropOffsets_Pawn.bExtra2 = cls->GetPropertyDataOffset("bExtra2");
	PropOffsets_Pawn.bExtra3 = cls->GetPropertyDataOffset("bExtra3");
	PropOffsets_Pawn.bFire = cls->GetPropertyDataOffset("bFire");
	PropOffsets_Pawn.bFixedStart = cls->GetPropertyDataOffset("bFixedStart");
	PropOffsets_Pawn.bFreeLook = cls->GetPropertyDataOffset("bFreeLook");
	PropOffsets_Pawn.bFromWall = cls->GetPropertyDataOffset("bFromWall");
	PropOffsets_Pawn.bHitSlopedWall = cls->GetPropertyDataOffset("bHitSlopedWall");
	PropOffsets_Pawn.bHunting = cls->GetPropertyDataOffset("bHunting");
	PropOffsets_Pawn.bIsFemale = cls->GetPropertyDataOffset("bIsFemale");
	PropOffsets_Pawn.bIsHuman = cls->GetPropertyDataOffset("bIsHuman");
	PropOffsets_Pawn.bIsMultiSkinned = cls->GetPropertyDataOffset("bIsMultiSkinned");
	PropOffsets_Pawn.bIsPlayer = cls->GetPropertyDataOffset("bIsPlayer");
	PropOffsets_Pawn.bIsWalking = cls->GetPropertyDataOffset("bIsWalking");
	PropOffsets_Pawn.bJumpOffPawn = cls->GetPropertyDataOffset("bJumpOffPawn");
	PropOffsets_Pawn.bJustLanded = cls->GetPropertyDataOffset("bJustLanded");
	PropOffsets_Pawn.bLOSflag = cls->GetPropertyDataOffset("bLOSflag");
	PropOffsets_Pawn.bLook = cls->GetPropertyDataOffset("bLook");
	PropOffsets_Pawn.bNeverSwitchOnPickup = cls->GetPropertyDataOffset("bNeverSwitchOnPickup");
	PropOffsets_Pawn.bReducedSpeed = cls->GetPropertyDataOffset("bReducedSpeed");
	PropOffsets_Pawn.bRun = cls->GetPropertyDataOffset("bRun");
	PropOffsets_Pawn.bShootSpecial = cls->GetPropertyDataOffset("bShootSpecial");
	PropOffsets_Pawn.bSnapLevel = cls->GetPropertyDataOffset("bSnapLevel");
	PropOffsets_Pawn.bStopAtLedges = cls->GetPropertyDataOffset("bStopAtLedges");
	PropOffsets_Pawn.bStrafe = cls->GetPropertyDataOffset("bStrafe");
	PropOffsets_Pawn.bUpAndOut = cls->GetPropertyDataOffset("bUpAndOut");
	PropOffsets_Pawn.bUpdatingDisplay = cls->GetPropertyDataOffset("bUpdatingDisplay");
	PropOffsets_Pawn.bViewTarget = cls->GetPropertyDataOffset("bViewTarget");
	PropOffsets_Pawn.bWarping = cls->GetPropertyDataOffset("bWarping");
	PropOffsets_Pawn.bZoom = cls->GetPropertyDataOffset("bZoom");
	PropOffsets_Pawn.carriedDecoration = cls->GetPropertyDataOffset("carriedDecoration");
	PropOffsets_Pawn.home = cls->GetPropertyDataOffset("home");
	PropOffsets_Pawn.nextPawn = cls->GetPropertyDataOffset("nextPawn");
	PropOffsets_Pawn.noise1loudness = cls->GetPropertyDataOffset("noise1loudness");
	PropOffsets_Pawn.noise1other = cls->GetPropertyDataOffset("noise1other");
	PropOffsets_Pawn.noise1spot = cls->GetPropertyDataOffset("noise1spot");
	PropOffsets_Pawn.noise1time = cls->GetPropertyDataOffset("noise1time");
	PropOffsets_Pawn.noise2loudness = cls->GetPropertyDataOffset("noise2loudness");
	PropOffsets_Pawn.noise2other = cls->GetPropertyDataOffset("noise2other");
	PropOffsets_Pawn.noise2spot = cls->GetPropertyDataOffset("noise2spot");
	PropOffsets_Pawn.noise2time = cls->GetPropertyDataOffset("noise2time");
}

PropertyOffsets_Actor PropOffsets_Actor;

static void InitPropertyOffsets_Actor(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Actor"));
	if (!cls)
	{
		memset(&PropOffsets_Actor, 0xff, sizeof(PropOffsets_Actor));
		return;
	}
	PropOffsets_Actor.Acceleration = cls->GetPropertyDataOffset("Acceleration");
	PropOffsets_Actor.AmbientGlow = cls->GetPropertyDataOffset("AmbientGlow");
	PropOffsets_Actor.AmbientSound = cls->GetPropertyDataOffset("AmbientSound");
	PropOffsets_Actor.AnimFrame = cls->GetPropertyDataOffset("AnimFrame");
	PropOffsets_Actor.AnimLast = cls->GetPropertyDataOffset("AnimLast");
	PropOffsets_Actor.AnimMinRate = cls->GetPropertyDataOffset("AnimMinRate");
	PropOffsets_Actor.AnimRate = cls->GetPropertyDataOffset("AnimRate");
	PropOffsets_Actor.AnimSequence = cls->GetPropertyDataOffset("AnimSequence");
	PropOffsets_Actor.AttachTag = cls->GetPropertyDataOffset("AttachTag");
	PropOffsets_Actor.Base = cls->GetPropertyDataOffset("Base");
	PropOffsets_Actor.Brush = cls->GetPropertyDataOffset("Brush");
	PropOffsets_Actor.Buoyancy = cls->GetPropertyDataOffset("Buoyancy");
	PropOffsets_Actor.ColLocation = cls->GetPropertyDataOffset("ColLocation");
	PropOffsets_Actor.CollisionHeight = cls->GetPropertyDataOffset("CollisionHeight");
	PropOffsets_Actor.CollisionRadius = cls->GetPropertyDataOffset("CollisionRadius");
	PropOffsets_Actor.CollisionTag = cls->GetPropertyDataOffset("CollisionTag");
	PropOffsets_Actor.Deleted = cls->GetPropertyDataOffset("Deleted");
	PropOffsets_Actor.DesiredRotation = cls->GetPropertyDataOffset("DesiredRotation");
	PropOffsets_Actor.DodgeDir = cls->GetPropertyDataOffset("DodgeDir");
	PropOffsets_Actor.DrawScale = cls->GetPropertyDataOffset("DrawScale");
	PropOffsets_Actor.DrawType = cls->GetPropertyDataOffset("DrawType");
	PropOffsets_Actor.Event = cls->GetPropertyDataOffset("Event");
	PropOffsets_Actor.ExtraTag = cls->GetPropertyDataOffset("ExtraTag");
	PropOffsets_Actor.Fatness = cls->GetPropertyDataOffset("Fatness");
	PropOffsets_Actor.Group = cls->GetPropertyDataOffset("Group");
	PropOffsets_Actor.HitActor = cls->GetPropertyDataOffset("HitActor");
	PropOffsets_Actor.InitialState = cls->GetPropertyDataOffset("InitialState");
	PropOffsets_Actor.Instigator = cls->GetPropertyDataOffset("Instigator");
	PropOffsets_Actor.Inventory = cls->GetPropertyDataOffset("Inventory");
	PropOffsets_Actor.LODBias = cls->GetPropertyDataOffset("LODBias");
	PropOffsets_Actor.LatentActor = cls->GetPropertyDataOffset("LatentActor");
	PropOffsets_Actor.LatentByte = cls->GetPropertyDataOffset("LatentByte");
	PropOffsets_Actor.LatentFloat = cls->GetPropertyDataOffset("LatentFloat");
	PropOffsets_Actor.LatentInt = cls->GetPropertyDataOffset("LatentInt");
	PropOffsets_Actor.Level = cls->GetPropertyDataOffset("Level");
	PropOffsets_Actor.LifeSpan = cls->GetPropertyDataOffset("LifeSpan");
	PropOffsets_Actor.LightBrightness = cls->GetPropertyDataOffset("LightBrightness");
	PropOffsets_Actor.LightCone = cls->GetPropertyDataOffset("LightCone");
	PropOffsets_Actor.LightEffect = cls->GetPropertyDataOffset("LightEffect");
	PropOffsets_Actor.LightHue = cls->GetPropertyDataOffset("LightHue");
	PropOffsets_Actor.LightPeriod = cls->GetPropertyDataOffset("LightPeriod");
	PropOffsets_Actor.LightPhase = cls->GetPropertyDataOffset("LightPhase");
	PropOffsets_Actor.LightRadius = cls->GetPropertyDataOffset("LightRadius");
	PropOffsets_Actor.LightSaturation = cls->GetPropertyDataOffset("LightSaturation");
	PropOffsets_Actor.LightType = cls->GetPropertyDataOffset("LightType");
	PropOffsets_Actor.LightingTag = cls->GetPropertyDataOffset("LightingTag");
	PropOffsets_Actor.Location = cls->GetPropertyDataOffset("Location");
	PropOffsets_Actor.Mass = cls->GetPropertyDataOffset("Mass");
	PropOffsets_Actor.Mesh = cls->GetPropertyDataOffset("Mesh");
	PropOffsets_Actor.MiscNumber = cls->GetPropertyDataOffset("MiscNumber");
	PropOffsets_Actor.MultiSkins = cls->GetPropertyDataOffset("MultiSkins");
	PropOffsets_Actor.NetPriority = cls->GetPropertyDataOffset("NetPriority");
	PropOffsets_Actor.NetTag = cls->GetPropertyDataOffset("NetTag");
	PropOffsets_Actor.NetUpdateFrequency = cls->GetPropertyDataOffset("NetUpdateFrequency");
	PropOffsets_Actor.OddsOfAppearing = cls->GetPropertyDataOffset("OddsOfAppearing");
	PropOffsets_Actor.OldAnimRate = cls->GetPropertyDataOffset("OldAnimRate");
	PropOffsets_Actor.OldLocation = cls->GetPropertyDataOffset("OldLocation");
	PropOffsets_Actor.OtherTag = cls->GetPropertyDataOffset("OtherTag");
	PropOffsets_Actor.Owner = cls->GetPropertyDataOffset("Owner");
	PropOffsets_Actor.PendingTouch = cls->GetPropertyDataOffset("PendingTouch");
	PropOffsets_Actor.PhysAlpha = cls->GetPropertyDataOffset("PhysAlpha");
	PropOffsets_Actor.PhysRate = cls->GetPropertyDataOffset("PhysRate");
	PropOffsets_Actor.Physics = cls->GetPropertyDataOffset("Physics");
	PropOffsets_Actor.PrePivot = cls->GetPropertyDataOffset("PrePivot");
	PropOffsets_Actor.Region = cls->GetPropertyDataOffset("Region");
	PropOffsets_Actor.RemoteRole = cls->GetPropertyDataOffset("RemoteRole");
	PropOffsets_Actor.RenderInterface = cls->GetPropertyDataOffset("RenderInterface");
	PropOffsets_Actor.RenderIteratorClass = cls->GetPropertyDataOffset("RenderIteratorClass");
	PropOffsets_Actor.Role = cls->GetPropertyDataOffset("Role");
	PropOffsets_Actor.Rotation = cls->GetPropertyDataOffset("Rotation");
	PropOffsets_Actor.RotationRate = cls->GetPropertyDataOffset("RotationRate");
	PropOffsets_Actor.ScaleGlow = cls->GetPropertyDataOffset("ScaleGlow");
	PropOffsets_Actor.SimAnim = cls->GetPropertyDataOffset("SimAnim");
	PropOffsets_Actor.SkelAnim = cls->GetPropertyDataOffset("SkelAnim");
	PropOffsets_Actor.Skin = cls->GetPropertyDataOffset("Skin");
	PropOffsets_Actor.SoundPitch = cls->GetPropertyDataOffset("SoundPitch");
	PropOffsets_Actor.SoundRadius = cls->GetPropertyDataOffset("SoundRadius");
	PropOffsets_Actor.SoundVolume = cls->GetPropertyDataOffset("SoundVolume");
	PropOffsets_Actor.SpecialTag = cls->GetPropertyDataOffset("SpecialTag");
	PropOffsets_Actor.Sprite = cls->GetPropertyDataOffset("Sprite");
	PropOffsets_Actor.SpriteProjForward = cls->GetPropertyDataOffset("SpriteProjForward");
	PropOffsets_Actor.StandingCount = cls->GetPropertyDataOffset("StandingCount");
	PropOffsets_Actor.Style = cls->GetPropertyDataOffset("Style");
	PropOffsets_Actor.Tag = cls->GetPropertyDataOffset("Tag");
	PropOffsets_Actor.Target = cls->GetPropertyDataOffset("Target");
	PropOffsets_Actor.Texture = cls->GetPropertyDataOffset("Texture");
	PropOffsets_Actor.TimerCounter = cls->GetPropertyDataOffset("TimerCounter");
	PropOffsets_Actor.TimerRate = cls->GetPropertyDataOffset("TimerRate");
	PropOffsets_Actor.Touching = cls->GetPropertyDataOffset("Touching");
	PropOffsets_Actor.TransientSoundRadius = cls->GetPropertyDataOffset("TransientSoundRadius");
	PropOffsets_Actor.TransientSoundVolume = cls->GetPropertyDataOffset("TransientSoundVolume");
	PropOffsets_Actor.TweenRate = cls->GetPropertyDataOffset("TweenRate");
	PropOffsets_Actor.Velocity = cls->GetPropertyDataOffset("Velocity");
	PropOffsets_Actor.VisibilityHeight = cls->GetPropertyDataOffset("VisibilityHeight");
	PropOffsets_Actor.VisibilityRadius = cls->GetPropertyDataOffset("VisibilityRadius");
	PropOffsets_Actor.VolumeBrightness = cls->GetPropertyDataOffset("VolumeBrightness");
	PropOffsets_Actor.VolumeFog = cls->GetPropertyDataOffset("VolumeFog");
	PropOffsets_Actor.VolumeRadius = cls->GetPropertyDataOffset("VolumeRadius");
	PropOffsets_Actor.XLevel = cls->GetPropertyDataOffset("XLevel");
	PropOffsets_Actor.bActorShadows = cls->GetPropertyDataOffset("bActorShadows");
	PropOffsets_Actor.bAlwaysRelevant = cls->GetPropertyDataOffset("bAlwaysRelevant");
	PropOffsets_Actor.bAlwaysTick = cls->GetPropertyDataOffset("bAlwaysTick");
	PropOffsets_Actor.bAnimByOwner = cls->GetPropertyDataOffset("bAnimByOwner");
	PropOffsets_Actor.bAnimFinished = cls->GetPropertyDataOffset("bAnimFinished");
	PropOffsets_Actor.bAnimLoop = cls->GetPropertyDataOffset("bAnimLoop");
	PropOffsets_Actor.bAnimNotify = cls->GetPropertyDataOffset("bAnimNotify");
	PropOffsets_Actor.bAssimilated = cls->GetPropertyDataOffset("bAssimilated");
	PropOffsets_Actor.bBlockActors = cls->GetPropertyDataOffset("bBlockActors");
	PropOffsets_Actor.bBlockPlayers = cls->GetPropertyDataOffset("bBlockPlayers");
	PropOffsets_Actor.bBounce = cls->GetPropertyDataOffset("bBounce");
	PropOffsets_Actor.bCanTeleport = cls->GetPropertyDataOffset("bCanTeleport");
	PropOffsets_Actor.bCarriedItem = cls->GetPropertyDataOffset("bCarriedItem");
	PropOffsets_Actor.bClientAnim = cls->GetPropertyDataOffset("bClientAnim");
	PropOffsets_Actor.bClientDemoNetFunc = cls->GetPropertyDataOffset("bClientDemoNetFunc");
	PropOffsets_Actor.bClientDemoRecording = cls->GetPropertyDataOffset("bClientDemoRecording");
	PropOffsets_Actor.bCollideActors = cls->GetPropertyDataOffset("bCollideActors");
	PropOffsets_Actor.bCollideWhenPlacing = cls->GetPropertyDataOffset("bCollideWhenPlacing");
	PropOffsets_Actor.bCollideWorld = cls->GetPropertyDataOffset("bCollideWorld");
	PropOffsets_Actor.bCorona = cls->GetPropertyDataOffset("bCorona");
	PropOffsets_Actor.bDeleteMe = cls->GetPropertyDataOffset("bDeleteMe");
	PropOffsets_Actor.bDemoRecording = cls->GetPropertyDataOffset("bDemoRecording");
	PropOffsets_Actor.bDifficulty0 = cls->GetPropertyDataOffset("bDifficulty0");
	PropOffsets_Actor.bDifficulty1 = cls->GetPropertyDataOffset("bDifficulty1");
	PropOffsets_Actor.bDifficulty2 = cls->GetPropertyDataOffset("bDifficulty2");
	PropOffsets_Actor.bDifficulty3 = cls->GetPropertyDataOffset("bDifficulty3");
	PropOffsets_Actor.bDirectional = cls->GetPropertyDataOffset("bDirectional");
	PropOffsets_Actor.bDynamicLight = cls->GetPropertyDataOffset("bDynamicLight");
	PropOffsets_Actor.bEdLocked = cls->GetPropertyDataOffset("bEdLocked");
	PropOffsets_Actor.bEdShouldSnap = cls->GetPropertyDataOffset("bEdShouldSnap");
	PropOffsets_Actor.bEdSnap = cls->GetPropertyDataOffset("bEdSnap");
	PropOffsets_Actor.bFilterByVolume = cls->GetPropertyDataOffset("bFilterByVolume");
	PropOffsets_Actor.bFixedRotationDir = cls->GetPropertyDataOffset("bFixedRotationDir");
	PropOffsets_Actor.bForcePhysicsUpdate = cls->GetPropertyDataOffset("bForcePhysicsUpdate");
	PropOffsets_Actor.bForceStasis = cls->GetPropertyDataOffset("bForceStasis");
	PropOffsets_Actor.bGameRelevant = cls->GetPropertyDataOffset("bGameRelevant");
	PropOffsets_Actor.bHidden = cls->GetPropertyDataOffset("bHidden");
	PropOffsets_Actor.bHiddenEd = cls->GetPropertyDataOffset("bHiddenEd");
	PropOffsets_Actor.bHighDetail = cls->GetPropertyDataOffset("bHighDetail");
	PropOffsets_Actor.bHighlighted = cls->GetPropertyDataOffset("bHighlighted");
	PropOffsets_Actor.bHurtEntry = cls->GetPropertyDataOffset("bHurtEntry");
	PropOffsets_Actor.bInterpolating = cls->GetPropertyDataOffset("bInterpolating");
	PropOffsets_Actor.bIsItemGoal = cls->GetPropertyDataOffset("bIsItemGoal");
	PropOffsets_Actor.bIsKillGoal = cls->GetPropertyDataOffset("bIsKillGoal");
	PropOffsets_Actor.bIsMover = cls->GetPropertyDataOffset("bIsMover");
	PropOffsets_Actor.bIsPawn = cls->GetPropertyDataOffset("bIsPawn");
	PropOffsets_Actor.bIsSecretGoal = cls->GetPropertyDataOffset("bIsSecretGoal");
	PropOffsets_Actor.bJustTeleported = cls->GetPropertyDataOffset("bJustTeleported");
	PropOffsets_Actor.bLensFlare = cls->GetPropertyDataOffset("bLensFlare");
	PropOffsets_Actor.bLightChanged = cls->GetPropertyDataOffset("bLightChanged");
	PropOffsets_Actor.bMemorized = cls->GetPropertyDataOffset("bMemorized");
	PropOffsets_Actor.bMeshCurvy = cls->GetPropertyDataOffset("bMeshCurvy");
	PropOffsets_Actor.bMeshEnviroMap = cls->GetPropertyDataOffset("bMeshEnviroMap");
	PropOffsets_Actor.bMovable = cls->GetPropertyDataOffset("bMovable");
	PropOffsets_Actor.bNet = cls->GetPropertyDataOffset("bNet");
	PropOffsets_Actor.bNetFeel = cls->GetPropertyDataOffset("bNetFeel");
	PropOffsets_Actor.bNetHear = cls->GetPropertyDataOffset("bNetHear");
	PropOffsets_Actor.bNetInitial = cls->GetPropertyDataOffset("bNetInitial");
	PropOffsets_Actor.bNetOptional = cls->GetPropertyDataOffset("bNetOptional");
	PropOffsets_Actor.bNetOwner = cls->GetPropertyDataOffset("bNetOwner");
	PropOffsets_Actor.bNetRelevant = cls->GetPropertyDataOffset("bNetRelevant");
	PropOffsets_Actor.bNetSee = cls->GetPropertyDataOffset("bNetSee");
	PropOffsets_Actor.bNetSpecial = cls->GetPropertyDataOffset("bNetSpecial");
	PropOffsets_Actor.bNetTemporary = cls->GetPropertyDataOffset("bNetTemporary");
	PropOffsets_Actor.bNoDelete = cls->GetPropertyDataOffset("bNoDelete");
	PropOffsets_Actor.bNoSmooth = cls->GetPropertyDataOffset("bNoSmooth");
	PropOffsets_Actor.bOnlyOwnerSee = cls->GetPropertyDataOffset("bOnlyOwnerSee");
	PropOffsets_Actor.bOwnerNoSee = cls->GetPropertyDataOffset("bOwnerNoSee");
	PropOffsets_Actor.bParticles = cls->GetPropertyDataOffset("bParticles");
	PropOffsets_Actor.bProjTarget = cls->GetPropertyDataOffset("bProjTarget");
	PropOffsets_Actor.bRandomFrame = cls->GetPropertyDataOffset("bRandomFrame");
	PropOffsets_Actor.bReplicateInstigator = cls->GetPropertyDataOffset("bReplicateInstigator");
	PropOffsets_Actor.bRotateToDesired = cls->GetPropertyDataOffset("bRotateToDesired");
	PropOffsets_Actor.bScriptInitialized = cls->GetPropertyDataOffset("bScriptInitialized");
	PropOffsets_Actor.bSelected = cls->GetPropertyDataOffset("bSelected");
	PropOffsets_Actor.bShadowCast = cls->GetPropertyDataOffset("bShadowCast");
	PropOffsets_Actor.bSimFall = cls->GetPropertyDataOffset("bSimFall");
	PropOffsets_Actor.bSimulatedPawn = cls->GetPropertyDataOffset("bSimulatedPawn");
	PropOffsets_Actor.bSinglePlayer = cls->GetPropertyDataOffset("bSinglePlayer");
	PropOffsets_Actor.bSpecialLit = cls->GetPropertyDataOffset("bSpecialLit");
	PropOffsets_Actor.bStasis = cls->GetPropertyDataOffset("bStasis");
	PropOffsets_Actor.bStatic = cls->GetPropertyDataOffset("bStatic");
	PropOffsets_Actor.bTempEditor = cls->GetPropertyDataOffset("bTempEditor");
	PropOffsets_Actor.bTicked = cls->GetPropertyDataOffset("bTicked");
	PropOffsets_Actor.bTimerLoop = cls->GetPropertyDataOffset("bTimerLoop");
	PropOffsets_Actor.bTrailerPrePivot = cls->GetPropertyDataOffset("bTrailerPrePivot");
	PropOffsets_Actor.bTrailerSameRotation = cls->GetPropertyDataOffset("bTrailerSameRotation");
	PropOffsets_Actor.bTravel = cls->GetPropertyDataOffset("bTravel");
	PropOffsets_Actor.bUnlit = cls->GetPropertyDataOffset("bUnlit");
}

PropertyOffsets_LevelInfo PropOffsets_LevelInfo;

static void InitPropertyOffsets_LevelInfo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "LevelInfo"));
	if (!cls)
	{
		memset(&PropOffsets_LevelInfo, 0xff, sizeof(PropOffsets_LevelInfo));
		return;
	}
	PropOffsets_LevelInfo.AIProfile = cls->GetPropertyDataOffset("AIProfile");
	PropOffsets_LevelInfo.Author = cls->GetPropertyDataOffset("Author");
	PropOffsets_LevelInfo.AvgAITime = cls->GetPropertyDataOffset("AvgAITime");
	PropOffsets_LevelInfo.Brightness = cls->GetPropertyDataOffset("Brightness");
	PropOffsets_LevelInfo.CdTrack = cls->GetPropertyDataOffset("CdTrack");
	PropOffsets_LevelInfo.ComputerName = cls->GetPropertyDataOffset("ComputerName");
	PropOffsets_LevelInfo.Day = cls->GetPropertyDataOffset("Day");
	PropOffsets_LevelInfo.DayOfWeek = cls->GetPropertyDataOffset("DayOfWeek");
	PropOffsets_LevelInfo.DefaultGameType = cls->GetPropertyDataOffset("DefaultGameType");
	PropOffsets_LevelInfo.DefaultTexture = cls->GetPropertyDataOffset("DefaultTexture");
	PropOffsets_LevelInfo.EngineVersion = cls->GetPropertyDataOffset("EngineVersion");
	PropOffsets_LevelInfo.Game = cls->GetPropertyDataOffset("Game");
	PropOffsets_LevelInfo.Hour = cls->GetPropertyDataOffset("Hour");
	PropOffsets_LevelInfo.HubStackLevel = cls->GetPropertyDataOffset("HubStackLevel");
	PropOffsets_LevelInfo.IdealPlayerCount = cls->GetPropertyDataOffset("IdealPlayerCount");
	PropOffsets_LevelInfo.LevelAction = cls->GetPropertyDataOffset("LevelAction");
	PropOffsets_LevelInfo.LevelEnterText = cls->GetPropertyDataOffset("LevelEnterText");
	PropOffsets_LevelInfo.LocalizedPkg = cls->GetPropertyDataOffset("LocalizedPkg");
	PropOffsets_LevelInfo.Millisecond = cls->GetPropertyDataOffset("Millisecond");
	PropOffsets_LevelInfo.MinNetVersion = cls->GetPropertyDataOffset("MinNetVersion");
	PropOffsets_LevelInfo.Minute = cls->GetPropertyDataOffset("Minute");
	PropOffsets_LevelInfo.Month = cls->GetPropertyDataOffset("Month");
	PropOffsets_LevelInfo.NavigationPointList = cls->GetPropertyDataOffset("NavigationPointList");
	PropOffsets_LevelInfo.NetMode = cls->GetPropertyDataOffset("NetMode");
	PropOffsets_LevelInfo.NextSwitchCountdown = cls->GetPropertyDataOffset("NextSwitchCountdown");
	PropOffsets_LevelInfo.NextURL = cls->GetPropertyDataOffset("NextURL");
	PropOffsets_LevelInfo.Pauser = cls->GetPropertyDataOffset("Pauser");
	PropOffsets_LevelInfo.PawnList = cls->GetPropertyDataOffset("PawnList");
	PropOffsets_LevelInfo.PlayerDoppler = cls->GetPropertyDataOffset("PlayerDoppler");
	PropOffsets_LevelInfo.RecommendedEnemies = cls->GetPropertyDataOffset("RecommendedEnemies");
	PropOffsets_LevelInfo.RecommendedTeammates = cls->GetPropertyDataOffset("RecommendedTeammates");
	PropOffsets_LevelInfo.Screenshot = cls->GetPropertyDataOffset("Screenshot");
	PropOffsets_LevelInfo.Second = cls->GetPropertyDataOffset("Second");
	PropOffsets_LevelInfo.Song = cls->GetPropertyDataOffset("Song");
	PropOffsets_LevelInfo.SongSection = cls->GetPropertyDataOffset("SongSection");
	PropOffsets_LevelInfo.SpawnNotify = cls->GetPropertyDataOffset("SpawnNotify");
	PropOffsets_LevelInfo.Summary = cls->GetPropertyDataOffset("Summary");
	PropOffsets_LevelInfo.TimeDilation = cls->GetPropertyDataOffset("TimeDilation");
	PropOffsets_LevelInfo.TimeSeconds = cls->GetPropertyDataOffset("TimeSeconds");
	PropOffsets_LevelInfo.Title = cls->GetPropertyDataOffset("Title");
	PropOffsets_LevelInfo.VisibleGroups = cls->GetPropertyDataOffset("VisibleGroups");
	PropOffsets_LevelInfo.Year = cls->GetPropertyDataOffset("Year");
	PropOffsets_LevelInfo.bAggressiveLOD = cls->GetPropertyDataOffset("bAggressiveLOD");
	PropOffsets_LevelInfo.bAllowFOV = cls->GetPropertyDataOffset("bAllowFOV");
	PropOffsets_LevelInfo.bBegunPlay = cls->GetPropertyDataOffset("bBegunPlay");
	PropOffsets_LevelInfo.bCheckWalkSurfaces = cls->GetPropertyDataOffset("bCheckWalkSurfaces");
	PropOffsets_LevelInfo.bDropDetail = cls->GetPropertyDataOffset("bDropDetail");
	PropOffsets_LevelInfo.bHighDetailMode = cls->GetPropertyDataOffset("bHighDetailMode");
	PropOffsets_LevelInfo.bHumansOnly = cls->GetPropertyDataOffset("bHumansOnly");
	PropOffsets_LevelInfo.bLonePlayer = cls->GetPropertyDataOffset("bLonePlayer");
	PropOffsets_LevelInfo.bLowRes = cls->GetPropertyDataOffset("bLowRes");
	PropOffsets_LevelInfo.bNeverPrecache = cls->GetPropertyDataOffset("bNeverPrecache");
	PropOffsets_LevelInfo.bNextItems = cls->GetPropertyDataOffset("bNextItems");
	PropOffsets_LevelInfo.bNoCheating = cls->GetPropertyDataOffset("bNoCheating");
	PropOffsets_LevelInfo.bPlayersOnly = cls->GetPropertyDataOffset("bPlayersOnly");
	PropOffsets_LevelInfo.bStartup = cls->GetPropertyDataOffset("bStartup");
}

PropertyOffsets_Inventory PropOffsets_Inventory;

static void InitPropertyOffsets_Inventory(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Inventory"));
	if (!cls)
	{
		memset(&PropOffsets_Inventory, 0xff, sizeof(PropOffsets_Inventory));
		return;
	}
	PropOffsets_Inventory.AbsorptionPriority = cls->GetPropertyDataOffset("AbsorptionPriority");
	PropOffsets_Inventory.ActivateSound = cls->GetPropertyDataOffset("ActivateSound");
	PropOffsets_Inventory.ArmorAbsorption = cls->GetPropertyDataOffset("ArmorAbsorption");
	PropOffsets_Inventory.AutoSwitchPriority = cls->GetPropertyDataOffset("AutoSwitchPriority");
	PropOffsets_Inventory.BobDamping = cls->GetPropertyDataOffset("BobDamping");
	PropOffsets_Inventory.Charge = cls->GetPropertyDataOffset("Charge");
	PropOffsets_Inventory.DeActivateSound = cls->GetPropertyDataOffset("DeActivateSound");
	PropOffsets_Inventory.FlashCount = cls->GetPropertyDataOffset("FlashCount");
	PropOffsets_Inventory.Icon = cls->GetPropertyDataOffset("Icon");
	PropOffsets_Inventory.InventoryGroup = cls->GetPropertyDataOffset("InventoryGroup");
	PropOffsets_Inventory.ItemArticle = cls->GetPropertyDataOffset("ItemArticle");
	PropOffsets_Inventory.ItemMessageClass = cls->GetPropertyDataOffset("ItemMessageClass");
	PropOffsets_Inventory.ItemName = cls->GetPropertyDataOffset("ItemName");
	PropOffsets_Inventory.M_Activated = cls->GetPropertyDataOffset("M_Activated");
	PropOffsets_Inventory.M_Deactivated = cls->GetPropertyDataOffset("M_Deactivated");
	PropOffsets_Inventory.M_Selected = cls->GetPropertyDataOffset("M_Selected");
	PropOffsets_Inventory.MaxDesireability = cls->GetPropertyDataOffset("MaxDesireability");
	PropOffsets_Inventory.MuzzleFlashMesh = cls->GetPropertyDataOffset("MuzzleFlashMesh");
	PropOffsets_Inventory.MuzzleFlashScale = cls->GetPropertyDataOffset("MuzzleFlashScale");
	PropOffsets_Inventory.MuzzleFlashStyle = cls->GetPropertyDataOffset("MuzzleFlashStyle");
	PropOffsets_Inventory.MuzzleFlashTexture = cls->GetPropertyDataOffset("MuzzleFlashTexture");
	PropOffsets_Inventory.NextArmor = cls->GetPropertyDataOffset("NextArmor");
	PropOffsets_Inventory.OldFlashCount = cls->GetPropertyDataOffset("OldFlashCount");
	PropOffsets_Inventory.PickupMessage = cls->GetPropertyDataOffset("PickupMessage");
	PropOffsets_Inventory.PickupMessageClass = cls->GetPropertyDataOffset("PickupMessageClass");
	PropOffsets_Inventory.PickupSound = cls->GetPropertyDataOffset("PickupSound");
	PropOffsets_Inventory.PickupViewMesh = cls->GetPropertyDataOffset("PickupViewMesh");
	PropOffsets_Inventory.PickupViewScale = cls->GetPropertyDataOffset("PickupViewScale");
	PropOffsets_Inventory.PlayerLastTouched = cls->GetPropertyDataOffset("PlayerLastTouched");
	PropOffsets_Inventory.PlayerViewMesh = cls->GetPropertyDataOffset("PlayerViewMesh");
	PropOffsets_Inventory.PlayerViewOffset = cls->GetPropertyDataOffset("PlayerViewOffset");
	PropOffsets_Inventory.PlayerViewScale = cls->GetPropertyDataOffset("PlayerViewScale");
	PropOffsets_Inventory.ProtectionType1 = cls->GetPropertyDataOffset("ProtectionType1");
	PropOffsets_Inventory.ProtectionType2 = cls->GetPropertyDataOffset("ProtectionType2");
	PropOffsets_Inventory.RespawnSound = cls->GetPropertyDataOffset("RespawnSound");
	PropOffsets_Inventory.RespawnTime = cls->GetPropertyDataOffset("RespawnTime");
	PropOffsets_Inventory.StatusIcon = cls->GetPropertyDataOffset("StatusIcon");
	PropOffsets_Inventory.ThirdPersonMesh = cls->GetPropertyDataOffset("ThirdPersonMesh");
	PropOffsets_Inventory.ThirdPersonScale = cls->GetPropertyDataOffset("ThirdPersonScale");
	PropOffsets_Inventory.bActivatable = cls->GetPropertyDataOffset("bActivatable");
	PropOffsets_Inventory.bActive = cls->GetPropertyDataOffset("bActive");
	PropOffsets_Inventory.bAmbientGlow = cls->GetPropertyDataOffset("bAmbientGlow");
	PropOffsets_Inventory.bDisplayableInv = cls->GetPropertyDataOffset("bDisplayableInv");
	PropOffsets_Inventory.bFirstFrame = cls->GetPropertyDataOffset("bFirstFrame");
	PropOffsets_Inventory.bHeldItem = cls->GetPropertyDataOffset("bHeldItem");
	PropOffsets_Inventory.bInstantRespawn = cls->GetPropertyDataOffset("bInstantRespawn");
	PropOffsets_Inventory.bIsAnArmor = cls->GetPropertyDataOffset("bIsAnArmor");
	PropOffsets_Inventory.bMuzzleFlashParticles = cls->GetPropertyDataOffset("bMuzzleFlashParticles");
	PropOffsets_Inventory.bRotatingPickup = cls->GetPropertyDataOffset("bRotatingPickup");
	PropOffsets_Inventory.bSleepTouch = cls->GetPropertyDataOffset("bSleepTouch");
	PropOffsets_Inventory.bSteadyFlash3rd = cls->GetPropertyDataOffset("bSteadyFlash3rd");
	PropOffsets_Inventory.bSteadyToggle = cls->GetPropertyDataOffset("bSteadyToggle");
	PropOffsets_Inventory.bToggleSteadyFlash = cls->GetPropertyDataOffset("bToggleSteadyFlash");
	PropOffsets_Inventory.bTossedOut = cls->GetPropertyDataOffset("bTossedOut");
	PropOffsets_Inventory.myMarker = cls->GetPropertyDataOffset("myMarker");
}

PropertyOffsets_PlayerPawn PropOffsets_PlayerPawn;

static void InitPropertyOffsets_PlayerPawn(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "PlayerPawn"));
	if (!cls)
	{
		memset(&PropOffsets_PlayerPawn, 0xff, sizeof(PropOffsets_PlayerPawn));
		return;
	}
	PropOffsets_PlayerPawn.AppliedBob = cls->GetPropertyDataOffset("AppliedBob");
	PropOffsets_PlayerPawn.Bob = cls->GetPropertyDataOffset("Bob");
	PropOffsets_PlayerPawn.BorrowedMouseX = cls->GetPropertyDataOffset("BorrowedMouseX");
	PropOffsets_PlayerPawn.BorrowedMouseY = cls->GetPropertyDataOffset("BorrowedMouseY");
	PropOffsets_PlayerPawn.CarcassType = cls->GetPropertyDataOffset("CarcassType");
	PropOffsets_PlayerPawn.CdTrack = cls->GetPropertyDataOffset("CdTrack");
	PropOffsets_PlayerPawn.ClientUpdateTime = cls->GetPropertyDataOffset("ClientUpdateTime");
	PropOffsets_PlayerPawn.ConstantGlowFog = cls->GetPropertyDataOffset("ConstantGlowFog");
	PropOffsets_PlayerPawn.ConstantGlowScale = cls->GetPropertyDataOffset("ConstantGlowScale");
	PropOffsets_PlayerPawn.CurrentTimeStamp = cls->GetPropertyDataOffset("CurrentTimeStamp");
	PropOffsets_PlayerPawn.DefaultFOV = cls->GetPropertyDataOffset("DefaultFOV");
	PropOffsets_PlayerPawn.DelayedCommand = cls->GetPropertyDataOffset("DelayedCommand");
	PropOffsets_PlayerPawn.DemoViewPitch = cls->GetPropertyDataOffset("DemoViewPitch");
	PropOffsets_PlayerPawn.DemoViewYaw = cls->GetPropertyDataOffset("DemoViewYaw");
	PropOffsets_PlayerPawn.DesiredFOV = cls->GetPropertyDataOffset("DesiredFOV");
	PropOffsets_PlayerPawn.DesiredFlashFog = cls->GetPropertyDataOffset("DesiredFlashFog");
	PropOffsets_PlayerPawn.DesiredFlashScale = cls->GetPropertyDataOffset("DesiredFlashScale");
	PropOffsets_PlayerPawn.DodgeClickTime = cls->GetPropertyDataOffset("DodgeClickTime");
	PropOffsets_PlayerPawn.DodgeClickTimer = cls->GetPropertyDataOffset("DodgeClickTimer");
	PropOffsets_PlayerPawn.FailedView = cls->GetPropertyDataOffset("FailedView");
	PropOffsets_PlayerPawn.FlashFog = cls->GetPropertyDataOffset("FlashFog");
	PropOffsets_PlayerPawn.FlashScale = cls->GetPropertyDataOffset("FlashScale");
	PropOffsets_PlayerPawn.FreeMoves = cls->GetPropertyDataOffset("FreeMoves");
	PropOffsets_PlayerPawn.GameReplicationInfo = cls->GetPropertyDataOffset("GameReplicationInfo");
	PropOffsets_PlayerPawn.HUDType = cls->GetPropertyDataOffset("HUDType");
	PropOffsets_PlayerPawn.Handedness = cls->GetPropertyDataOffset("Handedness");
	PropOffsets_PlayerPawn.InstantFlash = cls->GetPropertyDataOffset("InstantFlash");
	PropOffsets_PlayerPawn.InstantFog = cls->GetPropertyDataOffset("InstantFog");
	PropOffsets_PlayerPawn.JumpSound = cls->GetPropertyDataOffset("JumpSound");
	PropOffsets_PlayerPawn.LandBob = cls->GetPropertyDataOffset("LandBob");
	PropOffsets_PlayerPawn.LastMessageWindow = cls->GetPropertyDataOffset("LastMessageWindow");
	PropOffsets_PlayerPawn.LastPlaySound = cls->GetPropertyDataOffset("LastPlaySound");
	PropOffsets_PlayerPawn.LastUpdateTime = cls->GetPropertyDataOffset("LastUpdateTime");
	PropOffsets_PlayerPawn.MaxTimeMargin = cls->GetPropertyDataOffset("MaxTimeMargin");
	PropOffsets_PlayerPawn.Misc1 = cls->GetPropertyDataOffset("Misc1");
	PropOffsets_PlayerPawn.Misc2 = cls->GetPropertyDataOffset("Misc2");
	PropOffsets_PlayerPawn.MouseSensitivity = cls->GetPropertyDataOffset("MouseSensitivity");
	PropOffsets_PlayerPawn.MouseSmoothThreshold = cls->GetPropertyDataOffset("MouseSmoothThreshold");
	PropOffsets_PlayerPawn.MouseZeroTime = cls->GetPropertyDataOffset("MouseZeroTime");
	PropOffsets_PlayerPawn.MyAutoAim = cls->GetPropertyDataOffset("MyAutoAim");
	PropOffsets_PlayerPawn.NoPauseMessage = cls->GetPropertyDataOffset("NoPauseMessage");
	PropOffsets_PlayerPawn.OwnCamera = cls->GetPropertyDataOffset("OwnCamera");
	PropOffsets_PlayerPawn.Password = cls->GetPropertyDataOffset("Password");
	PropOffsets_PlayerPawn.PendingMove = cls->GetPropertyDataOffset("PendingMove");
	PropOffsets_PlayerPawn.Player = cls->GetPropertyDataOffset("Player");
	PropOffsets_PlayerPawn.ProgressColor = cls->GetPropertyDataOffset("ProgressColor");
	PropOffsets_PlayerPawn.ProgressMessage = cls->GetPropertyDataOffset("ProgressMessage");
	PropOffsets_PlayerPawn.ProgressTimeOut = cls->GetPropertyDataOffset("ProgressTimeOut");
	PropOffsets_PlayerPawn.QuickSaveString = cls->GetPropertyDataOffset("QuickSaveString");
	PropOffsets_PlayerPawn.ReceivedSecretChecksum = cls->GetPropertyDataOffset("ReceivedSecretChecksum");
	PropOffsets_PlayerPawn.RendMap = cls->GetPropertyDataOffset("RendMap");
	PropOffsets_PlayerPawn.SavedMoves = cls->GetPropertyDataOffset("SavedMoves");
	PropOffsets_PlayerPawn.Scoring = cls->GetPropertyDataOffset("Scoring");
	PropOffsets_PlayerPawn.ScoringType = cls->GetPropertyDataOffset("ScoringType");
	PropOffsets_PlayerPawn.ServerTimeStamp = cls->GetPropertyDataOffset("ServerTimeStamp");
	PropOffsets_PlayerPawn.ShowFlags = cls->GetPropertyDataOffset("ShowFlags");
	PropOffsets_PlayerPawn.SmoothMouseX = cls->GetPropertyDataOffset("SmoothMouseX");
	PropOffsets_PlayerPawn.SmoothMouseY = cls->GetPropertyDataOffset("SmoothMouseY");
	PropOffsets_PlayerPawn.Song = cls->GetPropertyDataOffset("Song");
	PropOffsets_PlayerPawn.SongSection = cls->GetPropertyDataOffset("SongSection");
	PropOffsets_PlayerPawn.SpecialMenu = cls->GetPropertyDataOffset("SpecialMenu");
	PropOffsets_PlayerPawn.TargetEyeHeight = cls->GetPropertyDataOffset("TargetEyeHeight");
	PropOffsets_PlayerPawn.TargetViewRotation = cls->GetPropertyDataOffset("TargetViewRotation");
	PropOffsets_PlayerPawn.TargetWeaponViewOffset = cls->GetPropertyDataOffset("TargetWeaponViewOffset");
	PropOffsets_PlayerPawn.TimeMargin = cls->GetPropertyDataOffset("TimeMargin");
	PropOffsets_PlayerPawn.Transition = cls->GetPropertyDataOffset("Transition");
	PropOffsets_PlayerPawn.ViewTarget = cls->GetPropertyDataOffset("ViewTarget");
	PropOffsets_PlayerPawn.ViewingFrom = cls->GetPropertyDataOffset("ViewingFrom");
	PropOffsets_PlayerPawn.WeaponPriority = cls->GetPropertyDataOffset("WeaponPriority");
	PropOffsets_PlayerPawn.ZoomLevel = cls->GetPropertyDataOffset("ZoomLevel");
	PropOffsets_PlayerPawn.aBaseX = cls->GetPropertyDataOffset("aBaseX");
	PropOffsets_PlayerPawn.aBaseY = cls->GetPropertyDataOffset("aBaseY");
	PropOffsets_PlayerPawn.aBaseZ = cls->GetPropertyDataOffset("aBaseZ");
	PropOffsets_PlayerPawn.aExtra0 = cls->GetPropertyDataOffset("aExtra0");
	PropOffsets_PlayerPawn.aExtra1 = cls->GetPropertyDataOffset("aExtra1");
	PropOffsets_PlayerPawn.aExtra2 = cls->GetPropertyDataOffset("aExtra2");
	PropOffsets_PlayerPawn.aExtra3 = cls->GetPropertyDataOffset("aExtra3");
	PropOffsets_PlayerPawn.aExtra4 = cls->GetPropertyDataOffset("aExtra4");
	PropOffsets_PlayerPawn.aForward = cls->GetPropertyDataOffset("aForward");
	PropOffsets_PlayerPawn.aLookUp = cls->GetPropertyDataOffset("aLookUp");
	PropOffsets_PlayerPawn.aMouseX = cls->GetPropertyDataOffset("aMouseX");
	PropOffsets_PlayerPawn.aMouseY = cls->GetPropertyDataOffset("aMouseY");
	PropOffsets_PlayerPawn.aStrafe = cls->GetPropertyDataOffset("aStrafe");
	PropOffsets_PlayerPawn.aTurn = cls->GetPropertyDataOffset("aTurn");
	PropOffsets_PlayerPawn.aUp = cls->GetPropertyDataOffset("aUp");
	PropOffsets_PlayerPawn.bAdmin = cls->GetPropertyDataOffset("bAdmin");
	PropOffsets_PlayerPawn.bAlwaysMouseLook = cls->GetPropertyDataOffset("bAlwaysMouseLook");
	PropOffsets_PlayerPawn.bAnimTransition = cls->GetPropertyDataOffset("bAnimTransition");
	PropOffsets_PlayerPawn.bBadConnectionAlert = cls->GetPropertyDataOffset("bBadConnectionAlert");
	PropOffsets_PlayerPawn.bCenterView = cls->GetPropertyDataOffset("bCenterView");
	PropOffsets_PlayerPawn.bCheatsEnabled = cls->GetPropertyDataOffset("bCheatsEnabled");
	PropOffsets_PlayerPawn.bDelayedCommand = cls->GetPropertyDataOffset("bDelayedCommand");
	PropOffsets_PlayerPawn.bEdgeBack = cls->GetPropertyDataOffset("bEdgeBack");
	PropOffsets_PlayerPawn.bEdgeForward = cls->GetPropertyDataOffset("bEdgeForward");
	PropOffsets_PlayerPawn.bEdgeLeft = cls->GetPropertyDataOffset("bEdgeLeft");
	PropOffsets_PlayerPawn.bEdgeRight = cls->GetPropertyDataOffset("bEdgeRight");
	PropOffsets_PlayerPawn.bFixedCamera = cls->GetPropertyDataOffset("bFixedCamera");
	PropOffsets_PlayerPawn.bFrozen = cls->GetPropertyDataOffset("bFrozen");
	PropOffsets_PlayerPawn.bInvertMouse = cls->GetPropertyDataOffset("bInvertMouse");
	PropOffsets_PlayerPawn.bIsCrouching = cls->GetPropertyDataOffset("bIsCrouching");
	PropOffsets_PlayerPawn.bIsTurning = cls->GetPropertyDataOffset("bIsTurning");
	PropOffsets_PlayerPawn.bIsTyping = cls->GetPropertyDataOffset("bIsTyping");
	PropOffsets_PlayerPawn.bJumpStatus = cls->GetPropertyDataOffset("bJumpStatus");
	PropOffsets_PlayerPawn.bJustAltFired = cls->GetPropertyDataOffset("bJustAltFired");
	PropOffsets_PlayerPawn.bJustFired = cls->GetPropertyDataOffset("bJustFired");
	PropOffsets_PlayerPawn.bKeyboardLook = cls->GetPropertyDataOffset("bKeyboardLook");
	PropOffsets_PlayerPawn.bLookUpStairs = cls->GetPropertyDataOffset("bLookUpStairs");
	PropOffsets_PlayerPawn.bMaxMouseSmoothing = cls->GetPropertyDataOffset("bMaxMouseSmoothing");
	PropOffsets_PlayerPawn.bMessageBeep = cls->GetPropertyDataOffset("bMessageBeep");
	PropOffsets_PlayerPawn.bMouseZeroed = cls->GetPropertyDataOffset("bMouseZeroed");
	PropOffsets_PlayerPawn.bNeverAutoSwitch = cls->GetPropertyDataOffset("bNeverAutoSwitch");
	PropOffsets_PlayerPawn.bNoFlash = cls->GetPropertyDataOffset("bNoFlash");
	PropOffsets_PlayerPawn.bNoVoices = cls->GetPropertyDataOffset("bNoVoices");
	PropOffsets_PlayerPawn.bPressedJump = cls->GetPropertyDataOffset("bPressedJump");
	PropOffsets_PlayerPawn.bReadyToPlay = cls->GetPropertyDataOffset("bReadyToPlay");
	PropOffsets_PlayerPawn.bReducedVis = cls->GetPropertyDataOffset("bReducedVis");
	PropOffsets_PlayerPawn.bRising = cls->GetPropertyDataOffset("bRising");
	PropOffsets_PlayerPawn.bShakeDir = cls->GetPropertyDataOffset("bShakeDir");
	PropOffsets_PlayerPawn.bShowMenu = cls->GetPropertyDataOffset("bShowMenu");
	PropOffsets_PlayerPawn.bShowScores = cls->GetPropertyDataOffset("bShowScores");
	PropOffsets_PlayerPawn.bSinglePlayer = cls->GetPropertyDataOffset("bSinglePlayer");
	PropOffsets_PlayerPawn.bSnapToLevel = cls->GetPropertyDataOffset("bSnapToLevel");
	PropOffsets_PlayerPawn.bSpecialMenu = cls->GetPropertyDataOffset("bSpecialMenu");
	PropOffsets_PlayerPawn.bUpdatePosition = cls->GetPropertyDataOffset("bUpdatePosition");
	PropOffsets_PlayerPawn.bUpdating = cls->GetPropertyDataOffset("bUpdating");
	PropOffsets_PlayerPawn.bWasBack = cls->GetPropertyDataOffset("bWasBack");
	PropOffsets_PlayerPawn.bWasForward = cls->GetPropertyDataOffset("bWasForward");
	PropOffsets_PlayerPawn.bWasLeft = cls->GetPropertyDataOffset("bWasLeft");
	PropOffsets_PlayerPawn.bWasRight = cls->GetPropertyDataOffset("bWasRight");
	PropOffsets_PlayerPawn.bWokeUp = cls->GetPropertyDataOffset("bWokeUp");
	PropOffsets_PlayerPawn.bZooming = cls->GetPropertyDataOffset("bZooming");
	PropOffsets_PlayerPawn.bobtime = cls->GetPropertyDataOffset("bobtime");
	PropOffsets_PlayerPawn.maxshake = cls->GetPropertyDataOffset("maxshake");
	PropOffsets_PlayerPawn.myHUD = cls->GetPropertyDataOffset("myHUD");
	PropOffsets_PlayerPawn.ngSecretSet = cls->GetPropertyDataOffset("ngSecretSet");
	PropOffsets_PlayerPawn.ngWorldSecret = cls->GetPropertyDataOffset("ngWorldSecret");
	PropOffsets_PlayerPawn.shakemag = cls->GetPropertyDataOffset("shakemag");
	PropOffsets_PlayerPawn.shaketimer = cls->GetPropertyDataOffset("shaketimer");
	PropOffsets_PlayerPawn.shakevert = cls->GetPropertyDataOffset("shakevert");
	PropOffsets_PlayerPawn.verttimer = cls->GetPropertyDataOffset("verttimer");
}

PropertyOffsets_PlayerReplicationInfo PropOffsets_PlayerReplicationInfo;

static void InitPropertyOffsets_PlayerReplicationInfo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "PlayerReplicationInfo"));
	if (!cls)
	{
		memset(&PropOffsets_PlayerReplicationInfo, 0xff, sizeof(PropOffsets_PlayerReplicationInfo));
		return;
	}
	PropOffsets_PlayerReplicationInfo.Deaths = cls->GetPropertyDataOffset("Deaths");
	PropOffsets_PlayerReplicationInfo.HasFlag = cls->GetPropertyDataOffset("HasFlag");
	PropOffsets_PlayerReplicationInfo.OldName = cls->GetPropertyDataOffset("OldName");
	PropOffsets_PlayerReplicationInfo.PacketLoss = cls->GetPropertyDataOffset("PacketLoss");
	PropOffsets_PlayerReplicationInfo.Ping = cls->GetPropertyDataOffset("Ping");
	PropOffsets_PlayerReplicationInfo.PlayerID = cls->GetPropertyDataOffset("PlayerID");
	PropOffsets_PlayerReplicationInfo.PlayerLocation = cls->GetPropertyDataOffset("PlayerLocation");
	PropOffsets_PlayerReplicationInfo.PlayerName = cls->GetPropertyDataOffset("PlayerName");
	PropOffsets_PlayerReplicationInfo.PlayerZone = cls->GetPropertyDataOffset("PlayerZone");
	PropOffsets_PlayerReplicationInfo.Score = cls->GetPropertyDataOffset("Score");
	PropOffsets_PlayerReplicationInfo.StartTime = cls->GetPropertyDataOffset("StartTime");
	PropOffsets_PlayerReplicationInfo.TalkTexture = cls->GetPropertyDataOffset("TalkTexture");
	PropOffsets_PlayerReplicationInfo.Team = cls->GetPropertyDataOffset("Team");
	PropOffsets_PlayerReplicationInfo.TeamID = cls->GetPropertyDataOffset("TeamID");
	PropOffsets_PlayerReplicationInfo.TeamName = cls->GetPropertyDataOffset("TeamName");
	PropOffsets_PlayerReplicationInfo.TimeAcc = cls->GetPropertyDataOffset("TimeAcc");
	PropOffsets_PlayerReplicationInfo.VoiceType = cls->GetPropertyDataOffset("VoiceType");
	PropOffsets_PlayerReplicationInfo.bAdmin = cls->GetPropertyDataOffset("bAdmin");
	PropOffsets_PlayerReplicationInfo.bFeigningDeath = cls->GetPropertyDataOffset("bFeigningDeath");
	PropOffsets_PlayerReplicationInfo.bIsABot = cls->GetPropertyDataOffset("bIsABot");
	PropOffsets_PlayerReplicationInfo.bIsFemale = cls->GetPropertyDataOffset("bIsFemale");
	PropOffsets_PlayerReplicationInfo.bIsSpectator = cls->GetPropertyDataOffset("bIsSpectator");
	PropOffsets_PlayerReplicationInfo.bWaitingPlayer = cls->GetPropertyDataOffset("bWaitingPlayer");
}

PropertyOffsets_Weapon PropOffsets_Weapon;

static void InitPropertyOffsets_Weapon(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Weapon"));
	if (!cls)
	{
		memset(&PropOffsets_Weapon, 0xff, sizeof(PropOffsets_Weapon));
		return;
	}
	PropOffsets_Weapon.AIRating = cls->GetPropertyDataOffset("AIRating");
	PropOffsets_Weapon.AdjustedAim = cls->GetPropertyDataOffset("AdjustedAim");
	PropOffsets_Weapon.AltDamageType = cls->GetPropertyDataOffset("AltDamageType");
	PropOffsets_Weapon.AltFireSound = cls->GetPropertyDataOffset("AltFireSound");
	PropOffsets_Weapon.AltProjectileClass = cls->GetPropertyDataOffset("AltProjectileClass");
	PropOffsets_Weapon.AltProjectileSpeed = cls->GetPropertyDataOffset("AltProjectileSpeed");
	PropOffsets_Weapon.AltRefireRate = cls->GetPropertyDataOffset("AltRefireRate");
	PropOffsets_Weapon.AmmoName = cls->GetPropertyDataOffset("AmmoName");
	PropOffsets_Weapon.AmmoType = cls->GetPropertyDataOffset("AmmoType");
	PropOffsets_Weapon.CockingSound = cls->GetPropertyDataOffset("CockingSound");
	PropOffsets_Weapon.DeathMessage = cls->GetPropertyDataOffset("DeathMessage");
	PropOffsets_Weapon.FireOffset = cls->GetPropertyDataOffset("FireOffset");
	PropOffsets_Weapon.FireSound = cls->GetPropertyDataOffset("FireSound");
	PropOffsets_Weapon.FiringSpeed = cls->GetPropertyDataOffset("FiringSpeed");
	PropOffsets_Weapon.FlareOffset = cls->GetPropertyDataOffset("FlareOffset");
	PropOffsets_Weapon.FlashC = cls->GetPropertyDataOffset("FlashC");
	PropOffsets_Weapon.FlashLength = cls->GetPropertyDataOffset("FlashLength");
	PropOffsets_Weapon.FlashO = cls->GetPropertyDataOffset("FlashO");
	PropOffsets_Weapon.FlashS = cls->GetPropertyDataOffset("FlashS");
	PropOffsets_Weapon.FlashTime = cls->GetPropertyDataOffset("FlashTime");
	PropOffsets_Weapon.FlashY = cls->GetPropertyDataOffset("FlashY");
	PropOffsets_Weapon.MFTexture = cls->GetPropertyDataOffset("MFTexture");
	PropOffsets_Weapon.MaxTargetRange = cls->GetPropertyDataOffset("MaxTargetRange");
	PropOffsets_Weapon.MessageNoAmmo = cls->GetPropertyDataOffset("MessageNoAmmo");
	PropOffsets_Weapon.Misc1Sound = cls->GetPropertyDataOffset("Misc1Sound");
	PropOffsets_Weapon.Misc2Sound = cls->GetPropertyDataOffset("Misc2Sound");
	PropOffsets_Weapon.Misc3Sound = cls->GetPropertyDataOffset("Misc3Sound");
	PropOffsets_Weapon.MuzzleFlare = cls->GetPropertyDataOffset("MuzzleFlare");
	PropOffsets_Weapon.MuzzleScale = cls->GetPropertyDataOffset("MuzzleScale");
	PropOffsets_Weapon.MyDamageType = cls->GetPropertyDataOffset("MyDamageType");
	PropOffsets_Weapon.NameColor = cls->GetPropertyDataOffset("NameColor");
	PropOffsets_Weapon.PickupAmmoCount = cls->GetPropertyDataOffset("PickupAmmoCount");
	PropOffsets_Weapon.ProjectileClass = cls->GetPropertyDataOffset("ProjectileClass");
	PropOffsets_Weapon.ProjectileSpeed = cls->GetPropertyDataOffset("ProjectileSpeed");
	PropOffsets_Weapon.RefireRate = cls->GetPropertyDataOffset("RefireRate");
	PropOffsets_Weapon.ReloadCount = cls->GetPropertyDataOffset("ReloadCount");
	PropOffsets_Weapon.SelectSound = cls->GetPropertyDataOffset("SelectSound");
	PropOffsets_Weapon.aimerror = cls->GetPropertyDataOffset("aimerror");
	PropOffsets_Weapon.bAltInstantHit = cls->GetPropertyDataOffset("bAltInstantHit");
	PropOffsets_Weapon.bAltWarnTarget = cls->GetPropertyDataOffset("bAltWarnTarget");
	PropOffsets_Weapon.bCanThrow = cls->GetPropertyDataOffset("bCanThrow");
	PropOffsets_Weapon.bChangeWeapon = cls->GetPropertyDataOffset("bChangeWeapon");
	PropOffsets_Weapon.bDrawMuzzleFlash = cls->GetPropertyDataOffset("bDrawMuzzleFlash");
	PropOffsets_Weapon.bHideWeapon = cls->GetPropertyDataOffset("bHideWeapon");
	PropOffsets_Weapon.bInstantHit = cls->GetPropertyDataOffset("bInstantHit");
	PropOffsets_Weapon.bLockedOn = cls->GetPropertyDataOffset("bLockedOn");
	PropOffsets_Weapon.bMeleeWeapon = cls->GetPropertyDataOffset("bMeleeWeapon");
	PropOffsets_Weapon.bMuzzleFlash = cls->GetPropertyDataOffset("bMuzzleFlash");
	PropOffsets_Weapon.bOwnsCrosshair = cls->GetPropertyDataOffset("bOwnsCrosshair");
	PropOffsets_Weapon.bPointing = cls->GetPropertyDataOffset("bPointing");
	PropOffsets_Weapon.bRapidFire = cls->GetPropertyDataOffset("bRapidFire");
	PropOffsets_Weapon.bRecommendAltSplashDamage = cls->GetPropertyDataOffset("bRecommendAltSplashDamage");
	PropOffsets_Weapon.bRecommendSplashDamage = cls->GetPropertyDataOffset("bRecommendSplashDamage");
	PropOffsets_Weapon.bSetFlashTime = cls->GetPropertyDataOffset("bSetFlashTime");
	PropOffsets_Weapon.bSpecialIcon = cls->GetPropertyDataOffset("bSpecialIcon");
	PropOffsets_Weapon.bSplashDamage = cls->GetPropertyDataOffset("bSplashDamage");
	PropOffsets_Weapon.bWarnTarget = cls->GetPropertyDataOffset("bWarnTarget");
	PropOffsets_Weapon.bWeaponStay = cls->GetPropertyDataOffset("bWeaponStay");
	PropOffsets_Weapon.bWeaponUp = cls->GetPropertyDataOffset("bWeaponUp");
	PropOffsets_Weapon.shakemag = cls->GetPropertyDataOffset("shakemag");
	PropOffsets_Weapon.shaketime = cls->GetPropertyDataOffset("shaketime");
	PropOffsets_Weapon.shakevert = cls->GetPropertyDataOffset("shakevert");
}

PropertyOffsets_GameInfo PropOffsets_GameInfo;

static void InitPropertyOffsets_GameInfo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "GameInfo"));
	if (!cls)
	{
		memset(&PropOffsets_GameInfo, 0xff, sizeof(PropOffsets_GameInfo));
		return;
	}
	PropOffsets_GameInfo.AdminPassword = cls->GetPropertyDataOffset("AdminPassword");
	PropOffsets_GameInfo.AutoAim = cls->GetPropertyDataOffset("AutoAim");
	PropOffsets_GameInfo.BaseMutator = cls->GetPropertyDataOffset("BaseMutator");
	PropOffsets_GameInfo.BeaconName = cls->GetPropertyDataOffset("BeaconName");
	PropOffsets_GameInfo.BotMenuType = cls->GetPropertyDataOffset("BotMenuType");
	PropOffsets_GameInfo.CurrentID = cls->GetPropertyDataOffset("CurrentID");
	PropOffsets_GameInfo.DMMessageClass = cls->GetPropertyDataOffset("DMMessageClass");
	PropOffsets_GameInfo.DamageMutator = cls->GetPropertyDataOffset("DamageMutator");
	PropOffsets_GameInfo.DeathMessageClass = cls->GetPropertyDataOffset("DeathMessageClass");
	PropOffsets_GameInfo.DefaultPlayerClass = cls->GetPropertyDataOffset("DefaultPlayerClass");
	PropOffsets_GameInfo.DefaultPlayerName = cls->GetPropertyDataOffset("DefaultPlayerName");
	PropOffsets_GameInfo.DefaultPlayerState = cls->GetPropertyDataOffset("DefaultPlayerState");
	PropOffsets_GameInfo.DefaultWeapon = cls->GetPropertyDataOffset("DefaultWeapon");
	PropOffsets_GameInfo.DemoBuild = cls->GetPropertyDataOffset("DemoBuild");
	PropOffsets_GameInfo.DemoHasTuts = cls->GetPropertyDataOffset("DemoHasTuts");
	PropOffsets_GameInfo.Difficulty = cls->GetPropertyDataOffset("Difficulty");
	PropOffsets_GameInfo.EnabledMutators = cls->GetPropertyDataOffset("EnabledMutators");
	PropOffsets_GameInfo.EnteredMessage = cls->GetPropertyDataOffset("EnteredMessage");
	PropOffsets_GameInfo.FailedPlaceMessage = cls->GetPropertyDataOffset("FailedPlaceMessage");
	PropOffsets_GameInfo.FailedSpawnMessage = cls->GetPropertyDataOffset("FailedSpawnMessage");
	PropOffsets_GameInfo.FailedTeamMessage = cls->GetPropertyDataOffset("FailedTeamMessage");
	PropOffsets_GameInfo.GameMenuType = cls->GetPropertyDataOffset("GameMenuType");
	PropOffsets_GameInfo.GameName = cls->GetPropertyDataOffset("GameName");
	PropOffsets_GameInfo.GameOptionsMenuType = cls->GetPropertyDataOffset("GameOptionsMenuType");
	PropOffsets_GameInfo.GamePassword = cls->GetPropertyDataOffset("GamePassword");
	PropOffsets_GameInfo.GameReplicationInfo = cls->GetPropertyDataOffset("GameReplicationInfo");
	PropOffsets_GameInfo.GameReplicationInfoClass = cls->GetPropertyDataOffset("GameReplicationInfoClass");
	PropOffsets_GameInfo.GameSpeed = cls->GetPropertyDataOffset("GameSpeed");
	PropOffsets_GameInfo.GameUMenuType = cls->GetPropertyDataOffset("GameUMenuType");
	PropOffsets_GameInfo.HUDType = cls->GetPropertyDataOffset("HUDType");
	PropOffsets_GameInfo.IPBanned = cls->GetPropertyDataOffset("IPBanned");
	PropOffsets_GameInfo.IPPolicies = cls->GetPropertyDataOffset("IPPolicies");
	PropOffsets_GameInfo.ItemGoals = cls->GetPropertyDataOffset("ItemGoals");
	PropOffsets_GameInfo.KillGoals = cls->GetPropertyDataOffset("KillGoals");
	PropOffsets_GameInfo.LeftMessage = cls->GetPropertyDataOffset("LeftMessage");
	PropOffsets_GameInfo.LocalLog = cls->GetPropertyDataOffset("LocalLog");
	PropOffsets_GameInfo.LocalLogFileName = cls->GetPropertyDataOffset("LocalLogFileName");
	PropOffsets_GameInfo.MapListType = cls->GetPropertyDataOffset("MapListType");
	PropOffsets_GameInfo.MapPrefix = cls->GetPropertyDataOffset("MapPrefix");
	PropOffsets_GameInfo.MaxPlayers = cls->GetPropertyDataOffset("MaxPlayers");
	PropOffsets_GameInfo.MaxSpectators = cls->GetPropertyDataOffset("MaxSpectators");
	PropOffsets_GameInfo.MaxedOutMessage = cls->GetPropertyDataOffset("MaxedOutMessage");
	PropOffsets_GameInfo.MessageMutator = cls->GetPropertyDataOffset("MessageMutator");
	PropOffsets_GameInfo.MultiplayerUMenuType = cls->GetPropertyDataOffset("MultiplayerUMenuType");
	PropOffsets_GameInfo.MutatorClass = cls->GetPropertyDataOffset("MutatorClass");
	PropOffsets_GameInfo.NameChangedMessage = cls->GetPropertyDataOffset("NameChangedMessage");
	PropOffsets_GameInfo.NeedPassword = cls->GetPropertyDataOffset("NeedPassword");
	PropOffsets_GameInfo.NumPlayers = cls->GetPropertyDataOffset("NumPlayers");
	PropOffsets_GameInfo.NumSpectators = cls->GetPropertyDataOffset("NumSpectators");
	PropOffsets_GameInfo.RulesMenuType = cls->GetPropertyDataOffset("RulesMenuType");
	PropOffsets_GameInfo.ScoreBoardType = cls->GetPropertyDataOffset("ScoreBoardType");
	PropOffsets_GameInfo.SecretGoals = cls->GetPropertyDataOffset("SecretGoals");
	PropOffsets_GameInfo.SentText = cls->GetPropertyDataOffset("SentText");
	PropOffsets_GameInfo.ServerLogName = cls->GetPropertyDataOffset("ServerLogName");
	PropOffsets_GameInfo.SettingsMenuType = cls->GetPropertyDataOffset("SettingsMenuType");
	PropOffsets_GameInfo.SpecialDamageString = cls->GetPropertyDataOffset("SpecialDamageString");
	PropOffsets_GameInfo.StartTime = cls->GetPropertyDataOffset("StartTime");
	PropOffsets_GameInfo.StatLogClass = cls->GetPropertyDataOffset("StatLogClass");
	PropOffsets_GameInfo.SwitchLevelMessage = cls->GetPropertyDataOffset("SwitchLevelMessage");
	PropOffsets_GameInfo.WaterZoneType = cls->GetPropertyDataOffset("WaterZoneType");
	PropOffsets_GameInfo.WorldLog = cls->GetPropertyDataOffset("WorldLog");
	PropOffsets_GameInfo.WorldLogFileName = cls->GetPropertyDataOffset("WorldLogFileName");
	PropOffsets_GameInfo.WrongPassword = cls->GetPropertyDataOffset("WrongPassword");
	PropOffsets_GameInfo.bAllowFOV = cls->GetPropertyDataOffset("bAllowFOV");
	PropOffsets_GameInfo.bAlternateMode = cls->GetPropertyDataOffset("bAlternateMode");
	PropOffsets_GameInfo.bBatchLocal = cls->GetPropertyDataOffset("bBatchLocal");
	PropOffsets_GameInfo.bCanChangeSkin = cls->GetPropertyDataOffset("bCanChangeSkin");
	PropOffsets_GameInfo.bCanViewOthers = cls->GetPropertyDataOffset("bCanViewOthers");
	PropOffsets_GameInfo.bClassicDeathMessages = cls->GetPropertyDataOffset("bClassicDeathMessages");
	PropOffsets_GameInfo.bCoopWeaponMode = cls->GetPropertyDataOffset("bCoopWeaponMode");
	PropOffsets_GameInfo.bDeathMatch = cls->GetPropertyDataOffset("bDeathMatch");
	PropOffsets_GameInfo.bExternalBatcher = cls->GetPropertyDataOffset("bExternalBatcher");
	PropOffsets_GameInfo.bGameEnded = cls->GetPropertyDataOffset("bGameEnded");
	PropOffsets_GameInfo.bHumansOnly = cls->GetPropertyDataOffset("bHumansOnly");
	PropOffsets_GameInfo.bLocalLog = cls->GetPropertyDataOffset("bLocalLog");
	PropOffsets_GameInfo.bLoggingGame = cls->GetPropertyDataOffset("bLoggingGame");
	PropOffsets_GameInfo.bLowGore = cls->GetPropertyDataOffset("bLowGore");
	PropOffsets_GameInfo.bMuteSpectators = cls->GetPropertyDataOffset("bMuteSpectators");
	PropOffsets_GameInfo.bNoCheating = cls->GetPropertyDataOffset("bNoCheating");
	PropOffsets_GameInfo.bNoMonsters = cls->GetPropertyDataOffset("bNoMonsters");
	PropOffsets_GameInfo.bOverTime = cls->GetPropertyDataOffset("bOverTime");
	PropOffsets_GameInfo.bPauseable = cls->GetPropertyDataOffset("bPauseable");
	PropOffsets_GameInfo.bRestartLevel = cls->GetPropertyDataOffset("bRestartLevel");
	PropOffsets_GameInfo.bTeamGame = cls->GetPropertyDataOffset("bTeamGame");
	PropOffsets_GameInfo.bVeryLowGore = cls->GetPropertyDataOffset("bVeryLowGore");
	PropOffsets_GameInfo.bWorldLog = cls->GetPropertyDataOffset("bWorldLog");
}

PropertyOffsets_ZoneInfo PropOffsets_ZoneInfo;

static void InitPropertyOffsets_ZoneInfo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "ZoneInfo"));
	if (!cls)
	{
		memset(&PropOffsets_ZoneInfo, 0xff, sizeof(PropOffsets_ZoneInfo));
		return;
	}
	PropOffsets_ZoneInfo.AmbientBrightness = cls->GetPropertyDataOffset("AmbientBrightness");
	PropOffsets_ZoneInfo.AmbientHue = cls->GetPropertyDataOffset("AmbientHue");
	PropOffsets_ZoneInfo.AmbientSaturation = cls->GetPropertyDataOffset("AmbientSaturation");
	PropOffsets_ZoneInfo.CutoffHz = cls->GetPropertyDataOffset("CutoffHz");
	PropOffsets_ZoneInfo.DamagePerSec = cls->GetPropertyDataOffset("DamagePerSec");
	PropOffsets_ZoneInfo.DamageString = cls->GetPropertyDataOffset("DamageString");
	PropOffsets_ZoneInfo.DamageType = cls->GetPropertyDataOffset("DamageType");
	PropOffsets_ZoneInfo.Delay = cls->GetPropertyDataOffset("Delay");
	PropOffsets_ZoneInfo.EntryActor = cls->GetPropertyDataOffset("EntryActor");
	PropOffsets_ZoneInfo.EntrySound = cls->GetPropertyDataOffset("EntrySound");
	PropOffsets_ZoneInfo.EnvironmentMap = cls->GetPropertyDataOffset("EnvironmentMap");
	PropOffsets_ZoneInfo.ExitActor = cls->GetPropertyDataOffset("ExitActor");
	PropOffsets_ZoneInfo.ExitSound = cls->GetPropertyDataOffset("ExitSound");
	PropOffsets_ZoneInfo.FogColor = cls->GetPropertyDataOffset("FogColor");
	PropOffsets_ZoneInfo.FogDistance = cls->GetPropertyDataOffset("FogDistance");
	PropOffsets_ZoneInfo.Gain = cls->GetPropertyDataOffset("Gain");
	PropOffsets_ZoneInfo.LensFlare = cls->GetPropertyDataOffset("LensFlare");
	PropOffsets_ZoneInfo.LensFlareOffset = cls->GetPropertyDataOffset("LensFlareOffset");
	PropOffsets_ZoneInfo.LensFlareScale = cls->GetPropertyDataOffset("LensFlareScale");
	PropOffsets_ZoneInfo.MasterGain = cls->GetPropertyDataOffset("MasterGain");
	PropOffsets_ZoneInfo.MaxCarcasses = cls->GetPropertyDataOffset("MaxCarcasses");
	PropOffsets_ZoneInfo.MaxLightCount = cls->GetPropertyDataOffset("MaxLightCount");
	PropOffsets_ZoneInfo.MaxLightingPolyCount = cls->GetPropertyDataOffset("MaxLightingPolyCount");
	PropOffsets_ZoneInfo.MinLightCount = cls->GetPropertyDataOffset("MinLightCount");
	PropOffsets_ZoneInfo.MinLightingPolyCount = cls->GetPropertyDataOffset("MinLightingPolyCount");
	PropOffsets_ZoneInfo.NumCarcasses = cls->GetPropertyDataOffset("NumCarcasses");
	PropOffsets_ZoneInfo.SkyZone = cls->GetPropertyDataOffset("SkyZone");
	PropOffsets_ZoneInfo.SpeedOfSound = cls->GetPropertyDataOffset("SpeedOfSound");
	PropOffsets_ZoneInfo.TexUPanSpeed = cls->GetPropertyDataOffset("TexUPanSpeed");
	PropOffsets_ZoneInfo.TexVPanSpeed = cls->GetPropertyDataOffset("TexVPanSpeed");
	PropOffsets_ZoneInfo.ViewFlash = cls->GetPropertyDataOffset("ViewFlash");
	PropOffsets_ZoneInfo.ViewFog = cls->GetPropertyDataOffset("ViewFog");
	PropOffsets_ZoneInfo.ZoneFluidFriction = cls->GetPropertyDataOffset("ZoneFluidFriction");
	PropOffsets_ZoneInfo.ZoneGravity = cls->GetPropertyDataOffset("ZoneGravity");
	PropOffsets_ZoneInfo.ZoneGroundFriction = cls->GetPropertyDataOffset("ZoneGroundFriction");
	PropOffsets_ZoneInfo.ZoneName = cls->GetPropertyDataOffset("ZoneName");
	PropOffsets_ZoneInfo.ZonePlayerCount = cls->GetPropertyDataOffset("ZonePlayerCount");
	PropOffsets_ZoneInfo.ZonePlayerEvent = cls->GetPropertyDataOffset("ZonePlayerEvent");
	PropOffsets_ZoneInfo.ZoneTag = cls->GetPropertyDataOffset("ZoneTag");
	PropOffsets_ZoneInfo.ZoneTerminalVelocity = cls->GetPropertyDataOffset("ZoneTerminalVelocity");
	PropOffsets_ZoneInfo.ZoneVelocity = cls->GetPropertyDataOffset("ZoneVelocity");
	PropOffsets_ZoneInfo.bBounceVelocity = cls->GetPropertyDataOffset("bBounceVelocity");
	PropOffsets_ZoneInfo.bDestructive = cls->GetPropertyDataOffset("bDestructive");
	PropOffsets_ZoneInfo.bFogZone = cls->GetPropertyDataOffset("bFogZone");
	PropOffsets_ZoneInfo.bGravityZone = cls->GetPropertyDataOffset("bGravityZone");
	PropOffsets_ZoneInfo.bKillZone = cls->GetPropertyDataOffset("bKillZone");
	PropOffsets_ZoneInfo.bMoveProjectiles = cls->GetPropertyDataOffset("bMoveProjectiles");
	PropOffsets_ZoneInfo.bNeutralZone = cls->GetPropertyDataOffset("bNeutralZone");
	PropOffsets_ZoneInfo.bNoInventory = cls->GetPropertyDataOffset("bNoInventory");
	PropOffsets_ZoneInfo.bPainZone = cls->GetPropertyDataOffset("bPainZone");
	PropOffsets_ZoneInfo.bRaytraceReverb = cls->GetPropertyDataOffset("bRaytraceReverb");
	PropOffsets_ZoneInfo.bReverbZone = cls->GetPropertyDataOffset("bReverbZone");
	PropOffsets_ZoneInfo.bWaterZone = cls->GetPropertyDataOffset("bWaterZone");
	PropOffsets_ZoneInfo.locationid = cls->GetPropertyDataOffset("locationid");
}

PropertyOffsets_Canvas PropOffsets_Canvas;

static void InitPropertyOffsets_Canvas(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Canvas"));
	if (!cls)
	{
		memset(&PropOffsets_Canvas, 0xff, sizeof(PropOffsets_Canvas));
		return;
	}
	PropOffsets_Canvas.BigFont = cls->GetPropertyDataOffset("BigFont");
	PropOffsets_Canvas.ClipX = cls->GetPropertyDataOffset("ClipX");
	PropOffsets_Canvas.ClipY = cls->GetPropertyDataOffset("ClipY");
	PropOffsets_Canvas.CurX = cls->GetPropertyDataOffset("CurX");
	PropOffsets_Canvas.CurY = cls->GetPropertyDataOffset("CurY");
	PropOffsets_Canvas.CurYL = cls->GetPropertyDataOffset("CurYL");
	PropOffsets_Canvas.DrawColor = cls->GetPropertyDataOffset("DrawColor");
	PropOffsets_Canvas.Font = cls->GetPropertyDataOffset("Font");
	PropOffsets_Canvas.FramePtr = cls->GetPropertyDataOffset("FramePtr");
	PropOffsets_Canvas.LargeFont = cls->GetPropertyDataOffset("LargeFont");
	PropOffsets_Canvas.MedFont = cls->GetPropertyDataOffset("MedFont");
	PropOffsets_Canvas.OrgX = cls->GetPropertyDataOffset("OrgX");
	PropOffsets_Canvas.OrgY = cls->GetPropertyDataOffset("OrgY");
	PropOffsets_Canvas.RenderPtr = cls->GetPropertyDataOffset("RenderPtr");
	PropOffsets_Canvas.SizeX = cls->GetPropertyDataOffset("SizeX");
	PropOffsets_Canvas.SizeY = cls->GetPropertyDataOffset("SizeY");
	PropOffsets_Canvas.SmallFont = cls->GetPropertyDataOffset("SmallFont");
	PropOffsets_Canvas.SpaceX = cls->GetPropertyDataOffset("SpaceX");
	PropOffsets_Canvas.SpaceY = cls->GetPropertyDataOffset("SpaceY");
	PropOffsets_Canvas.Style = cls->GetPropertyDataOffset("Style");
	PropOffsets_Canvas.Viewport = cls->GetPropertyDataOffset("Viewport");
	PropOffsets_Canvas.Z = cls->GetPropertyDataOffset("Z");
	PropOffsets_Canvas.bCenter = cls->GetPropertyDataOffset("bCenter");
	PropOffsets_Canvas.bNoSmooth = cls->GetPropertyDataOffset("bNoSmooth");
}

PropertyOffsets_SavedMove PropOffsets_SavedMove;

static void InitPropertyOffsets_SavedMove(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "SavedMove"));
	if (!cls)
	{
		memset(&PropOffsets_SavedMove, 0xff, sizeof(PropOffsets_SavedMove));
		return;
	}
	PropOffsets_SavedMove.Delta = cls->GetPropertyDataOffset("Delta");
	PropOffsets_SavedMove.DodgeMove = cls->GetPropertyDataOffset("DodgeMove");
	PropOffsets_SavedMove.NextMove = cls->GetPropertyDataOffset("NextMove");
	PropOffsets_SavedMove.TimeStamp = cls->GetPropertyDataOffset("TimeStamp");
	PropOffsets_SavedMove.bAltFire = cls->GetPropertyDataOffset("bAltFire");
	PropOffsets_SavedMove.bDuck = cls->GetPropertyDataOffset("bDuck");
	PropOffsets_SavedMove.bFire = cls->GetPropertyDataOffset("bFire");
	PropOffsets_SavedMove.bForceAltFire = cls->GetPropertyDataOffset("bForceAltFire");
	PropOffsets_SavedMove.bForceFire = cls->GetPropertyDataOffset("bForceFire");
	PropOffsets_SavedMove.bPressedJump = cls->GetPropertyDataOffset("bPressedJump");
	PropOffsets_SavedMove.bRun = cls->GetPropertyDataOffset("bRun");
}

PropertyOffsets_StatLog PropOffsets_StatLog;

static void InitPropertyOffsets_StatLog(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "StatLog"));
	if (!cls)
	{
		memset(&PropOffsets_StatLog, 0xff, sizeof(PropOffsets_StatLog));
		return;
	}
	PropOffsets_StatLog.Context = cls->GetPropertyDataOffset("Context");
	PropOffsets_StatLog.DecoderRingURL = cls->GetPropertyDataOffset("DecoderRingURL");
	PropOffsets_StatLog.GameCreator = cls->GetPropertyDataOffset("GameCreator");
	PropOffsets_StatLog.GameCreatorURL = cls->GetPropertyDataOffset("GameCreatorURL");
	PropOffsets_StatLog.GameName = cls->GetPropertyDataOffset("GameName");
	PropOffsets_StatLog.LocalBatcherParams = cls->GetPropertyDataOffset("LocalBatcherParams");
	PropOffsets_StatLog.LocalBatcherURL = cls->GetPropertyDataOffset("LocalBatcherURL");
	PropOffsets_StatLog.LocalLogDir = cls->GetPropertyDataOffset("LocalLogDir");
	PropOffsets_StatLog.LocalStandard = cls->GetPropertyDataOffset("LocalStandard");
	PropOffsets_StatLog.LocalStatsURL = cls->GetPropertyDataOffset("LocalStatsURL");
	PropOffsets_StatLog.LogInfoURL = cls->GetPropertyDataOffset("LogInfoURL");
	PropOffsets_StatLog.LogVersion = cls->GetPropertyDataOffset("LogVersion");
	PropOffsets_StatLog.TimeStamp = cls->GetPropertyDataOffset("TimeStamp");
	PropOffsets_StatLog.WorldBatcherParams = cls->GetPropertyDataOffset("WorldBatcherParams");
	PropOffsets_StatLog.WorldBatcherURL = cls->GetPropertyDataOffset("WorldBatcherURL");
	PropOffsets_StatLog.WorldLogDir = cls->GetPropertyDataOffset("WorldLogDir");
	PropOffsets_StatLog.WorldStandard = cls->GetPropertyDataOffset("WorldStandard");
	PropOffsets_StatLog.WorldStatsURL = cls->GetPropertyDataOffset("WorldStatsURL");
	PropOffsets_StatLog.bWorld = cls->GetPropertyDataOffset("bWorld");
	PropOffsets_StatLog.bWorldBatcherError = cls->GetPropertyDataOffset("bWorldBatcherError");
}

PropertyOffsets_Texture PropOffsets_Texture;

static void InitPropertyOffsets_Texture(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Texture"));
	if (!cls)
	{
		memset(&PropOffsets_Texture, 0xff, sizeof(PropOffsets_Texture));
		return;
	}
	PropOffsets_Texture.Accumulator = cls->GetPropertyDataOffset("Accumulator");
	PropOffsets_Texture.Alpha = cls->GetPropertyDataOffset("Alpha");
	PropOffsets_Texture.AnimCurrent = cls->GetPropertyDataOffset("AnimCurrent");
	PropOffsets_Texture.AnimNext = cls->GetPropertyDataOffset("AnimNext");
	PropOffsets_Texture.BumpMap = cls->GetPropertyDataOffset("BumpMap");
	PropOffsets_Texture.CompFormat = cls->GetPropertyDataOffset("CompFormat");
	PropOffsets_Texture.CompMips = cls->GetPropertyDataOffset("CompMips");
	PropOffsets_Texture.DetailTexture = cls->GetPropertyDataOffset("DetailTexture");
	PropOffsets_Texture.Diffuse = cls->GetPropertyDataOffset("Diffuse");
	PropOffsets_Texture.DrawScale = cls->GetPropertyDataOffset("DrawScale");
	PropOffsets_Texture.FootstepSound = cls->GetPropertyDataOffset("FootstepSound");
	PropOffsets_Texture.Friction = cls->GetPropertyDataOffset("Friction");
	PropOffsets_Texture.HitSound = cls->GetPropertyDataOffset("HitSound");
	PropOffsets_Texture.LODSet = cls->GetPropertyDataOffset("LODSet");
	PropOffsets_Texture.MacroTexture = cls->GetPropertyDataOffset("MacroTexture");
	PropOffsets_Texture.MaxFrameRate = cls->GetPropertyDataOffset("MaxFrameRate");
	PropOffsets_Texture.MinFrameRate = cls->GetPropertyDataOffset("MinFrameRate");
	PropOffsets_Texture.MipMult = cls->GetPropertyDataOffset("MipMult");
	PropOffsets_Texture.Mips = cls->GetPropertyDataOffset("Mips");
	PropOffsets_Texture.PrimeCount = cls->GetPropertyDataOffset("PrimeCount");
	PropOffsets_Texture.PrimeCurrent = cls->GetPropertyDataOffset("PrimeCurrent");
	PropOffsets_Texture.Specular = cls->GetPropertyDataOffset("Specular");
	PropOffsets_Texture.bAutoUPan = cls->GetPropertyDataOffset("bAutoUPan");
	PropOffsets_Texture.bAutoVPan = cls->GetPropertyDataOffset("bAutoVPan");
	PropOffsets_Texture.bBigWavy = cls->GetPropertyDataOffset("bBigWavy");
	PropOffsets_Texture.bCloudWavy = cls->GetPropertyDataOffset("bCloudWavy");
	PropOffsets_Texture.bDirtyShadows = cls->GetPropertyDataOffset("bDirtyShadows");
	PropOffsets_Texture.bEnvironment = cls->GetPropertyDataOffset("bEnvironment");
	PropOffsets_Texture.bFakeBackdrop = cls->GetPropertyDataOffset("bFakeBackdrop");
	PropOffsets_Texture.bGouraud = cls->GetPropertyDataOffset("bGouraud");
	PropOffsets_Texture.bHasComp = cls->GetPropertyDataOffset("bHasComp");
	PropOffsets_Texture.bHighColorQuality = cls->GetPropertyDataOffset("bHighColorQuality");
	PropOffsets_Texture.bHighLedge = cls->GetPropertyDataOffset("bHighLedge");
	PropOffsets_Texture.bHighShadowDetail = cls->GetPropertyDataOffset("bHighShadowDetail");
	PropOffsets_Texture.bHighTextureQuality = cls->GetPropertyDataOffset("bHighTextureQuality");
	PropOffsets_Texture.bInvisible = cls->GetPropertyDataOffset("bInvisible");
	PropOffsets_Texture.bLowShadowDetail = cls->GetPropertyDataOffset("bLowShadowDetail");
	PropOffsets_Texture.bMasked = cls->GetPropertyDataOffset("bMasked");
	PropOffsets_Texture.bMirrored = cls->GetPropertyDataOffset("bMirrored");
	PropOffsets_Texture.bModulate = cls->GetPropertyDataOffset("bModulate");
	PropOffsets_Texture.bNoMerge = cls->GetPropertyDataOffset("bNoMerge");
	PropOffsets_Texture.bNoSmooth = cls->GetPropertyDataOffset("bNoSmooth");
	PropOffsets_Texture.bNotSolid = cls->GetPropertyDataOffset("bNotSolid");
	PropOffsets_Texture.bParametric = cls->GetPropertyDataOffset("bParametric");
	PropOffsets_Texture.bPortal = cls->GetPropertyDataOffset("bPortal");
	PropOffsets_Texture.bRealtime = cls->GetPropertyDataOffset("bRealtime");
	PropOffsets_Texture.bRealtimeChanged = cls->GetPropertyDataOffset("bRealtimeChanged");
	PropOffsets_Texture.bSemisolid = cls->GetPropertyDataOffset("bSemisolid");
	PropOffsets_Texture.bSmallWavy = cls->GetPropertyDataOffset("bSmallWavy");
	PropOffsets_Texture.bSpecialLit = cls->GetPropertyDataOffset("bSpecialLit");
	PropOffsets_Texture.bTransparent = cls->GetPropertyDataOffset("bTransparent");
	PropOffsets_Texture.bTwoSided = cls->GetPropertyDataOffset("bTwoSided");
	PropOffsets_Texture.bUnlit = cls->GetPropertyDataOffset("bUnlit");
	PropOffsets_Texture.bWaterWavy = cls->GetPropertyDataOffset("bWaterWavy");
	PropOffsets_Texture.bX2 = cls->GetPropertyDataOffset("bX2");
	PropOffsets_Texture.bX3 = cls->GetPropertyDataOffset("bX3");
	PropOffsets_Texture.bX4 = cls->GetPropertyDataOffset("bX4");
	PropOffsets_Texture.bX5 = cls->GetPropertyDataOffset("bX5");
	PropOffsets_Texture.bX6 = cls->GetPropertyDataOffset("bX6");
	PropOffsets_Texture.bX7 = cls->GetPropertyDataOffset("bX7");
}

PropertyOffsets_Ammo PropOffsets_Ammo;

static void InitPropertyOffsets_Ammo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Ammo"));
	if (!cls)
	{
		memset(&PropOffsets_Ammo, 0xff, sizeof(PropOffsets_Ammo));
		return;
	}
	PropOffsets_Ammo.AmmoAmount = cls->GetPropertyDataOffset("AmmoAmount");
	PropOffsets_Ammo.MaxAmmo = cls->GetPropertyDataOffset("MaxAmmo");
	PropOffsets_Ammo.PAmmo = cls->GetPropertyDataOffset("PAmmo");
	PropOffsets_Ammo.ParentAmmo = cls->GetPropertyDataOffset("ParentAmmo");
	PropOffsets_Ammo.UsedInWeaponSlot = cls->GetPropertyDataOffset("UsedInWeaponSlot");
}

PropertyOffsets_NavigationPoint PropOffsets_NavigationPoint;

static void InitPropertyOffsets_NavigationPoint(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "NavigationPoint"));
	if (!cls)
	{
		memset(&PropOffsets_NavigationPoint, 0xff, sizeof(PropOffsets_NavigationPoint));
		return;
	}
	PropOffsets_NavigationPoint.ExtraCost = cls->GetPropertyDataOffset("ExtraCost");
	PropOffsets_NavigationPoint.Paths = cls->GetPropertyDataOffset("Paths");
	PropOffsets_NavigationPoint.PrunedPaths = cls->GetPropertyDataOffset("PrunedPaths");
	PropOffsets_NavigationPoint.RouteCache = cls->GetPropertyDataOffset("RouteCache");
	PropOffsets_NavigationPoint.VisNoReachPaths = cls->GetPropertyDataOffset("VisNoReachPaths");
	PropOffsets_NavigationPoint.bAutoBuilt = cls->GetPropertyDataOffset("bAutoBuilt");
	PropOffsets_NavigationPoint.bEndPoint = cls->GetPropertyDataOffset("bEndPoint");
	PropOffsets_NavigationPoint.bEndPointOnly = cls->GetPropertyDataOffset("bEndPointOnly");
	PropOffsets_NavigationPoint.bNeverUseStrafing = cls->GetPropertyDataOffset("bNeverUseStrafing");
	PropOffsets_NavigationPoint.bOneWayPath = cls->GetPropertyDataOffset("bOneWayPath");
	PropOffsets_NavigationPoint.bPlayerOnly = cls->GetPropertyDataOffset("bPlayerOnly");
	PropOffsets_NavigationPoint.bSpecialCost = cls->GetPropertyDataOffset("bSpecialCost");
	PropOffsets_NavigationPoint.bTwoWay = cls->GetPropertyDataOffset("bTwoWay");
	PropOffsets_NavigationPoint.bestPathWeight = cls->GetPropertyDataOffset("bestPathWeight");
	PropOffsets_NavigationPoint.cost = cls->GetPropertyDataOffset("cost");
	PropOffsets_NavigationPoint.nextNavigationPoint = cls->GetPropertyDataOffset("nextNavigationPoint");
	PropOffsets_NavigationPoint.nextOrdered = cls->GetPropertyDataOffset("nextOrdered");
	PropOffsets_NavigationPoint.ownerTeam = cls->GetPropertyDataOffset("ownerTeam");
	PropOffsets_NavigationPoint.prevOrdered = cls->GetPropertyDataOffset("prevOrdered");
	PropOffsets_NavigationPoint.previousPath = cls->GetPropertyDataOffset("previousPath");
	PropOffsets_NavigationPoint.startPath = cls->GetPropertyDataOffset("startPath");
	PropOffsets_NavigationPoint.taken = cls->GetPropertyDataOffset("taken");
	PropOffsets_NavigationPoint.upstreamPaths = cls->GetPropertyDataOffset("upstreamPaths");
	PropOffsets_NavigationPoint.visitedWeight = cls->GetPropertyDataOffset("visitedWeight");
}

PropertyOffsets_Mutator PropOffsets_Mutator;

static void InitPropertyOffsets_Mutator(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Mutator"));
	if (!cls)
	{
		memset(&PropOffsets_Mutator, 0xff, sizeof(PropOffsets_Mutator));
		return;
	}
	PropOffsets_Mutator.DefaultWeapon = cls->GetPropertyDataOffset("DefaultWeapon");
	PropOffsets_Mutator.NextDamageMutator = cls->GetPropertyDataOffset("NextDamageMutator");
	PropOffsets_Mutator.NextHUDMutator = cls->GetPropertyDataOffset("NextHUDMutator");
	PropOffsets_Mutator.NextMessageMutator = cls->GetPropertyDataOffset("NextMessageMutator");
	PropOffsets_Mutator.NextMutator = cls->GetPropertyDataOffset("NextMutator");
	PropOffsets_Mutator.bHUDMutator = cls->GetPropertyDataOffset("bHUDMutator");
}

PropertyOffsets_Mover PropOffsets_Mover;

static void InitPropertyOffsets_Mover(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Mover"));
	if (!cls)
	{
		memset(&PropOffsets_Mover, 0xff, sizeof(PropOffsets_Mover));
		return;
	}
	PropOffsets_Mover.BasePos = cls->GetPropertyDataOffset("BasePos");
	PropOffsets_Mover.BaseRot = cls->GetPropertyDataOffset("BaseRot");
	PropOffsets_Mover.BrushRaytraceKey = cls->GetPropertyDataOffset("BrushRaytraceKey");
	PropOffsets_Mover.BumpEvent = cls->GetPropertyDataOffset("BumpEvent");
	PropOffsets_Mover.BumpType = cls->GetPropertyDataOffset("BumpType");
	PropOffsets_Mover.ClientUpdate = cls->GetPropertyDataOffset("ClientUpdate");
	PropOffsets_Mover.ClosedSound = cls->GetPropertyDataOffset("ClosedSound");
	PropOffsets_Mover.ClosingSound = cls->GetPropertyDataOffset("ClosingSound");
	PropOffsets_Mover.DamageThreshold = cls->GetPropertyDataOffset("DamageThreshold");
	PropOffsets_Mover.DelayTime = cls->GetPropertyDataOffset("DelayTime");
	PropOffsets_Mover.EncroachDamage = cls->GetPropertyDataOffset("EncroachDamage");
	PropOffsets_Mover.Follower = cls->GetPropertyDataOffset("Follower");
	PropOffsets_Mover.KeyNum = cls->GetPropertyDataOffset("KeyNum");
	PropOffsets_Mover.KeyPos = cls->GetPropertyDataOffset("KeyPos");
	PropOffsets_Mover.KeyRot = cls->GetPropertyDataOffset("KeyRot");
	PropOffsets_Mover.Leader = cls->GetPropertyDataOffset("Leader");
	PropOffsets_Mover.MoveAmbientSound = cls->GetPropertyDataOffset("MoveAmbientSound");
	PropOffsets_Mover.MoveTime = cls->GetPropertyDataOffset("MoveTime");
	PropOffsets_Mover.MoverEncroachType = cls->GetPropertyDataOffset("MoverEncroachType");
	PropOffsets_Mover.MoverGlideType = cls->GetPropertyDataOffset("MoverGlideType");
	PropOffsets_Mover.NumKeys = cls->GetPropertyDataOffset("NumKeys");
	PropOffsets_Mover.OldPos = cls->GetPropertyDataOffset("OldPos");
	PropOffsets_Mover.OldPrePivot = cls->GetPropertyDataOffset("OldPrePivot");
	PropOffsets_Mover.OldRot = cls->GetPropertyDataOffset("OldRot");
	PropOffsets_Mover.OpenedSound = cls->GetPropertyDataOffset("OpenedSound");
	PropOffsets_Mover.OpeningSound = cls->GetPropertyDataOffset("OpeningSound");
	PropOffsets_Mover.OtherTime = cls->GetPropertyDataOffset("OtherTime");
	PropOffsets_Mover.PlayerBumpEvent = cls->GetPropertyDataOffset("PlayerBumpEvent");
	PropOffsets_Mover.PrevKeyNum = cls->GetPropertyDataOffset("PrevKeyNum");
	PropOffsets_Mover.RealPosition = cls->GetPropertyDataOffset("RealPosition");
	PropOffsets_Mover.RealRotation = cls->GetPropertyDataOffset("RealRotation");
	PropOffsets_Mover.RecommendedTrigger = cls->GetPropertyDataOffset("RecommendedTrigger");
	PropOffsets_Mover.ReturnGroup = cls->GetPropertyDataOffset("ReturnGroup");
	PropOffsets_Mover.SavedPos = cls->GetPropertyDataOffset("SavedPos");
	PropOffsets_Mover.SavedRot = cls->GetPropertyDataOffset("SavedRot");
	PropOffsets_Mover.SavedTrigger = cls->GetPropertyDataOffset("SavedTrigger");
	PropOffsets_Mover.SimInterpolate = cls->GetPropertyDataOffset("SimInterpolate");
	PropOffsets_Mover.SimOldPos = cls->GetPropertyDataOffset("SimOldPos");
	PropOffsets_Mover.SimOldRotPitch = cls->GetPropertyDataOffset("SimOldRotPitch");
	PropOffsets_Mover.SimOldRotRoll = cls->GetPropertyDataOffset("SimOldRotRoll");
	PropOffsets_Mover.SimOldRotYaw = cls->GetPropertyDataOffset("SimOldRotYaw");
	PropOffsets_Mover.StayOpenTime = cls->GetPropertyDataOffset("StayOpenTime");
	PropOffsets_Mover.TriggerActor = cls->GetPropertyDataOffset("TriggerActor");
	PropOffsets_Mover.TriggerActor2 = cls->GetPropertyDataOffset("TriggerActor2");
	PropOffsets_Mover.WaitingPawn = cls->GetPropertyDataOffset("WaitingPawn");
	PropOffsets_Mover.WorldRaytraceKey = cls->GetPropertyDataOffset("WorldRaytraceKey");
	PropOffsets_Mover.bClientPause = cls->GetPropertyDataOffset("bClientPause");
	PropOffsets_Mover.bDamageTriggered = cls->GetPropertyDataOffset("bDamageTriggered");
	PropOffsets_Mover.bDelaying = cls->GetPropertyDataOffset("bDelaying");
	PropOffsets_Mover.bDynamicLightMover = cls->GetPropertyDataOffset("bDynamicLightMover");
	PropOffsets_Mover.bOpening = cls->GetPropertyDataOffset("bOpening");
	PropOffsets_Mover.bPlayerOnly = cls->GetPropertyDataOffset("bPlayerOnly");
	PropOffsets_Mover.bSlave = cls->GetPropertyDataOffset("bSlave");
	PropOffsets_Mover.bTriggerOnceOnly = cls->GetPropertyDataOffset("bTriggerOnceOnly");
	PropOffsets_Mover.bUseTriggered = cls->GetPropertyDataOffset("bUseTriggered");
	PropOffsets_Mover.myMarker = cls->GetPropertyDataOffset("myMarker");
	PropOffsets_Mover.numTriggerEvents = cls->GetPropertyDataOffset("numTriggerEvents");
}

PropertyOffsets_HUD PropOffsets_HUD;

static void InitPropertyOffsets_HUD(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "HUD"));
	if (!cls)
	{
		memset(&PropOffsets_HUD, 0xff, sizeof(PropOffsets_HUD));
		return;
	}
	PropOffsets_HUD.Crosshair = cls->GetPropertyDataOffset("Crosshair");
	PropOffsets_HUD.HUDConfigWindowType = cls->GetPropertyDataOffset("HUDConfigWindowType");
	PropOffsets_HUD.HUDMutator = cls->GetPropertyDataOffset("HUDMutator");
	PropOffsets_HUD.HudMode = cls->GetPropertyDataOffset("HudMode");
	PropOffsets_HUD.MainMenu = cls->GetPropertyDataOffset("MainMenu");
	PropOffsets_HUD.MainMenuType = cls->GetPropertyDataOffset("MainMenuType");
	PropOffsets_HUD.PlayerOwner = cls->GetPropertyDataOffset("PlayerOwner");
	PropOffsets_HUD.WhiteColor = cls->GetPropertyDataOffset("WhiteColor");
}

PropertyOffsets_Decoration PropOffsets_Decoration;

static void InitPropertyOffsets_Decoration(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Decoration"));
	if (!cls)
	{
		memset(&PropOffsets_Decoration, 0xff, sizeof(PropOffsets_Decoration));
		return;
	}
	PropOffsets_Decoration.EffectWhenDestroyed = cls->GetPropertyDataOffset("EffectWhenDestroyed");
	PropOffsets_Decoration.EndPushSound = cls->GetPropertyDataOffset("EndPushSound");
	PropOffsets_Decoration.PushSound = cls->GetPropertyDataOffset("PushSound");
	PropOffsets_Decoration.bBobbing = cls->GetPropertyDataOffset("bBobbing");
	PropOffsets_Decoration.bOnlyTriggerable = cls->GetPropertyDataOffset("bOnlyTriggerable");
	PropOffsets_Decoration.bPushSoundPlaying = cls->GetPropertyDataOffset("bPushSoundPlaying");
	PropOffsets_Decoration.bPushable = cls->GetPropertyDataOffset("bPushable");
	PropOffsets_Decoration.bSplash = cls->GetPropertyDataOffset("bSplash");
	PropOffsets_Decoration.bWasCarried = cls->GetPropertyDataOffset("bWasCarried");
	PropOffsets_Decoration.content2 = cls->GetPropertyDataOffset("content2");
	PropOffsets_Decoration.content3 = cls->GetPropertyDataOffset("content3");
	PropOffsets_Decoration.contents = cls->GetPropertyDataOffset("contents");
	PropOffsets_Decoration.numLandings = cls->GetPropertyDataOffset("numLandings");
}

PropertyOffsets_TestInfo PropOffsets_TestInfo;

static void InitPropertyOffsets_TestInfo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "TestInfo"));
	if (!cls)
	{
		memset(&PropOffsets_TestInfo, 0xff, sizeof(PropOffsets_TestInfo));
		return;
	}
	PropOffsets_TestInfo.MyArray = cls->GetPropertyDataOffset("MyArray");
	PropOffsets_TestInfo.ST = cls->GetPropertyDataOffset("ST");
	PropOffsets_TestInfo.TestRepStr = cls->GetPropertyDataOffset("TestRepStr");
	PropOffsets_TestInfo.bBool1 = cls->GetPropertyDataOffset("bBool1");
	PropOffsets_TestInfo.bBool2 = cls->GetPropertyDataOffset("bBool2");
	PropOffsets_TestInfo.bFalse1 = cls->GetPropertyDataOffset("bFalse1");
	PropOffsets_TestInfo.bFalse2 = cls->GetPropertyDataOffset("bFalse2");
	PropOffsets_TestInfo.bTrue1 = cls->GetPropertyDataOffset("bTrue1");
	PropOffsets_TestInfo.bTrue2 = cls->GetPropertyDataOffset("bTrue2");
	PropOffsets_TestInfo.ppp = cls->GetPropertyDataOffset("ppp");
	PropOffsets_TestInfo.sxx = cls->GetPropertyDataOffset("sxx");
	PropOffsets_TestInfo.v1 = cls->GetPropertyDataOffset("v1");
	PropOffsets_TestInfo.v2 = cls->GetPropertyDataOffset("v2");
	PropOffsets_TestInfo.xnum = cls->GetPropertyDataOffset("xnum");
}

PropertyOffsets_GameReplicationInfo PropOffsets_GameReplicationInfo;

static void InitPropertyOffsets_GameReplicationInfo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "GameReplicationInfo"));
	if (!cls)
	{
		memset(&PropOffsets_GameReplicationInfo, 0xff, sizeof(PropOffsets_GameReplicationInfo));
		return;
	}
	PropOffsets_GameReplicationInfo.AdminEmail = cls->GetPropertyDataOffset("AdminEmail");
	PropOffsets_GameReplicationInfo.AdminName = cls->GetPropertyDataOffset("AdminName");
	PropOffsets_GameReplicationInfo.ElapsedTime = cls->GetPropertyDataOffset("ElapsedTime");
	PropOffsets_GameReplicationInfo.GameClass = cls->GetPropertyDataOffset("GameClass");
	PropOffsets_GameReplicationInfo.GameEndedComments = cls->GetPropertyDataOffset("GameEndedComments");
	PropOffsets_GameReplicationInfo.GameName = cls->GetPropertyDataOffset("GameName");
	PropOffsets_GameReplicationInfo.MOTDLine1 = cls->GetPropertyDataOffset("MOTDLine1");
	PropOffsets_GameReplicationInfo.MOTDLine2 = cls->GetPropertyDataOffset("MOTDLine2");
	PropOffsets_GameReplicationInfo.MOTDLine3 = cls->GetPropertyDataOffset("MOTDLine3");
	PropOffsets_GameReplicationInfo.MOTDLine4 = cls->GetPropertyDataOffset("MOTDLine4");
	PropOffsets_GameReplicationInfo.NumPlayers = cls->GetPropertyDataOffset("NumPlayers");
	PropOffsets_GameReplicationInfo.PRIArray = cls->GetPropertyDataOffset("PRIArray");
	PropOffsets_GameReplicationInfo.Region = cls->GetPropertyDataOffset("Region");
	PropOffsets_GameReplicationInfo.RemainingMinute = cls->GetPropertyDataOffset("RemainingMinute");
	PropOffsets_GameReplicationInfo.RemainingTime = cls->GetPropertyDataOffset("RemainingTime");
	PropOffsets_GameReplicationInfo.SecondCount = cls->GetPropertyDataOffset("SecondCount");
	PropOffsets_GameReplicationInfo.ServerName = cls->GetPropertyDataOffset("ServerName");
	PropOffsets_GameReplicationInfo.ShortName = cls->GetPropertyDataOffset("ShortName");
	PropOffsets_GameReplicationInfo.SumFrags = cls->GetPropertyDataOffset("SumFrags");
	PropOffsets_GameReplicationInfo.UpdateTimer = cls->GetPropertyDataOffset("UpdateTimer");
	PropOffsets_GameReplicationInfo.bClassicDeathMessages = cls->GetPropertyDataOffset("bClassicDeathMessages");
	PropOffsets_GameReplicationInfo.bStopCountDown = cls->GetPropertyDataOffset("bStopCountDown");
	PropOffsets_GameReplicationInfo.bTeamGame = cls->GetPropertyDataOffset("bTeamGame");
}

PropertyOffsets_Menu PropOffsets_Menu;

static void InitPropertyOffsets_Menu(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Menu"));
	if (!cls)
	{
		memset(&PropOffsets_Menu, 0xff, sizeof(PropOffsets_Menu));
		return;
	}
	PropOffsets_Menu.CenterString = cls->GetPropertyDataOffset("CenterString");
	PropOffsets_Menu.DisabledString = cls->GetPropertyDataOffset("DisabledString");
	PropOffsets_Menu.EnabledString = cls->GetPropertyDataOffset("EnabledString");
	PropOffsets_Menu.HelpMessage = cls->GetPropertyDataOffset("HelpMessage");
	PropOffsets_Menu.LeftString = cls->GetPropertyDataOffset("LeftString");
	PropOffsets_Menu.MenuLength = cls->GetPropertyDataOffset("MenuLength");
	PropOffsets_Menu.MenuList = cls->GetPropertyDataOffset("MenuList");
	PropOffsets_Menu.MenuTitle = cls->GetPropertyDataOffset("MenuTitle");
	PropOffsets_Menu.NoString = cls->GetPropertyDataOffset("NoString");
	PropOffsets_Menu.ParentMenu = cls->GetPropertyDataOffset("ParentMenu");
	PropOffsets_Menu.PlayerOwner = cls->GetPropertyDataOffset("PlayerOwner");
	PropOffsets_Menu.RightString = cls->GetPropertyDataOffset("RightString");
	PropOffsets_Menu.Selection = cls->GetPropertyDataOffset("Selection");
	PropOffsets_Menu.YesString = cls->GetPropertyDataOffset("YesString");
	PropOffsets_Menu.bConfigChanged = cls->GetPropertyDataOffset("bConfigChanged");
	PropOffsets_Menu.bExitAllMenus = cls->GetPropertyDataOffset("bExitAllMenus");
}

PropertyOffsets_LiftExit PropOffsets_LiftExit;

static void InitPropertyOffsets_LiftExit(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "LiftExit"));
	if (!cls)
	{
		memset(&PropOffsets_LiftExit, 0xff, sizeof(PropOffsets_LiftExit));
		return;
	}
	PropOffsets_LiftExit.LastTriggerTime = cls->GetPropertyDataOffset("LastTriggerTime");
	PropOffsets_LiftExit.LiftTag = cls->GetPropertyDataOffset("LiftTag");
	PropOffsets_LiftExit.LiftTrigger = cls->GetPropertyDataOffset("LiftTrigger");
	PropOffsets_LiftExit.MyLift = cls->GetPropertyDataOffset("MyLift");
	PropOffsets_LiftExit.RecommendedTrigger = cls->GetPropertyDataOffset("RecommendedTrigger");
}

PropertyOffsets_Trigger PropOffsets_Trigger;

static void InitPropertyOffsets_Trigger(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Trigger"));
	if (!cls)
	{
		memset(&PropOffsets_Trigger, 0xff, sizeof(PropOffsets_Trigger));
		return;
	}
	PropOffsets_Trigger.ClassProximityType = cls->GetPropertyDataOffset("ClassProximityType");
	PropOffsets_Trigger.DamageThreshold = cls->GetPropertyDataOffset("DamageThreshold");
	PropOffsets_Trigger.Message = cls->GetPropertyDataOffset("Message");
	PropOffsets_Trigger.ReTriggerDelay = cls->GetPropertyDataOffset("ReTriggerDelay");
	PropOffsets_Trigger.RepeatTriggerTime = cls->GetPropertyDataOffset("RepeatTriggerTime");
	PropOffsets_Trigger.TriggerActor = cls->GetPropertyDataOffset("TriggerActor");
	PropOffsets_Trigger.TriggerActor2 = cls->GetPropertyDataOffset("TriggerActor2");
	PropOffsets_Trigger.TriggerTime = cls->GetPropertyDataOffset("TriggerTime");
	PropOffsets_Trigger.TriggerType = cls->GetPropertyDataOffset("TriggerType");
	PropOffsets_Trigger.bInitiallyActive = cls->GetPropertyDataOffset("bInitiallyActive");
	PropOffsets_Trigger.bTriggerOnceOnly = cls->GetPropertyDataOffset("bTriggerOnceOnly");
}

PropertyOffsets_Player PropOffsets_Player;

static void InitPropertyOffsets_Player(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Player"));
	if (!cls)
	{
		memset(&PropOffsets_Player, 0xff, sizeof(PropOffsets_Player));
		return;
	}
	PropOffsets_Player.Actor = cls->GetPropertyDataOffset("Actor");
	PropOffsets_Player.ConfiguredInternetSpeed = cls->GetPropertyDataOffset("ConfiguredInternetSpeed");
	PropOffsets_Player.ConfiguredLanSpeed = cls->GetPropertyDataOffset("ConfiguredLanSpeed");
	PropOffsets_Player.Console = cls->GetPropertyDataOffset("Console");
	PropOffsets_Player.CurrentNetSpeed = cls->GetPropertyDataOffset("CurrentNetSpeed");
	PropOffsets_Player.SelectedCursor = cls->GetPropertyDataOffset("SelectedCursor");
	PropOffsets_Player.WindowsMouseX = cls->GetPropertyDataOffset("WindowsMouseX");
	PropOffsets_Player.WindowsMouseY = cls->GetPropertyDataOffset("WindowsMouseY");
	PropOffsets_Player.bShowWindowsMouse = cls->GetPropertyDataOffset("bShowWindowsMouse");
	PropOffsets_Player.bSuspendPrecaching = cls->GetPropertyDataOffset("bSuspendPrecaching");
	PropOffsets_Player.bWindowsMouseAvailable = cls->GetPropertyDataOffset("bWindowsMouseAvailable");
	PropOffsets_Player.vfExec = cls->GetPropertyDataOffset("vfExec");
	PropOffsets_Player.vfOut = cls->GetPropertyDataOffset("vfOut");
}

PropertyOffsets_LocalMessage PropOffsets_LocalMessage;

static void InitPropertyOffsets_LocalMessage(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "LocalMessage"));
	if (!cls)
	{
		memset(&PropOffsets_LocalMessage, 0xff, sizeof(PropOffsets_LocalMessage));
		return;
	}
	PropOffsets_LocalMessage.ChildMessage = cls->GetPropertyDataOffset("ChildMessage");
	PropOffsets_LocalMessage.DrawColor = cls->GetPropertyDataOffset("DrawColor");
	PropOffsets_LocalMessage.Lifetime = cls->GetPropertyDataOffset("Lifetime");
	PropOffsets_LocalMessage.XPos = cls->GetPropertyDataOffset("XPos");
	PropOffsets_LocalMessage.YPos = cls->GetPropertyDataOffset("YPos");
	PropOffsets_LocalMessage.bBeep = cls->GetPropertyDataOffset("bBeep");
	PropOffsets_LocalMessage.bCenter = cls->GetPropertyDataOffset("bCenter");
	PropOffsets_LocalMessage.bComplexString = cls->GetPropertyDataOffset("bComplexString");
	PropOffsets_LocalMessage.bFadeMessage = cls->GetPropertyDataOffset("bFadeMessage");
	PropOffsets_LocalMessage.bFromBottom = cls->GetPropertyDataOffset("bFromBottom");
	PropOffsets_LocalMessage.bIsConsoleMessage = cls->GetPropertyDataOffset("bIsConsoleMessage");
	PropOffsets_LocalMessage.bIsSpecial = cls->GetPropertyDataOffset("bIsSpecial");
	PropOffsets_LocalMessage.bIsUnique = cls->GetPropertyDataOffset("bIsUnique");
	PropOffsets_LocalMessage.bOffsetYPos = cls->GetPropertyDataOffset("bOffsetYPos");
}

PropertyOffsets_locationid PropOffsets_locationid;

static void InitPropertyOffsets_locationid(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "locationid"));
	if (!cls)
	{
		memset(&PropOffsets_locationid, 0xff, sizeof(PropOffsets_locationid));
		return;
	}
	PropOffsets_locationid.LocationName = cls->GetPropertyDataOffset("LocationName");
	PropOffsets_locationid.NextLocation = cls->GetPropertyDataOffset("NextLocation");
	PropOffsets_locationid.Radius = cls->GetPropertyDataOffset("Radius");
}

PropertyOffsets_Carcass PropOffsets_Carcass;

static void InitPropertyOffsets_Carcass(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Carcass"));
	if (!cls)
	{
		memset(&PropOffsets_Carcass, 0xff, sizeof(PropOffsets_Carcass));
		return;
	}
	PropOffsets_Carcass.Bugs = cls->GetPropertyDataOffset("Bugs");
	PropOffsets_Carcass.CumulativeDamage = cls->GetPropertyDataOffset("CumulativeDamage");
	PropOffsets_Carcass.PlayerOwner = cls->GetPropertyDataOffset("PlayerOwner");
	PropOffsets_Carcass.bDecorative = cls->GetPropertyDataOffset("bDecorative");
	PropOffsets_Carcass.bPlayerCarcass = cls->GetPropertyDataOffset("bPlayerCarcass");
	PropOffsets_Carcass.bReducedHeight = cls->GetPropertyDataOffset("bReducedHeight");
	PropOffsets_Carcass.bSlidingCarcass = cls->GetPropertyDataOffset("bSlidingCarcass");
	PropOffsets_Carcass.flies = cls->GetPropertyDataOffset("flies");
	PropOffsets_Carcass.rats = cls->GetPropertyDataOffset("rats");
}

PropertyOffsets_InterpolationPoint PropOffsets_InterpolationPoint;

static void InitPropertyOffsets_InterpolationPoint(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "InterpolationPoint"));
	if (!cls)
	{
		memset(&PropOffsets_InterpolationPoint, 0xff, sizeof(PropOffsets_InterpolationPoint));
		return;
	}
	PropOffsets_InterpolationPoint.FovModifier = cls->GetPropertyDataOffset("FovModifier");
	PropOffsets_InterpolationPoint.GameSpeedModifier = cls->GetPropertyDataOffset("GameSpeedModifier");
	PropOffsets_InterpolationPoint.Next = cls->GetPropertyDataOffset("Next");
	PropOffsets_InterpolationPoint.Position = cls->GetPropertyDataOffset("Position");
	PropOffsets_InterpolationPoint.Prev = cls->GetPropertyDataOffset("Prev");
	PropOffsets_InterpolationPoint.RateModifier = cls->GetPropertyDataOffset("RateModifier");
	PropOffsets_InterpolationPoint.ScreenFlashFog = cls->GetPropertyDataOffset("ScreenFlashFog");
	PropOffsets_InterpolationPoint.ScreenFlashScale = cls->GetPropertyDataOffset("ScreenFlashScale");
	PropOffsets_InterpolationPoint.bEndOfPath = cls->GetPropertyDataOffset("bEndOfPath");
	PropOffsets_InterpolationPoint.bSkipNextPath = cls->GetPropertyDataOffset("bSkipNextPath");
}

PropertyOffsets_Projectile PropOffsets_Projectile;

static void InitPropertyOffsets_Projectile(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Projectile"));
	if (!cls)
	{
		memset(&PropOffsets_Projectile, 0xff, sizeof(PropOffsets_Projectile));
		return;
	}
	PropOffsets_Projectile.Damage = cls->GetPropertyDataOffset("Damage");
	PropOffsets_Projectile.ExploWallOut = cls->GetPropertyDataOffset("ExploWallOut");
	PropOffsets_Projectile.ExplosionDecal = cls->GetPropertyDataOffset("ExplosionDecal");
	PropOffsets_Projectile.ImpactSound = cls->GetPropertyDataOffset("ImpactSound");
	PropOffsets_Projectile.MaxSpeed = cls->GetPropertyDataOffset("MaxSpeed");
	PropOffsets_Projectile.MiscSound = cls->GetPropertyDataOffset("MiscSound");
	PropOffsets_Projectile.MomentumTransfer = cls->GetPropertyDataOffset("MomentumTransfer");
	PropOffsets_Projectile.MyDamageType = cls->GetPropertyDataOffset("MyDamageType");
	PropOffsets_Projectile.SpawnSound = cls->GetPropertyDataOffset("SpawnSound");
	PropOffsets_Projectile.speed = cls->GetPropertyDataOffset("speed");
}

PropertyOffsets_Teleporter PropOffsets_Teleporter;

static void InitPropertyOffsets_Teleporter(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Teleporter"));
	if (!cls)
	{
		memset(&PropOffsets_Teleporter, 0xff, sizeof(PropOffsets_Teleporter));
		return;
	}
	PropOffsets_Teleporter.LastFired = cls->GetPropertyDataOffset("LastFired");
	PropOffsets_Teleporter.ProductRequired = cls->GetPropertyDataOffset("ProductRequired");
	PropOffsets_Teleporter.TargetVelocity = cls->GetPropertyDataOffset("TargetVelocity");
	PropOffsets_Teleporter.TriggerActor = cls->GetPropertyDataOffset("TriggerActor");
	PropOffsets_Teleporter.TriggerActor2 = cls->GetPropertyDataOffset("TriggerActor2");
	PropOffsets_Teleporter.URL = cls->GetPropertyDataOffset("URL");
	PropOffsets_Teleporter.bChangesVelocity = cls->GetPropertyDataOffset("bChangesVelocity");
	PropOffsets_Teleporter.bChangesYaw = cls->GetPropertyDataOffset("bChangesYaw");
	PropOffsets_Teleporter.bEnabled = cls->GetPropertyDataOffset("bEnabled");
	PropOffsets_Teleporter.bReversesX = cls->GetPropertyDataOffset("bReversesX");
	PropOffsets_Teleporter.bReversesY = cls->GetPropertyDataOffset("bReversesY");
	PropOffsets_Teleporter.bReversesZ = cls->GetPropertyDataOffset("bReversesZ");
}

PropertyOffsets_Palette PropOffsets_Palette;

static void InitPropertyOffsets_Palette(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Palette"));
	if (!cls)
	{
		memset(&PropOffsets_Palette, 0xff, sizeof(PropOffsets_Palette));
		return;
	}
	PropOffsets_Palette.Colors = cls->GetPropertyDataOffset("Colors");
}

PropertyOffsets_SpawnNotify PropOffsets_SpawnNotify;

static void InitPropertyOffsets_SpawnNotify(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "SpawnNotify"));
	if (!cls)
	{
		memset(&PropOffsets_SpawnNotify, 0xff, sizeof(PropOffsets_SpawnNotify));
		return;
	}
	PropOffsets_SpawnNotify.ActorClass = cls->GetPropertyDataOffset("ActorClass");
	PropOffsets_SpawnNotify.Next = cls->GetPropertyDataOffset("Next");
}

PropertyOffsets_Fragment PropOffsets_Fragment;

static void InitPropertyOffsets_Fragment(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Fragment"));
	if (!cls)
	{
		memset(&PropOffsets_Fragment, 0xff, sizeof(PropOffsets_Fragment));
		return;
	}
	PropOffsets_Fragment.Fragments = cls->GetPropertyDataOffset("Fragments");
	PropOffsets_Fragment.bFirstHit = cls->GetPropertyDataOffset("bFirstHit");
	PropOffsets_Fragment.numFragmentTypes = cls->GetPropertyDataOffset("numFragmentTypes");
}

PropertyOffsets_WarpZoneInfo PropOffsets_WarpZoneInfo;

static void InitPropertyOffsets_WarpZoneInfo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "WarpZoneInfo"));
	if (!cls)
	{
		memset(&PropOffsets_WarpZoneInfo, 0xff, sizeof(PropOffsets_WarpZoneInfo));
		return;
	}
	PropOffsets_WarpZoneInfo.Destinations = cls->GetPropertyDataOffset("Destinations");
	PropOffsets_WarpZoneInfo.OtherSideActor = cls->GetPropertyDataOffset("OtherSideActor");
	PropOffsets_WarpZoneInfo.OtherSideLevel = cls->GetPropertyDataOffset("OtherSideLevel");
	PropOffsets_WarpZoneInfo.OtherSideURL = cls->GetPropertyDataOffset("OtherSideURL");
	PropOffsets_WarpZoneInfo.ThisTag = cls->GetPropertyDataOffset("ThisTag");
	PropOffsets_WarpZoneInfo.WarpCoords = cls->GetPropertyDataOffset("WarpCoords");
	PropOffsets_WarpZoneInfo.bNoTeleFrag = cls->GetPropertyDataOffset("bNoTeleFrag");
	PropOffsets_WarpZoneInfo.iWarpZone = cls->GetPropertyDataOffset("iWarpZone");
	PropOffsets_WarpZoneInfo.numDestinations = cls->GetPropertyDataOffset("numDestinations");
}

PropertyOffsets_Console PropOffsets_Console;

static void InitPropertyOffsets_Console(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Console"));
	if (!cls)
	{
		memset(&PropOffsets_Console, 0xff, sizeof(PropOffsets_Console));
		return;
	}
	PropOffsets_Console.AvgText = cls->GetPropertyDataOffset("AvgText");
	PropOffsets_Console.Border = cls->GetPropertyDataOffset("Border");
	PropOffsets_Console.BorderLines = cls->GetPropertyDataOffset("BorderLines");
	PropOffsets_Console.BorderPixels = cls->GetPropertyDataOffset("BorderPixels");
	PropOffsets_Console.BorderSize = cls->GetPropertyDataOffset("BorderSize");
	PropOffsets_Console.ConBackground = cls->GetPropertyDataOffset("ConBackground");
	PropOffsets_Console.ConnectingMessage = cls->GetPropertyDataOffset("ConnectingMessage");
	PropOffsets_Console.ConsoleDest = cls->GetPropertyDataOffset("ConsoleDest");
	PropOffsets_Console.ConsoleLines = cls->GetPropertyDataOffset("ConsoleLines");
	PropOffsets_Console.ConsolePos = cls->GetPropertyDataOffset("ConsolePos");
	PropOffsets_Console.ExtraTime = cls->GetPropertyDataOffset("ExtraTime");
	PropOffsets_Console.FrameCount = cls->GetPropertyDataOffset("FrameCount");
	PropOffsets_Console.FrameRateText = cls->GetPropertyDataOffset("FrameRateText");
	PropOffsets_Console.FrameX = cls->GetPropertyDataOffset("FrameX");
	PropOffsets_Console.FrameY = cls->GetPropertyDataOffset("FrameY");
	PropOffsets_Console.FramesText = cls->GetPropertyDataOffset("FramesText");
	PropOffsets_Console.History = cls->GetPropertyDataOffset("History");
	PropOffsets_Console.HistoryBot = cls->GetPropertyDataOffset("HistoryBot");
	PropOffsets_Console.HistoryCur = cls->GetPropertyDataOffset("HistoryCur");
	PropOffsets_Console.HistoryTop = cls->GetPropertyDataOffset("HistoryTop");
	PropOffsets_Console.LastFrameTime = cls->GetPropertyDataOffset("LastFrameTime");
	PropOffsets_Console.LastSecFPS = cls->GetPropertyDataOffset("LastSecFPS");
	PropOffsets_Console.LastSecText = cls->GetPropertyDataOffset("LastSecText");
	PropOffsets_Console.LastSecondFrameCount = cls->GetPropertyDataOffset("LastSecondFrameCount");
	PropOffsets_Console.LastSecondStartTime = cls->GetPropertyDataOffset("LastSecondStartTime");
	PropOffsets_Console.LoadingMessage = cls->GetPropertyDataOffset("LoadingMessage");
	PropOffsets_Console.MaxFPS = cls->GetPropertyDataOffset("MaxFPS");
	PropOffsets_Console.MaxText = cls->GetPropertyDataOffset("MaxText");
	PropOffsets_Console.MinFPS = cls->GetPropertyDataOffset("MinFPS");
	PropOffsets_Console.MinText = cls->GetPropertyDataOffset("MinText");
	PropOffsets_Console.MsgPlayer = cls->GetPropertyDataOffset("MsgPlayer");
	PropOffsets_Console.MsgText = cls->GetPropertyDataOffset("MsgText");
	PropOffsets_Console.MsgTick = cls->GetPropertyDataOffset("MsgTick");
	PropOffsets_Console.MsgTickTime = cls->GetPropertyDataOffset("MsgTickTime");
	PropOffsets_Console.MsgTime = cls->GetPropertyDataOffset("MsgTime");
	PropOffsets_Console.MsgType = cls->GetPropertyDataOffset("MsgType");
	PropOffsets_Console.PausedMessage = cls->GetPropertyDataOffset("PausedMessage");
	PropOffsets_Console.PrecachingMessage = cls->GetPropertyDataOffset("PrecachingMessage");
	PropOffsets_Console.SavingMessage = cls->GetPropertyDataOffset("SavingMessage");
	PropOffsets_Console.Scrollback = cls->GetPropertyDataOffset("Scrollback");
	PropOffsets_Console.SecondsText = cls->GetPropertyDataOffset("SecondsText");
	PropOffsets_Console.StartTime = cls->GetPropertyDataOffset("StartTime");
	PropOffsets_Console.TextLines = cls->GetPropertyDataOffset("TextLines");
	PropOffsets_Console.TimeDemoFont = cls->GetPropertyDataOffset("TimeDemoFont");
	PropOffsets_Console.TopLine = cls->GetPropertyDataOffset("TopLine");
	PropOffsets_Console.TypedStr = cls->GetPropertyDataOffset("TypedStr");
	PropOffsets_Console.Viewport = cls->GetPropertyDataOffset("Viewport");
	PropOffsets_Console.bNoDrawWorld = cls->GetPropertyDataOffset("bNoDrawWorld");
	PropOffsets_Console.bNoStuff = cls->GetPropertyDataOffset("bNoStuff");
	PropOffsets_Console.bRestartTimeDemo = cls->GetPropertyDataOffset("bRestartTimeDemo");
	PropOffsets_Console.bSaveTimeDemoToFile = cls->GetPropertyDataOffset("bSaveTimeDemoToFile");
	PropOffsets_Console.bStartTimeDemo = cls->GetPropertyDataOffset("bStartTimeDemo");
	PropOffsets_Console.bTimeDemo = cls->GetPropertyDataOffset("bTimeDemo");
	PropOffsets_Console.bTyping = cls->GetPropertyDataOffset("bTyping");
	PropOffsets_Console.fpsText = cls->GetPropertyDataOffset("fpsText");
	PropOffsets_Console.numLines = cls->GetPropertyDataOffset("numLines");
	PropOffsets_Console.vtblOut = cls->GetPropertyDataOffset("vtblOut");
}

PropertyOffsets_PlayerStart PropOffsets_PlayerStart;

static void InitPropertyOffsets_PlayerStart(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "PlayerStart"));
	if (!cls)
	{
		memset(&PropOffsets_PlayerStart, 0xff, sizeof(PropOffsets_PlayerStart));
		return;
	}
	PropOffsets_PlayerStart.TeamNumber = cls->GetPropertyDataOffset("TeamNumber");
	PropOffsets_PlayerStart.bCoopStart = cls->GetPropertyDataOffset("bCoopStart");
	PropOffsets_PlayerStart.bEnabled = cls->GetPropertyDataOffset("bEnabled");
	PropOffsets_PlayerStart.bSinglePlayerStart = cls->GetPropertyDataOffset("bSinglePlayerStart");
}

PropertyOffsets_Pickup PropOffsets_Pickup;

static void InitPropertyOffsets_Pickup(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Pickup"));
	if (!cls)
	{
		memset(&PropOffsets_Pickup, 0xff, sizeof(PropOffsets_Pickup));
		return;
	}
	PropOffsets_Pickup.ExpireMessage = cls->GetPropertyDataOffset("ExpireMessage");
	PropOffsets_Pickup.Inv = cls->GetPropertyDataOffset("Inv");
	PropOffsets_Pickup.NumCopies = cls->GetPropertyDataOffset("NumCopies");
	PropOffsets_Pickup.bAutoActivate = cls->GetPropertyDataOffset("bAutoActivate");
	PropOffsets_Pickup.bCanActivate = cls->GetPropertyDataOffset("bCanActivate");
	PropOffsets_Pickup.bCanHaveMultipleCopies = cls->GetPropertyDataOffset("bCanHaveMultipleCopies");
}

PropertyOffsets_Brush PropOffsets_Brush;

static void InitPropertyOffsets_Brush(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Brush"));
	if (!cls)
	{
		memset(&PropOffsets_Brush, 0xff, sizeof(PropOffsets_Brush));
		return;
	}
	PropOffsets_Brush.BrushColor = cls->GetPropertyDataOffset("BrushColor");
	PropOffsets_Brush.CsgOper = cls->GetPropertyDataOffset("CsgOper");
	PropOffsets_Brush.MainScale = cls->GetPropertyDataOffset("MainScale");
	PropOffsets_Brush.PolyFlags = cls->GetPropertyDataOffset("PolyFlags");
	PropOffsets_Brush.PostPivot = cls->GetPropertyDataOffset("PostPivot");
	PropOffsets_Brush.PostScale = cls->GetPropertyDataOffset("PostScale");
	PropOffsets_Brush.TempScale = cls->GetPropertyDataOffset("TempScale");
	PropOffsets_Brush.UnusedLightMesh = cls->GetPropertyDataOffset("UnusedLightMesh");
	PropOffsets_Brush.bColored = cls->GetPropertyDataOffset("bColored");
}

PropertyOffsets_ScoreBoard PropOffsets_ScoreBoard;

static void InitPropertyOffsets_ScoreBoard(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "ScoreBoard"));
	if (!cls)
	{
		memset(&PropOffsets_ScoreBoard, 0xff, sizeof(PropOffsets_ScoreBoard));
		return;
	}
	PropOffsets_ScoreBoard.OwnerHUD = cls->GetPropertyDataOffset("OwnerHUD");
	PropOffsets_ScoreBoard.RegFont = cls->GetPropertyDataOffset("RegFont");
}

PropertyOffsets_Spectator PropOffsets_Spectator;

static void InitPropertyOffsets_Spectator(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Spectator"));
	if (!cls)
	{
		memset(&PropOffsets_Spectator, 0xff, sizeof(PropOffsets_Spectator));
		return;
	}
	PropOffsets_Spectator.bChaseCam = cls->GetPropertyDataOffset("bChaseCam");
}

PropertyOffsets_InventorySpot PropOffsets_InventorySpot;

static void InitPropertyOffsets_InventorySpot(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "InventorySpot"));
	if (!cls)
	{
		memset(&PropOffsets_InventorySpot, 0xff, sizeof(PropOffsets_InventorySpot));
		return;
	}
	PropOffsets_InventorySpot.markedItem = cls->GetPropertyDataOffset("markedItem");
}

PropertyOffsets_Decal PropOffsets_Decal;

static void InitPropertyOffsets_Decal(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Decal"));
	if (!cls)
	{
		memset(&PropOffsets_Decal, 0xff, sizeof(PropOffsets_Decal));
		return;
	}
	PropOffsets_Decal.LastRenderedTime = cls->GetPropertyDataOffset("LastRenderedTime");
	PropOffsets_Decal.MultiDecalLevel = cls->GetPropertyDataOffset("MultiDecalLevel");
	PropOffsets_Decal.SurfList = cls->GetPropertyDataOffset("SurfList");
}

PropertyOffsets_PatrolPoint PropOffsets_PatrolPoint;

static void InitPropertyOffsets_PatrolPoint(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "PatrolPoint"));
	if (!cls)
	{
		memset(&PropOffsets_PatrolPoint, 0xff, sizeof(PropOffsets_PatrolPoint));
		return;
	}
	PropOffsets_PatrolPoint.AnimCount = cls->GetPropertyDataOffset("AnimCount");
	PropOffsets_PatrolPoint.NextPatrolPoint = cls->GetPropertyDataOffset("NextPatrolPoint");
	PropOffsets_PatrolPoint.Nextpatrol = cls->GetPropertyDataOffset("Nextpatrol");
	PropOffsets_PatrolPoint.PatrolAnim = cls->GetPropertyDataOffset("PatrolAnim");
	PropOffsets_PatrolPoint.PatrolSound = cls->GetPropertyDataOffset("PatrolSound");
	PropOffsets_PatrolPoint.lookDir = cls->GetPropertyDataOffset("lookDir");
	PropOffsets_PatrolPoint.numAnims = cls->GetPropertyDataOffset("numAnims");
	PropOffsets_PatrolPoint.pausetime = cls->GetPropertyDataOffset("pausetime");
}

PropertyOffsets_Counter PropOffsets_Counter;

static void InitPropertyOffsets_Counter(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Counter"));
	if (!cls)
	{
		memset(&PropOffsets_Counter, 0xff, sizeof(PropOffsets_Counter));
		return;
	}
	PropOffsets_Counter.CompleteMessage = cls->GetPropertyDataOffset("CompleteMessage");
	PropOffsets_Counter.CountMessage = cls->GetPropertyDataOffset("CountMessage");
	PropOffsets_Counter.NumToCount = cls->GetPropertyDataOffset("NumToCount");
	PropOffsets_Counter.OriginalNum = cls->GetPropertyDataOffset("OriginalNum");
	PropOffsets_Counter.bShowMessage = cls->GetPropertyDataOffset("bShowMessage");
}

PropertyOffsets_Bitmap PropOffsets_Bitmap;

static void InitPropertyOffsets_Bitmap(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Bitmap"));
	if (!cls)
	{
		memset(&PropOffsets_Bitmap, 0xff, sizeof(PropOffsets_Bitmap));
		return;
	}
	PropOffsets_Bitmap.Format = cls->GetPropertyDataOffset("Format");
	PropOffsets_Bitmap.InternalTime = cls->GetPropertyDataOffset("InternalTime");
	PropOffsets_Bitmap.MaxColor = cls->GetPropertyDataOffset("MaxColor");
	PropOffsets_Bitmap.MipZero = cls->GetPropertyDataOffset("MipZero");
	PropOffsets_Bitmap.Palette = cls->GetPropertyDataOffset("Palette");
	PropOffsets_Bitmap.UBits = cls->GetPropertyDataOffset("UBits");
	PropOffsets_Bitmap.UClamp = cls->GetPropertyDataOffset("UClamp");
	PropOffsets_Bitmap.USize = cls->GetPropertyDataOffset("USize");
	PropOffsets_Bitmap.VBits = cls->GetPropertyDataOffset("VBits");
	PropOffsets_Bitmap.VClamp = cls->GetPropertyDataOffset("VClamp");
	PropOffsets_Bitmap.VSize = cls->GetPropertyDataOffset("VSize");
}

PropertyOffsets_MapList PropOffsets_MapList;

static void InitPropertyOffsets_MapList(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "MapList"));
	if (!cls)
	{
		memset(&PropOffsets_MapList, 0xff, sizeof(PropOffsets_MapList));
		return;
	}
	PropOffsets_MapList.MapNum = cls->GetPropertyDataOffset("MapNum");
	PropOffsets_MapList.Maps = cls->GetPropertyDataOffset("Maps");
}

PropertyOffsets_Effects PropOffsets_Effects;

static void InitPropertyOffsets_Effects(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Effects"));
	if (!cls)
	{
		memset(&PropOffsets_Effects, 0xff, sizeof(PropOffsets_Effects));
		return;
	}
	PropOffsets_Effects.EffectSound1 = cls->GetPropertyDataOffset("EffectSound1");
	PropOffsets_Effects.EffectSound2 = cls->GetPropertyDataOffset("EffectSound2");
	PropOffsets_Effects.bOnlyTriggerable = cls->GetPropertyDataOffset("bOnlyTriggerable");
}

PropertyOffsets_StatLogFile PropOffsets_StatLogFile;

static void InitPropertyOffsets_StatLogFile(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "StatLogFile"));
	if (!cls)
	{
		memset(&PropOffsets_StatLogFile, 0xff, sizeof(PropOffsets_StatLogFile));
		return;
	}
	PropOffsets_StatLogFile.LogAr = cls->GetPropertyDataOffset("LogAr");
	PropOffsets_StatLogFile.StatLogFile = cls->GetPropertyDataOffset("StatLogFile");
	PropOffsets_StatLogFile.StatLogFinal = cls->GetPropertyDataOffset("StatLogFinal");
	PropOffsets_StatLogFile.bWatermark = cls->GetPropertyDataOffset("bWatermark");
}

PropertyOffsets_LevelSummary PropOffsets_LevelSummary;

static void InitPropertyOffsets_LevelSummary(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "LevelSummary"));
	if (!cls)
	{
		memset(&PropOffsets_LevelSummary, 0xff, sizeof(PropOffsets_LevelSummary));
		return;
	}
	PropOffsets_LevelSummary.Author = cls->GetPropertyDataOffset("Author");
	PropOffsets_LevelSummary.IdealPlayerCount = cls->GetPropertyDataOffset("IdealPlayerCount");
	PropOffsets_LevelSummary.LevelEnterText = cls->GetPropertyDataOffset("LevelEnterText");
	PropOffsets_LevelSummary.RecommendedEnemies = cls->GetPropertyDataOffset("RecommendedEnemies");
	PropOffsets_LevelSummary.RecommendedTeammates = cls->GetPropertyDataOffset("RecommendedTeammates");
	PropOffsets_LevelSummary.Title = cls->GetPropertyDataOffset("Title");
}

PropertyOffsets_ScriptedTexture PropOffsets_ScriptedTexture;

static void InitPropertyOffsets_ScriptedTexture(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "ScriptedTexture"));
	if (!cls)
	{
		memset(&PropOffsets_ScriptedTexture, 0xff, sizeof(PropOffsets_ScriptedTexture));
		return;
	}
	PropOffsets_ScriptedTexture.Junk1 = cls->GetPropertyDataOffset("Junk1");
	PropOffsets_ScriptedTexture.Junk2 = cls->GetPropertyDataOffset("Junk2");
	PropOffsets_ScriptedTexture.Junk3 = cls->GetPropertyDataOffset("Junk3");
	PropOffsets_ScriptedTexture.LocalTime = cls->GetPropertyDataOffset("LocalTime");
	PropOffsets_ScriptedTexture.NotifyActor = cls->GetPropertyDataOffset("NotifyActor");
	PropOffsets_ScriptedTexture.SourceTexture = cls->GetPropertyDataOffset("SourceTexture");
}

PropertyOffsets_Engine PropOffsets_Engine;

static void InitPropertyOffsets_Engine(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Engine"));
	if (!cls)
	{
		memset(&PropOffsets_Engine, 0xff, sizeof(PropOffsets_Engine));
		return;
	}
	PropOffsets_Engine.Audio = cls->GetPropertyDataOffset("Audio");
	PropOffsets_Engine.AudioDevice = cls->GetPropertyDataOffset("AudioDevice");
	PropOffsets_Engine.CacheSizeMegs = cls->GetPropertyDataOffset("CacheSizeMegs");
	PropOffsets_Engine.Client = cls->GetPropertyDataOffset("Client");
	PropOffsets_Engine.ClientCycles = cls->GetPropertyDataOffset("ClientCycles");
	PropOffsets_Engine.Console = cls->GetPropertyDataOffset("Console");
	PropOffsets_Engine.CurrentTickRate = cls->GetPropertyDataOffset("CurrentTickRate");
	PropOffsets_Engine.Cylinder = cls->GetPropertyDataOffset("Cylinder");
	PropOffsets_Engine.GameCycles = cls->GetPropertyDataOffset("GameCycles");
	PropOffsets_Engine.GameRenderDevice = cls->GetPropertyDataOffset("GameRenderDevice");
	PropOffsets_Engine.Language = cls->GetPropertyDataOffset("Language");
	PropOffsets_Engine.NetworkDevice = cls->GetPropertyDataOffset("NetworkDevice");
	PropOffsets_Engine.Render = cls->GetPropertyDataOffset("Render");
	PropOffsets_Engine.TickCycles = cls->GetPropertyDataOffset("TickCycles");
	PropOffsets_Engine.UseSound = cls->GetPropertyDataOffset("UseSound");
}

PropertyOffsets_TriggerLight PropOffsets_TriggerLight;

static void InitPropertyOffsets_TriggerLight(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "TriggerLight"));
	if (!cls)
	{
		memset(&PropOffsets_TriggerLight, 0xff, sizeof(PropOffsets_TriggerLight));
		return;
	}
	PropOffsets_TriggerLight.Alpha = cls->GetPropertyDataOffset("Alpha");
	PropOffsets_TriggerLight.ChangeTime = cls->GetPropertyDataOffset("ChangeTime");
	PropOffsets_TriggerLight.Direction = cls->GetPropertyDataOffset("Direction");
	PropOffsets_TriggerLight.InitialBrightness = cls->GetPropertyDataOffset("InitialBrightness");
	PropOffsets_TriggerLight.RemainOnTime = cls->GetPropertyDataOffset("RemainOnTime");
	PropOffsets_TriggerLight.SavedTrigger = cls->GetPropertyDataOffset("SavedTrigger");
	PropOffsets_TriggerLight.bDelayFullOn = cls->GetPropertyDataOffset("bDelayFullOn");
	PropOffsets_TriggerLight.bInitiallyOn = cls->GetPropertyDataOffset("bInitiallyOn");
	PropOffsets_TriggerLight.poundTime = cls->GetPropertyDataOffset("poundTime");
}

PropertyOffsets_SpecialEvent PropOffsets_SpecialEvent;

static void InitPropertyOffsets_SpecialEvent(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "SpecialEvent"));
	if (!cls)
	{
		memset(&PropOffsets_SpecialEvent, 0xff, sizeof(PropOffsets_SpecialEvent));
		return;
	}
	PropOffsets_SpecialEvent.Damage = cls->GetPropertyDataOffset("Damage");
	PropOffsets_SpecialEvent.DamageString = cls->GetPropertyDataOffset("DamageString");
	PropOffsets_SpecialEvent.DamageType = cls->GetPropertyDataOffset("DamageType");
	PropOffsets_SpecialEvent.Message = cls->GetPropertyDataOffset("Message");
	PropOffsets_SpecialEvent.Sound = cls->GetPropertyDataOffset("Sound");
	PropOffsets_SpecialEvent.bBroadcast = cls->GetPropertyDataOffset("bBroadcast");
	PropOffsets_SpecialEvent.bPlayerViewRot = cls->GetPropertyDataOffset("bPlayerViewRot");
}

PropertyOffsets_RoundRobin PropOffsets_RoundRobin;

static void InitPropertyOffsets_RoundRobin(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "RoundRobin"));
	if (!cls)
	{
		memset(&PropOffsets_RoundRobin, 0xff, sizeof(PropOffsets_RoundRobin));
		return;
	}
	PropOffsets_RoundRobin.OutEvents = cls->GetPropertyDataOffset("OutEvents");
	PropOffsets_RoundRobin.bLoop = cls->GetPropertyDataOffset("bLoop");
	PropOffsets_RoundRobin.i = cls->GetPropertyDataOffset("i");
}

PropertyOffsets_MusicEvent PropOffsets_MusicEvent;

static void InitPropertyOffsets_MusicEvent(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "MusicEvent"));
	if (!cls)
	{
		memset(&PropOffsets_MusicEvent, 0xff, sizeof(PropOffsets_MusicEvent));
		return;
	}
	PropOffsets_MusicEvent.CdTrack = cls->GetPropertyDataOffset("CdTrack");
	PropOffsets_MusicEvent.Song = cls->GetPropertyDataOffset("Song");
	PropOffsets_MusicEvent.SongSection = cls->GetPropertyDataOffset("SongSection");
	PropOffsets_MusicEvent.Transition = cls->GetPropertyDataOffset("Transition");
	PropOffsets_MusicEvent.bAffectAllPlayers = cls->GetPropertyDataOffset("bAffectAllPlayers");
	PropOffsets_MusicEvent.bOnceOnly = cls->GetPropertyDataOffset("bOnceOnly");
	PropOffsets_MusicEvent.bSilence = cls->GetPropertyDataOffset("bSilence");
}

PropertyOffsets_HomeBase PropOffsets_HomeBase;

static void InitPropertyOffsets_HomeBase(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "HomeBase"));
	if (!cls)
	{
		memset(&PropOffsets_HomeBase, 0xff, sizeof(PropOffsets_HomeBase));
		return;
	}
	PropOffsets_HomeBase.Extent = cls->GetPropertyDataOffset("Extent");
	PropOffsets_HomeBase.lookDir = cls->GetPropertyDataOffset("lookDir");
}

PropertyOffsets_Dispatcher PropOffsets_Dispatcher;

static void InitPropertyOffsets_Dispatcher(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Dispatcher"));
	if (!cls)
	{
		memset(&PropOffsets_Dispatcher, 0xff, sizeof(PropOffsets_Dispatcher));
		return;
	}
	PropOffsets_Dispatcher.OutDelays = cls->GetPropertyDataOffset("OutDelays");
	PropOffsets_Dispatcher.OutEvents = cls->GetPropertyDataOffset("OutEvents");
	PropOffsets_Dispatcher.i = cls->GetPropertyDataOffset("i");
}

PropertyOffsets_DemoRecSpectator PropOffsets_DemoRecSpectator;

static void InitPropertyOffsets_DemoRecSpectator(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "DemoRecSpectator"));
	if (!cls)
	{
		memset(&PropOffsets_DemoRecSpectator, 0xff, sizeof(PropOffsets_DemoRecSpectator));
		return;
	}
	PropOffsets_DemoRecSpectator.PlaybackActor = cls->GetPropertyDataOffset("PlaybackActor");
	PropOffsets_DemoRecSpectator.PlaybackGRI = cls->GetPropertyDataOffset("PlaybackGRI");
}

PropertyOffsets_DamageType PropOffsets_DamageType;

static void InitPropertyOffsets_DamageType(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "DamageType"));
	if (!cls)
	{
		memset(&PropOffsets_DamageType, 0xff, sizeof(PropOffsets_DamageType));
		return;
	}
	PropOffsets_DamageType.AltName = cls->GetPropertyDataOffset("AltName");
	PropOffsets_DamageType.DamageEffect = cls->GetPropertyDataOffset("DamageEffect");
	PropOffsets_DamageType.Name = cls->GetPropertyDataOffset("Name");
	PropOffsets_DamageType.ViewFlash = cls->GetPropertyDataOffset("ViewFlash");
	PropOffsets_DamageType.ViewFog = cls->GetPropertyDataOffset("ViewFog");
}

PropertyOffsets_Ambushpoint PropOffsets_Ambushpoint;

static void InitPropertyOffsets_Ambushpoint(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "Ambushpoint"));
	if (!cls)
	{
		memset(&PropOffsets_Ambushpoint, 0xff, sizeof(PropOffsets_Ambushpoint));
		return;
	}
	PropOffsets_Ambushpoint.SightRadius = cls->GetPropertyDataOffset("SightRadius");
	PropOffsets_Ambushpoint.bSniping = cls->GetPropertyDataOffset("bSniping");
	PropOffsets_Ambushpoint.lookDir = cls->GetPropertyDataOffset("lookDir");
	PropOffsets_Ambushpoint.survivecount = cls->GetPropertyDataOffset("survivecount");
}

PropertyOffsets_WarpZoneMarker PropOffsets_WarpZoneMarker;

static void InitPropertyOffsets_WarpZoneMarker(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "WarpZoneMarker"));
	if (!cls)
	{
		memset(&PropOffsets_WarpZoneMarker, 0xff, sizeof(PropOffsets_WarpZoneMarker));
		return;
	}
	PropOffsets_WarpZoneMarker.TriggerActor = cls->GetPropertyDataOffset("TriggerActor");
	PropOffsets_WarpZoneMarker.TriggerActor2 = cls->GetPropertyDataOffset("TriggerActor2");
	PropOffsets_WarpZoneMarker.markedWarpZone = cls->GetPropertyDataOffset("markedWarpZone");
}

PropertyOffsets_LiftCenter PropOffsets_LiftCenter;

static void InitPropertyOffsets_LiftCenter(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "LiftCenter"));
	if (!cls)
	{
		memset(&PropOffsets_LiftCenter, 0xff, sizeof(PropOffsets_LiftCenter));
		return;
	}
	PropOffsets_LiftCenter.LastTriggerTime = cls->GetPropertyDataOffset("LastTriggerTime");
	PropOffsets_LiftCenter.LiftOffset = cls->GetPropertyDataOffset("LiftOffset");
	PropOffsets_LiftCenter.LiftTag = cls->GetPropertyDataOffset("LiftTag");
	PropOffsets_LiftCenter.LiftTrigger = cls->GetPropertyDataOffset("LiftTrigger");
	PropOffsets_LiftCenter.MaxDist2D = cls->GetPropertyDataOffset("MaxDist2D");
	PropOffsets_LiftCenter.MaxZDiffAdd = cls->GetPropertyDataOffset("MaxZDiffAdd");
	PropOffsets_LiftCenter.MyLift = cls->GetPropertyDataOffset("MyLift");
	PropOffsets_LiftCenter.RecommendedTrigger = cls->GetPropertyDataOffset("RecommendedTrigger");
}

PropertyOffsets_RenderIterator PropOffsets_RenderIterator;

static void InitPropertyOffsets_RenderIterator(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Engine")->GetUObject("Class", "RenderIterator"));
	if (!cls)
	{
		memset(&PropOffsets_RenderIterator, 0xff, sizeof(PropOffsets_RenderIterator));
		return;
	}
	PropOffsets_RenderIterator.Frame = cls->GetPropertyDataOffset("Frame");
	PropOffsets_RenderIterator.Index = cls->GetPropertyDataOffset("Index");
	PropOffsets_RenderIterator.MaxItems = cls->GetPropertyDataOffset("MaxItems");
	PropOffsets_RenderIterator.Observer = cls->GetPropertyDataOffset("Observer");
}

PropertyOffsets_FractalTexture PropOffsets_FractalTexture;

static void InitPropertyOffsets_FractalTexture(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Fire")->GetUObject("Class", "FractalTexture"));
	if (!cls)
	{
		memset(&PropOffsets_FractalTexture, 0xff, sizeof(PropOffsets_FractalTexture));
		return;
	}
	PropOffsets_FractalTexture.AuxPhase = cls->GetPropertyDataOffset("AuxPhase");
	PropOffsets_FractalTexture.DrawPhase = cls->GetPropertyDataOffset("DrawPhase");
	PropOffsets_FractalTexture.GlobalPhase = cls->GetPropertyDataOffset("GlobalPhase");
	PropOffsets_FractalTexture.LightOutput = cls->GetPropertyDataOffset("LightOutput");
	PropOffsets_FractalTexture.SoundOutput = cls->GetPropertyDataOffset("SoundOutput");
	PropOffsets_FractalTexture.UMask = cls->GetPropertyDataOffset("UMask");
	PropOffsets_FractalTexture.VMask = cls->GetPropertyDataOffset("VMask");
}

PropertyOffsets_WaterTexture PropOffsets_WaterTexture;

static void InitPropertyOffsets_WaterTexture(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Fire")->GetUObject("Class", "WaterTexture"));
	if (!cls)
	{
		memset(&PropOffsets_WaterTexture, 0xff, sizeof(PropOffsets_WaterTexture));
		return;
	}
	PropOffsets_WaterTexture.DropType = cls->GetPropertyDataOffset("DropType");
	PropOffsets_WaterTexture.Drops = cls->GetPropertyDataOffset("Drops");
	PropOffsets_WaterTexture.FX_Amplitude = cls->GetPropertyDataOffset("FX_Amplitude");
	PropOffsets_WaterTexture.FX_Depth = cls->GetPropertyDataOffset("FX_Depth");
	PropOffsets_WaterTexture.FX_Frequency = cls->GetPropertyDataOffset("FX_Frequency");
	PropOffsets_WaterTexture.FX_Phase = cls->GetPropertyDataOffset("FX_Phase");
	PropOffsets_WaterTexture.FX_Radius = cls->GetPropertyDataOffset("FX_Radius");
	PropOffsets_WaterTexture.FX_Size = cls->GetPropertyDataOffset("FX_Size");
	PropOffsets_WaterTexture.FX_Speed = cls->GetPropertyDataOffset("FX_Speed");
	PropOffsets_WaterTexture.FX_Time = cls->GetPropertyDataOffset("FX_Time");
	PropOffsets_WaterTexture.NumDrops = cls->GetPropertyDataOffset("NumDrops");
	PropOffsets_WaterTexture.OldWaveAmp = cls->GetPropertyDataOffset("OldWaveAmp");
	PropOffsets_WaterTexture.RenderTable = cls->GetPropertyDataOffset("RenderTable");
	PropOffsets_WaterTexture.SourceFields = cls->GetPropertyDataOffset("SourceFields");
	PropOffsets_WaterTexture.WaterParity = cls->GetPropertyDataOffset("WaterParity");
	PropOffsets_WaterTexture.WaterTable = cls->GetPropertyDataOffset("WaterTable");
	PropOffsets_WaterTexture.WaveAmp = cls->GetPropertyDataOffset("WaveAmp");
}

PropertyOffsets_WaveTexture PropOffsets_WaveTexture;

static void InitPropertyOffsets_WaveTexture(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Fire")->GetUObject("Class", "WaveTexture"));
	if (!cls)
	{
		memset(&PropOffsets_WaveTexture, 0xff, sizeof(PropOffsets_WaveTexture));
		return;
	}
	PropOffsets_WaveTexture.BumpMapAngle = cls->GetPropertyDataOffset("BumpMapAngle");
	PropOffsets_WaveTexture.BumpMapLight = cls->GetPropertyDataOffset("BumpMapLight");
	PropOffsets_WaveTexture.PhongRange = cls->GetPropertyDataOffset("PhongRange");
	PropOffsets_WaveTexture.PhongSize = cls->GetPropertyDataOffset("PhongSize");
}

PropertyOffsets_FireTexture PropOffsets_FireTexture;

static void InitPropertyOffsets_FireTexture(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Fire")->GetUObject("Class", "FireTexture"));
	if (!cls)
	{
		memset(&PropOffsets_FireTexture, 0xff, sizeof(PropOffsets_FireTexture));
		return;
	}
	PropOffsets_FireTexture.DrawMode = cls->GetPropertyDataOffset("DrawMode");
	PropOffsets_FireTexture.FX_Area = cls->GetPropertyDataOffset("FX_Area");
	PropOffsets_FireTexture.FX_AuxSize = cls->GetPropertyDataOffset("FX_AuxSize");
	PropOffsets_FireTexture.FX_Frequency = cls->GetPropertyDataOffset("FX_Frequency");
	PropOffsets_FireTexture.FX_Heat = cls->GetPropertyDataOffset("FX_Heat");
	PropOffsets_FireTexture.FX_HorizSpeed = cls->GetPropertyDataOffset("FX_HorizSpeed");
	PropOffsets_FireTexture.FX_Phase = cls->GetPropertyDataOffset("FX_Phase");
	PropOffsets_FireTexture.FX_Size = cls->GetPropertyDataOffset("FX_Size");
	PropOffsets_FireTexture.FX_VertSpeed = cls->GetPropertyDataOffset("FX_VertSpeed");
	PropOffsets_FireTexture.NumSparks = cls->GetPropertyDataOffset("NumSparks");
	PropOffsets_FireTexture.OldRenderHeat = cls->GetPropertyDataOffset("OldRenderHeat");
	PropOffsets_FireTexture.PenDownX = cls->GetPropertyDataOffset("PenDownX");
	PropOffsets_FireTexture.PenDownY = cls->GetPropertyDataOffset("PenDownY");
	PropOffsets_FireTexture.RenderHeat = cls->GetPropertyDataOffset("RenderHeat");
	PropOffsets_FireTexture.RenderTable = cls->GetPropertyDataOffset("RenderTable");
	PropOffsets_FireTexture.SparkType = cls->GetPropertyDataOffset("SparkType");
	PropOffsets_FireTexture.Sparks = cls->GetPropertyDataOffset("Sparks");
	PropOffsets_FireTexture.SparksLimit = cls->GetPropertyDataOffset("SparksLimit");
	PropOffsets_FireTexture.StarStatus = cls->GetPropertyDataOffset("StarStatus");
	PropOffsets_FireTexture.bRising = cls->GetPropertyDataOffset("bRising");
}

PropertyOffsets_WetTexture PropOffsets_WetTexture;

static void InitPropertyOffsets_WetTexture(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Fire")->GetUObject("Class", "WetTexture"));
	if (!cls)
	{
		memset(&PropOffsets_WetTexture, 0xff, sizeof(PropOffsets_WetTexture));
		return;
	}
	PropOffsets_WetTexture.LocalSourceBitmap = cls->GetPropertyDataOffset("LocalSourceBitmap");
	PropOffsets_WetTexture.OldSourceTex = cls->GetPropertyDataOffset("OldSourceTex");
	PropOffsets_WetTexture.SourceTexture = cls->GetPropertyDataOffset("SourceTexture");
}

PropertyOffsets_IceTexture PropOffsets_IceTexture;

static void InitPropertyOffsets_IceTexture(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Fire")->GetUObject("Class", "IceTexture"));
	if (!cls)
	{
		memset(&PropOffsets_IceTexture, 0xff, sizeof(PropOffsets_IceTexture));
		return;
	}
	PropOffsets_IceTexture.Amplitude = cls->GetPropertyDataOffset("Amplitude");
	PropOffsets_IceTexture.ForceRefresh = cls->GetPropertyDataOffset("ForceRefresh");
	PropOffsets_IceTexture.Frequency = cls->GetPropertyDataOffset("Frequency");
	PropOffsets_IceTexture.GlassTexture = cls->GetPropertyDataOffset("GlassTexture");
	PropOffsets_IceTexture.HorizPanSpeed = cls->GetPropertyDataOffset("HorizPanSpeed");
	PropOffsets_IceTexture.LocalSource = cls->GetPropertyDataOffset("LocalSource");
	PropOffsets_IceTexture.MasterCount = cls->GetPropertyDataOffset("MasterCount");
	PropOffsets_IceTexture.MoveIce = cls->GetPropertyDataOffset("MoveIce");
	PropOffsets_IceTexture.OldGlassTex = cls->GetPropertyDataOffset("OldGlassTex");
	PropOffsets_IceTexture.OldSourceTex = cls->GetPropertyDataOffset("OldSourceTex");
	PropOffsets_IceTexture.OldUDisplace = cls->GetPropertyDataOffset("OldUDisplace");
	PropOffsets_IceTexture.OldVDisplace = cls->GetPropertyDataOffset("OldVDisplace");
	PropOffsets_IceTexture.PanningStyle = cls->GetPropertyDataOffset("PanningStyle");
	PropOffsets_IceTexture.SourceTexture = cls->GetPropertyDataOffset("SourceTexture");
	PropOffsets_IceTexture.TickAccu = cls->GetPropertyDataOffset("TickAccu");
	PropOffsets_IceTexture.TimeMethod = cls->GetPropertyDataOffset("TimeMethod");
	PropOffsets_IceTexture.UDisplace = cls->GetPropertyDataOffset("UDisplace");
	PropOffsets_IceTexture.UPosition = cls->GetPropertyDataOffset("UPosition");
	PropOffsets_IceTexture.VDisplace = cls->GetPropertyDataOffset("VDisplace");
	PropOffsets_IceTexture.VPosition = cls->GetPropertyDataOffset("VPosition");
	PropOffsets_IceTexture.VertPanSpeed = cls->GetPropertyDataOffset("VertPanSpeed");
}

PropertyOffsets_InternetLink PropOffsets_InternetLink;

static void InitPropertyOffsets_InternetLink(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("IpDrv")->GetUObject("Class", "InternetLink"));
	if (!cls)
	{
		memset(&PropOffsets_InternetLink, 0xff, sizeof(PropOffsets_InternetLink));
		return;
	}
	PropOffsets_InternetLink.DataPending = cls->GetPropertyDataOffset("DataPending");
	PropOffsets_InternetLink.LinkMode = cls->GetPropertyDataOffset("LinkMode");
	PropOffsets_InternetLink.Port = cls->GetPropertyDataOffset("Port");
	PropOffsets_InternetLink.PrivateResolveInfo = cls->GetPropertyDataOffset("PrivateResolveInfo");
	PropOffsets_InternetLink.ReceiveMode = cls->GetPropertyDataOffset("ReceiveMode");
	PropOffsets_InternetLink.RemoteSocket = cls->GetPropertyDataOffset("RemoteSocket");
	PropOffsets_InternetLink.Socket = cls->GetPropertyDataOffset("Socket");
}

PropertyOffsets_UdpLink PropOffsets_UdpLink;

static void InitPropertyOffsets_UdpLink(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("IpDrv")->GetUObject("Class", "UdpLink"));
	if (!cls)
	{
		memset(&PropOffsets_UdpLink, 0xff, sizeof(PropOffsets_UdpLink));
		return;
	}
	PropOffsets_UdpLink.BroadcastAddr = cls->GetPropertyDataOffset("BroadcastAddr");
}

PropertyOffsets_TcpLink PropOffsets_TcpLink;

static void InitPropertyOffsets_TcpLink(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("IpDrv")->GetUObject("Class", "TcpLink"));
	if (!cls)
	{
		memset(&PropOffsets_TcpLink, 0xff, sizeof(PropOffsets_TcpLink));
		return;
	}
	PropOffsets_TcpLink.AcceptClass = cls->GetPropertyDataOffset("AcceptClass");
	PropOffsets_TcpLink.LinkState = cls->GetPropertyDataOffset("LinkState");
	PropOffsets_TcpLink.RemoteAddr = cls->GetPropertyDataOffset("RemoteAddr");
	PropOffsets_TcpLink.SendFIFO = cls->GetPropertyDataOffset("SendFIFO");
}

PropertyOffsets_UPakPathNodeIterator PropOffsets_UPakPathNodeIterator;

static void InitPropertyOffsets_UPakPathNodeIterator(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("UPak")->GetUObject("Class", "PathNodeIterator"));
	if (!cls)
	{
		memset(&PropOffsets_UPakPathNodeIterator, 0xff, sizeof(PropOffsets_UPakPathNodeIterator));
		return;
	}
	PropOffsets_UPakPathNodeIterator.NodeCost = cls->GetPropertyDataOffset("NodeCost");
	PropOffsets_UPakPathNodeIterator.NodeCount = cls->GetPropertyDataOffset("NodeCount");
	PropOffsets_UPakPathNodeIterator.NodeIndex = cls->GetPropertyDataOffset("NodeIndex");
	PropOffsets_UPakPathNodeIterator.NodePath = cls->GetPropertyDataOffset("NodePath");
	PropOffsets_UPakPathNodeIterator.NodeStart = cls->GetPropertyDataOffset("NodeStart");
}

PropertyOffsets_UPakPawnPathNodeIterator PropOffsets_UPakPawnPathNodeIterator;

static void InitPropertyOffsets_UPakPawnPathNodeIterator(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("UPak")->GetUObject("Class", "PawnPathNodeIterator"));
	if (!cls)
	{
		memset(&PropOffsets_UPakPathNodeIterator, 0xff, sizeof(PropOffsets_UPakPathNodeIterator));
		return;
	}
	PropOffsets_UPakPawnPathNodeIterator.Pawn = cls->GetPropertyDataOffset("Pawn");
}

/////////////////////////////////////////////////////////////////////////////
// Deus Ex

PropertyOffsets_ConEvent PropOffsets_ConEvent;

static void InitPropertyOffsets_ConEvent(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEvent"));
	if (!cls)
	{
		memset(&PropOffsets_ConEvent, 0xff, sizeof(PropOffsets_ConEvent));
		return;
	}
	PropOffsets_ConEvent.Conversation = cls->GetPropertyDataOffset("Conversation");
	PropOffsets_ConEvent.Label = cls->GetPropertyDataOffset("Label");
	PropOffsets_ConEvent.eventType = cls->GetPropertyDataOffset("eventType");
	PropOffsets_ConEvent.nextEvent = cls->GetPropertyDataOffset("nextEvent");
}

PropertyOffsets_ConEventRandomLabel PropOffsets_ConEventRandomLabel;

static void InitPropertyOffsets_ConEventRandomLabel(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventRandomLabel"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventRandomLabel, 0xff, sizeof(PropOffsets_ConEventRandomLabel));
		return;
	}
	PropOffsets_ConEventRandomLabel.bCycleEvents = cls->GetPropertyDataOffset("bCycleEvents");
	PropOffsets_ConEventRandomLabel.bCycleOnce = cls->GetPropertyDataOffset("bCycleOnce");
	PropOffsets_ConEventRandomLabel.bCycleRandom = cls->GetPropertyDataOffset("bCycleRandom");
	PropOffsets_ConEventRandomLabel.bLabelsCycled = cls->GetPropertyDataOffset("bLabelsCycled");
	PropOffsets_ConEventRandomLabel.cycleIndex = cls->GetPropertyDataOffset("cycleIndex");
	PropOffsets_ConEventRandomLabel.labels = cls->GetPropertyDataOffset("labels");
}

PropertyOffsets_Conversation PropOffsets_Conversation;

static void InitPropertyOffsets_Conversation(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "Conversation"));
	if (!cls)
	{
		memset(&PropOffsets_Conversation, 0xff, sizeof(PropOffsets_Conversation));
		return;
	}
	PropOffsets_Conversation.CreatedBy = cls->GetPropertyDataOffset("CreatedBy");
	PropOffsets_Conversation.Description = cls->GetPropertyDataOffset("Description");
	PropOffsets_Conversation.audioPackageName = cls->GetPropertyDataOffset("audioPackageName");
	PropOffsets_Conversation.bCanBeInterrupted = cls->GetPropertyDataOffset("bCanBeInterrupted");
	PropOffsets_Conversation.bCannotBeInterrupted = cls->GetPropertyDataOffset("bCannotBeInterrupted");
	PropOffsets_Conversation.bDataLinkCon = cls->GetPropertyDataOffset("bDataLinkCon");
	PropOffsets_Conversation.bDisplayOnce = cls->GetPropertyDataOffset("bDisplayOnce");
	PropOffsets_Conversation.bFirstPerson = cls->GetPropertyDataOffset("bFirstPerson");
	PropOffsets_Conversation.bGenerateAudioNames = cls->GetPropertyDataOffset("bGenerateAudioNames");
	PropOffsets_Conversation.bInvokeBump = cls->GetPropertyDataOffset("bInvokeBump");
	PropOffsets_Conversation.bInvokeFrob = cls->GetPropertyDataOffset("bInvokeFrob");
	PropOffsets_Conversation.bInvokeRadius = cls->GetPropertyDataOffset("bInvokeRadius");
	PropOffsets_Conversation.bInvokeSight = cls->GetPropertyDataOffset("bInvokeSight");
	PropOffsets_Conversation.bNonInteractive = cls->GetPropertyDataOffset("bNonInteractive");
	PropOffsets_Conversation.bRandomCamera = cls->GetPropertyDataOffset("bRandomCamera");
	PropOffsets_Conversation.conName = cls->GetPropertyDataOffset("conName");
	PropOffsets_Conversation.conOwnerName = cls->GetPropertyDataOffset("conOwnerName");
	PropOffsets_Conversation.conversationID = cls->GetPropertyDataOffset("conversationID");
	PropOffsets_Conversation.eventList = cls->GetPropertyDataOffset("eventList");
	PropOffsets_Conversation.flagRefList = cls->GetPropertyDataOffset("flagRefList");
	PropOffsets_Conversation.lastPlayedTime = cls->GetPropertyDataOffset("lastPlayedTime");
	PropOffsets_Conversation.ownerRefCount = cls->GetPropertyDataOffset("ownerRefCount");
	PropOffsets_Conversation.radiusDistance = cls->GetPropertyDataOffset("radiusDistance");
}

PropertyOffsets_DeusExDecoration PropOffsets_DeusExDecoration;

static void InitPropertyOffsets_DeusExDecoration(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "DeusExDecoration"));
	if (!cls)
	{
		memset(&PropOffsets_DeusExDecoration, 0xff, sizeof(PropOffsets_DeusExDecoration));
		return;
	}
	PropOffsets_DeusExDecoration.Flammability = cls->GetPropertyDataOffset("Flammability");
	PropOffsets_DeusExDecoration.FragType = cls->GetPropertyDataOffset("FragType");
	PropOffsets_DeusExDecoration.HitPoints = cls->GetPropertyDataOffset("HitPoints");
	PropOffsets_DeusExDecoration.ItemArticle = cls->GetPropertyDataOffset("ItemArticle");
	PropOffsets_DeusExDecoration.ItemName = cls->GetPropertyDataOffset("ItemName");
	PropOffsets_DeusExDecoration.NextLabel = cls->GetPropertyDataOffset("NextLabel");
	PropOffsets_DeusExDecoration.NextState = cls->GetPropertyDataOffset("NextState");
	PropOffsets_DeusExDecoration.bCanBeBase = cls->GetPropertyDataOffset("bCanBeBase");
	PropOffsets_DeusExDecoration.bExplosive = cls->GetPropertyDataOffset("bExplosive");
	PropOffsets_DeusExDecoration.bFlammable = cls->GetPropertyDataOffset("bFlammable");
	PropOffsets_DeusExDecoration.bFloating = cls->GetPropertyDataOffset("bFloating");
	PropOffsets_DeusExDecoration.bGenerateFlies = cls->GetPropertyDataOffset("bGenerateFlies");
	PropOffsets_DeusExDecoration.bHighlight = cls->GetPropertyDataOffset("bHighlight");
	PropOffsets_DeusExDecoration.bInvincible = cls->GetPropertyDataOffset("bInvincible");
	PropOffsets_DeusExDecoration.explosionDamage = cls->GetPropertyDataOffset("explosionDamage");
	PropOffsets_DeusExDecoration.explosionRadius = cls->GetPropertyDataOffset("explosionRadius");
	PropOffsets_DeusExDecoration.flyGen = cls->GetPropertyDataOffset("flyGen");
	PropOffsets_DeusExDecoration.gradualHurtCounter = cls->GetPropertyDataOffset("gradualHurtCounter");
	PropOffsets_DeusExDecoration.gradualHurtSteps = cls->GetPropertyDataOffset("gradualHurtSteps");
	PropOffsets_DeusExDecoration.minDamageThreshold = cls->GetPropertyDataOffset("minDamageThreshold");
	PropOffsets_DeusExDecoration.moverTag = cls->GetPropertyDataOffset("moverTag");
	PropOffsets_DeusExDecoration.origRot = cls->GetPropertyDataOffset("origRot");
	PropOffsets_DeusExDecoration.pushSoundId = cls->GetPropertyDataOffset("pushSoundId");
}

PropertyOffsets_DeusExPlayer PropOffsets_DeusExPlayer;

static void InitPropertyOffsets_DeusExPlayer(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "DeusExPlayer"));
	if (!cls)
	{
		memset(&PropOffsets_DeusExPlayer, 0xff, sizeof(PropOffsets_DeusExPlayer));
		return;
	}
	PropOffsets_DeusExPlayer.ActiveComputer = cls->GetPropertyDataOffset("ActiveComputer");
	PropOffsets_DeusExPlayer.AddedNanoKey = cls->GetPropertyDataOffset("AddedNanoKey");
	PropOffsets_DeusExPlayer.AugPrefs = cls->GetPropertyDataOffset("AugPrefs");
	PropOffsets_DeusExPlayer.AugmentationSystem = cls->GetPropertyDataOffset("AugmentationSystem");
	PropOffsets_DeusExPlayer.BarkManager = cls->GetPropertyDataOffset("BarkManager");
	PropOffsets_DeusExPlayer.BleedRate = cls->GetPropertyDataOffset("BleedRate");
	PropOffsets_DeusExPlayer.BurnString = cls->GetPropertyDataOffset("BurnString");
	PropOffsets_DeusExPlayer.CanCarryOnlyOne = cls->GetPropertyDataOffset("CanCarryOnlyOne");
	PropOffsets_DeusExPlayer.CannotDropHere = cls->GetPropertyDataOffset("CannotDropHere");
	PropOffsets_DeusExPlayer.CannotLift = cls->GetPropertyDataOffset("CannotLift");
	PropOffsets_DeusExPlayer.ClientinHandPending = cls->GetPropertyDataOffset("ClientinHandPending");
	PropOffsets_DeusExPlayer.ClotPeriod = cls->GetPropertyDataOffset("ClotPeriod");
	PropOffsets_DeusExPlayer.CombatDifficulty = cls->GetPropertyDataOffset("CombatDifficulty");
	PropOffsets_DeusExPlayer.ConHistory = cls->GetPropertyDataOffset("ConHistory");
	PropOffsets_DeusExPlayer.ConPlay = cls->GetPropertyDataOffset("ConPlay");
	PropOffsets_DeusExPlayer.ConversationActor = cls->GetPropertyDataOffset("ConversationActor");
	PropOffsets_DeusExPlayer.Credits = cls->GetPropertyDataOffset("Credits");
	PropOffsets_DeusExPlayer.DXGame = cls->GetPropertyDataOffset("DXGame");
	PropOffsets_DeusExPlayer.DamageShield = cls->GetPropertyDataOffset("DamageShield");
	PropOffsets_DeusExPlayer.DataLinkPlay = cls->GetPropertyDataOffset("DataLinkPlay");
	PropOffsets_DeusExPlayer.DropCounter = cls->GetPropertyDataOffset("DropCounter");
	PropOffsets_DeusExPlayer.Energy = cls->GetPropertyDataOffset("Energy");
	PropOffsets_DeusExPlayer.EnergyDepleted = cls->GetPropertyDataOffset("EnergyDepleted");
	PropOffsets_DeusExPlayer.EnergyDrain = cls->GetPropertyDataOffset("EnergyDrain");
	PropOffsets_DeusExPlayer.EnergyDrainTotal = cls->GetPropertyDataOffset("EnergyDrainTotal");
	PropOffsets_DeusExPlayer.EnergyMax = cls->GetPropertyDataOffset("EnergyMax");
	PropOffsets_DeusExPlayer.FirstGoal = cls->GetPropertyDataOffset("FirstGoal");
	PropOffsets_DeusExPlayer.FirstImage = cls->GetPropertyDataOffset("FirstImage");
	PropOffsets_DeusExPlayer.FirstLog = cls->GetPropertyDataOffset("FirstLog");
	PropOffsets_DeusExPlayer.FirstNote = cls->GetPropertyDataOffset("FirstNote");
	PropOffsets_DeusExPlayer.FlashTimer = cls->GetPropertyDataOffset("FlashTimer");
	PropOffsets_DeusExPlayer.FloorMaterial = cls->GetPropertyDataOffset("FloorMaterial");
	PropOffsets_DeusExPlayer.FrobTarget = cls->GetPropertyDataOffset("FrobTarget");
	PropOffsets_DeusExPlayer.FrobTime = cls->GetPropertyDataOffset("FrobTime");
	PropOffsets_DeusExPlayer.GlobalDebugObj = cls->GetPropertyDataOffset("GlobalDebugObj");
	PropOffsets_DeusExPlayer.GoalAdded = cls->GetPropertyDataOffset("GoalAdded");
	PropOffsets_DeusExPlayer.HUDThemeName = cls->GetPropertyDataOffset("HUDThemeName");
	PropOffsets_DeusExPlayer.HandsFull = cls->GetPropertyDataOffset("HandsFull");
	PropOffsets_DeusExPlayer.HeadString = cls->GetPropertyDataOffset("HeadString");
	PropOffsets_DeusExPlayer.HealedPointLabel = cls->GetPropertyDataOffset("HealedPointLabel");
	PropOffsets_DeusExPlayer.HealedPointsLabel = cls->GetPropertyDataOffset("HealedPointsLabel");
	PropOffsets_DeusExPlayer.InventoryFull = cls->GetPropertyDataOffset("InventoryFull");
	PropOffsets_DeusExPlayer.JoltMagnitude = cls->GetPropertyDataOffset("JoltMagnitude");
	PropOffsets_DeusExPlayer.KeyList = cls->GetPropertyDataOffset("KeyList");
	PropOffsets_DeusExPlayer.KeyRing = cls->GetPropertyDataOffset("KeyRing");
	PropOffsets_DeusExPlayer.LastGoal = cls->GetPropertyDataOffset("LastGoal");
	PropOffsets_DeusExPlayer.LastLog = cls->GetPropertyDataOffset("LastLog");
	PropOffsets_DeusExPlayer.LastNote = cls->GetPropertyDataOffset("LastNote");
	PropOffsets_DeusExPlayer.LastRefreshTime = cls->GetPropertyDataOffset("LastRefreshTime");
	PropOffsets_DeusExPlayer.LastinHand = cls->GetPropertyDataOffset("LastinHand");
	PropOffsets_DeusExPlayer.LegsString = cls->GetPropertyDataOffset("LegsString");
	PropOffsets_DeusExPlayer.MPDamageMult = cls->GetPropertyDataOffset("MPDamageMult");
	PropOffsets_DeusExPlayer.MaxFrobDistance = cls->GetPropertyDataOffset("MaxFrobDistance");
	PropOffsets_DeusExPlayer.MaxRegenPoint = cls->GetPropertyDataOffset("MaxRegenPoint");
	PropOffsets_DeusExPlayer.MenuThemeName = cls->GetPropertyDataOffset("MenuThemeName");
	PropOffsets_DeusExPlayer.NextMap = cls->GetPropertyDataOffset("NextMap");
	PropOffsets_DeusExPlayer.NintendoImmunityTime = cls->GetPropertyDataOffset("NintendoImmunityTime");
	PropOffsets_DeusExPlayer.NintendoImmunityTimeLeft = cls->GetPropertyDataOffset("NintendoImmunityTimeLeft");
	PropOffsets_DeusExPlayer.NoRoomToLift = cls->GetPropertyDataOffset("NoRoomToLift");
	PropOffsets_DeusExPlayer.NoneString = cls->GetPropertyDataOffset("NoneString");
	PropOffsets_DeusExPlayer.NoteAdded = cls->GetPropertyDataOffset("NoteAdded");
	PropOffsets_DeusExPlayer.PlayerSkin = cls->GetPropertyDataOffset("PlayerSkin");
	PropOffsets_DeusExPlayer.PoisonString = cls->GetPropertyDataOffset("PoisonString");
	PropOffsets_DeusExPlayer.PrimaryGoalCompleted = cls->GetPropertyDataOffset("PrimaryGoalCompleted");
	PropOffsets_DeusExPlayer.QuickSaveGameTitle = cls->GetPropertyDataOffset("QuickSaveGameTitle");
	PropOffsets_DeusExPlayer.RegenRate = cls->GetPropertyDataOffset("RegenRate");
	PropOffsets_DeusExPlayer.RunSilentValue = cls->GetPropertyDataOffset("RunSilentValue");
	PropOffsets_DeusExPlayer.SecondaryGoalCompleted = cls->GetPropertyDataOffset("SecondaryGoalCompleted");
	PropOffsets_DeusExPlayer.ServerTimeDiff = cls->GetPropertyDataOffset("ServerTimeDiff");
	PropOffsets_DeusExPlayer.ServerTimeLastRefresh = cls->GetPropertyDataOffset("ServerTimeLastRefresh");
	PropOffsets_DeusExPlayer.ShieldStatus = cls->GetPropertyDataOffset("ShieldStatus");
	PropOffsets_DeusExPlayer.ShieldTimer = cls->GetPropertyDataOffset("ShieldTimer");
	PropOffsets_DeusExPlayer.SkillPointsAvail = cls->GetPropertyDataOffset("SkillPointsAvail");
	PropOffsets_DeusExPlayer.SkillPointsAward = cls->GetPropertyDataOffset("SkillPointsAward");
	PropOffsets_DeusExPlayer.SkillPointsTotal = cls->GetPropertyDataOffset("SkillPointsTotal");
	PropOffsets_DeusExPlayer.SkillSystem = cls->GetPropertyDataOffset("SkillSystem");
	PropOffsets_DeusExPlayer.TakenOverString = cls->GetPropertyDataOffset("TakenOverString");
	PropOffsets_DeusExPlayer.ThemeManager = cls->GetPropertyDataOffset("ThemeManager");
	PropOffsets_DeusExPlayer.TooHeavyToLift = cls->GetPropertyDataOffset("TooHeavyToLift");
	PropOffsets_DeusExPlayer.TooMuchAmmo = cls->GetPropertyDataOffset("TooMuchAmmo");
	PropOffsets_DeusExPlayer.TorsoString = cls->GetPropertyDataOffset("TorsoString");
	PropOffsets_DeusExPlayer.TruePlayerName = cls->GetPropertyDataOffset("TruePlayerName");
	PropOffsets_DeusExPlayer.UIBackground = cls->GetPropertyDataOffset("UIBackground");
	PropOffsets_DeusExPlayer.ViewModelActor = cls->GetPropertyDataOffset("ViewModelActor");
	PropOffsets_DeusExPlayer.WallMaterial = cls->GetPropertyDataOffset("WallMaterial");
	PropOffsets_DeusExPlayer.WallNormal = cls->GetPropertyDataOffset("WallNormal");
	PropOffsets_DeusExPlayer.WarrenSlot = cls->GetPropertyDataOffset("WarrenSlot");
	PropOffsets_DeusExPlayer.WarrenTimer = cls->GetPropertyDataOffset("WarrenTimer");
	PropOffsets_DeusExPlayer.WeaponUnCloak = cls->GetPropertyDataOffset("WeaponUnCloak");
	PropOffsets_DeusExPlayer.WithString = cls->GetPropertyDataOffset("WithString");
	PropOffsets_DeusExPlayer.WithTheString = cls->GetPropertyDataOffset("WithTheString");
	PropOffsets_DeusExPlayer.aDrone = cls->GetPropertyDataOffset("aDrone");
	PropOffsets_DeusExPlayer.bAlwaysRun = cls->GetPropertyDataOffset("bAlwaysRun");
	PropOffsets_DeusExPlayer.bAmmoDisplayVisible = cls->GetPropertyDataOffset("bAmmoDisplayVisible");
	PropOffsets_DeusExPlayer.bAskedToTrain = cls->GetPropertyDataOffset("bAskedToTrain");
	PropOffsets_DeusExPlayer.bAugDisplayVisible = cls->GetPropertyDataOffset("bAugDisplayVisible");
	PropOffsets_DeusExPlayer.bAutoReload = cls->GetPropertyDataOffset("bAutoReload");
	PropOffsets_DeusExPlayer.bBeltIsMPInventory = cls->GetPropertyDataOffset("bBeltIsMPInventory");
	PropOffsets_DeusExPlayer.bBuySkills = cls->GetPropertyDataOffset("bBuySkills");
	PropOffsets_DeusExPlayer.bCanLean = cls->GetPropertyDataOffset("bCanLean");
	PropOffsets_DeusExPlayer.bCompassVisible = cls->GetPropertyDataOffset("bCompassVisible");
	PropOffsets_DeusExPlayer.bConfirmNoteDeletes = cls->GetPropertyDataOffset("bConfirmNoteDeletes");
	PropOffsets_DeusExPlayer.bConfirmSaveDeletes = cls->GetPropertyDataOffset("bConfirmSaveDeletes");
	PropOffsets_DeusExPlayer.bCrosshairVisible = cls->GetPropertyDataOffset("bCrosshairVisible");
	PropOffsets_DeusExPlayer.bCrouchOn = cls->GetPropertyDataOffset("bCrouchOn");
	PropOffsets_DeusExPlayer.bDisplayAllGoals = cls->GetPropertyDataOffset("bDisplayAllGoals");
	PropOffsets_DeusExPlayer.bDisplayAmmoByClip = cls->GetPropertyDataOffset("bDisplayAmmoByClip");
	PropOffsets_DeusExPlayer.bDisplayCompletedGoals = cls->GetPropertyDataOffset("bDisplayCompletedGoals");
	PropOffsets_DeusExPlayer.bFirstOptionsSynced = cls->GetPropertyDataOffset("bFirstOptionsSynced");
	PropOffsets_DeusExPlayer.bForceDuck = cls->GetPropertyDataOffset("bForceDuck");
	PropOffsets_DeusExPlayer.bHUDBackgroundTranslucent = cls->GetPropertyDataOffset("bHUDBackgroundTranslucent");
	PropOffsets_DeusExPlayer.bHUDBordersTranslucent = cls->GetPropertyDataOffset("bHUDBordersTranslucent");
	PropOffsets_DeusExPlayer.bHUDBordersVisible = cls->GetPropertyDataOffset("bHUDBordersVisible");
	PropOffsets_DeusExPlayer.bHUDShowAllAugs = cls->GetPropertyDataOffset("bHUDShowAllAugs");
	PropOffsets_DeusExPlayer.bHelpMessages = cls->GetPropertyDataOffset("bHelpMessages");
	PropOffsets_DeusExPlayer.bHitDisplayVisible = cls->GetPropertyDataOffset("bHitDisplayVisible");
	PropOffsets_DeusExPlayer.bIgnoreNextShowMenu = cls->GetPropertyDataOffset("bIgnoreNextShowMenu");
	PropOffsets_DeusExPlayer.bInHandTransition = cls->GetPropertyDataOffset("bInHandTransition");
	PropOffsets_DeusExPlayer.bKillerProfile = cls->GetPropertyDataOffset("bKillerProfile");
	PropOffsets_DeusExPlayer.bMenusTranslucent = cls->GetPropertyDataOffset("bMenusTranslucent");
	PropOffsets_DeusExPlayer.bNPCHighlighting = cls->GetPropertyDataOffset("bNPCHighlighting");
	PropOffsets_DeusExPlayer.bNintendoImmunity = cls->GetPropertyDataOffset("bNintendoImmunity");
	PropOffsets_DeusExPlayer.bObjectBeltVisible = cls->GetPropertyDataOffset("bObjectBeltVisible");
	PropOffsets_DeusExPlayer.bObjectNames = cls->GetPropertyDataOffset("bObjectNames");
	PropOffsets_DeusExPlayer.bQuotesEnabled = cls->GetPropertyDataOffset("bQuotesEnabled");
	PropOffsets_DeusExPlayer.bSavingSkillsAugs = cls->GetPropertyDataOffset("bSavingSkillsAugs");
	PropOffsets_DeusExPlayer.bSecondOptionsSynced = cls->GetPropertyDataOffset("bSecondOptionsSynced");
	PropOffsets_DeusExPlayer.bShowAmmoDescriptions = cls->GetPropertyDataOffset("bShowAmmoDescriptions");
	PropOffsets_DeusExPlayer.bSpyDroneActive = cls->GetPropertyDataOffset("bSpyDroneActive");
	PropOffsets_DeusExPlayer.bStartNewGameAfterIntro = cls->GetPropertyDataOffset("bStartNewGameAfterIntro");
	PropOffsets_DeusExPlayer.bStartingNewGame = cls->GetPropertyDataOffset("bStartingNewGame");
	PropOffsets_DeusExPlayer.bSubtitles = cls->GetPropertyDataOffset("bSubtitles");
	PropOffsets_DeusExPlayer.bToggleCrouch = cls->GetPropertyDataOffset("bToggleCrouch");
	PropOffsets_DeusExPlayer.bToggleWalk = cls->GetPropertyDataOffset("bToggleWalk");
	PropOffsets_DeusExPlayer.bWarrenEMPField = cls->GetPropertyDataOffset("bWarrenEMPField");
	PropOffsets_DeusExPlayer.bWasCrouchOn = cls->GetPropertyDataOffset("bWasCrouchOn");
	PropOffsets_DeusExPlayer.curLeanDist = cls->GetPropertyDataOffset("curLeanDist");
	PropOffsets_DeusExPlayer.drugEffectTimer = cls->GetPropertyDataOffset("drugEffectTimer");
	PropOffsets_DeusExPlayer.inHand = cls->GetPropertyDataOffset("inHand");
	PropOffsets_DeusExPlayer.inHandPending = cls->GetPropertyDataOffset("inHandPending");
	PropOffsets_DeusExPlayer.invSlots = cls->GetPropertyDataOffset("invSlots");
	PropOffsets_DeusExPlayer.invulnSph = cls->GetPropertyDataOffset("invulnSph");
	PropOffsets_DeusExPlayer.killProfile = cls->GetPropertyDataOffset("killProfile");
	PropOffsets_DeusExPlayer.lastFirstPersonConvoActor = cls->GetPropertyDataOffset("lastFirstPersonConvoActor");
	PropOffsets_DeusExPlayer.lastFirstPersonConvoTime = cls->GetPropertyDataOffset("lastFirstPersonConvoTime");
	PropOffsets_DeusExPlayer.lastThirdPersonConvoActor = cls->GetPropertyDataOffset("lastThirdPersonConvoActor");
	PropOffsets_DeusExPlayer.lastThirdPersonConvoTime = cls->GetPropertyDataOffset("lastThirdPersonConvoTime");
	PropOffsets_DeusExPlayer.lastbDuck = cls->GetPropertyDataOffset("lastbDuck");
	PropOffsets_DeusExPlayer.logTimeout = cls->GetPropertyDataOffset("logTimeout");
	PropOffsets_DeusExPlayer.maxInvCols = cls->GetPropertyDataOffset("maxInvCols");
	PropOffsets_DeusExPlayer.maxInvRows = cls->GetPropertyDataOffset("maxInvRows");
	PropOffsets_DeusExPlayer.maxLogLines = cls->GetPropertyDataOffset("maxLogLines");
	PropOffsets_DeusExPlayer.mpMsgCode = cls->GetPropertyDataOffset("mpMsgCode");
	PropOffsets_DeusExPlayer.mpMsgFlags = cls->GetPropertyDataOffset("mpMsgFlags");
	PropOffsets_DeusExPlayer.mpMsgOptionalParam = cls->GetPropertyDataOffset("mpMsgOptionalParam");
	PropOffsets_DeusExPlayer.mpMsgOptionalString = cls->GetPropertyDataOffset("mpMsgOptionalString");
	PropOffsets_DeusExPlayer.mpMsgServerFlags = cls->GetPropertyDataOffset("mpMsgServerFlags");
	PropOffsets_DeusExPlayer.mpMsgTime = cls->GetPropertyDataOffset("mpMsgTime");
	PropOffsets_DeusExPlayer.musicChangeTimer = cls->GetPropertyDataOffset("musicChangeTimer");
	PropOffsets_DeusExPlayer.musicCheckTimer = cls->GetPropertyDataOffset("musicCheckTimer");
	PropOffsets_DeusExPlayer.musicMode = cls->GetPropertyDataOffset("musicMode");
	PropOffsets_DeusExPlayer.myBurner = cls->GetPropertyDataOffset("myBurner");
	PropOffsets_DeusExPlayer.myKiller = cls->GetPropertyDataOffset("myKiller");
	PropOffsets_DeusExPlayer.myPoisoner = cls->GetPropertyDataOffset("myPoisoner");
	PropOffsets_DeusExPlayer.myProjKiller = cls->GetPropertyDataOffset("myProjKiller");
	PropOffsets_DeusExPlayer.myTurretKiller = cls->GetPropertyDataOffset("myTurretKiller");
	PropOffsets_DeusExPlayer.poisonCounter = cls->GetPropertyDataOffset("poisonCounter");
	PropOffsets_DeusExPlayer.poisonDamage = cls->GetPropertyDataOffset("poisonDamage");
	PropOffsets_DeusExPlayer.poisonTimer = cls->GetPropertyDataOffset("poisonTimer");
	PropOffsets_DeusExPlayer.prevLeanDist = cls->GetPropertyDataOffset("prevLeanDist");
	PropOffsets_DeusExPlayer.saveCount = cls->GetPropertyDataOffset("saveCount");
	PropOffsets_DeusExPlayer.saveTime = cls->GetPropertyDataOffset("saveTime");
	PropOffsets_DeusExPlayer.savedSection = cls->GetPropertyDataOffset("savedSection");
	PropOffsets_DeusExPlayer.spyDroneLevel = cls->GetPropertyDataOffset("spyDroneLevel");
	PropOffsets_DeusExPlayer.spyDroneLevelValue = cls->GetPropertyDataOffset("spyDroneLevelValue");
	PropOffsets_DeusExPlayer.strStartMap = cls->GetPropertyDataOffset("strStartMap");
	PropOffsets_DeusExPlayer.swimBubbleTimer = cls->GetPropertyDataOffset("swimBubbleTimer");
	PropOffsets_DeusExPlayer.swimDuration = cls->GetPropertyDataOffset("swimDuration");
	PropOffsets_DeusExPlayer.swimTimer = cls->GetPropertyDataOffset("swimTimer");
	PropOffsets_DeusExPlayer.translucencyLevel = cls->GetPropertyDataOffset("translucencyLevel");
}

PropertyOffsets_DeusExSaveInfo PropOffsets_DeusExSaveInfo;

static void InitPropertyOffsets_DeusExSaveInfo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "DeusExSaveInfo"));
	if (!cls)
	{
		memset(&PropOffsets_DeusExSaveInfo, 0xff, sizeof(PropOffsets_DeusExSaveInfo));
		return;
	}
	PropOffsets_DeusExSaveInfo.Day = cls->GetPropertyDataOffset("Day");
	PropOffsets_DeusExSaveInfo.Description = cls->GetPropertyDataOffset("Description");
	PropOffsets_DeusExSaveInfo.DirectoryIndex = cls->GetPropertyDataOffset("DirectoryIndex");
	PropOffsets_DeusExSaveInfo.Hour = cls->GetPropertyDataOffset("Hour");
	PropOffsets_DeusExSaveInfo.MapName = cls->GetPropertyDataOffset("MapName");
	PropOffsets_DeusExSaveInfo.Minute = cls->GetPropertyDataOffset("Minute");
	PropOffsets_DeusExSaveInfo.MissionLocation = cls->GetPropertyDataOffset("MissionLocation");
	PropOffsets_DeusExSaveInfo.Month = cls->GetPropertyDataOffset("Month");
	PropOffsets_DeusExSaveInfo.Second = cls->GetPropertyDataOffset("Second");
	PropOffsets_DeusExSaveInfo.Snapshot = cls->GetPropertyDataOffset("Snapshot");
	PropOffsets_DeusExSaveInfo.Year = cls->GetPropertyDataOffset("Year");
	PropOffsets_DeusExSaveInfo.bCheatsEnabled = cls->GetPropertyDataOffset("bCheatsEnabled");
	PropOffsets_DeusExSaveInfo.saveCount = cls->GetPropertyDataOffset("saveCount");
	PropOffsets_DeusExSaveInfo.saveTime = cls->GetPropertyDataOffset("saveTime");
}

PropertyOffsets_DumpLocation PropOffsets_DumpLocation;

static void InitPropertyOffsets_DumpLocation(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "DumpLocation"));
	if (!cls)
	{
		memset(&PropOffsets_DumpLocation, 0xff, sizeof(PropOffsets_DumpLocation));
		return;
	}
	PropOffsets_DumpLocation.Player = cls->GetPropertyDataOffset("Player");
	PropOffsets_DumpLocation.currentDumpFileIndex = cls->GetPropertyDataOffset("currentDumpFileIndex");
	PropOffsets_DumpLocation.currentDumpFileLocation = cls->GetPropertyDataOffset("currentDumpFileLocation");
	PropOffsets_DumpLocation.currentDumpLocation = cls->GetPropertyDataOffset("currentDumpLocation");
	PropOffsets_DumpLocation.currentDumpLocationIndex = cls->GetPropertyDataOffset("currentDumpLocationIndex");
	PropOffsets_DumpLocation.currentUser = cls->GetPropertyDataOffset("currentUser");
	PropOffsets_DumpLocation.dumpFile = cls->GetPropertyDataOffset("dumpFile");
	PropOffsets_DumpLocation.dumpFileDirectory = cls->GetPropertyDataOffset("dumpFileDirectory");
	PropOffsets_DumpLocation.dumpLocationCount = cls->GetPropertyDataOffset("dumpLocationCount");
}

PropertyOffsets_GameDirectory PropOffsets_GameDirectory;

static void InitPropertyOffsets_GameDirectory(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "GameDirectory"));
	if (!cls)
	{
		memset(&PropOffsets_GameDirectory, 0xff, sizeof(PropOffsets_GameDirectory));
		return;
	}
	PropOffsets_GameDirectory.currentFilter = cls->GetPropertyDataOffset("currentFilter");
	PropOffsets_GameDirectory.directoryList = cls->GetPropertyDataOffset("directoryList");
	PropOffsets_GameDirectory.gameDirectoryType = cls->GetPropertyDataOffset("gameDirectoryType");
	PropOffsets_GameDirectory.loadedSaveInfoPointers = cls->GetPropertyDataOffset("loadedSaveInfoPointers");
	PropOffsets_GameDirectory.tempSaveInfo = cls->GetPropertyDataOffset("tempSaveInfo");
}

PropertyOffsets_ParticleIterator PropOffsets_ParticleIterator;

static void InitPropertyOffsets_ParticleIterator(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "ParticleIterator"));
	if (!cls)
	{
		memset(&PropOffsets_ParticleIterator, 0xff, sizeof(PropOffsets_ParticleIterator));
		return;
	}
	PropOffsets_ParticleIterator.OwnerDrawScale = cls->GetPropertyDataOffset("OwnerDrawScale");
	PropOffsets_ParticleIterator.OwnerLifeSpan = cls->GetPropertyDataOffset("OwnerLifeSpan");
	PropOffsets_ParticleIterator.OwnerRiseRate = cls->GetPropertyDataOffset("OwnerRiseRate");
	PropOffsets_ParticleIterator.OwnerZoneGravity = cls->GetPropertyDataOffset("OwnerZoneGravity");
	PropOffsets_ParticleIterator.Particles = cls->GetPropertyDataOffset("Particles");
	PropOffsets_ParticleIterator.bOwnerFades = cls->GetPropertyDataOffset("bOwnerFades");
	PropOffsets_ParticleIterator.bOwnerScales = cls->GetPropertyDataOffset("bOwnerScales");
	PropOffsets_ParticleIterator.bOwnerUsesGravity = cls->GetPropertyDataOffset("bOwnerUsesGravity");
	PropOffsets_ParticleIterator.nextFreeParticle = cls->GetPropertyDataOffset("nextFreeParticle");
	PropOffsets_ParticleIterator.proxy = cls->GetPropertyDataOffset("proxy");
}

PropertyOffsets_ScriptedPawn PropOffsets_ScriptedPawn;

static void InitPropertyOffsets_ScriptedPawn(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "ScriptedPawn"));
	if (!cls)
	{
		memset(&PropOffsets_ScriptedPawn, 0xff, sizeof(PropOffsets_ScriptedPawn));
		return;
	}
	PropOffsets_ScriptedPawn.ActorAvoiding = cls->GetPropertyDataOffset("ActorAvoiding");
	PropOffsets_ScriptedPawn.AgitationCheckTimer = cls->GetPropertyDataOffset("AgitationCheckTimer");
	PropOffsets_ScriptedPawn.AgitationDecayRate = cls->GetPropertyDataOffset("AgitationDecayRate");
	PropOffsets_ScriptedPawn.AgitationSustainTime = cls->GetPropertyDataOffset("AgitationSustainTime");
	PropOffsets_ScriptedPawn.AgitationTimer = cls->GetPropertyDataOffset("AgitationTimer");
	PropOffsets_ScriptedPawn.AlarmActor = cls->GetPropertyDataOffset("AlarmActor");
	PropOffsets_ScriptedPawn.AlarmTimer = cls->GetPropertyDataOffset("AlarmTimer");
	PropOffsets_ScriptedPawn.AlliancesEx = cls->GetPropertyDataOffset("AlliancesEx");
	PropOffsets_ScriptedPawn.AvoidAccuracy = cls->GetPropertyDataOffset("AvoidAccuracy");
	PropOffsets_ScriptedPawn.AvoidBumpTimer = cls->GetPropertyDataOffset("AvoidBumpTimer");
	PropOffsets_ScriptedPawn.AvoidWallTimer = cls->GetPropertyDataOffset("AvoidWallTimer");
	PropOffsets_ScriptedPawn.BackpedalTimer = cls->GetPropertyDataOffset("BackpedalTimer");
	PropOffsets_ScriptedPawn.BaseAccuracy = cls->GetPropertyDataOffset("BaseAccuracy");
	PropOffsets_ScriptedPawn.BaseAssHeight = cls->GetPropertyDataOffset("BaseAssHeight");
	PropOffsets_ScriptedPawn.BeamCheckTimer = cls->GetPropertyDataOffset("BeamCheckTimer");
	PropOffsets_ScriptedPawn.BleedRate = cls->GetPropertyDataOffset("BleedRate");
	PropOffsets_ScriptedPawn.BodyIndex = cls->GetPropertyDataOffset("BodyIndex");
	PropOffsets_ScriptedPawn.BurnPeriod = cls->GetPropertyDataOffset("BurnPeriod");
	PropOffsets_ScriptedPawn.CarcassCheckTimer = cls->GetPropertyDataOffset("CarcassCheckTimer");
	PropOffsets_ScriptedPawn.CarcassHateTimer = cls->GetPropertyDataOffset("CarcassHateTimer");
	PropOffsets_ScriptedPawn.CarcassTimer = cls->GetPropertyDataOffset("CarcassTimer");
	PropOffsets_ScriptedPawn.CarcassType = cls->GetPropertyDataOffset("CarcassType");
	PropOffsets_ScriptedPawn.Carcasses = cls->GetPropertyDataOffset("Carcasses");
	PropOffsets_ScriptedPawn.CheckPeriod = cls->GetPropertyDataOffset("CheckPeriod");
	PropOffsets_ScriptedPawn.CloakEMPTimer = cls->GetPropertyDataOffset("CloakEMPTimer");
	PropOffsets_ScriptedPawn.CloakThreshold = cls->GetPropertyDataOffset("CloakThreshold");
	PropOffsets_ScriptedPawn.CloseCombatMult = cls->GetPropertyDataOffset("CloseCombatMult");
	PropOffsets_ScriptedPawn.ClotPeriod = cls->GetPropertyDataOffset("ClotPeriod");
	PropOffsets_ScriptedPawn.ConvOrderTag = cls->GetPropertyDataOffset("ConvOrderTag");
	PropOffsets_ScriptedPawn.ConvOrders = cls->GetPropertyDataOffset("ConvOrders");
	PropOffsets_ScriptedPawn.ConversationActor = cls->GetPropertyDataOffset("ConversationActor");
	PropOffsets_ScriptedPawn.Cowardice = cls->GetPropertyDataOffset("Cowardice");
	PropOffsets_ScriptedPawn.CrouchRate = cls->GetPropertyDataOffset("CrouchRate");
	PropOffsets_ScriptedPawn.CrouchTimer = cls->GetPropertyDataOffset("CrouchTimer");
	PropOffsets_ScriptedPawn.CycleCandidate = cls->GetPropertyDataOffset("CycleCandidate");
	PropOffsets_ScriptedPawn.CycleCumulative = cls->GetPropertyDataOffset("CycleCumulative");
	PropOffsets_ScriptedPawn.CycleDistance = cls->GetPropertyDataOffset("CycleDistance");
	PropOffsets_ScriptedPawn.CyclePeriod = cls->GetPropertyDataOffset("CyclePeriod");
	PropOffsets_ScriptedPawn.CycleTimer = cls->GetPropertyDataOffset("CycleTimer");
	PropOffsets_ScriptedPawn.DeathTimer = cls->GetPropertyDataOffset("DeathTimer");
	PropOffsets_ScriptedPawn.DesiredPrePivot = cls->GetPropertyDataOffset("DesiredPrePivot");
	PropOffsets_ScriptedPawn.DestAttempts = cls->GetPropertyDataOffset("DestAttempts");
	PropOffsets_ScriptedPawn.DistressTimer = cls->GetPropertyDataOffset("DistressTimer");
	PropOffsets_ScriptedPawn.DropCounter = cls->GetPropertyDataOffset("DropCounter");
	PropOffsets_ScriptedPawn.EnemyLastSeen = cls->GetPropertyDataOffset("EnemyLastSeen");
	PropOffsets_ScriptedPawn.EnemyReadiness = cls->GetPropertyDataOffset("EnemyReadiness");
	PropOffsets_ScriptedPawn.EnemyTimeout = cls->GetPropertyDataOffset("EnemyTimeout");
	PropOffsets_ScriptedPawn.EnemyTimer = cls->GetPropertyDataOffset("EnemyTimer");
	PropOffsets_ScriptedPawn.FearDecayRate = cls->GetPropertyDataOffset("FearDecayRate");
	PropOffsets_ScriptedPawn.FearLevel = cls->GetPropertyDataOffset("FearLevel");
	PropOffsets_ScriptedPawn.FearSustainTime = cls->GetPropertyDataOffset("FearSustainTime");
	PropOffsets_ScriptedPawn.FearTimer = cls->GetPropertyDataOffset("FearTimer");
	PropOffsets_ScriptedPawn.FireAngle = cls->GetPropertyDataOffset("FireAngle");
	PropOffsets_ScriptedPawn.FireElevation = cls->GetPropertyDataOffset("FireElevation");
	PropOffsets_ScriptedPawn.FireTimer = cls->GetPropertyDataOffset("FireTimer");
	PropOffsets_ScriptedPawn.FutzTimer = cls->GetPropertyDataOffset("FutzTimer");
	PropOffsets_ScriptedPawn.HarmAccuracy = cls->GetPropertyDataOffset("HarmAccuracy");
	PropOffsets_ScriptedPawn.HomeActor = cls->GetPropertyDataOffset("HomeActor");
	PropOffsets_ScriptedPawn.HomeExtent = cls->GetPropertyDataOffset("HomeExtent");
	PropOffsets_ScriptedPawn.HomeLoc = cls->GetPropertyDataOffset("HomeLoc");
	PropOffsets_ScriptedPawn.HomeRot = cls->GetPropertyDataOffset("HomeRot");
	PropOffsets_ScriptedPawn.HomeTag = cls->GetPropertyDataOffset("HomeTag");
	PropOffsets_ScriptedPawn.InitialAlliances = cls->GetPropertyDataOffset("InitialAlliances");
	PropOffsets_ScriptedPawn.InitialInventory = cls->GetPropertyDataOffset("InitialInventory");
	PropOffsets_ScriptedPawn.LastDestLoc = cls->GetPropertyDataOffset("LastDestLoc");
	PropOffsets_ScriptedPawn.LastDestPoint = cls->GetPropertyDataOffset("LastDestPoint");
	PropOffsets_ScriptedPawn.LastPainAnim = cls->GetPropertyDataOffset("LastPainAnim");
	PropOffsets_ScriptedPawn.LastPainTime = cls->GetPropertyDataOffset("LastPainTime");
	PropOffsets_ScriptedPawn.MaxProvocations = cls->GetPropertyDataOffset("MaxProvocations");
	PropOffsets_ScriptedPawn.MinHealth = cls->GetPropertyDataOffset("MinHealth");
	PropOffsets_ScriptedPawn.MinRange = cls->GetPropertyDataOffset("MinRange");
	PropOffsets_ScriptedPawn.NextAnim = cls->GetPropertyDataOffset("NextAnim");
	PropOffsets_ScriptedPawn.NextDirection = cls->GetPropertyDataOffset("NextDirection");
	PropOffsets_ScriptedPawn.NumCarcasses = cls->GetPropertyDataOffset("NumCarcasses");
	PropOffsets_ScriptedPawn.ObstacleTimer = cls->GetPropertyDataOffset("ObstacleTimer");
	PropOffsets_ScriptedPawn.OrderActor = cls->GetPropertyDataOffset("OrderActor");
	PropOffsets_ScriptedPawn.OrderTag = cls->GetPropertyDataOffset("OrderTag");
	PropOffsets_ScriptedPawn.Orders = cls->GetPropertyDataOffset("Orders");
	PropOffsets_ScriptedPawn.PlayerAgitationTimer = cls->GetPropertyDataOffset("PlayerAgitationTimer");
	PropOffsets_ScriptedPawn.PotentialEnemyAlliance = cls->GetPropertyDataOffset("PotentialEnemyAlliance");
	PropOffsets_ScriptedPawn.PotentialEnemyTimer = cls->GetPropertyDataOffset("PotentialEnemyTimer");
	PropOffsets_ScriptedPawn.PrePivotOffset = cls->GetPropertyDataOffset("PrePivotOffset");
	PropOffsets_ScriptedPawn.PrePivotTime = cls->GetPropertyDataOffset("PrePivotTime");
	PropOffsets_ScriptedPawn.ProjectileSpeed = cls->GetPropertyDataOffset("ProjectileSpeed");
	PropOffsets_ScriptedPawn.RaiseAlarm = cls->GetPropertyDataOffset("RaiseAlarm");
	PropOffsets_ScriptedPawn.RandomWandering = cls->GetPropertyDataOffset("RandomWandering");
	PropOffsets_ScriptedPawn.ReactionLevel = cls->GetPropertyDataOffset("ReactionLevel");
	PropOffsets_ScriptedPawn.ReloadTimer = cls->GetPropertyDataOffset("ReloadTimer");
	PropOffsets_ScriptedPawn.Restlessness = cls->GetPropertyDataOffset("Restlessness");
	PropOffsets_ScriptedPawn.SeatActor = cls->GetPropertyDataOffset("SeatActor");
	PropOffsets_ScriptedPawn.SeatHack = cls->GetPropertyDataOffset("SeatHack");
	PropOffsets_ScriptedPawn.SeatLocation = cls->GetPropertyDataOffset("SeatLocation");
	PropOffsets_ScriptedPawn.SeatSlot = cls->GetPropertyDataOffset("SeatSlot");
	PropOffsets_ScriptedPawn.SeekLevel = cls->GetPropertyDataOffset("SeekLevel");
	PropOffsets_ScriptedPawn.SeekPawn = cls->GetPropertyDataOffset("SeekPawn");
	PropOffsets_ScriptedPawn.SeekType = cls->GetPropertyDataOffset("SeekType");
	PropOffsets_ScriptedPawn.ShadowScale = cls->GetPropertyDataOffset("ShadowScale");
	PropOffsets_ScriptedPawn.SightPercentage = cls->GetPropertyDataOffset("SightPercentage");
	PropOffsets_ScriptedPawn.SpecialTimer = cls->GetPropertyDataOffset("SpecialTimer");
	PropOffsets_ScriptedPawn.SprintRate = cls->GetPropertyDataOffset("SprintRate");
	PropOffsets_ScriptedPawn.StandRate = cls->GetPropertyDataOffset("StandRate");
	PropOffsets_ScriptedPawn.SurprisePeriod = cls->GetPropertyDataOffset("SurprisePeriod");
	PropOffsets_ScriptedPawn.TakeHitTimer = cls->GetPropertyDataOffset("TakeHitTimer");
	PropOffsets_ScriptedPawn.TurnDirection = cls->GetPropertyDataOffset("TurnDirection");
	PropOffsets_ScriptedPawn.WalkSound = cls->GetPropertyDataOffset("WalkSound");
	PropOffsets_ScriptedPawn.WalkingSpeed = cls->GetPropertyDataOffset("WalkingSpeed");
	PropOffsets_ScriptedPawn.Wanderlust = cls->GetPropertyDataOffset("Wanderlust");
	PropOffsets_ScriptedPawn.WeaponTimer = cls->GetPropertyDataOffset("WeaponTimer");
	PropOffsets_ScriptedPawn.WorldPosition = cls->GetPropertyDataOffset("WorldPosition");
	PropOffsets_ScriptedPawn.bAcceptBump = cls->GetPropertyDataOffset("bAcceptBump");
	PropOffsets_ScriptedPawn.bAimForHead = cls->GetPropertyDataOffset("bAimForHead");
	PropOffsets_ScriptedPawn.bAlliancesChanged = cls->GetPropertyDataOffset("bAlliancesChanged");
	PropOffsets_ScriptedPawn.bAlwaysPatrol = cls->GetPropertyDataOffset("bAlwaysPatrol");
	PropOffsets_ScriptedPawn.bAttacking = cls->GetPropertyDataOffset("bAttacking");
	PropOffsets_ScriptedPawn.bAvoidAim = cls->GetPropertyDataOffset("bAvoidAim");
	PropOffsets_ScriptedPawn.bAvoidHarm = cls->GetPropertyDataOffset("bAvoidHarm");
	PropOffsets_ScriptedPawn.bBurnedToDeath = cls->GetPropertyDataOffset("bBurnedToDeath");
	PropOffsets_ScriptedPawn.bCanBleed = cls->GetPropertyDataOffset("bCanBleed");
	PropOffsets_ScriptedPawn.bCanConverse = cls->GetPropertyDataOffset("bCanConverse");
	PropOffsets_ScriptedPawn.bCanCrouch = cls->GetPropertyDataOffset("bCanCrouch");
	PropOffsets_ScriptedPawn.bCanFire = cls->GetPropertyDataOffset("bCanFire");
	PropOffsets_ScriptedPawn.bCanSit = cls->GetPropertyDataOffset("bCanSit");
	PropOffsets_ScriptedPawn.bCanTurnHead = cls->GetPropertyDataOffset("bCanTurnHead");
	PropOffsets_ScriptedPawn.bClearedObstacle = cls->GetPropertyDataOffset("bClearedObstacle");
	PropOffsets_ScriptedPawn.bCloakOn = cls->GetPropertyDataOffset("bCloakOn");
	PropOffsets_ScriptedPawn.bConvEndState = cls->GetPropertyDataOffset("bConvEndState");
	PropOffsets_ScriptedPawn.bConversationEndedNormally = cls->GetPropertyDataOffset("bConversationEndedNormally");
	PropOffsets_ScriptedPawn.bCower = cls->GetPropertyDataOffset("bCower");
	PropOffsets_ScriptedPawn.bCrouching = cls->GetPropertyDataOffset("bCrouching");
	PropOffsets_ScriptedPawn.bDancing = cls->GetPropertyDataOffset("bDancing");
	PropOffsets_ScriptedPawn.bDefendHome = cls->GetPropertyDataOffset("bDefendHome");
	PropOffsets_ScriptedPawn.bDisappear = cls->GetPropertyDataOffset("bDisappear");
	PropOffsets_ScriptedPawn.bDistressed = cls->GetPropertyDataOffset("bDistressed");
	PropOffsets_ScriptedPawn.bEmitDistress = cls->GetPropertyDataOffset("bEmitDistress");
	PropOffsets_ScriptedPawn.bEnableCheckDest = cls->GetPropertyDataOffset("bEnableCheckDest");
	PropOffsets_ScriptedPawn.bFacingTarget = cls->GetPropertyDataOffset("bFacingTarget");
	PropOffsets_ScriptedPawn.bFearAlarm = cls->GetPropertyDataOffset("bFearAlarm");
	PropOffsets_ScriptedPawn.bFearCarcass = cls->GetPropertyDataOffset("bFearCarcass");
	PropOffsets_ScriptedPawn.bFearDistress = cls->GetPropertyDataOffset("bFearDistress");
	PropOffsets_ScriptedPawn.bFearHacking = cls->GetPropertyDataOffset("bFearHacking");
	PropOffsets_ScriptedPawn.bFearIndirectInjury = cls->GetPropertyDataOffset("bFearIndirectInjury");
	PropOffsets_ScriptedPawn.bFearInjury = cls->GetPropertyDataOffset("bFearInjury");
	PropOffsets_ScriptedPawn.bFearProjectiles = cls->GetPropertyDataOffset("bFearProjectiles");
	PropOffsets_ScriptedPawn.bFearShot = cls->GetPropertyDataOffset("bFearShot");
	PropOffsets_ScriptedPawn.bFearWeapon = cls->GetPropertyDataOffset("bFearWeapon");
	PropOffsets_ScriptedPawn.bHasCloak = cls->GetPropertyDataOffset("bHasCloak");
	PropOffsets_ScriptedPawn.bHasShadow = cls->GetPropertyDataOffset("bHasShadow");
	PropOffsets_ScriptedPawn.bHateCarcass = cls->GetPropertyDataOffset("bHateCarcass");
	PropOffsets_ScriptedPawn.bHateDistress = cls->GetPropertyDataOffset("bHateDistress");
	PropOffsets_ScriptedPawn.bHateHacking = cls->GetPropertyDataOffset("bHateHacking");
	PropOffsets_ScriptedPawn.bHateIndirectInjury = cls->GetPropertyDataOffset("bHateIndirectInjury");
	PropOffsets_ScriptedPawn.bHateInjury = cls->GetPropertyDataOffset("bHateInjury");
	PropOffsets_ScriptedPawn.bHateShot = cls->GetPropertyDataOffset("bHateShot");
	PropOffsets_ScriptedPawn.bHateWeapon = cls->GetPropertyDataOffset("bHateWeapon");
	PropOffsets_ScriptedPawn.bHighlight = cls->GetPropertyDataOffset("bHighlight");
	PropOffsets_ScriptedPawn.bHokeyPokey = cls->GetPropertyDataOffset("bHokeyPokey");
	PropOffsets_ScriptedPawn.bImportant = cls->GetPropertyDataOffset("bImportant");
	PropOffsets_ScriptedPawn.bInConversation = cls->GetPropertyDataOffset("bInConversation");
	PropOffsets_ScriptedPawn.bInTransientState = cls->GetPropertyDataOffset("bInTransientState");
	PropOffsets_ScriptedPawn.bInWorld = cls->GetPropertyDataOffset("bInWorld");
	PropOffsets_ScriptedPawn.bInitialized = cls->GetPropertyDataOffset("bInitialized");
	PropOffsets_ScriptedPawn.bInterruptSeek = cls->GetPropertyDataOffset("bInterruptSeek");
	PropOffsets_ScriptedPawn.bInterruptState = cls->GetPropertyDataOffset("bInterruptState");
	PropOffsets_ScriptedPawn.bInvincible = cls->GetPropertyDataOffset("bInvincible");
	PropOffsets_ScriptedPawn.bKeepWeaponDrawn = cls->GetPropertyDataOffset("bKeepWeaponDrawn");
	PropOffsets_ScriptedPawn.bLeaveAfterFleeing = cls->GetPropertyDataOffset("bLeaveAfterFleeing");
	PropOffsets_ScriptedPawn.bLikesNeutral = cls->GetPropertyDataOffset("bLikesNeutral");
	PropOffsets_ScriptedPawn.bLookingForAlarm = cls->GetPropertyDataOffset("bLookingForAlarm");
	PropOffsets_ScriptedPawn.bLookingForCarcass = cls->GetPropertyDataOffset("bLookingForCarcass");
	PropOffsets_ScriptedPawn.bLookingForDistress = cls->GetPropertyDataOffset("bLookingForDistress");
	PropOffsets_ScriptedPawn.bLookingForEnemy = cls->GetPropertyDataOffset("bLookingForEnemy");
	PropOffsets_ScriptedPawn.bLookingForFutz = cls->GetPropertyDataOffset("bLookingForFutz");
	PropOffsets_ScriptedPawn.bLookingForHacking = cls->GetPropertyDataOffset("bLookingForHacking");
	PropOffsets_ScriptedPawn.bLookingForIndirectInjury = cls->GetPropertyDataOffset("bLookingForIndirectInjury");
	PropOffsets_ScriptedPawn.bLookingForInjury = cls->GetPropertyDataOffset("bLookingForInjury");
	PropOffsets_ScriptedPawn.bLookingForLoudNoise = cls->GetPropertyDataOffset("bLookingForLoudNoise");
	PropOffsets_ScriptedPawn.bLookingForProjectiles = cls->GetPropertyDataOffset("bLookingForProjectiles");
	PropOffsets_ScriptedPawn.bLookingForShot = cls->GetPropertyDataOffset("bLookingForShot");
	PropOffsets_ScriptedPawn.bLookingForWeapon = cls->GetPropertyDataOffset("bLookingForWeapon");
	PropOffsets_ScriptedPawn.bMustFaceTarget = cls->GetPropertyDataOffset("bMustFaceTarget");
	PropOffsets_ScriptedPawn.bNoNegativeAlliances = cls->GetPropertyDataOffset("bNoNegativeAlliances");
	PropOffsets_ScriptedPawn.bPausing = cls->GetPropertyDataOffset("bPausing");
	PropOffsets_ScriptedPawn.bPlayIdle = cls->GetPropertyDataOffset("bPlayIdle");
	PropOffsets_ScriptedPawn.bReactAlarm = cls->GetPropertyDataOffset("bReactAlarm");
	PropOffsets_ScriptedPawn.bReactCarcass = cls->GetPropertyDataOffset("bReactCarcass");
	PropOffsets_ScriptedPawn.bReactDistress = cls->GetPropertyDataOffset("bReactDistress");
	PropOffsets_ScriptedPawn.bReactFutz = cls->GetPropertyDataOffset("bReactFutz");
	PropOffsets_ScriptedPawn.bReactLoudNoise = cls->GetPropertyDataOffset("bReactLoudNoise");
	PropOffsets_ScriptedPawn.bReactPresence = cls->GetPropertyDataOffset("bReactPresence");
	PropOffsets_ScriptedPawn.bReactProjectiles = cls->GetPropertyDataOffset("bReactProjectiles");
	PropOffsets_ScriptedPawn.bReactShot = cls->GetPropertyDataOffset("bReactShot");
	PropOffsets_ScriptedPawn.bReadyToReload = cls->GetPropertyDataOffset("bReadyToReload");
	PropOffsets_ScriptedPawn.bReverseAlliances = cls->GetPropertyDataOffset("bReverseAlliances");
	PropOffsets_ScriptedPawn.bRunningStealthy = cls->GetPropertyDataOffset("bRunningStealthy");
	PropOffsets_ScriptedPawn.bSeatHackUsed = cls->GetPropertyDataOffset("bSeatHackUsed");
	PropOffsets_ScriptedPawn.bSeatLocationValid = cls->GetPropertyDataOffset("bSeatLocationValid");
	PropOffsets_ScriptedPawn.bSeekCover = cls->GetPropertyDataOffset("bSeekCover");
	PropOffsets_ScriptedPawn.bSeekLocation = cls->GetPropertyDataOffset("bSeekLocation");
	PropOffsets_ScriptedPawn.bSeekPostCombat = cls->GetPropertyDataOffset("bSeekPostCombat");
	PropOffsets_ScriptedPawn.bShowPain = cls->GetPropertyDataOffset("bShowPain");
	PropOffsets_ScriptedPawn.bSitAnywhere = cls->GetPropertyDataOffset("bSitAnywhere");
	PropOffsets_ScriptedPawn.bSitInterpolation = cls->GetPropertyDataOffset("bSitInterpolation");
	PropOffsets_ScriptedPawn.bSitting = cls->GetPropertyDataOffset("bSitting");
	PropOffsets_ScriptedPawn.bSpawnBubbles = cls->GetPropertyDataOffset("bSpawnBubbles");
	PropOffsets_ScriptedPawn.bSprint = cls->GetPropertyDataOffset("bSprint");
	PropOffsets_ScriptedPawn.bStandInterpolation = cls->GetPropertyDataOffset("bStandInterpolation");
	PropOffsets_ScriptedPawn.bStaring = cls->GetPropertyDataOffset("bStaring");
	PropOffsets_ScriptedPawn.bStunned = cls->GetPropertyDataOffset("bStunned");
	PropOffsets_ScriptedPawn.bTickVisibleOnly = cls->GetPropertyDataOffset("bTickVisibleOnly");
	PropOffsets_ScriptedPawn.bUseFallbackWeapons = cls->GetPropertyDataOffset("bUseFallbackWeapons");
	PropOffsets_ScriptedPawn.bUseFirstSeatOnly = cls->GetPropertyDataOffset("bUseFirstSeatOnly");
	PropOffsets_ScriptedPawn.bUseHome = cls->GetPropertyDataOffset("bUseHome");
	PropOffsets_ScriptedPawn.bUseSecondaryAttack = cls->GetPropertyDataOffset("bUseSecondaryAttack");
	PropOffsets_ScriptedPawn.bWalkAround = cls->GetPropertyDataOffset("bWalkAround");
	PropOffsets_ScriptedPawn.bWorldBlockActors = cls->GetPropertyDataOffset("bWorldBlockActors");
	PropOffsets_ScriptedPawn.bWorldBlockPlayers = cls->GetPropertyDataOffset("bWorldBlockPlayers");
	PropOffsets_ScriptedPawn.bWorldCollideActors = cls->GetPropertyDataOffset("bWorldCollideActors");
	PropOffsets_ScriptedPawn.cycleIndex = cls->GetPropertyDataOffset("cycleIndex");
	PropOffsets_ScriptedPawn.destLoc = cls->GetPropertyDataOffset("destLoc");
	PropOffsets_ScriptedPawn.destPoint = cls->GetPropertyDataOffset("destPoint");
	PropOffsets_ScriptedPawn.lastPoints = cls->GetPropertyDataOffset("lastPoints");
	PropOffsets_ScriptedPawn.maxRange = cls->GetPropertyDataOffset("maxRange");
	PropOffsets_ScriptedPawn.poisonCounter = cls->GetPropertyDataOffset("poisonCounter");
	PropOffsets_ScriptedPawn.poisonDamage = cls->GetPropertyDataOffset("poisonDamage");
	PropOffsets_ScriptedPawn.poisonTimer = cls->GetPropertyDataOffset("poisonTimer");
	PropOffsets_ScriptedPawn.poisoner = cls->GetPropertyDataOffset("poisoner");
	PropOffsets_ScriptedPawn.remainingSitTime = cls->GetPropertyDataOffset("remainingSitTime");
	PropOffsets_ScriptedPawn.remainingStandTime = cls->GetPropertyDataOffset("remainingStandTime");
	PropOffsets_ScriptedPawn.runAnimMult = cls->GetPropertyDataOffset("runAnimMult");
	PropOffsets_ScriptedPawn.seekDistance = cls->GetPropertyDataOffset("seekDistance");
	PropOffsets_ScriptedPawn.sleepTime = cls->GetPropertyDataOffset("sleepTime");
	PropOffsets_ScriptedPawn.swimBubbleTimer = cls->GetPropertyDataOffset("swimBubbleTimer");
	PropOffsets_ScriptedPawn.useLoc = cls->GetPropertyDataOffset("useLoc");
	PropOffsets_ScriptedPawn.useRot = cls->GetPropertyDataOffset("useRot");
	PropOffsets_ScriptedPawn.walkAnimMult = cls->GetPropertyDataOffset("walkAnimMult");
}

PropertyOffsets_DeusExTextParser PropOffsets_DeusExTextParser;

static void InitPropertyOffsets_DeusExTextParser(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusExText")->GetUObject("Class", "DeusExTextParser"));
	if (!cls)
	{
		memset(&PropOffsets_DeusExTextParser, 0xff, sizeof(PropOffsets_DeusExTextParser));
		return;
	}
	PropOffsets_DeusExTextParser.DefaultColor = cls->GetPropertyDataOffset("DefaultColor");
	PropOffsets_DeusExTextParser.PlayerName = cls->GetPropertyDataOffset("PlayerName");
	PropOffsets_DeusExTextParser.Text = cls->GetPropertyDataOffset("Text");
	PropOffsets_DeusExTextParser.bParagraphStarted = cls->GetPropertyDataOffset("bParagraphStarted");
	PropOffsets_DeusExTextParser.lastColor = cls->GetPropertyDataOffset("lastColor");
	PropOffsets_DeusExTextParser.lastEmailCC = cls->GetPropertyDataOffset("lastEmailCC");
	PropOffsets_DeusExTextParser.lastEmailFrom = cls->GetPropertyDataOffset("lastEmailFrom");
	PropOffsets_DeusExTextParser.lastEmailName = cls->GetPropertyDataOffset("lastEmailName");
	PropOffsets_DeusExTextParser.lastEmailSubject = cls->GetPropertyDataOffset("lastEmailSubject");
	PropOffsets_DeusExTextParser.lastEmailTo = cls->GetPropertyDataOffset("lastEmailTo");
	PropOffsets_DeusExTextParser.lastFileDescription = cls->GetPropertyDataOffset("lastFileDescription");
	PropOffsets_DeusExTextParser.lastFileName = cls->GetPropertyDataOffset("lastFileName");
	PropOffsets_DeusExTextParser.lastName = cls->GetPropertyDataOffset("lastName");
	PropOffsets_DeusExTextParser.lastTag = cls->GetPropertyDataOffset("lastTag");
	PropOffsets_DeusExTextParser.lastText = cls->GetPropertyDataOffset("lastText");
	PropOffsets_DeusExTextParser.playerFirstName = cls->GetPropertyDataOffset("playerFirstName");
	PropOffsets_DeusExTextParser.tagEndPos = cls->GetPropertyDataOffset("tagEndPos");
	PropOffsets_DeusExTextParser.textPos = cls->GetPropertyDataOffset("textPos");
}

PropertyOffsets_BorderWindow PropOffsets_BorderWindow;

static void InitPropertyOffsets_BorderWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "BorderWindow"));
	if (!cls)
	{
		memset(&PropOffsets_BorderWindow, 0xff, sizeof(PropOffsets_BorderWindow));
		return;
	}
	PropOffsets_BorderWindow.BottomMargin = cls->GetPropertyDataOffset("BottomMargin");
	PropOffsets_BorderWindow.DragX = cls->GetPropertyDataOffset("DragX");
	PropOffsets_BorderWindow.DragY = cls->GetPropertyDataOffset("DragY");
	PropOffsets_BorderWindow.MoveCursor = cls->GetPropertyDataOffset("MoveCursor");
	PropOffsets_BorderWindow.TopMargin = cls->GetPropertyDataOffset("TopMargin");
	PropOffsets_BorderWindow.bDownDrag = cls->GetPropertyDataOffset("bDownDrag");
	PropOffsets_BorderWindow.bLeftDrag = cls->GetPropertyDataOffset("bLeftDrag");
	PropOffsets_BorderWindow.bMarginsFromBorder = cls->GetPropertyDataOffset("bMarginsFromBorder");
	PropOffsets_BorderWindow.bResizeable = cls->GetPropertyDataOffset("bResizeable");
	PropOffsets_BorderWindow.bRightDrag = cls->GetPropertyDataOffset("bRightDrag");
	PropOffsets_BorderWindow.bSmoothBorder = cls->GetPropertyDataOffset("bSmoothBorder");
	PropOffsets_BorderWindow.bStretchBorder = cls->GetPropertyDataOffset("bStretchBorder");
	PropOffsets_BorderWindow.bUpDrag = cls->GetPropertyDataOffset("bUpDrag");
	PropOffsets_BorderWindow.borderBottom = cls->GetPropertyDataOffset("borderBottom");
	PropOffsets_BorderWindow.borderBottomLeft = cls->GetPropertyDataOffset("borderBottomLeft");
	PropOffsets_BorderWindow.borderBottomRight = cls->GetPropertyDataOffset("borderBottomRight");
	PropOffsets_BorderWindow.borderLeft = cls->GetPropertyDataOffset("borderLeft");
	PropOffsets_BorderWindow.borderRight = cls->GetPropertyDataOffset("borderRight");
	PropOffsets_BorderWindow.borderStyle = cls->GetPropertyDataOffset("borderStyle");
	PropOffsets_BorderWindow.borderTop = cls->GetPropertyDataOffset("borderTop");
	PropOffsets_BorderWindow.borderTopLeft = cls->GetPropertyDataOffset("borderTopLeft");
	PropOffsets_BorderWindow.borderTopRight = cls->GetPropertyDataOffset("borderTopRight");
	PropOffsets_BorderWindow.center = cls->GetPropertyDataOffset("center");
	PropOffsets_BorderWindow.childBottomMargin = cls->GetPropertyDataOffset("childBottomMargin");
	PropOffsets_BorderWindow.childLeftMargin = cls->GetPropertyDataOffset("childLeftMargin");
	PropOffsets_BorderWindow.childRightMargin = cls->GetPropertyDataOffset("childRightMargin");
	PropOffsets_BorderWindow.childTopMargin = cls->GetPropertyDataOffset("childTopMargin");
	PropOffsets_BorderWindow.dragHeight = cls->GetPropertyDataOffset("dragHeight");
	PropOffsets_BorderWindow.dragWidth = cls->GetPropertyDataOffset("dragWidth");
	PropOffsets_BorderWindow.hMoveCursor = cls->GetPropertyDataOffset("hMoveCursor");
	PropOffsets_BorderWindow.lastMouseX = cls->GetPropertyDataOffset("lastMouseX");
	PropOffsets_BorderWindow.lastMouseY = cls->GetPropertyDataOffset("lastMouseY");
	PropOffsets_BorderWindow.leftMargin = cls->GetPropertyDataOffset("leftMargin");
	PropOffsets_BorderWindow.rightMargin = cls->GetPropertyDataOffset("rightMargin");
	PropOffsets_BorderWindow.tlMoveCursor = cls->GetPropertyDataOffset("tlMoveCursor");
	PropOffsets_BorderWindow.trMoveCursor = cls->GetPropertyDataOffset("trMoveCursor");
	PropOffsets_BorderWindow.vMoveCursor = cls->GetPropertyDataOffset("vMoveCursor");
}

PropertyOffsets_ButtonWindow PropOffsets_ButtonWindow;

static void InitPropertyOffsets_ButtonWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ButtonWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ButtonWindow, 0xff, sizeof(PropOffsets_ButtonWindow));
		return;
	}
	PropOffsets_ButtonWindow.Info = cls->GetPropertyDataOffset("Info");
	PropOffsets_ButtonWindow.activateDelay = cls->GetPropertyDataOffset("activateDelay");
	PropOffsets_ButtonWindow.activateTimer = cls->GetPropertyDataOffset("activateTimer");
	PropOffsets_ButtonWindow.bAutoRepeat = cls->GetPropertyDataOffset("bAutoRepeat");
	PropOffsets_ButtonWindow.bButtonPressed = cls->GetPropertyDataOffset("bButtonPressed");
	PropOffsets_ButtonWindow.bEnableRightMouseClick = cls->GetPropertyDataOffset("bEnableRightMouseClick");
	PropOffsets_ButtonWindow.bMousePressed = cls->GetPropertyDataOffset("bMousePressed");
	PropOffsets_ButtonWindow.clickSound = cls->GetPropertyDataOffset("clickSound");
	PropOffsets_ButtonWindow.curTextColor = cls->GetPropertyDataOffset("curTextColor");
	PropOffsets_ButtonWindow.curTexture = cls->GetPropertyDataOffset("curTexture");
	PropOffsets_ButtonWindow.curTileColor = cls->GetPropertyDataOffset("curTileColor");
	PropOffsets_ButtonWindow.initialDelay = cls->GetPropertyDataOffset("initialDelay");
	PropOffsets_ButtonWindow.lastInputKey = cls->GetPropertyDataOffset("lastInputKey");
	PropOffsets_ButtonWindow.pressSound = cls->GetPropertyDataOffset("pressSound");
	PropOffsets_ButtonWindow.repeatRate = cls->GetPropertyDataOffset("repeatRate");
	PropOffsets_ButtonWindow.repeatTime = cls->GetPropertyDataOffset("repeatTime");
}

PropertyOffsets_CheckboxWindow PropOffsets_CheckboxWindow;

static void InitPropertyOffsets_CheckboxWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "CheckboxWindow"));
	if (!cls)
	{
		memset(&PropOffsets_CheckboxWindow, 0xff, sizeof(PropOffsets_CheckboxWindow));
		return;
	}
	PropOffsets_CheckboxWindow.bRightSide = cls->GetPropertyDataOffset("bRightSide");
	PropOffsets_CheckboxWindow.checkboxColor = cls->GetPropertyDataOffset("checkboxColor");
	PropOffsets_CheckboxWindow.checkboxSpacing = cls->GetPropertyDataOffset("checkboxSpacing");
	PropOffsets_CheckboxWindow.checkboxStyle = cls->GetPropertyDataOffset("checkboxStyle");
	PropOffsets_CheckboxWindow.textureHeight = cls->GetPropertyDataOffset("textureHeight");
	PropOffsets_CheckboxWindow.textureWidth = cls->GetPropertyDataOffset("textureWidth");
	PropOffsets_CheckboxWindow.toggleOff = cls->GetPropertyDataOffset("toggleOff");
	PropOffsets_CheckboxWindow.toggleOn = cls->GetPropertyDataOffset("toggleOn");
}

PropertyOffsets_ClipWindow PropOffsets_ClipWindow;

static void InitPropertyOffsets_ClipWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ClipWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ClipWindow, 0xff, sizeof(PropOffsets_ClipWindow));
		return;
	}
	PropOffsets_ClipWindow.areaHSize = cls->GetPropertyDataOffset("areaHSize");
	PropOffsets_ClipWindow.areaVSize = cls->GetPropertyDataOffset("areaVSize");
	PropOffsets_ClipWindow.bFillWindow = cls->GetPropertyDataOffset("bFillWindow");
	PropOffsets_ClipWindow.bForceChildHeight = cls->GetPropertyDataOffset("bForceChildHeight");
	PropOffsets_ClipWindow.bForceChildWidth = cls->GetPropertyDataOffset("bForceChildWidth");
	PropOffsets_ClipWindow.bSnapToUnits = cls->GetPropertyDataOffset("bSnapToUnits");
	PropOffsets_ClipWindow.childH = cls->GetPropertyDataOffset("childH");
	PropOffsets_ClipWindow.childHSize = cls->GetPropertyDataOffset("childHSize");
	PropOffsets_ClipWindow.childV = cls->GetPropertyDataOffset("childV");
	PropOffsets_ClipWindow.childVSize = cls->GetPropertyDataOffset("childVSize");
	PropOffsets_ClipWindow.hMult = cls->GetPropertyDataOffset("hMult");
	PropOffsets_ClipWindow.prefHUnits = cls->GetPropertyDataOffset("prefHUnits");
	PropOffsets_ClipWindow.prefVUnits = cls->GetPropertyDataOffset("prefVUnits");
	PropOffsets_ClipWindow.vMult = cls->GetPropertyDataOffset("vMult");
}

PropertyOffsets_ComputerWindow PropOffsets_ComputerWindow;

static void InitPropertyOffsets_ComputerWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ComputerWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ComputerWindow, 0xff, sizeof(PropOffsets_ComputerWindow));
		return;
	}
	PropOffsets_ComputerWindow.FontColor = cls->GetPropertyDataOffset("FontColor");
	PropOffsets_ComputerWindow.Player = cls->GetPropertyDataOffset("Player");
	PropOffsets_ComputerWindow.TextWindow = cls->GetPropertyDataOffset("TextWindow");
	PropOffsets_ComputerWindow.TextX = cls->GetPropertyDataOffset("TextX");
	PropOffsets_ComputerWindow.TextY = cls->GetPropertyDataOffset("TextY");
	PropOffsets_ComputerWindow.bComputerStartInvoked = cls->GetPropertyDataOffset("bComputerStartInvoked");
	PropOffsets_ComputerWindow.bCursorVisible = cls->GetPropertyDataOffset("bCursorVisible");
	PropOffsets_ComputerWindow.bEchoKey = cls->GetPropertyDataOffset("bEchoKey");
	PropOffsets_ComputerWindow.bFirstTick = cls->GetPropertyDataOffset("bFirstTick");
	PropOffsets_ComputerWindow.bGamePaused = cls->GetPropertyDataOffset("bGamePaused");
	PropOffsets_ComputerWindow.bIgnoreGamePaused = cls->GetPropertyDataOffset("bIgnoreGamePaused");
	PropOffsets_ComputerWindow.bIgnoreTick = cls->GetPropertyDataOffset("bIgnoreTick");
	PropOffsets_ComputerWindow.bInvokeComputerStart = cls->GetPropertyDataOffset("bInvokeComputerStart");
	PropOffsets_ComputerWindow.bLastLineWrapped = cls->GetPropertyDataOffset("bLastLineWrapped");
	PropOffsets_ComputerWindow.bPauseProcessing = cls->GetPropertyDataOffset("bPauseProcessing");
	PropOffsets_ComputerWindow.bShowCursor = cls->GetPropertyDataOffset("bShowCursor");
	PropOffsets_ComputerWindow.bWaitingForKey = cls->GetPropertyDataOffset("bWaitingForKey");
	PropOffsets_ComputerWindow.bWordWrap = cls->GetPropertyDataOffset("bWordWrap");
	PropOffsets_ComputerWindow.backgroundTextures = cls->GetPropertyDataOffset("backgroundTextures");
	PropOffsets_ComputerWindow.colGraphicTile = cls->GetPropertyDataOffset("colGraphicTile");
	PropOffsets_ComputerWindow.computerSoundVolume = cls->GetPropertyDataOffset("computerSoundVolume");
	PropOffsets_ComputerWindow.cursorBlinkSpeed = cls->GetPropertyDataOffset("cursorBlinkSpeed");
	PropOffsets_ComputerWindow.cursorColor = cls->GetPropertyDataOffset("cursorColor");
	PropOffsets_ComputerWindow.cursorHeight = cls->GetPropertyDataOffset("cursorHeight");
	PropOffsets_ComputerWindow.cursorNextEvent = cls->GetPropertyDataOffset("cursorNextEvent");
	PropOffsets_ComputerWindow.cursorTexture = cls->GetPropertyDataOffset("cursorTexture");
	PropOffsets_ComputerWindow.cursorWidth = cls->GetPropertyDataOffset("cursorWidth");
	PropOffsets_ComputerWindow.displayBuffer = cls->GetPropertyDataOffset("displayBuffer");
	PropOffsets_ComputerWindow.eventTimeInterval = cls->GetPropertyDataOffset("eventTimeInterval");
	PropOffsets_ComputerWindow.fadeOutStart = cls->GetPropertyDataOffset("fadeOutStart");
	PropOffsets_ComputerWindow.fadeOutTimer = cls->GetPropertyDataOffset("fadeOutTimer");
	PropOffsets_ComputerWindow.fadeSpeed = cls->GetPropertyDataOffset("fadeSpeed");
	PropOffsets_ComputerWindow.fontHeight = cls->GetPropertyDataOffset("fontHeight");
	PropOffsets_ComputerWindow.fontWidth = cls->GetPropertyDataOffset("fontWidth");
	PropOffsets_ComputerWindow.inputKey = cls->GetPropertyDataOffset("inputKey");
	PropOffsets_ComputerWindow.inputMask = cls->GetPropertyDataOffset("inputMask");
	PropOffsets_ComputerWindow.inputWindow = cls->GetPropertyDataOffset("inputWindow");
	PropOffsets_ComputerWindow.queuedBuffer = cls->GetPropertyDataOffset("queuedBuffer");
	PropOffsets_ComputerWindow.queuedBufferStart = cls->GetPropertyDataOffset("queuedBufferStart");
	PropOffsets_ComputerWindow.textCols = cls->GetPropertyDataOffset("textCols");
	PropOffsets_ComputerWindow.textFont = cls->GetPropertyDataOffset("textFont");
	PropOffsets_ComputerWindow.textRows = cls->GetPropertyDataOffset("textRows");
	PropOffsets_ComputerWindow.textSound = cls->GetPropertyDataOffset("textSound");
	PropOffsets_ComputerWindow.throttle = cls->GetPropertyDataOffset("throttle");
	PropOffsets_ComputerWindow.timeCurrent = cls->GetPropertyDataOffset("timeCurrent");
	PropOffsets_ComputerWindow.timeLastEvent = cls->GetPropertyDataOffset("timeLastEvent");
	PropOffsets_ComputerWindow.timeNextEvent = cls->GetPropertyDataOffset("timeNextEvent");
	PropOffsets_ComputerWindow.typingSound = cls->GetPropertyDataOffset("typingSound");
}

PropertyOffsets_EditWindow PropOffsets_EditWindow;

static void InitPropertyOffsets_EditWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "EditWindow"));
	if (!cls)
	{
		memset(&PropOffsets_EditWindow, 0xff, sizeof(PropOffsets_EditWindow));
		return;
	}
	PropOffsets_EditWindow.bCursorShowing = cls->GetPropertyDataOffset("bCursorShowing");
	PropOffsets_EditWindow.bDragging = cls->GetPropertyDataOffset("bDragging");
	PropOffsets_EditWindow.bEditable = cls->GetPropertyDataOffset("bEditable");
	PropOffsets_EditWindow.bSelectWords = cls->GetPropertyDataOffset("bSelectWords");
	PropOffsets_EditWindow.bSingleLine = cls->GetPropertyDataOffset("bSingleLine");
	PropOffsets_EditWindow.bUppercaseOnly = cls->GetPropertyDataOffset("bUppercaseOnly");
	PropOffsets_EditWindow.blinkDelay = cls->GetPropertyDataOffset("blinkDelay");
	PropOffsets_EditWindow.blinkPeriod = cls->GetPropertyDataOffset("blinkPeriod");
	PropOffsets_EditWindow.blinkStart = cls->GetPropertyDataOffset("blinkStart");
	PropOffsets_EditWindow.bufferList = cls->GetPropertyDataOffset("bufferList");
	PropOffsets_EditWindow.currentUndo = cls->GetPropertyDataOffset("currentUndo");
	PropOffsets_EditWindow.deleteSound = cls->GetPropertyDataOffset("deleteSound");
	PropOffsets_EditWindow.dragDelay = cls->GetPropertyDataOffset("dragDelay");
	PropOffsets_EditWindow.editCursor = cls->GetPropertyDataOffset("editCursor");
	PropOffsets_EditWindow.editCursorColor = cls->GetPropertyDataOffset("editCursorColor");
	PropOffsets_EditWindow.editCursorShadow = cls->GetPropertyDataOffset("editCursorShadow");
	PropOffsets_EditWindow.enterSound = cls->GetPropertyDataOffset("enterSound");
	PropOffsets_EditWindow.insertColor = cls->GetPropertyDataOffset("insertColor");
	PropOffsets_EditWindow.insertHeight = cls->GetPropertyDataOffset("insertHeight");
	PropOffsets_EditWindow.insertHookPos = cls->GetPropertyDataOffset("insertHookPos");
	PropOffsets_EditWindow.insertPos = cls->GetPropertyDataOffset("insertPos");
	PropOffsets_EditWindow.insertPrefHeight = cls->GetPropertyDataOffset("insertPrefHeight");
	PropOffsets_EditWindow.insertPrefWidth = cls->GetPropertyDataOffset("insertPrefWidth");
	PropOffsets_EditWindow.insertPreferredCol = cls->GetPropertyDataOffset("insertPreferredCol");
	PropOffsets_EditWindow.insertTexture = cls->GetPropertyDataOffset("insertTexture");
	PropOffsets_EditWindow.insertType = cls->GetPropertyDataOffset("insertType");
	PropOffsets_EditWindow.insertWidth = cls->GetPropertyDataOffset("insertWidth");
	PropOffsets_EditWindow.insertX = cls->GetPropertyDataOffset("insertX");
	PropOffsets_EditWindow.insertY = cls->GetPropertyDataOffset("insertY");
	PropOffsets_EditWindow.inverseColor = cls->GetPropertyDataOffset("inverseColor");
	PropOffsets_EditWindow.lastConfigHeight = cls->GetPropertyDataOffset("lastConfigHeight");
	PropOffsets_EditWindow.lastConfigWidth = cls->GetPropertyDataOffset("lastConfigWidth");
	PropOffsets_EditWindow.maxSize = cls->GetPropertyDataOffset("maxSize");
	PropOffsets_EditWindow.maxUndos = cls->GetPropertyDataOffset("maxUndos");
	PropOffsets_EditWindow.moveSound = cls->GetPropertyDataOffset("moveSound");
	PropOffsets_EditWindow.selectColor = cls->GetPropertyDataOffset("selectColor");
	PropOffsets_EditWindow.selectEnd = cls->GetPropertyDataOffset("selectEnd");
	PropOffsets_EditWindow.selectEndRow = cls->GetPropertyDataOffset("selectEndRow");
	PropOffsets_EditWindow.selectEndX = cls->GetPropertyDataOffset("selectEndX");
	PropOffsets_EditWindow.selectStart = cls->GetPropertyDataOffset("selectStart");
	PropOffsets_EditWindow.selectStartRow = cls->GetPropertyDataOffset("selectStartRow");
	PropOffsets_EditWindow.selectStartX = cls->GetPropertyDataOffset("selectStartX");
	PropOffsets_EditWindow.selectTexture = cls->GetPropertyDataOffset("selectTexture");
	PropOffsets_EditWindow.showAreaHeight = cls->GetPropertyDataOffset("showAreaHeight");
	PropOffsets_EditWindow.showAreaWidth = cls->GetPropertyDataOffset("showAreaWidth");
	PropOffsets_EditWindow.showAreaX = cls->GetPropertyDataOffset("showAreaX");
	PropOffsets_EditWindow.showAreaY = cls->GetPropertyDataOffset("showAreaY");
	PropOffsets_EditWindow.typeSound = cls->GetPropertyDataOffset("typeSound");
	PropOffsets_EditWindow.unchangedUndo = cls->GetPropertyDataOffset("unchangedUndo");
}

PropertyOffsets_ExtString PropOffsets_ExtString;

static void InitPropertyOffsets_ExtString(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ExtString"));
	if (!cls)
	{
		memset(&PropOffsets_ExtString, 0xff, sizeof(PropOffsets_ExtString));
		return;
	}
	PropOffsets_ExtString.Text = cls->GetPropertyDataOffset("Text");
	PropOffsets_ExtString.speechPage = cls->GetPropertyDataOffset("speechPage");
}

PropertyOffsets_ExtensionObject PropOffsets_ExtensionObject;

static void InitPropertyOffsets_ExtensionObject(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ExtensionObject"));
	if (!cls)
	{
		memset(&PropOffsets_ExtensionObject, 0xff, sizeof(PropOffsets_ExtensionObject));
		return;
	}
}

PropertyOffsets_FlagBase PropOffsets_FlagBase;

static void InitPropertyOffsets_FlagBase(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "FlagBase"));
	if (!cls)
	{
		memset(&PropOffsets_FlagBase, 0xff, sizeof(PropOffsets_FlagBase));
		return;
	}
	PropOffsets_FlagBase.defaultFlagExpiration = cls->GetPropertyDataOffset("defaultFlagExpiration");
	PropOffsets_FlagBase.hashTable = cls->GetPropertyDataOffset("hashTable");
}

PropertyOffsets_GC PropOffsets_GC;

static void InitPropertyOffsets_GC(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "GC"));
	if (!cls)
	{
		memset(&PropOffsets_GC, 0xff, sizeof(PropOffsets_GC));
		return;
	}
	PropOffsets_GC.Canvas = cls->GetPropertyDataOffset("Canvas");
	PropOffsets_GC.HAlign = cls->GetPropertyDataOffset("HAlign");
	PropOffsets_GC.PolyFlags = cls->GetPropertyDataOffset("PolyFlags");
	PropOffsets_GC.Style = cls->GetPropertyDataOffset("Style");
	PropOffsets_GC.TextColor = cls->GetPropertyDataOffset("TextColor");
	PropOffsets_GC.VAlign = cls->GetPropertyDataOffset("VAlign");
	PropOffsets_GC.bDrawEnabled = cls->GetPropertyDataOffset("bDrawEnabled");
	PropOffsets_GC.bFree = cls->GetPropertyDataOffset("bFree");
	PropOffsets_GC.bMasked = cls->GetPropertyDataOffset("bMasked");
	PropOffsets_GC.bModulated = cls->GetPropertyDataOffset("bModulated");
	PropOffsets_GC.bParseMetachars = cls->GetPropertyDataOffset("bParseMetachars");
	PropOffsets_GC.bSmoothed = cls->GetPropertyDataOffset("bSmoothed");
	PropOffsets_GC.bTextTranslucent = cls->GetPropertyDataOffset("bTextTranslucent");
	PropOffsets_GC.bTranslucent = cls->GetPropertyDataOffset("bTranslucent");
	PropOffsets_GC.bWordWrap = cls->GetPropertyDataOffset("bWordWrap");
	PropOffsets_GC.baselineOffset = cls->GetPropertyDataOffset("baselineOffset");
	PropOffsets_GC.boldFont = cls->GetPropertyDataOffset("boldFont");
	PropOffsets_GC.gcClipRect = cls->GetPropertyDataOffset("gcClipRect");
	PropOffsets_GC.gcCount = cls->GetPropertyDataOffset("gcCount");
	PropOffsets_GC.gcFree = cls->GetPropertyDataOffset("gcFree");
	PropOffsets_GC.gcOwner = cls->GetPropertyDataOffset("gcOwner");
	PropOffsets_GC.gcStack = cls->GetPropertyDataOffset("gcStack");
	PropOffsets_GC.hMultiplier = cls->GetPropertyDataOffset("hMultiplier");
	PropOffsets_GC.normalFont = cls->GetPropertyDataOffset("normalFont");
	PropOffsets_GC.textPlane = cls->GetPropertyDataOffset("textPlane");
	PropOffsets_GC.textPolyFlags = cls->GetPropertyDataOffset("textPolyFlags");
	PropOffsets_GC.textVSpacing = cls->GetPropertyDataOffset("textVSpacing");
	PropOffsets_GC.tileColor = cls->GetPropertyDataOffset("tileColor");
	PropOffsets_GC.tilePlane = cls->GetPropertyDataOffset("tilePlane");
	PropOffsets_GC.underlineHeight = cls->GetPropertyDataOffset("underlineHeight");
	PropOffsets_GC.underlineTexture = cls->GetPropertyDataOffset("underlineTexture");
	PropOffsets_GC.vMultiplier = cls->GetPropertyDataOffset("vMultiplier");
}

PropertyOffsets_LargeTextWindow PropOffsets_LargeTextWindow;

static void InitPropertyOffsets_LargeTextWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "LargeTextWindow"));
	if (!cls)
	{
		memset(&PropOffsets_LargeTextWindow, 0xff, sizeof(PropOffsets_LargeTextWindow));
		return;
	}
	PropOffsets_LargeTextWindow.lineHeight = cls->GetPropertyDataOffset("lineHeight");
	PropOffsets_LargeTextWindow.queryRowData = cls->GetPropertyDataOffset("queryRowData");
	PropOffsets_LargeTextWindow.queryTextParams = cls->GetPropertyDataOffset("queryTextParams");
	PropOffsets_LargeTextWindow.rowData = cls->GetPropertyDataOffset("rowData");
	PropOffsets_LargeTextWindow.tempRowData = cls->GetPropertyDataOffset("tempRowData");
	PropOffsets_LargeTextWindow.textParams = cls->GetPropertyDataOffset("textParams");
	PropOffsets_LargeTextWindow.vSpace = cls->GetPropertyDataOffset("vSpace");
}

PropertyOffsets_ListWindow PropOffsets_ListWindow;

static void InitPropertyOffsets_ListWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ListWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ListWindow, 0xff, sizeof(PropOffsets_ListWindow));
		return;
	}
	PropOffsets_ListWindow.ActivateSound = cls->GetPropertyDataOffset("ActivateSound");
	PropOffsets_ListWindow.Delimiter = cls->GetPropertyDataOffset("Delimiter");
	PropOffsets_ListWindow.anchorLine = cls->GetPropertyDataOffset("anchorLine");
	PropOffsets_ListWindow.bAutoExpandColumns = cls->GetPropertyDataOffset("bAutoExpandColumns");
	PropOffsets_ListWindow.bAutoSort = cls->GetPropertyDataOffset("bAutoSort");
	PropOffsets_ListWindow.bDragging = cls->GetPropertyDataOffset("bDragging");
	PropOffsets_ListWindow.bHotKeys = cls->GetPropertyDataOffset("bHotKeys");
	PropOffsets_ListWindow.bMultiSelect = cls->GetPropertyDataOffset("bMultiSelect");
	PropOffsets_ListWindow.colMargin = cls->GetPropertyDataOffset("colMargin");
	PropOffsets_ListWindow.cols = cls->GetPropertyDataOffset("cols");
	PropOffsets_ListWindow.focusColor = cls->GetPropertyDataOffset("focusColor");
	PropOffsets_ListWindow.focusLine = cls->GetPropertyDataOffset("focusLine");
	PropOffsets_ListWindow.focusTexture = cls->GetPropertyDataOffset("focusTexture");
	PropOffsets_ListWindow.focusThickness = cls->GetPropertyDataOffset("focusThickness");
	PropOffsets_ListWindow.highlightColor = cls->GetPropertyDataOffset("highlightColor");
	PropOffsets_ListWindow.highlightTexture = cls->GetPropertyDataOffset("highlightTexture");
	PropOffsets_ListWindow.hotKeyCol = cls->GetPropertyDataOffset("hotKeyCol");
	PropOffsets_ListWindow.hotKeyString = cls->GetPropertyDataOffset("hotKeyString");
	PropOffsets_ListWindow.hotKeyTimer = cls->GetPropertyDataOffset("hotKeyTimer");
	PropOffsets_ListWindow.inverseColor = cls->GetPropertyDataOffset("inverseColor");
	PropOffsets_ListWindow.lastIndex = cls->GetPropertyDataOffset("lastIndex");
	PropOffsets_ListWindow.lineSize = cls->GetPropertyDataOffset("lineSize");
	PropOffsets_ListWindow.moveSound = cls->GetPropertyDataOffset("moveSound");
	PropOffsets_ListWindow.numSelected = cls->GetPropertyDataOffset("numSelected");
	PropOffsets_ListWindow.remainingDelay = cls->GetPropertyDataOffset("remainingDelay");
	PropOffsets_ListWindow.rowMargin = cls->GetPropertyDataOffset("rowMargin");
	PropOffsets_ListWindow.rows = cls->GetPropertyDataOffset("rows");
}

PropertyOffsets_ModalWindow PropOffsets_ModalWindow;

static void InitPropertyOffsets_ModalWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ModalWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ModalWindow, 0xff, sizeof(PropOffsets_ModalWindow));
		return;
	}
	PropOffsets_ModalWindow.acceleratorTable = cls->GetPropertyDataOffset("acceleratorTable");
	PropOffsets_ModalWindow.bDirtyAccelerators = cls->GetPropertyDataOffset("bDirtyAccelerators");
	PropOffsets_ModalWindow.focusMode = cls->GetPropertyDataOffset("focusMode");
	PropOffsets_ModalWindow.preferredFocus = cls->GetPropertyDataOffset("preferredFocus");
	PropOffsets_ModalWindow.tabGroupWindowList = cls->GetPropertyDataOffset("tabGroupWindowList");
}

PropertyOffsets_PlayerPawnExt PropOffsets_PlayerPawnExt;

static void InitPropertyOffsets_PlayerPawnExt(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "PlayerPawnExt"));
	if (!cls)
	{
		memset(&PropOffsets_PlayerPawnExt, 0xff, sizeof(PropOffsets_PlayerPawnExt));
		return;
	}
	PropOffsets_PlayerPawnExt.FlagBase = cls->GetPropertyDataOffset("FlagBase");
	PropOffsets_PlayerPawnExt.RootWindow = cls->GetPropertyDataOffset("RootWindow");
	PropOffsets_PlayerPawnExt.actorCount = cls->GetPropertyDataOffset("actorCount");
	PropOffsets_PlayerPawnExt.actorList = cls->GetPropertyDataOffset("actorList");
}

PropertyOffsets_RadioBoxWindow PropOffsets_RadioBoxWindow;

static void InitPropertyOffsets_RadioBoxWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "RadioBoxWindow"));
	if (!cls)
	{
		memset(&PropOffsets_RadioBoxWindow, 0xff, sizeof(PropOffsets_RadioBoxWindow));
		return;
	}
	PropOffsets_RadioBoxWindow.bOneCheck = cls->GetPropertyDataOffset("bOneCheck");
	PropOffsets_RadioBoxWindow.currentSelection = cls->GetPropertyDataOffset("currentSelection");
	PropOffsets_RadioBoxWindow.toggleButtons = cls->GetPropertyDataOffset("toggleButtons");
}

PropertyOffsets_RootWindow PropOffsets_RootWindow;

static void InitPropertyOffsets_RootWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "RootWindow"));
	if (!cls)
	{
		memset(&PropOffsets_RootWindow, 0xff, sizeof(PropOffsets_RootWindow));
		return;
	}
	PropOffsets_RootWindow.DefaultMoveCursor = cls->GetPropertyDataOffset("DefaultMoveCursor");
	PropOffsets_RootWindow.FocusWindow = cls->GetPropertyDataOffset("FocusWindow");
	PropOffsets_RootWindow.MouseX = cls->GetPropertyDataOffset("MouseX");
	PropOffsets_RootWindow.MouseY = cls->GetPropertyDataOffset("MouseY");
	PropOffsets_RootWindow.TickCycles = cls->GetPropertyDataOffset("TickCycles");
	PropOffsets_RootWindow.bClipRender = cls->GetPropertyDataOffset("bClipRender");
	PropOffsets_RootWindow.bCursorVisible = cls->GetPropertyDataOffset("bCursorVisible");
	PropOffsets_RootWindow.bMouseButtonLocked = cls->GetPropertyDataOffset("bMouseButtonLocked");
	PropOffsets_RootWindow.bMouseMoveLocked = cls->GetPropertyDataOffset("bMouseMoveLocked");
	PropOffsets_RootWindow.bMouseMoved = cls->GetPropertyDataOffset("bMouseMoved");
	PropOffsets_RootWindow.bPositionalSound = cls->GetPropertyDataOffset("bPositionalSound");
	PropOffsets_RootWindow.bRender = cls->GetPropertyDataOffset("bRender");
	PropOffsets_RootWindow.bShowFrames = cls->GetPropertyDataOffset("bShowFrames");
	PropOffsets_RootWindow.bShowStats = cls->GetPropertyDataOffset("bShowStats");
	PropOffsets_RootWindow.bStretchRawBackground = cls->GetPropertyDataOffset("bStretchRawBackground");
	PropOffsets_RootWindow.clickCount = cls->GetPropertyDataOffset("clickCount");
	PropOffsets_RootWindow.debugTexture = cls->GetPropertyDataOffset("debugTexture");
	PropOffsets_RootWindow.defaultEditCursor = cls->GetPropertyDataOffset("defaultEditCursor");
	PropOffsets_RootWindow.defaultHorizontalMoveCursor = cls->GetPropertyDataOffset("defaultHorizontalMoveCursor");
	PropOffsets_RootWindow.defaultTopLeftMoveCursor = cls->GetPropertyDataOffset("defaultTopLeftMoveCursor");
	PropOffsets_RootWindow.defaultTopRightMoveCursor = cls->GetPropertyDataOffset("defaultTopRightMoveCursor");
	PropOffsets_RootWindow.defaultVerticalMoveCursor = cls->GetPropertyDataOffset("defaultVerticalMoveCursor");
	PropOffsets_RootWindow.firstButtonMouseX = cls->GetPropertyDataOffset("firstButtonMouseX");
	PropOffsets_RootWindow.firstButtonMouseY = cls->GetPropertyDataOffset("firstButtonMouseY");
	PropOffsets_RootWindow.frameTimer = cls->GetPropertyDataOffset("frameTimer");
	PropOffsets_RootWindow.grabbedWindow = cls->GetPropertyDataOffset("grabbedWindow");
	PropOffsets_RootWindow.hMultiplier = cls->GetPropertyDataOffset("hMultiplier");
	PropOffsets_RootWindow.handleKeyboardRef = cls->GetPropertyDataOffset("handleKeyboardRef");
	PropOffsets_RootWindow.handleMouseRef = cls->GetPropertyDataOffset("handleMouseRef");
	PropOffsets_RootWindow.initCount = cls->GetPropertyDataOffset("initCount");
	PropOffsets_RootWindow.keyDownMap = cls->GetPropertyDataOffset("keyDownMap");
	PropOffsets_RootWindow.lastButtonPress = cls->GetPropertyDataOffset("lastButtonPress");
	PropOffsets_RootWindow.lastButtonType = cls->GetPropertyDataOffset("lastButtonType");
	PropOffsets_RootWindow.lastButtonWindow = cls->GetPropertyDataOffset("lastButtonWindow");
	PropOffsets_RootWindow.lastMouseWindow = cls->GetPropertyDataOffset("lastMouseWindow");
	PropOffsets_RootWindow.maxMouseDist = cls->GetPropertyDataOffset("maxMouseDist");
	PropOffsets_RootWindow.multiClickTimeout = cls->GetPropertyDataOffset("multiClickTimeout");
	PropOffsets_RootWindow.nextRootWindow = cls->GetPropertyDataOffset("nextRootWindow");
	PropOffsets_RootWindow.paintCycles = cls->GetPropertyDataOffset("paintCycles");
	PropOffsets_RootWindow.parentPawn = cls->GetPropertyDataOffset("parentPawn");
	PropOffsets_RootWindow.prevMouseX = cls->GetPropertyDataOffset("prevMouseX");
	PropOffsets_RootWindow.prevMouseY = cls->GetPropertyDataOffset("prevMouseY");
	PropOffsets_RootWindow.rawBackground = cls->GetPropertyDataOffset("rawBackground");
	PropOffsets_RootWindow.rawBackgroundHeight = cls->GetPropertyDataOffset("rawBackgroundHeight");
	PropOffsets_RootWindow.rawBackgroundWidth = cls->GetPropertyDataOffset("rawBackgroundWidth");
	PropOffsets_RootWindow.rawColor = cls->GetPropertyDataOffset("rawColor");
	PropOffsets_RootWindow.renderHeight = cls->GetPropertyDataOffset("renderHeight");
	PropOffsets_RootWindow.renderWidth = cls->GetPropertyDataOffset("renderWidth");
	PropOffsets_RootWindow.renderX = cls->GetPropertyDataOffset("renderX");
	PropOffsets_RootWindow.renderY = cls->GetPropertyDataOffset("renderY");
	PropOffsets_RootWindow.rootFrame = cls->GetPropertyDataOffset("rootFrame");
	PropOffsets_RootWindow.snapshotHeight = cls->GetPropertyDataOffset("snapshotHeight");
	PropOffsets_RootWindow.snapshotWidth = cls->GetPropertyDataOffset("snapshotWidth");
	PropOffsets_RootWindow.vMultiplier = cls->GetPropertyDataOffset("vMultiplier");
}

PropertyOffsets_ScaleManagerWindow PropOffsets_ScaleManagerWindow;

static void InitPropertyOffsets_ScaleManagerWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ScaleManagerWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ScaleManagerWindow, 0xff, sizeof(PropOffsets_ScaleManagerWindow));
		return;
	}
	PropOffsets_ScaleManagerWindow.Scale = cls->GetPropertyDataOffset("Scale");
	PropOffsets_ScaleManagerWindow.Spacing = cls->GetPropertyDataOffset("Spacing");
	PropOffsets_ScaleManagerWindow.bStretchScaleField = cls->GetPropertyDataOffset("bStretchScaleField");
	PropOffsets_ScaleManagerWindow.bStretchValueField = cls->GetPropertyDataOffset("bStretchValueField");
	PropOffsets_ScaleManagerWindow.childHAlign = cls->GetPropertyDataOffset("childHAlign");
	PropOffsets_ScaleManagerWindow.childVAlign = cls->GetPropertyDataOffset("childVAlign");
	PropOffsets_ScaleManagerWindow.decButton = cls->GetPropertyDataOffset("decButton");
	PropOffsets_ScaleManagerWindow.incButton = cls->GetPropertyDataOffset("incButton");
	PropOffsets_ScaleManagerWindow.marginHeight = cls->GetPropertyDataOffset("marginHeight");
	PropOffsets_ScaleManagerWindow.marginWidth = cls->GetPropertyDataOffset("marginWidth");
	PropOffsets_ScaleManagerWindow.orientation = cls->GetPropertyDataOffset("orientation");
	PropOffsets_ScaleManagerWindow.valueField = cls->GetPropertyDataOffset("valueField");
}

PropertyOffsets_ScaleWindow PropOffsets_ScaleWindow;

static void InitPropertyOffsets_ScaleWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ScaleWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ScaleWindow, 0xff, sizeof(PropOffsets_ScaleWindow));
		return;
	}
	PropOffsets_ScaleWindow.RemainingTime = cls->GetPropertyDataOffset("RemainingTime");
	PropOffsets_ScaleWindow.ThumbHeight = cls->GetPropertyDataOffset("ThumbHeight");
	PropOffsets_ScaleWindow.ThumbWidth = cls->GetPropertyDataOffset("ThumbWidth");
	PropOffsets_ScaleWindow.absEndScale = cls->GetPropertyDataOffset("absEndScale");
	PropOffsets_ScaleWindow.absStartScale = cls->GetPropertyDataOffset("absStartScale");
	PropOffsets_ScaleWindow.bDraggingThumb = cls->GetPropertyDataOffset("bDraggingThumb");
	PropOffsets_ScaleWindow.bDrawEndTicks = cls->GetPropertyDataOffset("bDrawEndTicks");
	PropOffsets_ScaleWindow.bRepeatScaleTexture = cls->GetPropertyDataOffset("bRepeatScaleTexture");
	PropOffsets_ScaleWindow.bRepeatThumbTexture = cls->GetPropertyDataOffset("bRepeatThumbTexture");
	PropOffsets_ScaleWindow.bSpanThumb = cls->GetPropertyDataOffset("bSpanThumb");
	PropOffsets_ScaleWindow.bStretchScale = cls->GetPropertyDataOffset("bStretchScale");
	PropOffsets_ScaleWindow.borderPattern = cls->GetPropertyDataOffset("borderPattern");
	PropOffsets_ScaleWindow.clickSound = cls->GetPropertyDataOffset("clickSound");
	PropOffsets_ScaleWindow.currentPos = cls->GetPropertyDataOffset("currentPos");
	PropOffsets_ScaleWindow.dragSound = cls->GetPropertyDataOffset("dragSound");
	PropOffsets_ScaleWindow.endOffset = cls->GetPropertyDataOffset("endOffset");
	PropOffsets_ScaleWindow.enumStrings = cls->GetPropertyDataOffset("enumStrings");
	PropOffsets_ScaleWindow.fromValue = cls->GetPropertyDataOffset("fromValue");
	PropOffsets_ScaleWindow.initialDelay = cls->GetPropertyDataOffset("initialDelay");
	PropOffsets_ScaleWindow.initialPos = cls->GetPropertyDataOffset("initialPos");
	PropOffsets_ScaleWindow.marginHeight = cls->GetPropertyDataOffset("marginHeight");
	PropOffsets_ScaleWindow.marginWidth = cls->GetPropertyDataOffset("marginWidth");
	PropOffsets_ScaleWindow.mousePos = cls->GetPropertyDataOffset("mousePos");
	PropOffsets_ScaleWindow.numPositions = cls->GetPropertyDataOffset("numPositions");
	PropOffsets_ScaleWindow.orientation = cls->GetPropertyDataOffset("orientation");
	PropOffsets_ScaleWindow.postCapH = cls->GetPropertyDataOffset("postCapH");
	PropOffsets_ScaleWindow.postCapHeight = cls->GetPropertyDataOffset("postCapHeight");
	PropOffsets_ScaleWindow.postCapTexture = cls->GetPropertyDataOffset("postCapTexture");
	PropOffsets_ScaleWindow.postCapW = cls->GetPropertyDataOffset("postCapW");
	PropOffsets_ScaleWindow.postCapWidth = cls->GetPropertyDataOffset("postCapWidth");
	PropOffsets_ScaleWindow.postCapXOff = cls->GetPropertyDataOffset("postCapXOff");
	PropOffsets_ScaleWindow.postCapYOff = cls->GetPropertyDataOffset("postCapYOff");
	PropOffsets_ScaleWindow.preCapH = cls->GetPropertyDataOffset("preCapH");
	PropOffsets_ScaleWindow.preCapHeight = cls->GetPropertyDataOffset("preCapHeight");
	PropOffsets_ScaleWindow.preCapTexture = cls->GetPropertyDataOffset("preCapTexture");
	PropOffsets_ScaleWindow.preCapW = cls->GetPropertyDataOffset("preCapW");
	PropOffsets_ScaleWindow.preCapWidth = cls->GetPropertyDataOffset("preCapWidth");
	PropOffsets_ScaleWindow.preCapXOff = cls->GetPropertyDataOffset("preCapXOff");
	PropOffsets_ScaleWindow.preCapYOff = cls->GetPropertyDataOffset("preCapYOff");
	PropOffsets_ScaleWindow.repeatDir = cls->GetPropertyDataOffset("repeatDir");
	PropOffsets_ScaleWindow.repeatRate = cls->GetPropertyDataOffset("repeatRate");
	PropOffsets_ScaleWindow.scaleBorderColor = cls->GetPropertyDataOffset("scaleBorderColor");
	PropOffsets_ScaleWindow.scaleBorderSize = cls->GetPropertyDataOffset("scaleBorderSize");
	PropOffsets_ScaleWindow.scaleColor = cls->GetPropertyDataOffset("scaleColor");
	PropOffsets_ScaleWindow.scaleH = cls->GetPropertyDataOffset("scaleH");
	PropOffsets_ScaleWindow.scaleHeight = cls->GetPropertyDataOffset("scaleHeight");
	PropOffsets_ScaleWindow.scaleStyle = cls->GetPropertyDataOffset("scaleStyle");
	PropOffsets_ScaleWindow.scaleTexture = cls->GetPropertyDataOffset("scaleTexture");
	PropOffsets_ScaleWindow.scaleW = cls->GetPropertyDataOffset("scaleW");
	PropOffsets_ScaleWindow.scaleWidth = cls->GetPropertyDataOffset("scaleWidth");
	PropOffsets_ScaleWindow.scaleX = cls->GetPropertyDataOffset("scaleX");
	PropOffsets_ScaleWindow.scaleY = cls->GetPropertyDataOffset("scaleY");
	PropOffsets_ScaleWindow.setSound = cls->GetPropertyDataOffset("setSound");
	PropOffsets_ScaleWindow.spanRange = cls->GetPropertyDataOffset("spanRange");
	PropOffsets_ScaleWindow.startOffset = cls->GetPropertyDataOffset("startOffset");
	PropOffsets_ScaleWindow.thumbBorderColor = cls->GetPropertyDataOffset("thumbBorderColor");
	PropOffsets_ScaleWindow.thumbBorderSize = cls->GetPropertyDataOffset("thumbBorderSize");
	PropOffsets_ScaleWindow.thumbColor = cls->GetPropertyDataOffset("thumbColor");
	PropOffsets_ScaleWindow.thumbH = cls->GetPropertyDataOffset("thumbH");
	PropOffsets_ScaleWindow.thumbStep = cls->GetPropertyDataOffset("thumbStep");
	PropOffsets_ScaleWindow.thumbStyle = cls->GetPropertyDataOffset("thumbStyle");
	PropOffsets_ScaleWindow.thumbTexture = cls->GetPropertyDataOffset("thumbTexture");
	PropOffsets_ScaleWindow.thumbW = cls->GetPropertyDataOffset("thumbW");
	PropOffsets_ScaleWindow.thumbX = cls->GetPropertyDataOffset("thumbX");
	PropOffsets_ScaleWindow.thumbY = cls->GetPropertyDataOffset("thumbY");
	PropOffsets_ScaleWindow.tickColor = cls->GetPropertyDataOffset("tickColor");
	PropOffsets_ScaleWindow.tickH = cls->GetPropertyDataOffset("tickH");
	PropOffsets_ScaleWindow.tickHeight = cls->GetPropertyDataOffset("tickHeight");
	PropOffsets_ScaleWindow.tickStyle = cls->GetPropertyDataOffset("tickStyle");
	PropOffsets_ScaleWindow.tickTexture = cls->GetPropertyDataOffset("tickTexture");
	PropOffsets_ScaleWindow.tickW = cls->GetPropertyDataOffset("tickW");
	PropOffsets_ScaleWindow.tickWidth = cls->GetPropertyDataOffset("tickWidth");
	PropOffsets_ScaleWindow.tickX = cls->GetPropertyDataOffset("tickX");
	PropOffsets_ScaleWindow.tickY = cls->GetPropertyDataOffset("tickY");
	PropOffsets_ScaleWindow.toValue = cls->GetPropertyDataOffset("toValue");
	PropOffsets_ScaleWindow.valueFmt = cls->GetPropertyDataOffset("valueFmt");
}

PropertyOffsets_ScrollAreaWindow PropOffsets_ScrollAreaWindow;

static void InitPropertyOffsets_ScrollAreaWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ScrollAreaWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ScrollAreaWindow, 0xff, sizeof(PropOffsets_ScrollAreaWindow));
		return;
	}
	PropOffsets_ScrollAreaWindow.ClipWindow = cls->GetPropertyDataOffset("ClipWindow");
	PropOffsets_ScrollAreaWindow.DownButton = cls->GetPropertyDataOffset("DownButton");
	PropOffsets_ScrollAreaWindow.LeftButton = cls->GetPropertyDataOffset("LeftButton");
	PropOffsets_ScrollAreaWindow.RightButton = cls->GetPropertyDataOffset("RightButton");
	PropOffsets_ScrollAreaWindow.UpButton = cls->GetPropertyDataOffset("UpButton");
	PropOffsets_ScrollAreaWindow.bHLastShow = cls->GetPropertyDataOffset("bHLastShow");
	PropOffsets_ScrollAreaWindow.bHideScrollbars = cls->GetPropertyDataOffset("bHideScrollbars");
	PropOffsets_ScrollAreaWindow.bVLastShow = cls->GetPropertyDataOffset("bVLastShow");
	PropOffsets_ScrollAreaWindow.hScale = cls->GetPropertyDataOffset("hScale");
	PropOffsets_ScrollAreaWindow.hScaleMgr = cls->GetPropertyDataOffset("hScaleMgr");
	PropOffsets_ScrollAreaWindow.marginHeight = cls->GetPropertyDataOffset("marginHeight");
	PropOffsets_ScrollAreaWindow.marginWidth = cls->GetPropertyDataOffset("marginWidth");
	PropOffsets_ScrollAreaWindow.scrollbarDistance = cls->GetPropertyDataOffset("scrollbarDistance");
	PropOffsets_ScrollAreaWindow.vScale = cls->GetPropertyDataOffset("vScale");
	PropOffsets_ScrollAreaWindow.vScaleMgr = cls->GetPropertyDataOffset("vScaleMgr");
}

PropertyOffsets_TextLogWindow PropOffsets_TextLogWindow;

static void InitPropertyOffsets_TextLogWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "TextLogWindow"));
	if (!cls)
	{
		memset(&PropOffsets_TextLogWindow, 0xff, sizeof(PropOffsets_TextLogWindow));
		return;
	}
	PropOffsets_TextLogWindow.Lines = cls->GetPropertyDataOffset("Lines");
	PropOffsets_TextLogWindow.bPaused = cls->GetPropertyDataOffset("bPaused");
	PropOffsets_TextLogWindow.bTooTall = cls->GetPropertyDataOffset("bTooTall");
	PropOffsets_TextLogWindow.textTimeout = cls->GetPropertyDataOffset("textTimeout");
}

PropertyOffsets_TextWindow PropOffsets_TextWindow;

static void InitPropertyOffsets_TextWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "TextWindow"));
	if (!cls)
	{
		memset(&PropOffsets_TextWindow, 0xff, sizeof(PropOffsets_TextWindow));
		return;
	}
	PropOffsets_TextWindow.HAlign = cls->GetPropertyDataOffset("HAlign");
	PropOffsets_TextWindow.MaxLines = cls->GetPropertyDataOffset("MaxLines");
	PropOffsets_TextWindow.MinWidth = cls->GetPropertyDataOffset("MinWidth");
	PropOffsets_TextWindow.Text = cls->GetPropertyDataOffset("Text");
	PropOffsets_TextWindow.VAlign = cls->GetPropertyDataOffset("VAlign");
	PropOffsets_TextWindow.bTextIsAccelerator = cls->GetPropertyDataOffset("bTextIsAccelerator");
	PropOffsets_TextWindow.bWordWrap = cls->GetPropertyDataOffset("bWordWrap");
	PropOffsets_TextWindow.hMargin = cls->GetPropertyDataOffset("hMargin");
	PropOffsets_TextWindow.minLines = cls->GetPropertyDataOffset("minLines");
	PropOffsets_TextWindow.vMargin = cls->GetPropertyDataOffset("vMargin");
}

PropertyOffsets_TileWindow PropOffsets_TileWindow;

static void InitPropertyOffsets_TileWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "TileWindow"));
	if (!cls)
	{
		memset(&PropOffsets_TileWindow, 0xff, sizeof(PropOffsets_TileWindow));
		return;
	}
	PropOffsets_TileWindow.bEqualHeight = cls->GetPropertyDataOffset("bEqualHeight");
	PropOffsets_TileWindow.bEqualWidth = cls->GetPropertyDataOffset("bEqualWidth");
	PropOffsets_TileWindow.bFillParent = cls->GetPropertyDataOffset("bFillParent");
	PropOffsets_TileWindow.bWrap = cls->GetPropertyDataOffset("bWrap");
	PropOffsets_TileWindow.hChildAlign = cls->GetPropertyDataOffset("hChildAlign");
	PropOffsets_TileWindow.hDirection = cls->GetPropertyDataOffset("hDirection");
	PropOffsets_TileWindow.hMargin = cls->GetPropertyDataOffset("hMargin");
	PropOffsets_TileWindow.majorSpacing = cls->GetPropertyDataOffset("majorSpacing");
	PropOffsets_TileWindow.minorSpacing = cls->GetPropertyDataOffset("minorSpacing");
	PropOffsets_TileWindow.orientation = cls->GetPropertyDataOffset("orientation");
	PropOffsets_TileWindow.rowArray = cls->GetPropertyDataOffset("rowArray");
	PropOffsets_TileWindow.vChildAlign = cls->GetPropertyDataOffset("vChildAlign");
	PropOffsets_TileWindow.vDirection = cls->GetPropertyDataOffset("vDirection");
	PropOffsets_TileWindow.vMargin = cls->GetPropertyDataOffset("vMargin");
}

PropertyOffsets_ToggleWindow PropOffsets_ToggleWindow;

static void InitPropertyOffsets_ToggleWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ToggleWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ToggleWindow, 0xff, sizeof(PropOffsets_ToggleWindow));
		return;
	}
	PropOffsets_ToggleWindow.disableSound = cls->GetPropertyDataOffset("disableSound");
	PropOffsets_ToggleWindow.enableSound = cls->GetPropertyDataOffset("enableSound");
}

PropertyOffsets_ViewportWindow PropOffsets_ViewportWindow;

static void InitPropertyOffsets_ViewportWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "ViewportWindow"));
	if (!cls)
	{
		memset(&PropOffsets_ViewportWindow, 0xff, sizeof(PropOffsets_ViewportWindow));
		return;
	}
	PropOffsets_ViewportWindow.DefaultColor = cls->GetPropertyDataOffset("DefaultColor");
	PropOffsets_ViewportWindow.DefaultTexture = cls->GetPropertyDataOffset("DefaultTexture");
	PropOffsets_ViewportWindow.FOV = cls->GetPropertyDataOffset("FOV");
	PropOffsets_ViewportWindow.Location = cls->GetPropertyDataOffset("Location");
	PropOffsets_ViewportWindow.Rotation = cls->GetPropertyDataOffset("Rotation");
	PropOffsets_ViewportWindow.bClearZ = cls->GetPropertyDataOffset("bClearZ");
	PropOffsets_ViewportWindow.bEnableViewport = cls->GetPropertyDataOffset("bEnableViewport");
	PropOffsets_ViewportWindow.bOriginActorDestroyed = cls->GetPropertyDataOffset("bOriginActorDestroyed");
	PropOffsets_ViewportWindow.bShowActor = cls->GetPropertyDataOffset("bShowActor");
	PropOffsets_ViewportWindow.bShowWeapons = cls->GetPropertyDataOffset("bShowWeapons");
	PropOffsets_ViewportWindow.bUseEyeHeight = cls->GetPropertyDataOffset("bUseEyeHeight");
	PropOffsets_ViewportWindow.bUseViewRotation = cls->GetPropertyDataOffset("bUseViewRotation");
	PropOffsets_ViewportWindow.bWatchEyeHeight = cls->GetPropertyDataOffset("bWatchEyeHeight");
	PropOffsets_ViewportWindow.lastLocation = cls->GetPropertyDataOffset("lastLocation");
	PropOffsets_ViewportWindow.lastRotation = cls->GetPropertyDataOffset("lastRotation");
	PropOffsets_ViewportWindow.originActor = cls->GetPropertyDataOffset("originActor");
	PropOffsets_ViewportWindow.relLocation = cls->GetPropertyDataOffset("relLocation");
	PropOffsets_ViewportWindow.relRotation = cls->GetPropertyDataOffset("relRotation");
	PropOffsets_ViewportWindow.viewportFrame = cls->GetPropertyDataOffset("viewportFrame");
	PropOffsets_ViewportWindow.watchActor = cls->GetPropertyDataOffset("watchActor");
}

PropertyOffsets_Window PropOffsets_Window;

static void InitPropertyOffsets_Window(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "Window"));
	if (!cls)
	{
		memset(&PropOffsets_Window, 0xff, sizeof(PropOffsets_Window));
		return;
	}
	PropOffsets_Window.Background = cls->GetPropertyDataOffset("Background");
	PropOffsets_Window.Height = cls->GetPropertyDataOffset("Height");
	PropOffsets_Window.SoundVolume = cls->GetPropertyDataOffset("SoundVolume");
	PropOffsets_Window.TextColor = cls->GetPropertyDataOffset("TextColor");
	PropOffsets_Window.Width = cls->GetPropertyDataOffset("Width");
	PropOffsets_Window.X = cls->GetPropertyDataOffset("X");
	PropOffsets_Window.Y = cls->GetPropertyDataOffset("Y");
	PropOffsets_Window.acceleratorKey = cls->GetPropertyDataOffset("acceleratorKey");
	PropOffsets_Window.att = cls->GetPropertyDataOffset("att");
	PropOffsets_Window.bBeingDestroyed = cls->GetPropertyDataOffset("bBeingDestroyed");
	PropOffsets_Window.bConfigured = cls->GetPropertyDataOffset("bConfigured");
	PropOffsets_Window.bDrawRawBackground = cls->GetPropertyDataOffset("bDrawRawBackground");
	PropOffsets_Window.bIsInitialized = cls->GetPropertyDataOffset("bIsInitialized");
	PropOffsets_Window.bIsSelectable = cls->GetPropertyDataOffset("bIsSelectable");
	PropOffsets_Window.bIsSensitive = cls->GetPropertyDataOffset("bIsSensitive");
	PropOffsets_Window.bIsVisible = cls->GetPropertyDataOffset("bIsVisible");
	PropOffsets_Window.bLastHeightSpecified = cls->GetPropertyDataOffset("bLastHeightSpecified");
	PropOffsets_Window.bLastWidthSpecified = cls->GetPropertyDataOffset("bLastWidthSpecified");
	PropOffsets_Window.bNeedsQuery = cls->GetPropertyDataOffset("bNeedsQuery");
	PropOffsets_Window.bNeedsReconfigure = cls->GetPropertyDataOffset("bNeedsReconfigure");
	PropOffsets_Window.bSmoothBackground = cls->GetPropertyDataOffset("bSmoothBackground");
	PropOffsets_Window.bSpecialText = cls->GetPropertyDataOffset("bSpecialText");
	PropOffsets_Window.bStretchBackground = cls->GetPropertyDataOffset("bStretchBackground");
	PropOffsets_Window.bTextTranslucent = cls->GetPropertyDataOffset("bTextTranslucent");
	PropOffsets_Window.bTickEnabled = cls->GetPropertyDataOffset("bTickEnabled");
	PropOffsets_Window.backgroundStyle = cls->GetPropertyDataOffset("backgroundStyle");
	PropOffsets_Window.boldFont = cls->GetPropertyDataOffset("boldFont");
	PropOffsets_Window.clientObject = cls->GetPropertyDataOffset("clientObject");
	PropOffsets_Window.colMajorIndex = cls->GetPropertyDataOffset("colMajorIndex");
	PropOffsets_Window.defaultCursor = cls->GetPropertyDataOffset("defaultCursor");
	PropOffsets_Window.defaultCursorColor = cls->GetPropertyDataOffset("defaultCursorColor");
	PropOffsets_Window.defaultCursorShadow = cls->GetPropertyDataOffset("defaultCursorShadow");
	PropOffsets_Window.defaultHotX = cls->GetPropertyDataOffset("defaultHotX");
	PropOffsets_Window.defaultHotY = cls->GetPropertyDataOffset("defaultHotY");
	PropOffsets_Window.firstChild = cls->GetPropertyDataOffset("firstChild");
	PropOffsets_Window.firstTimer = cls->GetPropertyDataOffset("firstTimer");
	PropOffsets_Window.focusSound = cls->GetPropertyDataOffset("focusSound");
	PropOffsets_Window.freeTimer = cls->GetPropertyDataOffset("freeTimer");
	PropOffsets_Window.gGc = cls->GetPropertyDataOffset("gGc");
	PropOffsets_Window.hMargin0 = cls->GetPropertyDataOffset("hMargin0");
	PropOffsets_Window.hMargin1 = cls->GetPropertyDataOffset("hMargin1");
	PropOffsets_Window.hardcodedHeight = cls->GetPropertyDataOffset("hardcodedHeight");
	PropOffsets_Window.hardcodedWidth = cls->GetPropertyDataOffset("hardcodedWidth");
	PropOffsets_Window.holdHeight = cls->GetPropertyDataOffset("holdHeight");
	PropOffsets_Window.holdWidth = cls->GetPropertyDataOffset("holdWidth");
	PropOffsets_Window.holdX = cls->GetPropertyDataOffset("holdX");
	PropOffsets_Window.holdY = cls->GetPropertyDataOffset("holdY");
	PropOffsets_Window.invisibleSound = cls->GetPropertyDataOffset("invisibleSound");
	PropOffsets_Window.lastChild = cls->GetPropertyDataOffset("lastChild");
	PropOffsets_Window.lastQueryHeight = cls->GetPropertyDataOffset("lastQueryHeight");
	PropOffsets_Window.lastQueryWidth = cls->GetPropertyDataOffset("lastQueryWidth");
	PropOffsets_Window.lastSpecifiedHeight = cls->GetPropertyDataOffset("lastSpecifiedHeight");
	PropOffsets_Window.lastSpecifiedWidth = cls->GetPropertyDataOffset("lastSpecifiedWidth");
	PropOffsets_Window.lockCount = cls->GetPropertyDataOffset("lockCount");
	PropOffsets_Window.maxClicks = cls->GetPropertyDataOffset("maxClicks");
	PropOffsets_Window.nextSibling = cls->GetPropertyDataOffset("nextSibling");
	PropOffsets_Window.normalFont = cls->GetPropertyDataOffset("normalFont");
	PropOffsets_Window.parentOwner = cls->GetPropertyDataOffset("parentOwner");
	PropOffsets_Window.prevSibling = cls->GetPropertyDataOffset("prevSibling");
	PropOffsets_Window.rowMajorIndex = cls->GetPropertyDataOffset("rowMajorIndex");
	PropOffsets_Window.textPlane = cls->GetPropertyDataOffset("textPlane");
	PropOffsets_Window.textVSpacing = cls->GetPropertyDataOffset("textVSpacing");
	PropOffsets_Window.tileColor = cls->GetPropertyDataOffset("tileColor");
	PropOffsets_Window.tilePlane = cls->GetPropertyDataOffset("tilePlane");
	PropOffsets_Window.unfocusSound = cls->GetPropertyDataOffset("unfocusSound");
	PropOffsets_Window.vMargin0 = cls->GetPropertyDataOffset("vMargin0");
	PropOffsets_Window.vMargin1 = cls->GetPropertyDataOffset("vMargin1");
	PropOffsets_Window.visibleSound = cls->GetPropertyDataOffset("visibleSound");
	PropOffsets_Window.wGc = cls->GetPropertyDataOffset("wGc");
	PropOffsets_Window.winClipRect = cls->GetPropertyDataOffset("winClipRect");
	PropOffsets_Window.winHAlign = cls->GetPropertyDataOffset("winHAlign");
	PropOffsets_Window.winParent = cls->GetPropertyDataOffset("winParent");
	PropOffsets_Window.winVAlign = cls->GetPropertyDataOffset("winVAlign");
	PropOffsets_Window.windowType = cls->GetPropertyDataOffset("windowType");
}

PropertyOffsets_ConCamera PropOffsets_ConCamera;

static void InitPropertyOffsets_ConCamera(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConCamera"));
	if (!cls)
	{
		memset(&PropOffsets_ConCamera, 0xff, sizeof(PropOffsets_ConCamera));
		return;
	}
	PropOffsets_ConCamera.Rotation = cls->GetPropertyDataOffset("Rotation");
	PropOffsets_ConCamera.bCameraLocationSaved = cls->GetPropertyDataOffset("bCameraLocationSaved");
	PropOffsets_ConCamera.bDebug = cls->GetPropertyDataOffset("bDebug");
	PropOffsets_ConCamera.bInteractiveCamera = cls->GetPropertyDataOffset("bInteractiveCamera");
	PropOffsets_ConCamera.bUsingFallback = cls->GetPropertyDataOffset("bUsingFallback");
	PropOffsets_ConCamera.cameraActor = cls->GetPropertyDataOffset("cameraActor");
	PropOffsets_ConCamera.cameraFallbackPositions = cls->GetPropertyDataOffset("cameraFallbackPositions");
	PropOffsets_ConCamera.cameraHeightPositions = cls->GetPropertyDataOffset("cameraHeightPositions");
	PropOffsets_ConCamera.cameraMode = cls->GetPropertyDataOffset("cameraMode");
	PropOffsets_ConCamera.cameraOffset = cls->GetPropertyDataOffset("cameraOffset");
	PropOffsets_ConCamera.cameraPosition = cls->GetPropertyDataOffset("cameraPosition");
	PropOffsets_ConCamera.cameraType = cls->GetPropertyDataOffset("cameraType");
	PropOffsets_ConCamera.centerModifier = cls->GetPropertyDataOffset("centerModifier");
	PropOffsets_ConCamera.conLightSpeaker = cls->GetPropertyDataOffset("conLightSpeaker");
	PropOffsets_ConCamera.conLightSpeakingTo = cls->GetPropertyDataOffset("conLightSpeakingTo");
	PropOffsets_ConCamera.cosAngle = cls->GetPropertyDataOffset("cosAngle");
	PropOffsets_ConCamera.currentFallback = cls->GetPropertyDataOffset("currentFallback");
	PropOffsets_ConCamera.distanceMultiplier = cls->GetPropertyDataOffset("distanceMultiplier");
	PropOffsets_ConCamera.firstActor = cls->GetPropertyDataOffset("firstActor");
	PropOffsets_ConCamera.firstActorRotation = cls->GetPropertyDataOffset("firstActorRotation");
	PropOffsets_ConCamera.heightFallbackTrigger = cls->GetPropertyDataOffset("heightFallbackTrigger");
	PropOffsets_ConCamera.heightModifier = cls->GetPropertyDataOffset("heightModifier");
	PropOffsets_ConCamera.ignoreSetActors = cls->GetPropertyDataOffset("ignoreSetActors");
	PropOffsets_ConCamera.lastFirstActor = cls->GetPropertyDataOffset("lastFirstActor");
	PropOffsets_ConCamera.lastLocation = cls->GetPropertyDataOffset("lastLocation");
	PropOffsets_ConCamera.lastRotation = cls->GetPropertyDataOffset("lastRotation");
	PropOffsets_ConCamera.lastSecondActor = cls->GetPropertyDataOffset("lastSecondActor");
	PropOffsets_ConCamera.secondActor = cls->GetPropertyDataOffset("secondActor");
	PropOffsets_ConCamera.setActorCount = cls->GetPropertyDataOffset("setActorCount");
}

PropertyOffsets_ConChoice PropOffsets_ConChoice;

static void InitPropertyOffsets_ConChoice(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConChoice"));
	if (!cls)
	{
		memset(&PropOffsets_ConChoice, 0xff, sizeof(PropOffsets_ConChoice));
		return;
	}
	PropOffsets_ConChoice.bDisplayAsSpeech = cls->GetPropertyDataOffset("bDisplayAsSpeech");
	PropOffsets_ConChoice.choiceLabel = cls->GetPropertyDataOffset("choiceLabel");
	PropOffsets_ConChoice.choiceText = cls->GetPropertyDataOffset("choiceText");
	PropOffsets_ConChoice.flagRef = cls->GetPropertyDataOffset("flagRef");
	PropOffsets_ConChoice.nextChoice = cls->GetPropertyDataOffset("nextChoice");
	PropOffsets_ConChoice.skillLevelNeeded = cls->GetPropertyDataOffset("skillLevelNeeded");
	PropOffsets_ConChoice.skillNeeded = cls->GetPropertyDataOffset("skillNeeded");
	PropOffsets_ConChoice.soundID = cls->GetPropertyDataOffset("soundID");
}

PropertyOffsets_ConEventAddCredits PropOffsets_ConEventAddCredits;

static void InitPropertyOffsets_ConEventAddCredits(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventAddCredits"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventAddCredits, 0xff, sizeof(PropOffsets_ConEventAddCredits));
		return;
	}
	PropOffsets_ConEventAddCredits.creditsToAdd = cls->GetPropertyDataOffset("creditsToAdd");
}

PropertyOffsets_ConEventAddGoal PropOffsets_ConEventAddGoal;

static void InitPropertyOffsets_ConEventAddGoal(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventAddGoal"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventAddGoal, 0xff, sizeof(PropOffsets_ConEventAddGoal));
		return;
	}
	PropOffsets_ConEventAddGoal.bGoalCompleted = cls->GetPropertyDataOffset("bGoalCompleted");
	PropOffsets_ConEventAddGoal.bPrimaryGoal = cls->GetPropertyDataOffset("bPrimaryGoal");
	PropOffsets_ConEventAddGoal.goalName = cls->GetPropertyDataOffset("goalName");
	PropOffsets_ConEventAddGoal.goalText = cls->GetPropertyDataOffset("goalText");
}

PropertyOffsets_ConEventAddNote PropOffsets_ConEventAddNote;

static void InitPropertyOffsets_ConEventAddNote(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventAddNote"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventAddNote, 0xff, sizeof(PropOffsets_ConEventAddNote));
		return;
	}
	PropOffsets_ConEventAddNote.bNoteAdded = cls->GetPropertyDataOffset("bNoteAdded");
	PropOffsets_ConEventAddNote.noteText = cls->GetPropertyDataOffset("noteText");
}

PropertyOffsets_ConEventAddSkillPoints PropOffsets_ConEventAddSkillPoints;

static void InitPropertyOffsets_ConEventAddSkillPoints(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventAddSkillPoints"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventAddSkillPoints, 0xff, sizeof(PropOffsets_ConEventAddSkillPoints));
		return;
	}
	PropOffsets_ConEventAddSkillPoints.awardMessage = cls->GetPropertyDataOffset("awardMessage");
	PropOffsets_ConEventAddSkillPoints.pointsToAdd = cls->GetPropertyDataOffset("pointsToAdd");
}

PropertyOffsets_ConEventAnimation PropOffsets_ConEventAnimation;

static void InitPropertyOffsets_ConEventAnimation(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventAnimation"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventAnimation, 0xff, sizeof(PropOffsets_ConEventAnimation));
		return;
	}
	PropOffsets_ConEventAnimation.Sequence = cls->GetPropertyDataOffset("Sequence");
	PropOffsets_ConEventAnimation.bFinishAnim = cls->GetPropertyDataOffset("bFinishAnim");
	PropOffsets_ConEventAnimation.bLoopAnim = cls->GetPropertyDataOffset("bLoopAnim");
	PropOffsets_ConEventAnimation.eventOwner = cls->GetPropertyDataOffset("eventOwner");
	PropOffsets_ConEventAnimation.eventOwnerName = cls->GetPropertyDataOffset("eventOwnerName");
}

PropertyOffsets_ConEventCheckFlag PropOffsets_ConEventCheckFlag;

static void InitPropertyOffsets_ConEventCheckFlag(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventCheckFlag"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventCheckFlag, 0xff, sizeof(PropOffsets_ConEventCheckFlag));
		return;
	}
	PropOffsets_ConEventCheckFlag.flagRef = cls->GetPropertyDataOffset("flagRef");
	PropOffsets_ConEventCheckFlag.setLabel = cls->GetPropertyDataOffset("setLabel");
}

PropertyOffsets_ConEventCheckObject PropOffsets_ConEventCheckObject;

static void InitPropertyOffsets_ConEventCheckObject(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventCheckObject"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventCheckObject, 0xff, sizeof(PropOffsets_ConEventCheckObject));
		return;
	}
	PropOffsets_ConEventCheckObject.ObjectName = cls->GetPropertyDataOffset("ObjectName");
	PropOffsets_ConEventCheckObject.checkObject = cls->GetPropertyDataOffset("checkObject");
	PropOffsets_ConEventCheckObject.failLabel = cls->GetPropertyDataOffset("failLabel");
}

PropertyOffsets_ConEventCheckPersona PropOffsets_ConEventCheckPersona;

static void InitPropertyOffsets_ConEventCheckPersona(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventCheckPersona"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventCheckPersona, 0xff, sizeof(PropOffsets_ConEventCheckPersona));
		return;
	}
	PropOffsets_ConEventCheckPersona.Value = cls->GetPropertyDataOffset("Value");
	PropOffsets_ConEventCheckPersona.condition = cls->GetPropertyDataOffset("condition");
	PropOffsets_ConEventCheckPersona.jumpLabel = cls->GetPropertyDataOffset("jumpLabel");
	PropOffsets_ConEventCheckPersona.personaType = cls->GetPropertyDataOffset("personaType");
}

PropertyOffsets_ConEventChoice PropOffsets_ConEventChoice;

static void InitPropertyOffsets_ConEventChoice(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventChoice"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventChoice, 0xff, sizeof(PropOffsets_ConEventChoice));
		return;
	}
	PropOffsets_ConEventChoice.ChoiceList = cls->GetPropertyDataOffset("ChoiceList");
	PropOffsets_ConEventChoice.bClearScreen = cls->GetPropertyDataOffset("bClearScreen");
}

PropertyOffsets_ConEventComment PropOffsets_ConEventComment;

static void InitPropertyOffsets_ConEventComment(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventComment"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventComment, 0xff, sizeof(PropOffsets_ConEventComment));
		return;
	}
	PropOffsets_ConEventComment.commentText = cls->GetPropertyDataOffset("commentText");
}

PropertyOffsets_ConEventJump PropOffsets_ConEventJump;

static void InitPropertyOffsets_ConEventJump(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventJump"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventJump, 0xff, sizeof(PropOffsets_ConEventJump));
		return;
	}
	PropOffsets_ConEventJump.conID = cls->GetPropertyDataOffset("conID");
	PropOffsets_ConEventJump.jumpCon = cls->GetPropertyDataOffset("jumpCon");
	PropOffsets_ConEventJump.jumpLabel = cls->GetPropertyDataOffset("jumpLabel");
}

PropertyOffsets_ConEventMoveCamera PropOffsets_ConEventMoveCamera;

static void InitPropertyOffsets_ConEventMoveCamera(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventMoveCamera"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventMoveCamera, 0xff, sizeof(PropOffsets_ConEventMoveCamera));
		return;
	}
	PropOffsets_ConEventMoveCamera.Rotation = cls->GetPropertyDataOffset("Rotation");
	PropOffsets_ConEventMoveCamera.cameraActor = cls->GetPropertyDataOffset("cameraActor");
	PropOffsets_ConEventMoveCamera.cameraActorName = cls->GetPropertyDataOffset("cameraActorName");
	PropOffsets_ConEventMoveCamera.cameraOffset = cls->GetPropertyDataOffset("cameraOffset");
	PropOffsets_ConEventMoveCamera.cameraPosition = cls->GetPropertyDataOffset("cameraPosition");
	PropOffsets_ConEventMoveCamera.cameraTransition = cls->GetPropertyDataOffset("cameraTransition");
	PropOffsets_ConEventMoveCamera.cameraType = cls->GetPropertyDataOffset("cameraType");
	PropOffsets_ConEventMoveCamera.centerModifier = cls->GetPropertyDataOffset("centerModifier");
	PropOffsets_ConEventMoveCamera.distanceMultiplier = cls->GetPropertyDataOffset("distanceMultiplier");
	PropOffsets_ConEventMoveCamera.heightModifier = cls->GetPropertyDataOffset("heightModifier");
}

PropertyOffsets_ConEventSetFlag PropOffsets_ConEventSetFlag;

static void InitPropertyOffsets_ConEventSetFlag(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventSetFlag"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventSetFlag, 0xff, sizeof(PropOffsets_ConEventSetFlag));
		return;
	}
	PropOffsets_ConEventSetFlag.flagRef = cls->GetPropertyDataOffset("flagRef");
}

PropertyOffsets_ConEventSpeech PropOffsets_ConEventSpeech;

static void InitPropertyOffsets_ConEventSpeech(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventSpeech"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventSpeech, 0xff, sizeof(PropOffsets_ConEventSpeech));
		return;
	}
	PropOffsets_ConEventSpeech.ConSpeech = cls->GetPropertyDataOffset("ConSpeech");
	PropOffsets_ConEventSpeech.bBold = cls->GetPropertyDataOffset("bBold");
	PropOffsets_ConEventSpeech.bContinued = cls->GetPropertyDataOffset("bContinued");
	PropOffsets_ConEventSpeech.speaker = cls->GetPropertyDataOffset("speaker");
	PropOffsets_ConEventSpeech.speakerName = cls->GetPropertyDataOffset("speakerName");
	PropOffsets_ConEventSpeech.speakingTo = cls->GetPropertyDataOffset("speakingTo");
	PropOffsets_ConEventSpeech.speakingToName = cls->GetPropertyDataOffset("speakingToName");
	PropOffsets_ConEventSpeech.speechFont = cls->GetPropertyDataOffset("speechFont");
}

PropertyOffsets_ConEventTrade PropOffsets_ConEventTrade;

static void InitPropertyOffsets_ConEventTrade(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventTrade"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventTrade, 0xff, sizeof(PropOffsets_ConEventTrade));
		return;
	}
	PropOffsets_ConEventTrade.eventOwner = cls->GetPropertyDataOffset("eventOwner");
	PropOffsets_ConEventTrade.eventOwnerName = cls->GetPropertyDataOffset("eventOwnerName");
}

PropertyOffsets_ConEventTransferObject PropOffsets_ConEventTransferObject;

static void InitPropertyOffsets_ConEventTransferObject(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventTransferObject"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventTransferObject, 0xff, sizeof(PropOffsets_ConEventTransferObject));
		return;
	}
	PropOffsets_ConEventTransferObject.ObjectName = cls->GetPropertyDataOffset("ObjectName");
	PropOffsets_ConEventTransferObject.failLabel = cls->GetPropertyDataOffset("failLabel");
	PropOffsets_ConEventTransferObject.fromActor = cls->GetPropertyDataOffset("fromActor");
	PropOffsets_ConEventTransferObject.fromName = cls->GetPropertyDataOffset("fromName");
	PropOffsets_ConEventTransferObject.giveObject = cls->GetPropertyDataOffset("giveObject");
	PropOffsets_ConEventTransferObject.toActor = cls->GetPropertyDataOffset("toActor");
	PropOffsets_ConEventTransferObject.toName = cls->GetPropertyDataOffset("toName");
	PropOffsets_ConEventTransferObject.transferCount = cls->GetPropertyDataOffset("transferCount");
}

PropertyOffsets_ConEventTrigger PropOffsets_ConEventTrigger;

static void InitPropertyOffsets_ConEventTrigger(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConEventTrigger"));
	if (!cls)
	{
		memset(&PropOffsets_ConEventTrigger, 0xff, sizeof(PropOffsets_ConEventTrigger));
		return;
	}
	PropOffsets_ConEventTrigger.triggerTag = cls->GetPropertyDataOffset("triggerTag");
}

PropertyOffsets_ConFlagRef PropOffsets_ConFlagRef;

static void InitPropertyOffsets_ConFlagRef(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConFlagRef"));
	if (!cls)
	{
		memset(&PropOffsets_ConFlagRef, 0xff, sizeof(PropOffsets_ConFlagRef));
		return;
	}
	PropOffsets_ConFlagRef.FlagName = cls->GetPropertyDataOffset("FlagName");
	PropOffsets_ConFlagRef.Value = cls->GetPropertyDataOffset("Value");
	PropOffsets_ConFlagRef.expiration = cls->GetPropertyDataOffset("expiration");
	PropOffsets_ConFlagRef.nextFlagRef = cls->GetPropertyDataOffset("nextFlagRef");
}

PropertyOffsets_ConHistory PropOffsets_ConHistory;

static void InitPropertyOffsets_ConHistory(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConHistory"));
	if (!cls)
	{
		memset(&PropOffsets_ConHistory, 0xff, sizeof(PropOffsets_ConHistory));
		return;
	}
	PropOffsets_ConHistory.Next = cls->GetPropertyDataOffset("Next");
	PropOffsets_ConHistory.bInfoLink = cls->GetPropertyDataOffset("bInfoLink");
	PropOffsets_ConHistory.conOwnerName = cls->GetPropertyDataOffset("conOwnerName");
	PropOffsets_ConHistory.firstEvent = cls->GetPropertyDataOffset("firstEvent");
	PropOffsets_ConHistory.lastEvent = cls->GetPropertyDataOffset("lastEvent");
	PropOffsets_ConHistory.strDescription = cls->GetPropertyDataOffset("strDescription");
	PropOffsets_ConHistory.strLocation = cls->GetPropertyDataOffset("strLocation");
}

PropertyOffsets_ConHistoryEvent PropOffsets_ConHistoryEvent;

static void InitPropertyOffsets_ConHistoryEvent(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConHistoryEvent"));
	if (!cls)
	{
		memset(&PropOffsets_ConHistoryEvent, 0xff, sizeof(PropOffsets_ConHistoryEvent));
		return;
	}
	PropOffsets_ConHistoryEvent.Next = cls->GetPropertyDataOffset("Next");
	PropOffsets_ConHistoryEvent.Speech = cls->GetPropertyDataOffset("Speech");
	PropOffsets_ConHistoryEvent.conSpeaker = cls->GetPropertyDataOffset("conSpeaker");
	PropOffsets_ConHistoryEvent.soundID = cls->GetPropertyDataOffset("soundID");
}

PropertyOffsets_ConItem PropOffsets_ConItem;

static void InitPropertyOffsets_ConItem(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConItem"));
	if (!cls)
	{
		memset(&PropOffsets_ConItem, 0xff, sizeof(PropOffsets_ConItem));
		return;
	}
	PropOffsets_ConItem.ConObject = cls->GetPropertyDataOffset("ConObject");
	PropOffsets_ConItem.Next = cls->GetPropertyDataOffset("Next");
}

PropertyOffsets_ConListItem PropOffsets_ConListItem;

static void InitPropertyOffsets_ConListItem(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConListItem"));
	if (!cls)
	{
		memset(&PropOffsets_ConListItem, 0xff, sizeof(PropOffsets_ConListItem));
		return;
	}
	PropOffsets_ConListItem.Next = cls->GetPropertyDataOffset("Next");
	PropOffsets_ConListItem.con = cls->GetPropertyDataOffset("con");
}

PropertyOffsets_ConSpeech PropOffsets_ConSpeech;

static void InitPropertyOffsets_ConSpeech(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConSpeech"));
	if (!cls)
	{
		memset(&PropOffsets_ConSpeech, 0xff, sizeof(PropOffsets_ConSpeech));
		return;
	}
	PropOffsets_ConSpeech.Speech = cls->GetPropertyDataOffset("Speech");
	PropOffsets_ConSpeech.soundID = cls->GetPropertyDataOffset("soundID");
}

PropertyOffsets_ConversationList PropOffsets_ConversationList;

static void InitPropertyOffsets_ConversationList(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConversationList"));
	if (!cls)
	{
		memset(&PropOffsets_ConversationList, 0xff, sizeof(PropOffsets_ConversationList));
		return;
	}
	PropOffsets_ConversationList.conversations = cls->GetPropertyDataOffset("conversations");
	PropOffsets_ConversationList.missionDescription = cls->GetPropertyDataOffset("missionDescription");
	PropOffsets_ConversationList.missionNumber = cls->GetPropertyDataOffset("missionNumber");
}

PropertyOffsets_ConversationMissionList PropOffsets_ConversationMissionList;

static void InitPropertyOffsets_ConversationMissionList(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConversationMissionList"));
	if (!cls)
	{
		memset(&PropOffsets_ConversationMissionList, 0xff, sizeof(PropOffsets_ConversationMissionList));
		return;
	}
	PropOffsets_ConversationMissionList.missions = cls->GetPropertyDataOffset("missions");
}


PropertyOffsets_ConAudioList PropOffsets_ConAudioList;

static void InitPropertyOffsets_ConAudioList(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("ConSys")->GetUObject("Class", "ConAudioList"));
	if (!cls)
	{
		memset(&PropOffsets_ConAudioList, 0xff, sizeof(PropOffsets_ConAudioList));
		return;
	}
	PropOffsets_ConAudioList.ConAudioList = cls->GetPropertyDataOffset("ConAudioList");
	PropOffsets_ConAudioList.audioCount = cls->GetPropertyDataOffset("audioCount");
}

PropertyOffsets_Augmentation PropOffsets_Augmentation;

static void InitPropertyOffsets_Augmentation(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "Augmentation"));
	if (!cls)
	{
		memset(&PropOffsets_Augmentation, 0xff, sizeof(PropOffsets_Augmentation));
		return;
	}
	PropOffsets_Augmentation.ActivateSound = cls->GetPropertyDataOffset("ActivateSound");
	PropOffsets_Augmentation.AlwaysActiveLabel = cls->GetPropertyDataOffset("AlwaysActiveLabel");
	PropOffsets_Augmentation.AugActivated = cls->GetPropertyDataOffset("AugActivated");
	PropOffsets_Augmentation.AugAlreadyHave = cls->GetPropertyDataOffset("AugAlreadyHave");
	PropOffsets_Augmentation.AugDeactivated = cls->GetPropertyDataOffset("AugDeactivated");
	PropOffsets_Augmentation.AugLocsText = cls->GetPropertyDataOffset("AugLocsText");
	PropOffsets_Augmentation.AugNowHave = cls->GetPropertyDataOffset("AugNowHave");
	PropOffsets_Augmentation.AugNowHaveAtLevel = cls->GetPropertyDataOffset("AugNowHaveAtLevel");
	PropOffsets_Augmentation.AugmentationLocation = cls->GetPropertyDataOffset("AugmentationLocation");
	PropOffsets_Augmentation.AugmentationName = cls->GetPropertyDataOffset("AugmentationName");
	PropOffsets_Augmentation.CanUpgradeLabel = cls->GetPropertyDataOffset("CanUpgradeLabel");
	PropOffsets_Augmentation.CurrentLevel = cls->GetPropertyDataOffset("CurrentLevel");
	PropOffsets_Augmentation.CurrentLevelLabel = cls->GetPropertyDataOffset("CurrentLevelLabel");
	PropOffsets_Augmentation.DeActivateSound = cls->GetPropertyDataOffset("DeActivateSound");
	PropOffsets_Augmentation.Description = cls->GetPropertyDataOffset("Description");
	PropOffsets_Augmentation.EnergyRate = cls->GetPropertyDataOffset("EnergyRate");
	PropOffsets_Augmentation.EnergyRateLabel = cls->GetPropertyDataOffset("EnergyRateLabel");
	PropOffsets_Augmentation.HotKeyNum = cls->GetPropertyDataOffset("HotKeyNum");
	PropOffsets_Augmentation.Icon = cls->GetPropertyDataOffset("Icon");
	PropOffsets_Augmentation.IconHeight = cls->GetPropertyDataOffset("IconHeight");
	PropOffsets_Augmentation.IconWidth = cls->GetPropertyDataOffset("IconWidth");
	PropOffsets_Augmentation.LevelValues = cls->GetPropertyDataOffset("LevelValues");
	PropOffsets_Augmentation.LoopSound = cls->GetPropertyDataOffset("LoopSound");
	PropOffsets_Augmentation.MPConflictSlot = cls->GetPropertyDataOffset("MPConflictSlot");
	PropOffsets_Augmentation.MPInfo = cls->GetPropertyDataOffset("MPInfo");
	PropOffsets_Augmentation.MaxLevel = cls->GetPropertyDataOffset("MaxLevel");
	PropOffsets_Augmentation.MaximumLabel = cls->GetPropertyDataOffset("MaximumLabel");
	PropOffsets_Augmentation.Next = cls->GetPropertyDataOffset("Next");
	PropOffsets_Augmentation.OccupiesSlotLabel = cls->GetPropertyDataOffset("OccupiesSlotLabel");
	PropOffsets_Augmentation.Player = cls->GetPropertyDataOffset("Player");
	PropOffsets_Augmentation.bAlwaysActive = cls->GetPropertyDataOffset("bAlwaysActive");
	PropOffsets_Augmentation.bAutomatic = cls->GetPropertyDataOffset("bAutomatic");
	PropOffsets_Augmentation.bBoosted = cls->GetPropertyDataOffset("bBoosted");
	PropOffsets_Augmentation.bHasIt = cls->GetPropertyDataOffset("bHasIt");
	PropOffsets_Augmentation.bIsActive = cls->GetPropertyDataOffset("bIsActive");
	PropOffsets_Augmentation.bUsingMedbot = cls->GetPropertyDataOffset("bUsingMedbot");
	PropOffsets_Augmentation.smallIcon = cls->GetPropertyDataOffset("smallIcon");
}

PropertyOffsets_AugmentationManager PropOffsets_AugmentationManager;

static void InitPropertyOffsets_AugmentationManager(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "AugmentationManager"));
	if (!cls)
	{
		memset(&PropOffsets_AugmentationManager, 0xff, sizeof(PropOffsets_AugmentationManager));
		return;
	}
	PropOffsets_AugmentationManager.AugLocationFull = cls->GetPropertyDataOffset("AugLocationFull");
	PropOffsets_AugmentationManager.AugLocs = cls->GetPropertyDataOffset("AugLocs");
	PropOffsets_AugmentationManager.FirstAug = cls->GetPropertyDataOffset("FirstAug");
	PropOffsets_AugmentationManager.NoAugInSlot = cls->GetPropertyDataOffset("NoAugInSlot");
	PropOffsets_AugmentationManager.Player = cls->GetPropertyDataOffset("Player");
	PropOffsets_AugmentationManager.augClasses = cls->GetPropertyDataOffset("augClasses");
	PropOffsets_AugmentationManager.defaultAugs = cls->GetPropertyDataOffset("defaultAugs");
}

PropertyOffsets_DataVaultImageNote PropOffsets_DataVaultImageNote;

static void InitPropertyOffsets_DataVaultImageNote(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "DataVaultImageNote"));
	if (!cls)
	{
		memset(&PropOffsets_DataVaultImageNote, 0xff, sizeof(PropOffsets_DataVaultImageNote));
		return;
	}
	PropOffsets_DataVaultImageNote.bExpanded = cls->GetPropertyDataOffset("bExpanded");
	PropOffsets_DataVaultImageNote.nextNote = cls->GetPropertyDataOffset("nextNote");
	PropOffsets_DataVaultImageNote.noteText = cls->GetPropertyDataOffset("noteText");
	PropOffsets_DataVaultImageNote.posX = cls->GetPropertyDataOffset("posX");
	PropOffsets_DataVaultImageNote.posY = cls->GetPropertyDataOffset("posY");
}

PropertyOffsets_DeusExLevelInfo PropOffsets_DeusExLevelInfo;

static void InitPropertyOffsets_DeusExLevelInfo(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "DeusExLevelInfo"));
	if (!cls)
	{
		memset(&PropOffsets_DeusExLevelInfo, 0xff, sizeof(PropOffsets_DeusExLevelInfo));
		return;
	}
	PropOffsets_DeusExLevelInfo.ConversationPackage = cls->GetPropertyDataOffset("ConversationPackage");
	PropOffsets_DeusExLevelInfo.MapAuthor = cls->GetPropertyDataOffset("MapAuthor");
	PropOffsets_DeusExLevelInfo.MapName = cls->GetPropertyDataOffset("MapName");
	PropOffsets_DeusExLevelInfo.MissionLocation = cls->GetPropertyDataOffset("MissionLocation");
	PropOffsets_DeusExLevelInfo.Script = cls->GetPropertyDataOffset("Script");
	PropOffsets_DeusExLevelInfo.TrueNorth = cls->GetPropertyDataOffset("TrueNorth");
	PropOffsets_DeusExLevelInfo.bMultiPlayerMap = cls->GetPropertyDataOffset("bMultiPlayerMap");
	PropOffsets_DeusExLevelInfo.missionNumber = cls->GetPropertyDataOffset("missionNumber");
	PropOffsets_DeusExLevelInfo.startupMessage = cls->GetPropertyDataOffset("startupMessage");
}

PropertyOffsets_DeusExLog PropOffsets_DeusExLog;

static void InitPropertyOffsets_DeusExLog(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "DeusExLog"));
	if (!cls)
	{
		memset(&PropOffsets_DeusExLog, 0xff, sizeof(PropOffsets_DeusExLog));
		return;
	}
	PropOffsets_DeusExLog.Next = cls->GetPropertyDataOffset("Next");
	PropOffsets_DeusExLog.Text = cls->GetPropertyDataOffset("Text");
}

PropertyOffsets_LaserIterator PropOffsets_LaserIterator;

static void InitPropertyOffsets_LaserIterator(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "LaserIterator"));
	if (!cls)
	{
		memset(&PropOffsets_LaserIterator, 0xff, sizeof(PropOffsets_LaserIterator));
		return;
	}
	PropOffsets_LaserIterator.Beams = cls->GetPropertyDataOffset("Beams");
	PropOffsets_LaserIterator.NextItem = cls->GetPropertyDataOffset("NextItem");
	PropOffsets_LaserIterator.SavedRot = cls->GetPropertyDataOffset("SavedRot");
	PropOffsets_LaserIterator.bRandomBeam = cls->GetPropertyDataOffset("bRandomBeam");
	PropOffsets_LaserIterator.prevRand = cls->GetPropertyDataOffset("prevRand");
	PropOffsets_LaserIterator.prevloc = cls->GetPropertyDataOffset("prevloc");
	PropOffsets_LaserIterator.proxy = cls->GetPropertyDataOffset("proxy");
	PropOffsets_LaserIterator.savedLoc = cls->GetPropertyDataOffset("savedLoc");
}

PropertyOffsets_Skill PropOffsets_Skill;

static void InitPropertyOffsets_Skill(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "Skill"));
	if (!cls)
	{
		memset(&PropOffsets_Skill, 0xff, sizeof(PropOffsets_Skill));
		return;
	}
	PropOffsets_Skill.CurrentLevel = cls->GetPropertyDataOffset("CurrentLevel");
	PropOffsets_Skill.Description = cls->GetPropertyDataOffset("Description");
	PropOffsets_Skill.LevelValues = cls->GetPropertyDataOffset("LevelValues");
	PropOffsets_Skill.Next = cls->GetPropertyDataOffset("Next");
	PropOffsets_Skill.Player = cls->GetPropertyDataOffset("Player");
	PropOffsets_Skill.SkillAtMaximum = cls->GetPropertyDataOffset("SkillAtMaximum");
	PropOffsets_Skill.SkillIcon = cls->GetPropertyDataOffset("SkillIcon");
	PropOffsets_Skill.SkillName = cls->GetPropertyDataOffset("SkillName");
	PropOffsets_Skill.bAutomatic = cls->GetPropertyDataOffset("bAutomatic");
	PropOffsets_Skill.bConversationBased = cls->GetPropertyDataOffset("bConversationBased");
	PropOffsets_Skill.cost = cls->GetPropertyDataOffset("cost");
	PropOffsets_Skill.itemNeeded = cls->GetPropertyDataOffset("itemNeeded");
	PropOffsets_Skill.skillLevelStrings = cls->GetPropertyDataOffset("skillLevelStrings");
}

PropertyOffsets_SkillManager PropOffsets_SkillManager;

static void InitPropertyOffsets_SkillManager(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("DeusEx")->GetUObject("Class", "SkillManager"));
	if (!cls)
	{
		memset(&PropOffsets_SkillManager, 0xff, sizeof(PropOffsets_SkillManager));
		return;
	}
	PropOffsets_SkillManager.FirstSkill = cls->GetPropertyDataOffset("FirstSkill");
	PropOffsets_SkillManager.NoSkillMessage = cls->GetPropertyDataOffset("NoSkillMessage");
	PropOffsets_SkillManager.NoToolMessage = cls->GetPropertyDataOffset("NoToolMessage");
	PropOffsets_SkillManager.Player = cls->GetPropertyDataOffset("Player");
	PropOffsets_SkillManager.SuccessMessage = cls->GetPropertyDataOffset("SuccessMessage");
	PropOffsets_SkillManager.YourSkillLevelAt = cls->GetPropertyDataOffset("YourSkillLevelAt");
	PropOffsets_SkillManager.skillClasses = cls->GetPropertyDataOffset("skillClasses");
}


PropertyOffsets_Flag PropOffsets_Flag;

static void InitPropertyOffsets_Flag(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "Flag"));
	if (!cls)
	{
		memset(&PropOffsets_Flag, 0xff, sizeof(PropOffsets_Flag));
		return;
	}
	PropOffsets_Flag.FlagBase = cls->GetPropertyDataOffset("FlagBase");
	PropOffsets_Flag.FlagName = cls->GetPropertyDataOffset("FlagName");
	PropOffsets_Flag.expiration = cls->GetPropertyDataOffset("expiration");
	PropOffsets_Flag.flagHash = cls->GetPropertyDataOffset("flagHash");
	PropOffsets_Flag.flagType = cls->GetPropertyDataOffset("flagType");
	PropOffsets_Flag.nextFlag = cls->GetPropertyDataOffset("nextFlag");
}

PropertyOffsets_FlagBool PropOffsets_FlagBool;

static void InitPropertyOffsets_FlagBool(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "FlagBool"));
	if (!cls)
	{
		memset(&PropOffsets_FlagBool, 0xff, sizeof(PropOffsets_FlagBool));
		return;
	}
	PropOffsets_FlagBool.bValue = cls->GetPropertyDataOffset("bValue");
}

PropertyOffsets_FlagByte PropOffsets_FlagByte;

static void InitPropertyOffsets_FlagByte(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "FlagByte"));
	if (!cls)
	{
		memset(&PropOffsets_FlagByte, 0xff, sizeof(PropOffsets_FlagByte));
		return;
	}
	PropOffsets_FlagByte.byteValue = cls->GetPropertyDataOffset("byteValue");
}

PropertyOffsets_FlagFloat PropOffsets_FlagFloat;

static void InitPropertyOffsets_FlagFloat(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "FlagFloat"));
	if (!cls)
	{
		memset(&PropOffsets_FlagFloat, 0xff, sizeof(PropOffsets_FlagFloat));
		return;
	}
	PropOffsets_FlagFloat.floatValue = cls->GetPropertyDataOffset("floatValue");
}

PropertyOffsets_FlagInt PropOffsets_FlagInt;

static void InitPropertyOffsets_FlagInt(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "FlagInt"));
	if (!cls)
	{
		memset(&PropOffsets_FlagInt, 0xff, sizeof(PropOffsets_FlagInt));
		return;
	}
	PropOffsets_FlagInt.intValue = cls->GetPropertyDataOffset("intValue");
}

PropertyOffsets_FlagName PropOffsets_FlagName;

static void InitPropertyOffsets_FlagName(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "FlagName"));
	if (!cls)
	{
		memset(&PropOffsets_FlagName, 0xff, sizeof(PropOffsets_FlagName));
		return;
	}
	PropOffsets_FlagName.nameValue = cls->GetPropertyDataOffset("nameValue");
}

PropertyOffsets_FlagRotator PropOffsets_FlagRotator;

static void InitPropertyOffsets_FlagRotator(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "FlagRotator"));
	if (!cls)
	{
		memset(&PropOffsets_FlagRotator, 0xff, sizeof(PropOffsets_FlagRotator));
		return;
	}
	PropOffsets_FlagRotator.rotatorValue = cls->GetPropertyDataOffset("rotatorValue");
}

PropertyOffsets_FlagVector PropOffsets_FlagVector;

static void InitPropertyOffsets_FlagVector(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "FlagVector"));
	if (!cls)
	{
		memset(&PropOffsets_FlagVector, 0xff, sizeof(PropOffsets_FlagVector));
		return;
	}
	PropOffsets_FlagVector.vectorValue = cls->GetPropertyDataOffset("vectorValue");
}

PropertyOffsets_TabGroupWindow PropOffsets_TabGroupWindow;

static void InitPropertyOffsets_TabGroupWindow(PackageManager* packages)
{
	UClass* cls = UObject::TryCast<UClass>(packages->GetPackage("Extension")->GetUObject("Class", "TabGroupWindow"));
	if (!cls)
	{
		memset(&PropOffsets_TabGroupWindow, 0xff, sizeof(PropOffsets_TabGroupWindow));
		return;
	}
	PropOffsets_TabGroupWindow.bSizeChildrenToParent = cls->GetPropertyDataOffset("bSizeChildrenToParent");
	PropOffsets_TabGroupWindow.bSizeParentToChildren = cls->GetPropertyDataOffset("bSizeParentToChildren");
	PropOffsets_TabGroupWindow.colMajorWindowList = cls->GetPropertyDataOffset("colMajorWindowList");
	PropOffsets_TabGroupWindow.firstAbsX = cls->GetPropertyDataOffset("firstAbsX");
	PropOffsets_TabGroupWindow.firstAbsY = cls->GetPropertyDataOffset("firstAbsY");
	PropOffsets_TabGroupWindow.rowMajorWindowList = cls->GetPropertyDataOffset("rowMajorWindowList");
	PropOffsets_TabGroupWindow.tabGroupIndex = cls->GetPropertyDataOffset("tabGroupIndex");
}

//////////////////////////////////////////

void InitPropertyOffsets(PackageManager* packages)
{
	InitPropertyOffsets_Object(packages);
	InitPropertyOffsets_Commandlet(packages);
	InitPropertyOffsets_Subsystem(packages);
	InitPropertyOffsets_HelloWorldCommandlet(packages);
	InitPropertyOffsets_SimpleCommandlet(packages);
	InitPropertyOffsets_Pawn(packages);
	InitPropertyOffsets_Actor(packages);
	InitPropertyOffsets_LevelInfo(packages);
	InitPropertyOffsets_Inventory(packages);
	InitPropertyOffsets_PlayerPawn(packages);
	InitPropertyOffsets_PlayerReplicationInfo(packages);
	InitPropertyOffsets_Weapon(packages);
	InitPropertyOffsets_GameInfo(packages);
	InitPropertyOffsets_ZoneInfo(packages);
	InitPropertyOffsets_Canvas(packages);
	InitPropertyOffsets_SavedMove(packages);
	InitPropertyOffsets_StatLog(packages);
	InitPropertyOffsets_Texture(packages);
	InitPropertyOffsets_Ammo(packages);
	InitPropertyOffsets_NavigationPoint(packages);
	InitPropertyOffsets_Mutator(packages);
	InitPropertyOffsets_Mover(packages);
	InitPropertyOffsets_HUD(packages);
	InitPropertyOffsets_Decoration(packages);
	InitPropertyOffsets_TestInfo(packages);
	InitPropertyOffsets_GameReplicationInfo(packages);
	InitPropertyOffsets_Menu(packages);
	InitPropertyOffsets_LiftExit(packages);
	InitPropertyOffsets_Trigger(packages);
	InitPropertyOffsets_Player(packages);
	InitPropertyOffsets_LocalMessage(packages);
	InitPropertyOffsets_locationid(packages);
	InitPropertyOffsets_Carcass(packages);
	InitPropertyOffsets_InterpolationPoint(packages);
	InitPropertyOffsets_Projectile(packages);
	InitPropertyOffsets_Teleporter(packages);
	InitPropertyOffsets_Palette(packages);
	InitPropertyOffsets_SpawnNotify(packages);
	InitPropertyOffsets_Fragment(packages);
	InitPropertyOffsets_WarpZoneInfo(packages);
	InitPropertyOffsets_Console(packages);
	InitPropertyOffsets_PlayerStart(packages);
	InitPropertyOffsets_Pickup(packages);
	InitPropertyOffsets_Brush(packages);
	InitPropertyOffsets_ScoreBoard(packages);
	InitPropertyOffsets_Spectator(packages);
	InitPropertyOffsets_InventorySpot(packages);
	InitPropertyOffsets_Decal(packages);
	InitPropertyOffsets_PatrolPoint(packages);
	InitPropertyOffsets_Counter(packages);
	InitPropertyOffsets_Bitmap(packages);
	InitPropertyOffsets_MapList(packages);
	InitPropertyOffsets_Effects(packages);
	InitPropertyOffsets_StatLogFile(packages);
	InitPropertyOffsets_LevelSummary(packages);
	InitPropertyOffsets_FractalTexture(packages);
	InitPropertyOffsets_WaterTexture(packages);
	InitPropertyOffsets_WaveTexture(packages);
	InitPropertyOffsets_FireTexture(packages);
	InitPropertyOffsets_WetTexture(packages);
	InitPropertyOffsets_IceTexture(packages);
	InitPropertyOffsets_ScriptedTexture(packages);
	InitPropertyOffsets_Engine(packages);
	InitPropertyOffsets_TriggerLight(packages);
	InitPropertyOffsets_SpecialEvent(packages);
	InitPropertyOffsets_RoundRobin(packages);
	InitPropertyOffsets_MusicEvent(packages);
	InitPropertyOffsets_HomeBase(packages);
	InitPropertyOffsets_Dispatcher(packages);
	InitPropertyOffsets_DemoRecSpectator(packages);
	InitPropertyOffsets_DamageType(packages);
	InitPropertyOffsets_Ambushpoint(packages);
	InitPropertyOffsets_WarpZoneMarker(packages);
	InitPropertyOffsets_LiftCenter(packages);
	InitPropertyOffsets_RenderIterator(packages);
	InitPropertyOffsets_InternetLink(packages);
	InitPropertyOffsets_UdpLink(packages);
	InitPropertyOffsets_TcpLink(packages);
	if (packages->IsUnreal1())
	{
		InitPropertyOffsets_UPakPathNodeIterator(packages);
		InitPropertyOffsets_UPakPawnPathNodeIterator(packages);
	}
	if (packages->IsDeusEx())
	{
		InitPropertyOffsets_ConEvent(packages);
		InitPropertyOffsets_ConEventRandomLabel(packages);
		InitPropertyOffsets_Conversation(packages);
		InitPropertyOffsets_DeusExDecoration(packages);
		InitPropertyOffsets_DeusExPlayer(packages);
		InitPropertyOffsets_DeusExSaveInfo(packages);
		InitPropertyOffsets_DumpLocation(packages);
		InitPropertyOffsets_GameDirectory(packages);
		InitPropertyOffsets_ParticleIterator(packages);
		InitPropertyOffsets_ScriptedPawn(packages);
		InitPropertyOffsets_DeusExTextParser(packages);
		InitPropertyOffsets_BorderWindow(packages);
		InitPropertyOffsets_ButtonWindow(packages);
		InitPropertyOffsets_CheckboxWindow(packages);
		InitPropertyOffsets_ClipWindow(packages);
		InitPropertyOffsets_ComputerWindow(packages);
		InitPropertyOffsets_EditWindow(packages);
		InitPropertyOffsets_ExtString(packages);
		InitPropertyOffsets_ExtensionObject(packages);
		InitPropertyOffsets_FlagBase(packages);
		InitPropertyOffsets_GC(packages);
		InitPropertyOffsets_LargeTextWindow(packages);
		InitPropertyOffsets_ListWindow(packages);
		InitPropertyOffsets_ModalWindow(packages);
		InitPropertyOffsets_PlayerPawnExt(packages);
		InitPropertyOffsets_RadioBoxWindow(packages);
		InitPropertyOffsets_RootWindow(packages);
		InitPropertyOffsets_ScaleManagerWindow(packages);
		InitPropertyOffsets_ScaleWindow(packages);
		InitPropertyOffsets_ScrollAreaWindow(packages);
		InitPropertyOffsets_TextLogWindow(packages);
		InitPropertyOffsets_TextWindow(packages);
		InitPropertyOffsets_TileWindow(packages);
		InitPropertyOffsets_ToggleWindow(packages);
		InitPropertyOffsets_ViewportWindow(packages);
		InitPropertyOffsets_Window(packages);
		InitPropertyOffsets_ConAudioList(packages);
		InitPropertyOffsets_ConCamera(packages);
		InitPropertyOffsets_ConChoice(packages);
		InitPropertyOffsets_ConEventAddCredits(packages);
		InitPropertyOffsets_ConEventAddGoal(packages);
		InitPropertyOffsets_ConEventAddNote(packages);
		InitPropertyOffsets_ConEventAddSkillPoints(packages);
		InitPropertyOffsets_ConEventAnimation(packages);
		InitPropertyOffsets_ConEventCheckFlag(packages);
		InitPropertyOffsets_ConEventCheckObject(packages);
		InitPropertyOffsets_ConEventCheckPersona(packages);
		InitPropertyOffsets_ConEventChoice(packages);
		InitPropertyOffsets_ConEventComment(packages);
		InitPropertyOffsets_ConEventJump(packages);
		InitPropertyOffsets_ConEventMoveCamera(packages);
		InitPropertyOffsets_ConEventSetFlag(packages);
		InitPropertyOffsets_ConEventSpeech(packages);
		InitPropertyOffsets_ConEventTrade(packages);
		InitPropertyOffsets_ConEventTransferObject(packages);
		InitPropertyOffsets_ConEventTrigger(packages);
		InitPropertyOffsets_ConFlagRef(packages);
		InitPropertyOffsets_ConHistory(packages);
		InitPropertyOffsets_ConHistoryEvent(packages);
		InitPropertyOffsets_ConItem(packages);
		InitPropertyOffsets_ConListItem(packages);
		InitPropertyOffsets_ConSpeech(packages);
		InitPropertyOffsets_ConversationList(packages);
		InitPropertyOffsets_ConversationMissionList(packages);
		InitPropertyOffsets_Augmentation(packages);
		InitPropertyOffsets_AugmentationManager(packages);
		InitPropertyOffsets_DataVaultImageNote(packages);
		InitPropertyOffsets_DeusExLevelInfo(packages);
		InitPropertyOffsets_DeusExLog(packages);
		InitPropertyOffsets_LaserIterator(packages);
		InitPropertyOffsets_Skill(packages);
		InitPropertyOffsets_SkillManager(packages);
		InitPropertyOffsets_Flag(packages);
		InitPropertyOffsets_FlagBool(packages);
		InitPropertyOffsets_FlagByte(packages);
		InitPropertyOffsets_FlagFloat(packages);
		InitPropertyOffsets_FlagInt(packages);
		InitPropertyOffsets_FlagName(packages);
		InitPropertyOffsets_FlagRotator(packages);
		InitPropertyOffsets_FlagVector(packages);
		InitPropertyOffsets_TabGroupWindow(packages);
	}
}
