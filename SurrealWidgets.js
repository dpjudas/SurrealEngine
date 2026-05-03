import { Project, Target, File, FilePath, Directory, Environment } from "cppbuild";

var enableMetal = true;
var enableOpenGL = true;
var enableSDL3 = false;
var enableSDL2 = false;
var enableX11 = true;
var enableWayland = true;
var enableDBUS = true;

var sources = [
	"src/core/canvas.cpp",
	"src/core/font.cpp",
	"src/core/font_impl.h",
	"src/core/image.cpp",
	"src/core/layout.cpp",
	"src/core/span_layout.cpp",
	"src/core/timer.cpp",
	"src/core/widget.cpp",
	"src/core/theme.cpp",
	"src/core/theme_default_stylesheet.h",
	"src/core/theme_stylesheet_document.cpp",
	"src/core/theme_stylesheet_document.h",
	"src/core/theme_stylesheet_tokenizer.cpp",
	"src/core/theme_stylesheet_tokenizer.h",
	"src/core/border_image_renderer.cpp",
	"src/core/border_image_renderer.h",
	"src/core/utf8reader.cpp",
	"src/core/pathfill.cpp",
	"src/core/truetypefont.cpp",
	"src/core/truetypefont.h",
	"src/core/picopng/picopng.cpp",
	"src/core/picopng/picopng.h",
	"src/core/nanosvg/nanosvg.cpp",
	"src/core/nanosvg/nanosvg.h",
	"src/core/nanosvg/nanosvgrast.h",
	"src/widgets/dialog/dialog.cpp",
	"src/widgets/dialog/messagebox.cpp",
	"src/widgets/dialog/textinputdialog.cpp",
	"src/widgets/lineedit/lineedit.cpp",
	"src/widgets/mainwindow/mainwindow.cpp",
	"src/widgets/menubar/menubar.cpp",
	"src/widgets/scrollbar/scrollbar.cpp",
	"src/widgets/statusbar/statusbar.cpp",
	"src/widgets/textedit/textedit.cpp",
	"src/widgets/toolbar/toolbar.cpp",
	"src/widgets/toolbar/toolbarbutton.cpp",
	"src/widgets/imagebox/imagebox.cpp",
	"src/widgets/textlabel/textlabel.cpp",
	"src/widgets/pushbutton/pushbutton.cpp",
	"src/widgets/checkboxlabel/checkboxlabel.cpp",
	"src/widgets/dropdown/dropdown.cpp",
	"src/widgets/listview/listview.cpp",
	"src/widgets/tabwidget/tabwidget.cpp",
	"src/widgets/treeview/treeview.cpp",
	"src/widgets/layout/hboxlayout.cpp",
	"src/widgets/layout/vboxlayout.cpp",
	"src/window/window.cpp",
	"src/window/stub/stub_open_folder_dialog.cpp",
	"src/window/stub/stub_open_folder_dialog.h",
	"src/window/stub/stub_open_file_dialog.cpp",
	"src/window/stub/stub_open_file_dialog.h",
	"src/window/stub/stub_save_file_dialog.cpp",
	"src/window/stub/stub_save_file_dialog.h",
	"src/systemdialogs/open_folder_dialog.cpp",
	"src/systemdialogs/open_file_dialog.cpp",
	"src/systemdialogs/save_file_dialog.cpp",
];

