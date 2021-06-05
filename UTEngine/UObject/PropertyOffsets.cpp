
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
		throw std::runtime_error("Could not find class object for Object");
	PropOffsets_Object.Class = cls->GetProperty("Class")->DataOffset;
	PropOffsets_Object.Name = cls->GetProperty("Name")->DataOffset;
	PropOffsets_Object.ObjectFlags = cls->GetProperty("ObjectFlags")->DataOffset;
	PropOffsets_Object.ObjectInternal = cls->GetProperty("ObjectInternal")->DataOffset;
	PropOffsets_Object.Outer = cls->GetProperty("Outer")->DataOffset;
}

PropertyOffsets_Commandlet PropOffsets_Commandlet;

static void InitPropertyOffsets_Commandlet(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("core")->GetUObject("Class", "Commandlet"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Commandlet");
	PropOffsets_Commandlet.HelpCmd = cls->GetProperty("HelpCmd")->DataOffset;
	PropOffsets_Commandlet.HelpDesc = cls->GetProperty("HelpDesc")->DataOffset;
	PropOffsets_Commandlet.HelpOneLiner = cls->GetProperty("HelpOneLiner")->DataOffset;
	PropOffsets_Commandlet.HelpParm = cls->GetProperty("HelpParm")->DataOffset;
	PropOffsets_Commandlet.HelpUsage = cls->GetProperty("HelpUsage")->DataOffset;
	PropOffsets_Commandlet.HelpWebLink = cls->GetProperty("HelpWebLink")->DataOffset;
	PropOffsets_Commandlet.IsClient = cls->GetProperty("IsClient")->DataOffset;
	PropOffsets_Commandlet.IsEditor = cls->GetProperty("IsEditor")->DataOffset;
	PropOffsets_Commandlet.IsServer = cls->GetProperty("IsServer")->DataOffset;
	PropOffsets_Commandlet.LazyLoad = cls->GetProperty("LazyLoad")->DataOffset;
	PropOffsets_Commandlet.LogToStdout = cls->GetProperty("LogToStdout")->DataOffset;
	PropOffsets_Commandlet.ShowBanner = cls->GetProperty("ShowBanner")->DataOffset;
	PropOffsets_Commandlet.ShowErrorCount = cls->GetProperty("ShowErrorCount")->DataOffset;
}

PropertyOffsets_Subsystem PropOffsets_Subsystem;

static void InitPropertyOffsets_Subsystem(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("core")->GetUObject("Class", "Subsystem"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Subsystem");
	PropOffsets_Subsystem.ExecVtbl = cls->GetProperty("ExecVtbl")->DataOffset;
}

PropertyOffsets_HelloWorldCommandlet PropOffsets_HelloWorldCommandlet;

static void InitPropertyOffsets_HelloWorldCommandlet(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("core")->GetUObject("Class", "HelloWorldCommandlet"));
	if (!cls)
		throw std::runtime_error("Could not find class object for HelloWorldCommandlet");
	PropOffsets_HelloWorldCommandlet.intparm = cls->GetProperty("intparm")->DataOffset;
	PropOffsets_HelloWorldCommandlet.strparm = cls->GetProperty("strparm")->DataOffset;
}

PropertyOffsets_SimpleCommandlet PropOffsets_SimpleCommandlet;

static void InitPropertyOffsets_SimpleCommandlet(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("core")->GetUObject("Class", "SimpleCommandlet"));
	if (!cls)
		throw std::runtime_error("Could not find class object for SimpleCommandlet");
	PropOffsets_SimpleCommandlet.intparm = cls->GetProperty("intparm")->DataOffset;
}

PropertyOffsets_Pawn PropOffsets_Pawn;

static void InitPropertyOffsets_Pawn(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Pawn"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Pawn");
	PropOffsets_Pawn.AccelRate = cls->GetProperty("AccelRate")->DataOffset;
	PropOffsets_Pawn.AirControl = cls->GetProperty("AirControl")->DataOffset;
	PropOffsets_Pawn.AirSpeed = cls->GetProperty("AirSpeed")->DataOffset;
	PropOffsets_Pawn.AlarmTag = cls->GetProperty("AlarmTag")->DataOffset;
	PropOffsets_Pawn.Alertness = cls->GetProperty("Alertness")->DataOffset;
	PropOffsets_Pawn.AttitudeToPlayer = cls->GetProperty("AttitudeToPlayer")->DataOffset;
	PropOffsets_Pawn.AvgPhysicsTime = cls->GetProperty("AvgPhysicsTime")->DataOffset;
	PropOffsets_Pawn.BaseEyeHeight = cls->GetProperty("BaseEyeHeight")->DataOffset;
	PropOffsets_Pawn.CombatStyle = cls->GetProperty("CombatStyle")->DataOffset;
	PropOffsets_Pawn.DamageScaling = cls->GetProperty("DamageScaling")->DataOffset;
	PropOffsets_Pawn.DesiredSpeed = cls->GetProperty("DesiredSpeed")->DataOffset;
	PropOffsets_Pawn.Destination = cls->GetProperty("Destination")->DataOffset;
	PropOffsets_Pawn.Die = cls->GetProperty("Die")->DataOffset;
	PropOffsets_Pawn.DieCount = cls->GetProperty("DieCount")->DataOffset;
	PropOffsets_Pawn.DropWhenKilled = cls->GetProperty("DropWhenKilled")->DataOffset;
	PropOffsets_Pawn.Enemy = cls->GetProperty("Enemy")->DataOffset;
	PropOffsets_Pawn.EyeHeight = cls->GetProperty("EyeHeight")->DataOffset;
	PropOffsets_Pawn.FaceTarget = cls->GetProperty("FaceTarget")->DataOffset;
	PropOffsets_Pawn.Floor = cls->GetProperty("Floor")->DataOffset;
	PropOffsets_Pawn.Focus = cls->GetProperty("Focus")->DataOffset;
	PropOffsets_Pawn.FootRegion = cls->GetProperty("FootRegion")->DataOffset;
	PropOffsets_Pawn.FovAngle = cls->GetProperty("FovAngle")->DataOffset;
	PropOffsets_Pawn.GroundSpeed = cls->GetProperty("GroundSpeed")->DataOffset;
	PropOffsets_Pawn.HeadRegion = cls->GetProperty("HeadRegion")->DataOffset;
	PropOffsets_Pawn.Health = cls->GetProperty("Health")->DataOffset;
	PropOffsets_Pawn.HearingThreshold = cls->GetProperty("HearingThreshold")->DataOffset;
	PropOffsets_Pawn.HitSound1 = cls->GetProperty("HitSound1")->DataOffset;
	PropOffsets_Pawn.HitSound2 = cls->GetProperty("HitSound2")->DataOffset;
	PropOffsets_Pawn.Intelligence = cls->GetProperty("Intelligence")->DataOffset;
	PropOffsets_Pawn.ItemCount = cls->GetProperty("ItemCount")->DataOffset;
	PropOffsets_Pawn.JumpZ = cls->GetProperty("JumpZ")->DataOffset;
	PropOffsets_Pawn.KillCount = cls->GetProperty("KillCount")->DataOffset;
	PropOffsets_Pawn.Land = cls->GetProperty("Land")->DataOffset;
	PropOffsets_Pawn.LastPainSound = cls->GetProperty("LastPainSound")->DataOffset;
	PropOffsets_Pawn.LastSeeingPos = cls->GetProperty("LastSeeingPos")->DataOffset;
	PropOffsets_Pawn.LastSeenPos = cls->GetProperty("LastSeenPos")->DataOffset;
	PropOffsets_Pawn.LastSeenTime = cls->GetProperty("LastSeenTime")->DataOffset;
	PropOffsets_Pawn.MaxDesiredSpeed = cls->GetProperty("MaxDesiredSpeed")->DataOffset;
	PropOffsets_Pawn.MaxStepHeight = cls->GetProperty("MaxStepHeight")->DataOffset;
	PropOffsets_Pawn.MeleeRange = cls->GetProperty("MeleeRange")->DataOffset;
	PropOffsets_Pawn.MenuName = cls->GetProperty("MenuName")->DataOffset;
	PropOffsets_Pawn.MinHitWall = cls->GetProperty("MinHitWall")->DataOffset;
	PropOffsets_Pawn.MoveTarget = cls->GetProperty("MoveTarget")->DataOffset;
	PropOffsets_Pawn.MoveTimer = cls->GetProperty("MoveTimer")->DataOffset;
	PropOffsets_Pawn.NameArticle = cls->GetProperty("NameArticle")->DataOffset;
	PropOffsets_Pawn.NextLabel = cls->GetProperty("NextLabel")->DataOffset;
	PropOffsets_Pawn.NextState = cls->GetProperty("NextState")->DataOffset;
	PropOffsets_Pawn.OldMessageTime = cls->GetProperty("OldMessageTime")->DataOffset;
	PropOffsets_Pawn.OrthoZoom = cls->GetProperty("OrthoZoom")->DataOffset;
	PropOffsets_Pawn.PainTime = cls->GetProperty("PainTime")->DataOffset;
	PropOffsets_Pawn.PendingWeapon = cls->GetProperty("PendingWeapon")->DataOffset;
	PropOffsets_Pawn.PeripheralVision = cls->GetProperty("PeripheralVision")->DataOffset;
	PropOffsets_Pawn.PlayerReStartState = cls->GetProperty("PlayerReStartState")->DataOffset;
	PropOffsets_Pawn.PlayerReplicationInfo = cls->GetProperty("PlayerReplicationInfo")->DataOffset;
	PropOffsets_Pawn.PlayerReplicationInfoClass = cls->GetProperty("PlayerReplicationInfoClass")->DataOffset;
	PropOffsets_Pawn.ReducedDamagePct = cls->GetProperty("ReducedDamagePct")->DataOffset;
	PropOffsets_Pawn.ReducedDamageType = cls->GetProperty("ReducedDamageType")->DataOffset;
	PropOffsets_Pawn.RouteCache = cls->GetProperty("RouteCache")->DataOffset;
	PropOffsets_Pawn.SecretCount = cls->GetProperty("SecretCount")->DataOffset;
	PropOffsets_Pawn.SelectedItem = cls->GetProperty("SelectedItem")->DataOffset;
	PropOffsets_Pawn.SelectionMesh = cls->GetProperty("SelectionMesh")->DataOffset;
	PropOffsets_Pawn.Shadow = cls->GetProperty("Shadow")->DataOffset;
	PropOffsets_Pawn.SharedAlarmTag = cls->GetProperty("SharedAlarmTag")->DataOffset;
	PropOffsets_Pawn.SightCounter = cls->GetProperty("SightCounter")->DataOffset;
	PropOffsets_Pawn.SightRadius = cls->GetProperty("SightRadius")->DataOffset;
	PropOffsets_Pawn.Skill = cls->GetProperty("Skill")->DataOffset;
	PropOffsets_Pawn.SoundDampening = cls->GetProperty("SoundDampening")->DataOffset;
	PropOffsets_Pawn.SpecialGoal = cls->GetProperty("SpecialGoal")->DataOffset;
	PropOffsets_Pawn.SpecialMesh = cls->GetProperty("SpecialMesh")->DataOffset;
	PropOffsets_Pawn.SpecialPause = cls->GetProperty("SpecialPause")->DataOffset;
	PropOffsets_Pawn.SpeechTime = cls->GetProperty("SpeechTime")->DataOffset;
	PropOffsets_Pawn.SplashTime = cls->GetProperty("SplashTime")->DataOffset;
	PropOffsets_Pawn.Spree = cls->GetProperty("Spree")->DataOffset;
	PropOffsets_Pawn.Stimulus = cls->GetProperty("Stimulus")->DataOffset;
	PropOffsets_Pawn.UnderWaterTime = cls->GetProperty("UnderWaterTime")->DataOffset;
	PropOffsets_Pawn.ViewRotation = cls->GetProperty("ViewRotation")->DataOffset;
	PropOffsets_Pawn.Visibility = cls->GetProperty("Visibility")->DataOffset;
	PropOffsets_Pawn.VoicePitch = cls->GetProperty("VoicePitch")->DataOffset;
	PropOffsets_Pawn.VoiceType = cls->GetProperty("VoiceType")->DataOffset;
	PropOffsets_Pawn.WalkBob = cls->GetProperty("WalkBob")->DataOffset;
	PropOffsets_Pawn.WaterSpeed = cls->GetProperty("WaterSpeed")->DataOffset;
	PropOffsets_Pawn.WaterStep = cls->GetProperty("WaterStep")->DataOffset;
	PropOffsets_Pawn.Weapon = cls->GetProperty("Weapon")->DataOffset;
	PropOffsets_Pawn.bAdvancedTactics = cls->GetProperty("bAdvancedTactics")->DataOffset;
	PropOffsets_Pawn.bAltFire = cls->GetProperty("bAltFire")->DataOffset;
	PropOffsets_Pawn.bAutoActivate = cls->GetProperty("bAutoActivate")->DataOffset;
	PropOffsets_Pawn.bAvoidLedges = cls->GetProperty("bAvoidLedges")->DataOffset;
	PropOffsets_Pawn.bBehindView = cls->GetProperty("bBehindView")->DataOffset;
	PropOffsets_Pawn.bCanDoSpecial = cls->GetProperty("bCanDoSpecial")->DataOffset;
	PropOffsets_Pawn.bCanFly = cls->GetProperty("bCanFly")->DataOffset;
	PropOffsets_Pawn.bCanJump = cls->GetProperty("bCanJump")->DataOffset;
	PropOffsets_Pawn.bCanOpenDoors = cls->GetProperty("bCanOpenDoors")->DataOffset;
	PropOffsets_Pawn.bCanStrafe = cls->GetProperty("bCanStrafe")->DataOffset;
	PropOffsets_Pawn.bCanSwim = cls->GetProperty("bCanSwim")->DataOffset;
	PropOffsets_Pawn.bCanWalk = cls->GetProperty("bCanWalk")->DataOffset;
	PropOffsets_Pawn.bCountJumps = cls->GetProperty("bCountJumps")->DataOffset;
	PropOffsets_Pawn.bDrowning = cls->GetProperty("bDrowning")->DataOffset;
	PropOffsets_Pawn.bDuck = cls->GetProperty("bDuck")->DataOffset;
	PropOffsets_Pawn.bExtra0 = cls->GetProperty("bExtra0")->DataOffset;
	PropOffsets_Pawn.bExtra1 = cls->GetProperty("bExtra1")->DataOffset;
	PropOffsets_Pawn.bExtra2 = cls->GetProperty("bExtra2")->DataOffset;
	PropOffsets_Pawn.bExtra3 = cls->GetProperty("bExtra3")->DataOffset;
	PropOffsets_Pawn.bFire = cls->GetProperty("bFire")->DataOffset;
	PropOffsets_Pawn.bFixedStart = cls->GetProperty("bFixedStart")->DataOffset;
	PropOffsets_Pawn.bFreeLook = cls->GetProperty("bFreeLook")->DataOffset;
	PropOffsets_Pawn.bFromWall = cls->GetProperty("bFromWall")->DataOffset;
	PropOffsets_Pawn.bHitSlopedWall = cls->GetProperty("bHitSlopedWall")->DataOffset;
	PropOffsets_Pawn.bHunting = cls->GetProperty("bHunting")->DataOffset;
	PropOffsets_Pawn.bIsFemale = cls->GetProperty("bIsFemale")->DataOffset;
	PropOffsets_Pawn.bIsHuman = cls->GetProperty("bIsHuman")->DataOffset;
	PropOffsets_Pawn.bIsMultiSkinned = cls->GetProperty("bIsMultiSkinned")->DataOffset;
	PropOffsets_Pawn.bIsPlayer = cls->GetProperty("bIsPlayer")->DataOffset;
	PropOffsets_Pawn.bIsWalking = cls->GetProperty("bIsWalking")->DataOffset;
	PropOffsets_Pawn.bJumpOffPawn = cls->GetProperty("bJumpOffPawn")->DataOffset;
	PropOffsets_Pawn.bJustLanded = cls->GetProperty("bJustLanded")->DataOffset;
	PropOffsets_Pawn.bLOSflag = cls->GetProperty("bLOSflag")->DataOffset;
	PropOffsets_Pawn.bLook = cls->GetProperty("bLook")->DataOffset;
	PropOffsets_Pawn.bNeverSwitchOnPickup = cls->GetProperty("bNeverSwitchOnPickup")->DataOffset;
	PropOffsets_Pawn.bReducedSpeed = cls->GetProperty("bReducedSpeed")->DataOffset;
	PropOffsets_Pawn.bRun = cls->GetProperty("bRun")->DataOffset;
	PropOffsets_Pawn.bShootSpecial = cls->GetProperty("bShootSpecial")->DataOffset;
	PropOffsets_Pawn.bSnapLevel = cls->GetProperty("bSnapLevel")->DataOffset;
	PropOffsets_Pawn.bStopAtLedges = cls->GetProperty("bStopAtLedges")->DataOffset;
	PropOffsets_Pawn.bStrafe = cls->GetProperty("bStrafe")->DataOffset;
	PropOffsets_Pawn.bUpAndOut = cls->GetProperty("bUpAndOut")->DataOffset;
	PropOffsets_Pawn.bUpdatingDisplay = cls->GetProperty("bUpdatingDisplay")->DataOffset;
	PropOffsets_Pawn.bViewTarget = cls->GetProperty("bViewTarget")->DataOffset;
	PropOffsets_Pawn.bWarping = cls->GetProperty("bWarping")->DataOffset;
	PropOffsets_Pawn.bZoom = cls->GetProperty("bZoom")->DataOffset;
	PropOffsets_Pawn.carriedDecoration = cls->GetProperty("carriedDecoration")->DataOffset;
	PropOffsets_Pawn.home = cls->GetProperty("home")->DataOffset;
	PropOffsets_Pawn.nextPawn = cls->GetProperty("nextPawn")->DataOffset;
	PropOffsets_Pawn.noise1loudness = cls->GetProperty("noise1loudness")->DataOffset;
	PropOffsets_Pawn.noise1other = cls->GetProperty("noise1other")->DataOffset;
	PropOffsets_Pawn.noise1spot = cls->GetProperty("noise1spot")->DataOffset;
	PropOffsets_Pawn.noise1time = cls->GetProperty("noise1time")->DataOffset;
	PropOffsets_Pawn.noise2loudness = cls->GetProperty("noise2loudness")->DataOffset;
	PropOffsets_Pawn.noise2other = cls->GetProperty("noise2other")->DataOffset;
	PropOffsets_Pawn.noise2spot = cls->GetProperty("noise2spot")->DataOffset;
	PropOffsets_Pawn.noise2time = cls->GetProperty("noise2time")->DataOffset;
}

PropertyOffsets_Actor PropOffsets_Actor;

