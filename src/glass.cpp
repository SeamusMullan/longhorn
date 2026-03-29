#include <longhorn/glass.hpp>

#include <longhorn/gl_funcs.hpp>

// Embedded shader sources (generated at build time)
extern "C" {
extern const char glass_vert_glsl[];
extern const unsigned int glass_vert_glsl_len;
extern const char glass_frag_glsl[];
extern const unsigned int glass_frag_glsl_len;
}

namespace longhorn {

static unsigned int compile_shader(unsigned int type, const char* source, int length) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, &length);
    glCompileShader(shader);

    int success = 0;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), nullptr, log);
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Shader compile error: %s", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static unsigned int link_program(unsigned int vert, unsigned int frag) {
    unsigned int prog = glCreateProgram();
    glAttachShader(prog, vert);
    glAttachShader(prog, frag);
    glLinkProgram(prog);

    int success = 0;
    glGetProgramiv(prog, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(prog, sizeof(log), nullptr, log);
        SDL_LogError(SDL_LOG_CATEGORY_RENDER, "Program link error: %s", log);
        glDeleteProgram(prog);
        return 0;
    }
    return prog;
}

GlassRenderer::GlassRenderer() {
    uniforms_.tint[0] = 0.15f;
    uniforms_.tint[1] = 0.20f;
    uniforms_.tint[2] = 0.35f;
    uniforms_.tint[3] = 0.4f;
    uniforms_.blur_radius = 8.0f;
    uniforms_.radius = 0.0f;
}

GlassRenderer::~GlassRenderer() {
    if (program_) glDeleteProgram(program_);
    if (vao_) glDeleteVertexArrays(1, &vao_);
}

bool GlassRenderer::init() {
    // Compile shaders from embedded sources
    unsigned int vert = compile_shader(GL_VERTEX_SHADER, glass_vert_glsl, static_cast<int>(glass_vert_glsl_len));
    if (!vert) return false;

    unsigned int frag = compile_shader(GL_FRAGMENT_SHADER, glass_frag_glsl, static_cast<int>(glass_frag_glsl_len));
    if (!frag) {
        glDeleteShader(vert);
        return false;
    }

    program_ = link_program(vert, frag);
    glDeleteShader(vert);
    glDeleteShader(frag);
    if (!program_) return false;

    // Cache uniform locations
    loc_rect_ = glGetUniformLocation(program_, "u_rect");
    loc_screen_ = glGetUniformLocation(program_, "u_screen");
    loc_radius_ = glGetUniformLocation(program_, "u_radius");
    loc_blur_ = glGetUniformLocation(program_, "u_blur");
    loc_tint_ = glGetUniformLocation(program_, "u_tint");
    loc_time_ = glGetUniformLocation(program_, "u_time");

    // Empty VAO for the full-screen triangle (vertices from gl_VertexID)
    glGenVertexArrays(1, &vao_);

    return true;
}

void GlassRenderer::update(const PixelRect& rect, int screen_w, int screen_h, float time) {
    uniforms_.rect[0] = static_cast<float>(rect.x);
    uniforms_.rect[1] = static_cast<float>(rect.y);
    uniforms_.rect[2] = static_cast<float>(rect.w);
    uniforms_.rect[3] = static_cast<float>(rect.h);
    uniforms_.screen[0] = static_cast<float>(screen_w);
    uniforms_.screen[1] = static_cast<float>(screen_h);
    uniforms_.time = time;
}

void GlassRenderer::render() {
    glUseProgram(program_);

    glUniform4fv(loc_rect_, 1, uniforms_.rect);
    glUniform2fv(loc_screen_, 1, uniforms_.screen);
    glUniform1f(loc_radius_, uniforms_.radius);
    glUniform1f(loc_blur_, uniforms_.blur_radius);
    glUniform4fv(loc_tint_, 1, uniforms_.tint);
    glUniform1f(loc_time_, uniforms_.time);

    glBindVertexArray(vao_);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glUseProgram(0);
}

void GlassRenderer::set_tint(float r, float g, float b, float a) {
    uniforms_.tint[0] = r; uniforms_.tint[1] = g;
    uniforms_.tint[2] = b; uniforms_.tint[3] = a;
}

void GlassRenderer::set_blur_radius(float radius) { uniforms_.blur_radius = radius; }
void GlassRenderer::set_corner_radius(float radius) { uniforms_.radius = radius; }

} // namespace longhorn
