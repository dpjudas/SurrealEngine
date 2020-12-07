
#include "Precomp.h"
#include "UActor.h"
#include "UTexture.h"
#include "UMesh.h"
#include "UClass.h"

UActor::UActor(ObjectStream* stream) : UObject(stream)
{
	if (Properties.HasScalar("Location")) Location = Properties.GetScalar("Location").ValueVector;
	if (Properties.HasScalar("LightBrightness")) LightBrightness = Properties.GetScalar("LightBrightness").ValueByte;
	if (Properties.HasScalar("LightHue")) LightHue = Properties.GetScalar("LightHue").ValueByte;
	if (Properties.HasScalar("LightSaturation")) LightSaturation = Properties.GetScalar("LightSaturation").ValueByte;
	if (Properties.HasScalar("LightRadius")) LightRadius = Properties.GetScalar("LightRadius").ValueByte;
	if (Properties.HasScalar("bCorona")) bCorona = Properties.GetScalar("bCorona").ValueBool;
	if (Properties.HasScalar("Skin")) Skin = Cast<UTexture>(Properties.GetUObject("Skin"));
	if (Properties.HasScalar("Texture")) Skin = Cast<UTexture>(Properties.GetUObject("Texture"));
	if (Properties.HasScalar("Mesh"))
		Mesh = Cast<UMesh>(Properties.GetUObject("Mesh"));
	else if (Base->Properties.HasScalar("Mesh"))
		Mesh = Cast<UMesh>(Base->Properties.GetUObject("Mesh"));
}