static void InitPropertyOffsets_Actor(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Actor"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Actor");
	PropOffsets_Actor.Acceleration = cls->GetProperty("Acceleration")->DataOffset;
	PropOffsets_Actor.AmbientGlow = cls->GetProperty("AmbientGlow")->DataOffset;
	PropOffsets_Actor.AmbientSound = cls->GetProperty("AmbientSound")->DataOffset;
	PropOffsets_Actor.AnimFrame = cls->GetProperty("AnimFrame")->DataOffset;
	PropOffsets_Actor.AnimLast = cls->GetProperty("AnimLast")->DataOffset;
	PropOffsets_Actor.AnimMinRate = cls->GetProperty("AnimMinRate")->DataOffset;
	PropOffsets_Actor.AnimRate = cls->GetProperty("AnimRate")->DataOffset;
	PropOffsets_Actor.AnimSequence = cls->GetProperty("AnimSequence")->DataOffset;
	PropOffsets_Actor.AttachTag = cls->GetProperty("AttachTag")->DataOffset;
	PropOffsets_Actor.Base = cls->GetProperty("Base")->DataOffset;
	PropOffsets_Actor.Brush = cls->GetProperty("Brush")->DataOffset;
	PropOffsets_Actor.Buoyancy = cls->GetProperty("Buoyancy")->DataOffset;
	PropOffsets_Actor.ColLocation = cls->GetProperty("ColLocation")->DataOffset;
	PropOffsets_Actor.CollisionHeight = cls->GetProperty("CollisionHeight")->DataOffset;
	PropOffsets_Actor.CollisionRadius = cls->GetProperty("CollisionRadius")->DataOffset;
	PropOffsets_Actor.CollisionTag = cls->GetProperty("CollisionTag")->DataOffset;
	PropOffsets_Actor.Deleted = cls->GetProperty("Deleted")->DataOffset;
	PropOffsets_Actor.DesiredRotation = cls->GetProperty("DesiredRotation")->DataOffset;
	PropOffsets_Actor.DodgeDir = cls->GetProperty("DodgeDir")->DataOffset;
	PropOffsets_Actor.DrawScale = cls->GetProperty("DrawScale")->DataOffset;
	PropOffsets_Actor.DrawType = cls->GetProperty("DrawType")->DataOffset;
	PropOffsets_Actor.Event = cls->GetProperty("Event")->DataOffset;
	PropOffsets_Actor.ExtraTag = cls->GetProperty("ExtraTag")->DataOffset;
	PropOffsets_Actor.Fatness = cls->GetProperty("Fatness")->DataOffset;
	PropOffsets_Actor.Group = cls->GetProperty("Group")->DataOffset;
	PropOffsets_Actor.HitActor = cls->GetProperty("HitActor")->DataOffset;
	PropOffsets_Actor.InitialState = cls->GetProperty("InitialState")->DataOffset;
	PropOffsets_Actor.Instigator = cls->GetProperty("Instigator")->DataOffset;
	PropOffsets_Actor.Inventory = cls->GetProperty("Inventory")->DataOffset;
	PropOffsets_Actor.LODBias = cls->GetProperty("LODBias")->DataOffset;
	PropOffsets_Actor.LatentActor = cls->GetProperty("LatentActor")->DataOffset;
	PropOffsets_Actor.LatentByte = cls->GetProperty("LatentByte")->DataOffset;
	PropOffsets_Actor.LatentFloat = cls->GetProperty("LatentFloat")->DataOffset;
	PropOffsets_Actor.LatentInt = cls->GetProperty("LatentInt")->DataOffset;
	PropOffsets_Actor.Level = cls->GetProperty("Level")->DataOffset;
	PropOffsets_Actor.LifeSpan = cls->GetProperty("LifeSpan")->DataOffset;
	PropOffsets_Actor.LightBrightness = cls->GetProperty("LightBrightness")->DataOffset;
	PropOffsets_Actor.LightCone = cls->GetProperty("LightCone")->DataOffset;
	PropOffsets_Actor.LightEffect = cls->GetProperty("LightEffect")->DataOffset;
	PropOffsets_Actor.LightHue = cls->GetProperty("LightHue")->DataOffset;
	PropOffsets_Actor.LightPeriod = cls->GetProperty("LightPeriod")->DataOffset;
	PropOffsets_Actor.LightPhase = cls->GetProperty("LightPhase")->DataOffset;
	PropOffsets_Actor.LightRadius = cls->GetProperty("LightRadius")->DataOffset;
	PropOffsets_Actor.LightSaturation = cls->GetProperty("LightSaturation")->DataOffset;
	PropOffsets_Actor.LightType = cls->GetProperty("LightType")->DataOffset;
	PropOffsets_Actor.LightingTag = cls->GetProperty("LightingTag")->DataOffset;
	PropOffsets_Actor.Location = cls->GetProperty("Location")->DataOffset;
	PropOffsets_Actor.Mass = cls->GetProperty("Mass")->DataOffset;
	PropOffsets_Actor.Mesh = cls->GetProperty("Mesh")->DataOffset;
	PropOffsets_Actor.MiscNumber = cls->GetProperty("MiscNumber")->DataOffset;
	PropOffsets_Actor.MultiSkins = cls->GetProperty("MultiSkins")->DataOffset;
	PropOffsets_Actor.NetPriority = cls->GetProperty("NetPriority")->DataOffset;
	PropOffsets_Actor.NetTag = cls->GetProperty("NetTag")->DataOffset;
	PropOffsets_Actor.NetUpdateFrequency = cls->GetProperty("NetUpdateFrequency")->DataOffset;
	PropOffsets_Actor.OddsOfAppearing = cls->GetProperty("OddsOfAppearing")->DataOffset;
	PropOffsets_Actor.OldAnimRate = cls->GetProperty("OldAnimRate")->DataOffset;
	PropOffsets_Actor.OldLocation = cls->GetProperty("OldLocation")->DataOffset;
	PropOffsets_Actor.OtherTag = cls->GetProperty("OtherTag")->DataOffset;
	PropOffsets_Actor.Owner = cls->GetProperty("Owner")->DataOffset;
	PropOffsets_Actor.PendingTouch = cls->GetProperty("PendingTouch")->DataOffset;
	PropOffsets_Actor.PhysAlpha = cls->GetProperty("PhysAlpha")->DataOffset;
	PropOffsets_Actor.PhysRate = cls->GetProperty("PhysRate")->DataOffset;
	PropOffsets_Actor.Physics = cls->GetProperty("Physics")->DataOffset;
	PropOffsets_Actor.PrePivot = cls->GetProperty("PrePivot")->DataOffset;
	PropOffsets_Actor.Region = cls->GetProperty("Region")->DataOffset;
	PropOffsets_Actor.RemoteRole = cls->GetProperty("RemoteRole")->DataOffset;
	PropOffsets_Actor.RenderInterface = cls->GetProperty("RenderInterface")->DataOffset;
	PropOffsets_Actor.RenderIteratorClass = cls->GetProperty("RenderIteratorClass")->DataOffset;
	PropOffsets_Actor.Role = cls->GetProperty("Role")->DataOffset;
	PropOffsets_Actor.Rotation = cls->GetProperty("Rotation")->DataOffset;
	PropOffsets_Actor.RotationRate = cls->GetProperty("RotationRate")->DataOffset;
	PropOffsets_Actor.ScaleGlow = cls->GetProperty("ScaleGlow")->DataOffset;
	PropOffsets_Actor.SimAnim = cls->GetProperty("SimAnim")->DataOffset;
	PropOffsets_Actor.SkelAnim = cls->GetProperty("SkelAnim")->DataOffset;
	PropOffsets_Actor.Skin = cls->GetProperty("Skin")->DataOffset;
	PropOffsets_Actor.SoundPitch = cls->GetProperty("SoundPitch")->DataOffset;
	PropOffsets_Actor.SoundRadius = cls->GetProperty("SoundRadius")->DataOffset;
	PropOffsets_Actor.SoundVolume = cls->GetProperty("SoundVolume")->DataOffset;
	PropOffsets_Actor.SpecialTag = cls->GetProperty("SpecialTag")->DataOffset;
	PropOffsets_Actor.Sprite = cls->GetProperty("Sprite")->DataOffset;
	PropOffsets_Actor.SpriteProjForward = cls->GetProperty("SpriteProjForward")->DataOffset;
	PropOffsets_Actor.StandingCount = cls->GetProperty("StandingCount")->DataOffset;
	PropOffsets_Actor.Style = cls->GetProperty("Style")->DataOffset;
	PropOffsets_Actor.Tag = cls->GetProperty("Tag")->DataOffset;
	PropOffsets_Actor.Target = cls->GetProperty("Target")->DataOffset;
	PropOffsets_Actor.Texture = cls->GetProperty("Texture")->DataOffset;
	PropOffsets_Actor.TimerCounter = cls->GetProperty("TimerCounter")->DataOffset;
	PropOffsets_Actor.TimerRate = cls->GetProperty("TimerRate")->DataOffset;
	PropOffsets_Actor.Touching = cls->GetProperty("Touching")->DataOffset;
	PropOffsets_Actor.TransientSoundRadius = cls->GetProperty("TransientSoundRadius")->DataOffset;
	PropOffsets_Actor.TransientSoundVolume = cls->GetProperty("TransientSoundVolume")->DataOffset;
	PropOffsets_Actor.TweenRate = cls->GetProperty("TweenRate")->DataOffset;
	PropOffsets_Actor.Velocity = cls->GetProperty("Velocity")->DataOffset;
	PropOffsets_Actor.VisibilityHeight = cls->GetProperty("VisibilityHeight")->DataOffset;
	PropOffsets_Actor.VisibilityRadius = cls->GetProperty("VisibilityRadius")->DataOffset;
	PropOffsets_Actor.VolumeBrightness = cls->GetProperty("VolumeBrightness")->DataOffset;
	PropOffsets_Actor.VolumeFog = cls->GetProperty("VolumeFog")->DataOffset;
	PropOffsets_Actor.VolumeRadius = cls->GetProperty("VolumeRadius")->DataOffset;
	PropOffsets_Actor.XLevel = cls->GetProperty("XLevel")->DataOffset;
	PropOffsets_Actor.bActorShadows = cls->GetProperty("bActorShadows")->DataOffset;
	PropOffsets_Actor.bAlwaysRelevant = cls->GetProperty("bAlwaysRelevant")->DataOffset;
	PropOffsets_Actor.bAlwaysTick = cls->GetProperty("bAlwaysTick")->DataOffset;
	PropOffsets_Actor.bAnimByOwner = cls->GetProperty("bAnimByOwner")->DataOffset;
	PropOffsets_Actor.bAnimFinished = cls->GetProperty("bAnimFinished")->DataOffset;
	PropOffsets_Actor.bAnimLoop = cls->GetProperty("bAnimLoop")->DataOffset;
	PropOffsets_Actor.bAnimNotify = cls->GetProperty("bAnimNotify")->DataOffset;
	PropOffsets_Actor.bAssimilated = cls->GetProperty("bAssimilated")->DataOffset;
	PropOffsets_Actor.bBlockActors = cls->GetProperty("bBlockActors")->DataOffset;
	PropOffsets_Actor.bBlockPlayers = cls->GetProperty("bBlockPlayers")->DataOffset;
	PropOffsets_Actor.bBounce = cls->GetProperty("bBounce")->DataOffset;
	PropOffsets_Actor.bCanTeleport = cls->GetProperty("bCanTeleport")->DataOffset;
	PropOffsets_Actor.bCarriedItem = cls->GetProperty("bCarriedItem")->DataOffset;
	PropOffsets_Actor.bClientAnim = cls->GetProperty("bClientAnim")->DataOffset;
	PropOffsets_Actor.bClientDemoNetFunc = cls->GetProperty("bClientDemoNetFunc")->DataOffset;
	PropOffsets_Actor.bClientDemoRecording = cls->GetProperty("bClientDemoRecording")->DataOffset;
	PropOffsets_Actor.bCollideActors = cls->GetProperty("bCollideActors")->DataOffset;
	PropOffsets_Actor.bCollideWhenPlacing = cls->GetProperty("bCollideWhenPlacing")->DataOffset;
	PropOffsets_Actor.bCollideWorld = cls->GetProperty("bCollideWorld")->DataOffset;
	PropOffsets_Actor.bCorona = cls->GetProperty("bCorona")->DataOffset;
	PropOffsets_Actor.bDeleteMe = cls->GetProperty("bDeleteMe")->DataOffset;
	PropOffsets_Actor.bDemoRecording = cls->GetProperty("bDemoRecording")->DataOffset;
	PropOffsets_Actor.bDifficulty0 = cls->GetProperty("bDifficulty0")->DataOffset;
	PropOffsets_Actor.bDifficulty1 = cls->GetProperty("bDifficulty1")->DataOffset;
	PropOffsets_Actor.bDifficulty2 = cls->GetProperty("bDifficulty2")->DataOffset;
	PropOffsets_Actor.bDifficulty3 = cls->GetProperty("bDifficulty3")->DataOffset;
	PropOffsets_Actor.bDirectional = cls->GetProperty("bDirectional")->DataOffset;
	PropOffsets_Actor.bDynamicLight = cls->GetProperty("bDynamicLight")->DataOffset;
	PropOffsets_Actor.bEdLocked = cls->GetProperty("bEdLocked")->DataOffset;
	PropOffsets_Actor.bEdShouldSnap = cls->GetProperty("bEdShouldSnap")->DataOffset;
	PropOffsets_Actor.bEdSnap = cls->GetProperty("bEdSnap")->DataOffset;
	PropOffsets_Actor.bFilterByVolume = cls->GetProperty("bFilterByVolume")->DataOffset;
	PropOffsets_Actor.bFixedRotationDir = cls->GetProperty("bFixedRotationDir")->DataOffset;
	PropOffsets_Actor.bForcePhysicsUpdate = cls->GetProperty("bForcePhysicsUpdate")->DataOffset;
	PropOffsets_Actor.bForceStasis = cls->GetProperty("bForceStasis")->DataOffset;
	PropOffsets_Actor.bGameRelevant = cls->GetProperty("bGameRelevant")->DataOffset;
	PropOffsets_Actor.bHidden = cls->GetProperty("bHidden")->DataOffset;
	PropOffsets_Actor.bHiddenEd = cls->GetProperty("bHiddenEd")->DataOffset;
	PropOffsets_Actor.bHighDetail = cls->GetProperty("bHighDetail")->DataOffset;
	PropOffsets_Actor.bHighlighted = cls->GetProperty("bHighlighted")->DataOffset;
	PropOffsets_Actor.bHurtEntry = cls->GetProperty("bHurtEntry")->DataOffset;
	PropOffsets_Actor.bInterpolating = cls->GetProperty("bInterpolating")->DataOffset;
	PropOffsets_Actor.bIsItemGoal = cls->GetProperty("bIsItemGoal")->DataOffset;
	PropOffsets_Actor.bIsKillGoal = cls->GetProperty("bIsKillGoal")->DataOffset;
	PropOffsets_Actor.bIsMover = cls->GetProperty("bIsMover")->DataOffset;
	PropOffsets_Actor.bIsPawn = cls->GetProperty("bIsPawn")->DataOffset;
	PropOffsets_Actor.bIsSecretGoal = cls->GetProperty("bIsSecretGoal")->DataOffset;
	PropOffsets_Actor.bJustTeleported = cls->GetProperty("bJustTeleported")->DataOffset;
	PropOffsets_Actor.bLensFlare = cls->GetProperty("bLensFlare")->DataOffset;
	PropOffsets_Actor.bLightChanged = cls->GetProperty("bLightChanged")->DataOffset;
	PropOffsets_Actor.bMemorized = cls->GetProperty("bMemorized")->DataOffset;
	PropOffsets_Actor.bMeshCurvy = cls->GetProperty("bMeshCurvy")->DataOffset;
	PropOffsets_Actor.bMeshEnviroMap = cls->GetProperty("bMeshEnviroMap")->DataOffset;
	PropOffsets_Actor.bMovable = cls->GetProperty("bMovable")->DataOffset;
	PropOffsets_Actor.bNet = cls->GetProperty("bNet")->DataOffset;
	PropOffsets_Actor.bNetFeel = cls->GetProperty("bNetFeel")->DataOffset;
	PropOffsets_Actor.bNetHear = cls->GetProperty("bNetHear")->DataOffset;
	PropOffsets_Actor.bNetInitial = cls->GetProperty("bNetInitial")->DataOffset;
	PropOffsets_Actor.bNetOptional = cls->GetProperty("bNetOptional")->DataOffset;
	PropOffsets_Actor.bNetOwner = cls->GetProperty("bNetOwner")->DataOffset;
	PropOffsets_Actor.bNetRelevant = cls->GetProperty("bNetRelevant")->DataOffset;
	PropOffsets_Actor.bNetSee = cls->GetProperty("bNetSee")->DataOffset;
	PropOffsets_Actor.bNetSpecial = cls->GetProperty("bNetSpecial")->DataOffset;
	PropOffsets_Actor.bNetTemporary = cls->GetProperty("bNetTemporary")->DataOffset;
	PropOffsets_Actor.bNoDelete = cls->GetProperty("bNoDelete")->DataOffset;
	PropOffsets_Actor.bNoSmooth = cls->GetProperty("bNoSmooth")->DataOffset;
	PropOffsets_Actor.bOnlyOwnerSee = cls->GetProperty("bOnlyOwnerSee")->DataOffset;
	PropOffsets_Actor.bOwnerNoSee = cls->GetProperty("bOwnerNoSee")->DataOffset;
	PropOffsets_Actor.bParticles = cls->GetProperty("bParticles")->DataOffset;
	PropOffsets_Actor.bProjTarget = cls->GetProperty("bProjTarget")->DataOffset;
	PropOffsets_Actor.bRandomFrame = cls->GetProperty("bRandomFrame")->DataOffset;
	PropOffsets_Actor.bReplicateInstigator = cls->GetProperty("bReplicateInstigator")->DataOffset;
	PropOffsets_Actor.bRotateToDesired = cls->GetProperty("bRotateToDesired")->DataOffset;
	PropOffsets_Actor.bScriptInitialized = cls->GetProperty("bScriptInitialized")->DataOffset;
	PropOffsets_Actor.bSelected = cls->GetProperty("bSelected")->DataOffset;
	PropOffsets_Actor.bShadowCast = cls->GetProperty("bShadowCast")->DataOffset;
	PropOffsets_Actor.bSimFall = cls->GetProperty("bSimFall")->DataOffset;
	PropOffsets_Actor.bSimulatedPawn = cls->GetProperty("bSimulatedPawn")->DataOffset;
	PropOffsets_Actor.bSinglePlayer = cls->GetProperty("bSinglePlayer")->DataOffset;
	PropOffsets_Actor.bSpecialLit = cls->GetProperty("bSpecialLit")->DataOffset;
	PropOffsets_Actor.bStasis = cls->GetProperty("bStasis")->DataOffset;
	PropOffsets_Actor.bStatic = cls->GetProperty("bStatic")->DataOffset;
	PropOffsets_Actor.bTempEditor = cls->GetProperty("bTempEditor")->DataOffset;
	PropOffsets_Actor.bTicked = cls->GetProperty("bTicked")->DataOffset;
	PropOffsets_Actor.bTimerLoop = cls->GetProperty("bTimerLoop")->DataOffset;
	PropOffsets_Actor.bTrailerPrePivot = cls->GetProperty("bTrailerPrePivot")->DataOffset;
	PropOffsets_Actor.bTrailerSameRotation = cls->GetProperty("bTrailerSameRotation")->DataOffset;
	PropOffsets_Actor.bTravel = cls->GetProperty("bTravel")->DataOffset;
	PropOffsets_Actor.bUnlit = cls->GetProperty("bUnlit")->DataOffset;
}

PropertyOffsets_LevelInfo PropOffsets_LevelInfo;

static void InitPropertyOffsets_LevelInfo(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LevelInfo"));
	if (!cls)
		throw std::runtime_error("Could not find class object for LevelInfo");
	PropOffsets_LevelInfo.AIProfile = cls->GetProperty("AIProfile")->DataOffset;
	PropOffsets_LevelInfo.Author = cls->GetProperty("Author")->DataOffset;
	PropOffsets_LevelInfo.AvgAITime = cls->GetProperty("AvgAITime")->DataOffset;
	PropOffsets_LevelInfo.Brightness = cls->GetProperty("Brightness")->DataOffset;
	PropOffsets_LevelInfo.CdTrack = cls->GetProperty("CdTrack")->DataOffset;
	PropOffsets_LevelInfo.ComputerName = cls->GetProperty("ComputerName")->DataOffset;
	PropOffsets_LevelInfo.Day = cls->GetProperty("Day")->DataOffset;
	PropOffsets_LevelInfo.DayOfWeek = cls->GetProperty("DayOfWeek")->DataOffset;
	PropOffsets_LevelInfo.DefaultGameType = cls->GetProperty("DefaultGameType")->DataOffset;
	PropOffsets_LevelInfo.DefaultTexture = cls->GetProperty("DefaultTexture")->DataOffset;
	PropOffsets_LevelInfo.EngineVersion = cls->GetProperty("EngineVersion")->DataOffset;
	PropOffsets_LevelInfo.Game = cls->GetProperty("Game")->DataOffset;
	PropOffsets_LevelInfo.Hour = cls->GetProperty("Hour")->DataOffset;
	PropOffsets_LevelInfo.HubStackLevel = cls->GetProperty("HubStackLevel")->DataOffset;
	PropOffsets_LevelInfo.IdealPlayerCount = cls->GetProperty("IdealPlayerCount")->DataOffset;
	PropOffsets_LevelInfo.LevelAction = cls->GetProperty("LevelAction")->DataOffset;
	PropOffsets_LevelInfo.LevelEnterText = cls->GetProperty("LevelEnterText")->DataOffset;
	PropOffsets_LevelInfo.LocalizedPkg = cls->GetProperty("LocalizedPkg")->DataOffset;
	PropOffsets_LevelInfo.Millisecond = cls->GetProperty("Millisecond")->DataOffset;
	PropOffsets_LevelInfo.MinNetVersion = cls->GetProperty("MinNetVersion")->DataOffset;
	PropOffsets_LevelInfo.Minute = cls->GetProperty("Minute")->DataOffset;
	PropOffsets_LevelInfo.Month = cls->GetProperty("Month")->DataOffset;
	PropOffsets_LevelInfo.NavigationPointList = cls->GetProperty("NavigationPointList")->DataOffset;
	PropOffsets_LevelInfo.NetMode = cls->GetProperty("NetMode")->DataOffset;
	PropOffsets_LevelInfo.NextSwitchCountdown = cls->GetProperty("NextSwitchCountdown")->DataOffset;
	PropOffsets_LevelInfo.NextURL = cls->GetProperty("NextURL")->DataOffset;
	PropOffsets_LevelInfo.Pauser = cls->GetProperty("Pauser")->DataOffset;
	PropOffsets_LevelInfo.PawnList = cls->GetProperty("PawnList")->DataOffset;
	PropOffsets_LevelInfo.PlayerDoppler = cls->GetProperty("PlayerDoppler")->DataOffset;
	PropOffsets_LevelInfo.RecommendedEnemies = cls->GetProperty("RecommendedEnemies")->DataOffset;
	PropOffsets_LevelInfo.RecommendedTeammates = cls->GetProperty("RecommendedTeammates")->DataOffset;
	PropOffsets_LevelInfo.Screenshot = cls->GetProperty("Screenshot")->DataOffset;
	PropOffsets_LevelInfo.Second = cls->GetProperty("Second")->DataOffset;
	PropOffsets_LevelInfo.Song = cls->GetProperty("Song")->DataOffset;
	PropOffsets_LevelInfo.SongSection = cls->GetProperty("SongSection")->DataOffset;
	PropOffsets_LevelInfo.SpawnNotify = cls->GetProperty("SpawnNotify")->DataOffset;
	PropOffsets_LevelInfo.Summary = cls->GetProperty("Summary")->DataOffset;
	PropOffsets_LevelInfo.TimeDilation = cls->GetProperty("TimeDilation")->DataOffset;
	PropOffsets_LevelInfo.TimeSeconds = cls->GetProperty("TimeSeconds")->DataOffset;
	PropOffsets_LevelInfo.Title = cls->GetProperty("Title")->DataOffset;
	PropOffsets_LevelInfo.VisibleGroups = cls->GetProperty("VisibleGroups")->DataOffset;
	PropOffsets_LevelInfo.Year = cls->GetProperty("Year")->DataOffset;
	PropOffsets_LevelInfo.bAggressiveLOD = cls->GetProperty("bAggressiveLOD")->DataOffset;
	PropOffsets_LevelInfo.bAllowFOV = cls->GetProperty("bAllowFOV")->DataOffset;
	PropOffsets_LevelInfo.bBegunPlay = cls->GetProperty("bBegunPlay")->DataOffset;
	PropOffsets_LevelInfo.bCheckWalkSurfaces = cls->GetProperty("bCheckWalkSurfaces")->DataOffset;
	PropOffsets_LevelInfo.bDropDetail = cls->GetProperty("bDropDetail")->DataOffset;
	PropOffsets_LevelInfo.bHighDetailMode = cls->GetProperty("bHighDetailMode")->DataOffset;
	PropOffsets_LevelInfo.bHumansOnly = cls->GetProperty("bHumansOnly")->DataOffset;
	PropOffsets_LevelInfo.bLonePlayer = cls->GetProperty("bLonePlayer")->DataOffset;
	PropOffsets_LevelInfo.bLowRes = cls->GetProperty("bLowRes")->DataOffset;
	PropOffsets_LevelInfo.bNeverPrecache = cls->GetProperty("bNeverPrecache")->DataOffset;
	PropOffsets_LevelInfo.bNextItems = cls->GetProperty("bNextItems")->DataOffset;
	PropOffsets_LevelInfo.bNoCheating = cls->GetProperty("bNoCheating")->DataOffset;
	PropOffsets_LevelInfo.bPlayersOnly = cls->GetProperty("bPlayersOnly")->DataOffset;
	PropOffsets_LevelInfo.bStartup = cls->GetProperty("bStartup")->DataOffset;
}