var includes = [
	"include/surrealwidgets/core/canvas.h",
	"include/surrealwidgets/core/colorf.h",
	"include/surrealwidgets/core/font.h",
	"include/surrealwidgets/core/image.h",
	"include/surrealwidgets/core/rect.h",
	"include/surrealwidgets/core/pathfill.h",
	"include/surrealwidgets/core/span_layout.h",
	"include/surrealwidgets/core/timer.h",
	"include/surrealwidgets/core/widget.h",
	"include/surrealwidgets/core/theme.h",
	"include/surrealwidgets/core/utf8reader.h",
	"include/surrealwidgets/core/resourcedata.h",
	"include/surrealwidgets/widgets/dialog/dialog.h",
	"include/surrealwidgets/widgets/dialog/messagebox.h",
	"include/surrealwidgets/widgets/dialog/textinputdialog.h",
	"include/surrealwidgets/widgets/lineedit/lineedit.h",
	"include/surrealwidgets/widgets/mainwindow/mainwindow.h",
	"include/surrealwidgets/widgets/menubar/menubar.h",
	"include/surrealwidgets/widgets/scrollbar/scrollbar.h",
	"include/surrealwidgets/widgets/statusbar/statusbar.h",
	"include/surrealwidgets/widgets/textedit/textedit.h",
	"include/surrealwidgets/widgets/toolbar/toolbar.h",
	"include/surrealwidgets/widgets/toolbar/toolbarbutton.h",
	"include/surrealwidgets/widgets/imagebox/imagebox.h",
	"include/surrealwidgets/widgets/textlabel/textlabel.h",
	"include/surrealwidgets/widgets/pushbutton/pushbutton.h",
	"include/surrealwidgets/widgets/checkboxlabel/checkboxlabel.h",
	"include/surrealwidgets/widgets/listview/listview.h",
	"include/surrealwidgets/widgets/tabwidget/tabwidget.h",
	"include/surrealwidgets/widgets/treeview/treeview.h",
	"include/surrealwidgets/widgets/layout/hboxlayout.h",
	"include/surrealwidgets/widgets/layout/vboxlayout.h",
	"include/surrealwidgets/window/window.h",
	"include/surrealwidgets/window/x11nativehandle.h",
	"include/surrealwidgets/window/waylandnativehandle.h",
	"include/surrealwidgets/window/win32nativehandle.h",
	"include/surrealwidgets/window/sdlnativehandle.h",
	"include/surrealwidgets/systemdialogs/open_folder_dialog.h",
	"include/surrealwidgets/systemdialogs/open_file_dialog.h",
	"include/surrealwidgets/systemdialogs/save_file_dialog.h",
];

var win32Sources = [
	"src/window/win32/win32_display_backend.cpp",
	"src/window/win32/win32_display_backend.h",
	"src/window/win32/win32_display_window.cpp",
	"src/window/win32/win32_display_window.h",
	"src/window/win32/win32_open_folder_dialog.cpp",
	"src/window/win32/win32_open_folder_dialog.h",
	"src/window/win32/win32_open_file_dialog.cpp",
	"src/window/win32/win32_open_file_dialog.h",
	"src/window/win32/win32_save_file_dialog.cpp",
	"src/window/win32/win32_save_file_dialog.h",
	"src/window/win32/win32_util.h",
	"src/core/resourcedata_win.cpp",
];

var dbusSources = [
	"src/window/dbus/dbus_open_folder_dialog.cpp",
	"src/window/dbus/dbus_open_folder_dialog.h",
	"src/window/dbus/dbus_open_file_dialog.cpp",
	"src/window/dbus/dbus_open_file_dialog.h",
	"src/window/dbus/dbus_save_file_dialog.cpp",
	"src/window/dbus/dbus_save_file_dialog.h",
];

var unixSources = [
	"src/core/resourcedata_unix.cpp",
];

var cocoaSources = [
	"src/core/resourcedata_mac.mm",
];

var cocoaArcSources = [
	"src/window/cocoa/cocoa_open_file_dialog.mm",
	"src/window/cocoa/cocoa_save_file_dialog.mm",
	"src/window/cocoa/cocoa_open_folder_dialog.mm",
	"src/window/cocoa/cocoa_display_backend.mm",
	"src/window/cocoa/cocoa_display_window.mm",
];

var sdl2Sources = [
	"src/window/sdl2/sdl2_display_backend.cpp",
	"src/window/sdl2/sdl2_display_backend.h",
	"src/window/sdl2/sdl2_display_window.cpp",
	"src/window/sdl2/sdl2_display_window.h",
];

