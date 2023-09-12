#pragma once

class PackageManager;

void InitPropertyOffsets(PackageManager* packages);

struct PropertyDataOffset
{
	size_t DataOffset = ~(size_t)0;
	uint32_t BitfieldMask = 1;
};

struct PropertyOffsets_Object
{
	PropertyDataOffset Class;
	PropertyDataOffset Name;
	PropertyDataOffset ObjectFlags;
	PropertyDataOffset ObjectInternal;
	PropertyDataOffset Outer;
};

extern PropertyOffsets_Object PropOffsets_Object;

struct PropertyOffsets_Commandlet
{
	PropertyDataOffset HelpCmd;
	PropertyDataOffset HelpDesc;
	PropertyDataOffset HelpOneLiner;
	PropertyDataOffset HelpParm;
	PropertyDataOffset HelpUsage;
	PropertyDataOffset HelpWebLink;
	PropertyDataOffset IsClient;
	PropertyDataOffset IsEditor;
	PropertyDataOffset IsServer;
	PropertyDataOffset LazyLoad;
	PropertyDataOffset LogToStdout;
	PropertyDataOffset ShowBanner;
	PropertyDataOffset ShowErrorCount;
};

extern PropertyOffsets_Commandlet PropOffsets_Commandlet;

struct PropertyOffsets_Subsystem
{
	PropertyDataOffset ExecVtbl;
};

extern PropertyOffsets_Subsystem PropOffsets_Subsystem;

struct PropertyOffsets_HelloWorldCommandlet
{
	PropertyDataOffset intparm;
	PropertyDataOffset strparm;
};

extern PropertyOffsets_HelloWorldCommandlet PropOffsets_HelloWorldCommandlet;

struct PropertyOffsets_SimpleCommandlet
{
	PropertyDataOffset intparm;
};

extern PropertyOffsets_SimpleCommandlet PropOffsets_SimpleCommandlet;

struct PropertyOffsets_Pawn
{
	PropertyDataOffset AccelRate;
	PropertyDataOffset AirControl;
	PropertyDataOffset AirSpeed;
	PropertyDataOffset AlarmTag;
	PropertyDataOffset Alertness;
	PropertyDataOffset AttitudeToPlayer;
	PropertyDataOffset AvgPhysicsTime;
	PropertyDataOffset BaseEyeHeight;
	PropertyDataOffset CombatStyle;
	PropertyDataOffset DamageScaling;
	PropertyDataOffset DesiredSpeed;
	PropertyDataOffset Destination;
	PropertyDataOffset Die;
	PropertyDataOffset DieCount;
	PropertyDataOffset DropWhenKilled;
	PropertyDataOffset Enemy;
	PropertyDataOffset EyeHeight;
	PropertyDataOffset FaceTarget;
	PropertyDataOffset Floor;
	PropertyDataOffset Focus;
	PropertyDataOffset FootRegion;
	PropertyDataOffset FovAngle;
	PropertyDataOffset GroundSpeed;
	PropertyDataOffset HeadRegion;
	PropertyDataOffset Health;
	PropertyDataOffset HearingThreshold;
	PropertyDataOffset HitSound1;
	PropertyDataOffset HitSound2;
	PropertyDataOffset Intelligence;
	PropertyDataOffset ItemCount;
	PropertyDataOffset JumpZ;
	PropertyDataOffset KillCount;
	PropertyDataOffset Land;
	PropertyDataOffset LastPainSound;
	PropertyDataOffset LastSeeingPos;
	PropertyDataOffset LastSeenPos;
	PropertyDataOffset LastSeenTime;
	PropertyDataOffset MaxDesiredSpeed;
	PropertyDataOffset MaxStepHeight;
	PropertyDataOffset MeleeRange;
	PropertyDataOffset MenuName;
	PropertyDataOffset MinHitWall;
	PropertyDataOffset MoveTarget;
	PropertyDataOffset MoveTimer;
	PropertyDataOffset NameArticle;
	PropertyDataOffset NextLabel;
	PropertyDataOffset NextState;
	PropertyDataOffset OldMessageTime;
	PropertyDataOffset OrthoZoom;
	PropertyDataOffset PainTime;
	PropertyDataOffset PendingWeapon;
	PropertyDataOffset PeripheralVision;
	PropertyDataOffset PlayerReStartState;
	PropertyDataOffset PlayerReplicationInfo;
	PropertyDataOffset PlayerReplicationInfoClass;
	PropertyDataOffset ReducedDamagePct;
	PropertyDataOffset ReducedDamageType;
	PropertyDataOffset RouteCache;
	PropertyDataOffset SecretCount;
	PropertyDataOffset SelectedItem;
	PropertyDataOffset SelectionMesh;
	PropertyDataOffset Shadow;
	PropertyDataOffset SharedAlarmTag;
	PropertyDataOffset SightCounter;
	PropertyDataOffset SightRadius;
	PropertyDataOffset Skill;
	PropertyDataOffset SoundDampening;
	PropertyDataOffset SpecialGoal;
	PropertyDataOffset SpecialMesh;
	PropertyDataOffset SpecialPause;
	PropertyDataOffset SpeechTime;
	PropertyDataOffset SplashTime;
	PropertyDataOffset Spree;
	PropertyDataOffset Stimulus;
	PropertyDataOffset UnderWaterTime;
	PropertyDataOffset ViewRotation;
	PropertyDataOffset Visibility;
	PropertyDataOffset VoicePitch;
	PropertyDataOffset VoiceType;
	PropertyDataOffset WalkBob;
	PropertyDataOffset WaterSpeed;
	PropertyDataOffset WaterStep;
	PropertyDataOffset Weapon;
	PropertyDataOffset bAdvancedTactics;
	PropertyDataOffset bAltFire;
	PropertyDataOffset bAutoActivate;
	PropertyDataOffset bAvoidLedges;
	PropertyDataOffset bBehindView;
	PropertyDataOffset bCanDoSpecial;
	PropertyDataOffset bCanFly;
	PropertyDataOffset bCanJump;
	PropertyDataOffset bCanOpenDoors;
	PropertyDataOffset bCanStrafe;
	PropertyDataOffset bCanSwim;
	PropertyDataOffset bCanWalk;
	PropertyDataOffset bCountJumps;
	PropertyDataOffset bDrowning;
	PropertyDataOffset bDuck;
	PropertyDataOffset bExtra0;
	PropertyDataOffset bExtra1;
	PropertyDataOffset bExtra2;
	PropertyDataOffset bExtra3;
	PropertyDataOffset bFire;
	PropertyDataOffset bFixedStart;
	PropertyDataOffset bFreeLook;
	PropertyDataOffset bFromWall;
	PropertyDataOffset bHitSlopedWall;
	PropertyDataOffset bHunting;
	PropertyDataOffset bIsFemale;
	PropertyDataOffset bIsHuman;
	PropertyDataOffset bIsMultiSkinned;
	PropertyDataOffset bIsPlayer;
	PropertyDataOffset bIsWalking;
	PropertyDataOffset bJumpOffPawn;
	PropertyDataOffset bJustLanded;
	PropertyDataOffset bLOSflag;
	PropertyDataOffset bLook;
	PropertyDataOffset bNeverSwitchOnPickup;
	PropertyDataOffset bReducedSpeed;
	PropertyDataOffset bRun;
	PropertyDataOffset bShootSpecial;
	PropertyDataOffset bSnapLevel;
	PropertyDataOffset bStopAtLedges;
	PropertyDataOffset bStrafe;
	PropertyDataOffset bUpAndOut;
	PropertyDataOffset bUpdatingDisplay;
	PropertyDataOffset bViewTarget;
	PropertyDataOffset bWarping;
	PropertyDataOffset bZoom;
	PropertyDataOffset carriedDecoration;
	PropertyDataOffset home;
	PropertyDataOffset nextPawn;
	PropertyDataOffset noise1loudness;
	PropertyDataOffset noise1other;
	PropertyDataOffset noise1spot;
	PropertyDataOffset noise1time;
	PropertyDataOffset noise2loudness;
	PropertyDataOffset noise2other;
	PropertyDataOffset noise2spot;
	PropertyDataOffset noise2time;
};

extern PropertyOffsets_Pawn PropOffsets_Pawn;

