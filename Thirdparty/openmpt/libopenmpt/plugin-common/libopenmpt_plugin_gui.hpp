/*
 * libopenmpt_plugin_gui.hpp
 * -------------------------
 * Purpose: libopenmpt plugin GUI
 * Notes  : (currently none)
 * Authors: OpenMPT Devs
 * The OpenMPT source code is released under the BSD license. Read LICENSE for more details.
 */

#ifndef LIBOPENMPT_PLUGIN_GUI_HPP
#define LIBOPENMPT_PLUGIN_GUI_HPP

#include "libopenmpt_plugin_settings.hpp"

#include <windows.h>

#include <string>

namespace libopenmpt {
namespace plugin {

#if defined(MPT_WITH_MFC)

void DllMainAttach();
void DllMainDetach();

#endif // MPT_WITH_MFC

void gui_edit_settings( libopenmpt_settings * s, HWND parent, std::basic_string<TCHAR> title );

void gui_show_file_info( HWND parent, std::basic_string<TCHAR> title, std::basic_string<TCHAR> info );

void gui_show_about( HWND parent, std::basic_string<TCHAR> title, std::basic_string<TCHAR> about, std::basic_string<TCHAR> credits );

} // namespace plugin
} // namespace libopenmpt

#endif // LIBOPENMPT_PLUGIN_GUI_HPP