var sdl3Sources = [
	"src/window/sdl3/sdl3_display_backend.cpp",
	"src/window/sdl3/sdl3_display_backend.h",
	"src/window/sdl3/sdl3_display_window.cpp",
	"src/window/sdl3/sdl3_display_window.h",
];

var x11Sources = [
	"src/window/x11/x11_display_backend.cpp",
	"src/window/x11/x11_display_backend.h",
	"src/window/x11/x11_display_window.cpp",
	"src/window/x11/x11_display_window.h",
	"src/window/x11/x11_connection.cpp",
	"src/window/x11/x11_connection.h",
];

var waylandSources = [
	"src/window/wayland/wayland_display_backend.cpp",
	"src/window/wayland/wayland_display_backend.h",
	"src/window/wayland/wayland_display_window.cpp",
	"src/window/wayland/wayland_display_window.h",
	"src/window/wayland/wl_fractional_scaling_protocol.cpp",
	"src/window/wayland/wl_fractional_scaling_protocol.hpp",
	"src/window/wayland/wl_cursor_shape.cpp",
	"src/window/wayland/wl_cursor_shape.hpp",
	"src/window/wayland/wl_xdg_dialog_v1.cpp",
	"src/window/wayland/wl_xdg_dialog_v1.hpp",
	"src/window/wayland/wl_xdg_toplevel_icon.cpp",
	"src/window/wayland/wl_xdg_toplevel_icon.hpp",
];

var surrealwidgets = Target.addStaticLibrary("surrealwidgets");
surrealwidgets.addFiles(sources);
surrealwidgets.addFiles(includes);
surrealwidgets.addIncludePaths(["include", "include/surrealwidgets", "src"]);

if (Environment.isWindows()) {
	surrealwidgets.addFiles(win32Sources);
	surrealwidgets.addDefines([
		"WIN32",
		"UNICODE",
		"_UNICODE",
		"WIN32_LEAN_AND_MEAN",
		"WINVER=0x0A00", // Windows 10
		"_WIN32_WINNT=0x0A00",
		"NOMINMAX",
	]);
}
else if (Environment.isApple()) {
	surrealwidgets.addFiles(cocoaSources);
	surrealwidgets.addFiles(cocoaArcSources);
	surrealwidgets.addDefines(["UNIX", "_UNIX"]);

	if (enableOpenGL) {
		surrealwidgets.addDefines(["HAVE_OPENGL"]);
	}
	if (enableMetal) {
		surrealwidgets.addDefines(["HAVE_METAL"]);
	}
}
else if (Environment.isUnix()) {
	surrealwidgets.addFiles(unixSources);
	surrealwidgets.addDefines(["UNIX", "_UNIX"]);
	surrealwidgets.addLinkLibraries(["gio-2.0"]);
	if (enableSDL2) {
		surrealwidgets.addFiles(sdl2Sources);
		surrealwidgets.addDefines(["USE_SDL2"]);
		surrealwidgets.addLinkLibraries(["sdl2"]);
	}
	if (enableSDL3) {
		surrealwidgets.addFiles(sdl3Sources);
		surrealwidgets.addDefines(["USE_SDL3"]);
		surrealwidgets.addLinkLibraries(["sdl3"]);
	}
	if (enableX11) {
		surrealwidgets.addFiles(x11Sources);
		surrealwidgets.addDefines(["USE_X11"]);
		surrealwidgets.addLinkLibraries(["X11", "Xi"]);
	}
	if (enableWayland) {
		surrealwidgets.addFiles(waylandSources);
		surrealwidgets.addDefines(["USE_WAYLAND"]);
	}
	if (enableDBUS) {
		surrealwidgets.addFiles(dbusSources);
		surrealwidgets.addDefines(["USE_DBUS"]);
		surrealwidgets.addIncludePaths(["/usr/lib64/dbus-1.0/include"]);
	}
}
