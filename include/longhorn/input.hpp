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
};

struct InputEvent {
    Action action = Action::None;
    std::string text;
};

// Translate an SDL event into a longhorn action
[[nodiscard]] InputEvent translate_event(const SDL_Event& event);

} // namespace longhorn
