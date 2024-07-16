#pragma once

struct wl_display;
struct wl_surface;

class WaylandNativeHandle
{
public:
	wl_display* display = nullptr;
	wl_surface* surface = nullptr;
};
