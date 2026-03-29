#include <longhorn/app.hpp>
#include <longhorn/config_file.hpp>
#include <longhorn/renderer.hpp>
#include <longhorn/input.hpp>
#include <longhorn/filter.hpp>
#include <longhorn/geometry.hpp>
#include <longhorn/history.hpp>

#include <SDL3/SDL.h>
#include <algorithm>
#include <cstdlib>
#include <memory>

namespace longhorn {

void load_config(Config& config, const std::string& path) {
    ConfigFile ini;
    if (!ini.load(path)) return;

    // [general]
    if (ini.has("general", "prompt"))
        config.prompt = ini.get("general", "prompt");
    if (ini.has("general", "font"))
        config.font_path = ini.get("general", "font");
    if (ini.has("general", "font_size"))
        config.font_size = ini.get_int("general", "font_size", config.font_size);
    if (ini.has("general", "layout"))
        config.layout_preset = ini.get("general", "layout");
    if (ini.has("general", "bottom"))
        config.bottom = ini.get_bool("general", "bottom", config.bottom);
    if (ini.has("general", "lines"))
        config.lines = ini.get_int("general", "lines", config.lines);

    // [appearance]
    if (ini.has("appearance", "corner_radius"))
        config.corner_radius = ini.get_float("appearance", "corner_radius", config.corner_radius);
    if (ini.has("appearance", "tint_r"))
        config.tint_r = ini.get_float("appearance", "tint_r", config.tint_r);
    if (ini.has("appearance", "tint_g"))
        config.tint_g = ini.get_float("appearance", "tint_g", config.tint_g);
    if (ini.has("appearance", "tint_b"))
        config.tint_b = ini.get_float("appearance", "tint_b", config.tint_b);
    if (ini.has("appearance", "tint_a"))
        config.tint_a = ini.get_float("appearance", "tint_a", config.tint_a);
}

struct App::Impl {
    Config config;
    std::vector<std::string> items;
    std::string input;
    std::vector<std::size_t> filtered_indices;
    std::vector<std::string> filtered_items;
    int selected = 0;
    int scroll_offset = 0;
    int cursor_pos = 0;
    std::vector<ItemHitbox> item_boxes;

    void update_scroll() {
        int lines = config.lines;
        if (lines <= 0) return;
        if (selected < scroll_offset) scroll_offset = selected;
        if (selected >= scroll_offset + lines) scroll_offset = selected - lines + 1;
        scroll_offset = std::max(0, scroll_offset);
    }

