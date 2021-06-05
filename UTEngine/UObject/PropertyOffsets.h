#pragma once

class PackageManager;

void InitPropertyOffsets(PackageManager* packages);

struct PropertyOffsets_Object
{
	size_t Class;
	size_t Name;
	size_t ObjectFlags;
	size_t ObjectInternal;
	size_t Outer;
};

extern PropertyOffsets_Object PropOffsets_Object;

struct PropertyOffsets_Commandlet
{
	size_t HelpCmd;
	size_t HelpDesc;
	size_t HelpOneLiner;
	size_t HelpParm;
	size_t HelpUsage;
	size_t HelpWebLink;
	size_t IsClient;
	size_t IsEditor;
	size_t IsServer;
	size_t LazyLoad;
	size_t LogToStdout;
	size_t ShowBanner;
	size_t ShowErrorCount;
};

extern PropertyOffsets_Commandlet PropOffsets_Commandlet;

struct PropertyOffsets_Subsystem
{
	size_t ExecVtbl;
};

extern PropertyOffsets_Subsystem PropOffsets_Subsystem;

struct PropertyOffsets_HelloWorldCommandlet
{
	size_t intparm;
	size_t strparm;
};

extern PropertyOffsets_HelloWorldCommandlet PropOffsets_HelloWorldCommandlet;

struct PropertyOffsets_SimpleCommandlet
{
	size_t intparm;
};

extern PropertyOffsets_SimpleCommandlet PropOffsets_SimpleCommandlet;

struct PropertyOffsets_Pawn
{
	size_t AccelRate;
	size_t AirControl;
	size_t AirSpeed;
	size_t AlarmTag;
	size_t Alertness;
	size_t AttitudeToPlayer;
	size_t AvgPhysicsTime;
	size_t BaseEyeHeight;
	size_t CombatStyle;
	size_t DamageScaling;
	size_t DesiredSpeed;
	size_t Destination;
	size_t Die;
	size_t DieCount;
	size_t DropWhenKilled;
	size_t Enemy;
	size_t EyeHeight;
	size_t FaceTarget;
	size_t Floor;
	size_t Focus;
	size_t FootRegion;
	size_t FovAngle;
	size_t GroundSpeed;
	size_t HeadRegion;
	size_t Health;
	size_t HearingThreshold;
	size_t HitSound1;
	size_t HitSound2;
	size_t Intelligence;
	size_t ItemCount;
	size_t JumpZ;
	size_t KillCount;
	size_t Land;
	size_t LastPainSound;
	size_t LastSeeingPos;
	size_t LastSeenPos;
	size_t LastSeenTime;
	size_t MaxDesiredSpeed;
	size_t MaxStepHeight;
	size_t MeleeRange;
	size_t MenuName;
	size_t MinHitWall;
	size_t MoveTarget;
	size_t MoveTimer;
	size_t NameArticle;
	size_t NextLabel;
	size_t NextState;
	size_t OldMessageTime;
	size_t OrthoZoom;
	size_t PainTime;
	size_t PendingWeapon;
	size_t PeripheralVision;
	size_t PlayerReStartState;
	size_t PlayerReplicationInfo;
	size_t PlayerReplicationInfoClass;
	size_t ReducedDamagePct;
	size_t ReducedDamageType;
	size_t RouteCache;
	size_t SecretCount;
	size_t SelectedItem;
	size_t SelectionMesh;
	size_t Shadow;
	size_t SharedAlarmTag;
	size_t SightCounter;
	size_t SightRadius;
	size_t Skill;
	size_t SoundDampening;
	size_t SpecialGoal;
	size_t SpecialMesh;
	size_t SpecialPause;
	size_t SpeechTime;
	size_t SplashTime;
	size_t Spree;
	size_t Stimulus;
	size_t UnderWaterTime;
	size_t ViewRotation;
	size_t Visibility;
	size_t VoicePitch;
	size_t VoiceType;
	size_t WalkBob;
	size_t WaterSpeed;
	size_t WaterStep;
	size_t Weapon;
	size_t bAdvancedTactics;
	size_t bAltFire;
	size_t bAutoActivate;
	size_t bAvoidLedges;
	size_t bBehindView;
	size_t bCanDoSpecial;
	size_t bCanFly;
	size_t bCanJump;
	size_t bCanOpenDoors;
	size_t bCanStrafe;
	size_t bCanSwim;
	size_t bCanWalk;
	size_t bCountJumps;
	size_t bDrowning;
	size_t bDuck;
	size_t bExtra0;
	size_t bExtra1;
	size_t bExtra2;
	size_t bExtra3;
	size_t bFire;
	size_t bFixedStart;
	size_t bFreeLook;
	size_t bFromWall;
	size_t bHitSlopedWall;
	size_t bHunting;
	size_t bIsFemale;
	size_t bIsHuman;
	size_t bIsMultiSkinned;
	size_t bIsPlayer;
	size_t bIsWalking;
	size_t bJumpOffPawn;
	size_t bJustLanded;
	size_t bLOSflag;
	size_t bLook;
	size_t bNeverSwitchOnPickup;
	size_t bReducedSpeed;
	size_t bRun;
	size_t bShootSpecial;
	size_t bSnapLevel;
	size_t bStopAtLedges;
	size_t bStrafe;
	size_t bUpAndOut;
	size_t bUpdatingDisplay;
	size_t bViewTarget;
	size_t bWarping;
	size_t bZoom;
	size_t carriedDecoration;
	size_t home;
	size_t nextPawn;
	size_t noise1loudness;
	size_t noise1other;
	size_t noise1spot;
	size_t noise1time;
	size_t noise2loudness;
	size_t noise2other;
	size_t noise2spot;
	size_t noise2time;
};

extern PropertyOffsets_Pawn PropOffsets_Pawn;

