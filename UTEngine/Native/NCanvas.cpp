
#include "Precomp.h"
#include "NCanvas.h"
#include "VM/NativeFunc.h"
#include "Engine.h"
#include "UObject/ULevel.h"

void NCanvas::RegisterFunctions()
{
	RegisterVMNativeFunc_3("Canvas", "DrawActor", &NCanvas::DrawActor, 467);
	RegisterVMNativeFunc_7("Canvas", "DrawClippedActor", &NCanvas::DrawClippedActor, 471);
	RegisterVMNativeFunc_9("Canvas", "DrawPortal", &NCanvas::DrawPortal, 480);
	RegisterVMNativeFunc_2("Canvas", "DrawText", &NCanvas::DrawText, 465);
	RegisterVMNativeFunc_2("Canvas", "DrawTextClipped", &NCanvas::DrawTextClipped, 469);
	RegisterVMNativeFunc_7("Canvas", "DrawTile", &NCanvas::DrawTile, 466);
	RegisterVMNativeFunc_7("Canvas", "DrawTileClipped", &NCanvas::DrawTileClipped, 468);
	RegisterVMNativeFunc_3("Canvas", "StrLen", &NCanvas::StrLen, 464);
	RegisterVMNativeFunc_3("Canvas", "TextSize", &NCanvas::TextSize, 470);
}

void NCanvas::DrawActor(UObject* Self, UObject* A, bool WireFrame, bool* ClearZ)
{
}

void NCanvas::DrawClippedActor(UObject* Self, UObject* A, bool WireFrame, int X, int Y, int XB, int YB, bool* ClearZ)
{
}

void NCanvas::DrawPortal(UObject* Self, int X, int Y, int Width, int Height, UObject* CamActor, const vec3& CamLocation, const Rotator& CamRotation, int* FOV, bool* ClearZ)
{
}

void NCanvas::DrawText(UObject* Self, const std::string& Text, bool* CR)
{
	float x = Self->GetFloat("CurX");
	float y = Self->GetFloat("CurY");
	Color c = Self->GetColor("DrawColor");
	UFont* font = (UFont*)Self->GetUObject("Font");
	Color color = Self->GetColor("DrawColor");
	float curYL = Self->GetFloat("CurYL");
	bool newline = !CR || *CR;

	// To do: process style

	color = { 255, 255, 255, 255 };

	ivec2 size = Engine::Instance->GetFontTextSize(font, Text);
	Engine::Instance->DrawFontText(&Engine::Instance->SceneFrame, font, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, 1.0f/*color.A / 255.0f*/}, x, y, Text);

	x = x + size.x;
	curYL = std::max(curYL, (float)size.y);
	if (newline)
	{
		x = 0;
		y += curYL;
		curYL = 0;
	}

	Self->SetFloat("CurX", x);
	Self->SetFloat("CurY", y);
	Self->SetFloat("CurYL", curYL);
}

void NCanvas::DrawTextClipped(UObject* Self, const std::string& Text, bool* bCheckHotKey)
{
	float x = Self->GetFloat("CurX");
	float y = Self->GetFloat("CurY");
	Color c = Self->GetColor("DrawColor");
	UFont* font = (UFont*)Self->GetUObject("Font");
	Color color = Self->GetColor("DrawColor");
	uint32_t style = Self->GetInt("Style");

	// To do: process style

	ivec2 size = Engine::Instance->GetFontTextSize(font, Text);
	Engine::Instance->DrawFontText(&Engine::Instance->SceneFrame, font, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, (int)x, (int)y, Text);

	Self->SetFloat("CurX", x + size.x);
	Self->SetFloat("CurY", y + size.y);
}

void NCanvas::DrawTile(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL)
{
	float orgX = Self->GetFloat("OrgX");
	float orgY = Self->GetFloat("OrgY");
	float x = Self->GetFloat("CurX");
	float y = Self->GetFloat("CurY");
	float z = Self->GetFloat("Z");
	uint32_t style = Self->GetInt("Style");
	Color color = Self->GetColor("DrawColor");
	float spaceX = Self->GetFloat("SpaceX");
	float spaceY = Self->GetFloat("SpaceY");
	float curYL = Self->GetFloat("CurYL");

	style = PF_NoSmooth | PF_Masked; // To do: process style properly

	Engine::Instance->DrawTile(&Engine::Instance->SceneFrame, (UTexture*)Tex, orgX + x, orgY + y, XL, YL, U, V, UL, VL, z, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, { 0.0f }, style);

	x += XL + spaceX;
	y += YL + spaceY;
	curYL = std::max(curYL, YL);

	Self->SetFloat("CurX", x);
	Self->SetFloat("CurY", y);
	Self->SetFloat("CurYL", curYL);
}

void NCanvas::DrawTileClipped(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL)
{
	float orgX = Self->GetFloat("OrgX");
	float orgY = Self->GetFloat("OrgY");
	float x = Self->GetFloat("CurX");
	float y = Self->GetFloat("CurY");
	float z = Self->GetFloat("Z");
	uint32_t style = Self->GetInt("Style");
	Color color = Self->GetColor("DrawColor");
	float spaceX = Self->GetFloat("SpaceX");
	float spaceY = Self->GetFloat("SpaceY");
	float curYL = Self->GetFloat("CurYL");

	Engine::Instance->DrawTile(&Engine::Instance->SceneFrame, (UTexture*)Tex, orgX + x, orgY + y, XL, YL, U, V, UL, VL, z, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, { 0.0f }, style);

	x += XL + spaceX;
	y += YL + spaceY;
	curYL = std::max(curYL, YL);
	Self->SetFloat("CurX", x);
	Self->SetFloat("CurY", y);
	Self->SetFloat("CurYL", curYL);
}

void NCanvas::StrLen(UObject* Self, const std::string& String, float& XL, float& YL)
{
	// To do: this needs to word wrap based on the ClipX and ClipY properties

	UFont* font = (UFont*)Self->GetUObject("Font");
	ivec2 size = Engine::Instance->GetFontTextSize(font, String);
	XL = size.x;
	YL = size.y;
}

void NCanvas::TextSize(UObject* Self, const std::string& String, float& XL, float& YL)
{
	UFont* font = (UFont*)Self->GetUObject("Font");
	ivec2 size = Engine::Instance->GetFontTextSize(font, String);
	XL = size.x;
	YL = size.y;
}