struct PropertyOffsets_Actor
{
	PropertyDataOffset Acceleration;
	PropertyDataOffset AmbientGlow;
	PropertyDataOffset AmbientSound;
	PropertyDataOffset AnimFrame;
	PropertyDataOffset AnimLast;
	PropertyDataOffset AnimMinRate;
	PropertyDataOffset AnimRate;
	PropertyDataOffset AnimSequence;
	PropertyDataOffset AttachTag;
	PropertyDataOffset Base;
	PropertyDataOffset Brush;
	PropertyDataOffset Buoyancy;
	PropertyDataOffset ColLocation;
	PropertyDataOffset CollisionHeight;
	PropertyDataOffset CollisionRadius;
	PropertyDataOffset CollisionTag;
	PropertyDataOffset Deleted;
	PropertyDataOffset DesiredRotation;
	PropertyDataOffset DodgeDir;
	PropertyDataOffset DrawScale;
	PropertyDataOffset DrawType;
	PropertyDataOffset Event;
	PropertyDataOffset ExtraTag;
	PropertyDataOffset Fatness;
	PropertyDataOffset Group;
	PropertyDataOffset HitActor;
	PropertyDataOffset InitialState;
	PropertyDataOffset Instigator;
	PropertyDataOffset Inventory;
	PropertyDataOffset LODBias;
	PropertyDataOffset LatentActor;
	PropertyDataOffset LatentByte;
	PropertyDataOffset LatentFloat;
	PropertyDataOffset LatentInt;
	PropertyDataOffset Level;
	PropertyDataOffset LifeSpan;
	PropertyDataOffset LightBrightness;
	PropertyDataOffset LightCone;
	PropertyDataOffset LightEffect;
	PropertyDataOffset LightHue;
	PropertyDataOffset LightPeriod;
	PropertyDataOffset LightPhase;
	PropertyDataOffset LightRadius;
	PropertyDataOffset LightSaturation;
	PropertyDataOffset LightType;
	PropertyDataOffset LightingTag;
	PropertyDataOffset Location;
	PropertyDataOffset Mass;
	PropertyDataOffset Mesh;
	PropertyDataOffset MiscNumber;
	PropertyDataOffset MultiSkins;
	PropertyDataOffset NetPriority;
	PropertyDataOffset NetTag;
	PropertyDataOffset NetUpdateFrequency;
	PropertyDataOffset OddsOfAppearing;
	PropertyDataOffset OldAnimRate;
	PropertyDataOffset OldLocation;
	PropertyDataOffset OtherTag;
	PropertyDataOffset Owner;
	PropertyDataOffset PendingTouch;
	PropertyDataOffset PhysAlpha;
	PropertyDataOffset PhysRate;
	PropertyDataOffset Physics;
	PropertyDataOffset PrePivot;
	PropertyDataOffset Region;
	PropertyDataOffset RemoteRole;
	PropertyDataOffset RenderInterface;
	PropertyDataOffset RenderIteratorClass;
	PropertyDataOffset Role;
	PropertyDataOffset Rotation;
	PropertyDataOffset RotationRate;
	PropertyDataOffset ScaleGlow;
	PropertyDataOffset SimAnim;
	PropertyDataOffset SkelAnim;
	PropertyDataOffset Skin;
	PropertyDataOffset SoundPitch;
	PropertyDataOffset SoundRadius;
	PropertyDataOffset SoundVolume;
	PropertyDataOffset SpecialTag;
	PropertyDataOffset Sprite;
	PropertyDataOffset SpriteProjForward;
	PropertyDataOffset StandingCount;
	PropertyDataOffset Style;
	PropertyDataOffset Tag;
	PropertyDataOffset Target;
	PropertyDataOffset Texture;
	PropertyDataOffset TimerCounter;
	PropertyDataOffset TimerRate;
	PropertyDataOffset Touching;
	PropertyDataOffset TransientSoundRadius;
	PropertyDataOffset TransientSoundVolume;
	PropertyDataOffset TweenRate;
	PropertyDataOffset Velocity;
	PropertyDataOffset VisibilityHeight;
	PropertyDataOffset VisibilityRadius;
	PropertyDataOffset VolumeBrightness;
	PropertyDataOffset VolumeFog;
	PropertyDataOffset VolumeRadius;
	PropertyDataOffset XLevel;
	PropertyDataOffset bActorShadows;
	PropertyDataOffset bAlwaysRelevant;
	PropertyDataOffset bAlwaysTick;
	PropertyDataOffset bAnimByOwner;
	PropertyDataOffset bAnimFinished;
	PropertyDataOffset bAnimLoop;
	PropertyDataOffset bAnimNotify;
	PropertyDataOffset bAssimilated;
	PropertyDataOffset bBlockActors;
	PropertyDataOffset bBlockPlayers;
	PropertyDataOffset bBounce;
	PropertyDataOffset bCanTeleport;
	PropertyDataOffset bCarriedItem;
	PropertyDataOffset bClientAnim;
	PropertyDataOffset bClientDemoNetFunc;
	PropertyDataOffset bClientDemoRecording;
	PropertyDataOffset bCollideActors;
	PropertyDataOffset bCollideWhenPlacing;
	PropertyDataOffset bCollideWorld;
	PropertyDataOffset bCorona;
	PropertyDataOffset bDeleteMe;
	PropertyDataOffset bDemoRecording;
	PropertyDataOffset bDifficulty0;
	PropertyDataOffset bDifficulty1;
	PropertyDataOffset bDifficulty2;
	PropertyDataOffset bDifficulty3;
	PropertyDataOffset bDirectional;
	PropertyDataOffset bDynamicLight;
	PropertyDataOffset bEdLocked;
	PropertyDataOffset bEdShouldSnap;
	PropertyDataOffset bEdSnap;
	PropertyDataOffset bFilterByVolume;
	PropertyDataOffset bFixedRotationDir;
	PropertyDataOffset bForcePhysicsUpdate;
	PropertyDataOffset bForceStasis;
	PropertyDataOffset bGameRelevant;
	PropertyDataOffset bHidden;
	PropertyDataOffset bHiddenEd;
	PropertyDataOffset bHighDetail;
	PropertyDataOffset bHighlighted;
	PropertyDataOffset bHurtEntry;
	PropertyDataOffset bInterpolating;
	PropertyDataOffset bIsItemGoal;
	PropertyDataOffset bIsKillGoal;
	PropertyDataOffset bIsMover;
	PropertyDataOffset bIsPawn;
	PropertyDataOffset bIsSecretGoal;
	PropertyDataOffset bJustTeleported;
	PropertyDataOffset bLensFlare;
	PropertyDataOffset bLightChanged;
	PropertyDataOffset bMemorized;
	PropertyDataOffset bMeshCurvy;
	PropertyDataOffset bMeshEnviroMap;
	PropertyDataOffset bMovable;
	PropertyDataOffset bNet;
	PropertyDataOffset bNetFeel;
	PropertyDataOffset bNetHear;
	PropertyDataOffset bNetInitial;
	PropertyDataOffset bNetOptional;
	PropertyDataOffset bNetOwner;
	PropertyDataOffset bNetRelevant;
	PropertyDataOffset bNetSee;
	PropertyDataOffset bNetSpecial;
	PropertyDataOffset bNetTemporary;
	PropertyDataOffset bNoDelete;
	PropertyDataOffset bNoSmooth;
	PropertyDataOffset bOnlyOwnerSee;
	PropertyDataOffset bOwnerNoSee;
	PropertyDataOffset bParticles;
	PropertyDataOffset bProjTarget;
	PropertyDataOffset bRandomFrame;
	PropertyDataOffset bReplicateInstigator;
	PropertyDataOffset bRotateToDesired;
	PropertyDataOffset bScriptInitialized;
	PropertyDataOffset bSelected;
	PropertyDataOffset bShadowCast;
	PropertyDataOffset bSimFall;
	PropertyDataOffset bSimulatedPawn;
	PropertyDataOffset bSinglePlayer;
	PropertyDataOffset bSpecialLit;
	PropertyDataOffset bStasis;
	PropertyDataOffset bStatic;
	PropertyDataOffset bTempEditor;
	PropertyDataOffset bTicked;
	PropertyDataOffset bTimerLoop;
	PropertyDataOffset bTrailerPrePivot;
	PropertyDataOffset bTrailerSameRotation;
	PropertyDataOffset bTravel;
	PropertyDataOffset bUnlit;
};

extern PropertyOffsets_Actor PropOffsets_Actor;

struct PropertyOffsets_LevelInfo
{
	PropertyDataOffset AIProfile;
	PropertyDataOffset Author;
	PropertyDataOffset AvgAITime;
	PropertyDataOffset Brightness;
	PropertyDataOffset CdTrack;
	PropertyDataOffset ComputerName;
	PropertyDataOffset Day;
	PropertyDataOffset DayOfWeek;
	PropertyDataOffset DefaultGameType;
	PropertyDataOffset DefaultTexture;
	PropertyDataOffset EngineVersion;
	PropertyDataOffset Game;
	PropertyDataOffset Hour;
	PropertyDataOffset HubStackLevel;
	PropertyDataOffset IdealPlayerCount;
	PropertyDataOffset LevelAction;
	PropertyDataOffset LevelEnterText;
	PropertyDataOffset LocalizedPkg;
	PropertyDataOffset Millisecond;
	PropertyDataOffset MinNetVersion;
	PropertyDataOffset Minute;
	PropertyDataOffset Month;
	PropertyDataOffset NavigationPointList;
	PropertyDataOffset NetMode;
	PropertyDataOffset NextSwitchCountdown;
	PropertyDataOffset NextURL;
	PropertyDataOffset Pauser;
	PropertyDataOffset PawnList;
	PropertyDataOffset PlayerDoppler;
	PropertyDataOffset RecommendedEnemies;
	PropertyDataOffset RecommendedTeammates;
	PropertyDataOffset Screenshot;
	PropertyDataOffset Second;
	PropertyDataOffset Song;
	PropertyDataOffset SongSection;
	PropertyDataOffset SpawnNotify;
	PropertyDataOffset Summary;
	PropertyDataOffset TimeDilation;
	PropertyDataOffset TimeSeconds;
	PropertyDataOffset Title;
	PropertyDataOffset VisibleGroups;
	PropertyDataOffset Year;
	PropertyDataOffset bAggressiveLOD;
	PropertyDataOffset bAllowFOV;
	PropertyDataOffset bBegunPlay;
	PropertyDataOffset bCheckWalkSurfaces;
	PropertyDataOffset bDropDetail;
	PropertyDataOffset bHighDetailMode;
	PropertyDataOffset bHumansOnly;
	PropertyDataOffset bLonePlayer;
	PropertyDataOffset bLowRes;
	PropertyDataOffset bNeverPrecache;
	PropertyDataOffset bNextItems;
	PropertyDataOffset bNoCheating;
	PropertyDataOffset bPlayersOnly;
	PropertyDataOffset bStartup;
};