struct PropertyOffsets_Actor
{
	size_t Acceleration;
	size_t AmbientGlow;
	size_t AmbientSound;
	size_t AnimFrame;
	size_t AnimLast;
	size_t AnimMinRate;
	size_t AnimRate;
	size_t AnimSequence;
	size_t AttachTag;
	size_t Base;
	size_t Brush;
	size_t Buoyancy;
	size_t ColLocation;
	size_t CollisionHeight;
	size_t CollisionRadius;
	size_t CollisionTag;
	size_t Deleted;
	size_t DesiredRotation;
	size_t DodgeDir;
	size_t DrawScale;
	size_t DrawType;
	size_t Event;
	size_t ExtraTag;
	size_t Fatness;
	size_t Group;
	size_t HitActor;
	size_t InitialState;
	size_t Instigator;
	size_t Inventory;
	size_t LODBias;
	size_t LatentActor;
	size_t LatentByte;
	size_t LatentFloat;
	size_t LatentInt;
	size_t Level;
	size_t LifeSpan;
	size_t LightBrightness;
	size_t LightCone;
	size_t LightEffect;
	size_t LightHue;
	size_t LightPeriod;
	size_t LightPhase;
	size_t LightRadius;
	size_t LightSaturation;
	size_t LightType;
	size_t LightingTag;
	size_t Location;
	size_t Mass;
	size_t Mesh;
	size_t MiscNumber;
	size_t MultiSkins;
	size_t NetPriority;
	size_t NetTag;
	size_t NetUpdateFrequency;
	size_t OddsOfAppearing;
	size_t OldAnimRate;
	size_t OldLocation;
	size_t OtherTag;
	size_t Owner;
	size_t PendingTouch;
	size_t PhysAlpha;
	size_t PhysRate;
	size_t Physics;
	size_t PrePivot;
	size_t Region;
	size_t RemoteRole;
	size_t RenderInterface;
	size_t RenderIteratorClass;
	size_t Role;
	size_t Rotation;
	size_t RotationRate;
	size_t ScaleGlow;
	size_t SimAnim;
	size_t SkelAnim;
	size_t Skin;
	size_t SoundPitch;
	size_t SoundRadius;
	size_t SoundVolume;
	size_t SpecialTag;
	size_t Sprite;
	size_t SpriteProjForward;
	size_t StandingCount;
	size_t Style;
	size_t Tag;
	size_t Target;
	size_t Texture;
	size_t TimerCounter;
	size_t TimerRate;
	size_t Touching;
	size_t TransientSoundRadius;
	size_t TransientSoundVolume;
	size_t TweenRate;
	size_t Velocity;
	size_t VisibilityHeight;
	size_t VisibilityRadius;
	size_t VolumeBrightness;
	size_t VolumeFog;
	size_t VolumeRadius;
	size_t XLevel;
	size_t bActorShadows;
	size_t bAlwaysRelevant;
	size_t bAlwaysTick;
	size_t bAnimByOwner;
	size_t bAnimFinished;
	size_t bAnimLoop;
	size_t bAnimNotify;
	size_t bAssimilated;
	size_t bBlockActors;
	size_t bBlockPlayers;
	size_t bBounce;
	size_t bCanTeleport;
	size_t bCarriedItem;
	size_t bClientAnim;
	size_t bClientDemoNetFunc;
	size_t bClientDemoRecording;
	size_t bCollideActors;
	size_t bCollideWhenPlacing;
	size_t bCollideWorld;
	size_t bCorona;
	size_t bDeleteMe;
	size_t bDemoRecording;
	size_t bDifficulty0;
	size_t bDifficulty1;
	size_t bDifficulty2;
	size_t bDifficulty3;
	size_t bDirectional;
	size_t bDynamicLight;
	size_t bEdLocked;
	size_t bEdShouldSnap;
	size_t bEdSnap;
	size_t bFilterByVolume;
	size_t bFixedRotationDir;
	size_t bForcePhysicsUpdate;
	size_t bForceStasis;
	size_t bGameRelevant;
	size_t bHidden;
	size_t bHiddenEd;
	size_t bHighDetail;
	size_t bHighlighted;
	size_t bHurtEntry;
	size_t bInterpolating;
	size_t bIsItemGoal;
	size_t bIsKillGoal;
	size_t bIsMover;
	size_t bIsPawn;
	size_t bIsSecretGoal;
	size_t bJustTeleported;
	size_t bLensFlare;
	size_t bLightChanged;
	size_t bMemorized;
	size_t bMeshCurvy;
	size_t bMeshEnviroMap;
	size_t bMovable;
	size_t bNet;
	size_t bNetFeel;
	size_t bNetHear;
	size_t bNetInitial;
	size_t bNetOptional;
	size_t bNetOwner;
	size_t bNetRelevant;
	size_t bNetSee;
	size_t bNetSpecial;
	size_t bNetTemporary;
	size_t bNoDelete;
	size_t bNoSmooth;
	size_t bOnlyOwnerSee;
	size_t bOwnerNoSee;
	size_t bParticles;
	size_t bProjTarget;
	size_t bRandomFrame;
	size_t bReplicateInstigator;
	size_t bRotateToDesired;
	size_t bScriptInitialized;
	size_t bSelected;
	size_t bShadowCast;
	size_t bSimFall;
	size_t bSimulatedPawn;
	size_t bSinglePlayer;
	size_t bSpecialLit;
	size_t bStasis;
	size_t bStatic;
	size_t bTempEditor;
	size_t bTicked;
	size_t bTimerLoop;
	size_t bTrailerPrePivot;
	size_t bTrailerSameRotation;
	size_t bTravel;
	size_t bUnlit;
};

extern PropertyOffsets_Actor PropOffsets_Actor;

struct PropertyOffsets_LevelInfo
{
	size_t AIProfile;
	size_t Author;
	size_t AvgAITime;
	size_t Brightness;
	size_t CdTrack;
	size_t ComputerName;
	size_t Day;
	size_t DayOfWeek;
	size_t DefaultGameType;
	size_t DefaultTexture;
	size_t EngineVersion;
	size_t Game;
	size_t Hour;
	size_t HubStackLevel;
	size_t IdealPlayerCount;
	size_t LevelAction;
	size_t LevelEnterText;
	size_t LocalizedPkg;
	size_t Millisecond;
	size_t MinNetVersion;
	size_t Minute;
	size_t Month;
	size_t NavigationPointList;
	size_t NetMode;
	size_t NextSwitchCountdown;
	size_t NextURL;
	size_t Pauser;
	size_t PawnList;
	size_t PlayerDoppler;
	size_t RecommendedEnemies;
	size_t RecommendedTeammates;
	size_t Screenshot;
	size_t Second;
	size_t Song;
	size_t SongSection;
	size_t SpawnNotify;
	size_t Summary;
	size_t TimeDilation;
	size_t TimeSeconds;
	size_t Title;
	size_t VisibleGroups;
	size_t Year;
	size_t bAggressiveLOD;
	size_t bAllowFOV;
	size_t bBegunPlay;
	size_t bCheckWalkSurfaces;
	size_t bDropDetail;
	size_t bHighDetailMode;
	size_t bHumansOnly;
	size_t bLonePlayer;
	size_t bLowRes;
	size_t bNeverPrecache;
	size_t bNextItems;
	size_t bNoCheating;
	size_t bPlayersOnly;
	size_t bStartup;
};

