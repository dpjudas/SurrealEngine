
#include "Precomp.h"
#include "NPawn.h"
#include "VM/NativeFunc.h"
#include "VM/Frame.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "Engine.h"

void NPawn::RegisterFunctions()
{
	RegisterVMNativeFunc_0("Pawn", "AddPawn", &NPawn::AddPawn, 529);
	RegisterVMNativeFunc_2("Pawn", "CanSee", &NPawn::CanSee, 533);
	RegisterVMNativeFunc_3("Pawn", "CheckValidSkinPackage", &NPawn::CheckValidSkinPackage, 0);
	RegisterVMNativeFunc_0("Pawn", "ClearPaths", &NPawn::ClearPaths, 522);
	RegisterVMNativeFunc_5("Pawn", "ClientHearSound", &NPawn::ClientHearSound, 0);
	RegisterVMNativeFunc_1("Pawn", "EAdjustJump", &NPawn::EAdjustJump, 523);
	RegisterVMNativeFunc_3("Pawn", "FindBestInventoryPath", &NPawn::FindBestInventoryPath, 540);
	RegisterVMNativeFunc_4("Pawn", "FindPathTo", &NPawn::FindPathTo, 518);
	RegisterVMNativeFunc_4("Pawn", "FindPathToward", &NPawn::FindPathToward, 517);
	RegisterVMNativeFunc_2("Pawn", "FindRandomDest", &NPawn::FindRandomDest, 525);
	RegisterVMNativeFunc_2("Pawn", "FindStairRotation", &NPawn::FindStairRotation, 524);
	RegisterVMNativeFunc_2("Pawn", "LineOfSightTo", &NPawn::LineOfSightTo, 514);
	RegisterVMNativeFunc_2("Pawn", "MoveTo", &NPawn::MoveTo, 500);
	RegisterVMNativeFunc_2("Pawn", "MoveToward", &NPawn::MoveToward, 502);
	RegisterVMNativeFunc_5("Pawn", "PickAnyTarget", &NPawn::PickAnyTarget, 534);
	RegisterVMNativeFunc_5("Pawn", "PickTarget", &NPawn::PickTarget, 531);
	RegisterVMNativeFunc_1("Pawn", "PickWallAdjust", &NPawn::PickWallAdjust, 526);
	RegisterVMNativeFunc_0("Pawn", "RemovePawn", &NPawn::RemovePawn, 530);
	RegisterVMNativeFunc_0("Pawn", "StopWaiting", &NPawn::StopWaiting, 0);
	RegisterVMNativeFunc_2("Pawn", "StrafeFacing", &NPawn::StrafeFacing, 506);
	RegisterVMNativeFunc_2("Pawn", "StrafeTo", &NPawn::StrafeTo, 504);
	RegisterVMNativeFunc_1("Pawn", "TurnTo", &NPawn::TurnTo, 508);
	RegisterVMNativeFunc_1("Pawn", "TurnToward", &NPawn::TurnToward, 510);
	RegisterVMNativeFunc_0("Pawn", "WaitForLanding", &NPawn::WaitForLanding, 527);
	RegisterVMNativeFunc_2("Pawn", "actorReachable", &NPawn::actorReachable, 520);
	RegisterVMNativeFunc_2("Pawn", "pointReachable", &NPawn::pointReachable, 521);
}

void NPawn::AddPawn(UObject* Self)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	SelfPawn->nextPawn() = SelfPawn->Level()->PawnList();
	SelfPawn->Level()->PawnList() = SelfPawn;
}

void NPawn::CanSee(UObject* Self, UObject* Other, BitfieldBool& ReturnValue)
{
	UPawn* selfPawn = UObject::Cast<UPawn>(Self);
	UActor* otherActor = UObject::Cast<UActor>(Other);
	ReturnValue = selfPawn->CanSee(otherActor);
}

void NPawn::CheckValidSkinPackage(const std::string& SkinPack, const std::string& MeshName, BitfieldBool& ReturnValue)
{
	Exception::Throw("Pawn.CheckValidSkinPackage not implemented");
}

void NPawn::ClearPaths(UObject* Self)
{
	UPawn* selfPawn = UObject::Cast<UPawn>(Self);
	selfPawn->ClearPaths();
}

void NPawn::ClientHearSound(UObject* Self, UObject* Actor, int Id, UObject* S, const vec3& SoundLocation, const vec3& Parameters)
{
	LogUnimplemented("Pawn.ClientHearSound");
}

void NPawn::EAdjustJump(UObject* Self, vec3& ReturnValue)
{
	LogUnimplemented("Pawn.EAdjustJump");
}

void NPawn::FindBestInventoryPath(UObject* Self, float& MinWeight, bool bPredictRespawns, UObject*& ReturnValue)
{
	UPawn* selfPawn = UObject::Cast<UPawn>(Self);
	ReturnValue = selfPawn->FindBestInventoryPath(bPredictRespawns, MinWeight);
}