extern PropertyOffsets_LevelInfo PropOffsets_LevelInfo;

struct PropertyOffsets_Inventory
{
	PropertyDataOffset AbsorptionPriority;
	PropertyDataOffset ActivateSound;
	PropertyDataOffset ArmorAbsorption;
	PropertyDataOffset AutoSwitchPriority;
	PropertyDataOffset BobDamping;
	PropertyDataOffset Charge;
	PropertyDataOffset DeActivateSound;
	PropertyDataOffset FlashCount;
	PropertyDataOffset Icon;
	PropertyDataOffset InventoryGroup;
	PropertyDataOffset ItemArticle;
	PropertyDataOffset ItemMessageClass;
	PropertyDataOffset ItemName;
	PropertyDataOffset M_Activated;
	PropertyDataOffset M_Deactivated;
	PropertyDataOffset M_Selected;
	PropertyDataOffset MaxDesireability;
	PropertyDataOffset MuzzleFlashMesh;
	PropertyDataOffset MuzzleFlashScale;
	PropertyDataOffset MuzzleFlashStyle;
	PropertyDataOffset MuzzleFlashTexture;
	PropertyDataOffset NextArmor;
	PropertyDataOffset OldFlashCount;
	PropertyDataOffset PickupMessage;
	PropertyDataOffset PickupMessageClass;
	PropertyDataOffset PickupSound;
	PropertyDataOffset PickupViewMesh;
	PropertyDataOffset PickupViewScale;
	PropertyDataOffset PlayerLastTouched;
	PropertyDataOffset PlayerViewMesh;
	PropertyDataOffset PlayerViewOffset;
	PropertyDataOffset PlayerViewScale;
	PropertyDataOffset ProtectionType1;
	PropertyDataOffset ProtectionType2;
	PropertyDataOffset RespawnSound;
	PropertyDataOffset RespawnTime;
	PropertyDataOffset StatusIcon;
	PropertyDataOffset ThirdPersonMesh;
	PropertyDataOffset ThirdPersonScale;
	PropertyDataOffset bActivatable;
	PropertyDataOffset bActive;
	PropertyDataOffset bAmbientGlow;
	PropertyDataOffset bDisplayableInv;
	PropertyDataOffset bFirstFrame;
	PropertyDataOffset bHeldItem;
	PropertyDataOffset bInstantRespawn;
	PropertyDataOffset bIsAnArmor;
	PropertyDataOffset bMuzzleFlashParticles;
	PropertyDataOffset bRotatingPickup;
	PropertyDataOffset bSleepTouch;
	PropertyDataOffset bSteadyFlash3rd;
	PropertyDataOffset bSteadyToggle;
	PropertyDataOffset bToggleSteadyFlash;
	PropertyDataOffset bTossedOut;
	PropertyDataOffset myMarker;
};

extern PropertyOffsets_Inventory PropOffsets_Inventory;

struct PropertyOffsets_PlayerPawn
{
	PropertyDataOffset AppliedBob;
	PropertyDataOffset Bob;
	PropertyDataOffset BorrowedMouseX;
	PropertyDataOffset BorrowedMouseY;
	PropertyDataOffset CarcassType;
	PropertyDataOffset CdTrack;
	PropertyDataOffset ClientUpdateTime;
	PropertyDataOffset ConstantGlowFog;
	PropertyDataOffset ConstantGlowScale;
	PropertyDataOffset CurrentTimeStamp;
	PropertyDataOffset DefaultFOV;
	PropertyDataOffset DelayedCommand;
	PropertyDataOffset DemoViewPitch;
	PropertyDataOffset DemoViewYaw;
	PropertyDataOffset DesiredFOV;
	PropertyDataOffset DesiredFlashFog;
	PropertyDataOffset DesiredFlashScale;
	PropertyDataOffset DodgeClickTime;
	PropertyDataOffset DodgeClickTimer;
	PropertyDataOffset FailedView;
	PropertyDataOffset FlashFog;
	PropertyDataOffset FlashScale;
	PropertyDataOffset FreeMoves;
	PropertyDataOffset GameReplicationInfo;
	PropertyDataOffset HUDType;
	PropertyDataOffset Handedness;
	PropertyDataOffset InstantFlash;
	PropertyDataOffset InstantFog;
	PropertyDataOffset JumpSound;
	PropertyDataOffset LandBob;
	PropertyDataOffset LastMessageWindow;
	PropertyDataOffset LastPlaySound;
	PropertyDataOffset LastUpdateTime;
	PropertyDataOffset MaxTimeMargin;
	PropertyDataOffset Misc1;
	PropertyDataOffset Misc2;
	PropertyDataOffset MouseSensitivity;
	PropertyDataOffset MouseSmoothThreshold;
	PropertyDataOffset MouseZeroTime;
	PropertyDataOffset MyAutoAim;
	PropertyDataOffset NoPauseMessage;
	PropertyDataOffset OwnCamera;
	PropertyDataOffset Password;
	PropertyDataOffset PendingMove;
	PropertyDataOffset Player;
	PropertyDataOffset ProgressColor;
	PropertyDataOffset ProgressMessage;
	PropertyDataOffset ProgressTimeOut;
	PropertyDataOffset QuickSaveString;
	PropertyDataOffset ReceivedSecretChecksum;
	PropertyDataOffset RendMap;
	PropertyDataOffset SavedMoves;
	PropertyDataOffset Scoring;
	PropertyDataOffset ScoringType;
	PropertyDataOffset ServerTimeStamp;
	PropertyDataOffset ShowFlags;
	PropertyDataOffset SmoothMouseX;
	PropertyDataOffset SmoothMouseY;
	PropertyDataOffset Song;
	PropertyDataOffset SongSection;
	PropertyDataOffset SpecialMenu;
	PropertyDataOffset TargetEyeHeight;
	PropertyDataOffset TargetViewRotation;
	PropertyDataOffset TargetWeaponViewOffset;
	PropertyDataOffset TimeMargin;
	PropertyDataOffset Transition;
	PropertyDataOffset ViewTarget;
	PropertyDataOffset ViewingFrom;
	PropertyDataOffset WeaponPriority;
	PropertyDataOffset ZoomLevel;
	PropertyDataOffset aBaseX;
	PropertyDataOffset aBaseY;
	PropertyDataOffset aBaseZ;
	PropertyDataOffset aExtra0;
	PropertyDataOffset aExtra1;
	PropertyDataOffset aExtra2;
	PropertyDataOffset aExtra3;
	PropertyDataOffset aExtra4;
	PropertyDataOffset aForward;
	PropertyDataOffset aLookUp;
	PropertyDataOffset aMouseX;
	PropertyDataOffset aMouseY;
	PropertyDataOffset aStrafe;
	PropertyDataOffset aTurn;
	PropertyDataOffset aUp;
	PropertyDataOffset bAdmin;
	PropertyDataOffset bAlwaysMouseLook;
	PropertyDataOffset bAnimTransition;
	PropertyDataOffset bBadConnectionAlert;
	PropertyDataOffset bCenterView;
	PropertyDataOffset bCheatsEnabled;
	PropertyDataOffset bDelayedCommand;
	PropertyDataOffset bEdgeBack;
	PropertyDataOffset bEdgeForward;
	PropertyDataOffset bEdgeLeft;
	PropertyDataOffset bEdgeRight;
	PropertyDataOffset bFixedCamera;
	PropertyDataOffset bFrozen;
	PropertyDataOffset bInvertMouse;
	PropertyDataOffset bIsCrouching;
	PropertyDataOffset bIsTurning;
	PropertyDataOffset bIsTyping;
	PropertyDataOffset bJumpStatus;
	PropertyDataOffset bJustAltFired;
	PropertyDataOffset bJustFired;
	PropertyDataOffset bKeyboardLook;
	PropertyDataOffset bLookUpStairs;
	PropertyDataOffset bMaxMouseSmoothing;
	PropertyDataOffset bMessageBeep;
	PropertyDataOffset bMouseZeroed;
	PropertyDataOffset bNeverAutoSwitch;
	PropertyDataOffset bNoFlash;
	PropertyDataOffset bNoVoices;
	PropertyDataOffset bPressedJump;
	PropertyDataOffset bReadyToPlay;
	PropertyDataOffset bReducedVis;
	PropertyDataOffset bRising;
	PropertyDataOffset bShakeDir;
	PropertyDataOffset bShowMenu;
	PropertyDataOffset bShowScores;
	PropertyDataOffset bSinglePlayer;
	PropertyDataOffset bSnapToLevel;
	PropertyDataOffset bSpecialMenu;
	PropertyDataOffset bUpdatePosition;
	PropertyDataOffset bUpdating;
	PropertyDataOffset bWasBack;
	PropertyDataOffset bWasForward;
	PropertyDataOffset bWasLeft;
	PropertyDataOffset bWasRight;
	PropertyDataOffset bWokeUp;
	PropertyDataOffset bZooming;
	PropertyDataOffset bobtime;
	PropertyDataOffset maxshake;
	PropertyDataOffset myHUD;
	PropertyDataOffset ngSecretSet;
	PropertyDataOffset ngWorldSecret;
	PropertyDataOffset shakemag;
	PropertyDataOffset shaketimer;
	PropertyDataOffset shakevert;
	PropertyDataOffset verttimer;
};

extern PropertyOffsets_PlayerPawn PropOffsets_PlayerPawn;

