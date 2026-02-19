#include "Precomp.h"
#include "NGC.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NGC::RegisterFunctions()
{
	RegisterVMNativeFunc_0("GC", "ClearZ", &NGC::ClearZ, 1295);
	RegisterVMNativeFunc_1("GC", "CopyGC", &NGC::CopyGC, 1270);
	RegisterVMNativeFunc_7("GC", "DrawActor", &NGC::DrawActor, 1288);
	RegisterVMNativeFunc_11("GC", "DrawBorders", &NGC::DrawBorders, 1289);
	RegisterVMNativeFunc_8("GC", "DrawBox", &NGC::DrawBox, 1286);
	RegisterVMNativeFunc_3("GC", "DrawIcon", &NGC::DrawIcon, 1283);
	RegisterVMNativeFunc_7("GC", "DrawPattern", &NGC::DrawPattern, 1285);
	RegisterVMNativeFunc_9("GC", "DrawStretchedTexture", &NGC::DrawStretchedTexture, 1287);
	RegisterVMNativeFunc_5("GC", "DrawText", &NGC::DrawText, 1282);
	RegisterVMNativeFunc_7("GC", "DrawTexture", &NGC::DrawTexture, 1284);
	RegisterVMNativeFunc_1("GC", "EnableDrawing", &NGC::EnableDrawing, 1214);
	RegisterVMNativeFunc_1("GC", "EnableMasking", &NGC::EnableMasking, 1216);
	RegisterVMNativeFunc_1("GC", "EnableModulation", &NGC::EnableModulation, 1220);
	RegisterVMNativeFunc_1("GC", "EnableSmoothing", &NGC::EnableSmoothing, 1210);
	RegisterVMNativeFunc_1("GC", "EnableSpecialText", &NGC::EnableSpecialText, 1260);
	RegisterVMNativeFunc_1("GC", "EnableTranslucency", &NGC::EnableTranslucency, 1218);
	RegisterVMNativeFunc_1("GC", "EnableTranslucentText", &NGC::EnableTranslucentText, 1247);
	RegisterVMNativeFunc_1("GC", "EnableWordWrap", &NGC::EnableWordWrap, 1258);
	RegisterVMNativeFunc_2("GC", "GetAlignments", &NGC::GetAlignments, 1257);
	RegisterVMNativeFunc_2("GC", "GetFontHeight", &NGC::GetFontHeight, 1281);
	RegisterVMNativeFunc_2("GC", "GetFonts", &NGC::GetFonts, 1246);
	RegisterVMNativeFunc_1("GC", "GetHorizontalAlignment", &NGC::GetHorizontalAlignment, 1253);
	RegisterVMNativeFunc_1("GC", "GetStyle", &NGC::GetStyle, 1213);
	RegisterVMNativeFunc_1("GC", "GetTextColor", &NGC::GetTextColor, 1241);
	RegisterVMNativeFunc_4("GC", "GetTextExtent", &NGC::GetTextExtent, 1280);
	RegisterVMNativeFunc_1("GC", "GetTextVSpacing", &NGC::GetTextVSpacing, 1251);
	RegisterVMNativeFunc_1("GC", "GetTileColor", &NGC::GetTileColor, 1231);
	RegisterVMNativeFunc_1("GC", "GetVerticalAlignment", &NGC::GetVerticalAlignment, 1255);
	RegisterVMNativeFunc_4("GC", "Intersect", &NGC::Intersect, 1200);
	RegisterVMNativeFunc_1("GC", "IsDrawingEnabled", &NGC::IsDrawingEnabled, 1215);
	RegisterVMNativeFunc_1("GC", "IsMaskingEnabled", &NGC::IsMaskingEnabled, 1217);
	RegisterVMNativeFunc_1("GC", "IsModulationEnabled", &NGC::IsModulationEnabled, 1221);
	RegisterVMNativeFunc_1("GC", "IsSmoothingEnabled", &NGC::IsSmoothingEnabled, 1211);
	RegisterVMNativeFunc_1("GC", "IsSpecialTextEnabled", &NGC::IsSpecialTextEnabled, 1261);
	RegisterVMNativeFunc_1("GC", "IsTranslucencyEnabled", &NGC::IsTranslucencyEnabled, 1219);
	RegisterVMNativeFunc_1("GC", "IsTranslucentTextEnabled", &NGC::IsTranslucentTextEnabled, 1248);
	RegisterVMNativeFunc_1("GC", "IsWordWrapEnabled", &NGC::IsWordWrapEnabled, 1259);
	RegisterVMNativeFunc_1("GC", "PopGC", &NGC::PopGC, 1272);
	RegisterVMNativeFunc_1("GC", "PushGC", &NGC::PushGC, 1271);
	RegisterVMNativeFunc_2("GC", "SetAlignments", &NGC::SetAlignments, 1256);
	RegisterVMNativeFunc_2("GC", "SetBaselineData", &NGC::SetBaselineData, 1262);
	RegisterVMNativeFunc_1("GC", "SetBoldFont", &NGC::SetBoldFont, 1244);
	RegisterVMNativeFunc_1("GC", "SetFont", &NGC::SetFont, 1242);
	RegisterVMNativeFunc_2("GC", "SetFonts", &NGC::SetFonts, 1245);
	RegisterVMNativeFunc_1("GC", "SetHorizontalAlignment", &NGC::SetHorizontalAlignment, 1252);
	RegisterVMNativeFunc_1("GC", "SetNormalFont", &NGC::SetNormalFont, 1243);
	RegisterVMNativeFunc_1("GC", "SetStyle", &NGC::SetStyle, 1212);
	RegisterVMNativeFunc_1("GC", "SetTextColor", &NGC::SetTextColor, 1240);
	RegisterVMNativeFunc_1("GC", "SetTextVSpacing", &NGC::SetTextVSpacing, 1250);
	RegisterVMNativeFunc_1("GC", "SetTileColor", &NGC::SetTileColor, 1230);
	RegisterVMNativeFunc_1("GC", "SetVerticalAlignment", &NGC::SetVerticalAlignment, 1254);
}

