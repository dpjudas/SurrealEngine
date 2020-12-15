
#include "Precomp.h"
#include "NScriptedTexture.h"
#include "VM/NativeFunc.h"

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
	throw std::runtime_error("ScriptedTexture.DrawColoredText not implemented");
}

void NScriptedTexture::DrawText(UObject* Self, float X, float Y, const std::string& Text, UObject* Font)
{
	throw std::runtime_error("ScriptedTexture.DrawText not implemented");
}

void NScriptedTexture::DrawTile(UObject* Self, float X, float Y, float XL, float YL, float U, float V, float UL, float VL, UObject* Tex, bool bMasked)
{
	throw std::runtime_error("ScriptedTexture.DrawTile not implemented");
}

void NScriptedTexture::ReplaceTexture(UObject* Self, UObject* Tex)
{
	throw std::runtime_error("ScriptedTexture.ReplaceTexture not implemented");
}

void NScriptedTexture::TextSize(UObject* Self, const std::string& Text, float& XL, float& YL, UObject* Font)
{
	throw std::runtime_error("ScriptedTexture.TextSize not implemented");
}
