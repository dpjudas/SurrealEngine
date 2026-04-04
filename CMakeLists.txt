cmake_minimum_required(VERSION 3.11)
project(surrealwidgets)

option(ENABLE_METAL "Enable Metal support for application rendering" ON)
option(ENABLE_OPENGL "Enable OpenGL support for application rendering" ON)

if(WIN32)
	option(ENABLE_SDL3 "Enable SDL3 backend. Optional on all platforms" OFF)
	option(ENABLE_SDL2 "Enable SDL2 backend. Optional on all platforms" OFF)
else()
	option(ENABLE_SDL3 "Enable SDL3 backend. Optional on all platforms" ON)
	option(ENABLE_SDL2 "Enable SDL2 backend. Optional on all platforms" ON)
endif()

if (UNIX AND NOT APPLE)
	include(FindPkgConfig)
	pkg_check_modules(DBUS dbus-1)
	if (!DBUS_FOUND)
		pkg_check_modules(DBUS REQUIRED dbus) # Fedora Linux looks for dbus instead
	endif()
	pkg_check_modules(WAYLANDPP
		wayland-client
		wayland-client++
		wayland-client-extra++
		wayland-client-unstable++
		wayland-cursor++
		xkbcommon>=0.5.0
	)
	pkg_check_modules(GLIB glib-2.0)
	pkg_check_modules(FONTCONFIG fontconfig)
endif()

# SDL3 finding stuff
# Optional on all platforms
# If both SDL3 and SDL2 are available, only SDL3 support will be enabled.
if(ENABLE_SDL3)
	find_package(SDL3 QUIET)
	if(NOT ${SDL3_FOUND})
		include(FindPkgConfig)
		pkg_search_module(SDL3 sdl3)
	endif()
endif()

# SDL2 finding stuff
# Optional on all platforms
if(ENABLE_SDL2)
	find_package(SDL2 QUIET)
	if(NOT ${SDL2_FOUND})
		include(FindPkgConfig)
		pkg_search_module(SDL2 sdl2)
	endif()
endif()

set(SURREALWIDGETS_SOURCES
	src/core/canvas.cpp
	src/core/font.cpp
	src/core/font_impl.h
	src/core/image.cpp
	src/core/layout.cpp
	src/core/span_layout.cpp
	src/core/timer.cpp
	src/core/widget.cpp
	src/core/theme.cpp
	src/core/theme_default_stylesheet.h
	src/core/theme_stylesheet_document.cpp
	src/core/theme_stylesheet_document.h
	src/core/theme_stylesheet_tokenizer.cpp
	src/core/theme_stylesheet_tokenizer.h
	src/core/border_image_renderer.cpp
	src/core/border_image_renderer.h
	src/core/utf8reader.cpp
	src/core/pathfill.cpp
	src/core/truetypefont.cpp
	src/core/truetypefont.h
	src/core/picopng/picopng.cpp
	src/core/picopng/picopng.h
	src/core/nanosvg/nanosvg.cpp
	src/core/nanosvg/nanosvg.h
	src/core/nanosvg/nanosvgrast.h
	src/widgets/dialog/dialog.cpp
	src/widgets/dialog/messagebox.cpp
	src/widgets/dialog/textinputdialog.cpp
	src/widgets/lineedit/lineedit.cpp
	src/widgets/mainwindow/mainwindow.cpp
	src/widgets/menubar/menubar.cpp
	src/widgets/scrollbar/scrollbar.cpp
	src/widgets/statusbar/statusbar.cpp
	src/widgets/textedit/textedit.cpp
	src/widgets/toolbar/toolbar.cpp
	src/widgets/toolbar/toolbarbutton.cpp
	src/widgets/imagebox/imagebox.cpp
	src/widgets/textlabel/textlabel.cpp
	src/widgets/pushbutton/pushbutton.cpp
	src/widgets/checkboxlabel/checkboxlabel.cpp
	src/widgets/dropdown/dropdown.cpp
	src/widgets/listview/listview.cpp
	src/widgets/tabwidget/tabwidget.cpp
	src/widgets/layout/hboxlayout.cpp
	src/widgets/layout/vboxlayout.cpp
	src/window/window.cpp
	src/window/stub/stub_open_folder_dialog.cpp
	src/window/stub/stub_open_folder_dialog.h
	src/window/stub/stub_open_file_dialog.cpp
	src/window/stub/stub_open_file_dialog.h
	src/window/stub/stub_save_file_dialog.cpp
	src/window/stub/stub_save_file_dialog.h
	src/systemdialogs/open_folder_dialog.cpp
	src/systemdialogs/open_file_dialog.cpp
	src/systemdialogs/save_file_dialog.cpp
)

