
#include "Precomp.h"
#include "NPawn.h"
#include "VM/NativeFunc.h"

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
	throw std::runtime_error("Pawn.AddPawn not implemented");
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
	throw std::runtime_error("Pawn.LineOfSightTo not implemented");
}

void NPawn::MoveTo(UObject* Self, const vec3& NewDestination, float* speed)
{
	throw std::runtime_error("Pawn.MoveTo not implemented");
}

void NPawn::MoveToward(UObject* Self, UObject* NewTarget, float* speed)
{
	throw std::runtime_error("Pawn.MoveToward not implemented");
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
	throw std::runtime_error("Pawn.RemovePawn not implemented");
}

void NPawn::StopWaiting(UObject* Self)
{
	throw std::runtime_error("Pawn.StopWaiting not implemented");
}

void NPawn::StrafeFacing(UObject* Self, const vec3& NewDestination, UObject* NewTarget)
{
	throw std::runtime_error("Pawn.StrafeFacing not implemented");
}

void NPawn::StrafeTo(UObject* Self, const vec3& NewDestination, const vec3& NewFocus)
{
	throw std::runtime_error("Pawn.StrafeTo not implemented");
}

void NPawn::TurnTo(UObject* Self, const vec3& NewFocus)
{
	throw std::runtime_error("Pawn.TurnTo not implemented");
}

void NPawn::TurnToward(UObject* Self, UObject* NewTarget)
{
	throw std::runtime_error("Pawn.TurnToward not implemented");
}

void NPawn::WaitForLanding(UObject* Self)
{
	throw std::runtime_error("Pawn.WaitForLanding not implemented");
}

void NPawn::actorReachable(UObject* Self, UObject* anActor, bool& ReturnValue)
{
	throw std::runtime_error("Pawn.actorReachable not implemented");
}

void NPawn::pointReachable(UObject* Self, const vec3& aPoint, bool& ReturnValue)
{
	throw std::runtime_error("Pawn.pointReachable not implemented");
}
