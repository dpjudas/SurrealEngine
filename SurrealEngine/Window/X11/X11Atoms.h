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

#include <algorithm>
#include <cassert>
#include <cstring>
#include <map>
#include <string>
#include <vector>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

using X11AtomMap = std::map< std::string, Atom >;

class X11Atoms
{
public:
	X11Atoms() : _display_(nullptr) {  }
	X11Atoms(::Display *display) : _display_(display)
	{
		//log_event("debug", "Initializing X11 Display Atoms...");
		for (const auto &elem : _atoms_)
		{
			_map_[elem] = XInternAtom(_display_, elem.c_str(), True);
			//log_event("debug", "  %1\t: %2", elem, (_map_[elem] == None) ? "None" : "OK");
		}
	}

	Atom &operator[](const std::string &elem)
	{
		auto iter = _map_.find(elem);
		assert(iter != _map_.end());
		return iter->second;
	}

	const Atom &operator[](const std::string &elem) const
	{
		auto iter = _map_.find(elem);
		assert(iter != _map_.end());
		return iter->second;
	}

	bool exists(const std::string &elem) const
	{
		auto iter = _map_.find(elem);
		if (iter != _map_.end())
			return iter->second != None;
		else
			return false;
	}

	Atom get_atom(::Display *display, const char *elem, bool only_if_exists)
	{
		assert(display == _display_); // Safety check.
		auto iter = _map_.find(elem);
		if (iter != _map_.end())
		{
			return iter->second;
		}
		else
		{
			Atom atom = XInternAtom(display, elem, only_if_exists);
			_map_[std::string(elem)] = atom;
			return atom;
		}
	}

	void clear()
	{
		_map_.clear();
	}

	// Important: Use XFree() on the returned pointer (if not NULL)
	static unsigned char *get_property(::Display *display, Window window, Atom property, Atom &actual_type, int &actual_format, unsigned long &item_count);
	static unsigned char *get_property(::Display *display, Window window, Atom property, unsigned long &item_count);
	unsigned char *get_property(Window window, const std::string &property, unsigned long &item_count) const
	{
		return get_property(_display_, window, (*this)[property], item_count);
	}

	//////////////////////////
	// _NET_WM_STATE methods
	//////////////////////////
	std::vector<bool> check_net_wm_state(Window window, std::vector<std::string> state_atoms) const
	{
		// Atom not in _NET_WM_STATE MUST be considered not set.
		std::vector< bool > states(state_atoms.size(), false);

		if ((*this)["_NET_WM_STATE"] == None)
		{
			//log_event("debug", "clan::X11Window::check_net_wm_state(): _NET_WM_STATE not provided by WM.");
			return states;
		}

		// Get window states from WM
		unsigned long  item_count;
		unsigned char *data = get_property(window, "_NET_WM_STATE", item_count);
		if (data == NULL)
		{
			//log_event("debug", "clan::X11Atoms::check_net_wm_state(): Failed to query _NET_WM_STATE.");
			return states;
		}

		unsigned long *items = (unsigned long *)data;

		// Map each state atom to state boolean.
		for (size_t i = 0; i < state_atoms.size(); i++)
		{
			const std::string &elem = state_atoms[i];
			Atom state = static_cast<unsigned long>((*this)[elem]);
			if (state == None)
			{
				//log_event("debug", "clan::X11Atoms::check_net_wm_state(): %1 is not provided by WM.", elem);
				continue; // Unsupported states are not queried.
			}

			auto it = std::find(items, items + item_count, state);
			states[i] = (it != items + item_count);
		}

		XFree(data);
		return states;
	}

	bool modify_net_wm_state(Window window, long action, const std::string &atom1, const std::string &atom2 = None)
	{
		Atom _NET_WM_STATE = (*this)["_NET_WM_STATE"];

		if (_NET_WM_STATE == None)
			return false;

		XEvent xevent;
		memset(&xevent, 0, sizeof(xevent));
		xevent.xclient.type = ClientMessage;
		xevent.xclient.window = window;
		xevent.xclient.message_type = _NET_WM_STATE;
		xevent.xclient.format = 32;
		xevent.xclient.data.l[0] =
			xevent.xclient.data.l[1] = (*this)[atom1];
		xevent.xclient.data.l[2] = (*this)[atom2];
		xevent.xclient.data.l[3] = 0; // or 2

		Status ret = XSendEvent(
			_display_, DefaultRootWindow(_display_), False,
			SubstructureNotifyMask | SubstructureRedirectMask, &xevent
			);

		XFlush(_display_);

		if (ret == 0)
		{
			//log_event("debug", "clan::X11Atoms::modify_net_wm_state(): XSendEvent failed.");
			return false;
		}
		else
		{
			return true;
		}
	}

public:
	//! List of all atoms handled by UICore.
	static const std::vector< std::string > _atoms_;

private:
	::Display* _display_;
	X11AtomMap _map_;
};
