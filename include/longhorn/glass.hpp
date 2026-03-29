#pragma once

#include <longhorn/geometry.hpp>
#include <SDL3/SDL.h>

namespace longhorn {

// Uniform data for the frosted glass shader
struct GlassUniforms {
    float rect[4];     // x, y, w, h in pixels
    float screen[2];   // display width, height
    float radius;      // corner radius
    float blur_radius;
    float tint[4];     // rgba
    float time;
};

// OpenGL-based frosted glass renderer
class GlassRenderer {
public:
    GlassRenderer();
    ~GlassRenderer();

    GlassRenderer(const GlassRenderer&) = delete;
    GlassRenderer& operator=(const GlassRenderer&) = delete;

    bool init();

    void update(const PixelRect& rect, int screen_w, int screen_h, float time);
    void render();

    void set_tint(float r, float g, float b, float a);
    void set_blur_radius(float radius);
    void set_corner_radius(float radius);

private:
    unsigned int program_ = 0;
    unsigned int vao_ = 0;
    GlassUniforms uniforms_{};

    // Uniform locations
    int loc_rect_ = -1;
    int loc_screen_ = -1;
    int loc_radius_ = -1;
    int loc_blur_ = -1;
    int loc_tint_ = -1;
    int loc_time_ = -1;
};

} // namespace longhorn
