
#include "Precomp.h"
#include "NParticleIterator.h"
#include "VM/NativeFunc.h"
#include "UObject/UActor.h"
#include "Engine.h"

void NParticleIterator::RegisterFunctions()
{
	RegisterVMNativeFunc_1("ParticleIterator", "UpdateParticles", &NParticleIterator::UpdateParticles, 3017);
}

void NParticleIterator::UpdateParticles(UObject* Self, float DeltaTime)
{
	throw std::runtime_error("ParticleIterator.UpdateParticles not implemented");
}
