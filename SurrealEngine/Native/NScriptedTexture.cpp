
#include "Precomp.h"
#include "NScriptedTexture.h"
#include "VM/NativeFunc.h"
#include "Engine.h"
#include "UObject/UFont.h"
#include "UObject/UTexture.h"

void NScriptedTexture::RegisterFunctions()
{
	RegisterVMNativeFunc_5("ScriptedTexture", "DrawColoredText", &NScriptedTexture::DrawColoredText, 474);
	RegisterVMNativeFunc_4("ScriptedTexture", "DrawText", &NScriptedTexture::DrawText, 472);
	RegisterVMNativeFunc_10("ScriptedTexture", "DrawTile", &NScriptedTexture::DrawTile, 473);
	RegisterVMNativeFunc_1("ScriptedTexture", "ReplaceTexture", &NScriptedTexture::ReplaceTexture, 475);
	RegisterVMNativeFunc_4("ScriptedTexture", "TextSize", &NScriptedTexture::TextSize, 476);
}

void NScriptedTexture::DrawColoredText(UObject* Self, float X, float Y, const std::string& Text, UObject* Font, const Color& FontColor)
{
	UObject::Cast<UScriptedTexture>(Self)->DrawColoredText(X, Y, Text, UObject::Cast<UFont>(Font), FontColor);
}

void NScriptedTexture::DrawText(UObject* Self, float X, float Y, const std::string& Text, UObject* Font)
{
	UObject::Cast<UScriptedTexture>(Self)->DrawText(X, Y, Text, UObject::Cast<UFont>(Font));
}

void NScriptedTexture::DrawTile(UObject* Self, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, UObject* Tex, bool bMasked)
{
	UObject::Cast<UScriptedTexture>(Self)->DrawTile(X, Y, XL, YL, U, V, UL, VL, UObject::Cast<UTexture>(Tex), bMasked);
}

void NScriptedTexture::ReplaceTexture(UObject* Self, UObject* Tex)
{
	UObject::Cast<UScriptedTexture>(Self)->ReplaceTexture(UObject::Cast<UTexture>(Tex));
}

void NScriptedTexture::TextSize(UObject* Self, const std::string& Text, float& XL, float& YL, UObject* Font)
{
	UObject::Cast<UScriptedTexture>(Self)->TextSize(Text, XL, YL, UObject::Cast<UFont>(Font));
}
