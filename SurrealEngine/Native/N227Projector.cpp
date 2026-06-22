#include "Precomp.h"
#include "N227Projector.h"
#include "NActor.h"
#include "Utils/Logger.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"

void N227Projector::RegisterFunctions()
{
    RegisterVMNativeFunc_0("Projector", "AttachPrjDecal", &N227Projector::AttachPrjDecal, 350);
    RegisterVMNativeFunc_0("Projector", "DeattachPrjDecal", &N227Projector::DeattachPrjDecal, 351);
    RegisterVMNativeFunc_1("Projector", "AttachActor", &N227Projector::AttachActor, 352);
    RegisterVMNativeFunc_1("Projector", "DeattachActor", &N227Projector::DeattachActor, 353);
    RegisterVMNativeFunc_0("Projector", "DeattachAllActors", &N227Projector::DeattachAllActors, 354);
}

void N227Projector::AttachPrjDecal(UObject* Self)
{
    LogUnimplemented("Projector.AttachPrjDecal() [U227]");
}

void N227Projector::DeattachPrjDecal(UObject* Self)
{
    LogUnimplemented("Projector.DeattachPrjDecal() [U227]");
}

void N227Projector::AttachActor(UObject* Self, UObject* OtherActor)
{
    LogUnimplemented("Projector.AttachActor() [U227]");
}

void N227Projector::DeattachActor(UObject* Self, UObject* OtherActor)
{
    LogUnimplemented("Projector.DetachActor() [U227]");
}

void N227Projector::DeattachAllActors(UObject* Self)
{
    LogUnimplemented("Projector.DetachAllActors() [U227]");
}
