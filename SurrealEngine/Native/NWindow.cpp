#include "Precomp.h"
#include "NWindow.h"
#include "VM/NativeFunc.h"
#include "Engine.h"

void NWindow::RegisterFunctions()
{
	RegisterVMNativeFunc_1("Window", "AddActorRef", &NWindow::AddActorRef, 1497);
	RegisterVMNativeFunc_5("Window", "AddTimer", &NWindow::AddTimer, 1490);
	RegisterVMNativeFunc_0("Window", "AskParentForReconfigure", &NWindow::AskParentForReconfigure, 1460);
	RegisterVMNativeFunc_4("Window", "AskParentToShowArea", &NWindow::AskParentToShowArea, 1468);
	RegisterVMNativeFunc_1("Window", "CarriageReturn", &NWindow::CarriageReturn, 1482);
	RegisterVMNativeFunc_0("Window", "ChangeStyle", &NWindow::ChangeStyle, 1493);
	RegisterVMNativeFunc_4("Window", "ConfigureChild", &NWindow::ConfigureChild, 1461);
	RegisterVMNativeFunc_6("Window", "ConvertCoordinates", &NWindow::ConvertCoordinates, 1449);
	RegisterVMNativeFunc_2("Window", "ConvertScriptString", &NWindow::ConvertScriptString, 1469);
	RegisterVMNativeFunc_4("Window", "ConvertVectorToCoordinates", &NWindow::ConvertVectorToCoordinates, 1489);
	RegisterVMNativeFunc_0("Window", "Destroy", &NWindow::Destroy, 1409);
	RegisterVMNativeFunc_0("Window", "DestroyAllChildren", &NWindow::DestroyAllChildren, 1459);
	RegisterVMNativeFunc_0("Window", "DisableWindow", &NWindow::DisableWindow, 1418);
	RegisterVMNativeFunc_1("Window", "EnableSpecialText", &NWindow::EnableSpecialText, 1481);
	RegisterVMNativeFunc_1("Window", "EnableTranslucentText", &NWindow::EnableTranslucentText, 1483);
	RegisterVMNativeFunc_1("Window", "EnableWindow", &NWindow::EnableWindow, 1417);
	RegisterVMNativeFunc_5("Window", "FindWindow", &NWindow::FindWindow, 1472);
	RegisterVMNativeFunc_2("Window", "GetBottomChild", &NWindow::GetBottomChild, 1456);
	RegisterVMNativeFunc_1("Window", "GetClientObject", &NWindow::GetClientObject, 1488);
	RegisterVMNativeFunc_2("Window", "GetCursorPos", &NWindow::GetCursorPos, 1452);
	RegisterVMNativeFunc_1("Window", "GetFocusWindow", &NWindow::GetFocusWindow, 1441);
	RegisterVMNativeFunc_1("Window", "GetGC", &NWindow::GetGC, 1485);
	RegisterVMNativeFunc_2("Window", "GetHigherSibling", &NWindow::GetHigherSibling, 1457);
	RegisterVMNativeFunc_2("Window", "GetLowerSibling", &NWindow::GetLowerSibling, 1458);
	RegisterVMNativeFunc_1("Window", "GetModalWindow", &NWindow::GetModalWindow, 1426);
	RegisterVMNativeFunc_1("Window", "GetParent", &NWindow::GetParent, 1428);
	RegisterVMNativeFunc_1("Window", "GetPlayerPawn", &NWindow::GetPlayerPawn, 1429);
	RegisterVMNativeFunc_1("Window", "GetRootWindow", &NWindow::GetRootWindow, 1425);
	RegisterVMNativeFunc_1("Window", "GetTabGroupWindow", &NWindow::GetTabGroupWindow, 1427);
	RegisterVMNativeFunc_1("Window", "GetTickOffset", &NWindow::GetTickOffset, 1492);
	RegisterVMNativeFunc_2("Window", "GetTopChild", &NWindow::GetTopChild, 1455);
	RegisterVMNativeFunc_0("Window", "GrabMouse", &NWindow::GrabMouse, 1450);
	RegisterVMNativeFunc_0("Window", "Hide", &NWindow::Hide, 1414);
	RegisterVMNativeFunc_2("Window", "IsActorValid", &NWindow::IsActorValid, 1499);
	RegisterVMNativeFunc_1("Window", "IsFocusWindow", &NWindow::IsFocusWindow, 1448);
	RegisterVMNativeFunc_2("Window", "IsKeyDown", &NWindow::IsKeyDown, 1470);
	RegisterVMNativeFunc_3("Window", "IsPointInWindow", &NWindow::IsPointInWindow, 1471);
	RegisterVMNativeFunc_2("Window", "IsSensitive", &NWindow::IsSensitive, 1419);
	RegisterVMNativeFunc_2("Window", "IsVisible", &NWindow::IsVisible, 1415);
	RegisterVMNativeFunc_0("Window", "Lower", &NWindow::Lower, 1412);
	RegisterVMNativeFunc_1("Window", "MoveFocusDown", &NWindow::MoveFocusDown, 1445);
	RegisterVMNativeFunc_1("Window", "MoveFocusLeft", &NWindow::MoveFocusLeft, 1442);
	RegisterVMNativeFunc_1("Window", "MoveFocusRight", &NWindow::MoveFocusRight, 1443);
	RegisterVMNativeFunc_1("Window", "MoveFocusUp", &NWindow::MoveFocusUp, 1444);
	RegisterVMNativeFunc_1("Window", "MoveTabGroupNext", &NWindow::MoveTabGroupNext, 1446);
	RegisterVMNativeFunc_1("Window", "MoveTabGroupPrev", &NWindow::MoveTabGroupPrev, 1447);
	RegisterVMNativeFunc_3("Window", "NewChild", &NWindow::NewChild, 1410);
	RegisterVMNativeFunc_5("Window", "PlaySound", &NWindow::PlaySound, 1473);
	RegisterVMNativeFunc_2("Window", "QueryGranularity", &NWindow::QueryGranularity, 1466);
	RegisterVMNativeFunc_2("Window", "QueryPreferredHeight", &NWindow::QueryPreferredHeight, 1464);
	RegisterVMNativeFunc_2("Window", "QueryPreferredSize", &NWindow::QueryPreferredSize, 1465);
	RegisterVMNativeFunc_2("Window", "QueryPreferredWidth", &NWindow::QueryPreferredWidth, 1463);
	RegisterVMNativeFunc_0("Window", "Raise", &NWindow::Raise, 1411);
	RegisterVMNativeFunc_1("Window", "ReleaseGC", &NWindow::ReleaseGC, 1486);
	RegisterVMNativeFunc_1("Window", "RemoveActorRef", &NWindow::RemoveActorRef, 1498);
	RegisterVMNativeFunc_1("Window", "RemoveTimer", &NWindow::RemoveTimer, 1491);
	RegisterVMNativeFunc_0("Window", "ResetHeight", &NWindow::ResetHeight, 1437);
	RegisterVMNativeFunc_0("Window", "ResetSize", &NWindow::ResetSize, 1435);
	RegisterVMNativeFunc_0("Window", "ResetWidth", &NWindow::ResetWidth, 1436);
	RegisterVMNativeFunc_0("Window", "ResizeChild", &NWindow::ResizeChild, 1462);
	RegisterVMNativeFunc_1("Window", "SetAcceleratorText", &NWindow::SetAcceleratorText, 1439);
	RegisterVMNativeFunc_1("Window", "SetBackground", &NWindow::SetBackground, 1421);
	RegisterVMNativeFunc_1("Window", "SetBackgroundSmoothing", &NWindow::SetBackgroundSmoothing, 1423);
	RegisterVMNativeFunc_1("Window", "SetBackgroundStretching", &NWindow::SetBackgroundStretching, 1424);
	RegisterVMNativeFunc_1("Window", "SetBackgroundStyle", &NWindow::SetBackgroundStyle, 1422);
	RegisterVMNativeFunc_2("Window", "SetBaselineData", &NWindow::SetBaselineData, 1484);
	RegisterVMNativeFunc_1("Window", "SetBoldFont", &NWindow::SetBoldFont, 1480);
	RegisterVMNativeFunc_1("Window", "SetChildVisibility", &NWindow::SetChildVisibility, 1467);
	RegisterVMNativeFunc_1("Window", "SetClientObject", &NWindow::SetClientObject, 1487);
	RegisterVMNativeFunc_4("Window", "SetConfiguration", &NWindow::SetConfiguration, 1430);
	RegisterVMNativeFunc_2("Window", "SetCursorPos", &NWindow::SetCursorPos, 1453);
	RegisterVMNativeFunc_5("Window", "SetDefaultCursor", &NWindow::SetDefaultCursor, 1454);
	RegisterVMNativeFunc_2("Window", "SetFocusSounds", &NWindow::SetFocusSounds, 1495);
	RegisterVMNativeFunc_2("Window", "SetFocusWindow", &NWindow::SetFocusWindow, 1440);
	RegisterVMNativeFunc_1("Window", "SetFont", &NWindow::SetFont, 1477);
	RegisterVMNativeFunc_2("Window", "SetFonts", &NWindow::SetFonts, 1478);
	RegisterVMNativeFunc_1("Window", "SetHeight", &NWindow::SetHeight, 1434);
	RegisterVMNativeFunc_1("Window", "SetNormalFont", &NWindow::SetNormalFont, 1479);
	RegisterVMNativeFunc_2("Window", "SetPos", &NWindow::SetPos, 1432);
	RegisterVMNativeFunc_1("Window", "SetSelectability", &NWindow::SetSelectability, 1420);
	RegisterVMNativeFunc_1("Window", "SetSensitivity", &NWindow::SetSensitivity, 1416);
	RegisterVMNativeFunc_2("Window", "SetSize", &NWindow::SetSize, 1431);
	RegisterVMNativeFunc_1("Window", "SetSoundVolume", &NWindow::SetSoundVolume, 1474);
	RegisterVMNativeFunc_1("Window", "SetTextColor", &NWindow::SetTextColor, 1476);
	RegisterVMNativeFunc_1("Window", "SetTileColor", &NWindow::SetTileColor, 1475);
	RegisterVMNativeFunc_2("Window", "SetVisibilitySounds", &NWindow::SetVisibilitySounds, 1496);
	RegisterVMNativeFunc_1("Window", "SetWidth", &NWindow::SetWidth, 1433);
	RegisterVMNativeFunc_6("Window", "SetWindowAlignments", &NWindow::SetWindowAlignments, 1438);
	RegisterVMNativeFunc_1("Window", "Show", &NWindow::Show, 1413);
	RegisterVMNativeFunc_0("Window", "UngrabMouse", &NWindow::UngrabMouse, 1451);
}