PropertyOffsets_Inventory PropOffsets_Inventory;

static void InitPropertyOffsets_Inventory(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Inventory"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Inventory");
	PropOffsets_Inventory.AbsorptionPriority = cls->GetProperty("AbsorptionPriority")->DataOffset;
	PropOffsets_Inventory.ActivateSound = cls->GetProperty("ActivateSound")->DataOffset;
	PropOffsets_Inventory.ArmorAbsorption = cls->GetProperty("ArmorAbsorption")->DataOffset;
	PropOffsets_Inventory.AutoSwitchPriority = cls->GetProperty("AutoSwitchPriority")->DataOffset;
	PropOffsets_Inventory.BobDamping = cls->GetProperty("BobDamping")->DataOffset;
	PropOffsets_Inventory.Charge = cls->GetProperty("Charge")->DataOffset;
	PropOffsets_Inventory.DeActivateSound = cls->GetProperty("DeActivateSound")->DataOffset;
	PropOffsets_Inventory.FlashCount = cls->GetProperty("FlashCount")->DataOffset;
	PropOffsets_Inventory.Icon = cls->GetProperty("Icon")->DataOffset;
	PropOffsets_Inventory.InventoryGroup = cls->GetProperty("InventoryGroup")->DataOffset;
	PropOffsets_Inventory.ItemArticle = cls->GetProperty("ItemArticle")->DataOffset;
	PropOffsets_Inventory.ItemMessageClass = cls->GetProperty("ItemMessageClass")->DataOffset;
	PropOffsets_Inventory.ItemName = cls->GetProperty("ItemName")->DataOffset;
	PropOffsets_Inventory.M_Activated = cls->GetProperty("M_Activated")->DataOffset;
	PropOffsets_Inventory.M_Deactivated = cls->GetProperty("M_Deactivated")->DataOffset;
	PropOffsets_Inventory.M_Selected = cls->GetProperty("M_Selected")->DataOffset;
	PropOffsets_Inventory.MaxDesireability = cls->GetProperty("MaxDesireability")->DataOffset;
	PropOffsets_Inventory.MuzzleFlashMesh = cls->GetProperty("MuzzleFlashMesh")->DataOffset;
	PropOffsets_Inventory.MuzzleFlashScale = cls->GetProperty("MuzzleFlashScale")->DataOffset;
	PropOffsets_Inventory.MuzzleFlashStyle = cls->GetProperty("MuzzleFlashStyle")->DataOffset;
	PropOffsets_Inventory.MuzzleFlashTexture = cls->GetProperty("MuzzleFlashTexture")->DataOffset;
	PropOffsets_Inventory.NextArmor = cls->GetProperty("NextArmor")->DataOffset;
	PropOffsets_Inventory.OldFlashCount = cls->GetProperty("OldFlashCount")->DataOffset;
	PropOffsets_Inventory.PickupMessage = cls->GetProperty("PickupMessage")->DataOffset;
	PropOffsets_Inventory.PickupMessageClass = cls->GetProperty("PickupMessageClass")->DataOffset;
	PropOffsets_Inventory.PickupSound = cls->GetProperty("PickupSound")->DataOffset;
	PropOffsets_Inventory.PickupViewMesh = cls->GetProperty("PickupViewMesh")->DataOffset;
	PropOffsets_Inventory.PickupViewScale = cls->GetProperty("PickupViewScale")->DataOffset;
	PropOffsets_Inventory.PlayerLastTouched = cls->GetProperty("PlayerLastTouched")->DataOffset;
	PropOffsets_Inventory.PlayerViewMesh = cls->GetProperty("PlayerViewMesh")->DataOffset;
	PropOffsets_Inventory.PlayerViewOffset = cls->GetProperty("PlayerViewOffset")->DataOffset;
	PropOffsets_Inventory.PlayerViewScale = cls->GetProperty("PlayerViewScale")->DataOffset;
	PropOffsets_Inventory.ProtectionType1 = cls->GetProperty("ProtectionType1")->DataOffset;
	PropOffsets_Inventory.ProtectionType2 = cls->GetProperty("ProtectionType2")->DataOffset;
	PropOffsets_Inventory.RespawnSound = cls->GetProperty("RespawnSound")->DataOffset;
	PropOffsets_Inventory.RespawnTime = cls->GetProperty("RespawnTime")->DataOffset;
	PropOffsets_Inventory.StatusIcon = cls->GetProperty("StatusIcon")->DataOffset;
	PropOffsets_Inventory.ThirdPersonMesh = cls->GetProperty("ThirdPersonMesh")->DataOffset;
	PropOffsets_Inventory.ThirdPersonScale = cls->GetProperty("ThirdPersonScale")->DataOffset;
	PropOffsets_Inventory.bActivatable = cls->GetProperty("bActivatable")->DataOffset;
	PropOffsets_Inventory.bActive = cls->GetProperty("bActive")->DataOffset;
	PropOffsets_Inventory.bAmbientGlow = cls->GetProperty("bAmbientGlow")->DataOffset;
	PropOffsets_Inventory.bDisplayableInv = cls->GetProperty("bDisplayableInv")->DataOffset;
	PropOffsets_Inventory.bFirstFrame = cls->GetProperty("bFirstFrame")->DataOffset;
	PropOffsets_Inventory.bHeldItem = cls->GetProperty("bHeldItem")->DataOffset;
	PropOffsets_Inventory.bInstantRespawn = cls->GetProperty("bInstantRespawn")->DataOffset;
	PropOffsets_Inventory.bIsAnArmor = cls->GetProperty("bIsAnArmor")->DataOffset;
	PropOffsets_Inventory.bMuzzleFlashParticles = cls->GetProperty("bMuzzleFlashParticles")->DataOffset;
	PropOffsets_Inventory.bRotatingPickup = cls->GetProperty("bRotatingPickup")->DataOffset;
	PropOffsets_Inventory.bSleepTouch = cls->GetProperty("bSleepTouch")->DataOffset;
	PropOffsets_Inventory.bSteadyFlash3rd = cls->GetProperty("bSteadyFlash3rd")->DataOffset;
	PropOffsets_Inventory.bSteadyToggle = cls->GetProperty("bSteadyToggle")->DataOffset;
	PropOffsets_Inventory.bToggleSteadyFlash = cls->GetProperty("bToggleSteadyFlash")->DataOffset;
	PropOffsets_Inventory.bTossedOut = cls->GetProperty("bTossedOut")->DataOffset;
	PropOffsets_Inventory.myMarker = cls->GetProperty("myMarker")->DataOffset;
}

PropertyOffsets_PlayerPawn PropOffsets_PlayerPawn;

static void InitPropertyOffsets_PlayerPawn(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "PlayerPawn"));
	if (!cls)
		throw std::runtime_error("Could not find class object for PlayerPawn");
	PropOffsets_PlayerPawn.AppliedBob = cls->GetProperty("AppliedBob")->DataOffset;
	PropOffsets_PlayerPawn.Bob = cls->GetProperty("Bob")->DataOffset;
	PropOffsets_PlayerPawn.BorrowedMouseX = cls->GetProperty("BorrowedMouseX")->DataOffset;
	PropOffsets_PlayerPawn.BorrowedMouseY = cls->GetProperty("BorrowedMouseY")->DataOffset;
	PropOffsets_PlayerPawn.CarcassType = cls->GetProperty("CarcassType")->DataOffset;
	PropOffsets_PlayerPawn.CdTrack = cls->GetProperty("CdTrack")->DataOffset;
	PropOffsets_PlayerPawn.ClientUpdateTime = cls->GetProperty("ClientUpdateTime")->DataOffset;
	PropOffsets_PlayerPawn.ConstantGlowFog = cls->GetProperty("ConstantGlowFog")->DataOffset;
	PropOffsets_PlayerPawn.ConstantGlowScale = cls->GetProperty("ConstantGlowScale")->DataOffset;
	PropOffsets_PlayerPawn.CurrentTimeStamp = cls->GetProperty("CurrentTimeStamp")->DataOffset;
	PropOffsets_PlayerPawn.DefaultFOV = cls->GetProperty("DefaultFOV")->DataOffset;
	PropOffsets_PlayerPawn.DelayedCommand = cls->GetProperty("DelayedCommand")->DataOffset;
	PropOffsets_PlayerPawn.DemoViewPitch = cls->GetProperty("DemoViewPitch")->DataOffset;
	PropOffsets_PlayerPawn.DemoViewYaw = cls->GetProperty("DemoViewYaw")->DataOffset;
	PropOffsets_PlayerPawn.DesiredFOV = cls->GetProperty("DesiredFOV")->DataOffset;
	PropOffsets_PlayerPawn.DesiredFlashFog = cls->GetProperty("DesiredFlashFog")->DataOffset;
	PropOffsets_PlayerPawn.DesiredFlashScale = cls->GetProperty("DesiredFlashScale")->DataOffset;
	PropOffsets_PlayerPawn.DodgeClickTime = cls->GetProperty("DodgeClickTime")->DataOffset;
	PropOffsets_PlayerPawn.DodgeClickTimer = cls->GetProperty("DodgeClickTimer")->DataOffset;
	PropOffsets_PlayerPawn.FailedView = cls->GetProperty("FailedView")->DataOffset;
	PropOffsets_PlayerPawn.FlashFog = cls->GetProperty("FlashFog")->DataOffset;
	PropOffsets_PlayerPawn.FlashScale = cls->GetProperty("FlashScale")->DataOffset;
	PropOffsets_PlayerPawn.FreeMoves = cls->GetProperty("FreeMoves")->DataOffset;
	PropOffsets_PlayerPawn.GameReplicationInfo = cls->GetProperty("GameReplicationInfo")->DataOffset;
	PropOffsets_PlayerPawn.HUDType = cls->GetProperty("HUDType")->DataOffset;
	PropOffsets_PlayerPawn.Handedness = cls->GetProperty("Handedness")->DataOffset;
	PropOffsets_PlayerPawn.InstantFlash = cls->GetProperty("InstantFlash")->DataOffset;
	PropOffsets_PlayerPawn.InstantFog = cls->GetProperty("InstantFog")->DataOffset;
	PropOffsets_PlayerPawn.JumpSound = cls->GetProperty("JumpSound")->DataOffset;
	PropOffsets_PlayerPawn.LandBob = cls->GetProperty("LandBob")->DataOffset;
	PropOffsets_PlayerPawn.LastMessageWindow = cls->GetProperty("LastMessageWindow")->DataOffset;
	PropOffsets_PlayerPawn.LastPlaySound = cls->GetProperty("LastPlaySound")->DataOffset;
	PropOffsets_PlayerPawn.LastUpdateTime = cls->GetProperty("LastUpdateTime")->DataOffset;
	PropOffsets_PlayerPawn.MaxTimeMargin = cls->GetProperty("MaxTimeMargin")->DataOffset;
	PropOffsets_PlayerPawn.Misc1 = cls->GetProperty("Misc1")->DataOffset;
	PropOffsets_PlayerPawn.Misc2 = cls->GetProperty("Misc2")->DataOffset;
	PropOffsets_PlayerPawn.MouseSensitivity = cls->GetProperty("MouseSensitivity")->DataOffset;
	PropOffsets_PlayerPawn.MouseSmoothThreshold = cls->GetProperty("MouseSmoothThreshold")->DataOffset;
	PropOffsets_PlayerPawn.MouseZeroTime = cls->GetProperty("MouseZeroTime")->DataOffset;
	PropOffsets_PlayerPawn.MyAutoAim = cls->GetProperty("MyAutoAim")->DataOffset;
	PropOffsets_PlayerPawn.NoPauseMessage = cls->GetProperty("NoPauseMessage")->DataOffset;
	PropOffsets_PlayerPawn.OwnCamera = cls->GetProperty("OwnCamera")->DataOffset;
	PropOffsets_PlayerPawn.Password = cls->GetProperty("Password")->DataOffset;
	PropOffsets_PlayerPawn.PendingMove = cls->GetProperty("PendingMove")->DataOffset;
	PropOffsets_PlayerPawn.Player = cls->GetProperty("Player")->DataOffset;
	PropOffsets_PlayerPawn.ProgressColor = cls->GetProperty("ProgressColor")->DataOffset;
	PropOffsets_PlayerPawn.ProgressMessage = cls->GetProperty("ProgressMessage")->DataOffset;
	PropOffsets_PlayerPawn.ProgressTimeOut = cls->GetProperty("ProgressTimeOut")->DataOffset;
	PropOffsets_PlayerPawn.QuickSaveString = cls->GetProperty("QuickSaveString")->DataOffset;
	PropOffsets_PlayerPawn.ReceivedSecretChecksum = cls->GetProperty("ReceivedSecretChecksum")->DataOffset;
	PropOffsets_PlayerPawn.RendMap = cls->GetProperty("RendMap")->DataOffset;
	PropOffsets_PlayerPawn.SavedMoves = cls->GetProperty("SavedMoves")->DataOffset;
	PropOffsets_PlayerPawn.Scoring = cls->GetProperty("Scoring")->DataOffset;
	PropOffsets_PlayerPawn.ScoringType = cls->GetProperty("ScoringType")->DataOffset;
	PropOffsets_PlayerPawn.ServerTimeStamp = cls->GetProperty("ServerTimeStamp")->DataOffset;
	PropOffsets_PlayerPawn.ShowFlags = cls->GetProperty("ShowFlags")->DataOffset;
	PropOffsets_PlayerPawn.SmoothMouseX = cls->GetProperty("SmoothMouseX")->DataOffset;
	PropOffsets_PlayerPawn.SmoothMouseY = cls->GetProperty("SmoothMouseY")->DataOffset;
	PropOffsets_PlayerPawn.Song = cls->GetProperty("Song")->DataOffset;
	PropOffsets_PlayerPawn.SongSection = cls->GetProperty("SongSection")->DataOffset;
	PropOffsets_PlayerPawn.SpecialMenu = cls->GetProperty("SpecialMenu")->DataOffset;
	PropOffsets_PlayerPawn.TargetEyeHeight = cls->GetProperty("TargetEyeHeight")->DataOffset;
	PropOffsets_PlayerPawn.TargetViewRotation = cls->GetProperty("TargetViewRotation")->DataOffset;
	PropOffsets_PlayerPawn.TargetWeaponViewOffset = cls->GetProperty("TargetWeaponViewOffset")->DataOffset;
	PropOffsets_PlayerPawn.TimeMargin = cls->GetProperty("TimeMargin")->DataOffset;
	PropOffsets_PlayerPawn.Transition = cls->GetProperty("Transition")->DataOffset;
	PropOffsets_PlayerPawn.ViewTarget = cls->GetProperty("ViewTarget")->DataOffset;
	PropOffsets_PlayerPawn.ViewingFrom = cls->GetProperty("ViewingFrom")->DataOffset;
	PropOffsets_PlayerPawn.WeaponPriority = cls->GetProperty("WeaponPriority")->DataOffset;
	PropOffsets_PlayerPawn.ZoomLevel = cls->GetProperty("ZoomLevel")->DataOffset;
	PropOffsets_PlayerPawn.aBaseX = cls->GetProperty("aBaseX")->DataOffset;
	PropOffsets_PlayerPawn.aBaseY = cls->GetProperty("aBaseY")->DataOffset;
	PropOffsets_PlayerPawn.aBaseZ = cls->GetProperty("aBaseZ")->DataOffset;
	PropOffsets_PlayerPawn.aExtra0 = cls->GetProperty("aExtra0")->DataOffset;
	PropOffsets_PlayerPawn.aExtra1 = cls->GetProperty("aExtra1")->DataOffset;
	PropOffsets_PlayerPawn.aExtra2 = cls->GetProperty("aExtra2")->DataOffset;
	PropOffsets_PlayerPawn.aExtra3 = cls->GetProperty("aExtra3")->DataOffset;
	PropOffsets_PlayerPawn.aExtra4 = cls->GetProperty("aExtra4")->DataOffset;
	PropOffsets_PlayerPawn.aForward = cls->GetProperty("aForward")->DataOffset;
	PropOffsets_PlayerPawn.aLookUp = cls->GetProperty("aLookUp")->DataOffset;
	PropOffsets_PlayerPawn.aMouseX = cls->GetProperty("aMouseX")->DataOffset;
	PropOffsets_PlayerPawn.aMouseY = cls->GetProperty("aMouseY")->DataOffset;
	PropOffsets_PlayerPawn.aStrafe = cls->GetProperty("aStrafe")->DataOffset;
	PropOffsets_PlayerPawn.aTurn = cls->GetProperty("aTurn")->DataOffset;
	PropOffsets_PlayerPawn.aUp = cls->GetProperty("aUp")->DataOffset;
	PropOffsets_PlayerPawn.bAdmin = cls->GetProperty("bAdmin")->DataOffset;
	PropOffsets_PlayerPawn.bAlwaysMouseLook = cls->GetProperty("bAlwaysMouseLook")->DataOffset;
	PropOffsets_PlayerPawn.bAnimTransition = cls->GetProperty("bAnimTransition")->DataOffset;
	PropOffsets_PlayerPawn.bBadConnectionAlert = cls->GetProperty("bBadConnectionAlert")->DataOffset;
	PropOffsets_PlayerPawn.bCenterView = cls->GetProperty("bCenterView")->DataOffset;
	PropOffsets_PlayerPawn.bCheatsEnabled = cls->GetProperty("bCheatsEnabled")->DataOffset;
	PropOffsets_PlayerPawn.bDelayedCommand = cls->GetProperty("bDelayedCommand")->DataOffset;
	PropOffsets_PlayerPawn.bEdgeBack = cls->GetProperty("bEdgeBack")->DataOffset;
	PropOffsets_PlayerPawn.bEdgeForward = cls->GetProperty("bEdgeForward")->DataOffset;
	PropOffsets_PlayerPawn.bEdgeLeft = cls->GetProperty("bEdgeLeft")->DataOffset;
	PropOffsets_PlayerPawn.bEdgeRight = cls->GetProperty("bEdgeRight")->DataOffset;
	PropOffsets_PlayerPawn.bFixedCamera = cls->GetProperty("bFixedCamera")->DataOffset;
	PropOffsets_PlayerPawn.bFrozen = cls->GetProperty("bFrozen")->DataOffset;
	PropOffsets_PlayerPawn.bInvertMouse = cls->GetProperty("bInvertMouse")->DataOffset;
	PropOffsets_PlayerPawn.bIsCrouching = cls->GetProperty("bIsCrouching")->DataOffset;
	PropOffsets_PlayerPawn.bIsTurning = cls->GetProperty("bIsTurning")->DataOffset;
	PropOffsets_PlayerPawn.bIsTyping = cls->GetProperty("bIsTyping")->DataOffset;
	PropOffsets_PlayerPawn.bJumpStatus = cls->GetProperty("bJumpStatus")->DataOffset;
	PropOffsets_PlayerPawn.bJustAltFired = cls->GetProperty("bJustAltFired")->DataOffset;
	PropOffsets_PlayerPawn.bJustFired = cls->GetProperty("bJustFired")->DataOffset;
	PropOffsets_PlayerPawn.bKeyboardLook = cls->GetProperty("bKeyboardLook")->DataOffset;
	PropOffsets_PlayerPawn.bLookUpStairs = cls->GetProperty("bLookUpStairs")->DataOffset;
	PropOffsets_PlayerPawn.bMaxMouseSmoothing = cls->GetProperty("bMaxMouseSmoothing")->DataOffset;
	PropOffsets_PlayerPawn.bMessageBeep = cls->GetProperty("bMessageBeep")->DataOffset;
	PropOffsets_PlayerPawn.bMouseZeroed = cls->GetProperty("bMouseZeroed")->DataOffset;
	PropOffsets_PlayerPawn.bNeverAutoSwitch = cls->GetProperty("bNeverAutoSwitch")->DataOffset;
	PropOffsets_PlayerPawn.bNoFlash = cls->GetProperty("bNoFlash")->DataOffset;
	PropOffsets_PlayerPawn.bNoVoices = cls->GetProperty("bNoVoices")->DataOffset;
	PropOffsets_PlayerPawn.bPressedJump = cls->GetProperty("bPressedJump")->DataOffset;
	PropOffsets_PlayerPawn.bReadyToPlay = cls->GetProperty("bReadyToPlay")->DataOffset;
	PropOffsets_PlayerPawn.bReducedVis = cls->GetProperty("bReducedVis")->DataOffset;
	PropOffsets_PlayerPawn.bRising = cls->GetProperty("bRising")->DataOffset;
	PropOffsets_PlayerPawn.bShakeDir = cls->GetProperty("bShakeDir")->DataOffset;
	PropOffsets_PlayerPawn.bShowMenu = cls->GetProperty("bShowMenu")->DataOffset;
	PropOffsets_PlayerPawn.bShowScores = cls->GetProperty("bShowScores")->DataOffset;
	PropOffsets_PlayerPawn.bSinglePlayer = cls->GetProperty("bSinglePlayer")->DataOffset;
	PropOffsets_PlayerPawn.bSnapToLevel = cls->GetProperty("bSnapToLevel")->DataOffset;
	PropOffsets_PlayerPawn.bSpecialMenu = cls->GetProperty("bSpecialMenu")->DataOffset;
	PropOffsets_PlayerPawn.bUpdatePosition = cls->GetProperty("bUpdatePosition")->DataOffset;
	PropOffsets_PlayerPawn.bUpdating = cls->GetProperty("bUpdating")->DataOffset;
	PropOffsets_PlayerPawn.bWasBack = cls->GetProperty("bWasBack")->DataOffset;
	PropOffsets_PlayerPawn.bWasForward = cls->GetProperty("bWasForward")->DataOffset;
	PropOffsets_PlayerPawn.bWasLeft = cls->GetProperty("bWasLeft")->DataOffset;
	PropOffsets_PlayerPawn.bWasRight = cls->GetProperty("bWasRight")->DataOffset;
	PropOffsets_PlayerPawn.bWokeUp = cls->GetProperty("bWokeUp")->DataOffset;
	PropOffsets_PlayerPawn.bZooming = cls->GetProperty("bZooming")->DataOffset;
	PropOffsets_PlayerPawn.bobtime = cls->GetProperty("bobtime")->DataOffset;
	PropOffsets_PlayerPawn.maxshake = cls->GetProperty("maxshake")->DataOffset;
	PropOffsets_PlayerPawn.myHUD = cls->GetProperty("myHUD")->DataOffset;
	PropOffsets_PlayerPawn.ngSecretSet = cls->GetProperty("ngSecretSet")->DataOffset;
	PropOffsets_PlayerPawn.ngWorldSecret = cls->GetProperty("ngWorldSecret")->DataOffset;
	PropOffsets_PlayerPawn.shakemag = cls->GetProperty("shakemag")->DataOffset;
	PropOffsets_PlayerPawn.shaketimer = cls->GetProperty("shaketimer")->DataOffset;
	PropOffsets_PlayerPawn.shakevert = cls->GetProperty("shakevert")->DataOffset;
	PropOffsets_PlayerPawn.verttimer = cls->GetProperty("verttimer")->DataOffset;
}

