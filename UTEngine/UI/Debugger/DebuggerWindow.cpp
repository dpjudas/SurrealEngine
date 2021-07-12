
#include "Precomp.h"
#include "DebuggerWindow.h"
#include "DisassemblyPage.h"
#include "ObjectViewerPage.h"
#include "UI/Core/View.h"
#include "UI/MainWindow/Menubar.h"
#include "UI/MainWindow/Toolbar.h"
#include "UI/Controls/TabControl/TabControl.h"

DebuggerWindow::DebuggerWindow()
{
	setTitle("UTEngine Debugger");
	setContentView(std::make_unique<VBoxView>(nullptr));
	contentView()->addClass("debuggerwindow");

	menubar = new Menubar(contentView());
	menubar->addItem("File", [=](Menu* menu) { onFileMenu(menu); });
	menubar->addItem("Edit", [=](Menu* menu) { onEditMenu(menu); });
	menubar->addItem("View", [=](Menu* menu) { onViewMenu(menu); });
	menubar->addItem("Help", [=](Menu* menu) { onHelpMenu(menu); });

	toolbar = new Toolbar(contentView());
	toolbar->addButton("", "Continue", [=]() { onContinue(); });
	toolbar->addButton("", "Step In", [=]() { onStepIn(); });
	toolbar->addButton("", "Step Over", [=]() { onStepOver(); });
	toolbar->addButton("", "Toggle Breakpoint", [=]() { onToggleBreakpoint(); });

	disassembly = new DisassemblyPage(nullptr);
	objectviewer = new ObjectViewerPage(nullptr);

	tabcontrol = new TabControl(contentView());
	tabcontrol->setExpanding();
	tabcontrol->addPage({}, "Disassembly", disassembly);
	tabcontrol->addPage({}, "Object Viewer", objectviewer);

	setSize(1700, 950);
}

void DebuggerWindow::onFileMenu(Menu* menu)
{
}

void DebuggerWindow::onEditMenu(Menu* menu)
{
}

void DebuggerWindow::onViewMenu(Menu* menu)
{
}

void DebuggerWindow::onHelpMenu(Menu* menu)
{
}

void DebuggerWindow::onToggleBreakpoint()
{
}

void DebuggerWindow::onContinue()
{
}

void DebuggerWindow::onStepIn()
{
}

void DebuggerWindow::onStepOver()
{
}