set(SURREALWIDGETS_INCLUDES
	include/surrealwidgets/core/canvas.h
	include/surrealwidgets/core/colorf.h
	include/surrealwidgets/core/font.h
	include/surrealwidgets/core/image.h
	include/surrealwidgets/core/rect.h
	include/surrealwidgets/core/pathfill.h
	include/surrealwidgets/core/span_layout.h
	include/surrealwidgets/core/timer.h
	include/surrealwidgets/core/widget.h
	include/surrealwidgets/core/theme.h
	include/surrealwidgets/core/utf8reader.h
	include/surrealwidgets/core/resourcedata.h
	include/surrealwidgets/widgets/dialog/dialog.h
	include/surrealwidgets/widgets/dialog/messagebox.h
	include/surrealwidgets/widgets/dialog/textinputdialog.h
	include/surrealwidgets/widgets/lineedit/lineedit.h
	include/surrealwidgets/widgets/mainwindow/mainwindow.h
	include/surrealwidgets/widgets/menubar/menubar.h
	include/surrealwidgets/widgets/scrollbar/scrollbar.h
	include/surrealwidgets/widgets/statusbar/statusbar.h
	include/surrealwidgets/widgets/textedit/textedit.h
	include/surrealwidgets/widgets/toolbar/toolbar.h
	include/surrealwidgets/widgets/toolbar/toolbarbutton.h
	include/surrealwidgets/widgets/imagebox/imagebox.h
	include/surrealwidgets/widgets/textlabel/textlabel.h
	include/surrealwidgets/widgets/pushbutton/pushbutton.h
	include/surrealwidgets/widgets/checkboxlabel/checkboxlabel.h
	include/surrealwidgets/widgets/listview/listview.h
	include/surrealwidgets/widgets/tabwidget/tabwidget.h
	include/surrealwidgets/widgets/layout/hboxlayout.h
	include/surrealwidgets/widgets/layout/vboxlayout.h
	include/surrealwidgets/window/window.h
	include/surrealwidgets/window/x11nativehandle.h
	include/surrealwidgets/window/waylandnativehandle.h
	include/surrealwidgets/window/win32nativehandle.h
	include/surrealwidgets/window/sdlnativehandle.h
	include/surrealwidgets/systemdialogs/open_folder_dialog.h
	include/surrealwidgets/systemdialogs/open_file_dialog.h
	include/surrealwidgets/systemdialogs/save_file_dialog.h
)

set(SURREALWIDGETS_WIN32_SOURCES
	src/window/win32/win32_display_backend.cpp
	src/window/win32/win32_display_backend.h
	src/window/win32/win32_display_window.cpp
	src/window/win32/win32_display_window.h
	src/window/win32/win32_open_folder_dialog.cpp
	src/window/win32/win32_open_folder_dialog.h
	src/window/win32/win32_open_file_dialog.cpp
	src/window/win32/win32_open_file_dialog.h
	src/window/win32/win32_save_file_dialog.cpp
	src/window/win32/win32_save_file_dialog.h
	src/window/win32/win32_util.h
	src/core/resourcedata_win.cpp
)

set(SURREALWIDGETS_DBUS_SOURCES
	src/window/dbus/dbus_open_folder_dialog.cpp
	src/window/dbus/dbus_open_folder_dialog.h
	src/window/dbus/dbus_open_file_dialog.cpp
	src/window/dbus/dbus_open_file_dialog.h
	src/window/dbus/dbus_save_file_dialog.cpp
	src/window/dbus/dbus_save_file_dialog.h
)

set(SURREALWIDGETS_UNIX_SOURCES
	src/core/resourcedata_unix.cpp
)

set(SURREALWIDGETS_COCOA_SOURCES
	src/core/resourcedata_mac.mm
)

set(SURREALWIDGETS_COCOA_ARC_SOURCES
	src/window/cocoa/cocoa_open_file_dialog.mm
	src/window/cocoa/cocoa_save_file_dialog.mm
	src/window/cocoa/cocoa_open_folder_dialog.mm
	src/window/cocoa/cocoa_display_backend.mm
	src/window/cocoa/cocoa_display_window.mm
)

