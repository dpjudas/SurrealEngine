
#include "Precomp.h"
#include "PropertyOffsets.h"
#include "Package/PackageManager.h"
#include "UClass.h"
#include "UProperty.h"

PropertyOffsets_Object PropOffsets_Object;

static void InitPropertyOffsets_Object(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("core")->GetUObject("Class", "Object"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("core")->GetUObject("Class", "Commandlet"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("core")->GetUObject("Class", "Subsystem"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("core")->GetUObject("Class", "HelloWorldCommandlet"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("core")->GetUObject("Class", "SimpleCommandlet"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Pawn"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Actor"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LevelInfo"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Inventory"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "PlayerPawn"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "PlayerReplicationInfo"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Weapon"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "GameInfo"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "ZoneInfo"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Canvas"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "SavedMove"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "StatLog"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Texture"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Ammo"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "NavigationPoint"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Mutator"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Mover"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "HUD"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Decoration"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "TestInfo"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "GameReplicationInfo"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Menu"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LiftExit"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Trigger"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Player"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LocalMessage"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "locationid"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Carcass"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "InterpolationPoint"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Projectile"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Teleporter"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Palette"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "SpawnNotify"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Fragment"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "WarpZoneInfo"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Console"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "PlayerStart"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Pickup"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Brush"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "ScoreBoard"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Spectator"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "InventorySpot"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Decal"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "PatrolPoint"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Counter"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Bitmap"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "MapList"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Effects"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "StatLogFile"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LevelSummary"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "ScriptedTexture"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Engine"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "TriggerLight"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "SpecialEvent"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "RoundRobin"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "MusicEvent"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "HomeBase"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Dispatcher"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "DemoRecSpectator"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "DamageType"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Ambushpoint"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "WarpZoneMarker"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LiftCenter"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "RenderIterator"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("fire")->GetUObject("Class", "FractalTexture"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("fire")->GetUObject("Class", "WaterTexture"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("fire")->GetUObject("Class", "WaveTexture"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("fire")->GetUObject("Class", "FireTexture"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("fire")->GetUObject("Class", "WetTexture"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("fire")->GetUObject("Class", "IceTexture"));
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
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("ipdrv")->GetUObject("Class", "InternetLink"));
	if (!cls)
	{
		memset(&PropOffsets_InternetLink, 0xff, sizeof(PropOffsets_InternetLink));
		return;
	}
	PropOffsets_InternetLink.DataPending = cls->GetProperty("DataPending")->DataOffset;
	PropOffsets_InternetLink.LinkMode = cls->GetProperty("LinkMode")->DataOffset;
	PropOffsets_InternetLink.Port = cls->GetProperty("Port")->DataOffset;
	PropOffsets_InternetLink.PrivateResolveInfo = cls->GetProperty("PrivateResolveInfo")->DataOffset;
	PropOffsets_InternetLink.ReceiveMode = cls->GetProperty("ReceiveMode")->DataOffset;
	PropOffsets_InternetLink.RemoteSocket = cls->GetProperty("RemoteSocket")->DataOffset;
	PropOffsets_InternetLink.Socket = cls->GetProperty("Socket")->DataOffset;
}

PropertyOffsets_UdpLink PropOffsets_UdpLink;

static void InitPropertyOffsets_UdpLink(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("ipdrv")->GetUObject("Class", "UdpLink"));
	if (!cls)
	{
		memset(&PropOffsets_UdpLink, 0xff, sizeof(PropOffsets_UdpLink));
		return;
	}
	PropOffsets_UdpLink.BroadcastAddr = cls->GetProperty("BroadcastAddr")->DataOffset;
}

PropertyOffsets_TcpLink PropOffsets_TcpLink;

static void InitPropertyOffsets_TcpLink(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("ipdrv")->GetUObject("Class", "TcpLink"));
	if (!cls)
	{
		memset(&PropOffsets_TcpLink, 0xff, sizeof(PropOffsets_TcpLink));
		return;
	}

	// AcceptClass and SendFIFO don't seem to exist in Unreal Gold 226 for some reason?
	if (!packages->IsUnreal1_226())
		PropOffsets_TcpLink.AcceptClass = cls->GetProperty("AcceptClass")->DataOffset;
	PropOffsets_TcpLink.LinkState = cls->GetProperty("LinkState")->DataOffset;
	PropOffsets_TcpLink.RemoteAddr = cls->GetProperty("RemoteAddr")->DataOffset;
	if (!packages->IsUnreal1_226())
		PropOffsets_TcpLink.SendFIFO = cls->GetProperty("SendFIFO")->DataOffset;
}

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
	InitPropertyOffsets_FractalTexture(packages);
	InitPropertyOffsets_WaterTexture(packages);
	InitPropertyOffsets_WaveTexture(packages);
	InitPropertyOffsets_FireTexture(packages);
	InitPropertyOffsets_WetTexture(packages);
	InitPropertyOffsets_IceTexture(packages);
	InitPropertyOffsets_InternetLink(packages);
	InitPropertyOffsets_UdpLink(packages);
	InitPropertyOffsets_TcpLink(packages);
}
