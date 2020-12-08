
#include "Precomp.h"
#include "UActor.h"
#include "UTexture.h"
#include "UMesh.h"
#include "UClass.h"

UActor::UActor(ObjectStream* stream) : UObject(stream)
{
	if (HasScalar("Location")) Location = GetScalar("Location").ValueVector;
	if (HasScalar("LightBrightness")) LightBrightness = GetScalar("LightBrightness").ValueByte;
	if (HasScalar("LightHue")) LightHue = GetScalar("LightHue").ValueByte;
	if (HasScalar("LightSaturation")) LightSaturation = GetScalar("LightSaturation").ValueByte;
	if (HasScalar("LightRadius")) LightRadius = GetScalar("LightRadius").ValueByte;
	if (HasScalar("bCorona")) bCorona = GetScalar("bCorona").ValueBool;
	if (HasScalar("Skin")) Skin = Cast<UTexture>(GetUObject("Skin"));
	if (HasScalar("Texture")) Skin = Cast<UTexture>(GetUObject("Texture"));
	if (HasScalar("Mesh")) Mesh = Cast<UMesh>(GetUObject("Mesh"));
	if (HasScalar("DrawType")) DrawType = (ActorDrawType)GetScalar("DrawType").ValueByte;
}
