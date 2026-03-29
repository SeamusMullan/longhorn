#include <longhorn/input.hpp>

namespace longhorn {

InputEvent translate_event(const SDL_Event& event) {
    if (event.type == SDL_EVENT_QUIT) {
        return {Action::Quit, {}};
    }

    if (event.type == SDL_EVENT_KEY_DOWN) {
        auto key = event.key.key;
        auto mod = event.key.mod;
        bool ctrl = (mod & SDL_KMOD_CTRL) != 0;

        if (key == SDLK_ESCAPE) return {Action::Quit, {}, 0, 0, 0, ctrl};
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) return {Action::Confirm, {}, 0, 0, 0, ctrl};
        if (key == SDLK_TAB) return {Action::Complete, {}, 0, 0, 0, ctrl};
        if (key == SDLK_BACKSPACE) {
            return {ctrl ? Action::DeleteWord : Action::DeleteChar, {}, 0, 0, 0, ctrl};
        }
        if (key == SDLK_LEFT) return {Action::MoveLeft, {}, 0, 0, 0, ctrl};
        if (key == SDLK_RIGHT) return {Action::MoveRight, {}, 0, 0, 0, ctrl};
        if (key == SDLK_UP) return {Action::MoveUp, {}, 0, 0, 0, ctrl};
        if (key == SDLK_DOWN) return {Action::MoveDown, {}, 0, 0, 0, ctrl};
        if (key == SDLK_HOME || (ctrl && key == SDLK_A)) return {Action::MoveHome, {}, 0, 0, 0, ctrl};
        if (key == SDLK_END || (ctrl && key == SDLK_E)) return {Action::MoveEnd, {}, 0, 0, 0, ctrl};
        if (ctrl && key == SDLK_U) return {Action::ClearInput, {}, 0, 0, 0, ctrl};
    }

    if (event.type == SDL_EVENT_TEXT_INPUT) {
        return {Action::TextInput, event.text.text};
    }

    if (event.type == SDL_EVENT_MOUSE_BUTTON_DOWN) {
        return {Action::MouseClick, {}, static_cast<int>(event.button.x), static_cast<int>(event.button.y), 0, false};
    }

    if (event.type == SDL_EVENT_MOUSE_WHEEL) {
        return {Action::MouseScroll, {}, 0, 0, static_cast<int>(event.wheel.y), false};
    }

    return {Action::None, {}};
}

} // namespace longhorn
