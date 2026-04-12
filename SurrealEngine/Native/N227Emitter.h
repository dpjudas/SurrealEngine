#pragma once

#include "UObject/UObject.h"

class NXParticleEmitter
{
public:
    static void RegisterFunctions();

    static void SetParticlesProps(UObject* Self, std::optional<float> Speed, std::optional<float> Scale);
    static void AllParticles(UObject* Self, UObject*& OutActor);
};

class NXEmitter
{
public:
    static void RegisterFunctions();

    static void SpawnParticles(UObject* Self, int Count);
    static void SetMaxParticles(UObject* Self, int MaxParts);
    static void Kill(UObject* Self);
    static void EmTrigger(UObject* Self);
};