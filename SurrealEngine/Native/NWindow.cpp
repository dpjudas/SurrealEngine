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
	Exception::Throw("NWindow::AddActorRef not implemented");
}

void NWindow::AddTimer(UObject* Self, float TimeOut, BitfieldBool* bLoop, int* clientData, NameString* functionName, int& ReturnValue)
{
	Exception::Throw("NWindow::AddTimer not implemented");
}

void NWindow::AskParentForReconfigure(UObject* Self)
{
	Exception::Throw("NWindow::AskParentForReconfigure not implemented");
}

void NWindow::AskParentToShowArea(UObject* Self, float* areaX, float* areaY, float* areaWidth, float* areaHeight)
{
	Exception::Throw("NWindow::AskParentToShowArea not implemented");
}

void NWindow::CarriageReturn(UObject* Self, std::string& ReturnValue)
{
	Exception::Throw("NWindow::CarriageReturn not implemented");
}

void NWindow::ChangeStyle(UObject* Self)
{
	Exception::Throw("NWindow::ChangeStyle not implemented");
}

void NWindow::ConfigureChild(UObject* Self, float newX, float newY, float newWidth, float NewHeight)
{
	Exception::Throw("NWindow::ConfigureChild not implemented");
}

void NWindow::ConvertCoordinates(UObject* Self, UObject* fromWin, float fromX, float fromY, UObject* toWin, float& toX, float& toY)
{
	Exception::Throw("NWindow::ConvertCoordinates not implemented");
}

void NWindow::ConvertScriptString(UObject* Self, const std::string& oldStr, std::string& ReturnValue)
{
	Exception::Throw("NWindow::ConvertScriptString not implemented");
}

void NWindow::ConvertVectorToCoordinates(UObject* Self, const vec3& Location, float& relativeX, float& relativeY, BitfieldBool& ReturnValue)
{
	Exception::Throw("NWindow::ConvertVectorToCoordinates not implemented");
}

void NWindow::Destroy(UObject* Self)
{
	Exception::Throw("NWindow::Destroy not implemented");
}

void NWindow::DestroyAllChildren(UObject* Self)
{
	Exception::Throw("NWindow::DestroyAllChildren not implemented");
}

void NWindow::DisableWindow(UObject* Self)
{
	Exception::Throw("NWindow::DisableWindow not implemented");
}

void NWindow::EnableSpecialText(UObject* Self, BitfieldBool* bEnable)
{
	Exception::Throw("NWindow::EnableSpecialText not implemented");
}

void NWindow::EnableTranslucentText(UObject* Self, BitfieldBool* bEnable)
{
	Exception::Throw("NWindow::EnableTranslucentText not implemented");
}

void NWindow::EnableWindow(UObject* Self, BitfieldBool* bEnable)
{
	Exception::Throw("NWindow::EnableWindow not implemented");
}

void NWindow::FindWindow(UObject* Self, float pointX, float pointY, float& relativeX, float& relativeY, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::FindWindow not implemented");
}

void NWindow::GetBottomChild(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetBottomChild not implemented");
}

void NWindow::GetClientObject(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetClientObject not implemented");
}

void NWindow::GetCursorPos(UObject* Self, float& MouseX, float& MouseY)
{
	Exception::Throw("NWindow::GetCursorPos not implemented");
}

void NWindow::GetFocusWindow(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetFocusWindow not implemented");
}

void NWindow::GetGC(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetGC not implemented");
}

void NWindow::GetHigherSibling(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetHigherSibling not implemented");
}

void NWindow::GetLowerSibling(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetLowerSibling not implemented");
}

void NWindow::GetModalWindow(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetModalWindow not implemented");
}

void NWindow::GetParent(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetParent not implemented");
}

void NWindow::GetPlayerPawn(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetPlayerPawn not implemented");
}

void NWindow::GetRootWindow(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetRootWindow not implemented");
}

void NWindow::GetTabGroupWindow(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetTabGroupWindow not implemented");
}

void NWindow::GetTickOffset(UObject* Self, float& ReturnValue)
{
	Exception::Throw("NWindow::GetTickOffset not implemented");
}

void NWindow::GetTopChild(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::GetTopChild not implemented");
}

void NWindow::GrabMouse(UObject* Self)
{
	Exception::Throw("NWindow::GrabMouse not implemented");
}

void NWindow::Hide(UObject* Self)
{
	Exception::Throw("NWindow::Hide not implemented");
}