set(SURREALWIDGETS_SDL2_SOURCES
	src/window/sdl2/sdl2_display_backend.cpp
	src/window/sdl2/sdl2_display_backend.h
	src/window/sdl2/sdl2_display_window.cpp
	src/window/sdl2/sdl2_display_window.h
)

set(SURREALWIDGETS_SDL3_SOURCES
	src/window/sdl3/sdl3_display_backend.cpp
	src/window/sdl3/sdl3_display_backend.h
	src/window/sdl3/sdl3_display_window.cpp
	src/window/sdl3/sdl3_display_window.h
)

set(SURREALWIDGETS_X11_SOURCES
	src/window/x11/x11_display_backend.cpp
	src/window/x11/x11_display_backend.h
	src/window/x11/x11_display_window.cpp
	src/window/x11/x11_display_window.h
	src/window/x11/x11_connection.cpp
	src/window/x11/x11_connection.h
)

set(SURREALWIDGETS_WAYLAND_SOURCES
	src/window/wayland/wayland_display_backend.cpp
	src/window/wayland/wayland_display_backend.h
	src/window/wayland/wayland_display_window.cpp
	src/window/wayland/wayland_display_window.h
	src/window/wayland/wayland_timer.h
	src/window/wayland/wayland_timer.cpp
	src/window/wayland/wl_fractional_scaling_protocol.cpp
	src/window/wayland/wl_fractional_scaling_protocol.hpp
	src/window/wayland/wl_cursor_shape.cpp
	src/window/wayland/wl_cursor_shape.hpp
	src/window/wayland/wl_xdg_dialog_v1.cpp
	src/window/wayland/wl_xdg_dialog_v1.hpp
	src/window/wayland/wl_xdg_toplevel_icon.cpp
	src/window/wayland/wl_xdg_toplevel_icon.hpp
)

