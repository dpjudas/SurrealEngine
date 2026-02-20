#include "Precomp.h"
#include "NWindow.h"
#include "VM/NativeFunc.h"
#include "UObject/UWindow.h"
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
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->AddActorRef(refActor);
}

void NWindow::AddTimer(UObject* Self, float TimeOut, BitfieldBool* bLoop, int* clientData, NameString* functionName, int& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->AddTimer(TimeOut, bLoop, clientData, functionName);
}

void NWindow::AskParentForReconfigure(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->AskParentForReconfigure();
}

void NWindow::AskParentToShowArea(UObject* Self, float* areaX, float* areaY, float* areaWidth, float* areaHeight)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->AskParentToShowArea(areaX, areaY, areaWidth, areaHeight);
}

void NWindow::CarriageReturn(UObject* Self, std::string& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->CarriageReturn();
}

void NWindow::ChangeStyle(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->ChangeStyle();
}

void NWindow::ConfigureChild(UObject* Self, float newX, float newY, float newWidth, float NewHeight)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->ConfigureChild(newX, newY, newWidth, NewHeight);
}

void NWindow::ConvertCoordinates(UObject* Self, UObject* fromWin, float fromX, float fromY, UObject* toWin, float& toX, float& toY)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->ConvertCoordinates(fromWin, fromX, fromY, toWin, toX, toY);
}

void NWindow::ConvertScriptString(UObject* Self, const std::string& oldStr, std::string& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->ConvertScriptString(oldStr);
}

void NWindow::ConvertVectorToCoordinates(UObject* Self, const vec3& Location, float& relativeX, float& relativeY, BitfieldBool& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->ConvertVectorToCoordinates(Location, relativeX, relativeY);
}

void NWindow::Destroy(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->Destroy();
}

void NWindow::DestroyAllChildren(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->DestroyAllChildren();
}

void NWindow::DisableWindow(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->DisableWindow();
}

void NWindow::EnableSpecialText(UObject* Self, BitfieldBool* bEnable)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->EnableSpecialText(bEnable);
}

void NWindow::EnableTranslucentText(UObject* Self, BitfieldBool* bEnable)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->EnableTranslucentText(bEnable);
}

void NWindow::EnableWindow(UObject* Self, BitfieldBool* bEnable)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->EnableWindow(bEnable);
}

void NWindow::FindWindow(UObject* Self, float pointX, float pointY, float& relativeX, float& relativeY, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->FindWindow(pointX, pointY, relativeX, relativeY);
}

void NWindow::GetBottomChild(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetBottomChild(bVisibleOnly);
}

void NWindow::GetClientObject(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetClientObject();
}

void NWindow::GetCursorPos(UObject* Self, float& MouseX, float& MouseY)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->GetCursorPos(MouseX, MouseY);
}

void NWindow::GetFocusWindow(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetFocusWindow();
}

void NWindow::GetGC(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetGC();
}

void NWindow::GetHigherSibling(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetHigherSibling(bVisibleOnly);
}

void NWindow::GetLowerSibling(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetLowerSibling(bVisibleOnly);
}

void NWindow::GetModalWindow(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetModalWindow();
}

void NWindow::GetParent(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetParent();
}

void NWindow::GetPlayerPawn(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetPlayerPawn();
}

void NWindow::GetRootWindow(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetRootWindow();
}

void NWindow::GetTabGroupWindow(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetTabGroupWindow();
}

void NWindow::GetTickOffset(UObject* Self, float& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetTickOffset();
}

void NWindow::GetTopChild(UObject* Self, BitfieldBool* bVisibleOnly, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->GetTopChild(bVisibleOnly);
}

void NWindow::GrabMouse(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->GrabMouse();
}

void NWindow::Hide(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->Hide();
}

void NWindow::IsActorValid(UObject* Self, UObject* refActor, BitfieldBool& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->IsActorValid(refActor);
}

void NWindow::IsFocusWindow(UObject* Self, BitfieldBool& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->IsFocusWindow();
}

void NWindow::IsKeyDown(UObject* Self, uint8_t Key, BitfieldBool& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->IsKeyDown(Key);
}

void NWindow::IsPointInWindow(UObject* Self, float pointX, float pointY, BitfieldBool& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->IsPointInWindow(pointX, pointY);
}

void NWindow::IsSensitive(UObject* Self, BitfieldBool* bRecurse, BitfieldBool& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->IsSensitive(bRecurse);
}

void NWindow::IsVisible(UObject* Self, BitfieldBool* bRecurse, BitfieldBool& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->IsVisible(bRecurse);
}

void NWindow::Lower(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->Lower();
}

void NWindow::MoveFocusDown(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->MoveFocusDown();
}

void NWindow::MoveFocusLeft(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->MoveFocusLeft();
}

void NWindow::MoveFocusRight(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->MoveFocusRight();
}

void NWindow::MoveFocusUp(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->MoveFocusUp();
}

void NWindow::MoveTabGroupNext(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->MoveTabGroupNext();
}

void NWindow::MoveTabGroupPrev(UObject* Self, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->MoveTabGroupPrev();
}

void NWindow::NewChild(UObject* Self, UObject* NewClass, BitfieldBool* bShow, UObject*& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->NewChild(NewClass, bShow);
}

void NWindow::PlaySound(UObject* Self, UObject* newsound, float* Volume, float* Pitch, float* posX, float* posY)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->PlaySound(newsound, Volume, Pitch, posX, posY);
}