void NWindow::AddActorRef(UObject* Self, UObject* refActor)
{
	LogUnimplemented("Window.AddActorRef");
}

void NWindow::AddTimer(UObject* Self, float TimeOut, BitfieldBool* bLoop, int* clientData, NameString* functionName, int& ReturnValue)
{
	LogUnimplemented("Window.AddTimer");
	ReturnValue = 0;
}

void NWindow::AskParentForReconfigure(UObject* Self)
{
	LogUnimplemented("Window.AskParentForReconfigure");
}

void NWindow::AskParentToShowArea(UObject* Self, float* areaX, float* areaY, float* areaWidth, float* areaHeight)
{
	LogUnimplemented("Window.AskParentToShowArea");
}

void NWindow::CarriageReturn(UObject* Self, std::string& ReturnValue)
{
	LogUnimplemented("Window.CarriageReturn");
	ReturnValue = "";
}

void NWindow::ChangeStyle(UObject* Self)
{
	LogUnimplemented("Window.ChangeStyle");
}

void NWindow::ConfigureChild(UObject* Self, float newX, float newY, float newWidth, float NewHeight)
{
	LogUnimplemented("Window.ConfigureChild");
}

void NWindow::ConvertCoordinates(UObject* Self, UObject* fromWin, float fromX, float fromY, UObject* toWin, float& toX, float& toY)
{
	LogUnimplemented("Window.ConvertCoordinates");
}

