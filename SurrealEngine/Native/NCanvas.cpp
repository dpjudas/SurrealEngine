
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
	RegisterVMNativeFunc_7("Canvas", "DrawClippedActor", &NCanvas::DrawClippedActor, 471);
	RegisterVMNativeFunc_9("Canvas", "DrawPortal", &NCanvas::DrawPortal, 480);
	RegisterVMNativeFunc_2("Canvas", "DrawText", &NCanvas::DrawText, 465);
	RegisterVMNativeFunc_2("Canvas", "DrawTextClipped", &NCanvas::DrawTextClipped, 469);
	RegisterVMNativeFunc_7("Canvas", "DrawTile", &NCanvas::DrawTile, 466);
	RegisterVMNativeFunc_7("Canvas", "DrawTileClipped", &NCanvas::DrawTileClipped, 468);
	if (engine->LaunchInfo.engineVersion > 219)
	{
		RegisterVMNativeFunc_3("Canvas", "DrawActor", &NCanvas::DrawActor, 467);
		RegisterVMNativeFunc_3("Canvas", "StrLen", &NCanvas::StrLen, 464);
	}
	else
	{
		RegisterVMNativeFunc_5("Canvas", "StrLen", &NCanvas::StrLen_219, 467); // Maybe only in KHG?
	}
	RegisterVMNativeFunc_3("Canvas", "TextSize", &NCanvas::TextSize, 470);
	if (engine->LaunchInfo.IsUnreal1_227())
	{
		RegisterVMNativeFunc_3("Canvas", "Draw2DLine", &NCanvas::Draw2DLine_U227, 1750);
		RegisterVMNativeFunc_3("Canvas", "Draw3DLine", &NCanvas::Draw3DLine_U227, 1751);
		RegisterVMNativeFunc_4("Canvas", "WorldToScreen", &NCanvas::WorldToScreen_U227, 1752);
		RegisterVMNativeFunc_3("Canvas", "ScreenToWorld", &NCanvas::ScreenToWorld_U227, 1753);
		RegisterVMNativeFunc_3("Canvas", "DrawPathNetwork", &NCanvas::DrawPathNetwork_U227, 1754);
		RegisterVMNativeFunc_5("Canvas", "DrawCircle", &NCanvas::DrawCircle_U227, 1755);
		RegisterVMNativeFunc_5("Canvas", "DrawBox", &NCanvas::DrawBox_U227, 1756);
		RegisterVMNativeFunc_2("Canvas", "GetCameraCoords", &NCanvas::GetCameraCoords_U227, 1757);
		RegisterVMNativeFunc_6("Canvas", "SetTile3DOffset", &NCanvas::SetTile3DOffset_U227, 1758);
		// RegisterVMNativeFunc_5("Canvas", "DrawTris", &NCanvas::DrawTris_U227, 1746);
		RegisterVMNativeFunc_13("Canvas", "DrawRotatedTile", &NCanvas::DrawRotatedTile_U227, 1747);
		RegisterVMNativeFunc_4("Canvas", "PushClipPlane", &NCanvas::PushClipPlane_U227, 1748);
		RegisterVMNativeFunc_1("Canvas", "PopClipPlane", &NCanvas::PopClipPlane_U227, 1749);
		RegisterVMNativeFunc_3("Canvas", "SetZTest", &NCanvas::SetZTest_U227, 1759);
		RegisterVMNativeFunc_3("Canvas", "PushCanvasScale", &NCanvas::PushCanvasScale_U227, 1762);
		RegisterVMNativeFunc_1("Canvas", "PopCanvasScale", &NCanvas::PopCanvasScale_U227, 1763);
		RegisterVMNativeFunc_3("Canvas", "SetCustomLighting", &NCanvas::SetCustomLighting_U227, 1774);
		RegisterVMNativeFunc_4("Canvas", "AddCustomLightSource", &NCanvas::AddCustomLightSource_U227, 1775);
		RegisterVMNativeFunc_1("Canvas", "ClearCustomLightSources", &NCanvas::ClearCustomLightSources_U227, 1776);
		RegisterVMNativeFunc_1("Canvas", "Reset", &NCanvas::Reset_U227, 0);
	}
}

