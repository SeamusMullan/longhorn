#pragma once

#include <string>

namespace longhorn {

// Normalized rect: values in [0,1] relative to screen dimensions
struct NormalizedRect {
    float x = 0.0f;
    float y = 0.0f;
    float w = 1.0f;
    float h = 0.0f; // 0 = auto-size based on content
};

// Concrete pixel rect after resolving against display
struct PixelRect {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
};

enum class LayoutPreset {
    TopBar,      // Full-width bar at top (classic dmenu)
    BottomBar,   // Full-width bar at bottom
    Centered,    // Centered floating box (spotlight-style)
    LeftColumn,  // Left-side vertical panel
    RightColumn, // Right-side vertical panel
    Custom,      // User-defined NormalizedRect
};

struct Layout {
    LayoutPreset preset = LayoutPreset::TopBar;
    NormalizedRect custom_rect; // Only used when preset == Custom
    float corner_radius = 0.0f; // px, 0 for bar modes
    float padding = 8.0f;

    // Resolve this layout to pixel coordinates given a display size
    [[nodiscard]] PixelRect resolve(int display_w, int display_h, int bar_height) const;

    // Get preset by name (for config files)
    [[nodiscard]] static LayoutPreset preset_from_string(const std::string& name);
};

// Interpolation between two pixel rects for smooth transitions
struct GeometryState {
    PixelRect current;
    PixelRect target;
    float t = 1.0f; // 0 = at current, 1 = at target (starts "arrived")
    float speed = 8.0f; // interpolation speed

    void set_target(PixelRect new_target);
    void update(float dt);

    // Get the interpolated rect for rendering
    [[nodiscard]] PixelRect interpolated() const;
};

} // namespace longhorn