void NWindow::IsActorValid(UObject* Self, UObject* refActor, BitfieldBool& ReturnValue)
{
	Exception::Throw("NWindow::IsActorValid not implemented");
}

void NWindow::IsFocusWindow(UObject* Self, BitfieldBool& ReturnValue)
{
	Exception::Throw("NWindow::IsFocusWindow not implemented");
}

void NWindow::IsKeyDown(UObject* Self, uint8_t Key, BitfieldBool& ReturnValue)
{
	Exception::Throw("NWindow::IsKeyDown not implemented");
}

void NWindow::IsPointInWindow(UObject* Self, float pointX, float pointY, BitfieldBool& ReturnValue)
{
	Exception::Throw("NWindow::IsPointInWindow not implemented");
}

void NWindow::IsSensitive(UObject* Self, BitfieldBool* bRecurse, BitfieldBool& ReturnValue)
{
	Exception::Throw("NWindow::IsSensitive not implemented");
}

void NWindow::IsVisible(UObject* Self, BitfieldBool* bRecurse, BitfieldBool& ReturnValue)
{
	Exception::Throw("NWindow::IsVisible not implemented");
}

void NWindow::Lower(UObject* Self)
{
	Exception::Throw("NWindow::Lower not implemented");
}

void NWindow::MoveFocusDown(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::MoveFocusDown not implemented");
}

void NWindow::MoveFocusLeft(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::MoveFocusLeft not implemented");
}

void NWindow::MoveFocusRight(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::MoveFocusRight not implemented");
}

void NWindow::MoveFocusUp(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::MoveFocusUp not implemented");
}

void NWindow::MoveTabGroupNext(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::MoveTabGroupNext not implemented");
}

void NWindow::MoveTabGroupPrev(UObject* Self, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::MoveTabGroupPrev not implemented");
}

void NWindow::NewChild(UObject* Self, UObject* NewClass, BitfieldBool* bShow, UObject*& ReturnValue)
{
	Exception::Throw("NWindow::NewChild not implemented");
}

void NWindow::PlaySound(UObject* Self, UObject* newsound, float* Volume, float* Pitch, float* posX, float* posY)
{
	Exception::Throw("NWindow::PlaySound not implemented");
}

void NWindow::QueryGranularity(UObject* Self, float& hGranularity, float& vGranularity)
{
	Exception::Throw("NWindow::QueryGranularity not implemented");
}

void NWindow::QueryPreferredHeight(UObject* Self, float queryWidth, float& ReturnValue)
{
	Exception::Throw("NWindow::QueryPreferredHeight not implemented");
}

void NWindow::QueryPreferredSize(UObject* Self, float& preferredWidth, float& preferredHeight)
{
	Exception::Throw("NWindow::QueryPreferredSize not implemented");
}

void NWindow::QueryPreferredWidth(UObject* Self, float queryHeight, float& ReturnValue)
{
	Exception::Throw("NWindow::QueryPreferredWidth not implemented");
}

void NWindow::Raise(UObject* Self)
{
	Exception::Throw("NWindow::Raise not implemented");
}

void NWindow::ReleaseGC(UObject* Self, UObject* GC)
{
	Exception::Throw("NWindow::ReleaseGC not implemented");
}

void NWindow::RemoveActorRef(UObject* Self, UObject* refActor)
{
	Exception::Throw("NWindow::RemoveActorRef not implemented");
}

void NWindow::RemoveTimer(UObject* Self, int timerId)
{
	Exception::Throw("NWindow::RemoveTimer not implemented");
}

void NWindow::ResetHeight(UObject* Self)
{
	Exception::Throw("NWindow::ResetHeight not implemented");
}

void NWindow::ResetSize(UObject* Self)
{
	Exception::Throw("NWindow::ResetSize not implemented");
}

void NWindow::ResetWidth(UObject* Self)
{
	Exception::Throw("NWindow::ResetWidth not implemented");
}

void NWindow::ResizeChild(UObject* Self)
{
	Exception::Throw("NWindow::ResizeChild not implemented");
}

void NWindow::SetAcceleratorText(UObject* Self, const std::string& newStr)
{
	Exception::Throw("NWindow::SetAcceleratorText not implemented");
}

void NWindow::SetBackground(UObject* Self, UObject* newBackground)
{
	Exception::Throw("NWindow::SetBackground not implemented");
}

void NWindow::SetBackgroundSmoothing(UObject* Self, bool newSmoothing)
{
	Exception::Throw("NWindow::SetBackgroundSmoothing not implemented");
}

