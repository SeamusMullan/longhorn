#include <longhorn/gl_text.hpp>
#include <longhorn/gl_funcs.hpp>

#include <functional>
#include <cstring>

namespace longhorn {

static const char* text_vert_src = R"(
#version 330 core
layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_uv;
out vec2 v_uv;
uniform vec2 u_viewport;
void main() {
    // Convert pixel coords to NDC
    vec2 ndc = (a_pos / u_viewport) * 2.0 - 1.0;
    ndc.y = -ndc.y; // Flip Y (screen coords: top=0)
    gl_Position = vec4(ndc, 0.0, 1.0);
    v_uv = a_uv;
}
)";

static const char* text_frag_src = R"(
#version 330 core
in vec2 v_uv;
out vec4 frag_color;
uniform sampler2D u_texture;
uniform vec4 u_color;
void main() {
    vec4 tex = texture(u_texture, v_uv);
    frag_color = vec4(u_color.rgb, u_color.a * tex.a);
}
)";

static const char* rect_vert_src = R"(
#version 330 core
layout(location = 0) in vec2 a_pos;
uniform vec2 u_viewport;
void main() {
    vec2 ndc = (a_pos / u_viewport) * 2.0 - 1.0;
    ndc.y = -ndc.y;
    gl_Position = vec4(ndc, 0.0, 1.0);
}
)";

static const char* rect_frag_src = R"(
#version 330 core
out vec4 frag_color;
uniform vec4 u_color;
void main() {
    frag_color = u_color;
}
)";

static unsigned int compile(unsigned int type, const char* src) {
    unsigned int s = glCreateShader(type);
    glShaderSource(s, 1, &src, nullptr);
    glCompileShader(s);
    int ok = 0;
    glGetShaderiv(s, GL_COMPILE_STATUS, &ok);
    if (!ok) {
        char log[256];
        glGetShaderInfoLog(s, sizeof(log), nullptr, log);
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Shader: %s", log);
        glDeleteShader(s);
        return 0;
    }
    return s;
}

static unsigned int link(unsigned int v, unsigned int f) {
    unsigned int p = glCreateProgram();
    glAttachShader(p, v);
    glAttachShader(p, f);
    glLinkProgram(p);
    int ok = 0;
    glGetProgramiv(p, GL_LINK_STATUS, &ok);
    if (!ok) {
        glDeleteProgram(p);
        return 0;
    }
    return p;
}

GLText::GLText() = default;

GLText::~GLText() {
    clear_cache();
    if (vbo_) glDeleteBuffers(1, &vbo_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
    if (text_program_) glDeleteProgram(text_program_);
    if (rect_program_) glDeleteProgram(rect_program_);
    if (font_) TTF_CloseFont(font_);
}

std::size_t GLText::cache_key(const std::string& text, SDL_Color color) {
    std::size_t h = std::hash<std::string>{}(text);
    h ^= std::hash<uint32_t>{}(
        (static_cast<uint32_t>(color.r) << 24) |
        (static_cast<uint32_t>(color.g) << 16) |
        (static_cast<uint32_t>(color.b) << 8)  |
        static_cast<uint32_t>(color.a)
    ) + 0x9e3779b9 + (h << 6) + (h >> 2);
    return h;
}

void GLText::clear_cache() {
    for (auto& [key, entry] : cache_) {
        if (entry.texture_id) glDeleteTextures(1, &entry.texture_id);
    }
    cache_.clear();
}

bool GLText::init(const std::string& font_path, int font_size) {
    font_ = TTF_OpenFont(font_path.c_str(), static_cast<float>(font_size));
    if (!font_) return false;

    if (!init_text_shader()) return false;
    if (!init_rect_shader()) return false;

    // Shared quad VAO/VBO
    glGenVertexArrays(1, &vao_);
    glGenBuffers(1, &vbo_);
    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    // 6 vertices * 4 floats (x, y, u, v), will be updated per-draw
    glBufferData(GL_ARRAY_BUFFER, 6 * 4 * sizeof(float), nullptr, GL_DYNAMIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float),
                          reinterpret_cast<void*>(2 * sizeof(float)));
    glBindVertexArray(0);

    return true;
}

bool GLText::init_text_shader() {
    unsigned int v = compile(GL_VERTEX_SHADER, text_vert_src);
    unsigned int f = compile(GL_FRAGMENT_SHADER, text_frag_src);
    if (!v || !f) return false;
    text_program_ = link(v, f);
    glDeleteShader(v);
    glDeleteShader(f);
    return text_program_ != 0;
}

