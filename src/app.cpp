#include <longhorn/app.hpp>
#include <longhorn/renderer.hpp>
#include <longhorn/input.hpp>
#include <longhorn/filter.hpp>
#include <longhorn/geometry.hpp>

#include <SDL3/SDL.h>
#include <algorithm>

namespace longhorn {

struct App::Impl {
    Config config;
    std::vector<std::string> items;
    std::string input;
    std::vector<std::size_t> filtered_indices;
    std::vector<std::string> filtered_items;
    int selected = 0;

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
    if (impl_->config.bottom) {
        layout.preset = LayoutPreset::BottomBar;
    } else {
        layout.preset = LayoutPreset::TopBar;
    }

    Renderer renderer(layout, impl_->config.font_path, impl_->config.font_size);
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
                    break;

                case Action::MoveRight:
                    if (impl_->selected < static_cast<int>(impl_->filtered_items.size()) - 1)
                        ++impl_->selected;
                    break;

                case Action::MoveHome:
                    impl_->selected = 0;
                    break;

                case Action::MoveEnd:
                    impl_->selected = std::max(0, static_cast<int>(impl_->filtered_items.size()) - 1);
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
        };
        renderer.draw(state, total_time);

        SDL_Delay(8);
    }

    return {};
}

} // namespace longhorn