extern PropertyOffsets_LevelInfo PropOffsets_LevelInfo;

struct PropertyOffsets_Inventory
{
	size_t AbsorptionPriority;
	size_t ActivateSound;
	size_t ArmorAbsorption;
	size_t AutoSwitchPriority;
	size_t BobDamping;
	size_t Charge;
	size_t DeActivateSound;
	size_t FlashCount;
	size_t Icon;
	size_t InventoryGroup;
	size_t ItemArticle;
	size_t ItemMessageClass;
	size_t ItemName;
	size_t M_Activated;
	size_t M_Deactivated;
	size_t M_Selected;
	size_t MaxDesireability;
	size_t MuzzleFlashMesh;
	size_t MuzzleFlashScale;
	size_t MuzzleFlashStyle;
	size_t MuzzleFlashTexture;
	size_t NextArmor;
	size_t OldFlashCount;
	size_t PickupMessage;
	size_t PickupMessageClass;
	size_t PickupSound;
	size_t PickupViewMesh;
	size_t PickupViewScale;
	size_t PlayerLastTouched;
	size_t PlayerViewMesh;
	size_t PlayerViewOffset;
	size_t PlayerViewScale;
	size_t ProtectionType1;
	size_t ProtectionType2;
	size_t RespawnSound;
	size_t RespawnTime;
	size_t StatusIcon;
	size_t ThirdPersonMesh;
	size_t ThirdPersonScale;
	size_t bActivatable;
	size_t bActive;
	size_t bAmbientGlow;
	size_t bDisplayableInv;
	size_t bFirstFrame;
	size_t bHeldItem;
	size_t bInstantRespawn;
	size_t bIsAnArmor;
	size_t bMuzzleFlashParticles;
	size_t bRotatingPickup;
	size_t bSleepTouch;
	size_t bSteadyFlash3rd;
	size_t bSteadyToggle;
	size_t bToggleSteadyFlash;
	size_t bTossedOut;
	size_t myMarker;
};

extern PropertyOffsets_Inventory PropOffsets_Inventory;

struct PropertyOffsets_PlayerPawn
{
	size_t AppliedBob;
	size_t Bob;
	size_t BorrowedMouseX;
	size_t BorrowedMouseY;
	size_t CarcassType;
	size_t CdTrack;
	size_t ClientUpdateTime;
	size_t ConstantGlowFog;
	size_t ConstantGlowScale;
	size_t CurrentTimeStamp;
	size_t DefaultFOV;
	size_t DelayedCommand;
	size_t DemoViewPitch;
	size_t DemoViewYaw;
	size_t DesiredFOV;
	size_t DesiredFlashFog;
	size_t DesiredFlashScale;
	size_t DodgeClickTime;
	size_t DodgeClickTimer;
	size_t FailedView;
	size_t FlashFog;
	size_t FlashScale;
	size_t FreeMoves;
	size_t GameReplicationInfo;
	size_t HUDType;
	size_t Handedness;
	size_t InstantFlash;
	size_t InstantFog;
	size_t JumpSound;
	size_t LandBob;
	size_t LastMessageWindow;
	size_t LastPlaySound;
	size_t LastUpdateTime;
	size_t MaxTimeMargin;
	size_t Misc1;
	size_t Misc2;
	size_t MouseSensitivity;
	size_t MouseSmoothThreshold;
	size_t MouseZeroTime;
	size_t MyAutoAim;
	size_t NoPauseMessage;
	size_t OwnCamera;
	size_t Password;
	size_t PendingMove;
	size_t Player;
	size_t ProgressColor;
	size_t ProgressMessage;
	size_t ProgressTimeOut;
	size_t QuickSaveString;
	size_t ReceivedSecretChecksum;
	size_t RendMap;
	size_t SavedMoves;
	size_t Scoring;
	size_t ScoringType;
	size_t ServerTimeStamp;
	size_t ShowFlags;
	size_t SmoothMouseX;
	size_t SmoothMouseY;
	size_t Song;
	size_t SongSection;
	size_t SpecialMenu;
	size_t TargetEyeHeight;
	size_t TargetViewRotation;
	size_t TargetWeaponViewOffset;
	size_t TimeMargin;
	size_t Transition;
	size_t ViewTarget;
	size_t ViewingFrom;
	size_t WeaponPriority;
	size_t ZoomLevel;
	size_t aBaseX;
	size_t aBaseY;
	size_t aBaseZ;
	size_t aExtra0;
	size_t aExtra1;
	size_t aExtra2;
	size_t aExtra3;
	size_t aExtra4;
	size_t aForward;
	size_t aLookUp;
	size_t aMouseX;
	size_t aMouseY;
	size_t aStrafe;
	size_t aTurn;
	size_t aUp;
	size_t bAdmin;
	size_t bAlwaysMouseLook;
	size_t bAnimTransition;
	size_t bBadConnectionAlert;
	size_t bCenterView;
	size_t bCheatsEnabled;
	size_t bDelayedCommand;
	size_t bEdgeBack;
	size_t bEdgeForward;
	size_t bEdgeLeft;
	size_t bEdgeRight;
	size_t bFixedCamera;
	size_t bFrozen;
	size_t bInvertMouse;
	size_t bIsCrouching;
	size_t bIsTurning;
	size_t bIsTyping;
	size_t bJumpStatus;
	size_t bJustAltFired;
	size_t bJustFired;
	size_t bKeyboardLook;
	size_t bLookUpStairs;
	size_t bMaxMouseSmoothing;
	size_t bMessageBeep;
	size_t bMouseZeroed;
	size_t bNeverAutoSwitch;
	size_t bNoFlash;
	size_t bNoVoices;
	size_t bPressedJump;
	size_t bReadyToPlay;
	size_t bReducedVis;
	size_t bRising;
	size_t bShakeDir;
	size_t bShowMenu;
	size_t bShowScores;
	size_t bSinglePlayer;
	size_t bSnapToLevel;
	size_t bSpecialMenu;
	size_t bUpdatePosition;
	size_t bUpdating;
	size_t bWasBack;
	size_t bWasForward;
	size_t bWasLeft;
	size_t bWasRight;
	size_t bWokeUp;
	size_t bZooming;
	size_t bobtime;
	size_t maxshake;
	size_t myHUD;
	size_t ngSecretSet;
	size_t ngWorldSecret;
	size_t shakemag;
	size_t shaketimer;
	size_t shakevert;
	size_t verttimer;
};

