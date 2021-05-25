
#include "Precomp.h"
#include "UActor.h"
#include "UTexture.h"
#include "UMesh.h"
#include "ULevel.h"
#include "UClass.h"

void UActor::CopyProperties()
{
	Location = GetVector("Location");
	Rotation = GetRotator("Rotation");
	RotationRate = GetRotator("RotationRate");
	LightBrightness = GetByte("LightBrightness");
	LightHue = GetByte("LightHue");
	LightSaturation = GetByte("LightSaturation");
	LightRadius = GetByte("LightRadius");
	bCorona = GetBool("bCorona");
	bHidden = GetBool("bHidden");
	Skin = static_cast<UTexture*>(GetUObject("Skin"));
	Texture = static_cast<UTexture*>(GetUObject("Texture"));
	Sprite = static_cast<UTexture*>(GetUObject("Sprite"));
	Mesh = static_cast<UMesh*>(GetUObject("Mesh"));
	Brush = static_cast<UModel*>(GetUObject("Brush"));
	DrawType = (ActorDrawType)GetByte("DrawType");
	DrawScale = GetFloat("DrawScale");
}
