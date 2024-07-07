
#include "Precomp.h"
#include "NScriptedTexture.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

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
	LogUnimplemented("ScriptedTexture.DrawColoredText");
}

void NScriptedTexture::DrawText(UObject* Self, float X, float Y, const std::string& Text, UObject* Font)
{
	LogUnimplemented("ScriptedTexture.DrawText");
}

void NScriptedTexture::DrawTile(UObject* Self, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, UObject* Tex, bool bMasked)
{
	LogUnimplemented("ScriptedTexture.DrawTile");
}

void NScriptedTexture::ReplaceTexture(UObject* Self, UObject* Tex)
{
	LogUnimplemented("ScriptedTexture.ReplaceTexture");
}

void NScriptedTexture::TextSize(UObject* Self, const std::string& Text, float& XL, float& YL, UObject* Font)
{
	LogUnimplemented("ScriptedTexture.TextSize");
	XL = 0.0f;
	YL = 0.0f;
}