struct PropertyOffsets_PlayerReplicationInfo
{
	PropertyDataOffset Deaths;
	PropertyDataOffset HasFlag;
	PropertyDataOffset OldName;
	PropertyDataOffset PacketLoss;
	PropertyDataOffset Ping;
	PropertyDataOffset PlayerID;
	PropertyDataOffset PlayerLocation;
	PropertyDataOffset PlayerName;
	PropertyDataOffset PlayerZone;
	PropertyDataOffset Score;
	PropertyDataOffset StartTime;
	PropertyDataOffset TalkTexture;
	PropertyDataOffset Team;
	PropertyDataOffset TeamID;
	PropertyDataOffset TeamName;
	PropertyDataOffset TimeAcc;
	PropertyDataOffset VoiceType;
	PropertyDataOffset bAdmin;
	PropertyDataOffset bFeigningDeath;
	PropertyDataOffset bIsABot;
	PropertyDataOffset bIsFemale;
	PropertyDataOffset bIsSpectator;
	PropertyDataOffset bWaitingPlayer;
};

extern PropertyOffsets_PlayerReplicationInfo PropOffsets_PlayerReplicationInfo;

struct PropertyOffsets_Weapon
{
	PropertyDataOffset AIRating;
	PropertyDataOffset AdjustedAim;
	PropertyDataOffset AltDamageType;
	PropertyDataOffset AltFireSound;
	PropertyDataOffset AltProjectileClass;
	PropertyDataOffset AltProjectileSpeed;
	PropertyDataOffset AltRefireRate;
	PropertyDataOffset AmmoName;
	PropertyDataOffset AmmoType;
	PropertyDataOffset CockingSound;
	PropertyDataOffset DeathMessage;
	PropertyDataOffset FireOffset;
	PropertyDataOffset FireSound;
	PropertyDataOffset FiringSpeed;
	PropertyDataOffset FlareOffset;
	PropertyDataOffset FlashC;
	PropertyDataOffset FlashLength;
	PropertyDataOffset FlashO;
	PropertyDataOffset FlashS;
	PropertyDataOffset FlashTime;
	PropertyDataOffset FlashY;
	PropertyDataOffset MFTexture;
	PropertyDataOffset MaxTargetRange;
	PropertyDataOffset MessageNoAmmo;
	PropertyDataOffset Misc1Sound;
	PropertyDataOffset Misc2Sound;
	PropertyDataOffset Misc3Sound;
	PropertyDataOffset MuzzleFlare;
	PropertyDataOffset MuzzleScale;
	PropertyDataOffset MyDamageType;
	PropertyDataOffset NameColor;
	PropertyDataOffset PickupAmmoCount;
	PropertyDataOffset ProjectileClass;
	PropertyDataOffset ProjectileSpeed;
	PropertyDataOffset RefireRate;
	PropertyDataOffset ReloadCount;
	PropertyDataOffset SelectSound;
	PropertyDataOffset aimerror;
	PropertyDataOffset bAltInstantHit;
	PropertyDataOffset bAltWarnTarget;
	PropertyDataOffset bCanThrow;
	PropertyDataOffset bChangeWeapon;
	PropertyDataOffset bDrawMuzzleFlash;
	PropertyDataOffset bHideWeapon;
	PropertyDataOffset bInstantHit;
	PropertyDataOffset bLockedOn;
	PropertyDataOffset bMeleeWeapon;
	PropertyDataOffset bMuzzleFlash;
	PropertyDataOffset bOwnsCrosshair;
	PropertyDataOffset bPointing;
	PropertyDataOffset bRapidFire;
	PropertyDataOffset bRecommendAltSplashDamage;
	PropertyDataOffset bRecommendSplashDamage;
	PropertyDataOffset bSetFlashTime;
	PropertyDataOffset bSpecialIcon;
	PropertyDataOffset bSplashDamage;
	PropertyDataOffset bWarnTarget;
	PropertyDataOffset bWeaponStay;
	PropertyDataOffset bWeaponUp;
	PropertyDataOffset shakemag;
	PropertyDataOffset shaketime;
	PropertyDataOffset shakevert;
};

extern PropertyOffsets_Weapon PropOffsets_Weapon;

struct PropertyOffsets_GameInfo
{
	PropertyDataOffset AdminPassword;
	PropertyDataOffset AutoAim;
	PropertyDataOffset BaseMutator;
	PropertyDataOffset BeaconName;
	PropertyDataOffset BotMenuType;
	PropertyDataOffset CurrentID;
	PropertyDataOffset DMMessageClass;
	PropertyDataOffset DamageMutator;
	PropertyDataOffset DeathMessageClass;
	PropertyDataOffset DefaultPlayerClass;
	PropertyDataOffset DefaultPlayerName;
	PropertyDataOffset DefaultPlayerState;
	PropertyDataOffset DefaultWeapon;
	PropertyDataOffset DemoBuild;
	PropertyDataOffset DemoHasTuts;
	PropertyDataOffset Difficulty;
	PropertyDataOffset EnabledMutators;
	PropertyDataOffset EnteredMessage;
	PropertyDataOffset FailedPlaceMessage;
	PropertyDataOffset FailedSpawnMessage;
	PropertyDataOffset FailedTeamMessage;
	PropertyDataOffset GameMenuType;
	PropertyDataOffset GameName;
	PropertyDataOffset GameOptionsMenuType;
	PropertyDataOffset GamePassword;
	PropertyDataOffset GameReplicationInfo;
	PropertyDataOffset GameReplicationInfoClass;
	PropertyDataOffset GameSpeed;
	PropertyDataOffset GameUMenuType;
	PropertyDataOffset HUDType;
	PropertyDataOffset IPBanned;
	PropertyDataOffset IPPolicies;
	PropertyDataOffset ItemGoals;
	PropertyDataOffset KillGoals;
	PropertyDataOffset LeftMessage;
	PropertyDataOffset LocalLog;
	PropertyDataOffset LocalLogFileName;
	PropertyDataOffset MapListType;
	PropertyDataOffset MapPrefix;
	PropertyDataOffset MaxPlayers;
	PropertyDataOffset MaxSpectators;
	PropertyDataOffset MaxedOutMessage;
	PropertyDataOffset MessageMutator;
	PropertyDataOffset MultiplayerUMenuType;
	PropertyDataOffset MutatorClass;
	PropertyDataOffset NameChangedMessage;
	PropertyDataOffset NeedPassword;
	PropertyDataOffset NumPlayers;
	PropertyDataOffset NumSpectators;
	PropertyDataOffset RulesMenuType;
	PropertyDataOffset ScoreBoardType;
	PropertyDataOffset SecretGoals;
	PropertyDataOffset SentText;
	PropertyDataOffset ServerLogName;
	PropertyDataOffset SettingsMenuType;
	PropertyDataOffset SpecialDamageString;
	PropertyDataOffset StartTime;
	PropertyDataOffset StatLogClass;
	PropertyDataOffset SwitchLevelMessage;
	PropertyDataOffset WaterZoneType;
	PropertyDataOffset WorldLog;
	PropertyDataOffset WorldLogFileName;
	PropertyDataOffset WrongPassword;
	PropertyDataOffset bAllowFOV;
	PropertyDataOffset bAlternateMode;
	PropertyDataOffset bBatchLocal;
	PropertyDataOffset bCanChangeSkin;
	PropertyDataOffset bCanViewOthers;
	PropertyDataOffset bClassicDeathMessages;
	PropertyDataOffset bCoopWeaponMode;
	PropertyDataOffset bDeathMatch;
	PropertyDataOffset bExternalBatcher;
	PropertyDataOffset bGameEnded;
	PropertyDataOffset bHumansOnly;
	PropertyDataOffset bLocalLog;
	PropertyDataOffset bLoggingGame;
	PropertyDataOffset bLowGore;
	PropertyDataOffset bMuteSpectators;
	PropertyDataOffset bNoCheating;
	PropertyDataOffset bNoMonsters;
	PropertyDataOffset bOverTime;
	PropertyDataOffset bPauseable;
	PropertyDataOffset bRestartLevel;
	PropertyDataOffset bTeamGame;
	PropertyDataOffset bVeryLowGore;
	PropertyDataOffset bWorldLog;
};

extern PropertyOffsets_GameInfo PropOffsets_GameInfo;

struct PropertyOffsets_ZoneInfo
{
	PropertyDataOffset AmbientBrightness;
	PropertyDataOffset AmbientHue;
	PropertyDataOffset AmbientSaturation;
	PropertyDataOffset CutoffHz;
	PropertyDataOffset DamagePerSec;
	PropertyDataOffset DamageString;
	PropertyDataOffset DamageType;
	PropertyDataOffset Delay;
	PropertyDataOffset EntryActor;
	PropertyDataOffset EntrySound;
	PropertyDataOffset EnvironmentMap;
	PropertyDataOffset ExitActor;
	PropertyDataOffset ExitSound;
	PropertyDataOffset FogColor;
	PropertyDataOffset FogDistance;
	PropertyDataOffset Gain;
	PropertyDataOffset LensFlare;
	PropertyDataOffset LensFlareOffset;
	PropertyDataOffset LensFlareScale;
	PropertyDataOffset MasterGain;
	PropertyDataOffset MaxCarcasses;
	PropertyDataOffset MaxLightCount;
	PropertyDataOffset MaxLightingPolyCount;
	PropertyDataOffset MinLightCount;
	PropertyDataOffset MinLightingPolyCount;
	PropertyDataOffset NumCarcasses;
	PropertyDataOffset SkyZone;
	PropertyDataOffset SpeedOfSound;
	PropertyDataOffset TexUPanSpeed;
	PropertyDataOffset TexVPanSpeed;
	PropertyDataOffset ViewFlash;
	PropertyDataOffset ViewFog;
	PropertyDataOffset ZoneFluidFriction;
	PropertyDataOffset ZoneGravity;
	PropertyDataOffset ZoneGroundFriction;
	PropertyDataOffset ZoneName;
	PropertyDataOffset ZonePlayerCount;
	PropertyDataOffset ZonePlayerEvent;
	PropertyDataOffset ZoneTag;
	PropertyDataOffset ZoneTerminalVelocity;
	PropertyDataOffset ZoneVelocity;
	PropertyDataOffset bBounceVelocity;
	PropertyDataOffset bDestructive;
	PropertyDataOffset bFogZone;
	PropertyDataOffset bGravityZone;
	PropertyDataOffset bKillZone;
	PropertyDataOffset bMoveProjectiles;
	PropertyDataOffset bNeutralZone;
	PropertyDataOffset bNoInventory;
	PropertyDataOffset bPainZone;
	PropertyDataOffset bRaytraceReverb;
	PropertyDataOffset bReverbZone;
	PropertyDataOffset bWaterZone;
	PropertyDataOffset locationid;
};