void NGC::ClearZ(UObject* Self)
{
	LogUnimplemented("GC.ClearZ");
}

void NGC::CopyGC(UObject* Self, UObject* Copy)
{
	LogUnimplemented("GC.CopyGC");
}

void NGC::DrawActor(UObject* Self, UObject* Actor, BitfieldBool* bClearZ, BitfieldBool* bConstrain, BitfieldBool* bUnlit, float* DrawScale, float* ScaleGlow, UObject** Skin)
{
	LogUnimplemented("GC.DrawActor");
}

void NGC::DrawBorders(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float leftMargin, float rightMargin, float TopMargin, float BottomMargin, UObject* borders, BitfieldBool* bStretchHorizontally, BitfieldBool* bStretchVertically)
{
	LogUnimplemented("GC.DrawBorders");
}

void NGC::DrawBox(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, float boxThickness, UObject* tX)
{
	LogUnimplemented("GC.DrawBox");
}

void NGC::DrawIcon(UObject* Self, float DestX, float DestY, UObject* tX)
{
	LogUnimplemented("GC.DrawIcon");
}

void NGC::DrawPattern(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, UObject* tX)
{
	LogUnimplemented("GC.DrawPattern");
}

void NGC::DrawStretchedTexture(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, float srcWidth, float srcHeight, UObject* tX)
{
	LogUnimplemented("GC.DrawStretchedTexture");
}

void NGC::DrawText(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, const std::string& textStr)
{
	LogUnimplemented("GC.DrawText");
}

void NGC::DrawTexture(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, UObject* tX)
{
	LogUnimplemented("GC.DrawTexture");
}

void NGC::EnableDrawing(UObject* Self, bool bDrawEnabled)
{
	LogUnimplemented("GC.EnableDrawing");
}

void NGC::EnableMasking(UObject* Self, bool bNewMasking)
{
	LogUnimplemented("GC.EnableMasking");
}

void NGC::EnableModulation(UObject* Self, bool bNewModulation)
{
	LogUnimplemented("GC.EnableModulation");
}

void NGC::EnableSmoothing(UObject* Self, bool bNewSmoothing)
{
	LogUnimplemented("GC.EnableSmoothing");
}

void NGC::EnableSpecialText(UObject* Self, bool bNewSpecialText)
{
	LogUnimplemented("GC.EnableSpecialText");
}

void NGC::EnableTranslucency(UObject* Self, bool bNewTranslucency)
{
	LogUnimplemented("GC.EnableTranslucency");
}

void NGC::EnableTranslucentText(UObject* Self, bool bNewTranslucency)
{
	LogUnimplemented("GC.EnableTranslucentText");
}

void NGC::EnableWordWrap(UObject* Self, bool bNewWordWrap)
{
	LogUnimplemented("GC.EnableWordWrap");
}

void NGC::GetAlignments(UObject* Self, uint8_t& HAlign, uint8_t& VAlign)
{
	LogUnimplemented("GC.GetAlignments");
}