PropertyOffsets_PlayerReplicationInfo PropOffsets_PlayerReplicationInfo;

static void InitPropertyOffsets_PlayerReplicationInfo(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "PlayerReplicationInfo"));
	if (!cls)
		throw std::runtime_error("Could not find class object for PlayerReplicationInfo");
	PropOffsets_PlayerReplicationInfo.Deaths = cls->GetProperty("Deaths")->DataOffset;
	PropOffsets_PlayerReplicationInfo.HasFlag = cls->GetProperty("HasFlag")->DataOffset;
	PropOffsets_PlayerReplicationInfo.OldName = cls->GetProperty("OldName")->DataOffset;
	PropOffsets_PlayerReplicationInfo.PacketLoss = cls->GetProperty("PacketLoss")->DataOffset;
	PropOffsets_PlayerReplicationInfo.Ping = cls->GetProperty("Ping")->DataOffset;
	PropOffsets_PlayerReplicationInfo.PlayerID = cls->GetProperty("PlayerID")->DataOffset;
	PropOffsets_PlayerReplicationInfo.PlayerLocation = cls->GetProperty("PlayerLocation")->DataOffset;
	PropOffsets_PlayerReplicationInfo.PlayerName = cls->GetProperty("PlayerName")->DataOffset;
	PropOffsets_PlayerReplicationInfo.PlayerZone = cls->GetProperty("PlayerZone")->DataOffset;
	PropOffsets_PlayerReplicationInfo.Score = cls->GetProperty("Score")->DataOffset;
	PropOffsets_PlayerReplicationInfo.StartTime = cls->GetProperty("StartTime")->DataOffset;
	PropOffsets_PlayerReplicationInfo.TalkTexture = cls->GetProperty("TalkTexture")->DataOffset;
	PropOffsets_PlayerReplicationInfo.Team = cls->GetProperty("Team")->DataOffset;
	PropOffsets_PlayerReplicationInfo.TeamID = cls->GetProperty("TeamID")->DataOffset;
	PropOffsets_PlayerReplicationInfo.TeamName = cls->GetProperty("TeamName")->DataOffset;
	PropOffsets_PlayerReplicationInfo.TimeAcc = cls->GetProperty("TimeAcc")->DataOffset;
	PropOffsets_PlayerReplicationInfo.VoiceType = cls->GetProperty("VoiceType")->DataOffset;
	PropOffsets_PlayerReplicationInfo.bAdmin = cls->GetProperty("bAdmin")->DataOffset;
	PropOffsets_PlayerReplicationInfo.bFeigningDeath = cls->GetProperty("bFeigningDeath")->DataOffset;
	PropOffsets_PlayerReplicationInfo.bIsABot = cls->GetProperty("bIsABot")->DataOffset;
	PropOffsets_PlayerReplicationInfo.bIsFemale = cls->GetProperty("bIsFemale")->DataOffset;
	PropOffsets_PlayerReplicationInfo.bIsSpectator = cls->GetProperty("bIsSpectator")->DataOffset;
	PropOffsets_PlayerReplicationInfo.bWaitingPlayer = cls->GetProperty("bWaitingPlayer")->DataOffset;
}

PropertyOffsets_Weapon PropOffsets_Weapon;

static void InitPropertyOffsets_Weapon(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Weapon"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Weapon");
	PropOffsets_Weapon.AIRating = cls->GetProperty("AIRating")->DataOffset;
	PropOffsets_Weapon.AdjustedAim = cls->GetProperty("AdjustedAim")->DataOffset;
	PropOffsets_Weapon.AltDamageType = cls->GetProperty("AltDamageType")->DataOffset;
	PropOffsets_Weapon.AltFireSound = cls->GetProperty("AltFireSound")->DataOffset;
	PropOffsets_Weapon.AltProjectileClass = cls->GetProperty("AltProjectileClass")->DataOffset;
	PropOffsets_Weapon.AltProjectileSpeed = cls->GetProperty("AltProjectileSpeed")->DataOffset;
	PropOffsets_Weapon.AltRefireRate = cls->GetProperty("AltRefireRate")->DataOffset;
	PropOffsets_Weapon.AmmoName = cls->GetProperty("AmmoName")->DataOffset;
	PropOffsets_Weapon.AmmoType = cls->GetProperty("AmmoType")->DataOffset;
	PropOffsets_Weapon.CockingSound = cls->GetProperty("CockingSound")->DataOffset;
	PropOffsets_Weapon.DeathMessage = cls->GetProperty("DeathMessage")->DataOffset;
	PropOffsets_Weapon.FireOffset = cls->GetProperty("FireOffset")->DataOffset;
	PropOffsets_Weapon.FireSound = cls->GetProperty("FireSound")->DataOffset;
	PropOffsets_Weapon.FiringSpeed = cls->GetProperty("FiringSpeed")->DataOffset;
	PropOffsets_Weapon.FlareOffset = cls->GetProperty("FlareOffset")->DataOffset;
	PropOffsets_Weapon.FlashC = cls->GetProperty("FlashC")->DataOffset;
	PropOffsets_Weapon.FlashLength = cls->GetProperty("FlashLength")->DataOffset;
	PropOffsets_Weapon.FlashO = cls->GetProperty("FlashO")->DataOffset;
	PropOffsets_Weapon.FlashS = cls->GetProperty("FlashS")->DataOffset;
	PropOffsets_Weapon.FlashTime = cls->GetProperty("FlashTime")->DataOffset;
	PropOffsets_Weapon.FlashY = cls->GetProperty("FlashY")->DataOffset;
	PropOffsets_Weapon.MFTexture = cls->GetProperty("MFTexture")->DataOffset;
	PropOffsets_Weapon.MaxTargetRange = cls->GetProperty("MaxTargetRange")->DataOffset;
	PropOffsets_Weapon.MessageNoAmmo = cls->GetProperty("MessageNoAmmo")->DataOffset;
	PropOffsets_Weapon.Misc1Sound = cls->GetProperty("Misc1Sound")->DataOffset;
	PropOffsets_Weapon.Misc2Sound = cls->GetProperty("Misc2Sound")->DataOffset;
	PropOffsets_Weapon.Misc3Sound = cls->GetProperty("Misc3Sound")->DataOffset;
	PropOffsets_Weapon.MuzzleFlare = cls->GetProperty("MuzzleFlare")->DataOffset;
	PropOffsets_Weapon.MuzzleScale = cls->GetProperty("MuzzleScale")->DataOffset;
	PropOffsets_Weapon.MyDamageType = cls->GetProperty("MyDamageType")->DataOffset;
	PropOffsets_Weapon.NameColor = cls->GetProperty("NameColor")->DataOffset;
	PropOffsets_Weapon.PickupAmmoCount = cls->GetProperty("PickupAmmoCount")->DataOffset;
	PropOffsets_Weapon.ProjectileClass = cls->GetProperty("ProjectileClass")->DataOffset;
	PropOffsets_Weapon.ProjectileSpeed = cls->GetProperty("ProjectileSpeed")->DataOffset;
	PropOffsets_Weapon.RefireRate = cls->GetProperty("RefireRate")->DataOffset;
	PropOffsets_Weapon.ReloadCount = cls->GetProperty("ReloadCount")->DataOffset;
	PropOffsets_Weapon.SelectSound = cls->GetProperty("SelectSound")->DataOffset;
	PropOffsets_Weapon.aimerror = cls->GetProperty("aimerror")->DataOffset;
	PropOffsets_Weapon.bAltInstantHit = cls->GetProperty("bAltInstantHit")->DataOffset;
	PropOffsets_Weapon.bAltWarnTarget = cls->GetProperty("bAltWarnTarget")->DataOffset;
	PropOffsets_Weapon.bCanThrow = cls->GetProperty("bCanThrow")->DataOffset;
	PropOffsets_Weapon.bChangeWeapon = cls->GetProperty("bChangeWeapon")->DataOffset;
	PropOffsets_Weapon.bDrawMuzzleFlash = cls->GetProperty("bDrawMuzzleFlash")->DataOffset;
	PropOffsets_Weapon.bHideWeapon = cls->GetProperty("bHideWeapon")->DataOffset;
	PropOffsets_Weapon.bInstantHit = cls->GetProperty("bInstantHit")->DataOffset;
	PropOffsets_Weapon.bLockedOn = cls->GetProperty("bLockedOn")->DataOffset;
	PropOffsets_Weapon.bMeleeWeapon = cls->GetProperty("bMeleeWeapon")->DataOffset;
	PropOffsets_Weapon.bMuzzleFlash = cls->GetProperty("bMuzzleFlash")->DataOffset;
	PropOffsets_Weapon.bOwnsCrosshair = cls->GetProperty("bOwnsCrosshair")->DataOffset;
	PropOffsets_Weapon.bPointing = cls->GetProperty("bPointing")->DataOffset;
	PropOffsets_Weapon.bRapidFire = cls->GetProperty("bRapidFire")->DataOffset;
	PropOffsets_Weapon.bRecommendAltSplashDamage = cls->GetProperty("bRecommendAltSplashDamage")->DataOffset;
	PropOffsets_Weapon.bRecommendSplashDamage = cls->GetProperty("bRecommendSplashDamage")->DataOffset;
	PropOffsets_Weapon.bSetFlashTime = cls->GetProperty("bSetFlashTime")->DataOffset;
	PropOffsets_Weapon.bSpecialIcon = cls->GetProperty("bSpecialIcon")->DataOffset;
	PropOffsets_Weapon.bSplashDamage = cls->GetProperty("bSplashDamage")->DataOffset;
	PropOffsets_Weapon.bWarnTarget = cls->GetProperty("bWarnTarget")->DataOffset;
	PropOffsets_Weapon.bWeaponStay = cls->GetProperty("bWeaponStay")->DataOffset;
	PropOffsets_Weapon.bWeaponUp = cls->GetProperty("bWeaponUp")->DataOffset;
	PropOffsets_Weapon.shakemag = cls->GetProperty("shakemag")->DataOffset;
	PropOffsets_Weapon.shaketime = cls->GetProperty("shaketime")->DataOffset;
	PropOffsets_Weapon.shakevert = cls->GetProperty("shakevert")->DataOffset;
}

PropertyOffsets_GameInfo PropOffsets_GameInfo;

static void InitPropertyOffsets_GameInfo(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "GameInfo"));
	if (!cls)
		throw std::runtime_error("Could not find class object for GameInfo");
	PropOffsets_GameInfo.AdminPassword = cls->GetProperty("AdminPassword")->DataOffset;
	PropOffsets_GameInfo.AutoAim = cls->GetProperty("AutoAim")->DataOffset;
	PropOffsets_GameInfo.BaseMutator = cls->GetProperty("BaseMutator")->DataOffset;
	PropOffsets_GameInfo.BeaconName = cls->GetProperty("BeaconName")->DataOffset;
	PropOffsets_GameInfo.BotMenuType = cls->GetProperty("BotMenuType")->DataOffset;
	PropOffsets_GameInfo.CurrentID = cls->GetProperty("CurrentID")->DataOffset;
	PropOffsets_GameInfo.DMMessageClass = cls->GetProperty("DMMessageClass")->DataOffset;
	PropOffsets_GameInfo.DamageMutator = cls->GetProperty("DamageMutator")->DataOffset;
	PropOffsets_GameInfo.DeathMessageClass = cls->GetProperty("DeathMessageClass")->DataOffset;
	PropOffsets_GameInfo.DefaultPlayerClass = cls->GetProperty("DefaultPlayerClass")->DataOffset;
	PropOffsets_GameInfo.DefaultPlayerName = cls->GetProperty("DefaultPlayerName")->DataOffset;
	PropOffsets_GameInfo.DefaultPlayerState = cls->GetProperty("DefaultPlayerState")->DataOffset;
	PropOffsets_GameInfo.DefaultWeapon = cls->GetProperty("DefaultWeapon")->DataOffset;
	PropOffsets_GameInfo.DemoBuild = cls->GetProperty("DemoBuild")->DataOffset;
	PropOffsets_GameInfo.DemoHasTuts = cls->GetProperty("DemoHasTuts")->DataOffset;
	PropOffsets_GameInfo.Difficulty = cls->GetProperty("Difficulty")->DataOffset;
	PropOffsets_GameInfo.EnabledMutators = cls->GetProperty("EnabledMutators")->DataOffset;
	PropOffsets_GameInfo.EnteredMessage = cls->GetProperty("EnteredMessage")->DataOffset;
	PropOffsets_GameInfo.FailedPlaceMessage = cls->GetProperty("FailedPlaceMessage")->DataOffset;
	PropOffsets_GameInfo.FailedSpawnMessage = cls->GetProperty("FailedSpawnMessage")->DataOffset;
	PropOffsets_GameInfo.FailedTeamMessage = cls->GetProperty("FailedTeamMessage")->DataOffset;
	PropOffsets_GameInfo.GameMenuType = cls->GetProperty("GameMenuType")->DataOffset;
	PropOffsets_GameInfo.GameName = cls->GetProperty("GameName")->DataOffset;
	PropOffsets_GameInfo.GameOptionsMenuType = cls->GetProperty("GameOptionsMenuType")->DataOffset;
	PropOffsets_GameInfo.GamePassword = cls->GetProperty("GamePassword")->DataOffset;
	PropOffsets_GameInfo.GameReplicationInfo = cls->GetProperty("GameReplicationInfo")->DataOffset;
	PropOffsets_GameInfo.GameReplicationInfoClass = cls->GetProperty("GameReplicationInfoClass")->DataOffset;
	PropOffsets_GameInfo.GameSpeed = cls->GetProperty("GameSpeed")->DataOffset;
	PropOffsets_GameInfo.GameUMenuType = cls->GetProperty("GameUMenuType")->DataOffset;
	PropOffsets_GameInfo.HUDType = cls->GetProperty("HUDType")->DataOffset;
	PropOffsets_GameInfo.IPBanned = cls->GetProperty("IPBanned")->DataOffset;
	PropOffsets_GameInfo.IPPolicies = cls->GetProperty("IPPolicies")->DataOffset;
	PropOffsets_GameInfo.ItemGoals = cls->GetProperty("ItemGoals")->DataOffset;
	PropOffsets_GameInfo.KillGoals = cls->GetProperty("KillGoals")->DataOffset;
	PropOffsets_GameInfo.LeftMessage = cls->GetProperty("LeftMessage")->DataOffset;
	PropOffsets_GameInfo.LocalLog = cls->GetProperty("LocalLog")->DataOffset;
	PropOffsets_GameInfo.LocalLogFileName = cls->GetProperty("LocalLogFileName")->DataOffset;
	PropOffsets_GameInfo.MapListType = cls->GetProperty("MapListType")->DataOffset;
	PropOffsets_GameInfo.MapPrefix = cls->GetProperty("MapPrefix")->DataOffset;
	PropOffsets_GameInfo.MaxPlayers = cls->GetProperty("MaxPlayers")->DataOffset;
	PropOffsets_GameInfo.MaxSpectators = cls->GetProperty("MaxSpectators")->DataOffset;
	PropOffsets_GameInfo.MaxedOutMessage = cls->GetProperty("MaxedOutMessage")->DataOffset;
	PropOffsets_GameInfo.MessageMutator = cls->GetProperty("MessageMutator")->DataOffset;
	PropOffsets_GameInfo.MultiplayerUMenuType = cls->GetProperty("MultiplayerUMenuType")->DataOffset;
	PropOffsets_GameInfo.MutatorClass = cls->GetProperty("MutatorClass")->DataOffset;
	PropOffsets_GameInfo.NameChangedMessage = cls->GetProperty("NameChangedMessage")->DataOffset;
	PropOffsets_GameInfo.NeedPassword = cls->GetProperty("NeedPassword")->DataOffset;
	PropOffsets_GameInfo.NumPlayers = cls->GetProperty("NumPlayers")->DataOffset;
	PropOffsets_GameInfo.NumSpectators = cls->GetProperty("NumSpectators")->DataOffset;
	PropOffsets_GameInfo.RulesMenuType = cls->GetProperty("RulesMenuType")->DataOffset;
	PropOffsets_GameInfo.ScoreBoardType = cls->GetProperty("ScoreBoardType")->DataOffset;
	PropOffsets_GameInfo.SecretGoals = cls->GetProperty("SecretGoals")->DataOffset;
	PropOffsets_GameInfo.SentText = cls->GetProperty("SentText")->DataOffset;
	PropOffsets_GameInfo.ServerLogName = cls->GetProperty("ServerLogName")->DataOffset;
	PropOffsets_GameInfo.SettingsMenuType = cls->GetProperty("SettingsMenuType")->DataOffset;
	PropOffsets_GameInfo.SpecialDamageString = cls->GetProperty("SpecialDamageString")->DataOffset;
	PropOffsets_GameInfo.StartTime = cls->GetProperty("StartTime")->DataOffset;
	PropOffsets_GameInfo.StatLogClass = cls->GetProperty("StatLogClass")->DataOffset;
	PropOffsets_GameInfo.SwitchLevelMessage = cls->GetProperty("SwitchLevelMessage")->DataOffset;
	PropOffsets_GameInfo.WaterZoneType = cls->GetProperty("WaterZoneType")->DataOffset;
	PropOffsets_GameInfo.WorldLog = cls->GetProperty("WorldLog")->DataOffset;
	PropOffsets_GameInfo.WorldLogFileName = cls->GetProperty("WorldLogFileName")->DataOffset;
	PropOffsets_GameInfo.WrongPassword = cls->GetProperty("WrongPassword")->DataOffset;
	PropOffsets_GameInfo.bAllowFOV = cls->GetProperty("bAllowFOV")->DataOffset;
	PropOffsets_GameInfo.bAlternateMode = cls->GetProperty("bAlternateMode")->DataOffset;
	PropOffsets_GameInfo.bBatchLocal = cls->GetProperty("bBatchLocal")->DataOffset;
	PropOffsets_GameInfo.bCanChangeSkin = cls->GetProperty("bCanChangeSkin")->DataOffset;
	PropOffsets_GameInfo.bCanViewOthers = cls->GetProperty("bCanViewOthers")->DataOffset;
	PropOffsets_GameInfo.bClassicDeathMessages = cls->GetProperty("bClassicDeathMessages")->DataOffset;
	PropOffsets_GameInfo.bCoopWeaponMode = cls->GetProperty("bCoopWeaponMode")->DataOffset;
	PropOffsets_GameInfo.bDeathMatch = cls->GetProperty("bDeathMatch")->DataOffset;
	PropOffsets_GameInfo.bExternalBatcher = cls->GetProperty("bExternalBatcher")->DataOffset;
	PropOffsets_GameInfo.bGameEnded = cls->GetProperty("bGameEnded")->DataOffset;
	PropOffsets_GameInfo.bHumansOnly = cls->GetProperty("bHumansOnly")->DataOffset;
	PropOffsets_GameInfo.bLocalLog = cls->GetProperty("bLocalLog")->DataOffset;
	PropOffsets_GameInfo.bLoggingGame = cls->GetProperty("bLoggingGame")->DataOffset;
	PropOffsets_GameInfo.bLowGore = cls->GetProperty("bLowGore")->DataOffset;
	PropOffsets_GameInfo.bMuteSpectators = cls->GetProperty("bMuteSpectators")->DataOffset;
	PropOffsets_GameInfo.bNoCheating = cls->GetProperty("bNoCheating")->DataOffset;
	PropOffsets_GameInfo.bNoMonsters = cls->GetProperty("bNoMonsters")->DataOffset;
	PropOffsets_GameInfo.bOverTime = cls->GetProperty("bOverTime")->DataOffset;
	PropOffsets_GameInfo.bPauseable = cls->GetProperty("bPauseable")->DataOffset;
	PropOffsets_GameInfo.bRestartLevel = cls->GetProperty("bRestartLevel")->DataOffset;
	PropOffsets_GameInfo.bTeamGame = cls->GetProperty("bTeamGame")->DataOffset;
	PropOffsets_GameInfo.bVeryLowGore = cls->GetProperty("bVeryLowGore")->DataOffset;
	PropOffsets_GameInfo.bWorldLog = cls->GetProperty("bWorldLog")->DataOffset;
}