void NWindow::ConvertScriptString(UObject* Self, const std::string& oldStr, std::string& ReturnValue)
{
	LogUnimplemented("Window.ConvertScriptString");
	ReturnValue = "";
}

void NWindow::ConvertVectorToCoordinates(UObject* Self, const vec3& Location, float& relativeX, float& relativeY, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Window.ConvertVectorToCoordinates");
	ReturnValue = false;
}

void NWindow::Destroy(UObject* Self)
{
	LogUnimplemented("Window.Destroy");
}

void NWindow::DestroyAllChildren(UObject* Self)
{
	LogUnimplemented("Window.DestroyAllChildren");
}

void NWindow::DisableWindow(UObject* Self)
{
	LogUnimplemented("Window.DisableWindow");
}

void NWindow::EnableSpecialText(UObject* Self, BitfieldBool* bEnable)
{
	LogUnimplemented("Window.EnableSpecialText");
}

void NWindow::EnableTranslucentText(UObject* Self, BitfieldBool* bEnable)
{
	LogUnimplemented("Window.EnableTranslucentText");
}

void NWindow::EnableWindow(UObject* Self, BitfieldBool* bEnable)
{
	LogUnimplemented("Window.EnableWindow");
}

void NWindow::FindWindow(UObject* Self, float pointX, float pointY, float& relativeX, float& relativeY, UObject*& ReturnValue)
{
	LogUnimplemented("Window.FindWindow");
	ReturnValue = nullptr;
}

