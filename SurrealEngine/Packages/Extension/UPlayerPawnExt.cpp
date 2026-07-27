
#include "Precomp.h"
#include "UPlayerPawnExt.h"
#include "Engine.h"
#include "Package/PackageManager.h"
#include "Packages/Extension/Windows/TabGroup/URootWindow.h"
#include "Render/RenderSubsystem.h"

void UPlayerPawnExt::InitRootWindow()
{
	auto dxIni = engine->packages->GetIniFile("System");
	NameString dxRootClassName = dxIni->GetValue("Engine.Engine", "Root", "");
	UClass* cls = engine->packages->FindClass(dxRootClassName);
	if (cls)
	{
		engine->dxRootWindow = UObject::Cast<URootWindow>(engine->packages->GetTransientPackage()->NewObject("dxRootWindow", cls, ObjectFlags::Transient));
		RootWindow() = engine->dxRootWindow;
		engine->dxRootWindow->parentPawn() = this;
		engine->dxRootWindow->bIsVisible() = true;
		engine->dxRootWindow->bIsSensitive() = true;
		engine->dxRootWindow->InitWindow();
	}
}

void UPlayerPawnExt::PreRenderWindows(UCanvas* canvas)
{
	engine->render->PreRenderWindows(canvas);
}

void UPlayerPawnExt::PostRenderWindows(UCanvas* canvas)
{
	engine->render->PostRenderWindows(canvas);
}