    void update_filter() {
        filtered_indices = filter_items(input, items);
        filtered_items.clear();
        filtered_items.reserve(filtered_indices.size());
        for (auto idx : filtered_indices) {
            filtered_items.push_back(items[idx]);
        }
        selected = std::clamp(selected, 0, std::max(0, static_cast<int>(filtered_items.size()) - 1));
    }
};

App::App(Config config, std::vector<std::string> items)
    : impl_(std::make_unique<Impl>()) {
    impl_->config = std::move(config);
    impl_->items = std::move(items);
    impl_->update_filter();
}

App::~App() = default;

std::string App::run() {
    Layout layout;
    layout.preset = Layout::preset_from_string(impl_->config.layout_preset);
    // Legacy -b flag override
    if (impl_->config.bottom) {
        layout.preset = LayoutPreset::BottomBar;
    }
    layout.corner_radius = impl_->config.corner_radius;

    Renderer renderer(layout, impl_->config.font_path, impl_->config.font_size, impl_->config.lines);
    renderer.set_tint(impl_->config.tint_r, impl_->config.tint_g,
                      impl_->config.tint_b, impl_->config.tint_a);
    SDL_StartTextInput(renderer.window());

    Uint64 last_tick = SDL_GetPerformanceCounter();
    Uint64 freq = SDL_GetPerformanceFrequency();
    float total_time = 0.0f;

    bool running = true;
    while (running) {
        // Delta time
        Uint64 now = SDL_GetPerformanceCounter();
        float dt = static_cast<float>(now - last_tick) / static_cast<float>(freq);
        last_tick = now;
        total_time += dt;

        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            auto input_event = translate_event(event);

            switch (input_event.action) {
                case Action::Quit:
                    return {};

                case Action::Confirm:
                    if (!impl_->filtered_items.empty()) {
                        return impl_->filtered_items[impl_->selected];
                    }
                    return impl_->input;

                case Action::TextInput:
                    impl_->input.insert(impl_->cursor_pos, input_event.text);
                    impl_->cursor_pos += static_cast<int>(input_event.text.size());
                    impl_->selected = 0;
                    impl_->update_filter();
                    break;

                case Action::DeleteChar:
                    if (impl_->cursor_pos > 0) {
                        impl_->input.erase(impl_->cursor_pos - 1, 1);
                        --impl_->cursor_pos;
                        impl_->selected = 0;
                        impl_->update_filter();
                    }
                    break;

                case Action::DeleteWord: {
                    auto& inp = impl_->input;
                    auto& cp = impl_->cursor_pos;
                    while (cp > 0 && inp[cp - 1] == ' ') { inp.erase(cp - 1, 1); --cp; }
                    while (cp > 0 && inp[cp - 1] != ' ') { inp.erase(cp - 1, 1); --cp; }
                    impl_->selected = 0;
                    impl_->update_filter();
                    break;
                }

                case Action::ClearInput:
                    impl_->input.clear();
                    impl_->cursor_pos = 0;
                    impl_->selected = 0;
                    impl_->update_filter();
                    break;

                case Action::MoveLeft:
                    if (impl_->config.lines > 0 || input_event.ctrl || impl_->filtered_items.empty()) {
                        if (impl_->cursor_pos > 0) --impl_->cursor_pos;
                    } else {
                        if (impl_->selected > 0) --impl_->selected;
                        impl_->update_scroll();
                    }
                    break;

                case Action::MoveRight:
                    if (impl_->config.lines > 0 || input_event.ctrl || impl_->filtered_items.empty()) {
                        if (impl_->cursor_pos < static_cast<int>(impl_->input.size())) ++impl_->cursor_pos;
                    } else {
                        if (impl_->selected < static_cast<int>(impl_->filtered_items.size()) - 1)
                            ++impl_->selected;
                        impl_->update_scroll();
                    }
                    break;

                case Action::MoveUp:
                    if (impl_->config.lines > 0) {
                        if (impl_->selected > 0) --impl_->selected;
                        impl_->update_scroll();
                    }
                    break;

                case Action::MoveDown:
                    if (impl_->config.lines > 0) {
                        if (impl_->selected < static_cast<int>(impl_->filtered_items.size()) - 1)
                            ++impl_->selected;
                        impl_->update_scroll();
                    }
                    break;

                case Action::MoveHome:
                    if (input_event.ctrl) {
                        impl_->cursor_pos = 0;
                    } else {
                        impl_->selected = 0;
                        impl_->update_scroll();
                    }
                    break;

                case Action::MoveEnd:
                    if (input_event.ctrl) {
                        impl_->cursor_pos = static_cast<int>(impl_->input.size());
                    } else {
                        impl_->selected = std::max(0, static_cast<int>(impl_->filtered_items.size()) - 1);
                        impl_->update_scroll();
                    }
                    break;

                case Action::Complete:
                    if (!impl_->filtered_items.empty()) {
                        impl_->input = impl_->filtered_items[impl_->selected];
                        impl_->cursor_pos = static_cast<int>(impl_->input.size());
                        impl_->update_filter();
                    }
                    break;

                case Action::MouseClick: {
                    auto& boxes = impl_->item_boxes;
                    int mx = input_event.mouse_x;
                    int my = input_event.mouse_y;
                    for (const auto& box : boxes) {
                        if (mx >= box.rect.x && mx <= box.rect.x + box.rect.w &&
                            my >= box.rect.y && my <= box.rect.y + box.rect.h) {
                            impl_->selected = box.index;
                            break;
                        }
                    }
                    break;
                }

                case Action::MouseScroll:
                    impl_->selected = std::clamp(
                        impl_->selected - input_event.scroll_delta,
                        0,
                        std::max(0, static_cast<int>(impl_->filtered_items.size()) - 1));
                    impl_->update_scroll();
                    break;

                case Action::None:
                    break;
            }
        }

        renderer.update_geometry(dt);

        impl_->item_boxes.clear();
        RenderState state{
            .input = impl_->input,
            .matches = impl_->filtered_items,
            .selected = impl_->selected,
            .prompt = impl_->config.prompt,
            .lines = impl_->config.lines,
            .scroll_offset = impl_->scroll_offset,
            .cursor_pos = impl_->cursor_pos,
            .time = total_time,
            .item_boxes = &impl_->item_boxes,
        };
        renderer.draw(state, total_time);

        SDL_Delay(8);
    }

    return {};
}

} // namespace longhorn
