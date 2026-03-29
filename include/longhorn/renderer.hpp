#pragma once

#include <SDL3/SDL.h>
#include <longhorn/geometry.hpp>
#include <longhorn/glass.hpp>
#include <longhorn/gl_text.hpp>
#include <memory>
#include <string>
#include <span>
#include <vector>
#include <utility>

namespace longhorn {

struct RenderState {
    std::string input;
    std::span<const std::string> matches;
    int selected = 0;
    std::string prompt;
    int lines = 0;          // 0 = horizontal, >0 = vertical with N visible lines
    int scroll_offset = 0;  // first visible match index in vertical mode
    int cursor_pos = 0;
    float time = 0.0f;
    std::vector<std::pair<int,int>>* item_positions = nullptr;
};

class Renderer {
public:
    Renderer(const Layout& layout, const std::string& font_path, int font_size, int lines = 0);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void draw(const RenderState& state, float time);

    void set_layout(const Layout& layout);
    void set_tint(float r, float g, float b, float a);
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
    int lines_ = 0;

    void draw_text_overlay(const RenderState& state);
    void draw_horizontal_overlay(const RenderState& state, int vw, int vh);
    void draw_vertical_overlay(const RenderState& state, int vw, int vh);
    static std::string find_font(const std::string& font_path);
};

} // namespace longhorn