void NWindow::QueryGranularity(UObject* Self, float& hGranularity, float& vGranularity)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->QueryGranularity(hGranularity, vGranularity);
}

void NWindow::QueryPreferredHeight(UObject* Self, float queryWidth, float& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->QueryPreferredHeight(queryWidth);
}

void NWindow::QueryPreferredSize(UObject* Self, float& preferredWidth, float& preferredHeight)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->QueryPreferredSize(preferredWidth, preferredHeight);
}

void NWindow::QueryPreferredWidth(UObject* Self, float queryHeight, float& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->QueryPreferredWidth(queryHeight);
}

void NWindow::Raise(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->Raise();
}

void NWindow::ReleaseGC(UObject* Self, UObject* GC)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->ReleaseGC(GC);
}

void NWindow::RemoveActorRef(UObject* Self, UObject* refActor)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->RemoveActorRef(refActor);
}

void NWindow::RemoveTimer(UObject* Self, int timerId)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->RemoveTimer(timerId);
}

void NWindow::ResetHeight(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->ResetHeight();
}

void NWindow::ResetSize(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->ResetSize();
}

void NWindow::ResetWidth(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->ResetWidth();
}

void NWindow::ResizeChild(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->ResizeChild();
}

void NWindow::SetAcceleratorText(UObject* Self, const std::string& newStr)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetAcceleratorText(newStr);
}

void NWindow::SetBackground(UObject* Self, UObject* newBackground)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetBackground(newBackground);
}

void NWindow::SetBackgroundSmoothing(UObject* Self, bool newSmoothing)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetBackgroundSmoothing(newSmoothing);
}

void NWindow::SetBackgroundStretching(UObject* Self, bool newStretching)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetBackgroundStretching(newStretching);
}

void NWindow::SetBackgroundStyle(UObject* Self, uint8_t NewStyle)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetBackgroundStyle(NewStyle);
}

void NWindow::SetBaselineData(UObject* Self, float* newBaselineOffset, float* newUnderlineHeight)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetBaselineData(newBaselineOffset, newUnderlineHeight);
}

void NWindow::SetBoldFont(UObject* Self, UObject* fn)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetBoldFont(fn);
}

void NWindow::SetChildVisibility(UObject* Self, bool bNewVisibility)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetChildVisibility(bNewVisibility);
}

void NWindow::SetClientObject(UObject* Self, UObject* newClientObject)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetClientObject(newClientObject);
}

void NWindow::SetConfiguration(UObject* Self, float newX, float newY, float newWidth, float NewHeight)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetConfiguration(newX, newY, newWidth, NewHeight);
}

void NWindow::SetCursorPos(UObject* Self, float newMouseX, float newMouseY)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetCursorPos(newMouseX, newMouseY);
}

void NWindow::SetDefaultCursor(UObject* Self, UObject* tX, UObject** shadowTexture, float* HotX, float* HotY, Color* cursorColor)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetDefaultCursor(tX, shadowTexture, HotX, HotY, cursorColor);
}

void NWindow::SetFocusSounds(UObject* Self, UObject** focusSound, UObject** unfocusSound)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetFocusSounds(focusSound, unfocusSound);
}

void NWindow::SetFocusWindow(UObject* Self, UObject* NewFocusWindow, BitfieldBool& ReturnValue)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	ReturnValue = selfWindow->SetFocusWindow(NewFocusWindow);
}

void NWindow::SetFont(UObject* Self, UObject* fn)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetFont(fn);
}

void NWindow::SetFonts(UObject* Self, UObject* nFont, UObject* bFont)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetFonts(nFont, bFont);
}

void NWindow::SetHeight(UObject* Self, float NewHeight)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetHeight(NewHeight);
}

void NWindow::SetNormalFont(UObject* Self, UObject* fn)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetNormalFont(fn);
}

void NWindow::SetPos(UObject* Self, float newX, float newY)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetPos(newX, newY);
}

void NWindow::SetSelectability(UObject* Self, bool newSelectability)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetSelectability(newSelectability);
}

void NWindow::SetSensitivity(UObject* Self, bool newSensitivity)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetSensitivity(newSensitivity);
}

void NWindow::SetSize(UObject* Self, float newWidth, float NewHeight)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetSize(newWidth, NewHeight);
}

void NWindow::SetSoundVolume(UObject* Self, float newVolume)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetSoundVolume(newVolume);
}

void NWindow::SetTextColor(UObject* Self, const Color& NewColor)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetTextColor(NewColor);
}

void NWindow::SetTileColor(UObject* Self, const Color& NewColor)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetTileColor(NewColor);
}

void NWindow::SetVisibilitySounds(UObject* Self, UObject** visSound, UObject** invisSound)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetVisibilitySounds(visSound, invisSound);
}

void NWindow::SetWidth(UObject* Self, float newWidth)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetWidth(newWidth);
}

void NWindow::SetWindowAlignments(UObject* Self, uint8_t HAlign, uint8_t VAlign, float* hMargin0, float* vMargin0, float* hMargin1, float* vMargin1)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->SetWindowAlignments(HAlign, VAlign, hMargin0, vMargin0, hMargin1, vMargin1);
}

void NWindow::Show(UObject* Self, BitfieldBool* bShow)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->Show(bShow);
}

void NWindow::UngrabMouse(UObject* Self)
{
	UWindow* selfWindow = UObject::Cast<UWindow>(Self);
	selfWindow->UngrabMouse();
}