void NWindow::SetBackgroundStretching(UObject* Self, bool newStretching)
{
	Exception::Throw("NWindow::SetBackgroundStretching not implemented");
}

void NWindow::SetBackgroundStyle(UObject* Self, uint8_t NewStyle)
{
	Exception::Throw("NWindow::SetBackgroundStyle not implemented");
}

void NWindow::SetBaselineData(UObject* Self, float* newBaselineOffset, float* newUnderlineHeight)
{
	Exception::Throw("NWindow::SetBaselineData not implemented");
}

void NWindow::SetBoldFont(UObject* Self, UObject* fn)
{
	Exception::Throw("NWindow::SetBoldFont not implemented");
}

void NWindow::SetChildVisibility(UObject* Self, bool bNewVisibility)
{
	Exception::Throw("NWindow::SetChildVisibility not implemented");
}

void NWindow::SetClientObject(UObject* Self, UObject* newClientObject)
{
	Exception::Throw("NWindow::SetClientObject not implemented");
}

void NWindow::SetConfiguration(UObject* Self, float newX, float newY, float newWidth, float NewHeight)
{
	Exception::Throw("NWindow::SetConfiguration not implemented");
}

void NWindow::SetCursorPos(UObject* Self, float newMouseX, float newMouseY)
{
	Exception::Throw("NWindow::SetCursorPos not implemented");
}

void NWindow::SetDefaultCursor(UObject* Self, UObject* tX, UObject** shadowTexture, float* HotX, float* HotY, Color* cursorColor)
{
	Exception::Throw("NWindow::SetDefaultCursor not implemented");
}

void NWindow::SetFocusSounds(UObject* Self, UObject** focusSound, UObject** unfocusSound)
{
	Exception::Throw("NWindow::SetFocusSounds not implemented");
}

void NWindow::SetFocusWindow(UObject* Self, UObject* NewFocusWindow, BitfieldBool& ReturnValue)
{
	Exception::Throw("NWindow::SetFocusWindow not implemented");
}

void NWindow::SetFont(UObject* Self, UObject* fn)
{
	Exception::Throw("NWindow::SetFont not implemented");
}

void NWindow::SetFonts(UObject* Self, UObject* nFont, UObject* bFont)
{
	Exception::Throw("NWindow::SetFonts not implemented");
}

void NWindow::SetHeight(UObject* Self, float NewHeight)
{
	Exception::Throw("NWindow::SetHeight not implemented");
}

void NWindow::SetNormalFont(UObject* Self, UObject* fn)
{
	Exception::Throw("NWindow::SetNormalFont not implemented");
}

void NWindow::SetPos(UObject* Self, float newX, float newY)
{
	Exception::Throw("NWindow::SetPos not implemented");
}

void NWindow::SetSelectability(UObject* Self, bool newSelectability)
{
	Exception::Throw("NWindow::SetSelectability not implemented");
}

void NWindow::SetSensitivity(UObject* Self, bool newSensitivity)
{
	Exception::Throw("NWindow::SetSensitivity not implemented");
}

void NWindow::SetSize(UObject* Self, float newWidth, float NewHeight)
{
	Exception::Throw("NWindow::SetSize not implemented");
}

void NWindow::SetSoundVolume(UObject* Self, float newVolume)
{
	Exception::Throw("NWindow::SetSoundVolume not implemented");
}

void NWindow::SetTextColor(UObject* Self, const Color& NewColor)
{
	Exception::Throw("NWindow::SetTextColor not implemented");
}

void NWindow::SetTileColor(UObject* Self, const Color& NewColor)
{
	Exception::Throw("NWindow::SetTileColor not implemented");
}

void NWindow::SetVisibilitySounds(UObject* Self, UObject** visSound, UObject** invisSound)
{
	Exception::Throw("NWindow::SetVisibilitySounds not implemented");
}

void NWindow::SetWidth(UObject* Self, float newWidth)
{
	Exception::Throw("NWindow::SetWidth not implemented");
}

void NWindow::SetWindowAlignments(UObject* Self, uint8_t HAlign, uint8_t VAlign, float* hMargin0, float* vMargin0, float* hMargin1, float* vMargin1)
{
	Exception::Throw("NWindow::SetWindowAlignments not implemented");
}

void NWindow::Show(UObject* Self, BitfieldBool* bShow)
{
	Exception::Throw("NWindow::Show not implemented");
}

void NWindow::UngrabMouse(UObject* Self)
{
	Exception::Throw("NWindow::UngrabMouse not implemented");
}
