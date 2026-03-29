#include <longhorn/platform.hpp>
#include <cstring>
#include <SDL3/SDL.h>

#include <wayland-client.h>
#define namespace namespace_
#include "wlr-layer-shell-unstable-v1-client-protocol.h"
#undef namespace

namespace longhorn {

// -- registry listener to find zwlr_layer_shell_v1 --

struct WaylandState {
    zwlr_layer_shell_v1* layer_shell = nullptr;
};

static void registry_global(void* data, wl_registry* registry,
                            uint32_t name, const char* interface, uint32_t version) {
    auto* state = static_cast<WaylandState*>(data);
    if (std::strcmp(interface, zwlr_layer_shell_v1_interface.name) == 0) {
        state->layer_shell = static_cast<zwlr_layer_shell_v1*>(
            wl_registry_bind(registry, name, &zwlr_layer_shell_v1_interface,
                             version < 4 ? version : 4));
    }
}

static void registry_global_remove(void*, wl_registry*, uint32_t) {}

static const wl_registry_listener registry_listener = {
    registry_global,
    registry_global_remove,
};

// -- layer surface listener --

static void layer_surface_configure(void*, zwlr_layer_surface_v1* surface,
                                    uint32_t serial, uint32_t /*w*/, uint32_t /*h*/) {
    zwlr_layer_surface_v1_ack_configure(surface, serial);
}

static void layer_surface_closed(void*, zwlr_layer_surface_v1* surface) {
    zwlr_layer_surface_v1_destroy(surface);
}

static const zwlr_layer_surface_v1_listener layer_surface_listener = {
    layer_surface_configure,
    layer_surface_closed,
};

void platform_setup_wayland(SDL_Window* window, bool bottom, int bar_height) {
    SDL_PropertiesID props = SDL_GetWindowProperties(window);

    auto* display = static_cast<wl_display*>(
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_DISPLAY_POINTER, nullptr));
    auto* surface = static_cast<wl_surface*>(
        SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WAYLAND_SURFACE_POINTER, nullptr));

    if (!display || !surface) {
        SDL_Log("platform_wayland: could not get wl_display/wl_surface from SDL");
        return;
    }

    // Bind to layer-shell
    wl_registry* registry = wl_display_get_registry(display);
    WaylandState state{};
    wl_registry_add_listener(registry, &registry_listener, &state);
    wl_display_roundtrip(display);
    wl_registry_destroy(registry);

    if (!state.layer_shell) {
        SDL_Log("platform_wayland: zwlr_layer_shell_v1 not available — "
                "compositor may not support wlr-layer-shell");
        return;
    }

    // Create layer surface
    auto* layer_surface = zwlr_layer_shell_v1_get_layer_surface(
        state.layer_shell, surface, nullptr,
        ZWLR_LAYER_SHELL_V1_LAYER_OVERLAY, "longhorn");

    // Anchor to top or bottom, plus left+right for full width
    uint32_t anchor = ZWLR_LAYER_SURFACE_V1_ANCHOR_LEFT |
                      ZWLR_LAYER_SURFACE_V1_ANCHOR_RIGHT;
    if (bottom)
        anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_BOTTOM;
    else
        anchor |= ZWLR_LAYER_SURFACE_V1_ANCHOR_TOP;

    zwlr_layer_surface_v1_set_anchor(layer_surface, anchor);
    zwlr_layer_surface_v1_set_size(layer_surface, 0, bar_height);
    zwlr_layer_surface_v1_set_exclusive_zone(layer_surface, bar_height);
    zwlr_layer_surface_v1_set_keyboard_interactivity(
        layer_surface, ZWLR_LAYER_SURFACE_V1_KEYBOARD_INTERACTIVITY_ON_DEMAND);

    zwlr_layer_surface_v1_add_listener(layer_surface, &layer_surface_listener, nullptr);

    wl_surface_commit(surface);
    wl_display_roundtrip(display);

    SDL_Log("platform_wayland: layer-shell surface created (%s, height=%d)",
            bottom ? "bottom" : "top", bar_height);
}

} // namespace longhorn
