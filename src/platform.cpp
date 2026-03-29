#include <longhorn/platform.hpp>
#include <SDL3/SDL.h>
#include <cstring>

// Forward declarations for platform backends
namespace longhorn {
#ifdef HAS_WAYLAND
void platform_setup_wayland(SDL_Window* window, bool bottom, int bar_height);
#endif
#ifdef HAS_X11
void platform_setup_x11(SDL_Window* window, bool bottom, int bar_height);
#endif
} // namespace longhorn

namespace longhorn {

void platform_setup_window(SDL_Window* window, bool bottom, int bar_height) {
    const char* driver = SDL_GetCurrentVideoDriver();
    if (!driver) {
        SDL_Log("platform: no video driver detected, skipping platform setup");
        return;
    }

#ifdef HAS_WAYLAND
    if (std::strcmp(driver, "wayland") == 0) {
        platform_setup_wayland(window, bottom, bar_height);
        return;
    }
#endif

#ifdef HAS_X11
    if (std::strcmp(driver, "x11") == 0) {
        platform_setup_x11(window, bottom, bar_height);
        return;
    }
#endif

    SDL_Log("platform: no platform backend for driver '%s'", driver);
}

} // namespace longhorn
