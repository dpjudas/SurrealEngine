#pragma once

#include "Packages/Core/UObject.h"

class UFont;
class UViewport;

// Unreal 227
enum class ERenderZTest : uint8_t
{
	ZTEST_Less,
	ZTEST_Equal,
	ZTEST_LessEqual,
	ZTest_Greater,
	ZTEST_GreaterEqual,
	ZTEST_NotEqual,
	ZTEST_Always
};

struct CanvasPoly_U227
{
	vec3 Points[3];
	vec2 UV[3];
	vec4 Colors[3], Fog[3];
};

class UCanvas : public UObject
{
public:
	using UObject::UObject;

	UFont*& BigFont() { return Value<UFont*>(PropOffsets_Canvas.BigFont); }
	float& ClipX() { return Value<float>(PropOffsets_Canvas.ClipX); }
	float& ClipY() { return Value<float>(PropOffsets_Canvas.ClipY); }
	float& CurX() { return Value<float>(PropOffsets_Canvas.CurX); }
	float& CurY() { return Value<float>(PropOffsets_Canvas.CurY); }
	float& CurYL() { return Value<float>(PropOffsets_Canvas.CurYL); }
	Color& DrawColor() { return Value<Color>(PropOffsets_Canvas.DrawColor); }
	UFont*& Font() { return Value<UFont*>(PropOffsets_Canvas.Font); }
	int& FramePtr() { return Value<int>(PropOffsets_Canvas.FramePtr); }
	UFont*& LargeFont() { return Value<UFont*>(PropOffsets_Canvas.LargeFont); }
	UFont*& MedFont() { return Value<UFont*>(PropOffsets_Canvas.MedFont); }
	float& OrgX() { return Value<float>(PropOffsets_Canvas.OrgX); }
	float& OrgY() { return Value<float>(PropOffsets_Canvas.OrgY); }
	int& RenderPtr() { return Value<int>(PropOffsets_Canvas.RenderPtr); }
	int& SizeX() { return Value<int>(PropOffsets_Canvas.SizeX); }
	int& SizeY() { return Value<int>(PropOffsets_Canvas.SizeY); }
	UFont*& SmallFont() { return Value<UFont*>(PropOffsets_Canvas.SmallFont); }
	float& SpaceX() { return Value<float>(PropOffsets_Canvas.SpaceX); }
	float& SpaceY() { return Value<float>(PropOffsets_Canvas.SpaceY); }
	uint8_t& Style() { return Value<uint8_t>(PropOffsets_Canvas.Style); }
	UViewport*& Viewport() { return Value<UViewport*>(PropOffsets_Canvas.Viewport); }
	float& Z() { return Value<float>(PropOffsets_Canvas.Z); }
	BitfieldBool bCenter() { return BoolValue(PropOffsets_Canvas.bCenter); }
	BitfieldBool bNoSmooth() { return BoolValue(PropOffsets_Canvas.bNoSmooth); }
};