PropertyOffsets_ZoneInfo PropOffsets_ZoneInfo;

static void InitPropertyOffsets_ZoneInfo(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "ZoneInfo"));
	if (!cls)
		throw std::runtime_error("Could not find class object for ZoneInfo");
	PropOffsets_ZoneInfo.AmbientBrightness = cls->GetProperty("AmbientBrightness")->DataOffset;
	PropOffsets_ZoneInfo.AmbientHue = cls->GetProperty("AmbientHue")->DataOffset;
	PropOffsets_ZoneInfo.AmbientSaturation = cls->GetProperty("AmbientSaturation")->DataOffset;
	PropOffsets_ZoneInfo.CutoffHz = cls->GetProperty("CutoffHz")->DataOffset;
	PropOffsets_ZoneInfo.DamagePerSec = cls->GetProperty("DamagePerSec")->DataOffset;
	PropOffsets_ZoneInfo.DamageString = cls->GetProperty("DamageString")->DataOffset;
	PropOffsets_ZoneInfo.DamageType = cls->GetProperty("DamageType")->DataOffset;
	PropOffsets_ZoneInfo.Delay = cls->GetProperty("Delay")->DataOffset;
	PropOffsets_ZoneInfo.EntryActor = cls->GetProperty("EntryActor")->DataOffset;
	PropOffsets_ZoneInfo.EntrySound = cls->GetProperty("EntrySound")->DataOffset;
	PropOffsets_ZoneInfo.EnvironmentMap = cls->GetProperty("EnvironmentMap")->DataOffset;
	PropOffsets_ZoneInfo.ExitActor = cls->GetProperty("ExitActor")->DataOffset;
	PropOffsets_ZoneInfo.ExitSound = cls->GetProperty("ExitSound")->DataOffset;
	PropOffsets_ZoneInfo.FogColor = cls->GetProperty("FogColor")->DataOffset;
	PropOffsets_ZoneInfo.FogDistance = cls->GetProperty("FogDistance")->DataOffset;
	PropOffsets_ZoneInfo.Gain = cls->GetProperty("Gain")->DataOffset;
	PropOffsets_ZoneInfo.LensFlare = cls->GetProperty("LensFlare")->DataOffset;
	PropOffsets_ZoneInfo.LensFlareOffset = cls->GetProperty("LensFlareOffset")->DataOffset;
	PropOffsets_ZoneInfo.LensFlareScale = cls->GetProperty("LensFlareScale")->DataOffset;
	PropOffsets_ZoneInfo.MasterGain = cls->GetProperty("MasterGain")->DataOffset;
	PropOffsets_ZoneInfo.MaxCarcasses = cls->GetProperty("MaxCarcasses")->DataOffset;
	PropOffsets_ZoneInfo.MaxLightCount = cls->GetProperty("MaxLightCount")->DataOffset;
	PropOffsets_ZoneInfo.MaxLightingPolyCount = cls->GetProperty("MaxLightingPolyCount")->DataOffset;
	PropOffsets_ZoneInfo.MinLightCount = cls->GetProperty("MinLightCount")->DataOffset;
	PropOffsets_ZoneInfo.MinLightingPolyCount = cls->GetProperty("MinLightingPolyCount")->DataOffset;
	PropOffsets_ZoneInfo.NumCarcasses = cls->GetProperty("NumCarcasses")->DataOffset;
	PropOffsets_ZoneInfo.SkyZone = cls->GetProperty("SkyZone")->DataOffset;
	PropOffsets_ZoneInfo.SpeedOfSound = cls->GetProperty("SpeedOfSound")->DataOffset;
	PropOffsets_ZoneInfo.TexUPanSpeed = cls->GetProperty("TexUPanSpeed")->DataOffset;
	PropOffsets_ZoneInfo.TexVPanSpeed = cls->GetProperty("TexVPanSpeed")->DataOffset;
	PropOffsets_ZoneInfo.ViewFlash = cls->GetProperty("ViewFlash")->DataOffset;
	PropOffsets_ZoneInfo.ViewFog = cls->GetProperty("ViewFog")->DataOffset;
	PropOffsets_ZoneInfo.ZoneFluidFriction = cls->GetProperty("ZoneFluidFriction")->DataOffset;
	PropOffsets_ZoneInfo.ZoneGravity = cls->GetProperty("ZoneGravity")->DataOffset;
	PropOffsets_ZoneInfo.ZoneGroundFriction = cls->GetProperty("ZoneGroundFriction")->DataOffset;
	PropOffsets_ZoneInfo.ZoneName = cls->GetProperty("ZoneName")->DataOffset;
	PropOffsets_ZoneInfo.ZonePlayerCount = cls->GetProperty("ZonePlayerCount")->DataOffset;
	PropOffsets_ZoneInfo.ZonePlayerEvent = cls->GetProperty("ZonePlayerEvent")->DataOffset;
	PropOffsets_ZoneInfo.ZoneTag = cls->GetProperty("ZoneTag")->DataOffset;
	PropOffsets_ZoneInfo.ZoneTerminalVelocity = cls->GetProperty("ZoneTerminalVelocity")->DataOffset;
	PropOffsets_ZoneInfo.ZoneVelocity = cls->GetProperty("ZoneVelocity")->DataOffset;
	PropOffsets_ZoneInfo.bBounceVelocity = cls->GetProperty("bBounceVelocity")->DataOffset;
	PropOffsets_ZoneInfo.bDestructive = cls->GetProperty("bDestructive")->DataOffset;
	PropOffsets_ZoneInfo.bFogZone = cls->GetProperty("bFogZone")->DataOffset;
	PropOffsets_ZoneInfo.bGravityZone = cls->GetProperty("bGravityZone")->DataOffset;
	PropOffsets_ZoneInfo.bKillZone = cls->GetProperty("bKillZone")->DataOffset;
	PropOffsets_ZoneInfo.bMoveProjectiles = cls->GetProperty("bMoveProjectiles")->DataOffset;
	PropOffsets_ZoneInfo.bNeutralZone = cls->GetProperty("bNeutralZone")->DataOffset;
	PropOffsets_ZoneInfo.bNoInventory = cls->GetProperty("bNoInventory")->DataOffset;
	PropOffsets_ZoneInfo.bPainZone = cls->GetProperty("bPainZone")->DataOffset;
	PropOffsets_ZoneInfo.bRaytraceReverb = cls->GetProperty("bRaytraceReverb")->DataOffset;
	PropOffsets_ZoneInfo.bReverbZone = cls->GetProperty("bReverbZone")->DataOffset;
	PropOffsets_ZoneInfo.bWaterZone = cls->GetProperty("bWaterZone")->DataOffset;
	PropOffsets_ZoneInfo.locationid = cls->GetProperty("locationid")->DataOffset;
}

PropertyOffsets_Canvas PropOffsets_Canvas;

static void InitPropertyOffsets_Canvas(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Canvas"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Canvas");
	PropOffsets_Canvas.BigFont = cls->GetProperty("BigFont")->DataOffset;
	PropOffsets_Canvas.ClipX = cls->GetProperty("ClipX")->DataOffset;
	PropOffsets_Canvas.ClipY = cls->GetProperty("ClipY")->DataOffset;
	PropOffsets_Canvas.CurX = cls->GetProperty("CurX")->DataOffset;
	PropOffsets_Canvas.CurY = cls->GetProperty("CurY")->DataOffset;
	PropOffsets_Canvas.CurYL = cls->GetProperty("CurYL")->DataOffset;
	PropOffsets_Canvas.DrawColor = cls->GetProperty("DrawColor")->DataOffset;
	PropOffsets_Canvas.Font = cls->GetProperty("Font")->DataOffset;
	PropOffsets_Canvas.FramePtr = cls->GetProperty("FramePtr")->DataOffset;
	PropOffsets_Canvas.LargeFont = cls->GetProperty("LargeFont")->DataOffset;
	PropOffsets_Canvas.MedFont = cls->GetProperty("MedFont")->DataOffset;
	PropOffsets_Canvas.OrgX = cls->GetProperty("OrgX")->DataOffset;
	PropOffsets_Canvas.OrgY = cls->GetProperty("OrgY")->DataOffset;
	PropOffsets_Canvas.RenderPtr = cls->GetProperty("RenderPtr")->DataOffset;
	PropOffsets_Canvas.SizeX = cls->GetProperty("SizeX")->DataOffset;
	PropOffsets_Canvas.SizeY = cls->GetProperty("SizeY")->DataOffset;
	PropOffsets_Canvas.SmallFont = cls->GetProperty("SmallFont")->DataOffset;
	PropOffsets_Canvas.SpaceX = cls->GetProperty("SpaceX")->DataOffset;
	PropOffsets_Canvas.SpaceY = cls->GetProperty("SpaceY")->DataOffset;
	PropOffsets_Canvas.Style = cls->GetProperty("Style")->DataOffset;
	PropOffsets_Canvas.Viewport = cls->GetProperty("Viewport")->DataOffset;
	PropOffsets_Canvas.Z = cls->GetProperty("Z")->DataOffset;
	PropOffsets_Canvas.bCenter = cls->GetProperty("bCenter")->DataOffset;
	PropOffsets_Canvas.bNoSmooth = cls->GetProperty("bNoSmooth")->DataOffset;
}

PropertyOffsets_SavedMove PropOffsets_SavedMove;

static void InitPropertyOffsets_SavedMove(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "SavedMove"));
	if (!cls)
		throw std::runtime_error("Could not find class object for SavedMove");
	PropOffsets_SavedMove.Delta = cls->GetProperty("Delta")->DataOffset;
	PropOffsets_SavedMove.DodgeMove = cls->GetProperty("DodgeMove")->DataOffset;
	PropOffsets_SavedMove.NextMove = cls->GetProperty("NextMove")->DataOffset;
	PropOffsets_SavedMove.TimeStamp = cls->GetProperty("TimeStamp")->DataOffset;
	PropOffsets_SavedMove.bAltFire = cls->GetProperty("bAltFire")->DataOffset;
	PropOffsets_SavedMove.bDuck = cls->GetProperty("bDuck")->DataOffset;
	PropOffsets_SavedMove.bFire = cls->GetProperty("bFire")->DataOffset;
	PropOffsets_SavedMove.bForceAltFire = cls->GetProperty("bForceAltFire")->DataOffset;
	PropOffsets_SavedMove.bForceFire = cls->GetProperty("bForceFire")->DataOffset;
	PropOffsets_SavedMove.bPressedJump = cls->GetProperty("bPressedJump")->DataOffset;
	PropOffsets_SavedMove.bRun = cls->GetProperty("bRun")->DataOffset;
}

PropertyOffsets_StatLog PropOffsets_StatLog;

static void InitPropertyOffsets_StatLog(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "StatLog"));
	if (!cls)
		throw std::runtime_error("Could not find class object for StatLog");
	PropOffsets_StatLog.Context = cls->GetProperty("Context")->DataOffset;
	PropOffsets_StatLog.DecoderRingURL = cls->GetProperty("DecoderRingURL")->DataOffset;
	PropOffsets_StatLog.GameCreator = cls->GetProperty("GameCreator")->DataOffset;
	PropOffsets_StatLog.GameCreatorURL = cls->GetProperty("GameCreatorURL")->DataOffset;
	PropOffsets_StatLog.GameName = cls->GetProperty("GameName")->DataOffset;
	PropOffsets_StatLog.LocalBatcherParams = cls->GetProperty("LocalBatcherParams")->DataOffset;
	PropOffsets_StatLog.LocalBatcherURL = cls->GetProperty("LocalBatcherURL")->DataOffset;
	PropOffsets_StatLog.LocalLogDir = cls->GetProperty("LocalLogDir")->DataOffset;
	PropOffsets_StatLog.LocalStandard = cls->GetProperty("LocalStandard")->DataOffset;
	PropOffsets_StatLog.LocalStatsURL = cls->GetProperty("LocalStatsURL")->DataOffset;
	PropOffsets_StatLog.LogInfoURL = cls->GetProperty("LogInfoURL")->DataOffset;
	PropOffsets_StatLog.LogVersion = cls->GetProperty("LogVersion")->DataOffset;
	PropOffsets_StatLog.TimeStamp = cls->GetProperty("TimeStamp")->DataOffset;
	PropOffsets_StatLog.WorldBatcherParams = cls->GetProperty("WorldBatcherParams")->DataOffset;
	PropOffsets_StatLog.WorldBatcherURL = cls->GetProperty("WorldBatcherURL")->DataOffset;
	PropOffsets_StatLog.WorldLogDir = cls->GetProperty("WorldLogDir")->DataOffset;
	PropOffsets_StatLog.WorldStandard = cls->GetProperty("WorldStandard")->DataOffset;
	PropOffsets_StatLog.WorldStatsURL = cls->GetProperty("WorldStatsURL")->DataOffset;
	PropOffsets_StatLog.bWorld = cls->GetProperty("bWorld")->DataOffset;
	PropOffsets_StatLog.bWorldBatcherError = cls->GetProperty("bWorldBatcherError")->DataOffset;
}

PropertyOffsets_Texture PropOffsets_Texture;

static void InitPropertyOffsets_Texture(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Texture"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Texture");
	PropOffsets_Texture.Accumulator = cls->GetProperty("Accumulator")->DataOffset;
	PropOffsets_Texture.Alpha = cls->GetProperty("Alpha")->DataOffset;
	PropOffsets_Texture.AnimCurrent = cls->GetProperty("AnimCurrent")->DataOffset;
	PropOffsets_Texture.AnimNext = cls->GetProperty("AnimNext")->DataOffset;
	PropOffsets_Texture.BumpMap = cls->GetProperty("BumpMap")->DataOffset;
	PropOffsets_Texture.CompFormat = cls->GetProperty("CompFormat")->DataOffset;
	PropOffsets_Texture.CompMips = cls->GetProperty("CompMips")->DataOffset;
	PropOffsets_Texture.DetailTexture = cls->GetProperty("DetailTexture")->DataOffset;
	PropOffsets_Texture.Diffuse = cls->GetProperty("Diffuse")->DataOffset;
	PropOffsets_Texture.DrawScale = cls->GetProperty("DrawScale")->DataOffset;
	PropOffsets_Texture.FootstepSound = cls->GetProperty("FootstepSound")->DataOffset;
	PropOffsets_Texture.Friction = cls->GetProperty("Friction")->DataOffset;
	PropOffsets_Texture.HitSound = cls->GetProperty("HitSound")->DataOffset;
	PropOffsets_Texture.LODSet = cls->GetProperty("LODSet")->DataOffset;
	PropOffsets_Texture.MacroTexture = cls->GetProperty("MacroTexture")->DataOffset;
	PropOffsets_Texture.MaxFrameRate = cls->GetProperty("MaxFrameRate")->DataOffset;
	PropOffsets_Texture.MinFrameRate = cls->GetProperty("MinFrameRate")->DataOffset;
	PropOffsets_Texture.MipMult = cls->GetProperty("MipMult")->DataOffset;
	PropOffsets_Texture.Mips = cls->GetProperty("Mips")->DataOffset;
	PropOffsets_Texture.PrimeCount = cls->GetProperty("PrimeCount")->DataOffset;
	PropOffsets_Texture.PrimeCurrent = cls->GetProperty("PrimeCurrent")->DataOffset;
	PropOffsets_Texture.Specular = cls->GetProperty("Specular")->DataOffset;
	PropOffsets_Texture.bAutoUPan = cls->GetProperty("bAutoUPan")->DataOffset;
	PropOffsets_Texture.bAutoVPan = cls->GetProperty("bAutoVPan")->DataOffset;
	PropOffsets_Texture.bBigWavy = cls->GetProperty("bBigWavy")->DataOffset;
	PropOffsets_Texture.bCloudWavy = cls->GetProperty("bCloudWavy")->DataOffset;
	PropOffsets_Texture.bDirtyShadows = cls->GetProperty("bDirtyShadows")->DataOffset;
	PropOffsets_Texture.bEnvironment = cls->GetProperty("bEnvironment")->DataOffset;
	PropOffsets_Texture.bFakeBackdrop = cls->GetProperty("bFakeBackdrop")->DataOffset;
	PropOffsets_Texture.bGouraud = cls->GetProperty("bGouraud")->DataOffset;
	PropOffsets_Texture.bHasComp = cls->GetProperty("bHasComp")->DataOffset;
	PropOffsets_Texture.bHighColorQuality = cls->GetProperty("bHighColorQuality")->DataOffset;
	PropOffsets_Texture.bHighLedge = cls->GetProperty("bHighLedge")->DataOffset;
	PropOffsets_Texture.bHighShadowDetail = cls->GetProperty("bHighShadowDetail")->DataOffset;
	PropOffsets_Texture.bHighTextureQuality = cls->GetProperty("bHighTextureQuality")->DataOffset;
	PropOffsets_Texture.bInvisible = cls->GetProperty("bInvisible")->DataOffset;
	PropOffsets_Texture.bLowShadowDetail = cls->GetProperty("bLowShadowDetail")->DataOffset;
	PropOffsets_Texture.bMasked = cls->GetProperty("bMasked")->DataOffset;
	PropOffsets_Texture.bMirrored = cls->GetProperty("bMirrored")->DataOffset;
	PropOffsets_Texture.bModulate = cls->GetProperty("bModulate")->DataOffset;
	PropOffsets_Texture.bNoMerge = cls->GetProperty("bNoMerge")->DataOffset;
	PropOffsets_Texture.bNoSmooth = cls->GetProperty("bNoSmooth")->DataOffset;
	PropOffsets_Texture.bNotSolid = cls->GetProperty("bNotSolid")->DataOffset;
	PropOffsets_Texture.bParametric = cls->GetProperty("bParametric")->DataOffset;
	PropOffsets_Texture.bPortal = cls->GetProperty("bPortal")->DataOffset;
	PropOffsets_Texture.bRealtime = cls->GetProperty("bRealtime")->DataOffset;
	PropOffsets_Texture.bRealtimeChanged = cls->GetProperty("bRealtimeChanged")->DataOffset;
	PropOffsets_Texture.bSemisolid = cls->GetProperty("bSemisolid")->DataOffset;
	PropOffsets_Texture.bSmallWavy = cls->GetProperty("bSmallWavy")->DataOffset;
	PropOffsets_Texture.bSpecialLit = cls->GetProperty("bSpecialLit")->DataOffset;
	PropOffsets_Texture.bTransparent = cls->GetProperty("bTransparent")->DataOffset;
	PropOffsets_Texture.bTwoSided = cls->GetProperty("bTwoSided")->DataOffset;
	PropOffsets_Texture.bUnlit = cls->GetProperty("bUnlit")->DataOffset;
	PropOffsets_Texture.bWaterWavy = cls->GetProperty("bWaterWavy")->DataOffset;
	PropOffsets_Texture.bX2 = cls->GetProperty("bX2")->DataOffset;
	PropOffsets_Texture.bX3 = cls->GetProperty("bX3")->DataOffset;
	PropOffsets_Texture.bX4 = cls->GetProperty("bX4")->DataOffset;
	PropOffsets_Texture.bX5 = cls->GetProperty("bX5")->DataOffset;
	PropOffsets_Texture.bX6 = cls->GetProperty("bX6")->DataOffset;
	PropOffsets_Texture.bX7 = cls->GetProperty("bX7")->DataOffset;
}

