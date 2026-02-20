#include "Precomp.h"
#include "NGC.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
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
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->ClearZ();
}

void NGC::CopyGC(UObject* Self, UObject* Copy)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->CopyGC(Copy);
}

void NGC::DrawActor(UObject* Self, UObject* Actor, BitfieldBool* bClearZ, BitfieldBool* bConstrain, BitfieldBool* bUnlit, float* DrawScale, float* ScaleGlow, UObject** Skin)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->DrawActor(Actor, bClearZ, bConstrain, bUnlit, DrawScale, ScaleGlow, Skin);
}

void NGC::DrawBorders(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float leftMargin, float rightMargin, float TopMargin, float BottomMargin, UObject* borders, BitfieldBool* bStretchHorizontally, BitfieldBool* bStretchVertically)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->DrawBorders(DestX, DestY, destWidth, destHeight, leftMargin, rightMargin, TopMargin, BottomMargin, borders, bStretchHorizontally, bStretchVertically);
}

void NGC::DrawBox(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, float boxThickness, UObject* tX)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->DrawBox(DestX, DestY, destWidth, destHeight, OrgX, OrgY, boxThickness, tX);
}

void NGC::DrawIcon(UObject* Self, float DestX, float DestY, UObject* tX)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->DrawIcon(DestX, DestY, tX);
}

void NGC::DrawPattern(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float OrgX, float OrgY, UObject* tX)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->DrawPattern(DestX, DestY, destWidth, destHeight, OrgX, OrgY, tX);
}

void NGC::DrawStretchedTexture(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, float srcWidth, float srcHeight, UObject* tX)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->DrawStretchedTexture(DestX, DestY, destWidth, destHeight, srcX, srcY, srcWidth, srcHeight, tX);
}

void NGC::DrawText(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, const std::string& textStr)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->DrawText(DestX, DestY, destWidth, destHeight, textStr);
}

void NGC::DrawTexture(UObject* Self, float DestX, float DestY, float destWidth, float destHeight, float srcX, float srcY, UObject* tX)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->DrawTexture(DestX, DestY, destWidth, destHeight, srcX, srcY, tX);
}

void NGC::EnableDrawing(UObject* Self, bool bDrawEnabled)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->EnableDrawing(bDrawEnabled);
}

void NGC::EnableMasking(UObject* Self, bool bNewMasking)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->EnableMasking(bNewMasking);
}

void NGC::EnableModulation(UObject* Self, bool bNewModulation)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->EnableModulation(bNewModulation);
}

void NGC::EnableSmoothing(UObject* Self, bool bNewSmoothing)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->EnableSmoothing(bNewSmoothing);
}

void NGC::EnableSpecialText(UObject* Self, bool bNewSpecialText)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->EnableSpecialText(bNewSpecialText);
}

void NGC::EnableTranslucency(UObject* Self, bool bNewTranslucency)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->EnableTranslucency(bNewTranslucency);
}

void NGC::EnableTranslucentText(UObject* Self, bool bNewTranslucency)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->EnableTranslucentText(bNewTranslucency);
}

void NGC::EnableWordWrap(UObject* Self, bool bNewWordWrap)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->EnableWordWrap(bNewWordWrap);
}

void NGC::GetAlignments(UObject* Self, uint8_t& HAlign, uint8_t& VAlign)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->GetAlignments(HAlign, VAlign);
}

void NGC::GetFontHeight(UObject* Self, BitfieldBool* bIncludeSpace, float& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->GetFontHeight(bIncludeSpace);
}

void NGC::GetFonts(UObject* Self, UObject*& normalFont, UObject*& boldFont)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->GetFonts(normalFont, boldFont);
}

void NGC::GetHorizontalAlignment(UObject* Self, uint8_t& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->GetHorizontalAlignment();
}

void NGC::GetStyle(UObject* Self, uint8_t& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->GetStyle();
}

void NGC::GetTextColor(UObject* Self, Color& TextColor)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->GetTextColor(TextColor);
}

void NGC::GetTextExtent(UObject* Self, float destWidth, float& xExtent, float& yExtent, const std::string& textStr)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->GetTextExtent(destWidth, xExtent, yExtent, textStr);
}

void NGC::GetTextVSpacing(UObject* Self, float& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->GetTextVSpacing();
}

void NGC::GetTileColor(UObject* Self, Color& tileColor)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->GetTileColor(tileColor);
}

void NGC::GetVerticalAlignment(UObject* Self, uint8_t& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->GetVerticalAlignment();
}

void NGC::Intersect(UObject* Self, float ClipX, float ClipY, float clipWidth, float clipHeight)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->Intersect(ClipX, ClipY, clipWidth, clipHeight);
}

void NGC::IsDrawingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->IsDrawingEnabled();
}

void NGC::IsMaskingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->IsMaskingEnabled();
}

void NGC::IsModulationEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->IsModulationEnabled();
}

void NGC::IsSmoothingEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->IsSmoothingEnabled();
}

void NGC::IsSpecialTextEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->IsSpecialTextEnabled();
}

void NGC::IsTranslucencyEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->IsTranslucencyEnabled();
}

void NGC::IsTranslucentTextEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->IsTranslucentTextEnabled();
}

void NGC::IsWordWrapEnabled(UObject* Self, BitfieldBool& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->IsWordWrapEnabled();
}

void NGC::PopGC(UObject* Self, int* gcNum)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->PopGC(gcNum);
}

void NGC::PushGC(UObject* Self, int& ReturnValue)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	ReturnValue = gc->PushGC();
}

void NGC::SetAlignments(UObject* Self, uint8_t newHAlign, uint8_t newVAlign)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetAlignments(newHAlign, newVAlign);
}

void NGC::SetBaselineData(UObject* Self, float* newBaselineOffset, float* newUnderlineHeight)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetBaselineData(newBaselineOffset, newUnderlineHeight);
}

void NGC::SetBoldFont(UObject* Self, UObject* newBoldFont)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetBoldFont(newBoldFont);
}

void NGC::SetFont(UObject* Self, UObject* NewFont)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetFont(NewFont);
}

void NGC::SetFonts(UObject* Self, UObject* newNormalFont, UObject* newBoldFont)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetFonts(newNormalFont, newBoldFont);
}

void NGC::SetHorizontalAlignment(UObject* Self, uint8_t newHAlign)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetHorizontalAlignment(newHAlign);
}

void NGC::SetNormalFont(UObject* Self, UObject* newNormalFont)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetNormalFont(newNormalFont);
}

void NGC::SetStyle(UObject* Self, uint8_t NewStyle)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetStyle(NewStyle);
}

void NGC::SetTextColor(UObject* Self, const Color& newTextColor)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetTextColor(newTextColor);
}

void NGC::SetTextVSpacing(UObject* Self, float newVSpacing)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetTextVSpacing(newVSpacing);
}

void NGC::SetTileColor(UObject* Self, const Color& newTileColor)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetTileColor(newTileColor);
}

void NGC::SetVerticalAlignment(UObject* Self, uint8_t newVAlign)
{
	UGC* gc = UObject::Cast<UGC>(Self);
	gc->SetVerticalAlignment(newVAlign);
}
