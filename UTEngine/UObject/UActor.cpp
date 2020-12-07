
#include "Precomp.h"
#include "UActor.h"
#include "UTexture.h"
#include "UMesh.h"

UActor::UActor(ObjectStream* stream) : UObject(stream)
{
	if (Properties.HasScalar("Location")) Location = Properties.GetScalar("Location").ValueVector;
	if (Properties.HasScalar("LightBrightness")) LightBrightness = Properties.GetScalar("LightBrightness").ValueByte;
	if (Properties.HasScalar("LightHue")) LightHue = Properties.GetScalar("LightHue").ValueByte;
	if (Properties.HasScalar("LightSaturation")) LightSaturation = Properties.GetScalar("LightSaturation").ValueByte;
	if (Properties.HasScalar("LightRadius")) LightRadius = Properties.GetScalar("LightRadius").ValueByte;
	if (Properties.HasScalar("bCorona")) bCorona = Properties.GetScalar("bCorona").ValueBool;
	if (Properties.HasScalar("Skin")) Skin = Cast<UTexture>(stream->GetUObject(Properties.GetScalar("Skin").ValueObject));
	if (Properties.HasScalar("Texture")) Skin = Cast<UTexture>(stream->GetUObject(Properties.GetScalar("Texture").ValueObject));
	if (Properties.HasScalar("Mesh")) Mesh = Cast<UMesh>(stream->GetUObject(Properties.GetScalar("Mesh").ValueObject));
}
