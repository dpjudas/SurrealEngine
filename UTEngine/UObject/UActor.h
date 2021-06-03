#pragma once

#include "UObject.h"

class UTexture;
class UMesh;
class UModel;

enum class ActorDrawType
{
	None,
	Sprite,
	Mesh,
	Brush,
	RopeSprite,
	VerticalSprite,
	Terraform,
	SpriteAnimOnce
};

class UActor : public UObject
{
public:
	using UObject::UObject;

	void CopyProperties();

	vec3 Location = { 0.0f };
	Rotator Rotation = { 0, 0, 0 };
	Rotator RotationRate = { 0, 0, 0 };
	float DrawScale = 1.0f;

	int LightBrightness = 64;
	int LightHue = 0;
	int LightSaturation = 255;
	int LightRadius = 64;
	bool bCorona = false;

	bool bHidden = false;
	ActorDrawType DrawType = ActorDrawType::None;
	UTexture* Skin = nullptr;
	UTexture* Texture = nullptr;
	UTexture* Sprite = nullptr;
	UMesh* Mesh = nullptr;
	UModel* Brush = nullptr;
};

class ULight : public UActor { using UActor::UActor; };
class UDecal : public UActor { using UActor::UActor; };
class USpawnNotify : public UActor { using UActor::UActor; };

class UInventory : public UActor { using UActor::UActor; };
class UWeapon : public UInventory { using UInventory::UInventory; };

class UNavigationPoint : public UActor { using UActor::UActor; };
class ULiftExit : public UNavigationPoint { using UNavigationPoint::UNavigationPoint; };
class ULiftCenter : public UNavigationPoint { using UNavigationPoint::UNavigationPoint; };
class UWarpZoneMarker : public UNavigationPoint { using UNavigationPoint::UNavigationPoint; };
class UInventorySpot : public UNavigationPoint { using UNavigationPoint::UNavigationPoint; };
class UTriggerMarker : public UNavigationPoint { using UNavigationPoint::UNavigationPoint; };
class UButtonMarker : public UNavigationPoint { using UNavigationPoint::UNavigationPoint; };
class UPlayerStart : public UNavigationPoint { using UNavigationPoint::UNavigationPoint; };
class UTeleporter : public UNavigationPoint { using UNavigationPoint::UNavigationPoint; };
class UPathNode : public UNavigationPoint { using UNavigationPoint::UNavigationPoint; };

class UDecoration : public UActor { using UActor::UActor; };
class UCarcass : public UDecoration { using UDecoration::UDecoration; };

class UProjectile : public UActor { using UActor::UActor; };
class UKeypoint : public UActor { using UActor::UActor; };

class Ulocationid : public UKeypoint { using UKeypoint::UKeypoint; };
class UInterpolationPoint : public UKeypoint { using UKeypoint::UKeypoint; };

class UTriggers : public UActor { using UActor::UActor; };
class UTrigger : public UTriggers { using UTriggers::UTriggers; };

class UHUD : public UActor { using UActor::UActor; };
class UMenu : public UActor { using UActor::UActor; };
class UInfo : public UActor { using UActor::UActor; };
class UMutator : public UInfo { using UInfo::UInfo; };
class UGameInfo : public UInfo { using UInfo::UInfo; };
class USavedMove : public UInfo { using UInfo::UInfo; };
class UInternetInfo : public UInfo { using UInfo::UInfo; };

class UZoneInfo : public UInfo { using UInfo::UInfo; };
class ULevelInfo : public UZoneInfo { using UZoneInfo::UZoneInfo; };
class UWarpZoneInfo : public UZoneInfo { using UZoneInfo::UZoneInfo; };
class USkyZoneInfo : public UZoneInfo { using UZoneInfo::UZoneInfo; };

class UReplicationInfo : public UInfo { using UInfo::UInfo; };
class UPlayerReplicationInfo : public UReplicationInfo { using UReplicationInfo::UReplicationInfo; };
class UGameReplicationInfo : public UReplicationInfo { using UReplicationInfo::UReplicationInfo; };

class UStatLog : public UInfo { using UInfo::UInfo; };
class UStatLogFile : public UStatLog { using UStatLog::UStatLog; };

class UBrush : public UActor { using UActor::UActor; };
class UMover : public UBrush { using UBrush::UBrush; };

class UPawn : public UActor { using UActor::UActor; };
class UScout : public UPawn { using UPawn::UPawn; };
class UPlayerPawn : public UPawn { using UPawn::UPawn; };
class UCamera : public UPlayerPawn { using UPlayerPawn::UPlayerPawn; };
