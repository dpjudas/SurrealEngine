
#include "Precomp.h"
#include "DebuggerWindow.h"
#include "DisassemblyPage.h"
#include "ObjectViewerPage.h"
#include "UI/Core/View.h"
#include "UI/MainWindow/Menubar.h"
#include "UI/MainWindow/Toolbar.h"
#include "UI/MainWindow/Statusbar.h"
#include "UI/Controls/TabControl/TabControl.h"
#include "VM/Frame.h"

DebuggerWindow::DebuggerWindow(std::function<void()> onCloseCallback) : onCloseCallback(std::move(onCloseCallback))
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

	statusbar = new Statusbar(contentView());
	statustext = statusbar->addItem("Ready");

	setSize(1700, 950);
}

void DebuggerWindow::onBreakpointTriggered()
{
	statustext->text->setText(Frame::ExceptionText);
	if (!Frame::Callstack.empty())
	{
		disassembly->setFunction(Frame::Callstack.back()->Func);
		objectviewer->setObject(Frame::Callstack.back()->Object);
	}
	else
	{
		disassembly->setFunction(nullptr);
		objectviewer->setObject(nullptr);
	}
}

void DebuggerWindow::onClose()
{
	onCloseCallback();
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
