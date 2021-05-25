
#include "Precomp.h"
#include "UActor.h"
#include "UTexture.h"
#include "UMesh.h"
#include "ULevel.h"
#include "UClass.h"

void UActor::CopyProperties()
{
	if (HasProperty("Location")) Location = GetVector("Location");
	if (HasProperty("Rotation")) Rotation = GetRotator("Rotation");
	if (HasProperty("RotationRate")) RotationRate = GetRotator("RotationRate");
	if (HasProperty("LightBrightness")) LightBrightness = GetByte("LightBrightness");
	if (HasProperty("LightHue")) LightHue = GetByte("LightHue");
	if (HasProperty("LightSaturation")) LightSaturation = GetByte("LightSaturation");
	if (HasProperty("LightRadius")) LightRadius = GetByte("LightRadius");
	if (HasProperty("bCorona")) bCorona = GetBool("bCorona");
	if (HasProperty("bHidden")) bHidden = GetBool("bHidden");
	if (HasProperty("Skin")) Skin = static_cast<UTexture*>(GetUObject("Skin"));
	if (HasProperty("Texture")) Texture = static_cast<UTexture*>(GetUObject("Texture"));
	if (HasProperty("Sprite")) Sprite = static_cast<UTexture*>(GetUObject("Sprite"));
	if (HasProperty("Mesh")) Mesh = static_cast<UMesh*>(GetUObject("Mesh"));
	if (HasProperty("Brush")) Brush = static_cast<UModel*>(GetUObject("Brush"));
	if (HasProperty("DrawType")) DrawType = (ActorDrawType)GetByte("DrawType");
	if (HasProperty("DrawScale")) DrawScale = GetFloat("DrawScale");
}
