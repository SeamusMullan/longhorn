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

    // [history]
    if (ini.has("history", "enabled"))
        config.use_history = ini.get_bool("history", "enabled", config.use_history);
    if (ini.has("history", "path"))
        config.history_path = ini.get("history", "path");

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

static std::string default_history_path() {
    std::string base;
    if (const char* cache = std::getenv("XDG_CACHE_HOME")) {
        base = cache;
    } else if (const char* home = std::getenv("HOME")) {
        base = std::string(home) + "/.cache";
    } else {
        return {};
    }
    return base + "/longhorn/history";
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
    std::vector<std::pair<int,int>> item_positions;
    std::unique_ptr<History> history;

    void update_scroll() {
        int lines = config.lines;
        if (lines <= 0) return;
        if (selected < scroll_offset) scroll_offset = selected;
        if (selected >= scroll_offset + lines) scroll_offset = selected - lines + 1;
        scroll_offset = std::max(0, scroll_offset);
    }

    void update_filter() {
        if (history) {
            filtered_indices = filter_items(input, items,
                [this](const std::string& item) { return history->score(item); });
        } else {
            filtered_indices = filter_items(input, items);
        }
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

    if (impl_->config.use_history) {
        std::string hpath = impl_->config.history_path.empty()
                                ? default_history_path()
                                : impl_->config.history_path;
        if (!hpath.empty()) {
            impl_->history = std::make_unique<History>(hpath);
            impl_->history->load();
        }
    }

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

                case Action::Confirm: {
                    std::string result;
                    if (!impl_->filtered_items.empty()) {
                        result = impl_->filtered_items[impl_->selected];
                    } else {
                        result = impl_->input;
                    }
                    if (impl_->history && !result.empty()) {
                        impl_->history->record(result);
                        impl_->history->save();
                    }
                    return result;
                }

                case Action::TextInput:
                    impl_->input += input_event.text;
                    impl_->selected = 0;
                    impl_->update_filter();
                    break;

                case Action::DeleteChar:
                    if (!impl_->input.empty()) {
                        impl_->input.pop_back();
                        impl_->selected = 0;
                        impl_->update_filter();
                    }
                    break;

                case Action::DeleteWord: {
                    auto& inp = impl_->input;
                    while (!inp.empty() && inp.back() == ' ') inp.pop_back();
                    while (!inp.empty() && inp.back() != ' ') inp.pop_back();
                    impl_->selected = 0;
                    impl_->update_filter();
                    break;
                }

                case Action::ClearInput:
                    impl_->input.clear();
                    impl_->selected = 0;
                    impl_->update_filter();
                    break;

                case Action::MoveLeft:
                    if (impl_->selected > 0) --impl_->selected;
                    impl_->update_scroll();
                    break;

                case Action::MoveRight:
                    if (impl_->selected < static_cast<int>(impl_->filtered_items.size()) - 1)
                        ++impl_->selected;
                    impl_->update_scroll();
                    break;

                case Action::MoveHome:
                    impl_->selected = 0;
                    impl_->update_scroll();
                    break;

                case Action::MoveEnd:
                    impl_->selected = std::max(0, static_cast<int>(impl_->filtered_items.size()) - 1);
                    impl_->update_scroll();
                    break;

                case Action::Complete:
                    if (!impl_->filtered_items.empty()) {
                        impl_->input = impl_->filtered_items[impl_->selected];
                        impl_->update_filter();
                    }
                    break;

                case Action::None:
                    break;
            }
        }

        renderer.update_geometry(dt);

        RenderState state{
            .input = impl_->input,
            .matches = impl_->filtered_items,
            .selected = impl_->selected,
            .prompt = impl_->config.prompt,
            .lines = impl_->config.lines,
            .scroll_offset = impl_->scroll_offset,
        };
        renderer.draw(state, total_time);

        SDL_Delay(8);
    }

    return {};
}

} // namespace longhorn