void NWindow::GetBottomChild(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetBottomChild");
	ReturnValue = nullptr;
}

void NWindow::GetClientObject(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetClientObject");
	ReturnValue = nullptr;
}

void NWindow::GetCursorPos(UObject* Self, float& MouseX, float& MouseY)
{
	LogUnimplemented("Window.GetCursorPos");
}

void NWindow::GetFocusWindow(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetFocusWindow");
	ReturnValue = nullptr;
}

void NWindow::GetGC(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetGC");
	ReturnValue = nullptr;
}

void NWindow::GetHigherSibling(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetHigherSibling");
	ReturnValue = nullptr;
}

void NWindow::GetLowerSibling(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetLowerSibling");
	ReturnValue = nullptr;
}

void NWindow::GetModalWindow(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetModalWindow");
	ReturnValue = nullptr;
}

void NWindow::GetParent(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetParent");
	ReturnValue = nullptr;
}

void NWindow::GetPlayerPawn(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetPlayerPawn");
	ReturnValue = nullptr;
}

void NWindow::GetRootWindow(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetRootWindow");
	ReturnValue = nullptr;
}

void NWindow::GetTabGroupWindow(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetTabGroupWindow");
	ReturnValue = nullptr;
}

void NWindow::GetTickOffset(UObject* Self, float& ReturnValue)
{
	LogUnimplemented("Window.GetTickOffset");
	ReturnValue = 0.0f;
}

void NWindow::GetTopChild(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	LogUnimplemented("Window.GetTopChild");
	ReturnValue = nullptr;
}

void NWindow::GrabMouse(UObject* Self)
{
	LogUnimplemented("Window.GrabMouse");
}

void NWindow::Hide(UObject* Self)
{
	LogUnimplemented("Window.Hide");
}

void NWindow::IsActorValid(UObject* Self, UObject* refActor, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Window.IsActorValid");
	ReturnValue = false;
}

void NWindow::IsFocusWindow(UObject* Self, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Window.IsFocusWindow");
	ReturnValue = false;
}

void NWindow::IsKeyDown(UObject* Self, uint8_t Key, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Window.IsKeyDown");
	ReturnValue = false;
}

void NWindow::IsPointInWindow(UObject* Self, float pointX, float pointY, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Window.IsPointInWindow");
	ReturnValue = false;
}

