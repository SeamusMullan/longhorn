#include <longhorn/renderer.hpp>
#include <longhorn/gl_funcs.hpp>
#include <stdexcept>
#include <cmath>
#include <algorithm>

namespace longhorn {

static constexpr SDL_Color TEXT_COLOR    = {255, 255, 255, 255};
static constexpr SDL_Color PROMPT_COLOR  = {120, 180, 255, 255};
static constexpr SDL_Color SEL_BG_COLOR  = {60, 100, 180, 180};
static constexpr SDL_Color SEL_COLOR     = {255, 255, 255, 255};
static constexpr SDL_Color MATCH_COLOR   = {180, 200, 240, 200};
static constexpr SDL_Color BORDER_COLOR  = {100, 140, 220, 180};

std::string Renderer::find_font(const std::string& font_path) {
    if (!font_path.empty()) return font_path;
    static const char* fallbacks[] = {
        "/usr/share/fonts/TTF/JetBrainsMono-Regular.ttf",
        "/usr/share/fonts/truetype/dejavu/DejaVuSansMono.ttf",
        "/usr/share/fonts/TTF/DejaVuSansMono.ttf",
        "/usr/share/fonts/liberation-mono/LiberationMono-Regular.ttf",
        nullptr
    };
    for (auto* fb = fallbacks; *fb; ++fb) {
        auto* io = SDL_IOFromFile(*fb, "rb");
        if (io) { SDL_CloseIO(io); return *fb; }
    }
    throw std::runtime_error("No font found");
}

Renderer::Renderer(const Layout& layout, const std::string& font_path, int font_size, int lines)
    : lines_(lines) {
    if (!SDL_Init(SDL_INIT_VIDEO))
        throw std::runtime_error(std::string("SDL_Init: ") + SDL_GetError());
    if (!TTF_Init())
        throw std::runtime_error(std::string("TTF_Init: ") + SDL_GetError());

    // Display info
    SDL_DisplayID display = SDL_GetPrimaryDisplay();
    const SDL_DisplayMode* mode = SDL_GetCurrentDisplayMode(display);
    if (!mode) throw std::runtime_error("Failed to get display mode");
    display_w_ = mode->w;
    display_h_ = mode->h;

    // Resolve layout
    bar_height_ = (lines_ > 0) ? font_size * 2 * (lines_ + 1) : font_size * 2;
    PixelRect rect = layout.resolve(display_w_, display_h_, bar_height_);
    geometry_.current = rect;
    geometry_.target = rect;

    // GL attributes for transparency
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    // Transparent OpenGL window
    Uint32 flags = SDL_WINDOW_OPENGL | SDL_WINDOW_BORDERLESS |
                   SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT;
    window_ = SDL_CreateWindow("longhorn", rect.w, rect.h, flags);
    if (!window_) {
        flags &= ~SDL_WINDOW_TRANSPARENT;
        window_ = SDL_CreateWindow("longhorn", rect.w, rect.h, flags);
    }
    if (!window_)
        throw std::runtime_error(std::string("SDL_CreateWindow: ") + SDL_GetError());
    SDL_SetWindowPosition(window_, rect.x, rect.y);

    // GL context
    gl_context_ = SDL_GL_CreateContext(window_);
    if (!gl_context_)
        throw std::runtime_error(std::string("GL context: ") + SDL_GetError());
    SDL_GL_SetSwapInterval(1);

    if (!load_gl_functions())
        throw std::runtime_error("Failed to load GL functions");

    // Glass shader
    glass_ = std::make_unique<GlassRenderer>();
    glass_->set_corner_radius(layout.corner_radius);
    if (!glass_->init())
        throw std::runtime_error("Failed to init glass renderer");

    // Text renderer
    auto fpath = find_font(font_path);
    text_ = std::make_unique<GLText>();
    if (!text_->init(fpath, font_size))
        throw std::runtime_error("Failed to init text renderer");
}

Renderer::~Renderer() {
    glass_.reset();
    text_.reset();
    if (gl_context_) SDL_GL_DestroyContext(gl_context_);
    if (window_) SDL_DestroyWindow(window_);
    TTF_Quit();
    SDL_Quit();
}

void Renderer::set_layout(const Layout& layout) {
    PixelRect target = layout.resolve(display_w_, display_h_, bar_height_);
    geometry_.set_target(target);
    glass_->set_corner_radius(layout.corner_radius);
}

void Renderer::set_tint(float r, float g, float b, float a) {
    glass_->set_tint(r, g, b, a);
}

void Renderer::update_geometry(float dt) {
    geometry_.update(dt);
    auto rect = geometry_.interpolated();
    SDL_SetWindowPosition(window_, rect.x, rect.y);
    SDL_SetWindowSize(window_, rect.w, rect.h);
}

void Renderer::draw_text_overlay(const RenderState& state) {
    auto rect = geometry_.interpolated();
    int vw = rect.w;
    int vh = rect.h;

    if (lines_ > 0) {
        draw_vertical_overlay(state, vw, vh);
    } else {
        draw_horizontal_overlay(state, vw, vh);
    }
}

void Renderer::draw_horizontal_overlay(const RenderState& state, int vw, int vh) {
    int font_h = text_->font_height();
    int x = 8;
    int y = (vh - font_h) / 2;

    // Prompt
    std::string prompt_str = state.prompt + " ";
    text_->draw(prompt_str, x, y, PROMPT_COLOR, vw, vh);
    x += text_->measure(prompt_str);

    // Input
    int input_start_x = x;
    text_->draw(state.input, x, y, TEXT_COLOR, vw, vh);

    // Blinking cursor
    if (std::sin(state.time * 6.0f) > 0.0f) {
        int cursor_x = input_start_x + text_->measure(state.input.substr(0, state.cursor_pos));
        text_->draw_rect(cursor_x, y, 2, font_h, TEXT_COLOR, vw, vh);
    }
    x += text_->measure(state.input) + 16;

    // Separator
    text_->draw("|", x, y, BORDER_COLOR, vw, vh);
    x += 16;

    // Matches
    for (std::size_t i = 0; i < state.matches.size(); ++i) {
        const auto& item = state.matches[i];
        bool selected = (static_cast<int>(i) == state.selected);

        int item_w = text_->measure(item);
        if (selected) {
            text_->draw_rect(x - 4, y - 2, item_w + 8, font_h + 4, SEL_BG_COLOR, vw, vh);
        }

        if (state.item_boxes) {
            state.item_boxes->push_back({
                static_cast<int>(i),
                {x - 4, y - 2, item_w + 8, font_h + 4}
            });
        }

        text_->draw(item, x, y, selected ? SEL_COLOR : MATCH_COLOR, vw, vh);
        x += item_w + 16;
    }
}

void Renderer::draw_vertical_overlay(const RenderState& state, int vw, int vh) {
    int font_h = text_->font_height();
    int line_h = font_h * 2;
    int x = 8;
    int y = (line_h - font_h) / 2;

    // First line: prompt + input
    std::string prompt_str = state.prompt + " ";
    text_->draw(prompt_str, x, y, PROMPT_COLOR, vw, vh);
    
    int input_start_x = x + text_->measure(prompt_str);
    text_->draw(state.input, input_start_x, y, TEXT_COLOR, vw, vh);

    // Blinking cursor
    if (std::sin(state.time * 6.0f) > 0.0f) {
        int cursor_x = input_start_x + text_->measure(state.input.substr(0, state.cursor_pos));
        text_->draw_rect(cursor_x, y, 2, font_h, TEXT_COLOR, vw, vh);
    }

    // Match lines
    int visible = lines_;
    int offset = state.scroll_offset;
    int end = std::min(offset + visible, static_cast<int>(state.matches.size()));

    for (int i = offset; i < end; ++i) {
        int line_idx = i - offset + 1; // +1 for prompt line
        int ly = line_idx * line_h + (line_h - font_h) / 2;
        const auto& item = state.matches[i];
        bool selected = (i == state.selected);

        int item_w = text_->measure(item);
        if (selected) {
            text_->draw_rect(x - 4, ly - 2, item_w + 8, font_h + 4, SEL_BG_COLOR, vw, vh);
        }

        if (state.item_boxes) {
            state.item_boxes->push_back({
                i,
                {x - 4, ly - 2, vw - x, font_h + 4} // Entire line width clickable
            });
        }

        text_->draw(item, x, ly, selected ? SEL_COLOR : MATCH_COLOR, vw, vh);
    }
}

void Renderer::draw(const RenderState& state, float time) {
    auto rect = geometry_.interpolated();

    SDL_GL_MakeCurrent(window_, gl_context_);
    glViewport(0, 0, rect.w, rect.h);
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Pass 1: frosted glass background
    glass_->update(rect, display_w_, display_h_, time);
    glass_->render();

    // Pass 2: text overlay
    draw_text_overlay(state);

    SDL_GL_SwapWindow(window_);
}

} // namespace longhorn