extern PropertyOffsets_PlayerPawn PropOffsets_PlayerPawn;

struct PropertyOffsets_PlayerReplicationInfo
{
	size_t Deaths;
	size_t HasFlag;
	size_t OldName;
	size_t PacketLoss;
	size_t Ping;
	size_t PlayerID;
	size_t PlayerLocation;
	size_t PlayerName;
	size_t PlayerZone;
	size_t Score;
	size_t StartTime;
	size_t TalkTexture;
	size_t Team;
	size_t TeamID;
	size_t TeamName;
	size_t TimeAcc;
	size_t VoiceType;
	size_t bAdmin;
	size_t bFeigningDeath;
	size_t bIsABot;
	size_t bIsFemale;
	size_t bIsSpectator;
	size_t bWaitingPlayer;
};

extern PropertyOffsets_PlayerReplicationInfo PropOffsets_PlayerReplicationInfo;

struct PropertyOffsets_Weapon
{
	size_t AIRating;
	size_t AdjustedAim;
	size_t AltDamageType;
	size_t AltFireSound;
	size_t AltProjectileClass;
	size_t AltProjectileSpeed;
	size_t AltRefireRate;
	size_t AmmoName;
	size_t AmmoType;
	size_t CockingSound;
	size_t DeathMessage;
	size_t FireOffset;
	size_t FireSound;
	size_t FiringSpeed;
	size_t FlareOffset;
	size_t FlashC;
	size_t FlashLength;
	size_t FlashO;
	size_t FlashS;
	size_t FlashTime;
	size_t FlashY;
	size_t MFTexture;
	size_t MaxTargetRange;
	size_t MessageNoAmmo;
	size_t Misc1Sound;
	size_t Misc2Sound;
	size_t Misc3Sound;
	size_t MuzzleFlare;
	size_t MuzzleScale;
	size_t MyDamageType;
	size_t NameColor;
	size_t PickupAmmoCount;
	size_t ProjectileClass;
	size_t ProjectileSpeed;
	size_t RefireRate;
	size_t ReloadCount;
	size_t SelectSound;
	size_t aimerror;
	size_t bAltInstantHit;
	size_t bAltWarnTarget;
	size_t bCanThrow;
	size_t bChangeWeapon;
	size_t bDrawMuzzleFlash;
	size_t bHideWeapon;
	size_t bInstantHit;
	size_t bLockedOn;
	size_t bMeleeWeapon;
	size_t bMuzzleFlash;
	size_t bOwnsCrosshair;
	size_t bPointing;
	size_t bRapidFire;
	size_t bRecommendAltSplashDamage;
	size_t bRecommendSplashDamage;
	size_t bSetFlashTime;
	size_t bSpecialIcon;
	size_t bSplashDamage;
	size_t bWarnTarget;
	size_t bWeaponStay;
	size_t bWeaponUp;
	size_t shakemag;
	size_t shaketime;
	size_t shakevert;
};

extern PropertyOffsets_Weapon PropOffsets_Weapon;

struct PropertyOffsets_GameInfo
{
	size_t AdminPassword;
	size_t AutoAim;
	size_t BaseMutator;
	size_t BeaconName;
	size_t BotMenuType;
	size_t CurrentID;
	size_t DMMessageClass;
	size_t DamageMutator;
	size_t DeathMessageClass;
	size_t DefaultPlayerClass;
	size_t DefaultPlayerName;
	size_t DefaultPlayerState;
	size_t DefaultWeapon;
	size_t DemoBuild;
	size_t DemoHasTuts;
	size_t Difficulty;
	size_t EnabledMutators;
	size_t EnteredMessage;
	size_t FailedPlaceMessage;
	size_t FailedSpawnMessage;
	size_t FailedTeamMessage;
	size_t GameMenuType;
	size_t GameName;
	size_t GameOptionsMenuType;
	size_t GamePassword;
	size_t GameReplicationInfo;
	size_t GameReplicationInfoClass;
	size_t GameSpeed;
	size_t GameUMenuType;
	size_t HUDType;
	size_t IPBanned;
	size_t IPPolicies;
	size_t ItemGoals;
	size_t KillGoals;
	size_t LeftMessage;
	size_t LocalLog;
	size_t LocalLogFileName;
	size_t MapListType;
	size_t MapPrefix;
	size_t MaxPlayers;
	size_t MaxSpectators;
	size_t MaxedOutMessage;
	size_t MessageMutator;
	size_t MultiplayerUMenuType;
	size_t MutatorClass;
	size_t NameChangedMessage;
	size_t NeedPassword;
	size_t NumPlayers;
	size_t NumSpectators;
	size_t RulesMenuType;
	size_t ScoreBoardType;
	size_t SecretGoals;
	size_t SentText;
	size_t ServerLogName;
	size_t SettingsMenuType;
	size_t SpecialDamageString;
	size_t StartTime;
	size_t StatLogClass;
	size_t SwitchLevelMessage;
	size_t WaterZoneType;
	size_t WorldLog;
	size_t WorldLogFileName;
	size_t WrongPassword;
	size_t bAllowFOV;
	size_t bAlternateMode;
	size_t bBatchLocal;
	size_t bCanChangeSkin;
	size_t bCanViewOthers;
	size_t bClassicDeathMessages;
	size_t bCoopWeaponMode;
	size_t bDeathMatch;
	size_t bExternalBatcher;
	size_t bGameEnded;
	size_t bHumansOnly;
	size_t bLocalLog;
	size_t bLoggingGame;
	size_t bLowGore;
	size_t bMuteSpectators;
	size_t bNoCheating;
	size_t bNoMonsters;
	size_t bOverTime;
	size_t bPauseable;
	size_t bRestartLevel;
	size_t bTeamGame;
	size_t bVeryLowGore;
	size_t bWorldLog;
};

extern PropertyOffsets_GameInfo PropOffsets_GameInfo;

