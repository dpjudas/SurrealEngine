
#include "Precomp.h"
#include "NCanvas.h"
#include "VM/NativeFunc.h"
#include "Engine.h"
#include "UObject/ULevel.h"
#include "Renderer/UTRenderer.h"
#include "Renderer/CanvasRender.h"

#ifdef _MSC_VER
#pragma warning(disable: 4244) // warning C4244: 'argument': conversion from 'float' to 'int', possible loss of data
#endif

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
	float orgX = Self->GetFloat("OrgX");
	float orgY = Self->GetFloat("OrgY");
	float curX = Self->GetFloat("CurX");
	float curY = Self->GetFloat("CurY");
	UFont* font = (UFont*)Self->GetUObject("Font");
	Color color = Self->GetColor("DrawColor");
	float curYL = Self->GetFloat("CurYL");
	bool newline = !CR || *CR;
	uint32_t style = Self->GetByte("Style");

	if (style != 0)
	{
		uint32_t renderflags = PF_NoSmooth | PF_Masked;
		if (style == 3)
			renderflags |= PF_Translucent;
		else if (style == 4)
			renderflags |= PF_Modulated;

		engine->renderer->canvas.DrawText(font, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, orgX, orgY, curX, curY, curYL, newline, Text, renderflags);
	}

	Self->SetFloat("CurX", curX);
	Self->SetFloat("CurY", curY);
	Self->SetFloat("CurYL", curYL);
}

void NCanvas::DrawTextClipped(UObject* Self, const std::string& Text, bool* bCheckHotKey)
{
	float orgX = Self->GetFloat("OrgX");
	float orgY = Self->GetFloat("OrgY");
	float curX = Self->GetFloat("CurX");
	float curY = Self->GetFloat("CurY");
	float clipX = Self->GetFloat("ClipX");
	UFont* font = (UFont*)Self->GetUObject("Font");
	Color color = Self->GetColor("DrawColor");
	uint32_t style = Self->GetByte("Style");
	bool checkHotKey = bCheckHotKey && *bCheckHotKey;
	
	if (style != 0)
	{
		uint32_t renderflags = PF_NoSmooth | PF_Masked;
		if (style == 3)
			renderflags |= PF_Translucent;
		else if (style == 4)
			renderflags |= PF_Modulated;

		engine->renderer->canvas.DrawTextClipped(font, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, orgX, orgY, curX, curY, Text, renderflags, checkHotKey, clipX);
	}
}

void NCanvas::DrawTile(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL)
{
	float orgX = Self->GetFloat("OrgX");
	float orgY = Self->GetFloat("OrgY");
	float curX = Self->GetFloat("CurX");
	float curY = Self->GetFloat("CurY");
	float z = Self->GetFloat("Z");
	uint32_t style = Self->GetByte("Style");
	Color color = Self->GetColor("DrawColor");
	float spaceX = Self->GetFloat("SpaceX");
	float curYL = Self->GetFloat("CurYL");
	bool noSmooth = Self->GetBool("bNoSmooth");

	if (style != 0)
	{
		uint32_t renderflags = PF_TwoSided;
		if (style == 3)
			renderflags |= PF_Translucent;
		else if (style == 4)
			renderflags |= PF_Modulated;
		if (noSmooth)
			renderflags |= PF_NoSmooth;
		engine->renderer->canvas.DrawTile((UTexture*)Tex, orgX + curX, orgY + curY, XL, YL, U, V, UL, VL, z, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, { 0.0f }, renderflags);
	}

	curX += XL + spaceX;
	curYL = std::max(curYL, YL);

	Self->SetFloat("CurX", curX);
	Self->SetFloat("CurYL", curYL);
}

void NCanvas::DrawTileClipped(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL)
{
	float orgX = Self->GetFloat("OrgX");
	float orgY = Self->GetFloat("OrgY");
	float curX = Self->GetFloat("CurX");
	float curY = Self->GetFloat("CurY");
	float curZ = Self->GetFloat("Z");
	uint32_t style = Self->GetByte("Style");
	Color color = Self->GetColor("DrawColor");
	float spaceX = Self->GetFloat("SpaceX");
	float spaceY = Self->GetFloat("SpaceY");
	float curYL = Self->GetFloat("CurYL");
	bool noSmooth = Self->GetBool("bNoSmooth");
	float clipX = Self->GetFloat("ClipX");
	float clipY = Self->GetFloat("ClipY");

	if (XL > 0.0f && YL > 0.0f)
	{
		if (curX < 0.0f)
		{
			float t = curX * UL / XL;
			U -= t;
			UL += t;
			XL += curX;
			curX = 0.0f;
		}
		if (curY < 0.0f)
		{
			float t = curY * VL / YL;
			V -= t;
			VL += t;
			YL += curY;
			curY = 0.0f;
		}
		if (XL > clipX - curX)
		{
			UL += (clipX - curX - XL) * UL / XL;
			XL = clipX - curX;
		}
		if (YL > clipY - curY)
		{
			VL += (clipY - curY - YL) * VL / YL;
			YL = clipY - curY;
		}
		if (style != 0)
		{
			uint32_t renderflags = PF_TwoSided;
			if (style == 3)
				renderflags |= PF_Translucent;
			else if (style == 4)
				renderflags |= PF_Modulated;
			if (noSmooth)
				renderflags |= PF_NoSmooth;
			engine->renderer->canvas.DrawTile((UTexture*)Tex, orgX + curX, orgY + curY, XL, YL, U, V, UL, VL, curZ, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, { 0.0f }, renderflags);
		}

		curX += XL + spaceX;
		curYL = std::max(curYL, YL);

		Self->SetFloat("CurX", curX);
		Self->SetFloat("CurYL", curYL);
	}
}

void NCanvas::StrLen(UObject* Self, const std::string& String, float& XL, float& YL)
{
	UFont* font = (UFont*)Self->GetUObject("Font");
	float clipX = Self->GetFloat("ClipX");
	ivec2 size = engine->renderer->canvas.GetTextClippedSize(font, String, clipX);
	XL = size.x + 100;
	YL = size.y;
}

void NCanvas::TextSize(UObject* Self, const std::string& String, float& XL, float& YL)
{
	UFont* font = (UFont*)Self->GetUObject("Font");
	ivec2 size = engine->renderer->canvas.GetTextSize(font, String);
	XL = size.x;
	YL = size.y;
}