bool GLText::init_rect_shader() {
    unsigned int v = compile(GL_VERTEX_SHADER, rect_vert_src);
    unsigned int f = compile(GL_FRAGMENT_SHADER, rect_frag_src);
    if (!v || !f) return false;
    rect_program_ = link(v, f);
    glDeleteShader(v);
    glDeleteShader(f);
    return rect_program_ != 0;
}

void GLText::draw(const std::string& text, int x, int y, SDL_Color color,
                  int viewport_w, int viewport_h) {
    if (text.empty()) return;

    std::size_t key = cache_key(text, color);
    int tw, th;
    unsigned int tex_id;

    auto it = cache_.find(key);
    if (it != cache_.end()) {
        // Cache hit
        tex_id = it->second.texture_id;
        tw = it->second.width;
        th = it->second.height;
    } else {
        // Cache miss — render and upload
        SDL_Surface* surface = TTF_RenderText_Blended(font_, text.c_str(), text.size(),
                                                       {255, 255, 255, 255});
        if (!surface) return;

        SDL_Surface* rgba = SDL_ConvertSurface(surface, SDL_PIXELFORMAT_RGBA32);
        SDL_DestroySurface(surface);
        if (!rgba) return;

        tw = rgba->w;
        th = rgba->h;

        // Evict all if full
        if (cache_.size() >= MAX_CACHE_SIZE) {
            clear_cache();
        }

        glGenTextures(1, &tex_id);
        glBindTexture(GL_TEXTURE_2D, tex_id);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_RGBA, GL_UNSIGNED_BYTE, rgba->pixels);
        SDL_DestroySurface(rgba);

        cache_[key] = {tex_id, tw, th};
    }

    // Quad vertices: pos(x,y) + uv(u,v)
    float x0 = static_cast<float>(x);
    float y0 = static_cast<float>(y);
    float x1 = x0 + static_cast<float>(tw);
    float y1 = y0 + static_cast<float>(th);

    float verts[] = {
        x0, y0, 0.0f, 0.0f,
        x1, y0, 1.0f, 0.0f,
        x0, y1, 0.0f, 1.0f,
        x1, y0, 1.0f, 0.0f,
        x1, y1, 1.0f, 1.0f,
        x0, y1, 0.0f, 1.0f,
    };

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

    glUseProgram(text_program_);
    float vp[] = {static_cast<float>(viewport_w), static_cast<float>(viewport_h)};
    glUniform2fv(glGetUniformLocation(text_program_, "u_viewport"), 1, vp);
    float col[] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
    glUniform4fv(glGetUniformLocation(text_program_, "u_color"), 1, col);
    glUniform1i(glGetUniformLocation(text_program_, "u_texture"), 0);

    if (glActiveTexture_) glActiveTexture_(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex_id);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
}

void GLText::draw_rect(int x, int y, int w, int h, SDL_Color color,
                       int viewport_w, int viewport_h) {
    float x0 = static_cast<float>(x);
    float y0 = static_cast<float>(y);
    float x1 = x0 + static_cast<float>(w);
    float y1 = y0 + static_cast<float>(h);

    // Only need pos (2 floats), but reuse the 4-float stride VAO
    float verts[] = {
        x0, y0, 0, 0,
        x1, y0, 0, 0,
        x0, y1, 0, 0,
        x1, y0, 0, 0,
        x1, y1, 0, 0,
        x0, y1, 0, 0,
    };

    glBindVertexArray(vao_);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_DYNAMIC_DRAW);

    glUseProgram(rect_program_);
    float vp[] = {static_cast<float>(viewport_w), static_cast<float>(viewport_h)};
    glUniform2fv(glGetUniformLocation(rect_program_, "u_viewport"), 1, vp);
    float col[] = {color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f};
    glUniform4fv(glGetUniformLocation(rect_program_, "u_color"), 1, col);

    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);
    glUseProgram(0);
}

int GLText::measure(const std::string& text) {
    if (text.empty()) return 0;
    int w = 0, h = 0;
    TTF_GetStringSize(font_, text.c_str(), text.size(), &w, &h);
    return w;
}

int GLText::font_height() const {
    return static_cast<int>(TTF_GetFontHeight(font_));
}

} // namespace longhorn
