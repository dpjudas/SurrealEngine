
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

void NPawn::CanSee(UObject* Self, UObject* Other, bool& ReturnValue)
{
	throw std::runtime_error("Pawn.CanSee not implemented");
}

void NPawn::CheckValidSkinPackage(const std::string& SkinPack, const std::string& MeshName, bool& ReturnValue)
{
	throw std::runtime_error("Pawn.CheckValidSkinPackage not implemented");
}

void NPawn::ClearPaths(UObject* Self)
{
	throw std::runtime_error("Pawn.ClearPaths not implemented");
}

void NPawn::ClientHearSound(UObject* Self, UObject* Actor, int Id, UObject* S, const vec3& SoundLocation, const vec3& Parameters)
{
	throw std::runtime_error("Pawn.ClientHearSound not implemented");
}

void NPawn::EAdjustJump(UObject* Self, vec3& ReturnValue)
{
	throw std::runtime_error("Pawn.EAdjustJump not implemented");
}

void NPawn::FindBestInventoryPath(UObject* Self, float& MinWeight, bool bPredictRespawns, UObject*& ReturnValue)
{
	throw std::runtime_error("Pawn.FindBestInventoryPath not implemented");
}

void NPawn::FindPathTo(UObject* Self, const vec3& aPoint, bool* bSinglePath, bool* bClearPaths, UObject*& ReturnValue)
{
	throw std::runtime_error("Pawn.FindPathTo not implemented");
}

void NPawn::FindPathToward(UObject* Self, UObject* anActor, bool* bSinglePath, bool* bClearPaths, UObject*& ReturnValue)
{
	throw std::runtime_error("Pawn.FindPathToward not implemented");
}

void NPawn::FindRandomDest(UObject* Self, bool* bClearPaths, UObject*& ReturnValue)
{
	throw std::runtime_error("Pawn.FindRandomDest not implemented");
}

void NPawn::FindStairRotation(UObject* Self, float DeltaTime, int& ReturnValue)
{
	throw std::runtime_error("Pawn.FindStairRotation not implemented");
}

void NPawn::LineOfSightTo(UObject* Self, UObject* Other, bool& ReturnValue)
{
	engine->LogUnimplemented("Pawn.LineOfSightTo(" + UObject::GetUClassName(Other) + ")");
	ReturnValue = false;
}

void NPawn::MoveTo(UObject* Self, const vec3& NewDestination, float* speed)
{
	engine->LogUnimplemented("MoveTo");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::MoveTo;
}

void NPawn::MoveToward(UObject* Self, UObject* NewTarget, float* speed)
{
	engine->LogUnimplemented("MoveToward");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::MoveToward;
}

void NPawn::PickAnyTarget(UObject* Self, float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart, UObject*& ReturnValue)
{
	throw std::runtime_error("Pawn.PickAnyTarget not implemented");
}

void NPawn::PickTarget(UObject* Self, float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart, UObject*& ReturnValue)
{
	throw std::runtime_error("Pawn.PickTarget not implemented");
}

void NPawn::PickWallAdjust(UObject* Self, bool& ReturnValue)
{
	throw std::runtime_error("Pawn.PickWallAdjust not implemented");
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
	throw std::runtime_error("Pawn.StopWaiting not implemented");
}

void NPawn::StrafeFacing(UObject* Self, const vec3& NewDestination, UObject* NewTarget)
{
	engine->LogUnimplemented("StrafeFacing");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::StrafeFacing;
}

void NPawn::StrafeTo(UObject* Self, const vec3& NewDestination, const vec3& NewFocus)
{
	engine->LogUnimplemented("StrafeTo");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::StrafeTo;
}

void NPawn::TurnTo(UObject* Self, const vec3& NewFocus)
{
	engine->LogUnimplemented("TurnTo");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::TurnTo;
}

void NPawn::TurnToward(UObject* Self, UObject* NewTarget)
{
	engine->LogUnimplemented("TurnToward");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::TurnToward;
}

void NPawn::WaitForLanding(UObject* Self)
{
	engine->LogUnimplemented("WaitForLanding");
	if (Self->StateFrame)
		Self->StateFrame->LatentState = LatentRunState::WaitForLanding;
}

void NPawn::actorReachable(UObject* Self, UObject* anActor, bool& ReturnValue)
{
	throw std::runtime_error("Pawn.actorReachable not implemented");
}

void NPawn::pointReachable(UObject* Self, const vec3& aPoint, bool& ReturnValue)
{
	throw std::runtime_error("Pawn.pointReachable not implemented");
}
