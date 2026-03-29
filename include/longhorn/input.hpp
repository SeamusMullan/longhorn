#pragma once

#include <SDL3/SDL.h>
#include <string>

namespace longhorn {

enum class Action {
    None,
    Quit,
    Confirm,
    DeleteChar,
    DeleteWord,
    ClearInput,
    MoveLeft,
    MoveRight,
    MoveHome,
    MoveEnd,
    TextInput,
    Complete, // Tab completion
    MouseClick,
    MouseScroll,
};

struct InputEvent {
    Action action = Action::None;
    std::string text;
    int mouse_x = 0;
    int mouse_y = 0;
    int scroll_delta = 0;
    bool ctrl = false;
};

// Translate an SDL event into a longhorn action
[[nodiscard]] InputEvent translate_event(const SDL_Event& event);

} // namespace longhorn
