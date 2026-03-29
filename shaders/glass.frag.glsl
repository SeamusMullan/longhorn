#version 330 core

in vec2 v_uv;
out vec4 frag_color;

uniform vec4 u_rect;     // x, y, w, h in pixels (screen position)
uniform vec2 u_screen;   // screen width, height
uniform float u_radius;  // corner radius
uniform float u_blur;    // blur kernel radius
uniform vec4 u_tint;     // glass tint rgba
uniform float u_time;    // animation time

// --- Noise ---

float hash(vec2 p) {
    vec3 p3 = fract(vec3(p.xyx) * 0.1031);
    p3 += dot(p3, p3.yzx + 33.33);
    return fract((p3.x + p3.y) * p3.z);
}

float value_noise(vec2 p) {
    vec2 i = floor(p);
    vec2 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    float a = hash(i);
    float b = hash(i + vec2(1.0, 0.0));
    float c = hash(i + vec2(0.0, 1.0));
    float d = hash(i + vec2(1.0, 1.0));

    return mix(mix(a, b, f.x), mix(c, d, f.x), f.y);
}

float fbm(vec2 p, int octaves) {
    float value = 0.0;
    float amp = 0.5;
    float freq = 1.0;
    for (int i = 0; i < octaves; i++) {
        value += amp * value_noise(p * freq);
        freq *= 2.0;
        amp *= 0.5;
    }
    return value;
}

// --- SDF for rounded rect in local pixel space ---

float sdf_rounded_rect(vec2 p, vec2 half_size, float r) {
    vec2 d = abs(p) - half_size + r;
    return length(max(d, 0.0)) + min(max(d.x, d.y), 0.0) - r;
}

// --- Caustics ---

float caustic_pattern(vec2 uv, float time) {
    vec2 p1 = uv * 6.0 + vec2(time * 0.08, time * 0.05);
    vec2 p2 = uv * 4.0 - vec2(time * 0.06, time * 0.09);

    float n1 = value_noise(p1);
    float n2 = value_noise(p2);

    vec2 warp = vec2(
        value_noise(p1 + vec2(n2 * 2.0, 0.0)),
        value_noise(p2 + vec2(0.0, n1 * 2.0))
    );

    float caustic = value_noise(uv * 8.0 + warp * 1.5 + time * 0.03);
    return pow(caustic, 2.5);
}

// --- Specular highlights ---

float specular_highlight(vec2 uv, float time) {
    vec2 light1 = vec2(0.25 + sin(time * 0.15) * 0.1, -0.1 + cos(time * 0.12) * 0.05);
    float spec = exp(-dot(uv - light1, uv - light1) * 8.0) * 0.6;

    vec2 light2 = vec2(0.75 + cos(time * 0.1) * 0.08, 0.05);
    spec += exp(-dot(uv - light2, uv - light2) * 12.0) * 0.25;

    return spec;
}

// --- Fresnel ---

float fresnel_edge(vec2 uv) {
    vec2 c = uv - 0.5;
    float edge = length(c * vec2(1.0, 2.0));
    return smoothstep(0.3, 0.9, edge) * 0.3;
}

void main() {
    // uv is [0,1] across the window (viewport = window size)
    vec2 uv = v_uv;

    // Local pixel coordinates centered on the rect
    vec2 local_pixel = (uv - 0.5) * u_rect.zw; // centered on window
    vec2 half_size = u_rect.zw * 0.5;

    // SDF for rounded corners
    float d = sdf_rounded_rect(local_pixel, half_size, u_radius);

    float alpha = 1.0 - smoothstep(-1.5, 0.5, d);
    if (alpha < 0.001) discard;

    // --- Refraction distortion ---
    float thickness = fbm(uv * 3.0 + u_time * 0.02, 3);
    vec2 refract_offset = vec2(
        fbm(uv * 5.0 + vec2(u_time * 0.03, 0.0), 2) - 0.5,
        fbm(uv * 5.0 + vec2(0.0, u_time * 0.04), 2) - 0.5
    ) * 0.015;
    vec2 duv = uv + refract_offset;

    // --- Glass body ---
    vec3 deep = vec3(0.04, 0.05, 0.10);
    vec3 surface = vec3(0.08, 0.10, 0.20);
    float depth = smoothstep(0.0, 1.0, duv.y) * 0.7 + thickness * 0.3;
    vec3 glass = mix(deep, surface, depth);

    // --- Caustics ---
    float caustic = caustic_pattern(duv, u_time);
    glass += vec3(0.15, 0.25, 0.50) * caustic * 0.35;

    // Chromatic aberration on caustics
    float caustic_r = caustic_pattern(duv + vec2(0.003, 0.0), u_time);
    float caustic_b = caustic_pattern(duv - vec2(0.003, 0.0), u_time);
    glass.r += caustic_r * 0.08;
    glass.b += caustic_b * 0.12;

    // --- Specular ---
    float spec = specular_highlight(duv, u_time);
    glass += vec3(0.7, 0.8, 1.0) * spec;

    // --- Fresnel ---
    glass += vec3(0.2, 0.25, 0.4) * fresnel_edge(uv);

    // --- Surface imperfections ---
    float streak = fbm(vec2(uv.x * 40.0 + uv.y * 2.0, uv.y * 3.0), 2);
    streak = smoothstep(0.55, 0.7, streak) * 0.04;
    glass += vec3(streak);

    // --- Internal reflections ---
    float internal = value_noise(uv * 12.0 - u_time * 0.05);
    internal = pow(internal, 4.0) * 0.08;
    glass += vec3(0.3, 0.4, 0.7) * internal;

    // --- Tint ---
    glass = mix(glass, u_tint.rgb, u_tint.a * 0.5);

    // --- Border glow ---
    float inner_border = smoothstep(2.0, 0.0, abs(d)) * 0.5;
    glass = mix(glass, vec3(0.4, 0.55, 0.9), inner_border);

    float outer_glow = smoothstep(4.0, 0.0, d) * smoothstep(-1.0, 0.0, d) * 0.2;
    glass += vec3(0.15, 0.2, 0.35) * outer_glow;

    // --- Top bevel ---
    float top_bevel = smoothstep(0.06, 0.0, uv.y) * alpha;
    glass += vec3(0.25, 0.3, 0.5) * top_bevel * 0.8;

    float top_line = smoothstep(0.015, 0.005, uv.y) * alpha;
    glass += vec3(0.5, 0.6, 0.9) * top_line * 0.4;

    // --- Bottom bevel ---
    float bot_bevel = smoothstep(0.94, 1.0, uv.y) * alpha;
    glass += vec3(0.08, 0.10, 0.18) * bot_bevel;

    frag_color = vec4(glass, alpha * 0.90);
}