source_group("src" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/.+")
source_group("src\\core" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/core/.+")
source_group("src\\core\\picopng" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/core/picopng/.+")
source_group("src\\core\\nanosvg" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/core/nanosvg/.+")
source_group("src\\widgets" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/.+")
source_group("src\\widgets\\dialog" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/dialog/.+")
source_group("src\\widgets\\lineedit" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/lineedit/.+")
source_group("src\\widgets\\mainwindow" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/mainwindow/.+")
source_group("src\\widgets\\menubar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/menubar/.+")
source_group("src\\widgets\\scrollbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/scrollbar/.+")
source_group("src\\widgets\\statusbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/statusbar/.+")
source_group("src\\widgets\\textedit" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/textedit/.+")
source_group("src\\widgets\\toolbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/toolbar/.+")
source_group("src\\widgets\\imagebox" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/imagebox/.+")
source_group("src\\widgets\\textlabel" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/textlabel/.+")
source_group("src\\widgets\\pushbutton" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/pushbutton/.+")
source_group("src\\widgets\\checkboxlabel" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/checkboxlabel/.+")
source_group("src\\widgets\\dropdown" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/dropdown/.+")
source_group("src\\widgets\\listview" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/listview/.+")
source_group("src\\widgets\\tabwidget" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/widgets/tabwidget/.+")
source_group("src\\window" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/.+")
source_group("src\\window\\stub" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/stub/.+")
source_group("src\\window\\win32" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/win32/.+")
source_group("src\\window\\sdl2" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/sdl2/.+")
source_group("src\\window\\sdl3" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/sdl3/.+")
source_group("src\\window\\x11" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/x11/.+")
source_group("src\\window\\wayland" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/wayland/.+")
source_group("src\\window\\dbus" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/window/dbus/.+")
source_group("src\\systemdialogs" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/src/systemdialogs/.+")
source_group("include" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/.+")
source_group("include\\core" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/core/.+")
source_group("include\\widgets" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/.+")
source_group("include\\widgets\\lineedit" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/lineedit/.+")
source_group("include\\widgets\\mainwindow" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/mainwindow/.+")
source_group("include\\widgets\\menubar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/menubar/.+")
source_group("include\\widgets\\scrollbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/scrollbar/.+")
source_group("include\\widgets\\statusbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/statusbar/.+")
source_group("include\\widgets\\textedit" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/textedit/.+")
source_group("include\\widgets\\toolbar" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/toolbar/.+")
source_group("include\\widgets\\imagebox" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/imagebox/.+")
source_group("include\\widgets\\textlabel" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/textlabel/.+")
source_group("include\\widgets\\pushbutton" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/pushbutton/.+")
source_group("include\\widgets\\checkboxlabel" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/checkboxlabel/.+")
source_group("include\\widgets\\listview" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/listview/.+")
source_group("include\\widgets\\tabwidget" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/widgets/tabwidget/.+")
source_group("include\\window" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/window/.+")
source_group("include\\systemdialogs" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/include/surrealwidgets/systemdialogs/.+")

# Include directory to external projects using surrealwidgets
include_directories(include)

# Internal include dirs for building surrealwidgets
set(SURREALWIDGETS_INCLUDE_DIRS include/surrealwidgets src)

set(SURREALWIDGETS_COMPILE_OPTIONS)

if(WIN32)
	list(APPEND SURREALWIDGETS_SOURCES ${SURREALWIDGETS_WIN32_SOURCES})
	set(SURREALWIDGETS_DEFINES -DUNICODE -D_UNICODE)
	set(SURREALWIDGETS_LINK_OPTIONS)
	if(MSVC)
		# Use all cores for compilation
		set(SURREALWIDGETS_COMPILE_OPTIONS ${SURREALWIDGETS_COMPILE_OPTIONS} /MP)

		# Ignore specific warnings
		#set(SURREALWIDGETS_COMPILE_OPTIONS ${SURREALWIDGETS_COMPILE_OPTIONS} /wd4244 /wd4267 /wd4005 /wd4018)

		# Don't slow down std containers in debug builds
		set(SURREALWIDGETS_DEFINES ${SURREALWIDGETS_DEFINES})

		# Ignore warning about legacy CRT functions
		#set(SURREALWIDGETS_DEFINES ${SURREALWIDGETS_DEFINES} -D_CRT_SECURE_NO_WARNINGS)
	endif()
elseif(APPLE)
	list(APPEND SURREALWIDGETS_SOURCES ${SURREALWIDGETS_COCOA_SOURCES} ${SURREALWIDGETS_COCOA_ARC_SOURCES})
	set_source_files_properties(${SURREALWIDGETS_COCOA_ARC_SOURCES} PROPERTIES COMPILE_FLAGS "-x objective-c++ -fobjc-arc")
	set(SURREALWIDGETS_LIBS ${CMAKE_DL_LIBS} -ldl)
	set(SURREALWIDGETS_DEFINES -DUNIX -D_UNIX)
	set(SURREALWIDGETS_LINK_OPTIONS -pthread)

	find_library(COCOA_FRAMEWORK Cocoa)
	if (NOT COCOA_FRAMEWORK)
		message(FATAL_ERROR "Cocoa framework not found")
	endif()

	find_library(COREVIDEO_FRAMEWORK CoreVideo)
	if (NOT COREVIDEO_FRAMEWORK)
		message(FATAL_ERROR "CoreVideo framework not found")
	endif()

	if(ENABLE_METAL)
		find_library(METAL_FRAMEWORK Metal)
		find_library(QUARTZCORE_FRAMEWORK QuartzCore)
		if (NOT QUARTZCORE_FRAMEWORK)
			message(FATAL_ERROR "QuartzCore framework not found")
		endif()
	endif()

	if(ENABLE_OPENGL)
		find_library(OPENGL_FRAMEWORK OpenGL)
		if (NOT OPENGL_FRAMEWORK)
			message(FATAL_ERROR "OpenGL framework not found")
		endif()
	endif()

	find_library(UNIFORMTYPEIDENTIFIERS_FRAMEWORK UniformTypeIdentifiers)
	if (NOT UNIFORMTYPEIDENTIFIERS_FRAMEWORK)
		message(FATAL_ERROR "UniformTypeIdentifiers framework not found")
	endif()
elseif(HAIKU)
	set(SURREALWIDGETS_SOURCES ${SURREALWIDGETS_SOURCES} ${SURREALWIDGETS_UNIX_SOURCES})
	set(SURREALWIDGETS_LIBS ${CMAKE_DL_LIBS})
	set(SURREALWIDGETS_DEFINES -DUNIX -D_UNIX)
	set(SURREALWIDGETS_LINK_OPTIONS -pthread)
else()
	set(SURREALWIDGETS_SOURCES ${SURREALWIDGETS_SOURCES} ${SURREALWIDGETS_UNIX_SOURCES})
	set(SURREALWIDGETS_SOURCES ${SURREALWIDGETS_SOURCES} ${SURREALWIDGETS_X11_SOURCES})
	set(SURREALWIDGETS_LIBS ${CMAKE_DL_LIBS} -lX11 -lXi)
	set(SURREALWIDGETS_DEFINES -DUNIX -D_UNIX -DUSE_X11)
	set(SURREALWIDGETS_LINK_OPTIONS -pthread)
	if (DBUS_FOUND)
		include_directories("/usr/lib64/dbus-1.0/include") # Bazzite (and probably Fedora 42) keeps the platform-specific dbus headers in this folder
		set(SURREALWIDGETS_SOURCES ${SURREALWIDGETS_SOURCES} ${SURREALWIDGETS_DBUS_SOURCES})
		set(SURREALWIDGETS_INCLUDE_DIRS ${SURREALWIDGETS_INCLUDE_DIRS} ${DBUS_INCLUDE_DIRS})
		set(SURREALWIDGETS_LIBS ${SURREALWIDGETS_LIBS} ${DBUS_LDFLAGS})
		set(SURREALWIDGETS_DEFINES ${SURREALWIDGETS_DEFINES} -DUSE_DBUS)
	endif()
	if (WAYLANDPP_FOUND)
		set(SURREALWIDGETS_SOURCES ${SURREALWIDGETS_SOURCES} ${SURREALWIDGETS_WAYLAND_SOURCES})
		set(SURREALWIDGETS_INCLUDE_DIRS ${SURREALWIDGETS_INCLUDE_DIRS} ${WAYLANDPP_INCLUDE_DIRS})
		set(SURREALWIDGETS_LIBS ${SURREALWIDGETS_LIBS} ${WAYLANDPP_LDFLAGS})
		set(SURREALWIDGETS_DEFINES ${SURREALWIDGETS_DEFINES} -DUSE_WAYLAND)
	endif()

	set(SURREALWIDGETS_INCLUDE_DIRS ${SURREALWIDGETS_INCLUDE_DIRS} ${GLIB_INCLUDE_DIRS})
	set(SURREALWIDGETS_LIBS ${SURREALWIDGETS_LIBS} ${GLIB_LDFLAGS} -lgio-2.0)

	set(SURREALWIDGETS_INCLUDE_DIRS ${SURREALWIDGETS_INCLUDE_DIRS} ${FONTCONFIG_INCLUDE_DIRS})
	set(SURREALWIDGETS_LIBS ${SURREALWIDGETS_LIBS} ${FONTCONFIG_LDFLAGS})
endif()

if(SDL3_FOUND)
	set(SURREALWIDGETS_SOURCES ${SURREALWIDGETS_SOURCES} ${SURREALWIDGETS_SDL3_SOURCES})
	set(SURREALWIDGETS_DEFINES ${SURREALWIDGETS_DEFINES} -DUSE_SDL3)
endif()

if(SDL2_FOUND AND NOT SDL3_FOUND)
	set(SURREALWIDGETS_SOURCES ${SURREALWIDGETS_SOURCES} ${SURREALWIDGETS_SDL2_SOURCES})
	set(SURREALWIDGETS_DEFINES ${SURREALWIDGETS_DEFINES} -DUSE_SDL2)
endif()

add_library(surrealwidgets STATIC ${SURREALWIDGETS_SOURCES} ${SURREALWIDGETS_INCLUDES})
target_compile_options(surrealwidgets PRIVATE ${SURREALWIDGETS_COMPILE_OPTIONS})
target_compile_definitions(surrealwidgets PRIVATE ${SURREALWIDGETS_DEFINES})
target_include_directories(surrealwidgets PRIVATE ${SURREALWIDGETS_INCLUDE_DIRS})
target_link_options(surrealwidgets PRIVATE ${SURREALWIDGETS_LINK_OPTIONS})
if(APPLE)
	target_link_libraries(surrealwidgets PRIVATE ${SURREALWIDGETS_LIBS} ${COCOA_FRAMEWORK} ${COREVIDEO_FRAMEWORK} ${UNIFORMTYPEIDENTIFIERS_FRAMEWORK} ${QUARTZCORE_FRAMEWORK})
	if(ENABLE_OPENGL AND OPENGL_FRAMEWORK)
		target_link_libraries(surrealwidgets PRIVATE ${OPENGL_FRAMEWORK})
		target_compile_definitions(surrealwidgets PRIVATE HAVE_OPENGL)
	endif()
	if(ENABLE_METAL AND METAL_FRAMEWORK)
		target_link_libraries(surrealwidgets PRIVATE ${METAL_FRAMEWORK} ${QUARTZCORE_FRAMEWORK})
		target_compile_definitions(surrealwidgets PRIVATE HAVE_METAL)
	endif()
else()
	target_link_libraries(surrealwidgets PRIVATE ${SURREALWIDGETS_LIBS})
endif()
if(SDL3_FOUND)
	target_include_directories(surrealwidgets PRIVATE ${SDL3_INCLUDE_DIRS})
	if(TARGET SDL3::SDL3)
		target_link_libraries(surrealwidgets PRIVATE SDL3::SDL3)
	else()
		target_link_libraries(surrealwidgets PRIVATE ${SDL3_LIBRARY})
	endif()
endif()
if(SDL2_FOUND AND NOT SDL3_FOUND)
	target_include_directories(surrealwidgets PRIVATE ${SDL2_INCLUDE_DIRS})
	if(TARGET SDL2::SDL2)
		target_link_libraries(surrealwidgets PRIVATE SDL2::SDL2)
	else()
		target_link_libraries(surrealwidgets PRIVATE ${SDL2_LIBRARY})
	endif()
endif()
set_target_properties(surrealwidgets PROPERTIES CXX_STANDARD 20)
target_compile_options(surrealwidgets PRIVATE ${CXX_WARNING_FLAGS})

if(MSVC)
	set_property(TARGET surrealwidgets PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
endif()

option(SURREALWIDGETS_BUILD_EXAMPLE "Build the surrealwidgets example application" ON)

if(SURREALWIDGETS_BUILD_EXAMPLE)
	source_group("example" REGULAR_EXPRESSION "${CMAKE_CURRENT_SOURCE_DIR}/example/.+")
	add_executable(surrealwidgets_example WIN32
		example/main.cpp
		example/example.cpp
		example/example.h
	)
	target_compile_options(surrealwidgets_example PRIVATE ${CXX_WARNING_FLAGS})

	set(SURREALWIDGET_EXAMPLE_RESOURCES
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/banner.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/button_disabled.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/button_hot.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/button_normal.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/button_pressed.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/button_pressed_hot.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/checkbox_checked_normal.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/checkbox_unchecked_normal.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/OpenSans.ttf"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram.txt"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram01.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram02.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram03.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram04.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram05.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram06.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram07.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram08.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram09.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram10.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram11.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram12.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram13.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/Pentagram14.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/surreal-engine-icon-16.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/surreal-engine-icon-24.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/surreal-engine-icon-32.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/surreal-engine-icon-48.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/surreal-engine-icon-64.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/surreal-engine-icon-128.png"
		"${CMAKE_CURRENT_SOURCE_DIR}/example/resources/surreal-engine-icon-256.png"
	)

	add_custom_command(
		TARGET surrealwidgets_example POST_BUILD
		COMMAND ${CMAKE_COMMAND} -E copy
			${SURREALWIDGET_EXAMPLE_RESOURCES}
			"${CMAKE_CURRENT_BINARY_DIR}"
	)

	target_include_directories(surrealwidgets_example PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/example)
	target_link_libraries(surrealwidgets_example PRIVATE surrealwidgets)

	if(WIN32)
		target_compile_definitions(surrealwidgets_example PRIVATE UNICODE _UNICODE)
		target_link_libraries(surrealwidgets_example PRIVATE gdi32 user32 shell32 comdlg32)
	elseif(APPLE)
		target_link_libraries(surrealwidgets_example PRIVATE "-framework Cocoa" "-framework CoreVideo")
		if(ENABLE_OPENGL AND OPENGL_FRAMEWORK)
			target_link_libraries(surrealwidgets_example PRIVATE "-framework OpenGL")
		endif()
		if(ENABLE_METAL AND METAL_FRAMEWORK)
			target_link_libraries(surrealwidgets_example PRIVATE "-framework Metal" "-framework QuartzCore")
		endif()
	else()
		target_link_libraries(surrealwidgets_example PRIVATE ${SURREALWIDGETS_LIBS})
	endif()

	if(SDL3_FOUND)
		target_link_libraries(surrealwidgets_example PRIVATE SDL3::SDL3)
	endif()

	if(SDL2_FOUND AND NOT SDL3_FOUND)
		target_link_libraries(surrealwidgets_example PRIVATE SDL2::SDL2)
	endif()

	set_target_properties(surrealwidgets_example PROPERTIES CXX_STANDARD 20)

	if(MSVC)
		set_property(TARGET surrealwidgets_example PROPERTY MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>")
	endif()
endif()
