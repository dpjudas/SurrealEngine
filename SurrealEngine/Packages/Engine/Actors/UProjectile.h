#pragma once

#include "UActor.h"

class USound;

class UProjectile : public UActor
{
public:
	using UActor::UActor;

	float& Damage() { return Value<float>(PropOffsets_Projectile.Damage); }
	float& ExploWallOut() { return Value<float>(PropOffsets_Projectile.ExploWallOut); }
	UClass*& ExplosionDecal() { return Value<UClass*>(PropOffsets_Projectile.ExplosionDecal); }
	USound*& ImpactSound() { return Value<USound*>(PropOffsets_Projectile.ImpactSound); }
	float& MaxSpeed() { return Value<float>(PropOffsets_Projectile.MaxSpeed); }
	USound*& MiscSound() { return Value<USound*>(PropOffsets_Projectile.MiscSound); }
	int& MomentumTransfer() { return Value<int>(PropOffsets_Projectile.MomentumTransfer); }
	NameString& MyDamageType() { return Value<NameString>(PropOffsets_Projectile.MyDamageType); }
	USound*& SpawnSound() { return Value<USound*>(PropOffsets_Projectile.SpawnSound); }
	float& speed() { return Value<float>(PropOffsets_Projectile.speed); }
};
