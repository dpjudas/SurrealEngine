
#include "Precomp.h"
#include "Random.h"
#include <random>

static std::default_random_engine s_Generator;

int RandInt(int Min, int Max)
{
    std::uniform_int_distribution<int> distribution(Min, Max);

    return distribution(s_Generator);
}

int RandInt(int Max)
{
    return RandInt(0, Max);
}

float FRand()
{
    return FRandRange(0.0f, 1.0f);
}

float FRandRange(float Min, float Max)
{
    std::uniform_real_distribution<float> distribution(Min, Max);

    return distribution(s_Generator);
}