void NCanvas::DrawActor(UObject* Self, UObject* A, bool WireFrame, std::optional<bool> ClearZ)
{
	engine->render->DrawActor(UObject::Cast<UActor>(A), WireFrame, ClearZ ? *ClearZ : false);
}

void NCanvas::DrawClippedActor(UObject* Self, UObject* A, bool WireFrame, int X, int Y, int XB, int YB, std::optional<bool> ClearZ)
{
	engine->render->DrawClippedActor(UObject::Cast<UActor>(A), WireFrame, X, Y, XB, YB, ClearZ ? *ClearZ : false);
}

void NCanvas::DrawPortal(UObject* Self, int X, int Y, int Width, int Height, UObject* CamActor, const vec3& CamLocation, const Rotator& CamRotation, std::optional<int> FOV, std::optional<bool> ClearZ)
{
}

void NCanvas::DrawText(UObject* Self, const std::string& Text, std::optional<bool> CR)
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

		float curXL = 0.0f;
		engine->render->DrawText(font, { color.R / 255.0f, color.G / 255.0f, color.B / 255.0f, color.A / 255.0f }, orgX, orgY, curX, curY, curXL, curYL, newline, Text, renderflags, center, spaceX, spaceY, clipX, clipY);
	}
	else
	{
		if (newline)
		{
			curX = 0;
			curY += curYL;
			curYL = 0;
		}
	}
}

void NCanvas::DrawTextClipped(UObject* Self, const std::string& Text, std::optional<bool> bCheckHotKey)
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
		if (style == 2)
			renderflags |= PF_Masked;
		else if (style == 3)
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
		if (style == 2)
			renderflags |= PF_Masked;
		else if (style == 3)
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
	float& orgX = SelfCanvas->OrgX();
	float& orgY = SelfCanvas->OrgY();
	float& clipX = SelfCanvas->ClipX();
	float& clipY = SelfCanvas->ClipY();
	UFont*& font = SelfCanvas->Font();
	bool center = SelfCanvas->bCenter();
	float& spaceX = SelfCanvas->SpaceX();
	float& spaceY = SelfCanvas->SpaceY();
	uint8_t& style = SelfCanvas->Style();

	if (style != 0)
	{
		float curX = 0.0f;
		float curY = 0.0f;
		float curXL = 0.0f;
		float curYL = 0.0f;
		engine->render->DrawText(font, vec4(1.0f), orgX, orgY, curX, curY, curXL, curYL, false, String, 0, center, spaceX, spaceY, clipX, clipY, true);
		XL = curXL;
		YL = curYL;
	}
	else
	{
		XL = 0.0f;
		YL = 0.0f;
	}
}

void NCanvas::StrLen_219(UObject* Self, const std::string& Text, int NumChars, int StartIndex, int& XL, int& YL)
{
	std::string String = Text.substr(StartIndex, NumChars);
	float width, height;
	StrLen(Self, String, width, height);
	XL = (int)std::round(width);
	YL = (int)std::round(height);
}

void NCanvas::TextSize(UObject* Self, const std::string& String, float& XL, float& YL)
{
	UCanvas* SelfCanvas = UObject::Cast<UCanvas>(Self);
	UFont*& font = SelfCanvas->Font();

	vec2 size = engine->render->GetTextSize(font, String);
	XL = size.x;
	YL = size.y;
}

void NCanvas::Draw2DLine_U227(UObject* Self, Color& Col, vec3& Start, vec3& End)
{
	LogUnimplemented("Canvas.Draw2DLine() [U227]");
}

void NCanvas::Draw3DLine_U227(UObject* Self, Color& Col, vec3& Start, vec3& End)
{
	LogUnimplemented("Canvas.Draw3DLine() [U227]");
}

