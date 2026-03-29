#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <unordered_map>
#include <cstddef>

namespace longhorn {

struct CachedTexture {
    unsigned int texture_id = 0;
    int width = 0;
    int height = 0;
};

// Renders text using TTF -> SDL_Surface -> GL texture -> textured quad
class GLText {
public:
    GLText();
    ~GLText();

    GLText(const GLText&) = delete;
    GLText& operator=(const GLText&) = delete;

    bool init(const std::string& font_path, int font_size);

    // Draw text at pixel coordinates (origin = top-left of window)
    void draw(const std::string& text, int x, int y, SDL_Color color, int viewport_w, int viewport_h);

    // Draw a filled rectangle
    void draw_rect(int x, int y, int w, int h, SDL_Color color, int viewport_w, int viewport_h);

    // Measure text width in pixels
    [[nodiscard]] int measure(const std::string& text);

    // Font height in pixels
    [[nodiscard]] int font_height() const;

private:
    TTF_Font* font_ = nullptr;
    unsigned int text_program_ = 0;
    unsigned int rect_program_ = 0;
    unsigned int vao_ = 0;
    unsigned int vbo_ = 0;

    // Text texture cache
    std::unordered_map<std::size_t, CachedTexture> cache_;
    static constexpr std::size_t MAX_CACHE_SIZE = 256;

    std::size_t cache_key(const std::string& text, SDL_Color color);
    void clear_cache();

    bool init_text_shader();
    bool init_rect_shader();
};

} // namespace longhorn