void NWindow::IsSensitive(UObject* Self, BitfieldBool* bRecurse, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Window.IsSensitive");
	ReturnValue = false;
}

void NWindow::IsVisible(UObject* Self, BitfieldBool* bRecurse, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Window.IsVisible");
	ReturnValue = false;
}

void NWindow::Lower(UObject* Self)
{
	LogUnimplemented("Window.Lower");
}

void NWindow::MoveFocusDown(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.MoveFocusDown");
	ReturnValue = nullptr;
}

void NWindow::MoveFocusLeft(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.MoveFocusLeft");
	ReturnValue = nullptr;
}

void NWindow::MoveFocusRight(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.MoveFocusRight");
	ReturnValue = nullptr;
}

void NWindow::MoveFocusUp(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.MoveFocusUp");
	ReturnValue = nullptr;
}

void NWindow::MoveTabGroupNext(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.MoveTabGroupNext");
	ReturnValue = nullptr;
}

void NWindow::MoveTabGroupPrev(UObject* Self, UObject*& ReturnValue)
{
	LogUnimplemented("Window.MoveTabGroupPrev");
	ReturnValue = nullptr;
}

void NWindow::NewChild(UObject* Self, UObject* NewClass, BitfieldBool* bShow, UObject*& ReturnValue)
{
	LogUnimplemented("Window.NewChild");
	ReturnValue = nullptr;
}

void NWindow::PlaySound(UObject* Self, UObject* newsound, float* Volume, float* Pitch, float* posX, float* posY)
{
	LogUnimplemented("Window.PlaySound");
}

void NWindow::QueryGranularity(UObject* Self, float& hGranularity, float& vGranularity)
{
	LogUnimplemented("Window.QueryGranularity");
}

void NWindow::QueryPreferredHeight(UObject* Self, float queryWidth, float& ReturnValue)
{
	LogUnimplemented("Window.QueryPreferredHeight");
	ReturnValue = 0.0f;
}

void NWindow::QueryPreferredSize(UObject* Self, float& preferredWidth, float& preferredHeight)
{
	LogUnimplemented("Window.QueryPreferredSize");
}

void NWindow::QueryPreferredWidth(UObject* Self, float queryHeight, float& ReturnValue)
{
	LogUnimplemented("Window.QueryPreferredWidth");
	ReturnValue = 0.0f;
}

void NWindow::Raise(UObject* Self)
{
	LogUnimplemented("Window.Raise");
}

void NWindow::ReleaseGC(UObject* Self, UObject* GC)
{
	LogUnimplemented("Window.ReleaseGC");
}

void NWindow::RemoveActorRef(UObject* Self, UObject* refActor)
{
	LogUnimplemented("Window.RemoveActorRef");
}

void NWindow::RemoveTimer(UObject* Self, int timerId)
{
	LogUnimplemented("Window.RemoveTimer");
}

void NWindow::ResetHeight(UObject* Self)
{
	LogUnimplemented("Window.ResetHeight");
}

void NWindow::ResetSize(UObject* Self)
{
	LogUnimplemented("Window.ResetSize");
}

void NWindow::ResetWidth(UObject* Self)
{
	LogUnimplemented("Window.ResetWidth");
}

void NWindow::ResizeChild(UObject* Self)
{
	LogUnimplemented("Window.ResizeChild");
}

void NWindow::SetAcceleratorText(UObject* Self, const std::string& newStr)
{
	LogUnimplemented("Window.SetAcceleratorText");
}

void NWindow::SetBackground(UObject* Self, UObject* newBackground)
{
	LogUnimplemented("Window.SetBackground");
}

void NWindow::SetBackgroundSmoothing(UObject* Self, bool newSmoothing)
{
	LogUnimplemented("Window.SetBackgroundSmoothing");
}

void NWindow::SetBackgroundStretching(UObject* Self, bool newStretching)
{
	LogUnimplemented("Window.SetBackgroundStretching");
}

