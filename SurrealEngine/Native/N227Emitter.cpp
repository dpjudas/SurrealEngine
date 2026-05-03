
#include "Precomp.h"
#include "N227Emitter.h"
#include "UObject/U227Emitter.h"
#include "NActor.h"
#include "VM/NativeFunc.h"
#include "VM/ScriptCall.h"
#include "VM/Frame.h"

void NXParticleEmitter::RegisterFunctions()
{
    RegisterVMNativeFunc_3("XParticleEmitter", "SetParticlesProps", &NXParticleEmitter::SetParticlesProps, 1770);
    RegisterVMNativeFunc_2("XParticleEmitter", "AllParticles", &NXParticleEmitter::AllParticles, 1771);
}

void NXParticleEmitter::AllParticles(UObject* Self, UObject*& OutActor)
{
    Frame::CreatedIterator = std::make_unique<AllParticlesIterator>(&OutActor);
}

void NXParticleEmitter::SetParticlesProps(UObject* Self, std::optional<float> Speed, std::optional<float> Scale)
{
    auto XPESelf = UObject::Cast<UXParticleEmitter>(Self);
    XPESelf->SetParticlesProps(Speed, Scale);
}

void NXEmitter::RegisterFunctions()
{
    RegisterVMNativeFunc_2("XEmitter", "SpawnParticles", &NXEmitter::SpawnParticles, 1766);
    RegisterVMNativeFunc_2("XEmitter", "SetMaxParticles", &NXEmitter::SetMaxParticles, 1767);
    RegisterVMNativeFunc_1("XEmitter", "Kill", &NXEmitter::Kill, 1768);
    RegisterVMNativeFunc_1("XEmitter", "EmTrigger", &NXEmitter::EmTrigger, 1769);
}

void NXEmitter::EmTrigger(UObject* Self)
{
    auto XESelf = UObject::Cast<UXEmitter>(Self);
    XESelf->EmTrigger();
}

void NXEmitter::Kill(UObject* Self)
{
    auto XESelf = UObject::Cast<UXEmitter>(Self);
    XESelf->Kill();
}

void NXEmitter::SetMaxParticles(UObject* Self, const int MaxParts)
{
    auto XESelf = UObject::Cast<UXEmitter>(Self);
    XESelf->SetMaxParticles(MaxParts);
}

void NXEmitter::SpawnParticles(UObject* Self, const int Count)
{
    auto XESelf = UObject::Cast<UXEmitter>(Self);
    XESelf->SpawnParticles(Count);
}