void NCanvas::WorldToScreen_U227(UObject* Self, vec3& WorldPos, std::optional<float> ZDistance, vec3& ReturnValue)
{
	LogUnimplemented("Canvas.WorldToScreen() [U227]");
	ReturnValue = vec3();
}

void NCanvas::ScreenToWorld_U227(UObject* Self, vec3& ScreenPos, vec3& ReturnValue)
{
	LogUnimplemented("Canvas.ScreenToWorld() [U227]");
	ReturnValue = vec3();
}

void NCanvas::DrawPathNetwork_U227(UObject* Self, BitfieldBool& bOnlyWalkable, std::optional<float> MaxDistance)
{
	LogUnimplemented("Canvas.DrawPathNetwork() [U227]");
}

void NCanvas::DrawCircle_U227(UObject* Self, Color& Col, int LineFlags, vec3& WorldPosition, float Radius)
{
	LogUnimplemented("Canvas.DrawCircle() [U227]");
}

void NCanvas::DrawBox_U227(UObject* Self, Color& Col, int LineFlags, vec3& Start, vec3& End)
{
	LogUnimplemented("Canvas.DrawBox() [U227]");
}

void NCanvas::GetCameraCoords_U227(UObject* Self, Coords& ReturnValue)
{
	LogUnimplemented("Canvas.GetCameraCoords() [U227]");
	ReturnValue = Coords();
}

void NCanvas::SetTile3DOffset_U227(UObject* Self, BitfieldBool& bEnable, std::optional<vec3> Offset, std::optional<Rotator> RotOffset, std::optional<bool> bFlatZ, std::optional<float> Scale, std::optional<bool> bWorldOffset)
{
	LogUnimplemented("Canvas.SetTile3DOffset() [U227]");
}

void NCanvas::DrawRotatedTile_U227(UObject* Self, UObject*& Tex, float Roll, float XL, float YL, float U, float V, float UL, float VL, float PivotX, float PivotY, std::optional<bool> bRotateUV, std::optional<int> PolyFlags)
{
	LogUnimplemented("Canvas.DrawRotatedTile() [U227]");
}

void NCanvas::PushClipPlane_U227(UObject* Self, vec3& Dir, float W, std::optional<bool> bLocal, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Canvas.PushClipPlane() [U227]");
}

void NCanvas::PopClipPlane_U227(UObject* Self)
{
	LogUnimplemented("Canvas.PopClipPlane() [U227]");
}

void NCanvas::SetZTest_U227(UObject* Self, uint8_t& ZTest, uint8_t& ReturnValue)
{
	LogUnimplemented("Canvas.SetZTest() [U227]");
	ReturnValue = static_cast<uint8_t>(ERenderZTest::ZTEST_NotEqual);
}

void NCanvas::PushCanvasScale_U227(UObject* Self, float Scale, std::optional<bool> bAbsolute)
{
	LogUnimplemented("Canvas.PushCanvasScale() [U227]");
}

void NCanvas::PopCanvasScale_U227(UObject* Self)
{
	LogUnimplemented("Canvas.PopCanvasScale() [U227]");
}

void NCanvas::SetCustomLighting_U227(UObject* Self, std::optional<bool> bDisableEngineLighting, std::optional<vec3> customAmbientLight)
{
	LogUnimplemented("Canvas.SetCustomLighting() [U227]");
}

void NCanvas::AddCustomLightSource_U227(UObject* Self, vec3& Pos, std::optional<Rotator> Dir, std::optional<bool> bDefault)
{
	LogUnimplemented("Canvas.AddCustomLightSource() [U227]");
}

void NCanvas::ClearCustomLightSources_U227(UObject* Self)
{
	LogUnimplemented("Canvas.ClearCustomLightSources() [U227]");
}

void NCanvas::Reset_U227(UObject* Self)
{
	LogUnimplemented("Canvas.Reset() [U227]");
}
