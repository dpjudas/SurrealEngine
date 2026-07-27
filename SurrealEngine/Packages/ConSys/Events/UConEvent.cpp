
#include "Precomp.h"
#include "UConEvent.h"
#include "Packages/Engine/Resources/USound.h"

float UConEvent::GetSoundLength(USound* sound)
{
	return sound ? sound->GetDuration() : 1.0f;
}
