#include <longhorn/platform.hpp>
#include <SDL3/SDL.h>

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>

namespace longhorn {

void platform_setup_x11(SDL_Window* window, bool bottom, int bar_height) {
    SDL_PropertiesID props = SDL_GetWindowProperties(window);

    auto* display = static_cast<Display*>(
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_X11_DISPLAY_POINTER, nullptr));
    auto xwindow = static_cast<Window>(
        SDL_GetNumberProperty(props, SDL_PROP_WINDOW_X11_WINDOW_NUMBER, 0));

    if (!display || xwindow == 0) {
        SDL_Log("platform_x11: could not get X11 Display/Window from SDL");
        return;
    }

    // Override-redirect: bypass window manager
    XSetWindowAttributes attrs{};
    attrs.override_redirect = True;
    XChangeWindowAttributes(display, xwindow, CWOverrideRedirect, &attrs);

    // _NET_WM_WINDOW_TYPE = _NET_WM_WINDOW_TYPE_DOCK
    Atom wm_type = XInternAtom(display, "_NET_WM_WINDOW_TYPE", False);
    Atom wm_dock = XInternAtom(display, "_NET_WM_WINDOW_TYPE_DOCK", False);
    XChangeProperty(display, xwindow, wm_type, XA_ATOM, 32,
                    PropModeReplace, reinterpret_cast<unsigned char*>(&wm_dock), 1);

    // Get screen dimensions for strut
    Screen* screen = DefaultScreenOfDisplay(display);
    int screen_h = screen->height;

    // _NET_WM_STRUT_PARTIAL: reserve space for the bar
    // Format: left, right, top, bottom, left_start_y, left_end_y,
    //         right_start_y, right_end_y, top_start_x, top_end_x,
    //         bottom_start_x, bottom_end_x
    long strut[12] = {};
    if (bottom) {
        strut[3] = bar_height;                      // bottom
        strut[10] = 0;                              // bottom_start_x
        strut[11] = screen->width - 1;              // bottom_end_x
    } else {
        strut[2] = bar_height;                      // top
        strut[8] = 0;                               // top_start_x
        strut[9] = screen->width - 1;               // top_end_x
    }

    Atom strut_partial = XInternAtom(display, "_NET_WM_STRUT_PARTIAL", False);
    XChangeProperty(display, xwindow, strut_partial, XA_CARDINAL, 32,
                    PropModeReplace, reinterpret_cast<unsigned char*>(strut), 12);

    // Also set basic _NET_WM_STRUT for older WMs
    Atom strut_atom = XInternAtom(display, "_NET_WM_STRUT", False);
    XChangeProperty(display, xwindow, strut_atom, XA_CARDINAL, 32,
                    PropModeReplace, reinterpret_cast<unsigned char*>(strut), 4);

    XFlush(display);

    (void)screen_h;

    SDL_Log("platform_x11: set override-redirect, dock type, and strut (%s, height=%d)",
            bottom ? "bottom" : "top", bar_height);
}

} // namespace longhorn
