#pragma once

#include "UTexture.h"

class UScriptedTexture : public UTexture
{
public:
	using UTexture::UTexture;

	void Load(ObjectStream* stream) override;
	void Save(PackageStreamWriter* stream) override;

	int& Junk1() { return Value<int>(PropOffsets_ScriptedTexture.Junk1); }
	int& Junk2() { return Value<int>(PropOffsets_ScriptedTexture.Junk2); }
	int& Junk3() { return Value<int>(PropOffsets_ScriptedTexture.Junk3); }
	float& LocalTime() { return Value<float>(PropOffsets_ScriptedTexture.LocalTime); }
	UActor*& NotifyActor() { return Value<UActor*>(PropOffsets_ScriptedTexture.NotifyActor); }
	UTexture*& SourceTexture() { return Value<UTexture*>(PropOffsets_ScriptedTexture.SourceTexture); }

	void UpdateFrame() override;

	void DrawColoredText(float X, float Y, const std::string& Text, UFont* Font, const Color& FontColor);
	void DrawText(float X, float Y, const std::string& Text, UFont* Font);
	void DrawTile(float X, float Y, float XL, float YL, float U, float V, float UL, float VL, UTexture* Tex, bool bMasked);
	void ReplaceTexture(UTexture* Tex);
	void TextSize(const std::string& Text, float& XL, float& YL, UFont* Font);

private:
	void DrawTileP8(UTexture* texture, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight);
	void DrawTileP8Masked(UTexture* texture, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight);
	void DrawTileP8Color(UTexture* texture, float left, float top, float width, float height, float u, float v, float uvwidth, float uvheight, uint8_t color);

	double LastUpdate = -1.0;
};
