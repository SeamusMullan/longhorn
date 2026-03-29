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

        if (key == SDLK_ESCAPE) return {Action::Quit, {}};
        if (key == SDLK_RETURN || key == SDLK_KP_ENTER) return {Action::Confirm, {}};
        if (key == SDLK_TAB) return {Action::Complete, {}};
        if (key == SDLK_BACKSPACE) {
            return ctrl ? InputEvent{Action::DeleteWord, {}} : InputEvent{Action::DeleteChar, {}};
        }
        if (key == SDLK_LEFT) return {Action::MoveLeft, {}};
        if (key == SDLK_RIGHT) return {Action::MoveRight, {}};
        if (key == SDLK_HOME || (ctrl && key == SDLK_A)) return {Action::MoveHome, {}};
        if (key == SDLK_END || (ctrl && key == SDLK_E)) return {Action::MoveEnd, {}};
        if (ctrl && key == SDLK_U) return {Action::ClearInput, {}};
    }

    if (event.type == SDL_EVENT_TEXT_INPUT) {
        return {Action::TextInput, event.text.text};
    }

    return {Action::None, {}};
}

} // namespace longhorn
