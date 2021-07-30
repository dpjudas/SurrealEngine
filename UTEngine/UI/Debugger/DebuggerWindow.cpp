
#include "Precomp.h"
#include "DebuggerWindow.h"
#include "DisassemblyPage.h"
#include "ObjectViewerPage.h"
#include "CallstackPage.h"
#include "LocalsPage.h"
#include "LogPage.h"
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
	callstack = new CallstackPage(nullptr);
	locals = new LocalsPage(nullptr);
	log = new LogPage(nullptr);

	tabcontrol = new TabControl(contentView());
	tabcontrol->setExpanding();
	tabcontrol->addPage({}, "Disassembly", disassembly);
	tabcontrol->addPage({}, "Object Viewer", objectviewer);
	tabcontrol->addPage({}, "Log", log);

	spacer1 = new View(contentView());
	spacer1->element->setStyle("height", "5px");

	panel = new HBoxView(contentView());
	panel->element->setStyle("height", "300px");
	tabLeft = new TabControl(panel);
	tabLeft->setBorderStyle(TabControlBorderStyle::left);
	tabLeft->setBarPosition(TabBarPosition::bottom);
	tabLeft->setExpanding();
	tabLeft->addPage({}, "Locals", locals);
	spacer2 = new View(panel);
	spacer2->element->setStyle("width", "5px");
	tabRight = new TabControl(panel);
	tabRight->setBorderStyle(TabControlBorderStyle::right);
	tabRight->setBarPosition(TabBarPosition::bottom);
	tabRight->setExpanding();
	tabRight->addPage({}, "Call Stack", callstack);

	statusbar = new Statusbar(contentView());
	statustext = statusbar->addItem("Ready");

	setSize(1700, 950);
}

void DebuggerWindow::onBreakpointTriggered()
{
	if (!Frame::ExceptionText.empty())
		statustext->text->setText(Frame::ExceptionText);
	else
		statustext->text->setText(" ");

	if (!Frame::Callstack.empty())
	{
		disassembly->setFunction(Frame::Callstack.back()->Func);
		objectviewer->setObject(Frame::Callstack.back()->Object);
		locals->setFrame(Frame::Callstack.back());
	}
	else
	{
		disassembly->setFunction(nullptr);
		objectviewer->setObject(nullptr);
		locals->setFrame(nullptr);
	}
	callstack->updateList();
	log->update();
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
	Frame::Resume();
}

void DebuggerWindow::onStepIn()
{
	Frame::StepInto();
}

void DebuggerWindow::onStepOver()
{
	Frame::StepOver();
}