PropertyOffsets_Ammo PropOffsets_Ammo;

static void InitPropertyOffsets_Ammo(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Ammo"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Ammo");
	PropOffsets_Ammo.AmmoAmount = cls->GetProperty("AmmoAmount")->DataOffset;
	PropOffsets_Ammo.MaxAmmo = cls->GetProperty("MaxAmmo")->DataOffset;
	PropOffsets_Ammo.PAmmo = cls->GetProperty("PAmmo")->DataOffset;
	PropOffsets_Ammo.ParentAmmo = cls->GetProperty("ParentAmmo")->DataOffset;
	PropOffsets_Ammo.UsedInWeaponSlot = cls->GetProperty("UsedInWeaponSlot")->DataOffset;
}

PropertyOffsets_NavigationPoint PropOffsets_NavigationPoint;

static void InitPropertyOffsets_NavigationPoint(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "NavigationPoint"));
	if (!cls)
		throw std::runtime_error("Could not find class object for NavigationPoint");
	PropOffsets_NavigationPoint.ExtraCost = cls->GetProperty("ExtraCost")->DataOffset;
	PropOffsets_NavigationPoint.Paths = cls->GetProperty("Paths")->DataOffset;
	PropOffsets_NavigationPoint.PrunedPaths = cls->GetProperty("PrunedPaths")->DataOffset;
	PropOffsets_NavigationPoint.RouteCache = cls->GetProperty("RouteCache")->DataOffset;
	PropOffsets_NavigationPoint.VisNoReachPaths = cls->GetProperty("VisNoReachPaths")->DataOffset;
	PropOffsets_NavigationPoint.bAutoBuilt = cls->GetProperty("bAutoBuilt")->DataOffset;
	PropOffsets_NavigationPoint.bEndPoint = cls->GetProperty("bEndPoint")->DataOffset;
	PropOffsets_NavigationPoint.bEndPointOnly = cls->GetProperty("bEndPointOnly")->DataOffset;
	PropOffsets_NavigationPoint.bNeverUseStrafing = cls->GetProperty("bNeverUseStrafing")->DataOffset;
	PropOffsets_NavigationPoint.bOneWayPath = cls->GetProperty("bOneWayPath")->DataOffset;
	PropOffsets_NavigationPoint.bPlayerOnly = cls->GetProperty("bPlayerOnly")->DataOffset;
	PropOffsets_NavigationPoint.bSpecialCost = cls->GetProperty("bSpecialCost")->DataOffset;
	PropOffsets_NavigationPoint.bTwoWay = cls->GetProperty("bTwoWay")->DataOffset;
	PropOffsets_NavigationPoint.bestPathWeight = cls->GetProperty("bestPathWeight")->DataOffset;
	PropOffsets_NavigationPoint.cost = cls->GetProperty("cost")->DataOffset;
	PropOffsets_NavigationPoint.nextNavigationPoint = cls->GetProperty("nextNavigationPoint")->DataOffset;
	PropOffsets_NavigationPoint.nextOrdered = cls->GetProperty("nextOrdered")->DataOffset;
	PropOffsets_NavigationPoint.ownerTeam = cls->GetProperty("ownerTeam")->DataOffset;
	PropOffsets_NavigationPoint.prevOrdered = cls->GetProperty("prevOrdered")->DataOffset;
	PropOffsets_NavigationPoint.previousPath = cls->GetProperty("previousPath")->DataOffset;
	PropOffsets_NavigationPoint.startPath = cls->GetProperty("startPath")->DataOffset;
	PropOffsets_NavigationPoint.taken = cls->GetProperty("taken")->DataOffset;
	PropOffsets_NavigationPoint.upstreamPaths = cls->GetProperty("upstreamPaths")->DataOffset;
	PropOffsets_NavigationPoint.visitedWeight = cls->GetProperty("visitedWeight")->DataOffset;
}

PropertyOffsets_Mutator PropOffsets_Mutator;

static void InitPropertyOffsets_Mutator(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Mutator"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Mutator");
	PropOffsets_Mutator.DefaultWeapon = cls->GetProperty("DefaultWeapon")->DataOffset;
	PropOffsets_Mutator.NextDamageMutator = cls->GetProperty("NextDamageMutator")->DataOffset;
	PropOffsets_Mutator.NextHUDMutator = cls->GetProperty("NextHUDMutator")->DataOffset;
	PropOffsets_Mutator.NextMessageMutator = cls->GetProperty("NextMessageMutator")->DataOffset;
	PropOffsets_Mutator.NextMutator = cls->GetProperty("NextMutator")->DataOffset;
	PropOffsets_Mutator.bHUDMutator = cls->GetProperty("bHUDMutator")->DataOffset;
}

PropertyOffsets_Mover PropOffsets_Mover;

static void InitPropertyOffsets_Mover(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Mover"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Mover");
	PropOffsets_Mover.BasePos = cls->GetProperty("BasePos")->DataOffset;
	PropOffsets_Mover.BaseRot = cls->GetProperty("BaseRot")->DataOffset;
	PropOffsets_Mover.BrushRaytraceKey = cls->GetProperty("BrushRaytraceKey")->DataOffset;
	PropOffsets_Mover.BumpEvent = cls->GetProperty("BumpEvent")->DataOffset;
	PropOffsets_Mover.BumpType = cls->GetProperty("BumpType")->DataOffset;
	PropOffsets_Mover.ClientUpdate = cls->GetProperty("ClientUpdate")->DataOffset;
	PropOffsets_Mover.ClosedSound = cls->GetProperty("ClosedSound")->DataOffset;
	PropOffsets_Mover.ClosingSound = cls->GetProperty("ClosingSound")->DataOffset;
	PropOffsets_Mover.DamageThreshold = cls->GetProperty("DamageThreshold")->DataOffset;
	PropOffsets_Mover.DelayTime = cls->GetProperty("DelayTime")->DataOffset;
	PropOffsets_Mover.EncroachDamage = cls->GetProperty("EncroachDamage")->DataOffset;
	PropOffsets_Mover.Follower = cls->GetProperty("Follower")->DataOffset;
	PropOffsets_Mover.KeyNum = cls->GetProperty("KeyNum")->DataOffset;
	PropOffsets_Mover.KeyPos = cls->GetProperty("KeyPos")->DataOffset;
	PropOffsets_Mover.KeyRot = cls->GetProperty("KeyRot")->DataOffset;
	PropOffsets_Mover.Leader = cls->GetProperty("Leader")->DataOffset;
	PropOffsets_Mover.MoveAmbientSound = cls->GetProperty("MoveAmbientSound")->DataOffset;
	PropOffsets_Mover.MoveTime = cls->GetProperty("MoveTime")->DataOffset;
	PropOffsets_Mover.MoverEncroachType = cls->GetProperty("MoverEncroachType")->DataOffset;
	PropOffsets_Mover.MoverGlideType = cls->GetProperty("MoverGlideType")->DataOffset;
	PropOffsets_Mover.NumKeys = cls->GetProperty("NumKeys")->DataOffset;
	PropOffsets_Mover.OldPos = cls->GetProperty("OldPos")->DataOffset;
	PropOffsets_Mover.OldPrePivot = cls->GetProperty("OldPrePivot")->DataOffset;
	PropOffsets_Mover.OldRot = cls->GetProperty("OldRot")->DataOffset;
	PropOffsets_Mover.OpenedSound = cls->GetProperty("OpenedSound")->DataOffset;
	PropOffsets_Mover.OpeningSound = cls->GetProperty("OpeningSound")->DataOffset;
	PropOffsets_Mover.OtherTime = cls->GetProperty("OtherTime")->DataOffset;
	PropOffsets_Mover.PlayerBumpEvent = cls->GetProperty("PlayerBumpEvent")->DataOffset;
	PropOffsets_Mover.PrevKeyNum = cls->GetProperty("PrevKeyNum")->DataOffset;
	PropOffsets_Mover.RealPosition = cls->GetProperty("RealPosition")->DataOffset;
	PropOffsets_Mover.RealRotation = cls->GetProperty("RealRotation")->DataOffset;
	PropOffsets_Mover.RecommendedTrigger = cls->GetProperty("RecommendedTrigger")->DataOffset;
	PropOffsets_Mover.ReturnGroup = cls->GetProperty("ReturnGroup")->DataOffset;
	PropOffsets_Mover.SavedPos = cls->GetProperty("SavedPos")->DataOffset;
	PropOffsets_Mover.SavedRot = cls->GetProperty("SavedRot")->DataOffset;
	PropOffsets_Mover.SavedTrigger = cls->GetProperty("SavedTrigger")->DataOffset;
	PropOffsets_Mover.SimInterpolate = cls->GetProperty("SimInterpolate")->DataOffset;
	PropOffsets_Mover.SimOldPos = cls->GetProperty("SimOldPos")->DataOffset;
	PropOffsets_Mover.SimOldRotPitch = cls->GetProperty("SimOldRotPitch")->DataOffset;
	PropOffsets_Mover.SimOldRotRoll = cls->GetProperty("SimOldRotRoll")->DataOffset;
	PropOffsets_Mover.SimOldRotYaw = cls->GetProperty("SimOldRotYaw")->DataOffset;
	PropOffsets_Mover.StayOpenTime = cls->GetProperty("StayOpenTime")->DataOffset;
	PropOffsets_Mover.TriggerActor = cls->GetProperty("TriggerActor")->DataOffset;
	PropOffsets_Mover.TriggerActor2 = cls->GetProperty("TriggerActor2")->DataOffset;
	PropOffsets_Mover.WaitingPawn = cls->GetProperty("WaitingPawn")->DataOffset;
	PropOffsets_Mover.WorldRaytraceKey = cls->GetProperty("WorldRaytraceKey")->DataOffset;
	PropOffsets_Mover.bClientPause = cls->GetProperty("bClientPause")->DataOffset;
	PropOffsets_Mover.bDamageTriggered = cls->GetProperty("bDamageTriggered")->DataOffset;
	PropOffsets_Mover.bDelaying = cls->GetProperty("bDelaying")->DataOffset;
	PropOffsets_Mover.bDynamicLightMover = cls->GetProperty("bDynamicLightMover")->DataOffset;
	PropOffsets_Mover.bOpening = cls->GetProperty("bOpening")->DataOffset;
	PropOffsets_Mover.bPlayerOnly = cls->GetProperty("bPlayerOnly")->DataOffset;
	PropOffsets_Mover.bSlave = cls->GetProperty("bSlave")->DataOffset;
	PropOffsets_Mover.bTriggerOnceOnly = cls->GetProperty("bTriggerOnceOnly")->DataOffset;
	PropOffsets_Mover.bUseTriggered = cls->GetProperty("bUseTriggered")->DataOffset;
	PropOffsets_Mover.myMarker = cls->GetProperty("myMarker")->DataOffset;
	PropOffsets_Mover.numTriggerEvents = cls->GetProperty("numTriggerEvents")->DataOffset;
}

PropertyOffsets_HUD PropOffsets_HUD;

static void InitPropertyOffsets_HUD(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "HUD"));
	if (!cls)
		throw std::runtime_error("Could not find class object for HUD");
	PropOffsets_HUD.Crosshair = cls->GetProperty("Crosshair")->DataOffset;
	PropOffsets_HUD.HUDConfigWindowType = cls->GetProperty("HUDConfigWindowType")->DataOffset;
	PropOffsets_HUD.HUDMutator = cls->GetProperty("HUDMutator")->DataOffset;
	PropOffsets_HUD.HudMode = cls->GetProperty("HudMode")->DataOffset;
	PropOffsets_HUD.MainMenu = cls->GetProperty("MainMenu")->DataOffset;
	PropOffsets_HUD.MainMenuType = cls->GetProperty("MainMenuType")->DataOffset;
	PropOffsets_HUD.PlayerOwner = cls->GetProperty("PlayerOwner")->DataOffset;
	PropOffsets_HUD.WhiteColor = cls->GetProperty("WhiteColor")->DataOffset;
}

PropertyOffsets_Decoration PropOffsets_Decoration;

static void InitPropertyOffsets_Decoration(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Decoration"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Decoration");
	PropOffsets_Decoration.EffectWhenDestroyed = cls->GetProperty("EffectWhenDestroyed")->DataOffset;
	PropOffsets_Decoration.EndPushSound = cls->GetProperty("EndPushSound")->DataOffset;
	PropOffsets_Decoration.PushSound = cls->GetProperty("PushSound")->DataOffset;
	PropOffsets_Decoration.bBobbing = cls->GetProperty("bBobbing")->DataOffset;
	PropOffsets_Decoration.bOnlyTriggerable = cls->GetProperty("bOnlyTriggerable")->DataOffset;
	PropOffsets_Decoration.bPushSoundPlaying = cls->GetProperty("bPushSoundPlaying")->DataOffset;
	PropOffsets_Decoration.bPushable = cls->GetProperty("bPushable")->DataOffset;
	PropOffsets_Decoration.bSplash = cls->GetProperty("bSplash")->DataOffset;
	PropOffsets_Decoration.bWasCarried = cls->GetProperty("bWasCarried")->DataOffset;
	PropOffsets_Decoration.content2 = cls->GetProperty("content2")->DataOffset;
	PropOffsets_Decoration.content3 = cls->GetProperty("content3")->DataOffset;
	PropOffsets_Decoration.contents = cls->GetProperty("contents")->DataOffset;
	PropOffsets_Decoration.numLandings = cls->GetProperty("numLandings")->DataOffset;
}

PropertyOffsets_TestInfo PropOffsets_TestInfo;

static void InitPropertyOffsets_TestInfo(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "TestInfo"));
	if (!cls)
		throw std::runtime_error("Could not find class object for TestInfo");
	PropOffsets_TestInfo.MyArray = cls->GetProperty("MyArray")->DataOffset;
	PropOffsets_TestInfo.ST = cls->GetProperty("ST")->DataOffset;
	PropOffsets_TestInfo.TestRepStr = cls->GetProperty("TestRepStr")->DataOffset;
	PropOffsets_TestInfo.bBool1 = cls->GetProperty("bBool1")->DataOffset;
	PropOffsets_TestInfo.bBool2 = cls->GetProperty("bBool2")->DataOffset;
	PropOffsets_TestInfo.bFalse1 = cls->GetProperty("bFalse1")->DataOffset;
	PropOffsets_TestInfo.bFalse2 = cls->GetProperty("bFalse2")->DataOffset;
	PropOffsets_TestInfo.bTrue1 = cls->GetProperty("bTrue1")->DataOffset;
	PropOffsets_TestInfo.bTrue2 = cls->GetProperty("bTrue2")->DataOffset;
	PropOffsets_TestInfo.ppp = cls->GetProperty("ppp")->DataOffset;
	PropOffsets_TestInfo.sxx = cls->GetProperty("sxx")->DataOffset;
	PropOffsets_TestInfo.v1 = cls->GetProperty("v1")->DataOffset;
	PropOffsets_TestInfo.v2 = cls->GetProperty("v2")->DataOffset;
	PropOffsets_TestInfo.xnum = cls->GetProperty("xnum")->DataOffset;
}

PropertyOffsets_GameReplicationInfo PropOffsets_GameReplicationInfo;

static void InitPropertyOffsets_GameReplicationInfo(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "GameReplicationInfo"));
	if (!cls)
		throw std::runtime_error("Could not find class object for GameReplicationInfo");
	PropOffsets_GameReplicationInfo.AdminEmail = cls->GetProperty("AdminEmail")->DataOffset;
	PropOffsets_GameReplicationInfo.AdminName = cls->GetProperty("AdminName")->DataOffset;
	PropOffsets_GameReplicationInfo.ElapsedTime = cls->GetProperty("ElapsedTime")->DataOffset;
	PropOffsets_GameReplicationInfo.GameClass = cls->GetProperty("GameClass")->DataOffset;
	PropOffsets_GameReplicationInfo.GameEndedComments = cls->GetProperty("GameEndedComments")->DataOffset;
	PropOffsets_GameReplicationInfo.GameName = cls->GetProperty("GameName")->DataOffset;
	PropOffsets_GameReplicationInfo.MOTDLine1 = cls->GetProperty("MOTDLine1")->DataOffset;
	PropOffsets_GameReplicationInfo.MOTDLine2 = cls->GetProperty("MOTDLine2")->DataOffset;
	PropOffsets_GameReplicationInfo.MOTDLine3 = cls->GetProperty("MOTDLine3")->DataOffset;
	PropOffsets_GameReplicationInfo.MOTDLine4 = cls->GetProperty("MOTDLine4")->DataOffset;
	PropOffsets_GameReplicationInfo.NumPlayers = cls->GetProperty("NumPlayers")->DataOffset;
	PropOffsets_GameReplicationInfo.PRIArray = cls->GetProperty("PRIArray")->DataOffset;
	PropOffsets_GameReplicationInfo.Region = cls->GetProperty("Region")->DataOffset;
	PropOffsets_GameReplicationInfo.RemainingMinute = cls->GetProperty("RemainingMinute")->DataOffset;
	PropOffsets_GameReplicationInfo.RemainingTime = cls->GetProperty("RemainingTime")->DataOffset;
	PropOffsets_GameReplicationInfo.SecondCount = cls->GetProperty("SecondCount")->DataOffset;
	PropOffsets_GameReplicationInfo.ServerName = cls->GetProperty("ServerName")->DataOffset;
	PropOffsets_GameReplicationInfo.ShortName = cls->GetProperty("ShortName")->DataOffset;
	PropOffsets_GameReplicationInfo.SumFrags = cls->GetProperty("SumFrags")->DataOffset;
	PropOffsets_GameReplicationInfo.UpdateTimer = cls->GetProperty("UpdateTimer")->DataOffset;
	PropOffsets_GameReplicationInfo.bClassicDeathMessages = cls->GetProperty("bClassicDeathMessages")->DataOffset;
	PropOffsets_GameReplicationInfo.bStopCountDown = cls->GetProperty("bStopCountDown")->DataOffset;
	PropOffsets_GameReplicationInfo.bTeamGame = cls->GetProperty("bTeamGame")->DataOffset;
}

PropertyOffsets_Menu PropOffsets_Menu;

