
#include "Precomp.h"
#include "ULight.h"
#include "UTexture.h"

ULight::ULight(ObjectStream* stream) : UObject(stream)
{
	if (Properties.HasScalar("Location")) Location = Properties.GetScalar("Location").ValueVector;
	if (Properties.HasScalar("LightBrightness")) LightBrightness = Properties.GetScalar("LightBrightness").ValueByte;
	if (Properties.HasScalar("LightHue")) LightHue = Properties.GetScalar("LightHue").ValueByte;
	if (Properties.HasScalar("LightSaturation")) LightSaturation = Properties.GetScalar("LightSaturation").ValueByte;
	if (Properties.HasScalar("LightRadius")) LightRadius = Properties.GetScalar("LightRadius").ValueByte;
	if (Properties.HasScalar("bCorona")) bCorona = Properties.GetScalar("bCorona").ValueBool;
	if (Properties.HasScalar("Skin")) Skin = Cast<UTexture>(stream->GetUObject(Properties.GetScalar("Skin").ValueObject));
}