extern PropertyOffsets_ZoneInfo PropOffsets_ZoneInfo;

struct PropertyOffsets_Canvas
{
	PropertyDataOffset BigFont;
	PropertyDataOffset ClipX;
	PropertyDataOffset ClipY;
	PropertyDataOffset CurX;
	PropertyDataOffset CurY;
	PropertyDataOffset CurYL;
	PropertyDataOffset DrawColor;
	PropertyDataOffset Font;
	PropertyDataOffset FramePtr;
	PropertyDataOffset LargeFont;
	PropertyDataOffset MedFont;
	PropertyDataOffset OrgX;
	PropertyDataOffset OrgY;
	PropertyDataOffset RenderPtr;
	PropertyDataOffset SizeX;
	PropertyDataOffset SizeY;
	PropertyDataOffset SmallFont;
	PropertyDataOffset SpaceX;
	PropertyDataOffset SpaceY;
	PropertyDataOffset Style;
	PropertyDataOffset Viewport;
	PropertyDataOffset Z;
	PropertyDataOffset bCenter;
	PropertyDataOffset bNoSmooth;
};

extern PropertyOffsets_Canvas PropOffsets_Canvas;

struct PropertyOffsets_SavedMove
{
	PropertyDataOffset Delta;
	PropertyDataOffset DodgeMove;
	PropertyDataOffset NextMove;
	PropertyDataOffset TimeStamp;
	PropertyDataOffset bAltFire;
	PropertyDataOffset bDuck;
	PropertyDataOffset bFire;
	PropertyDataOffset bForceAltFire;
	PropertyDataOffset bForceFire;
	PropertyDataOffset bPressedJump;
	PropertyDataOffset bRun;
};

extern PropertyOffsets_SavedMove PropOffsets_SavedMove;

struct PropertyOffsets_StatLog
{
	PropertyDataOffset Context;
	PropertyDataOffset DecoderRingURL;
	PropertyDataOffset GameCreator;
	PropertyDataOffset GameCreatorURL;
	PropertyDataOffset GameName;
	PropertyDataOffset LocalBatcherParams;
	PropertyDataOffset LocalBatcherURL;
	PropertyDataOffset LocalLogDir;
	PropertyDataOffset LocalStandard;
	PropertyDataOffset LocalStatsURL;
	PropertyDataOffset LogInfoURL;
	PropertyDataOffset LogVersion;
	PropertyDataOffset TimeStamp;
	PropertyDataOffset WorldBatcherParams;
	PropertyDataOffset WorldBatcherURL;
	PropertyDataOffset WorldLogDir;
	PropertyDataOffset WorldStandard;
	PropertyDataOffset WorldStatsURL;
	PropertyDataOffset bWorld;
	PropertyDataOffset bWorldBatcherError;
};

extern PropertyOffsets_StatLog PropOffsets_StatLog;

struct PropertyOffsets_Texture
{
	PropertyDataOffset Accumulator;
	PropertyDataOffset Alpha;
	PropertyDataOffset AnimCurrent;
	PropertyDataOffset AnimNext;
	PropertyDataOffset BumpMap;
	PropertyDataOffset CompFormat;
	PropertyDataOffset CompMips;
	PropertyDataOffset DetailTexture;
	PropertyDataOffset Diffuse;
	PropertyDataOffset DrawScale;
	PropertyDataOffset FootstepSound;
	PropertyDataOffset Friction;
	PropertyDataOffset HitSound;
	PropertyDataOffset LODSet;
	PropertyDataOffset MacroTexture;
	PropertyDataOffset MaxFrameRate;
	PropertyDataOffset MinFrameRate;
	PropertyDataOffset MipMult;
	PropertyDataOffset Mips;
	PropertyDataOffset PrimeCount;
	PropertyDataOffset PrimeCurrent;
	PropertyDataOffset Specular;
	PropertyDataOffset bAutoUPan;
	PropertyDataOffset bAutoVPan;
	PropertyDataOffset bBigWavy;
	PropertyDataOffset bCloudWavy;
	PropertyDataOffset bDirtyShadows;
	PropertyDataOffset bEnvironment;
	PropertyDataOffset bFakeBackdrop;
	PropertyDataOffset bGouraud;
	PropertyDataOffset bHasComp;
	PropertyDataOffset bHighColorQuality;
	PropertyDataOffset bHighLedge;
	PropertyDataOffset bHighShadowDetail;
	PropertyDataOffset bHighTextureQuality;
	PropertyDataOffset bInvisible;
	PropertyDataOffset bLowShadowDetail;
	PropertyDataOffset bMasked;
	PropertyDataOffset bMirrored;
	PropertyDataOffset bModulate;
	PropertyDataOffset bNoMerge;
	PropertyDataOffset bNoSmooth;
	PropertyDataOffset bNotSolid;
	PropertyDataOffset bParametric;
	PropertyDataOffset bPortal;
	PropertyDataOffset bRealtime;
	PropertyDataOffset bRealtimeChanged;
	PropertyDataOffset bSemisolid;
	PropertyDataOffset bSmallWavy;
	PropertyDataOffset bSpecialLit;
	PropertyDataOffset bTransparent;
	PropertyDataOffset bTwoSided;
	PropertyDataOffset bUnlit;
	PropertyDataOffset bWaterWavy;
	PropertyDataOffset bX2;
	PropertyDataOffset bX3;
	PropertyDataOffset bX4;
	PropertyDataOffset bX5;
	PropertyDataOffset bX6;
	PropertyDataOffset bX7;
};

extern PropertyOffsets_Texture PropOffsets_Texture;

struct PropertyOffsets_Ammo
{
	PropertyDataOffset AmmoAmount;
	PropertyDataOffset MaxAmmo;
	PropertyDataOffset PAmmo;
	PropertyDataOffset ParentAmmo;
	PropertyDataOffset UsedInWeaponSlot;
};

extern PropertyOffsets_Ammo PropOffsets_Ammo;

struct PropertyOffsets_NavigationPoint
{
	PropertyDataOffset ExtraCost;
	PropertyDataOffset Paths;
	PropertyDataOffset PrunedPaths;
	PropertyDataOffset RouteCache;
	PropertyDataOffset VisNoReachPaths;
	PropertyDataOffset bAutoBuilt;
	PropertyDataOffset bEndPoint;
	PropertyDataOffset bEndPointOnly;
	PropertyDataOffset bNeverUseStrafing;
	PropertyDataOffset bOneWayPath;
	PropertyDataOffset bPlayerOnly;
	PropertyDataOffset bSpecialCost;
	PropertyDataOffset bTwoWay;
	PropertyDataOffset bestPathWeight;
	PropertyDataOffset cost;
	PropertyDataOffset nextNavigationPoint;
	PropertyDataOffset nextOrdered;
	PropertyDataOffset ownerTeam;
	PropertyDataOffset prevOrdered;
	PropertyDataOffset previousPath;
	PropertyDataOffset startPath;
	PropertyDataOffset taken;
	PropertyDataOffset upstreamPaths;
	PropertyDataOffset visitedWeight;
};

extern PropertyOffsets_NavigationPoint PropOffsets_NavigationPoint;

struct PropertyOffsets_Mutator
{
	PropertyDataOffset DefaultWeapon;
	PropertyDataOffset NextDamageMutator;
	PropertyDataOffset NextHUDMutator;
	PropertyDataOffset NextMessageMutator;
	PropertyDataOffset NextMutator;
	PropertyDataOffset bHUDMutator;
};

extern PropertyOffsets_Mutator PropOffsets_Mutator;

