#pragma once

#include "UI/Core/Element/WindowFrame.h"
#include <functional>

class Menubar;
class Menu;
class Toolbar;
class Statusbar;
class StatusbarItem;
class TabControl;
class DisassemblyPage;
class ObjectViewerPage;
class CallstackPage;
class LocalsPage;

class DebuggerWindow : public WindowFrame
{
public:
	DebuggerWindow(std::function<void()> onCloseCallback);

	void onBreakpointTriggered();
	void onClose() override;

	void onFileMenu(Menu* menu);
	void onEditMenu(Menu* menu);
	void onViewMenu(Menu* menu);
	void onHelpMenu(Menu* menu);
	void onToggleBreakpoint();
	void onContinue();
	void onStepIn();
	void onStepOver();

	std::function<void()> onCloseCallback;

	Menubar* menubar = nullptr;
	Toolbar* toolbar = nullptr;
	TabControl* tabcontrol = nullptr;
	Statusbar* statusbar = nullptr;
	StatusbarItem* statustext = nullptr;
	DisassemblyPage* disassembly = nullptr;
	ObjectViewerPage* objectviewer = nullptr;
	CallstackPage* callstack = nullptr;
	LocalsPage* locals = nullptr;
};
