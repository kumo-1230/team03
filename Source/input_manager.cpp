#include "input_manager.h"
#include "System/Graphics.h"

void InputManager::Update() {
    UpdateCursorPosition();
    UpdateKeyStates();
    UpdateMouseButtons();
}

void InputManager::UpdateCursorPosition() {
    if (!window_handle_) {
        window_handle_ = Graphics::Instance().GetWindowHandle();
    }

    POINT cursor;
    GetCursorPos(&cursor);
    ScreenToClient(window_handle_, &cursor);

    // カーソルの中心を基準点とする
    constexpr float kCursorHalfSize = 5.0f;
    cursor_position_.x = static_cast<float>(cursor.x) - kCursorHalfSize;
    cursor_position_.y = static_cast<float>(cursor.y) - kCursorHalfSize;
}

void InputManager::UpdateKeyStates() {
    for (auto& [vkey, state] : key_states_) {
        bool is_pressed = (GetAsyncKeyState(vkey) & 0x8000) != 0;

        if (is_pressed) {
            if (!state.held) {
                state.down = true;
            }
            else {
                state.down = false;
            }
            state.up = false;
            state.held = true;
        }
        else {
            if (state.held) {
                state.up = true;
            }
            else {
                state.up = false;
            }
            state.down = false;
            state.held = false;
        }
    }
}

void InputManager::UpdateMouseButtons() {
    // VK_LBUTTON, VK_RBUTTON, VK_MBUTTON
    const int mouse_vkeys[] = { VK_LBUTTON, VK_RBUTTON, VK_MBUTTON };

    for (int i = 0; i < 3; ++i) {
        bool is_pressed = (GetAsyncKeyState(mouse_vkeys[i]) & 0x8000) != 0;

        if (is_pressed) {
            if (!mouse_buttons_[i].held) {
                mouse_buttons_[i].down = true;
            }
            else {
                mouse_buttons_[i].down = false;
            }
            mouse_buttons_[i].up = false;
            mouse_buttons_[i].held = true;
        }
        else {
            if (mouse_buttons_[i].held) {
                mouse_buttons_[i].up = true;
            }
            else {
                mouse_buttons_[i].up = false;
            }
            mouse_buttons_[i].down = false;
            mouse_buttons_[i].held = false;
        }
    }
}

bool InputManager::IsMouseButtonDown(int button) const {
    if (button < 0 || button >= 3) return false;
    return mouse_buttons_[button].down;
}

bool InputManager::IsMouseButtonUp(int button) const {
    if (button < 0 || button >= 3) return false;
    return mouse_buttons_[button].up;
}

bool InputManager::IsMouseButtonHeld(int button) const {
    if (button < 0 || button >= 3) return false;
    return mouse_buttons_[button].held;
}

bool InputManager::IsKeyDown(int virtual_key) {
    return GetOrCreateKeyState(virtual_key).down;
}

bool InputManager::IsKeyUp(int virtual_key) {
    return GetOrCreateKeyState(virtual_key).up;
}

bool InputManager::IsKeyHeld(int virtual_key) {
    return GetOrCreateKeyState(virtual_key).held;
}

InputManager::KeyState& InputManager::GetOrCreateKeyState(int virtual_key) {
    auto it = key_states_.find(virtual_key);
    if (it != key_states_.end()) {
        return it->second;
    }
    // 新しいキーを登録
    key_states_[virtual_key] = KeyState{};
    return key_states_[virtual_key];
}