struct PropertyOffsets_Mover
{
	PropertyDataOffset BasePos;
	PropertyDataOffset BaseRot;
	PropertyDataOffset BrushRaytraceKey;
	PropertyDataOffset BumpEvent;
	PropertyDataOffset BumpType;
	PropertyDataOffset ClientUpdate;
	PropertyDataOffset ClosedSound;
	PropertyDataOffset ClosingSound;
	PropertyDataOffset DamageThreshold;
	PropertyDataOffset DelayTime;
	PropertyDataOffset EncroachDamage;
	PropertyDataOffset Follower;
	PropertyDataOffset KeyNum;
	PropertyDataOffset KeyPos;
	PropertyDataOffset KeyRot;
	PropertyDataOffset Leader;
	PropertyDataOffset MoveAmbientSound;
	PropertyDataOffset MoveTime;
	PropertyDataOffset MoverEncroachType;
	PropertyDataOffset MoverGlideType;
	PropertyDataOffset NumKeys;
	PropertyDataOffset OldPos;
	PropertyDataOffset OldPrePivot;
	PropertyDataOffset OldRot;
	PropertyDataOffset OpenedSound;
	PropertyDataOffset OpeningSound;
	PropertyDataOffset OtherTime;
	PropertyDataOffset PlayerBumpEvent;
	PropertyDataOffset PrevKeyNum;
	PropertyDataOffset RealPosition;
	PropertyDataOffset RealRotation;
	PropertyDataOffset RecommendedTrigger;
	PropertyDataOffset ReturnGroup;
	PropertyDataOffset SavedPos;
	PropertyDataOffset SavedRot;
	PropertyDataOffset SavedTrigger;
	PropertyDataOffset SimInterpolate;
	PropertyDataOffset SimOldPos;
	PropertyDataOffset SimOldRotPitch;
	PropertyDataOffset SimOldRotRoll;
	PropertyDataOffset SimOldRotYaw;
	PropertyDataOffset StayOpenTime;
	PropertyDataOffset TriggerActor;
	PropertyDataOffset TriggerActor2;
	PropertyDataOffset WaitingPawn;
	PropertyDataOffset WorldRaytraceKey;
	PropertyDataOffset bClientPause;
	PropertyDataOffset bDamageTriggered;
	PropertyDataOffset bDelaying;
	PropertyDataOffset bDynamicLightMover;
	PropertyDataOffset bOpening;
	PropertyDataOffset bPlayerOnly;
	PropertyDataOffset bSlave;
	PropertyDataOffset bTriggerOnceOnly;
	PropertyDataOffset bUseTriggered;
	PropertyDataOffset myMarker;
	PropertyDataOffset numTriggerEvents;
};

extern PropertyOffsets_Mover PropOffsets_Mover;

struct PropertyOffsets_HUD
{
	PropertyDataOffset Crosshair;
	PropertyDataOffset HUDConfigWindowType;
	PropertyDataOffset HUDMutator;
	PropertyDataOffset HudMode;
	PropertyDataOffset MainMenu;
	PropertyDataOffset MainMenuType;
	PropertyDataOffset PlayerOwner;
	PropertyDataOffset WhiteColor;
};

extern PropertyOffsets_HUD PropOffsets_HUD;

struct PropertyOffsets_Decoration
{
	PropertyDataOffset EffectWhenDestroyed;
	PropertyDataOffset EndPushSound;
	PropertyDataOffset PushSound;
	PropertyDataOffset bBobbing;
	PropertyDataOffset bOnlyTriggerable;
	PropertyDataOffset bPushSoundPlaying;
	PropertyDataOffset bPushable;
	PropertyDataOffset bSplash;
	PropertyDataOffset bWasCarried;
	PropertyDataOffset content2;
	PropertyDataOffset content3;
	PropertyDataOffset contents;
	PropertyDataOffset numLandings;
};

extern PropertyOffsets_Decoration PropOffsets_Decoration;

struct PropertyOffsets_TestInfo
{
	PropertyDataOffset MyArray;
	PropertyDataOffset ST;
	PropertyDataOffset TestRepStr;
	PropertyDataOffset bBool1;
	PropertyDataOffset bBool2;
	PropertyDataOffset bFalse1;
	PropertyDataOffset bFalse2;
	PropertyDataOffset bTrue1;
	PropertyDataOffset bTrue2;
	PropertyDataOffset ppp;
	PropertyDataOffset sxx;
	PropertyDataOffset v1;
	PropertyDataOffset v2;
	PropertyDataOffset xnum;
};

extern PropertyOffsets_TestInfo PropOffsets_TestInfo;

struct PropertyOffsets_GameReplicationInfo
{
	PropertyDataOffset AdminEmail;
	PropertyDataOffset AdminName;
	PropertyDataOffset ElapsedTime;
	PropertyDataOffset GameClass;
	PropertyDataOffset GameEndedComments;
	PropertyDataOffset GameName;
	PropertyDataOffset MOTDLine1;
	PropertyDataOffset MOTDLine2;
	PropertyDataOffset MOTDLine3;
	PropertyDataOffset MOTDLine4;
	PropertyDataOffset NumPlayers;
	PropertyDataOffset PRIArray;
	PropertyDataOffset Region;
	PropertyDataOffset RemainingMinute;
	PropertyDataOffset RemainingTime;
	PropertyDataOffset SecondCount;
	PropertyDataOffset ServerName;
	PropertyDataOffset ShortName;
	PropertyDataOffset SumFrags;
	PropertyDataOffset UpdateTimer;
	PropertyDataOffset bClassicDeathMessages;
	PropertyDataOffset bStopCountDown;
	PropertyDataOffset bTeamGame;
};

extern PropertyOffsets_GameReplicationInfo PropOffsets_GameReplicationInfo;

struct PropertyOffsets_Menu
{
	PropertyDataOffset CenterString;
	PropertyDataOffset DisabledString;
	PropertyDataOffset EnabledString;
	PropertyDataOffset HelpMessage;
	PropertyDataOffset LeftString;
	PropertyDataOffset MenuLength;
	PropertyDataOffset MenuList;
	PropertyDataOffset MenuTitle;
	PropertyDataOffset NoString;
	PropertyDataOffset ParentMenu;
	PropertyDataOffset PlayerOwner;
	PropertyDataOffset RightString;
	PropertyDataOffset Selection;
	PropertyDataOffset YesString;
	PropertyDataOffset bConfigChanged;
	PropertyDataOffset bExitAllMenus;
};

extern PropertyOffsets_Menu PropOffsets_Menu;

struct PropertyOffsets_LiftExit
{
	PropertyDataOffset LastTriggerTime;
	PropertyDataOffset LiftTag;
	PropertyDataOffset LiftTrigger;
	PropertyDataOffset MyLift;
	PropertyDataOffset RecommendedTrigger;
};

extern PropertyOffsets_LiftExit PropOffsets_LiftExit;

struct PropertyOffsets_Trigger
{
	PropertyDataOffset ClassProximityType;
	PropertyDataOffset DamageThreshold;
	PropertyDataOffset Message;
	PropertyDataOffset ReTriggerDelay;
	PropertyDataOffset RepeatTriggerTime;
	PropertyDataOffset TriggerActor;
	PropertyDataOffset TriggerActor2;
	PropertyDataOffset TriggerTime;
	PropertyDataOffset TriggerType;
	PropertyDataOffset bInitiallyActive;
	PropertyDataOffset bTriggerOnceOnly;
};

extern PropertyOffsets_Trigger PropOffsets_Trigger;

struct PropertyOffsets_Player
{
	PropertyDataOffset Actor;
	PropertyDataOffset ConfiguredInternetSpeed;
	PropertyDataOffset ConfiguredLanSpeed;
	PropertyDataOffset Console;
	PropertyDataOffset CurrentNetSpeed;
	PropertyDataOffset SelectedCursor;
	PropertyDataOffset WindowsMouseX;
	PropertyDataOffset WindowsMouseY;
	PropertyDataOffset bShowWindowsMouse;
	PropertyDataOffset bSuspendPrecaching;
	PropertyDataOffset bWindowsMouseAvailable;
	PropertyDataOffset vfExec;
	PropertyDataOffset vfOut;
};

extern PropertyOffsets_Player PropOffsets_Player;

struct PropertyOffsets_LocalMessage
{
	PropertyDataOffset ChildMessage;
	PropertyDataOffset DrawColor;
	PropertyDataOffset Lifetime;
	PropertyDataOffset XPos;
	PropertyDataOffset YPos;
	PropertyDataOffset bBeep;
	PropertyDataOffset bCenter;
	PropertyDataOffset bComplexString;
	PropertyDataOffset bFadeMessage;
	PropertyDataOffset bFromBottom;
	PropertyDataOffset bIsConsoleMessage;
	PropertyDataOffset bIsSpecial;
	PropertyDataOffset bIsUnique;
	PropertyDataOffset bOffsetYPos;
};

extern PropertyOffsets_LocalMessage PropOffsets_LocalMessage;

struct PropertyOffsets_locationid
{
	PropertyDataOffset LocationName;
	PropertyDataOffset NextLocation;
	PropertyDataOffset Radius;
};

extern PropertyOffsets_locationid PropOffsets_locationid;

struct PropertyOffsets_Carcass
{
	PropertyDataOffset Bugs;
	PropertyDataOffset CumulativeDamage;
	PropertyDataOffset PlayerOwner;
	PropertyDataOffset bDecorative;
	PropertyDataOffset bPlayerCarcass;
	PropertyDataOffset bReducedHeight;
	PropertyDataOffset bSlidingCarcass;
	PropertyDataOffset flies;
	PropertyDataOffset rats;
};

extern PropertyOffsets_Carcass PropOffsets_Carcass;

struct PropertyOffsets_InterpolationPoint
{
	PropertyDataOffset FovModifier;
	PropertyDataOffset GameSpeedModifier;
	PropertyDataOffset Next;
	PropertyDataOffset Position;
	PropertyDataOffset Prev;
	PropertyDataOffset RateModifier;
	PropertyDataOffset ScreenFlashFog;
	PropertyDataOffset ScreenFlashScale;
	PropertyDataOffset bEndOfPath;
	PropertyDataOffset bSkipNextPath;
};

extern PropertyOffsets_InterpolationPoint PropOffsets_InterpolationPoint;

struct PropertyOffsets_Projectile
{
	PropertyDataOffset Damage;
	PropertyDataOffset ExploWallOut;
	PropertyDataOffset ExplosionDecal;
	PropertyDataOffset ImpactSound;
	PropertyDataOffset MaxSpeed;
	PropertyDataOffset MiscSound;
	PropertyDataOffset MomentumTransfer;
	PropertyDataOffset MyDamageType;
	PropertyDataOffset SpawnSound;
	PropertyDataOffset speed;
};

