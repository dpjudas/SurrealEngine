/*
**  Copyright (c) Chu Chin Kuan
**
**  This software is provided 'as-is', without any express or implied
**  warranty.  In no event will the authors be held liable for any damages
**  arising from the use of this software.
**
**  Permission is granted to anyone to use this software for any purpose,
**  including commercial applications, and to alter it and redistribute it
**  freely, subject to the following restrictions:
**
**  1. The origin of this software must not be misrepresented; you must not
**     claim that you wrote the original software. If you use this software
**     in a product, an acknowledgment in the product documentation would be
**     appreciated but is not required.
**  2. Altered source versions must be plainly marked as such, and must not be
**     misrepresented as being the original software.
**  3. This notice may not be removed or altered from any source distribution.
**
*/

#include "X11Atoms.h"

const std::vector< std::string > X11Atoms::_atoms_ =
{
	"WM_PROTOCOLS",

	"WM_DELETE_WINDOW",
	"WM_STATE",


	"_NET_FRAME_EXTENTS",

	//! Requests the WM to calculate frame extents of the window at its current
	//! configuration. Some WMs have _NET_FRAME_EXTENTS set anyway and do not
	//! support this atom.
	"_NET_REQUEST_FRAME_EXTENTS",


	"_NET_WM_PING",

	"_NET_WM_STATE", //! Set by WM, lists the following atoms:
	"_NET_WM_STATE_HIDDEN",
	"_NET_WM_STATE_FULLSCREEN",
	"_NET_WM_STATE_MAXIMIZED_HORZ",
	"_NET_WM_STATE_MAXIMIZED_VERT",
	"_NET_WM_STATE_MODAL",

	"_NET_WM_FULLSCREEN_MONITORS",


	"_NET_WM_WINDOW_TYPE",

	"_NET_WM_WINDOW_TYPE_DESKTOP",
	"_NET_WM_WINDOW_TYPE_DOCK",
	"_NET_WM_WINDOW_TYPE_TOOLBAR",
	"_NET_WM_WINDOW_TYPE_MENU",
	"_NET_WM_WINDOW_TYPE_UTILITY",
	"_NET_WM_WINDOW_TYPE_SPLASH",
	"_NET_WM_WINDOW_TYPE_DIALOG",
	"_NET_WM_WINDOW_TYPE_DROPDOWN_MENU",
	"_NET_WM_WINDOW_TYPE_POPUP_MENU",
	"_NET_WM_WINDOW_TYPE_TOOLTIP",
	"_NET_WM_WINDOW_TYPE_NOTIFICATION",
	"_NET_WM_WINDOW_TYPE_COMBO",
	"_NET_WM_WINDOW_TYPE_DND",
	"_NET_WM_WINDOW_TYPE_NORMAL"
};

unsigned char *X11Atoms::get_property(::Display *display, Window window, Atom property, Atom &actual_type, int &actual_format, unsigned long &item_count)
{
	/* IO */ long  read_bytes = 0; // Request 0 bytes first.
	Atom _actual_type = actual_type;
	int  _actual_format = actual_format;
	unsigned long _item_count = item_count;
	unsigned long  bytes_remaining;
	unsigned char *read_data = NULL;

	do
	{
		int result = XGetWindowProperty(
			display, window, property, 0ul, read_bytes,
			False, AnyPropertyType, &actual_type, &actual_format,
			&_item_count, &bytes_remaining, &read_data
			);

		if (result != Success)
		{
			actual_type = None;
			actual_format = 0;
			item_count = 0;
			return NULL;
		}

		read_bytes = bytes_remaining;
	} while (bytes_remaining > 0);

	item_count = _item_count;
	return read_data;
}

unsigned char *X11Atoms::get_property(::Display *display, Window window, Atom property, unsigned long &item_count)
{
	Atom _actual_type;
	int  _actual_format;

	return X11Atoms::get_property(display, window, property, _actual_type, _actual_format, item_count);
}
