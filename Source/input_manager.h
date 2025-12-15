#pragma once
#include <DirectXMath.h>
#include <Windows.h>
#include <map>

// 入力処理を一元管理するクラス
class InputManager {
public:
    static InputManager& Instance() {
        static InputManager instance;
        return instance;
    }

    void Update();

    // マウス位置取得（スクリーン座標）
    DirectX::XMFLOAT2 GetCursorPosition() const { return cursor_position_; }

    // マウスボタンの状態
    bool IsMouseButtonDown(int button) const;      // 押された瞬間
    bool IsMouseButtonUp(int button) const;        // 離された瞬間
    bool IsMouseButtonHeld(int button) const;      // 押され続けている

    // キーボードの状態
    bool IsKeyDown(int vkey) const;    // 押された瞬間
    bool IsKeyUp(int vkey) const;      // 離された瞬間
    bool IsKeyHeld(int vkey) const;    // 押され続けている

    void SetWindowHandle(HWND hwnd) { window_handle_ = hwnd; }

private:
    InputManager() = default;
    ~InputManager() = default;
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;

    struct KeyState {
        bool down = false;   // 押された瞬間
        bool up = false;     // 離された瞬間
        bool held = false;   // 押され続けている
    };

    DirectX::XMFLOAT2 cursor_position_{ 0.0f, 0.0f };

    // キー状態管理（KeyInputと同じ方式）
    std::map<int, KeyState> key_states_;

    // マウスボタン（0:Left, 1:Right, 2:Middle）
    KeyState mouse_buttons_[3];

    HWND window_handle_ = nullptr;

    void UpdateCursorPosition();
    void UpdateKeyStates();
    void UpdateMouseButtons();

    // キー状態の取得または作成
    KeyState& GetOrCreateKeyState(int vkey);
};