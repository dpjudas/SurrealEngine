
#include "Precomp.h"
#include "NCanvas.h"
#include "VM/NativeFunc.h"
#include "Engine.h"
#include "UObject/ULevel.h"
#include "UObject/UClient.h"
#include "UObject/UActor.h"
#include "Render/RenderSubsystem.h"

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

void NCanvas::DrawActor(UObject* Self, UObject* A, bool WireFrame, BitfieldBool* ClearZ)
{
	engine->render->DrawActor(UObject::Cast<UActor>(A), WireFrame, ClearZ ? *ClearZ : false);
}

void NCanvas::DrawClippedActor(UObject* Self, UObject* A, bool WireFrame, int X, int Y, int XB, int YB, BitfieldBool* ClearZ)
{
	engine->render->DrawClippedActor(UObject::Cast<UActor>(A), WireFrame, X, Y, XB, YB, ClearZ ? *ClearZ : false);
}

void NCanvas::DrawPortal(UObject* Self, int X, int Y, int Width, int Height, UObject* CamActor, const vec3& CamLocation, const Rotator& CamRotation, int* FOV, BitfieldBool* ClearZ)
{
}

void NCanvas::DrawText(UObject* Self, const std::string& Text, BitfieldBool* CR)
{
	UCanvas* SelfCanvas = UObject::Cast<UCanvas>(Self);
	float& orgX = SelfCanvas->OrgX();
	float& orgY = SelfCanvas->OrgY();
	float& curX = SelfCanvas->CurX();
	float& curY = SelfCanvas->CurY();
	UFont*& font = SelfCanvas->Font();
	Color& color = SelfCanvas->DrawColor();
	float& curYL = SelfCanvas->CurYL();
	uint8_t& style = SelfCanvas->Style();
	bool center = SelfCanvas->bCenter();
	float& spaceX = SelfCanvas->SpaceX();
	float& spaceY = SelfCanvas->SpaceY();

	bool newline = CR ? *CR : true;

	if (style != 0)
	{
		uint32_t renderflags = PF_NoSmooth | PF_Masked;
		if (style == 3)
			renderflags |= PF_Translucent;
		else if (style == 4)
			renderflags |= PF_Modulated;

		engine->render->DrawText(font, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, orgX, orgY, curX, curY, curYL, newline, Text, renderflags, center, spaceX, spaceY);
	}
}

void NCanvas::DrawTextClipped(UObject* Self, const std::string& Text, BitfieldBool* bCheckHotKey)
{
	UCanvas* SelfCanvas = UObject::Cast<UCanvas>(Self);
	float& orgX = SelfCanvas->OrgX();
	float& orgY = SelfCanvas->OrgY();
	float& curX = SelfCanvas->CurX();
	float& curY = SelfCanvas->CurY();
	float& clipX = SelfCanvas->ClipX();
	float& clipY = SelfCanvas->ClipY();
	UFont*& font = SelfCanvas->Font();
	Color& color = SelfCanvas->DrawColor();
	uint8_t& style = SelfCanvas->Style();
	bool center = SelfCanvas->bCenter();

	bool checkHotKey = bCheckHotKey && *bCheckHotKey;
	
	if (style != 0)
	{
		uint32_t renderflags = PF_NoSmooth | PF_Masked;
		if (style == 3)
			renderflags |= PF_Translucent;
		else if (style == 4)
			renderflags |= PF_Modulated;

		engine->render->DrawTextClipped(font, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, orgX, orgY, curX, curY, Text, renderflags, checkHotKey, clipX, clipY, center);
	}
}

void NCanvas::DrawTile(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL)
{
	UCanvas* SelfCanvas = UObject::Cast<UCanvas>(Self);
	float& orgX = SelfCanvas->OrgX();
	float& orgY = SelfCanvas->OrgY();
	float& curX = SelfCanvas->CurX();
	float& curY = SelfCanvas->CurY();
	float& z = SelfCanvas->Z();
	uint8_t& style = SelfCanvas->Style();
	Color& color = SelfCanvas->DrawColor();
	float& spaceX = SelfCanvas->SpaceX();
	float& spaceY = SelfCanvas->SpaceY();
	float& curYL = SelfCanvas->CurYL();
	bool noSmooth = SelfCanvas->bNoSmooth();

	if (XL <= 0.0f || YL <= 0.0f)
		return;

	if (style != 0)
	{
		uint32_t renderflags = PF_TwoSided;
		if (style == 3)
			renderflags |= PF_Translucent;
		else if (style == 4)
			renderflags |= PF_Modulated;
		if (noSmooth)
			renderflags |= PF_NoSmooth;
		engine->render->DrawTile((UTexture*)Tex, orgX + curX, orgY + curY, XL, YL, U, V, UL, VL, 1.0f, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, { 0.0f }, renderflags);
	}

	curX += XL + spaceX;
	curYL = std::max(curYL, YL + spaceY);
}

void NCanvas::DrawTileClipped(UObject* Self, UObject* Tex, float XL, float YL, float U, float V, float UL, float VL)
{
	UCanvas* SelfCanvas = UObject::Cast<UCanvas>(Self);
	float& orgX = SelfCanvas->OrgX();
	float& orgY = SelfCanvas->OrgY();
	float& curX = SelfCanvas->CurX();
	float& curY = SelfCanvas->CurY();
	float& z = SelfCanvas->Z();
	uint8_t& style = SelfCanvas->Style();
	Color& color = SelfCanvas->DrawColor();
	float& spaceX = SelfCanvas->SpaceX();
	float& spaceY = SelfCanvas->SpaceY();
	float& curYL = SelfCanvas->CurYL();
	bool noSmooth = SelfCanvas->bNoSmooth();
	float& clipX = SelfCanvas->ClipX();
	float& clipY = SelfCanvas->ClipY();

	if (XL <= 0.0f || YL <= 0.0f)
		return;

	if (style != 0)
	{
		uint32_t renderflags = PF_TwoSided;
		if (style == 3)
			renderflags |= PF_Translucent;
		else if (style == 4)
			renderflags |= PF_Modulated;
		if (noSmooth)
			renderflags |= PF_NoSmooth;
		engine->render->DrawTileClipped((UTexture*)Tex, orgX, orgY, curX, curY, XL, YL, U, V, UL, VL, 1.0f, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, { 0.0f }, renderflags, clipX, clipY);
	}

	float nextX = clamp(curX + XL, 0.0f, clipX);
	float nextY = clamp(curY + YL, 0.0f, clipY);

	XL = nextX - curX;
	YL = nextY - curY;

	curX += XL + spaceX;
	curYL = std::max(curYL, YL + spaceY);
}

void NCanvas::StrLen(UObject* Self, const std::string& String, float& XL, float& YL)
{
	UCanvas* SelfCanvas = UObject::Cast<UCanvas>(Self);
	UFont*& font = SelfCanvas->Font();
	float& clipX = SelfCanvas->ClipX();

	ivec2 size = engine->render->GetTextClippedSize(font, String, clipX);
	XL = size.x;
	YL = size.y;
}

void NCanvas::TextSize(UObject* Self, const std::string& String, float& XL, float& YL)
{
	UCanvas* SelfCanvas = UObject::Cast<UCanvas>(Self);
	UFont*& font = SelfCanvas->Font();

	ivec2 size = engine->render->GetTextSize(font, String);
	XL = size.x;
	YL = size.y;
}
