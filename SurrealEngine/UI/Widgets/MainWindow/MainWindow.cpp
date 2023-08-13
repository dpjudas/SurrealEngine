
#include "Precomp.h"
#include "MainWindow.h"
#include "UI/Widgets/Menubar/Menubar.h"
#include "UI/Widgets/Toolbar/Toolbar.h"
#include "UI/Widgets/Statusbar/Statusbar.h"

MainWindow::MainWindow()
{
	MenubarWidget = new Menubar(this);
	ToolbarWidget = new Toolbar(this);
	StatusbarWidget = new Statusbar(this);
}

MainWindow::~MainWindow()
{
}

void MainWindow::SetCentralWidget(Widget* widget)
{
	if (CentralWidget != widget)
	{
		delete CentralWidget;
		CentralWidget = widget;
		if (CentralWidget)
			CentralWidget->SetParent(this);
		OnGeometryChanged();
	}
}

void MainWindow::OnGeometryChanged()
{
}
