
#include "Precomp.h"
#include "NPawn.h"
#include "VM/NativeFunc.h"
#include "VM/Frame.h"
#include "UObject/UActor.h"
#include "UObject/ULevel.h"
#include "UObject/USound.h"
#include "Engine.h"

void NPawn::RegisterFunctions()
{
	RegisterVMNativeFunc_0("Pawn", "AddPawn", &NPawn::AddPawn, 529);
	RegisterVMNativeFunc_9("Pawn", "AIPickRandomDestination", &NPawn::AIPickRandomDestination, 709);
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
	if (!engine->LaunchInfo.IsDeusEx())
		RegisterVMNativeFunc_2("Pawn", "LineOfSightTo", &NPawn::LineOfSightTo, 514);
	else
		RegisterVMNativeFunc_3("Pawn", "LineOfSightTo", &NPawn::LineOfSightTo_Deus, 514);
	RegisterVMNativeFunc_2("Pawn", "MoveTo", &NPawn::MoveTo, 500);
	RegisterLatentAction(501, LatentRunState::MoveTo);
	RegisterVMNativeFunc_2("Pawn", "MoveToward", &NPawn::MoveToward, 502);
	RegisterLatentAction(502, LatentRunState::MoveToward);
	RegisterVMNativeFunc_5("Pawn", "PickAnyTarget", &NPawn::PickAnyTarget, 534);
	RegisterVMNativeFunc_5("Pawn", "PickTarget", &NPawn::PickTarget, 531);
	RegisterVMNativeFunc_1("Pawn", "PickWallAdjust", &NPawn::PickWallAdjust, 526);
	RegisterVMNativeFunc_0("Pawn", "RemovePawn", &NPawn::RemovePawn, 530);
	RegisterVMNativeFunc_0("Pawn", "StopWaiting", &NPawn::StopWaiting, 0);
	if (!engine->LaunchInfo.IsDeusEx())
		RegisterVMNativeFunc_2("Pawn", "StrafeFacing", &NPawn::StrafeFacing, 506);
	else
		RegisterVMNativeFunc_3("Pawn", "StrafeFacing", &NPawn::StrafeFacing_Deus, 506);
	RegisterLatentAction(507, LatentRunState::StrafeFacing);
	if (!engine->LaunchInfo.IsDeusEx())
		RegisterVMNativeFunc_2("Pawn", "StrafeTo", &NPawn::StrafeTo, 504);
	else
		RegisterVMNativeFunc_3("Pawn", "StrafeTo", &NPawn::StrafeTo_Deus, 504);
	RegisterLatentAction(505, LatentRunState::StrafeTo);
	RegisterVMNativeFunc_1("Pawn", "TurnTo", &NPawn::TurnTo, 508);
	RegisterLatentAction(509, LatentRunState::TurnTo);
	RegisterVMNativeFunc_1("Pawn", "TurnToward", &NPawn::TurnToward, 510);
	RegisterLatentAction(511, LatentRunState::TurnToward);
	RegisterVMNativeFunc_0("Pawn", "WaitForLanding", &NPawn::WaitForLanding, 527);
	RegisterLatentAction(528, LatentRunState::WaitForLanding);
	RegisterVMNativeFunc_2("Pawn", "actorReachable", &NPawn::actorReachable, 520);
	RegisterVMNativeFunc_2("Pawn", "pointReachable", &NPawn::pointReachable, 521);

	if (engine->LaunchInfo.IsDeusEx())
	{
		RegisterVMNativeFunc_4("Pawn", "AICanHear", &NPawn::AICanHear, 706);
		RegisterVMNativeFunc_7("Pawn", "AICanSee", &NPawn::AICanSee, 705);
		RegisterVMNativeFunc_3("Pawn", "AICanSmell", &NPawn::AICanSmell, 707);
		RegisterVMNativeFunc_7("Pawn", "AIDirectionReachable", &NPawn::AIDirectionReachable, 708);
		RegisterVMNativeFunc_10("Pawn", "AIPickRandomDestination", &NPawn::AIPickRandomDestination, 709);
		RegisterVMNativeFunc_1("Pawn", "ComputePathnodeDistances", &NPawn::ComputePathnodeDistances, 1020);
		RegisterVMNativeFunc_5("Pawn", "ReachablePathnodes", &NPawn::ReachablePathnodes, 1004);
	}
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
	LogUnimplemented("Pawn.CheckValidSkinPackage");
	ReturnValue = false;
}

void NPawn::ClearPaths(UObject* Self)
{
	UPawn* selfPawn = UObject::Cast<UPawn>(Self);
	selfPawn->ClearPaths();
}

void NPawn::ClientHearSound(UObject* Self, UObject* Actor, int Id, UObject* S, const vec3& SoundLocation, const vec3& Parameters)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	UActor* AActor = UObject::Cast<UActor>(Actor);
	USound* Sound = UObject::Cast<USound>(S);

	SelfPawn->ClientHearSound(AActor, Id, Sound, SoundLocation, Parameters);
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

