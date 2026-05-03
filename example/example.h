#pragma once

#include <surrealwidgets/core/widget.h>
#include <surrealwidgets/core/resourcedata.h>
#include <surrealwidgets/core/image.h>
#include <surrealwidgets/core/theme.h>
#include <surrealwidgets/window/window.h>
#include <surrealwidgets/widgets/dropdown/dropdown.h>
#include <surrealwidgets/widgets/textedit/textedit.h>
#include <surrealwidgets/widgets/mainwindow/mainwindow.h>
#include <surrealwidgets/widgets/dialog/messagebox.h>
#include <surrealwidgets/widgets/layout/vboxlayout.h>
#include <surrealwidgets/widgets/layout/hboxlayout.h>
#include <surrealwidgets/widgets/listview/listview.h>
#include <surrealwidgets/widgets/imagebox/imagebox.h>
#include <surrealwidgets/widgets/textlabel/textlabel.h>
#include <surrealwidgets/widgets/pushbutton/pushbutton.h>
#include <surrealwidgets/widgets/checkboxlabel/checkboxlabel.h>
#include <surrealwidgets/widgets/lineedit/lineedit.h>
#include <surrealwidgets/widgets/tabwidget/tabwidget.h>
#include <surrealwidgets/widgets/dialog/textinputdialog.h>
#include <surrealwidgets/widgets/treeview/treeview.h>

class LauncherWindowTab1 : public Widget
{
public:
	LauncherWindowTab1(Widget parent);
private:
	TextEdit* Text = nullptr;
};

class LauncherWindowTab2 : public Widget
{
public:
	LauncherWindowTab2(Widget parent);
private:
	TextLabel* WelcomeLabel = nullptr;
	TextLabel* VersionLabel = nullptr;
	TextLabel* SelectLabel = nullptr;
	TextLabel* GeneralLabel = nullptr;
	TextLabel* ExtrasLabel = nullptr;
	CheckboxLabel* FullscreenCheckbox = nullptr;
	CheckboxLabel* DisableAutoloadCheckbox = nullptr;
	CheckboxLabel* DontAskAgainCheckbox = nullptr;
	CheckboxLabel* LightsCheckbox = nullptr;
	CheckboxLabel* BrightmapsCheckbox = nullptr;
	CheckboxLabel* WidescreenCheckbox = nullptr;
	ListView* GamesList = nullptr;
};

class LauncherWindowTab3 : public Widget
{
public:
	LauncherWindowTab3(Widget parent);
private:
	TextLabel* Label = nullptr;
	Dropdown* Choices = nullptr;
	PushButton* Popup = nullptr;
	PushButton* QuestionPopup = nullptr;
	PushButton* TextInputPopup = nullptr;

	TreeView* m_TreeView = nullptr;
};

class LauncherWindow : public Widget
{
public:
	LauncherWindow();
private:
	void OnClose() override;

	ImageBox* Logo = nullptr;
	TabWidget* Pages = nullptr;
	PushButton* ExitButton = nullptr;

	LauncherWindowTab1* Tab1 = nullptr;
	LauncherWindowTab2* Tab2 = nullptr;
	LauncherWindowTab3* Tab3 = nullptr;

	std::shared_ptr<CustomCursor> Cursor;
};

enum class Backend
{
	Default, Win32, SDL2, X11, Wayland
};

enum class Theme
{
	Default, Light, Dark
};

class ExampleResourceLoader : public ResourceLoader
{
public:
	std::vector<SingleFontData> LoadFont(const std::string& name) override;
	std::vector<uint8_t> ReadAllBytes(const std::string& filename) override;
};

int example(Backend backend = Backend::Default, Theme theme = Theme::Default);
