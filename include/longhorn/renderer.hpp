#pragma once

#include <SDL3/SDL.h>
#include <longhorn/geometry.hpp>
#include <longhorn/glass.hpp>
#include <longhorn/gl_text.hpp>
#include <memory>
#include <string>
#include <span>

namespace longhorn {

struct RenderState {
    std::string input;
    std::span<const std::string> matches;
    int selected = 0;
    std::string prompt;
};

class Renderer {
public:
    Renderer(const Layout& layout, const std::string& font_path, int font_size);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void draw(const RenderState& state, float time);

    void set_layout(const Layout& layout);
    void update_geometry(float dt);

    [[nodiscard]] SDL_Window* window() const { return window_; }
    [[nodiscard]] int display_width() const { return display_w_; }
    [[nodiscard]] int display_height() const { return display_h_; }

private:
    SDL_Window* window_ = nullptr;
    SDL_GLContext gl_context_ = nullptr;

    std::unique_ptr<GlassRenderer> glass_;
    std::unique_ptr<GLText> text_;
    GeometryState geometry_;
    int display_w_ = 0;
    int display_h_ = 0;
    int bar_height_ = 32;

    void draw_text_overlay(const RenderState& state);
    static std::string find_font(const std::string& font_path);
};

} // namespace longhorn