static void InitPropertyOffsets_Menu(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Menu"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Menu");
	PropOffsets_Menu.CenterString = cls->GetProperty("CenterString")->DataOffset;
	PropOffsets_Menu.DisabledString = cls->GetProperty("DisabledString")->DataOffset;
	PropOffsets_Menu.EnabledString = cls->GetProperty("EnabledString")->DataOffset;
	PropOffsets_Menu.HelpMessage = cls->GetProperty("HelpMessage")->DataOffset;
	PropOffsets_Menu.LeftString = cls->GetProperty("LeftString")->DataOffset;
	PropOffsets_Menu.MenuLength = cls->GetProperty("MenuLength")->DataOffset;
	PropOffsets_Menu.MenuList = cls->GetProperty("MenuList")->DataOffset;
	PropOffsets_Menu.MenuTitle = cls->GetProperty("MenuTitle")->DataOffset;
	PropOffsets_Menu.NoString = cls->GetProperty("NoString")->DataOffset;
	PropOffsets_Menu.ParentMenu = cls->GetProperty("ParentMenu")->DataOffset;
	PropOffsets_Menu.PlayerOwner = cls->GetProperty("PlayerOwner")->DataOffset;
	PropOffsets_Menu.RightString = cls->GetProperty("RightString")->DataOffset;
	PropOffsets_Menu.Selection = cls->GetProperty("Selection")->DataOffset;
	PropOffsets_Menu.YesString = cls->GetProperty("YesString")->DataOffset;
	PropOffsets_Menu.bConfigChanged = cls->GetProperty("bConfigChanged")->DataOffset;
	PropOffsets_Menu.bExitAllMenus = cls->GetProperty("bExitAllMenus")->DataOffset;
}

PropertyOffsets_LiftExit PropOffsets_LiftExit;

static void InitPropertyOffsets_LiftExit(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LiftExit"));
	if (!cls)
		throw std::runtime_error("Could not find class object for LiftExit");
	PropOffsets_LiftExit.LastTriggerTime = cls->GetProperty("LastTriggerTime")->DataOffset;
	PropOffsets_LiftExit.LiftTag = cls->GetProperty("LiftTag")->DataOffset;
	PropOffsets_LiftExit.LiftTrigger = cls->GetProperty("LiftTrigger")->DataOffset;
	PropOffsets_LiftExit.MyLift = cls->GetProperty("MyLift")->DataOffset;
	PropOffsets_LiftExit.RecommendedTrigger = cls->GetProperty("RecommendedTrigger")->DataOffset;
}

PropertyOffsets_Trigger PropOffsets_Trigger;

static void InitPropertyOffsets_Trigger(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Trigger"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Trigger");
	PropOffsets_Trigger.ClassProximityType = cls->GetProperty("ClassProximityType")->DataOffset;
	PropOffsets_Trigger.DamageThreshold = cls->GetProperty("DamageThreshold")->DataOffset;
	PropOffsets_Trigger.Message = cls->GetProperty("Message")->DataOffset;
	PropOffsets_Trigger.ReTriggerDelay = cls->GetProperty("ReTriggerDelay")->DataOffset;
	PropOffsets_Trigger.RepeatTriggerTime = cls->GetProperty("RepeatTriggerTime")->DataOffset;
	PropOffsets_Trigger.TriggerActor = cls->GetProperty("TriggerActor")->DataOffset;
	PropOffsets_Trigger.TriggerActor2 = cls->GetProperty("TriggerActor2")->DataOffset;
	PropOffsets_Trigger.TriggerTime = cls->GetProperty("TriggerTime")->DataOffset;
	PropOffsets_Trigger.TriggerType = cls->GetProperty("TriggerType")->DataOffset;
	PropOffsets_Trigger.bInitiallyActive = cls->GetProperty("bInitiallyActive")->DataOffset;
	PropOffsets_Trigger.bTriggerOnceOnly = cls->GetProperty("bTriggerOnceOnly")->DataOffset;
}

PropertyOffsets_Player PropOffsets_Player;

static void InitPropertyOffsets_Player(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Player"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Player");
	PropOffsets_Player.Actor = cls->GetProperty("Actor")->DataOffset;
	PropOffsets_Player.ConfiguredInternetSpeed = cls->GetProperty("ConfiguredInternetSpeed")->DataOffset;
	PropOffsets_Player.ConfiguredLanSpeed = cls->GetProperty("ConfiguredLanSpeed")->DataOffset;
	PropOffsets_Player.Console = cls->GetProperty("Console")->DataOffset;
	PropOffsets_Player.CurrentNetSpeed = cls->GetProperty("CurrentNetSpeed")->DataOffset;
	PropOffsets_Player.SelectedCursor = cls->GetProperty("SelectedCursor")->DataOffset;
	PropOffsets_Player.WindowsMouseX = cls->GetProperty("WindowsMouseX")->DataOffset;
	PropOffsets_Player.WindowsMouseY = cls->GetProperty("WindowsMouseY")->DataOffset;
	PropOffsets_Player.bShowWindowsMouse = cls->GetProperty("bShowWindowsMouse")->DataOffset;
	PropOffsets_Player.bSuspendPrecaching = cls->GetProperty("bSuspendPrecaching")->DataOffset;
	PropOffsets_Player.bWindowsMouseAvailable = cls->GetProperty("bWindowsMouseAvailable")->DataOffset;
	PropOffsets_Player.vfExec = cls->GetProperty("vfExec")->DataOffset;
	PropOffsets_Player.vfOut = cls->GetProperty("vfOut")->DataOffset;
}

PropertyOffsets_LocalMessage PropOffsets_LocalMessage;

static void InitPropertyOffsets_LocalMessage(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LocalMessage"));
	if (!cls)
		throw std::runtime_error("Could not find class object for LocalMessage");
	PropOffsets_LocalMessage.ChildMessage = cls->GetProperty("ChildMessage")->DataOffset;
	PropOffsets_LocalMessage.DrawColor = cls->GetProperty("DrawColor")->DataOffset;
	PropOffsets_LocalMessage.Lifetime = cls->GetProperty("Lifetime")->DataOffset;
	PropOffsets_LocalMessage.XPos = cls->GetProperty("XPos")->DataOffset;
	PropOffsets_LocalMessage.YPos = cls->GetProperty("YPos")->DataOffset;
	PropOffsets_LocalMessage.bBeep = cls->GetProperty("bBeep")->DataOffset;
	PropOffsets_LocalMessage.bCenter = cls->GetProperty("bCenter")->DataOffset;
	PropOffsets_LocalMessage.bComplexString = cls->GetProperty("bComplexString")->DataOffset;
	PropOffsets_LocalMessage.bFadeMessage = cls->GetProperty("bFadeMessage")->DataOffset;
	PropOffsets_LocalMessage.bFromBottom = cls->GetProperty("bFromBottom")->DataOffset;
	PropOffsets_LocalMessage.bIsConsoleMessage = cls->GetProperty("bIsConsoleMessage")->DataOffset;
	PropOffsets_LocalMessage.bIsSpecial = cls->GetProperty("bIsSpecial")->DataOffset;
	PropOffsets_LocalMessage.bIsUnique = cls->GetProperty("bIsUnique")->DataOffset;
	PropOffsets_LocalMessage.bOffsetYPos = cls->GetProperty("bOffsetYPos")->DataOffset;
}

PropertyOffsets_locationid PropOffsets_locationid;

static void InitPropertyOffsets_locationid(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "locationid"));
	if (!cls)
		throw std::runtime_error("Could not find class object for locationid");
	PropOffsets_locationid.LocationName = cls->GetProperty("LocationName")->DataOffset;
	PropOffsets_locationid.NextLocation = cls->GetProperty("NextLocation")->DataOffset;
	PropOffsets_locationid.Radius = cls->GetProperty("Radius")->DataOffset;
}

PropertyOffsets_Carcass PropOffsets_Carcass;

static void InitPropertyOffsets_Carcass(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Carcass"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Carcass");
	PropOffsets_Carcass.Bugs = cls->GetProperty("Bugs")->DataOffset;
	PropOffsets_Carcass.CumulativeDamage = cls->GetProperty("CumulativeDamage")->DataOffset;
	PropOffsets_Carcass.PlayerOwner = cls->GetProperty("PlayerOwner")->DataOffset;
	PropOffsets_Carcass.bDecorative = cls->GetProperty("bDecorative")->DataOffset;
	PropOffsets_Carcass.bPlayerCarcass = cls->GetProperty("bPlayerCarcass")->DataOffset;
	PropOffsets_Carcass.bReducedHeight = cls->GetProperty("bReducedHeight")->DataOffset;
	PropOffsets_Carcass.bSlidingCarcass = cls->GetProperty("bSlidingCarcass")->DataOffset;
	PropOffsets_Carcass.flies = cls->GetProperty("flies")->DataOffset;
	PropOffsets_Carcass.rats = cls->GetProperty("rats")->DataOffset;
}

PropertyOffsets_InterpolationPoint PropOffsets_InterpolationPoint;

static void InitPropertyOffsets_InterpolationPoint(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "InterpolationPoint"));
	if (!cls)
		throw std::runtime_error("Could not find class object for InterpolationPoint");
	PropOffsets_InterpolationPoint.FovModifier = cls->GetProperty("FovModifier")->DataOffset;
	PropOffsets_InterpolationPoint.GameSpeedModifier = cls->GetProperty("GameSpeedModifier")->DataOffset;
	PropOffsets_InterpolationPoint.Next = cls->GetProperty("Next")->DataOffset;
	PropOffsets_InterpolationPoint.Position = cls->GetProperty("Position")->DataOffset;
	PropOffsets_InterpolationPoint.Prev = cls->GetProperty("Prev")->DataOffset;
	PropOffsets_InterpolationPoint.RateModifier = cls->GetProperty("RateModifier")->DataOffset;
	PropOffsets_InterpolationPoint.ScreenFlashFog = cls->GetProperty("ScreenFlashFog")->DataOffset;
	PropOffsets_InterpolationPoint.ScreenFlashScale = cls->GetProperty("ScreenFlashScale")->DataOffset;
	PropOffsets_InterpolationPoint.bEndOfPath = cls->GetProperty("bEndOfPath")->DataOffset;
	PropOffsets_InterpolationPoint.bSkipNextPath = cls->GetProperty("bSkipNextPath")->DataOffset;
}

PropertyOffsets_Projectile PropOffsets_Projectile;

static void InitPropertyOffsets_Projectile(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Projectile"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Projectile");
	PropOffsets_Projectile.Damage = cls->GetProperty("Damage")->DataOffset;
	PropOffsets_Projectile.ExploWallOut = cls->GetProperty("ExploWallOut")->DataOffset;
	PropOffsets_Projectile.ExplosionDecal = cls->GetProperty("ExplosionDecal")->DataOffset;
	PropOffsets_Projectile.ImpactSound = cls->GetProperty("ImpactSound")->DataOffset;
	PropOffsets_Projectile.MaxSpeed = cls->GetProperty("MaxSpeed")->DataOffset;
	PropOffsets_Projectile.MiscSound = cls->GetProperty("MiscSound")->DataOffset;
	PropOffsets_Projectile.MomentumTransfer = cls->GetProperty("MomentumTransfer")->DataOffset;
	PropOffsets_Projectile.MyDamageType = cls->GetProperty("MyDamageType")->DataOffset;
	PropOffsets_Projectile.SpawnSound = cls->GetProperty("SpawnSound")->DataOffset;
	PropOffsets_Projectile.speed = cls->GetProperty("speed")->DataOffset;
}

PropertyOffsets_Teleporter PropOffsets_Teleporter;

static void InitPropertyOffsets_Teleporter(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Teleporter"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Teleporter");
	PropOffsets_Teleporter.LastFired = cls->GetProperty("LastFired")->DataOffset;
	PropOffsets_Teleporter.ProductRequired = cls->GetProperty("ProductRequired")->DataOffset;
	PropOffsets_Teleporter.TargetVelocity = cls->GetProperty("TargetVelocity")->DataOffset;
	PropOffsets_Teleporter.TriggerActor = cls->GetProperty("TriggerActor")->DataOffset;
	PropOffsets_Teleporter.TriggerActor2 = cls->GetProperty("TriggerActor2")->DataOffset;
	PropOffsets_Teleporter.URL = cls->GetProperty("URL")->DataOffset;
	PropOffsets_Teleporter.bChangesVelocity = cls->GetProperty("bChangesVelocity")->DataOffset;
	PropOffsets_Teleporter.bChangesYaw = cls->GetProperty("bChangesYaw")->DataOffset;
	PropOffsets_Teleporter.bEnabled = cls->GetProperty("bEnabled")->DataOffset;
	PropOffsets_Teleporter.bReversesX = cls->GetProperty("bReversesX")->DataOffset;
	PropOffsets_Teleporter.bReversesY = cls->GetProperty("bReversesY")->DataOffset;
	PropOffsets_Teleporter.bReversesZ = cls->GetProperty("bReversesZ")->DataOffset;
}

PropertyOffsets_Palette PropOffsets_Palette;

static void InitPropertyOffsets_Palette(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Palette"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Palette");
	PropOffsets_Palette.Colors = cls->GetProperty("Colors")->DataOffset;
}

PropertyOffsets_SpawnNotify PropOffsets_SpawnNotify;

static void InitPropertyOffsets_SpawnNotify(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "SpawnNotify"));
	if (!cls)
		throw std::runtime_error("Could not find class object for SpawnNotify");
	PropOffsets_SpawnNotify.ActorClass = cls->GetProperty("ActorClass")->DataOffset;
	PropOffsets_SpawnNotify.Next = cls->GetProperty("Next")->DataOffset;
}

PropertyOffsets_Fragment PropOffsets_Fragment;

static void InitPropertyOffsets_Fragment(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Fragment"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Fragment");
	PropOffsets_Fragment.Fragments = cls->GetProperty("Fragments")->DataOffset;
	PropOffsets_Fragment.bFirstHit = cls->GetProperty("bFirstHit")->DataOffset;
	PropOffsets_Fragment.numFragmentTypes = cls->GetProperty("numFragmentTypes")->DataOffset;
}

PropertyOffsets_WarpZoneInfo PropOffsets_WarpZoneInfo;

static void InitPropertyOffsets_WarpZoneInfo(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "WarpZoneInfo"));
	if (!cls)
		throw std::runtime_error("Could not find class object for WarpZoneInfo");
	PropOffsets_WarpZoneInfo.Destinations = cls->GetProperty("Destinations")->DataOffset;
	PropOffsets_WarpZoneInfo.OtherSideActor = cls->GetProperty("OtherSideActor")->DataOffset;
	PropOffsets_WarpZoneInfo.OtherSideLevel = cls->GetProperty("OtherSideLevel")->DataOffset;
	PropOffsets_WarpZoneInfo.OtherSideURL = cls->GetProperty("OtherSideURL")->DataOffset;
	PropOffsets_WarpZoneInfo.ThisTag = cls->GetProperty("ThisTag")->DataOffset;
	PropOffsets_WarpZoneInfo.WarpCoords = cls->GetProperty("WarpCoords")->DataOffset;
	PropOffsets_WarpZoneInfo.bNoTeleFrag = cls->GetProperty("bNoTeleFrag")->DataOffset;
	PropOffsets_WarpZoneInfo.iWarpZone = cls->GetProperty("iWarpZone")->DataOffset;
	PropOffsets_WarpZoneInfo.numDestinations = cls->GetProperty("numDestinations")->DataOffset;
}

PropertyOffsets_Console PropOffsets_Console;

static void InitPropertyOffsets_Console(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Console"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Console");
	PropOffsets_Console.AvgText = cls->GetProperty("AvgText")->DataOffset;
	PropOffsets_Console.Border = cls->GetProperty("Border")->DataOffset;
	PropOffsets_Console.BorderLines = cls->GetProperty("BorderLines")->DataOffset;
	PropOffsets_Console.BorderPixels = cls->GetProperty("BorderPixels")->DataOffset;
	PropOffsets_Console.BorderSize = cls->GetProperty("BorderSize")->DataOffset;
	PropOffsets_Console.ConBackground = cls->GetProperty("ConBackground")->DataOffset;
	PropOffsets_Console.ConnectingMessage = cls->GetProperty("ConnectingMessage")->DataOffset;
	PropOffsets_Console.ConsoleDest = cls->GetProperty("ConsoleDest")->DataOffset;
	PropOffsets_Console.ConsoleLines = cls->GetProperty("ConsoleLines")->DataOffset;
	PropOffsets_Console.ConsolePos = cls->GetProperty("ConsolePos")->DataOffset;
	PropOffsets_Console.ExtraTime = cls->GetProperty("ExtraTime")->DataOffset;
	PropOffsets_Console.FrameCount = cls->GetProperty("FrameCount")->DataOffset;
	PropOffsets_Console.FrameRateText = cls->GetProperty("FrameRateText")->DataOffset;
	PropOffsets_Console.FrameX = cls->GetProperty("FrameX")->DataOffset;
	PropOffsets_Console.FrameY = cls->GetProperty("FrameY")->DataOffset;
	PropOffsets_Console.FramesText = cls->GetProperty("FramesText")->DataOffset;
	PropOffsets_Console.History = cls->GetProperty("History")->DataOffset;
	PropOffsets_Console.HistoryBot = cls->GetProperty("HistoryBot")->DataOffset;
	PropOffsets_Console.HistoryCur = cls->GetProperty("HistoryCur")->DataOffset;
	PropOffsets_Console.HistoryTop = cls->GetProperty("HistoryTop")->DataOffset;
	PropOffsets_Console.LastFrameTime = cls->GetProperty("LastFrameTime")->DataOffset;
	PropOffsets_Console.LastSecFPS = cls->GetProperty("LastSecFPS")->DataOffset;
	PropOffsets_Console.LastSecText = cls->GetProperty("LastSecText")->DataOffset;
	PropOffsets_Console.LastSecondFrameCount = cls->GetProperty("LastSecondFrameCount")->DataOffset;
	PropOffsets_Console.LastSecondStartTime = cls->GetProperty("LastSecondStartTime")->DataOffset;
	PropOffsets_Console.LoadingMessage = cls->GetProperty("LoadingMessage")->DataOffset;
	PropOffsets_Console.MaxFPS = cls->GetProperty("MaxFPS")->DataOffset;
	PropOffsets_Console.MaxText = cls->GetProperty("MaxText")->DataOffset;
	PropOffsets_Console.MinFPS = cls->GetProperty("MinFPS")->DataOffset;
	PropOffsets_Console.MinText = cls->GetProperty("MinText")->DataOffset;
	PropOffsets_Console.MsgPlayer = cls->GetProperty("MsgPlayer")->DataOffset;
	PropOffsets_Console.MsgText = cls->GetProperty("MsgText")->DataOffset;
	PropOffsets_Console.MsgTick = cls->GetProperty("MsgTick")->DataOffset;
	PropOffsets_Console.MsgTickTime = cls->GetProperty("MsgTickTime")->DataOffset;
	PropOffsets_Console.MsgTime = cls->GetProperty("MsgTime")->DataOffset;
	PropOffsets_Console.MsgType = cls->GetProperty("MsgType")->DataOffset;
	PropOffsets_Console.PausedMessage = cls->GetProperty("PausedMessage")->DataOffset;
	PropOffsets_Console.PrecachingMessage = cls->GetProperty("PrecachingMessage")->DataOffset;
	PropOffsets_Console.SavingMessage = cls->GetProperty("SavingMessage")->DataOffset;
	PropOffsets_Console.Scrollback = cls->GetProperty("Scrollback")->DataOffset;
	PropOffsets_Console.SecondsText = cls->GetProperty("SecondsText")->DataOffset;
	PropOffsets_Console.StartTime = cls->GetProperty("StartTime")->DataOffset;
	PropOffsets_Console.TextLines = cls->GetProperty("TextLines")->DataOffset;
	PropOffsets_Console.TimeDemoFont = cls->GetProperty("TimeDemoFont")->DataOffset;
	PropOffsets_Console.TopLine = cls->GetProperty("TopLine")->DataOffset;
	PropOffsets_Console.TypedStr = cls->GetProperty("TypedStr")->DataOffset;
	PropOffsets_Console.Viewport = cls->GetProperty("Viewport")->DataOffset;
	PropOffsets_Console.bNoDrawWorld = cls->GetProperty("bNoDrawWorld")->DataOffset;
	PropOffsets_Console.bNoStuff = cls->GetProperty("bNoStuff")->DataOffset;
	PropOffsets_Console.bRestartTimeDemo = cls->GetProperty("bRestartTimeDemo")->DataOffset;
	PropOffsets_Console.bSaveTimeDemoToFile = cls->GetProperty("bSaveTimeDemoToFile")->DataOffset;
	PropOffsets_Console.bStartTimeDemo = cls->GetProperty("bStartTimeDemo")->DataOffset;
	PropOffsets_Console.bTimeDemo = cls->GetProperty("bTimeDemo")->DataOffset;
	PropOffsets_Console.bTyping = cls->GetProperty("bTyping")->DataOffset;
	PropOffsets_Console.fpsText = cls->GetProperty("fpsText")->DataOffset;
	PropOffsets_Console.numLines = cls->GetProperty("numLines")->DataOffset;
	PropOffsets_Console.vtblOut = cls->GetProperty("vtblOut")->DataOffset;
}

PropertyOffsets_PlayerStart PropOffsets_PlayerStart;

