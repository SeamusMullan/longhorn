#pragma once

#include <SDL3/SDL.h>

namespace longhorn {

/// Call after SDL_CreateWindow to apply platform-specific window properties
/// (layer-shell on Wayland, override-redirect + struts on X11).
void platform_setup_window(SDL_Window* window, bool bottom, int bar_height);

} // namespace longhorn