struct PropertyOffsets_ZoneInfo
{
	size_t AmbientBrightness;
	size_t AmbientHue;
	size_t AmbientSaturation;
	size_t CutoffHz;
	size_t DamagePerSec;
	size_t DamageString;
	size_t DamageType;
	size_t Delay;
	size_t EntryActor;
	size_t EntrySound;
	size_t EnvironmentMap;
	size_t ExitActor;
	size_t ExitSound;
	size_t FogColor;
	size_t FogDistance;
	size_t Gain;
	size_t LensFlare;
	size_t LensFlareOffset;
	size_t LensFlareScale;
	size_t MasterGain;
	size_t MaxCarcasses;
	size_t MaxLightCount;
	size_t MaxLightingPolyCount;
	size_t MinLightCount;
	size_t MinLightingPolyCount;
	size_t NumCarcasses;
	size_t SkyZone;
	size_t SpeedOfSound;
	size_t TexUPanSpeed;
	size_t TexVPanSpeed;
	size_t ViewFlash;
	size_t ViewFog;
	size_t ZoneFluidFriction;
	size_t ZoneGravity;
	size_t ZoneGroundFriction;
	size_t ZoneName;
	size_t ZonePlayerCount;
	size_t ZonePlayerEvent;
	size_t ZoneTag;
	size_t ZoneTerminalVelocity;
	size_t ZoneVelocity;
	size_t bBounceVelocity;
	size_t bDestructive;
	size_t bFogZone;
	size_t bGravityZone;
	size_t bKillZone;
	size_t bMoveProjectiles;
	size_t bNeutralZone;
	size_t bNoInventory;
	size_t bPainZone;
	size_t bRaytraceReverb;
	size_t bReverbZone;
	size_t bWaterZone;
	size_t locationid;
};

extern PropertyOffsets_ZoneInfo PropOffsets_ZoneInfo;

struct PropertyOffsets_Canvas
{
	size_t BigFont;
	size_t ClipX;
	size_t ClipY;
	size_t CurX;
	size_t CurY;
	size_t CurYL;
	size_t DrawColor;
	size_t Font;
	size_t FramePtr;
	size_t LargeFont;
	size_t MedFont;
	size_t OrgX;
	size_t OrgY;
	size_t RenderPtr;
	size_t SizeX;
	size_t SizeY;
	size_t SmallFont;
	size_t SpaceX;
	size_t SpaceY;
	size_t Style;
	size_t Viewport;
	size_t Z;
	size_t bCenter;
	size_t bNoSmooth;
};

extern PropertyOffsets_Canvas PropOffsets_Canvas;

struct PropertyOffsets_SavedMove
{
	size_t Delta;
	size_t DodgeMove;
	size_t NextMove;
	size_t TimeStamp;
	size_t bAltFire;
	size_t bDuck;
	size_t bFire;
	size_t bForceAltFire;
	size_t bForceFire;
	size_t bPressedJump;
	size_t bRun;
};

extern PropertyOffsets_SavedMove PropOffsets_SavedMove;

struct PropertyOffsets_StatLog
{
	size_t Context;
	size_t DecoderRingURL;
	size_t GameCreator;
	size_t GameCreatorURL;
	size_t GameName;
	size_t LocalBatcherParams;
	size_t LocalBatcherURL;
	size_t LocalLogDir;
	size_t LocalStandard;
	size_t LocalStatsURL;
	size_t LogInfoURL;
	size_t LogVersion;
	size_t TimeStamp;
	size_t WorldBatcherParams;
	size_t WorldBatcherURL;
	size_t WorldLogDir;
	size_t WorldStandard;
	size_t WorldStatsURL;
	size_t bWorld;
	size_t bWorldBatcherError;
};

extern PropertyOffsets_StatLog PropOffsets_StatLog;

struct PropertyOffsets_Texture
{
	size_t Accumulator;
	size_t Alpha;
	size_t AnimCurrent;
	size_t AnimNext;
	size_t BumpMap;
	size_t CompFormat;
	size_t CompMips;
	size_t DetailTexture;
	size_t Diffuse;
	size_t DrawScale;
	size_t FootstepSound;
	size_t Friction;
	size_t HitSound;
	size_t LODSet;
	size_t MacroTexture;
	size_t MaxFrameRate;
	size_t MinFrameRate;
	size_t MipMult;
	size_t Mips;
	size_t PrimeCount;
	size_t PrimeCurrent;
	size_t Specular;
	size_t bAutoUPan;
	size_t bAutoVPan;
	size_t bBigWavy;
	size_t bCloudWavy;
	size_t bDirtyShadows;
	size_t bEnvironment;
	size_t bFakeBackdrop;
	size_t bGouraud;
	size_t bHasComp;
	size_t bHighColorQuality;
	size_t bHighLedge;
	size_t bHighShadowDetail;
	size_t bHighTextureQuality;
	size_t bInvisible;
	size_t bLowShadowDetail;
	size_t bMasked;
	size_t bMirrored;
	size_t bModulate;
	size_t bNoMerge;
	size_t bNoSmooth;
	size_t bNotSolid;
	size_t bParametric;
	size_t bPortal;
	size_t bRealtime;
	size_t bRealtimeChanged;
	size_t bSemisolid;
	size_t bSmallWavy;
	size_t bSpecialLit;
	size_t bTransparent;
	size_t bTwoSided;
	size_t bUnlit;
	size_t bWaterWavy;
	size_t bX2;
	size_t bX3;
	size_t bX4;
	size_t bX5;
	size_t bX6;
	size_t bX7;
};

extern PropertyOffsets_Texture PropOffsets_Texture;

struct PropertyOffsets_Ammo
{
	size_t AmmoAmount;
	size_t MaxAmmo;
	size_t PAmmo;
	size_t ParentAmmo;
	size_t UsedInWeaponSlot;
};

extern PropertyOffsets_Ammo PropOffsets_Ammo;

struct PropertyOffsets_NavigationPoint
{
	size_t ExtraCost;
	size_t Paths;
	size_t PrunedPaths;
	size_t RouteCache;
	size_t VisNoReachPaths;
	size_t bAutoBuilt;
	size_t bEndPoint;
	size_t bEndPointOnly;
	size_t bNeverUseStrafing;
	size_t bOneWayPath;
	size_t bPlayerOnly;
	size_t bSpecialCost;
	size_t bTwoWay;
	size_t bestPathWeight;
	size_t cost;
	size_t nextNavigationPoint;
	size_t nextOrdered;
	size_t ownerTeam;
	size_t prevOrdered;
	size_t previousPath;
	size_t startPath;
	size_t taken;
	size_t upstreamPaths;
	size_t visitedWeight;
};