void NGC::GetFontHeight(UObject* Self, BitfieldBool* bIncludeSpace, float& ReturnValue)
{
	LogUnimplemented("GC.GetFontHeight");
	ReturnValue = 0.0f;
}

void NGC::GetFonts(UObject* Self, UObject*& normalFont, UObject*& boldFont)
{
	LogUnimplemented("GC.GetFonts");
}

void NGC::GetHorizontalAlignment(UObject* Self, uint8_t& ReturnValue)
{
	LogUnimplemented("GC.GetHorizontalAlignment");
	ReturnValue = 0;
}

void NGC::GetStyle(UObject* Self, uint8_t& ReturnValue)
{
	LogUnimplemented("GC.GetStyle");
	ReturnValue = 0;
}

void NGC::GetTextColor(UObject* Self, Color& TextColor)
{
	LogUnimplemented("GC.GetTextColor");
}

void NGC::GetTextExtent(UObject* Self, float destWidth, float& xExtent, float& yExtent, const std::string& textStr)
{
	LogUnimplemented("GC.GetTextExtent");
}

void NGC::GetTextVSpacing(UObject* Self, float& ReturnValue)
{
	LogUnimplemented("GC.GetTextVSpacing");
	ReturnValue = 0.0f;
}

void NGC::GetTileColor(UObject* Self, Color& tileColor)
{
	LogUnimplemented("GC.GetTileColor");
}

void NGC::GetVerticalAlignment(UObject* Self, uint8_t& ReturnValue)
{
	LogUnimplemented("GC.GetVerticalAlignment");
	ReturnValue = 0;
}

void NGC::Intersect(UObject* Self, float ClipX, float ClipY, float clipWidth, float clipHeight)
{
	LogUnimplemented("GC.Intersect");
}

void NGC::IsDrawingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("GC.IsDrawingEnabled");
	ReturnValue = false;
}

void NGC::IsMaskingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("GC.IsMaskingEnabled");
	ReturnValue = false;
}

void NGC::IsModulationEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("GC.IsModulationEnabled");
	ReturnValue = false;
}

void NGC::IsSmoothingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("GC.IsSmoothingEnabled");
	ReturnValue = false;
}

void NGC::IsSpecialTextEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("GC.IsSpecialTextEnabled");
	ReturnValue = false;
}

void NGC::IsTranslucencyEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("GC.IsTranslucencyEnabled");
	ReturnValue = false;
}

void NGC::IsTranslucentTextEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("GC.IsTranslucentTextEnabled");
	ReturnValue = false;
}

void NGC::IsWordWrapEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("GC.IsWordWrapEnabled");
	ReturnValue = false;
}

void NGC::PopGC(UObject* Self, int* gcNum)
{
	LogUnimplemented("GC.PopGC");
}

void NGC::PushGC(UObject* Self, int& ReturnValue)
{
	LogUnimplemented("GC.PushGC");
	ReturnValue = 0;
}

void NGC::SetAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign)
{
	LogUnimplemented("GC.SetAlignments");
}

void NGC::SetBaselineData(UObject* Self, float* newBaselineOffset, float* newUnderlineHeight)
{
	LogUnimplemented("GC.SetBaselineData");
}

void NGC::SetBoldFont(UObject* Self, UObject* newBoldFont)
{
	LogUnimplemented("GC.SetBoldFont");
}

void NGC::SetFont(UObject* Self, UObject* NewFont)
{
	LogUnimplemented("GC.SetFont");
}

void NGC::SetFonts(UObject* Self, UObject* newNormalFont, UObject* newBoldFont)
{
	LogUnimplemented("GC.SetFonts");
}

void NGC::SetHorizontalAlignment(UObject* Self, uint8_t newHAlign)
{
	LogUnimplemented("GC.SetHorizontalAlignment");
}

void NGC::SetNormalFont(UObject* Self, UObject* newNormalFont)
{
	LogUnimplemented("GC.SetNormalFont");
}

void NGC::SetStyle(UObject* Self, uint8_t NewStyle)
{
	LogUnimplemented("GC.SetStyle");
}

void NGC::SetTextColor(UObject* Self, const Color& newTextColor)
{
	LogUnimplemented("GC.SetTextColor");
}

void NGC::SetTextVSpacing(UObject* Self, float newVSpacing)
{
	LogUnimplemented("GC.SetTextVSpacing");
}

void NGC::SetTileColor(UObject* Self, const Color& newTileColor)
{
	LogUnimplemented("GC.SetTileColor");
}

void NGC::SetVerticalAlignment(UObject* Self, uint8_t newVAlign)
{
	LogUnimplemented("GC.SetVerticalAlignment");
}