void NWindow::SetBackgroundStyle(UObject* Self, uint8_t NewStyle)
{
	LogUnimplemented("Window.SetBackgroundStyle");
}

void NWindow::SetBaselineData(UObject* Self, float* newBaselineOffset, float* newUnderlineHeight)
{
	LogUnimplemented("Window.SetBaselineData");
}

void NWindow::SetBoldFont(UObject* Self, UObject* fn)
{
	LogUnimplemented("Window.SetBoldFont");
}

void NWindow::SetChildVisibility(UObject* Self, bool bNewVisibility)
{
	LogUnimplemented("Window.SetChildVisibility");
}

void NWindow::SetClientObject(UObject* Self, UObject* newClientObject)
{
	LogUnimplemented("Window.SetClientObject");
}

void NWindow::SetConfiguration(UObject* Self, float newX, float newY, float newWidth, float NewHeight)
{
	LogUnimplemented("Window.SetConfiguration");
}

void NWindow::SetCursorPos(UObject* Self, float newMouseX, float newMouseY)
{
	LogUnimplemented("Window.SetCursorPos");
}

void NWindow::SetDefaultCursor(UObject* Self, UObject* tX, UObject** shadowTexture, float* HotX, float* HotY, Color* cursorColor)
{
	LogUnimplemented("Window.SetDefaultCursor");
}

void NWindow::SetFocusSounds(UObject* Self, UObject** focusSound, UObject** unfocusSound)
{
	LogUnimplemented("Window.SetFocusSounds");
}

void NWindow::SetFocusWindow(UObject* Self, UObject* NewFocusWindow, BitfieldBool& ReturnValue)
{
	LogUnimplemented("Window.SetFocusWindow");
	ReturnValue = false;
}

void NWindow::SetFont(UObject* Self, UObject* fn)
{
	LogUnimplemented("Window.SetFont");
}

void NWindow::SetFonts(UObject* Self, UObject* nFont, UObject* bFont)
{
	LogUnimplemented("Window.SetFonts");
}

void NWindow::SetHeight(UObject* Self, float NewHeight)
{
	LogUnimplemented("Window.SetHeight");
}

void NWindow::SetNormalFont(UObject* Self, UObject* fn)
{
	LogUnimplemented("Window.SetNormalFont");
}

void NWindow::SetPos(UObject* Self, float newX, float newY)
{
	LogUnimplemented("Window.SetPos");
}

void NWindow::SetSelectability(UObject* Self, bool newSelectability)
{
	LogUnimplemented("Window.SetSelectability");
}

void NWindow::SetSensitivity(UObject* Self, bool newSensitivity)
{
	LogUnimplemented("Window.SetSensitivity");
}

void NWindow::SetSize(UObject* Self, float newWidth, float NewHeight)
{
	LogUnimplemented("Window.SetSize");
}

void NWindow::SetSoundVolume(UObject* Self, float newVolume)
{
	LogUnimplemented("Window.SetSoundVolume");
}

void NWindow::SetTextColor(UObject* Self, const Color& NewColor)
{
	LogUnimplemented("Window.SetTextColor");
}

void NWindow::SetTileColor(UObject* Self, const Color& NewColor)
{
	LogUnimplemented("Window.SetTileColor");
}

void NWindow::SetVisibilitySounds(UObject* Self, UObject** visSound, UObject** invisSound)
{
	LogUnimplemented("Window.SetVisibilitySounds");
}

void NWindow::SetWidth(UObject* Self, float newWidth)
{
	LogUnimplemented("Window.SetWidth");
}

void NWindow::SetWindowAlignments(UObject* Self, uint8_t HAlign, uint8_t VAlign, float* hMargin0, float* vMargin0, float* hMargin1, float* vMargin1)
{
	LogUnimplemented("Window.SetWindowAlignments");
}

void NWindow::Show(UObject* Self, BitfieldBool* bShow)
{
	LogUnimplemented("Window.Show");
}

void NWindow::UngrabMouse(UObject* Self)
{
	LogUnimplemented("Window.UngrabMouse");
}
