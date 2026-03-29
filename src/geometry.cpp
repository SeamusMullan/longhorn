#include <longhorn/geometry.hpp>
#include <algorithm>
#include <cmath>
#include <unordered_map>

namespace longhorn {

PixelRect Layout::resolve(int display_w, int display_h, int bar_height) const {
    switch (preset) {
        case LayoutPreset::TopBar:
            return {0, 0, display_w, bar_height};

        case LayoutPreset::BottomBar:
            return {0, display_h - bar_height, display_w, bar_height};

        case LayoutPreset::Centered: {
            int w = static_cast<int>(display_w * 0.5f);
            int h = bar_height * 2;
            return {(display_w - w) / 2, display_h / 3, w, h};
        }

        case LayoutPreset::LeftColumn: {
            int w = static_cast<int>(display_w * 0.25f);
            return {0, 0, w, display_h};
        }

        case LayoutPreset::RightColumn: {
            int w = static_cast<int>(display_w * 0.25f);
            return {display_w - w, 0, w, display_h};
        }

        case LayoutPreset::Custom: {
            int x = static_cast<int>(custom_rect.x * display_w);
            int y = static_cast<int>(custom_rect.y * display_h);
            int w = static_cast<int>(custom_rect.w * display_w);
            int h = custom_rect.h > 0.0f
                ? static_cast<int>(custom_rect.h * display_h)
                : bar_height;
            return {x, y, w, h};
        }
    }
    return {0, 0, display_w, bar_height};
}

LayoutPreset Layout::preset_from_string(const std::string& name) {
    static const std::unordered_map<std::string, LayoutPreset> map = {
        {"top",     LayoutPreset::TopBar},
        {"bottom",  LayoutPreset::BottomBar},
        {"center",  LayoutPreset::Centered},
        {"left",    LayoutPreset::LeftColumn},
        {"right",   LayoutPreset::RightColumn},
        {"custom",  LayoutPreset::Custom},
    };
    auto it = map.find(name);
    return it != map.end() ? it->second : LayoutPreset::TopBar;
}

// Smooth interpolation with ease-out
static float lerp(float a, float b, float t) {
    return a + (b - a) * t;
}

static int lerp_int(int a, int b, float t) {
    return static_cast<int>(std::round(lerp(static_cast<float>(a), static_cast<float>(b), t)));
}

void GeometryState::set_target(PixelRect new_target) {
    if (new_target.x == target.x && new_target.y == target.y &&
        new_target.w == target.w && new_target.h == target.h) {
        return;
    }
    current = interpolated();
    target = new_target;
    t = 0.0f;
}

void GeometryState::update(float dt) {
    if (t >= 1.0f) return;
    // Exponential ease-out
    t = std::min(1.0f, t + dt * speed * (1.0f - t * 0.5f));
    if (t > 0.999f) t = 1.0f;
}

PixelRect GeometryState::interpolated() const {
    if (t >= 1.0f) return target;
    // Smooth-step for extra polish
    float s = t * t * (3.0f - 2.0f * t);
    return {
        lerp_int(current.x, target.x, s),
        lerp_int(current.y, target.y, s),
        lerp_int(current.w, target.w, s),
        lerp_int(current.h, target.h, s),
    };
}

} // namespace longhorn