extern PropertyOffsets_Projectile PropOffsets_Projectile;

struct PropertyOffsets_Teleporter
{
	PropertyDataOffset LastFired;
	PropertyDataOffset ProductRequired;
	PropertyDataOffset TargetVelocity;
	PropertyDataOffset TriggerActor;
	PropertyDataOffset TriggerActor2;
	PropertyDataOffset URL;
	PropertyDataOffset bChangesVelocity;
	PropertyDataOffset bChangesYaw;
	PropertyDataOffset bEnabled;
	PropertyDataOffset bReversesX;
	PropertyDataOffset bReversesY;
	PropertyDataOffset bReversesZ;
};

extern PropertyOffsets_Teleporter PropOffsets_Teleporter;

struct PropertyOffsets_Palette
{
	PropertyDataOffset Colors;
};

extern PropertyOffsets_Palette PropOffsets_Palette;

struct PropertyOffsets_SpawnNotify
{
	PropertyDataOffset ActorClass;
	PropertyDataOffset Next;
};

extern PropertyOffsets_SpawnNotify PropOffsets_SpawnNotify;

struct PropertyOffsets_Fragment
{
	PropertyDataOffset Fragments;
	PropertyDataOffset bFirstHit;
	PropertyDataOffset numFragmentTypes;
};

extern PropertyOffsets_Fragment PropOffsets_Fragment;

struct PropertyOffsets_WarpZoneInfo
{
	PropertyDataOffset Destinations;
	PropertyDataOffset OtherSideActor;
	PropertyDataOffset OtherSideLevel;
	PropertyDataOffset OtherSideURL;
	PropertyDataOffset ThisTag;
	PropertyDataOffset WarpCoords;
	PropertyDataOffset bNoTeleFrag;
	PropertyDataOffset iWarpZone;
	PropertyDataOffset numDestinations;
};

extern PropertyOffsets_WarpZoneInfo PropOffsets_WarpZoneInfo;

struct PropertyOffsets_Console
{
	PropertyDataOffset AvgText;
	PropertyDataOffset Border;
	PropertyDataOffset BorderLines;
	PropertyDataOffset BorderPixels;
	PropertyDataOffset BorderSize;
	PropertyDataOffset ConBackground;
	PropertyDataOffset ConnectingMessage;
	PropertyDataOffset ConsoleDest;
	PropertyDataOffset ConsoleLines;
	PropertyDataOffset ConsolePos;
	PropertyDataOffset ExtraTime;
	PropertyDataOffset FrameCount;
	PropertyDataOffset FrameRateText;
	PropertyDataOffset FrameX;
	PropertyDataOffset FrameY;
	PropertyDataOffset FramesText;
	PropertyDataOffset History;
	PropertyDataOffset HistoryBot;
	PropertyDataOffset HistoryCur;
	PropertyDataOffset HistoryTop;
	PropertyDataOffset LastFrameTime;
	PropertyDataOffset LastSecFPS;
	PropertyDataOffset LastSecText;
	PropertyDataOffset LastSecondFrameCount;
	PropertyDataOffset LastSecondStartTime;
	PropertyDataOffset LoadingMessage;
	PropertyDataOffset MaxFPS;
	PropertyDataOffset MaxText;
	PropertyDataOffset MinFPS;
	PropertyDataOffset MinText;
	PropertyDataOffset MsgPlayer;
	PropertyDataOffset MsgText;
	PropertyDataOffset MsgTick;
	PropertyDataOffset MsgTickTime;
	PropertyDataOffset MsgTime;
	PropertyDataOffset MsgType;
	PropertyDataOffset PausedMessage;
	PropertyDataOffset PrecachingMessage;
	PropertyDataOffset SavingMessage;
	PropertyDataOffset Scrollback;
	PropertyDataOffset SecondsText;
	PropertyDataOffset StartTime;
	PropertyDataOffset TextLines;
	PropertyDataOffset TimeDemoFont;
	PropertyDataOffset TopLine;
	PropertyDataOffset TypedStr;
	PropertyDataOffset Viewport;
	PropertyDataOffset bNoDrawWorld;
	PropertyDataOffset bNoStuff;
	PropertyDataOffset bRestartTimeDemo;
	PropertyDataOffset bSaveTimeDemoToFile;
	PropertyDataOffset bStartTimeDemo;
	PropertyDataOffset bTimeDemo;
	PropertyDataOffset bTyping;
	PropertyDataOffset fpsText;
	PropertyDataOffset numLines;
	PropertyDataOffset vtblOut;
};

extern PropertyOffsets_Console PropOffsets_Console;

struct PropertyOffsets_PlayerStart
{
	PropertyDataOffset TeamNumber;
	PropertyDataOffset bCoopStart;
	PropertyDataOffset bEnabled;
	PropertyDataOffset bSinglePlayerStart;
};

extern PropertyOffsets_PlayerStart PropOffsets_PlayerStart;

struct PropertyOffsets_Pickup
{
	PropertyDataOffset ExpireMessage;
	PropertyDataOffset Inv;
	PropertyDataOffset NumCopies;
	PropertyDataOffset bAutoActivate;
	PropertyDataOffset bCanActivate;
	PropertyDataOffset bCanHaveMultipleCopies;
};

extern PropertyOffsets_Pickup PropOffsets_Pickup;

struct PropertyOffsets_Brush
{
	PropertyDataOffset BrushColor;
	PropertyDataOffset CsgOper;
	PropertyDataOffset MainScale;
	PropertyDataOffset PolyFlags;
	PropertyDataOffset PostPivot;
	PropertyDataOffset PostScale;
	PropertyDataOffset TempScale;
	PropertyDataOffset UnusedLightMesh;
	PropertyDataOffset bColored;
};

extern PropertyOffsets_Brush PropOffsets_Brush;

struct PropertyOffsets_ScoreBoard
{
	PropertyDataOffset OwnerHUD;
	PropertyDataOffset RegFont;
};

extern PropertyOffsets_ScoreBoard PropOffsets_ScoreBoard;

struct PropertyOffsets_Spectator
{
	PropertyDataOffset bChaseCam;
};

extern PropertyOffsets_Spectator PropOffsets_Spectator;

struct PropertyOffsets_InventorySpot
{
	PropertyDataOffset markedItem;
};

extern PropertyOffsets_InventorySpot PropOffsets_InventorySpot;

struct PropertyOffsets_Decal
{
	PropertyDataOffset LastRenderedTime;
	PropertyDataOffset MultiDecalLevel;
	PropertyDataOffset SurfList;
};

extern PropertyOffsets_Decal PropOffsets_Decal;

struct PropertyOffsets_PatrolPoint
{
	PropertyDataOffset AnimCount;
	PropertyDataOffset NextPatrolPoint;
	PropertyDataOffset Nextpatrol;
	PropertyDataOffset PatrolAnim;
	PropertyDataOffset PatrolSound;
	PropertyDataOffset lookDir;
	PropertyDataOffset numAnims;
	PropertyDataOffset pausetime;
};

extern PropertyOffsets_PatrolPoint PropOffsets_PatrolPoint;

struct PropertyOffsets_Counter
{
	PropertyDataOffset CompleteMessage;
	PropertyDataOffset CountMessage;
	PropertyDataOffset NumToCount;
	PropertyDataOffset OriginalNum;
	PropertyDataOffset bShowMessage;
};

extern PropertyOffsets_Counter PropOffsets_Counter;

struct PropertyOffsets_Bitmap
{
	PropertyDataOffset Format;
	PropertyDataOffset InternalTime;
	PropertyDataOffset MaxColor;
	PropertyDataOffset MipZero;
	PropertyDataOffset Palette;
	PropertyDataOffset UBits;
	PropertyDataOffset UClamp;
	PropertyDataOffset USize;
	PropertyDataOffset VBits;
	PropertyDataOffset VClamp;
	PropertyDataOffset VSize;
};

extern PropertyOffsets_Bitmap PropOffsets_Bitmap;

struct PropertyOffsets_MapList
{
	PropertyDataOffset MapNum;
	PropertyDataOffset Maps;
};

extern PropertyOffsets_MapList PropOffsets_MapList;

struct PropertyOffsets_Effects
{
	PropertyDataOffset EffectSound1;
	PropertyDataOffset EffectSound2;
	PropertyDataOffset bOnlyTriggerable;
};

extern PropertyOffsets_Effects PropOffsets_Effects;

struct PropertyOffsets_StatLogFile
{
	PropertyDataOffset LogAr;
	PropertyDataOffset StatLogFile;
	PropertyDataOffset StatLogFinal;
	PropertyDataOffset bWatermark;
};

extern PropertyOffsets_StatLogFile PropOffsets_StatLogFile;

struct PropertyOffsets_LevelSummary
{
	PropertyDataOffset Author;
	PropertyDataOffset IdealPlayerCount;
	PropertyDataOffset LevelEnterText;
	PropertyDataOffset RecommendedEnemies;
	PropertyDataOffset RecommendedTeammates;
	PropertyDataOffset Title;
};

extern PropertyOffsets_LevelSummary PropOffsets_LevelSummary;

struct PropertyOffsets_ScriptedTexture
{
	PropertyDataOffset Junk1;
	PropertyDataOffset Junk2;
	PropertyDataOffset Junk3;
	PropertyDataOffset LocalTime;
	PropertyDataOffset NotifyActor;
	PropertyDataOffset SourceTexture;
};

extern PropertyOffsets_ScriptedTexture PropOffsets_ScriptedTexture;