extern PropertyOffsets_NavigationPoint PropOffsets_NavigationPoint;

struct PropertyOffsets_Mutator
{
	size_t DefaultWeapon;
	size_t NextDamageMutator;
	size_t NextHUDMutator;
	size_t NextMessageMutator;
	size_t NextMutator;
	size_t bHUDMutator;
};

extern PropertyOffsets_Mutator PropOffsets_Mutator;

struct PropertyOffsets_Mover
{
	size_t BasePos;
	size_t BaseRot;
	size_t BrushRaytraceKey;
	size_t BumpEvent;
	size_t BumpType;
	size_t ClientUpdate;
	size_t ClosedSound;
	size_t ClosingSound;
	size_t DamageThreshold;
	size_t DelayTime;
	size_t EncroachDamage;
	size_t Follower;
	size_t KeyNum;
	size_t KeyPos;
	size_t KeyRot;
	size_t Leader;
	size_t MoveAmbientSound;
	size_t MoveTime;
	size_t MoverEncroachType;
	size_t MoverGlideType;
	size_t NumKeys;
	size_t OldPos;
	size_t OldPrePivot;
	size_t OldRot;
	size_t OpenedSound;
	size_t OpeningSound;
	size_t OtherTime;
	size_t PlayerBumpEvent;
	size_t PrevKeyNum;
	size_t RealPosition;
	size_t RealRotation;
	size_t RecommendedTrigger;
	size_t ReturnGroup;
	size_t SavedPos;
	size_t SavedRot;
	size_t SavedTrigger;
	size_t SimInterpolate;
	size_t SimOldPos;
	size_t SimOldRotPitch;
	size_t SimOldRotRoll;
	size_t SimOldRotYaw;
	size_t StayOpenTime;
	size_t TriggerActor;
	size_t TriggerActor2;
	size_t WaitingPawn;
	size_t WorldRaytraceKey;
	size_t bClientPause;
	size_t bDamageTriggered;
	size_t bDelaying;
	size_t bDynamicLightMover;
	size_t bOpening;
	size_t bPlayerOnly;
	size_t bSlave;
	size_t bTriggerOnceOnly;
	size_t bUseTriggered;
	size_t myMarker;
	size_t numTriggerEvents;
};

extern PropertyOffsets_Mover PropOffsets_Mover;

struct PropertyOffsets_HUD
{
	size_t Crosshair;
	size_t HUDConfigWindowType;
	size_t HUDMutator;
	size_t HudMode;
	size_t MainMenu;
	size_t MainMenuType;
	size_t PlayerOwner;
	size_t WhiteColor;
};

extern PropertyOffsets_HUD PropOffsets_HUD;

struct PropertyOffsets_Decoration
{
	size_t EffectWhenDestroyed;
	size_t EndPushSound;
	size_t PushSound;
	size_t bBobbing;
	size_t bOnlyTriggerable;
	size_t bPushSoundPlaying;
	size_t bPushable;
	size_t bSplash;
	size_t bWasCarried;
	size_t content2;
	size_t content3;
	size_t contents;
	size_t numLandings;
};

extern PropertyOffsets_Decoration PropOffsets_Decoration;

struct PropertyOffsets_TestInfo
{
	size_t MyArray;
	size_t ST;
	size_t TestRepStr;
	size_t bBool1;
	size_t bBool2;
	size_t bFalse1;
	size_t bFalse2;
	size_t bTrue1;
	size_t bTrue2;
	size_t ppp;
	size_t sxx;
	size_t v1;
	size_t v2;
	size_t xnum;
};

extern PropertyOffsets_TestInfo PropOffsets_TestInfo;

struct PropertyOffsets_GameReplicationInfo
{
	size_t AdminEmail;
	size_t AdminName;
	size_t ElapsedTime;
	size_t GameClass;
	size_t GameEndedComments;
	size_t GameName;
	size_t MOTDLine1;
	size_t MOTDLine2;
	size_t MOTDLine3;
	size_t MOTDLine4;
	size_t NumPlayers;
	size_t PRIArray;
	size_t Region;
	size_t RemainingMinute;
	size_t RemainingTime;
	size_t SecondCount;
	size_t ServerName;
	size_t ShortName;
	size_t SumFrags;
	size_t UpdateTimer;
	size_t bClassicDeathMessages;
	size_t bStopCountDown;
	size_t bTeamGame;
};

extern PropertyOffsets_GameReplicationInfo PropOffsets_GameReplicationInfo;

struct PropertyOffsets_Menu
{
	size_t CenterString;
	size_t DisabledString;
	size_t EnabledString;
	size_t HelpMessage;
	size_t LeftString;
	size_t MenuLength;
	size_t MenuList;
	size_t MenuTitle;
	size_t NoString;
	size_t ParentMenu;
	size_t PlayerOwner;
	size_t RightString;
	size_t Selection;
	size_t YesString;
	size_t bConfigChanged;
	size_t bExitAllMenus;
};

extern PropertyOffsets_Menu PropOffsets_Menu;

struct PropertyOffsets_LiftExit
{
	size_t LastTriggerTime;
	size_t LiftTag;
	size_t LiftTrigger;
	size_t MyLift;
	size_t RecommendedTrigger;
};

extern PropertyOffsets_LiftExit PropOffsets_LiftExit;

struct PropertyOffsets_Trigger
{
	size_t ClassProximityType;
	size_t DamageThreshold;
	size_t Message;
	size_t ReTriggerDelay;
	size_t RepeatTriggerTime;
	size_t TriggerActor;
	size_t TriggerActor2;
	size_t TriggerTime;
	size_t TriggerType;
	size_t bInitiallyActive;
	size_t bTriggerOnceOnly;
};

extern PropertyOffsets_Trigger PropOffsets_Trigger;

struct PropertyOffsets_Player
{
	size_t Actor;
	size_t ConfiguredInternetSpeed;
	size_t ConfiguredLanSpeed;
	size_t Console;
	size_t CurrentNetSpeed;
	size_t SelectedCursor;
	size_t WindowsMouseX;
	size_t WindowsMouseY;
	size_t bShowWindowsMouse;
	size_t bSuspendPrecaching;
	size_t bWindowsMouseAvailable;
	size_t vfExec;
	size_t vfOut;
};

extern PropertyOffsets_Player PropOffsets_Player;