void NPawn::FindPathTo(UObject* Self, const vec3& aPoint, BitfieldBool* bSinglePath, BitfieldBool* bClearPaths, UObject*& ReturnValue)
{
	UPawn* selfPawn = UObject::Cast<UPawn>(Self);
	if (!bClearPaths || *bClearPaths)
		selfPawn->ClearPaths();
	ReturnValue = selfPawn->FindPathTo(aPoint, bSinglePath ? *bSinglePath : false);
}

void NPawn::FindPathToward(UObject* Self, UObject* anActor, BitfieldBool* bSinglePath, BitfieldBool* bClearPaths, UObject*& ReturnValue)
{
	UPawn* selfPawn = UObject::Cast<UPawn>(Self);
	if (!bClearPaths || *bClearPaths)
		selfPawn->ClearPaths();
	ReturnValue = selfPawn->FindPathToward(anActor, bSinglePath ? *bSinglePath : false);
}

void NPawn::FindRandomDest(UObject* Self, BitfieldBool* bClearPaths, UObject*& ReturnValue)
{
	UPawn* selfPawn = UObject::Cast<UPawn>(Self);
	if (!bClearPaths || *bClearPaths)
		selfPawn->ClearPaths();
	ReturnValue = selfPawn->FindRandomDest();
}

void NPawn::FindStairRotation(UObject* Self, float DeltaTime, int& ReturnValue)
{
	LogUnimplemented("Pawn.FindStairRotation");
	ReturnValue = 0;
}

void NPawn::LineOfSightTo(UObject* Self, UObject* Other, BitfieldBool& ReturnValue)
{
	UPawn* selfPawn = UObject::Cast<UPawn>(Self);
	UActor* otherActor = UObject::Cast<UActor>(Other);
	ReturnValue = selfPawn->LineOfSightTo(otherActor);
}

void NPawn::MoveTo(UObject* Self, const vec3& NewDestination, float* speed)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	SelfPawn->MoveTo(NewDestination, speed ? *speed : 1.0f);
}

void NPawn::MoveToward(UObject* Self, UObject* NewTarget, float* speed)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	SelfPawn->MoveToward(UObject::Cast<UActor>(NewTarget), speed ? *speed : 1.0f);
}

void NPawn::PickAnyTarget(UObject* Self, float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart, UObject*& ReturnValue)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	ReturnValue = SelfPawn->PickAnyTarget(bestAim, bestDist, FireDir, projStart);
}

void NPawn::PickTarget(UObject* Self, float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart, UObject*& ReturnValue)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	ReturnValue = SelfPawn->PickTarget(bestAim, bestDist, FireDir, projStart);
}

void NPawn::PickWallAdjust(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Pawn.PickWallAdjust");
	ReturnValue = false;
}

void NPawn::RemovePawn(UObject* Self)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);

	if (SelfPawn->Level()->PawnList() == SelfPawn)
	{
		SelfPawn->Level()->PawnList() = SelfPawn->nextPawn();
		SelfPawn->nextPawn() = nullptr;
	}
	else
	{
		UPawn* prevPawn = nullptr;
		for (UPawn* cur = SelfPawn->Level()->PawnList(); cur != nullptr; cur = cur->nextPawn())
		{
			if (cur->nextPawn() == SelfPawn)
			{
				cur->nextPawn() = SelfPawn->nextPawn();
				SelfPawn->nextPawn() = nullptr;
				break;
			}
		}
	}
}

void NPawn::StopWaiting(UObject* Self)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	SelfPawn->SleepTimeLeft = 0.0f;
}

void NPawn::StrafeFacing(UObject* Self, const vec3& NewDestination, UObject* NewTarget)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	SelfPawn->StrafeFacing(NewDestination, UObject::Cast<UActor>(NewTarget));
}

void NPawn::StrafeTo(UObject* Self, const vec3& NewDestination, const vec3& NewFocus)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	SelfPawn->StrafeTo(NewDestination, NewFocus);
}

void NPawn::TurnTo(UObject* Self, const vec3& NewFocus)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	SelfPawn->TurnTo(NewFocus);
}

void NPawn::TurnToward(UObject* Self, UObject* NewTarget)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	SelfPawn->TurnToward(UObject::Cast<UActor>(NewTarget));
}

void NPawn::WaitForLanding(UObject* Self)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	SelfPawn->WaitForLanding();
}

void NPawn::actorReachable(UObject* Self, UObject* anActor, BitfieldBool& ReturnValue)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	ReturnValue = SelfPawn->ActorReachable(UObject::Cast<UActor>(anActor));
}

void NPawn::pointReachable(UObject* Self, const vec3& aPoint, BitfieldBool& ReturnValue)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	ReturnValue = SelfPawn->PointReachable(aPoint);
}