struct PropertyOffsets_Engine
{
	PropertyDataOffset Audio;
	PropertyDataOffset AudioDevice;
	PropertyDataOffset CacheSizeMegs;
	PropertyDataOffset Client;
	PropertyDataOffset ClientCycles;
	PropertyDataOffset Console;
	PropertyDataOffset CurrentTickRate;
	PropertyDataOffset Cylinder;
	PropertyDataOffset GameCycles;
	PropertyDataOffset GameRenderDevice;
	PropertyDataOffset Language;
	PropertyDataOffset NetworkDevice;
	PropertyDataOffset Render;
	PropertyDataOffset TickCycles;
	PropertyDataOffset UseSound;
};

extern PropertyOffsets_Engine PropOffsets_Engine;

struct PropertyOffsets_TriggerLight
{
	PropertyDataOffset Alpha;
	PropertyDataOffset ChangeTime;
	PropertyDataOffset Direction;
	PropertyDataOffset InitialBrightness;
	PropertyDataOffset RemainOnTime;
	PropertyDataOffset SavedTrigger;
	PropertyDataOffset bDelayFullOn;
	PropertyDataOffset bInitiallyOn;
	PropertyDataOffset poundTime;
};

extern PropertyOffsets_TriggerLight PropOffsets_TriggerLight;

struct PropertyOffsets_SpecialEvent
{
	PropertyDataOffset Damage;
	PropertyDataOffset DamageString;
	PropertyDataOffset DamageType;
	PropertyDataOffset Message;
	PropertyDataOffset Sound;
	PropertyDataOffset bBroadcast;
	PropertyDataOffset bPlayerViewRot;
};

extern PropertyOffsets_SpecialEvent PropOffsets_SpecialEvent;

struct PropertyOffsets_RoundRobin
{
	PropertyDataOffset OutEvents;
	PropertyDataOffset bLoop;
	PropertyDataOffset i;
};

extern PropertyOffsets_RoundRobin PropOffsets_RoundRobin;

struct PropertyOffsets_MusicEvent
{
	PropertyDataOffset CdTrack;
	PropertyDataOffset Song;
	PropertyDataOffset SongSection;
	PropertyDataOffset Transition;
	PropertyDataOffset bAffectAllPlayers;
	PropertyDataOffset bOnceOnly;
	PropertyDataOffset bSilence;
};

extern PropertyOffsets_MusicEvent PropOffsets_MusicEvent;

struct PropertyOffsets_HomeBase
{
	PropertyDataOffset Extent;
	PropertyDataOffset lookDir;
};

extern PropertyOffsets_HomeBase PropOffsets_HomeBase;

struct PropertyOffsets_Dispatcher
{
	PropertyDataOffset OutDelays;
	PropertyDataOffset OutEvents;
	PropertyDataOffset i;
};

extern PropertyOffsets_Dispatcher PropOffsets_Dispatcher;

struct PropertyOffsets_DemoRecSpectator
{
	PropertyDataOffset PlaybackActor;
	PropertyDataOffset PlaybackGRI;
};

extern PropertyOffsets_DemoRecSpectator PropOffsets_DemoRecSpectator;

struct PropertyOffsets_DamageType
{
	PropertyDataOffset AltName;
	PropertyDataOffset DamageEffect;
	PropertyDataOffset Name;
	PropertyDataOffset ViewFlash;
	PropertyDataOffset ViewFog;
};

extern PropertyOffsets_DamageType PropOffsets_DamageType;

struct PropertyOffsets_Ambushpoint
{
	PropertyDataOffset SightRadius;
	PropertyDataOffset bSniping;
	PropertyDataOffset lookDir;
	PropertyDataOffset survivecount;
};

extern PropertyOffsets_Ambushpoint PropOffsets_Ambushpoint;

struct PropertyOffsets_WarpZoneMarker
{
	PropertyDataOffset TriggerActor;
	PropertyDataOffset TriggerActor2;
	PropertyDataOffset markedWarpZone;
};

extern PropertyOffsets_WarpZoneMarker PropOffsets_WarpZoneMarker;

struct PropertyOffsets_LiftCenter
{
	PropertyDataOffset LastTriggerTime;
	PropertyDataOffset LiftOffset;
	PropertyDataOffset LiftTag;
	PropertyDataOffset LiftTrigger;
	PropertyDataOffset MaxDist2D;
	PropertyDataOffset MaxZDiffAdd;
	PropertyDataOffset MyLift;
	PropertyDataOffset RecommendedTrigger;
};

extern PropertyOffsets_LiftCenter PropOffsets_LiftCenter;

struct PropertyOffsets_RenderIterator
{
	PropertyDataOffset Frame;
	PropertyDataOffset Index;
	PropertyDataOffset MaxItems;
	PropertyDataOffset Observer;
};

extern PropertyOffsets_RenderIterator PropOffsets_RenderIterator;

struct PropertyOffsets_FractalTexture
{
	PropertyDataOffset AuxPhase;
	PropertyDataOffset DrawPhase;
	PropertyDataOffset GlobalPhase;
	PropertyDataOffset LightOutput;
	PropertyDataOffset SoundOutput;
	PropertyDataOffset UMask;
	PropertyDataOffset VMask;
};

extern PropertyOffsets_FractalTexture PropOffsets_FractalTexture;

struct PropertyOffsets_WaterTexture
{
	PropertyDataOffset DropType;
	PropertyDataOffset Drops;
	PropertyDataOffset FX_Amplitude;
	PropertyDataOffset FX_Depth;
	PropertyDataOffset FX_Frequency;
	PropertyDataOffset FX_Phase;
	PropertyDataOffset FX_Radius;
	PropertyDataOffset FX_Size;
	PropertyDataOffset FX_Speed;
	PropertyDataOffset FX_Time;
	PropertyDataOffset NumDrops;
	PropertyDataOffset OldWaveAmp;
	PropertyDataOffset RenderTable;
	PropertyDataOffset SourceFields;
	PropertyDataOffset WaterParity;
	PropertyDataOffset WaterTable;
	PropertyDataOffset WaveAmp;
};

extern PropertyOffsets_WaterTexture PropOffsets_WaterTexture;

struct PropertyOffsets_WaveTexture
{
	PropertyDataOffset BumpMapAngle;
	PropertyDataOffset BumpMapLight;
	PropertyDataOffset PhongRange;
	PropertyDataOffset PhongSize;
};

extern PropertyOffsets_WaveTexture PropOffsets_WaveTexture;

struct PropertyOffsets_FireTexture
{
	PropertyDataOffset DrawMode;
	PropertyDataOffset FX_Area;
	PropertyDataOffset FX_AuxSize;
	PropertyDataOffset FX_Frequency;
	PropertyDataOffset FX_Heat;
	PropertyDataOffset FX_HorizSpeed;
	PropertyDataOffset FX_Phase;
	PropertyDataOffset FX_Size;
	PropertyDataOffset FX_VertSpeed;
	PropertyDataOffset NumSparks;
	PropertyDataOffset OldRenderHeat;
	PropertyDataOffset PenDownX;
	PropertyDataOffset PenDownY;
	PropertyDataOffset RenderHeat;
	PropertyDataOffset RenderTable;
	PropertyDataOffset SparkType;
	PropertyDataOffset Sparks;
	PropertyDataOffset SparksLimit;
	PropertyDataOffset StarStatus;
	PropertyDataOffset bRising;
};

extern PropertyOffsets_FireTexture PropOffsets_FireTexture;

struct PropertyOffsets_WetTexture
{
	PropertyDataOffset LocalSourceBitmap;
	PropertyDataOffset OldSourceTex;
	PropertyDataOffset SourceTexture;
};

extern PropertyOffsets_WetTexture PropOffsets_WetTexture;

struct PropertyOffsets_IceTexture
{
	PropertyDataOffset Amplitude;
	PropertyDataOffset ForceRefresh;
	PropertyDataOffset Frequency;
	PropertyDataOffset GlassTexture;
	PropertyDataOffset HorizPanSpeed;
	PropertyDataOffset LocalSource;
	PropertyDataOffset MasterCount;
	PropertyDataOffset MoveIce;
	PropertyDataOffset OldGlassTex;
	PropertyDataOffset OldSourceTex;
	PropertyDataOffset OldUDisplace;
	PropertyDataOffset OldVDisplace;
	PropertyDataOffset PanningStyle;
	PropertyDataOffset SourceTexture;
	PropertyDataOffset TickAccu;
	PropertyDataOffset TimeMethod;
	PropertyDataOffset UDisplace;
	PropertyDataOffset UPosition;
	PropertyDataOffset VDisplace;
	PropertyDataOffset VPosition;
	PropertyDataOffset VertPanSpeed;
};

extern PropertyOffsets_IceTexture PropOffsets_IceTexture;

struct PropertyOffsets_InternetLink
{
	PropertyDataOffset DataPending;
	PropertyDataOffset LinkMode;
	PropertyDataOffset Port;
	PropertyDataOffset PrivateResolveInfo;
	PropertyDataOffset ReceiveMode;
	PropertyDataOffset RemoteSocket;
	PropertyDataOffset Socket;
};

extern PropertyOffsets_InternetLink PropOffsets_InternetLink;

struct PropertyOffsets_UdpLink
{
	PropertyDataOffset BroadcastAddr;
};

extern PropertyOffsets_UdpLink PropOffsets_UdpLink;

struct PropertyOffsets_TcpLink
{
	PropertyDataOffset AcceptClass;
	PropertyDataOffset LinkState;
	PropertyDataOffset RemoteAddr;
	PropertyDataOffset SendFIFO;
};

extern PropertyOffsets_TcpLink PropOffsets_TcpLink;
