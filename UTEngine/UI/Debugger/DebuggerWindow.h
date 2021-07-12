#pragma once

#include "UI/Core/Element/WindowFrame.h"

class Menubar;
class Menu;
class Toolbar;
class TabControl;
class DisassemblyPage;
class ObjectViewerPage;

class DebuggerWindow : public WindowFrame
{
public:
	DebuggerWindow();

	void onFileMenu(Menu* menu);
	void onEditMenu(Menu* menu);
	void onViewMenu(Menu* menu);
	void onHelpMenu(Menu* menu);
	void onToggleBreakpoint();
	void onContinue();
	void onStepIn();
	void onStepOver();

	Menubar* menubar = nullptr;
	Toolbar* toolbar = nullptr;
	TabControl* tabcontrol = nullptr;
	DisassemblyPage* disassembly = nullptr;
	ObjectViewerPage* objectviewer = nullptr;
};