struct PropertyOffsets_LocalMessage
{
	size_t ChildMessage;
	size_t DrawColor;
	size_t Lifetime;
	size_t XPos;
	size_t YPos;
	size_t bBeep;
	size_t bCenter;
	size_t bComplexString;
	size_t bFadeMessage;
	size_t bFromBottom;
	size_t bIsConsoleMessage;
	size_t bIsSpecial;
	size_t bIsUnique;
	size_t bOffsetYPos;
};

extern PropertyOffsets_LocalMessage PropOffsets_LocalMessage;

struct PropertyOffsets_locationid
{
	size_t LocationName;
	size_t NextLocation;
	size_t Radius;
};

extern PropertyOffsets_locationid PropOffsets_locationid;

struct PropertyOffsets_Carcass
{
	size_t Bugs;
	size_t CumulativeDamage;
	size_t PlayerOwner;
	size_t bDecorative;
	size_t bPlayerCarcass;
	size_t bReducedHeight;
	size_t bSlidingCarcass;
	size_t flies;
	size_t rats;
};

extern PropertyOffsets_Carcass PropOffsets_Carcass;

struct PropertyOffsets_InterpolationPoint
{
	size_t FovModifier;
	size_t GameSpeedModifier;
	size_t Next;
	size_t Position;
	size_t Prev;
	size_t RateModifier;
	size_t ScreenFlashFog;
	size_t ScreenFlashScale;
	size_t bEndOfPath;
	size_t bSkipNextPath;
};

extern PropertyOffsets_InterpolationPoint PropOffsets_InterpolationPoint;

struct PropertyOffsets_Projectile
{
	size_t Damage;
	size_t ExploWallOut;
	size_t ExplosionDecal;
	size_t ImpactSound;
	size_t MaxSpeed;
	size_t MiscSound;
	size_t MomentumTransfer;
	size_t MyDamageType;
	size_t SpawnSound;
	size_t speed;
};

extern PropertyOffsets_Projectile PropOffsets_Projectile;

struct PropertyOffsets_Teleporter
{
	size_t LastFired;
	size_t ProductRequired;
	size_t TargetVelocity;
	size_t TriggerActor;
	size_t TriggerActor2;
	size_t URL;
	size_t bChangesVelocity;
	size_t bChangesYaw;
	size_t bEnabled;
	size_t bReversesX;
	size_t bReversesY;
	size_t bReversesZ;
};

extern PropertyOffsets_Teleporter PropOffsets_Teleporter;

struct PropertyOffsets_Palette
{
	size_t Colors;
};

extern PropertyOffsets_Palette PropOffsets_Palette;

struct PropertyOffsets_SpawnNotify
{
	size_t ActorClass;
	size_t Next;
};

extern PropertyOffsets_SpawnNotify PropOffsets_SpawnNotify;

struct PropertyOffsets_Fragment
{
	size_t Fragments;
	size_t bFirstHit;
	size_t numFragmentTypes;
};

extern PropertyOffsets_Fragment PropOffsets_Fragment;

struct PropertyOffsets_WarpZoneInfo
{
	size_t Destinations;
	size_t OtherSideActor;
	size_t OtherSideLevel;
	size_t OtherSideURL;
	size_t ThisTag;
	size_t WarpCoords;
	size_t bNoTeleFrag;
	size_t iWarpZone;
	size_t numDestinations;
};

extern PropertyOffsets_WarpZoneInfo PropOffsets_WarpZoneInfo;

struct PropertyOffsets_Console
{
	size_t AvgText;
	size_t Border;
	size_t BorderLines;
	size_t BorderPixels;
	size_t BorderSize;
	size_t ConBackground;
	size_t ConnectingMessage;
	size_t ConsoleDest;
	size_t ConsoleLines;
	size_t ConsolePos;
	size_t ExtraTime;
	size_t FrameCount;
	size_t FrameRateText;
	size_t FrameX;
	size_t FrameY;
	size_t FramesText;
	size_t History;
	size_t HistoryBot;
	size_t HistoryCur;
	size_t HistoryTop;
	size_t LastFrameTime;
	size_t LastSecFPS;
	size_t LastSecText;
	size_t LastSecondFrameCount;
	size_t LastSecondStartTime;
	size_t LoadingMessage;
	size_t MaxFPS;
	size_t MaxText;
	size_t MinFPS;
	size_t MinText;
	size_t MsgPlayer;
	size_t MsgText;
	size_t MsgTick;
	size_t MsgTickTime;
	size_t MsgTime;
	size_t MsgType;
	size_t PausedMessage;
	size_t PrecachingMessage;
	size_t SavingMessage;
	size_t Scrollback;
	size_t SecondsText;
	size_t StartTime;
	size_t TextLines;
	size_t TimeDemoFont;
	size_t TopLine;
	size_t TypedStr;
	size_t Viewport;
	size_t bNoDrawWorld;
	size_t bNoStuff;
	size_t bRestartTimeDemo;
	size_t bSaveTimeDemoToFile;
	size_t bStartTimeDemo;
	size_t bTimeDemo;
	size_t bTyping;
	size_t fpsText;
	size_t numLines;
	size_t vtblOut;
};

extern PropertyOffsets_Console PropOffsets_Console;

struct PropertyOffsets_PlayerStart
{
	size_t TeamNumber;
	size_t bCoopStart;
	size_t bEnabled;
	size_t bSinglePlayerStart;
};

extern PropertyOffsets_PlayerStart PropOffsets_PlayerStart;

struct PropertyOffsets_Pickup
{
	size_t ExpireMessage;
	size_t Inv;
	size_t NumCopies;
	size_t bAutoActivate;
	size_t bCanActivate;
	size_t bCanHaveMultipleCopies;
};

extern PropertyOffsets_Pickup PropOffsets_Pickup;

struct PropertyOffsets_Brush
{
	size_t BrushColor;
	size_t CsgOper;
	size_t MainScale;
	size_t PolyFlags;
	size_t PostPivot;
	size_t PostScale;
	size_t TempScale;
	size_t UnusedLightMesh;
	size_t bColored;
};

extern PropertyOffsets_Brush PropOffsets_Brush;

struct PropertyOffsets_ScoreBoard
{
	size_t OwnerHUD;
	size_t RegFont;
};

extern PropertyOffsets_ScoreBoard PropOffsets_ScoreBoard;

struct PropertyOffsets_Spectator
{
	size_t bChaseCam;
};

extern PropertyOffsets_Spectator PropOffsets_Spectator;