void NPawn::AIPickRandomDestination(UObject* Self, float minDist, float maxDist, int centralYaw, float yawDistribution, int centralPitch, float pitchDistribution, int tries, float multiplier, vec3& dest)  
{  
    UPawn* selfPawn = UObject::Cast<UPawn>(Self);  
    if (!selfPawn) { dest = {}; return; }  
  
    selfPawn->ClearPaths();  
  
    Array<UNavigationPoint*> candidates;  
    for (UNavigationPoint* nav = selfPawn->Level()->NavigationPointList(); nav; nav = nav->nextNavigationPoint())  
    {  
        if (!selfPawn->ActorReachable(nav)) continue;  
        vec3 toPoint = nav->Location() - selfPawn->Location();  
        float dist = length(toPoint);  
        if (dist < minDist || dist > maxDist * multiplier) continue;  
  
        Rotator dir = Rotator::FromVector(normalize(toPoint));  
        int yawDiff = std::abs(dir.Yaw - centralYaw);  
        int pitchDiff = std::abs(dir.Pitch - centralPitch);  
        if (yawDiff > yawDistribution * 65536.0f / 360.0f) continue;  
        if (pitchDiff > pitchDistribution * 65536.0f / 360.0f) continue;  
  
        candidates.push_back(nav);  
    }  
  
    if (candidates.empty()) { dest = {}; return; }  
  
    float r = static_cast<float>(std::rand()) / RAND_MAX;  
    size_t idx = static_cast<size_t>(r * candidates.size());  
    dest = candidates[idx]->Location();  
}

void NPawn::PickTarget(UObject* Self, float& bestAim, float& bestDist, const vec3& FireDir, const vec3& projStart, UObject*& ReturnValue)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	ReturnValue = SelfPawn->PickTarget(bestAim, bestDist, FireDir, projStart);
}

void NPawn::PickWallAdjust(UObject* Self, BitfieldBool& ReturnValue)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	ReturnValue = SelfPawn->PickWallAdjust();
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
	ReturnValue = SelfPawn->ActorReachable(UObject::Cast<UActor>(anActor), true);
}

void NPawn::pointReachable(UObject* Self, const vec3& aPoint, BitfieldBool& ReturnValue)
{
	UPawn* SelfPawn = UObject::Cast<UPawn>(Self);
	ReturnValue = SelfPawn->PointReachable(aPoint);
}

void NPawn::AICanHear(UObject* Self, UObject* Other, float* Volume, float* Radius, float& ReturnValue)
{
	LogUnimplemented("Pawn.AICanHear");
	ReturnValue = 0.0f;
}

void NPawn::AICanSee(UObject* Self, UObject* Other, float* Visibility, BitfieldBool* bCheckVisibility, BitfieldBool* bCheckDir, BitfieldBool* bCheckCylinder, BitfieldBool* bCheckLOS, float& ReturnValue)
{
	LogUnimplemented("Pawn.AICanSee");
	ReturnValue = 0.0f;
}

void NPawn::AICanSmell(UObject* Self, UObject* Other, float* Smell, float& ReturnValue)
{
	LogUnimplemented("Pawn.AICanSmell");
	ReturnValue = 0.0f;
}

void NPawn::AIDirectionReachable(UObject* Self, const vec3& Focus, int Yaw, int Pitch, float minDist, float maxDist, vec3& bestDest, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Pawn.AIDirectionReachable");
	ReturnValue = false;
}

void NPawn::AIPickRandomDestination_Deus(UObject* Self, float minDist, float maxDist, int centralYaw, float yawDistribution, int centralPitch, float pitchDistribution, int tries, float multiplier, vec3& dest, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Pawn.AIPickRandomDestination_Deus");
	ReturnValue = false;
}

void NPawn::ComputePathnodeDistances(UObject* Self, UObject** startActor)
{
	LogUnimplemented("Pawn.ComputePathnodeDistances");
}

void NPawn::LineOfSightTo_Deus(UObject* Self, UObject* Other, BitfieldBool* bIgnoreDistance, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Pawn.LineOfSightTo_Deus");
	ReturnValue = false;
}

void NPawn::ReachablePathnodes(UObject* Self, UObject* BaseClass, UObject*& NavPoint, UObject* FromPoint, float& distance, BitfieldBool* bUsePrunedPaths)
{
	LogUnimplemented("Pawn.ReachablePathnodes");
}

void NPawn::StrafeFacing_Deus(UObject* Self, const vec3& NewDestination, UObject* NewTarget, float* speed)
{
	LogUnimplemented("Pawn.StrafeFacing_Deus");
}

void NPawn::StrafeTo_Deus(UObject* Self, const vec3& NewDestination, const vec3& NewFocus, float* speed)
{
	LogUnimplemented("Pawn.StrafeTo_Deus");
}