static void InitPropertyOffsets_PlayerStart(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "PlayerStart"));
	if (!cls)
		throw std::runtime_error("Could not find class object for PlayerStart");
	PropOffsets_PlayerStart.TeamNumber = cls->GetProperty("TeamNumber")->DataOffset;
	PropOffsets_PlayerStart.bCoopStart = cls->GetProperty("bCoopStart")->DataOffset;
	PropOffsets_PlayerStart.bEnabled = cls->GetProperty("bEnabled")->DataOffset;
	PropOffsets_PlayerStart.bSinglePlayerStart = cls->GetProperty("bSinglePlayerStart")->DataOffset;
}

PropertyOffsets_Pickup PropOffsets_Pickup;

static void InitPropertyOffsets_Pickup(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Pickup"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Pickup");
	PropOffsets_Pickup.ExpireMessage = cls->GetProperty("ExpireMessage")->DataOffset;
	PropOffsets_Pickup.Inv = cls->GetProperty("Inv")->DataOffset;
	PropOffsets_Pickup.NumCopies = cls->GetProperty("NumCopies")->DataOffset;
	PropOffsets_Pickup.bAutoActivate = cls->GetProperty("bAutoActivate")->DataOffset;
	PropOffsets_Pickup.bCanActivate = cls->GetProperty("bCanActivate")->DataOffset;
	PropOffsets_Pickup.bCanHaveMultipleCopies = cls->GetProperty("bCanHaveMultipleCopies")->DataOffset;
}

PropertyOffsets_Brush PropOffsets_Brush;

static void InitPropertyOffsets_Brush(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Brush"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Brush");
	PropOffsets_Brush.BrushColor = cls->GetProperty("BrushColor")->DataOffset;
	PropOffsets_Brush.CsgOper = cls->GetProperty("CsgOper")->DataOffset;
	PropOffsets_Brush.MainScale = cls->GetProperty("MainScale")->DataOffset;
	PropOffsets_Brush.PolyFlags = cls->GetProperty("PolyFlags")->DataOffset;
	PropOffsets_Brush.PostPivot = cls->GetProperty("PostPivot")->DataOffset;
	PropOffsets_Brush.PostScale = cls->GetProperty("PostScale")->DataOffset;
	PropOffsets_Brush.TempScale = cls->GetProperty("TempScale")->DataOffset;
	PropOffsets_Brush.UnusedLightMesh = cls->GetProperty("UnusedLightMesh")->DataOffset;
	PropOffsets_Brush.bColored = cls->GetProperty("bColored")->DataOffset;
}

PropertyOffsets_ScoreBoard PropOffsets_ScoreBoard;

static void InitPropertyOffsets_ScoreBoard(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "ScoreBoard"));
	if (!cls)
		throw std::runtime_error("Could not find class object for ScoreBoard");
	PropOffsets_ScoreBoard.OwnerHUD = cls->GetProperty("OwnerHUD")->DataOffset;
	PropOffsets_ScoreBoard.RegFont = cls->GetProperty("RegFont")->DataOffset;
}

PropertyOffsets_Spectator PropOffsets_Spectator;

static void InitPropertyOffsets_Spectator(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Spectator"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Spectator");
	PropOffsets_Spectator.bChaseCam = cls->GetProperty("bChaseCam")->DataOffset;
}

PropertyOffsets_InventorySpot PropOffsets_InventorySpot;

static void InitPropertyOffsets_InventorySpot(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "InventorySpot"));
	if (!cls)
		throw std::runtime_error("Could not find class object for InventorySpot");
	PropOffsets_InventorySpot.markedItem = cls->GetProperty("markedItem")->DataOffset;
}

PropertyOffsets_Decal PropOffsets_Decal;

static void InitPropertyOffsets_Decal(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Decal"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Decal");
	PropOffsets_Decal.LastRenderedTime = cls->GetProperty("LastRenderedTime")->DataOffset;
	PropOffsets_Decal.MultiDecalLevel = cls->GetProperty("MultiDecalLevel")->DataOffset;
	PropOffsets_Decal.SurfList = cls->GetProperty("SurfList")->DataOffset;
}

PropertyOffsets_PatrolPoint PropOffsets_PatrolPoint;

static void InitPropertyOffsets_PatrolPoint(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "PatrolPoint"));
	if (!cls)
		throw std::runtime_error("Could not find class object for PatrolPoint");
	PropOffsets_PatrolPoint.AnimCount = cls->GetProperty("AnimCount")->DataOffset;
	PropOffsets_PatrolPoint.NextPatrolPoint = cls->GetProperty("NextPatrolPoint")->DataOffset;
	PropOffsets_PatrolPoint.Nextpatrol = cls->GetProperty("Nextpatrol")->DataOffset;
	PropOffsets_PatrolPoint.PatrolAnim = cls->GetProperty("PatrolAnim")->DataOffset;
	PropOffsets_PatrolPoint.PatrolSound = cls->GetProperty("PatrolSound")->DataOffset;
	PropOffsets_PatrolPoint.lookDir = cls->GetProperty("lookDir")->DataOffset;
	PropOffsets_PatrolPoint.numAnims = cls->GetProperty("numAnims")->DataOffset;
	PropOffsets_PatrolPoint.pausetime = cls->GetProperty("pausetime")->DataOffset;
}

PropertyOffsets_Counter PropOffsets_Counter;

static void InitPropertyOffsets_Counter(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Counter"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Counter");
	PropOffsets_Counter.CompleteMessage = cls->GetProperty("CompleteMessage")->DataOffset;
	PropOffsets_Counter.CountMessage = cls->GetProperty("CountMessage")->DataOffset;
	PropOffsets_Counter.NumToCount = cls->GetProperty("NumToCount")->DataOffset;
	PropOffsets_Counter.OriginalNum = cls->GetProperty("OriginalNum")->DataOffset;
	PropOffsets_Counter.bShowMessage = cls->GetProperty("bShowMessage")->DataOffset;
}

PropertyOffsets_Bitmap PropOffsets_Bitmap;

static void InitPropertyOffsets_Bitmap(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Bitmap"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Bitmap");
	PropOffsets_Bitmap.Format = cls->GetProperty("Format")->DataOffset;
	PropOffsets_Bitmap.InternalTime = cls->GetProperty("InternalTime")->DataOffset;
	PropOffsets_Bitmap.MaxColor = cls->GetProperty("MaxColor")->DataOffset;
	PropOffsets_Bitmap.MipZero = cls->GetProperty("MipZero")->DataOffset;
	PropOffsets_Bitmap.Palette = cls->GetProperty("Palette")->DataOffset;
	PropOffsets_Bitmap.UBits = cls->GetProperty("UBits")->DataOffset;
	PropOffsets_Bitmap.UClamp = cls->GetProperty("UClamp")->DataOffset;
	PropOffsets_Bitmap.USize = cls->GetProperty("USize")->DataOffset;
	PropOffsets_Bitmap.VBits = cls->GetProperty("VBits")->DataOffset;
	PropOffsets_Bitmap.VClamp = cls->GetProperty("VClamp")->DataOffset;
	PropOffsets_Bitmap.VSize = cls->GetProperty("VSize")->DataOffset;
}

PropertyOffsets_MapList PropOffsets_MapList;

static void InitPropertyOffsets_MapList(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "MapList"));
	if (!cls)
		throw std::runtime_error("Could not find class object for MapList");
	PropOffsets_MapList.MapNum = cls->GetProperty("MapNum")->DataOffset;
	PropOffsets_MapList.Maps = cls->GetProperty("Maps")->DataOffset;
}

PropertyOffsets_Effects PropOffsets_Effects;

static void InitPropertyOffsets_Effects(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Effects"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Effects");
	PropOffsets_Effects.EffectSound1 = cls->GetProperty("EffectSound1")->DataOffset;
	PropOffsets_Effects.EffectSound2 = cls->GetProperty("EffectSound2")->DataOffset;
	PropOffsets_Effects.bOnlyTriggerable = cls->GetProperty("bOnlyTriggerable")->DataOffset;
}

PropertyOffsets_StatLogFile PropOffsets_StatLogFile;

static void InitPropertyOffsets_StatLogFile(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "StatLogFile"));
	if (!cls)
		throw std::runtime_error("Could not find class object for StatLogFile");
	PropOffsets_StatLogFile.LogAr = cls->GetProperty("LogAr")->DataOffset;
	PropOffsets_StatLogFile.StatLogFile = cls->GetProperty("StatLogFile")->DataOffset;
	PropOffsets_StatLogFile.StatLogFinal = cls->GetProperty("StatLogFinal")->DataOffset;
	PropOffsets_StatLogFile.bWatermark = cls->GetProperty("bWatermark")->DataOffset;
}

PropertyOffsets_LevelSummary PropOffsets_LevelSummary;

static void InitPropertyOffsets_LevelSummary(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LevelSummary"));
	if (!cls)
		throw std::runtime_error("Could not find class object for LevelSummary");
	PropOffsets_LevelSummary.Author = cls->GetProperty("Author")->DataOffset;
	PropOffsets_LevelSummary.IdealPlayerCount = cls->GetProperty("IdealPlayerCount")->DataOffset;
	PropOffsets_LevelSummary.LevelEnterText = cls->GetProperty("LevelEnterText")->DataOffset;
	PropOffsets_LevelSummary.RecommendedEnemies = cls->GetProperty("RecommendedEnemies")->DataOffset;
	PropOffsets_LevelSummary.RecommendedTeammates = cls->GetProperty("RecommendedTeammates")->DataOffset;
	PropOffsets_LevelSummary.Title = cls->GetProperty("Title")->DataOffset;
}

PropertyOffsets_ScriptedTexture PropOffsets_ScriptedTexture;

static void InitPropertyOffsets_ScriptedTexture(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "ScriptedTexture"));
	if (!cls)
		throw std::runtime_error("Could not find class object for ScriptedTexture");
	PropOffsets_ScriptedTexture.Junk1 = cls->GetProperty("Junk1")->DataOffset;
	PropOffsets_ScriptedTexture.Junk2 = cls->GetProperty("Junk2")->DataOffset;
	PropOffsets_ScriptedTexture.Junk3 = cls->GetProperty("Junk3")->DataOffset;
	PropOffsets_ScriptedTexture.LocalTime = cls->GetProperty("LocalTime")->DataOffset;
	PropOffsets_ScriptedTexture.NotifyActor = cls->GetProperty("NotifyActor")->DataOffset;
	PropOffsets_ScriptedTexture.SourceTexture = cls->GetProperty("SourceTexture")->DataOffset;
}

PropertyOffsets_Engine PropOffsets_Engine;

static void InitPropertyOffsets_Engine(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Engine"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Engine");
	PropOffsets_Engine.Audio = cls->GetProperty("Audio")->DataOffset;
	PropOffsets_Engine.AudioDevice = cls->GetProperty("AudioDevice")->DataOffset;
	PropOffsets_Engine.CacheSizeMegs = cls->GetProperty("CacheSizeMegs")->DataOffset;
	PropOffsets_Engine.Client = cls->GetProperty("Client")->DataOffset;
	PropOffsets_Engine.ClientCycles = cls->GetProperty("ClientCycles")->DataOffset;
	PropOffsets_Engine.Console = cls->GetProperty("Console")->DataOffset;
	PropOffsets_Engine.CurrentTickRate = cls->GetProperty("CurrentTickRate")->DataOffset;
	PropOffsets_Engine.Cylinder = cls->GetProperty("Cylinder")->DataOffset;
	PropOffsets_Engine.GameCycles = cls->GetProperty("GameCycles")->DataOffset;
	PropOffsets_Engine.GameRenderDevice = cls->GetProperty("GameRenderDevice")->DataOffset;
	PropOffsets_Engine.Language = cls->GetProperty("Language")->DataOffset;
	PropOffsets_Engine.NetworkDevice = cls->GetProperty("NetworkDevice")->DataOffset;
	PropOffsets_Engine.Render = cls->GetProperty("Render")->DataOffset;
	PropOffsets_Engine.TickCycles = cls->GetProperty("TickCycles")->DataOffset;
	PropOffsets_Engine.UseSound = cls->GetProperty("UseSound")->DataOffset;
}

PropertyOffsets_TriggerLight PropOffsets_TriggerLight;

static void InitPropertyOffsets_TriggerLight(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "TriggerLight"));
	if (!cls)
		throw std::runtime_error("Could not find class object for TriggerLight");
	PropOffsets_TriggerLight.Alpha = cls->GetProperty("Alpha")->DataOffset;
	PropOffsets_TriggerLight.ChangeTime = cls->GetProperty("ChangeTime")->DataOffset;
	PropOffsets_TriggerLight.Direction = cls->GetProperty("Direction")->DataOffset;
	PropOffsets_TriggerLight.InitialBrightness = cls->GetProperty("InitialBrightness")->DataOffset;
	PropOffsets_TriggerLight.RemainOnTime = cls->GetProperty("RemainOnTime")->DataOffset;
	PropOffsets_TriggerLight.SavedTrigger = cls->GetProperty("SavedTrigger")->DataOffset;
	PropOffsets_TriggerLight.bDelayFullOn = cls->GetProperty("bDelayFullOn")->DataOffset;
	PropOffsets_TriggerLight.bInitiallyOn = cls->GetProperty("bInitiallyOn")->DataOffset;
	PropOffsets_TriggerLight.poundTime = cls->GetProperty("poundTime")->DataOffset;
}

PropertyOffsets_SpecialEvent PropOffsets_SpecialEvent;

static void InitPropertyOffsets_SpecialEvent(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "SpecialEvent"));
	if (!cls)
		throw std::runtime_error("Could not find class object for SpecialEvent");
	PropOffsets_SpecialEvent.Damage = cls->GetProperty("Damage")->DataOffset;
	PropOffsets_SpecialEvent.DamageString = cls->GetProperty("DamageString")->DataOffset;
	PropOffsets_SpecialEvent.DamageType = cls->GetProperty("DamageType")->DataOffset;
	PropOffsets_SpecialEvent.Message = cls->GetProperty("Message")->DataOffset;
	PropOffsets_SpecialEvent.Sound = cls->GetProperty("Sound")->DataOffset;
	PropOffsets_SpecialEvent.bBroadcast = cls->GetProperty("bBroadcast")->DataOffset;
	PropOffsets_SpecialEvent.bPlayerViewRot = cls->GetProperty("bPlayerViewRot")->DataOffset;
}

PropertyOffsets_RoundRobin PropOffsets_RoundRobin;

static void InitPropertyOffsets_RoundRobin(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "RoundRobin"));
	if (!cls)
		throw std::runtime_error("Could not find class object for RoundRobin");
	PropOffsets_RoundRobin.OutEvents = cls->GetProperty("OutEvents")->DataOffset;
	PropOffsets_RoundRobin.bLoop = cls->GetProperty("bLoop")->DataOffset;
	PropOffsets_RoundRobin.i = cls->GetProperty("i")->DataOffset;
}

PropertyOffsets_MusicEvent PropOffsets_MusicEvent;

static void InitPropertyOffsets_MusicEvent(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "MusicEvent"));
	if (!cls)
		throw std::runtime_error("Could not find class object for MusicEvent");
	PropOffsets_MusicEvent.CdTrack = cls->GetProperty("CdTrack")->DataOffset;
	PropOffsets_MusicEvent.Song = cls->GetProperty("Song")->DataOffset;
	PropOffsets_MusicEvent.SongSection = cls->GetProperty("SongSection")->DataOffset;
	PropOffsets_MusicEvent.Transition = cls->GetProperty("Transition")->DataOffset;
	PropOffsets_MusicEvent.bAffectAllPlayers = cls->GetProperty("bAffectAllPlayers")->DataOffset;
	PropOffsets_MusicEvent.bOnceOnly = cls->GetProperty("bOnceOnly")->DataOffset;
	PropOffsets_MusicEvent.bSilence = cls->GetProperty("bSilence")->DataOffset;
}

PropertyOffsets_HomeBase PropOffsets_HomeBase;

static void InitPropertyOffsets_HomeBase(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "HomeBase"));
	if (!cls)
		throw std::runtime_error("Could not find class object for HomeBase");
	PropOffsets_HomeBase.Extent = cls->GetProperty("Extent")->DataOffset;
	PropOffsets_HomeBase.lookDir = cls->GetProperty("lookDir")->DataOffset;
}

PropertyOffsets_Dispatcher PropOffsets_Dispatcher;

static void InitPropertyOffsets_Dispatcher(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Dispatcher"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Dispatcher");
	PropOffsets_Dispatcher.OutDelays = cls->GetProperty("OutDelays")->DataOffset;
	PropOffsets_Dispatcher.OutEvents = cls->GetProperty("OutEvents")->DataOffset;
	PropOffsets_Dispatcher.i = cls->GetProperty("i")->DataOffset;
}

PropertyOffsets_DemoRecSpectator PropOffsets_DemoRecSpectator;

static void InitPropertyOffsets_DemoRecSpectator(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "DemoRecSpectator"));
	if (!cls)
		throw std::runtime_error("Could not find class object for DemoRecSpectator");
	PropOffsets_DemoRecSpectator.PlaybackActor = cls->GetProperty("PlaybackActor")->DataOffset;
	PropOffsets_DemoRecSpectator.PlaybackGRI = cls->GetProperty("PlaybackGRI")->DataOffset;
}

PropertyOffsets_DamageType PropOffsets_DamageType;

static void InitPropertyOffsets_DamageType(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "DamageType"));
	if (!cls)
		throw std::runtime_error("Could not find class object for DamageType");
	PropOffsets_DamageType.AltName = cls->GetProperty("AltName")->DataOffset;
	PropOffsets_DamageType.DamageEffect = cls->GetProperty("DamageEffect")->DataOffset;
	PropOffsets_DamageType.Name = cls->GetProperty("Name")->DataOffset;
	PropOffsets_DamageType.ViewFlash = cls->GetProperty("ViewFlash")->DataOffset;
	PropOffsets_DamageType.ViewFog = cls->GetProperty("ViewFog")->DataOffset;
}

PropertyOffsets_Ambushpoint PropOffsets_Ambushpoint;

static void InitPropertyOffsets_Ambushpoint(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "Ambushpoint"));
	if (!cls)
		throw std::runtime_error("Could not find class object for Ambushpoint");
	PropOffsets_Ambushpoint.SightRadius = cls->GetProperty("SightRadius")->DataOffset;
	PropOffsets_Ambushpoint.bSniping = cls->GetProperty("bSniping")->DataOffset;
	PropOffsets_Ambushpoint.lookDir = cls->GetProperty("lookDir")->DataOffset;
	PropOffsets_Ambushpoint.survivecount = cls->GetProperty("survivecount")->DataOffset;
}

PropertyOffsets_WarpZoneMarker PropOffsets_WarpZoneMarker;

static void InitPropertyOffsets_WarpZoneMarker(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "WarpZoneMarker"));
	if (!cls)
		throw std::runtime_error("Could not find class object for WarpZoneMarker");
	PropOffsets_WarpZoneMarker.TriggerActor = cls->GetProperty("TriggerActor")->DataOffset;
	PropOffsets_WarpZoneMarker.TriggerActor2 = cls->GetProperty("TriggerActor2")->DataOffset;
	PropOffsets_WarpZoneMarker.markedWarpZone = cls->GetProperty("markedWarpZone")->DataOffset;
}

PropertyOffsets_LiftCenter PropOffsets_LiftCenter;

static void InitPropertyOffsets_LiftCenter(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "LiftCenter"));
	if (!cls)
		throw std::runtime_error("Could not find class object for LiftCenter");
	PropOffsets_LiftCenter.LastTriggerTime = cls->GetProperty("LastTriggerTime")->DataOffset;
	PropOffsets_LiftCenter.LiftOffset = cls->GetProperty("LiftOffset")->DataOffset;
	PropOffsets_LiftCenter.LiftTag = cls->GetProperty("LiftTag")->DataOffset;
	PropOffsets_LiftCenter.LiftTrigger = cls->GetProperty("LiftTrigger")->DataOffset;
	PropOffsets_LiftCenter.MaxDist2D = cls->GetProperty("MaxDist2D")->DataOffset;
	PropOffsets_LiftCenter.MaxZDiffAdd = cls->GetProperty("MaxZDiffAdd")->DataOffset;
	PropOffsets_LiftCenter.MyLift = cls->GetProperty("MyLift")->DataOffset;
	PropOffsets_LiftCenter.RecommendedTrigger = cls->GetProperty("RecommendedTrigger")->DataOffset;
}

PropertyOffsets_RenderIterator PropOffsets_RenderIterator;

static void InitPropertyOffsets_RenderIterator(PackageManager* packages)
{
	UClass* cls = dynamic_cast<UClass*>(packages->GetPackage("engine")->GetUObject("Class", "RenderIterator"));
	if (!cls)
		throw std::runtime_error("Could not find class object for RenderIterator");
	PropOffsets_RenderIterator.Frame = cls->GetProperty("Frame")->DataOffset;
	PropOffsets_RenderIterator.Index = cls->GetProperty("Index")->DataOffset;
	PropOffsets_RenderIterator.MaxItems = cls->GetProperty("MaxItems")->DataOffset;
	PropOffsets_RenderIterator.Observer = cls->GetProperty("Observer")->DataOffset;
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
}