struct PropertyOffsets_InventorySpot
{
	size_t markedItem;
};

extern PropertyOffsets_InventorySpot PropOffsets_InventorySpot;

struct PropertyOffsets_Decal
{
	size_t LastRenderedTime;
	size_t MultiDecalLevel;
	size_t SurfList;
};

extern PropertyOffsets_Decal PropOffsets_Decal;

struct PropertyOffsets_PatrolPoint
{
	size_t AnimCount;
	size_t NextPatrolPoint;
	size_t Nextpatrol;
	size_t PatrolAnim;
	size_t PatrolSound;
	size_t lookDir;
	size_t numAnims;
	size_t pausetime;
};

extern PropertyOffsets_PatrolPoint PropOffsets_PatrolPoint;

struct PropertyOffsets_Counter
{
	size_t CompleteMessage;
	size_t CountMessage;
	size_t NumToCount;
	size_t OriginalNum;
	size_t bShowMessage;
};

extern PropertyOffsets_Counter PropOffsets_Counter;

struct PropertyOffsets_Bitmap
{
	size_t Format;
	size_t InternalTime;
	size_t MaxColor;
	size_t MipZero;
	size_t Palette;
	size_t UBits;
	size_t UClamp;
	size_t USize;
	size_t VBits;
	size_t VClamp;
	size_t VSize;
};

extern PropertyOffsets_Bitmap PropOffsets_Bitmap;

struct PropertyOffsets_MapList
{
	size_t MapNum;
	size_t Maps;
};

extern PropertyOffsets_MapList PropOffsets_MapList;

struct PropertyOffsets_Effects
{
	size_t EffectSound1;
	size_t EffectSound2;
	size_t bOnlyTriggerable;
};

extern PropertyOffsets_Effects PropOffsets_Effects;

struct PropertyOffsets_StatLogFile
{
	size_t LogAr;
	size_t StatLogFile;
	size_t StatLogFinal;
	size_t bWatermark;
};

extern PropertyOffsets_StatLogFile PropOffsets_StatLogFile;

struct PropertyOffsets_LevelSummary
{
	size_t Author;
	size_t IdealPlayerCount;
	size_t LevelEnterText;
	size_t RecommendedEnemies;
	size_t RecommendedTeammates;
	size_t Title;
};

extern PropertyOffsets_LevelSummary PropOffsets_LevelSummary;

struct PropertyOffsets_ScriptedTexture
{
	size_t Junk1;
	size_t Junk2;
	size_t Junk3;
	size_t LocalTime;
	size_t NotifyActor;
	size_t SourceTexture;
};

extern PropertyOffsets_ScriptedTexture PropOffsets_ScriptedTexture;

struct PropertyOffsets_Engine
{
	size_t Audio;
	size_t AudioDevice;
	size_t CacheSizeMegs;
	size_t Client;
	size_t ClientCycles;
	size_t Console;
	size_t CurrentTickRate;
	size_t Cylinder;
	size_t GameCycles;
	size_t GameRenderDevice;
	size_t Language;
	size_t NetworkDevice;
	size_t Render;
	size_t TickCycles;
	size_t UseSound;
};

extern PropertyOffsets_Engine PropOffsets_Engine;

struct PropertyOffsets_TriggerLight
{
	size_t Alpha;
	size_t ChangeTime;
	size_t Direction;
	size_t InitialBrightness;
	size_t RemainOnTime;
	size_t SavedTrigger;
	size_t bDelayFullOn;
	size_t bInitiallyOn;
	size_t poundTime;
};

extern PropertyOffsets_TriggerLight PropOffsets_TriggerLight;

struct PropertyOffsets_SpecialEvent
{
	size_t Damage;
	size_t DamageString;
	size_t DamageType;
	size_t Message;
	size_t Sound;
	size_t bBroadcast;
	size_t bPlayerViewRot;
};

extern PropertyOffsets_SpecialEvent PropOffsets_SpecialEvent;

struct PropertyOffsets_RoundRobin
{
	size_t OutEvents;
	size_t bLoop;
	size_t i;
};

extern PropertyOffsets_RoundRobin PropOffsets_RoundRobin;

struct PropertyOffsets_MusicEvent
{
	size_t CdTrack;
	size_t Song;
	size_t SongSection;
	size_t Transition;
	size_t bAffectAllPlayers;
	size_t bOnceOnly;
	size_t bSilence;
};

extern PropertyOffsets_MusicEvent PropOffsets_MusicEvent;

struct PropertyOffsets_HomeBase
{
	size_t Extent;
	size_t lookDir;
};

extern PropertyOffsets_HomeBase PropOffsets_HomeBase;

struct PropertyOffsets_Dispatcher
{
	size_t OutDelays;
	size_t OutEvents;
	size_t i;
};

extern PropertyOffsets_Dispatcher PropOffsets_Dispatcher;

struct PropertyOffsets_DemoRecSpectator
{
	size_t PlaybackActor;
	size_t PlaybackGRI;
};

extern PropertyOffsets_DemoRecSpectator PropOffsets_DemoRecSpectator;

struct PropertyOffsets_DamageType
{
	size_t AltName;
	size_t DamageEffect;
	size_t Name;
	size_t ViewFlash;
	size_t ViewFog;
};

extern PropertyOffsets_DamageType PropOffsets_DamageType;

struct PropertyOffsets_Ambushpoint
{
	size_t SightRadius;
	size_t bSniping;
	size_t lookDir;
	size_t survivecount;
};

extern PropertyOffsets_Ambushpoint PropOffsets_Ambushpoint;

struct PropertyOffsets_WarpZoneMarker
{
	size_t TriggerActor;
	size_t TriggerActor2;
	size_t markedWarpZone;
};

extern PropertyOffsets_WarpZoneMarker PropOffsets_WarpZoneMarker;

struct PropertyOffsets_LiftCenter
{
	size_t LastTriggerTime;
	size_t LiftOffset;
	size_t LiftTag;
	size_t LiftTrigger;
	size_t MaxDist2D;
	size_t MaxZDiffAdd;
	size_t MyLift;
	size_t RecommendedTrigger;
};

extern PropertyOffsets_LiftCenter PropOffsets_LiftCenter;

struct PropertyOffsets_RenderIterator
{
	size_t Frame;
	size_t Index;
	size_t MaxItems;
	size_t Observer;
};

extern PropertyOffsets_RenderIterator PropOffsets_RenderIterator;
