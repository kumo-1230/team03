#pragma once
#include "ui_button.h"
#include "ui_element.h"
#include "System/Sprite.h"
#include <vector>
#include <memory>
#include <DirectXMath.h>

// メニュー背景表示モード
namespace MenuBackgroundMode {
    constexpr int kBackgroundVisible = 0;
    constexpr int kBackgroundHidden = 1;
}

// ボタン入力状態
struct ButtonInputState {
    bool is_button_down = false;
    bool is_button_held = false;
    bool is_button_up = false;
};

// UIパネルクラス
// 複数のボタンとスプライトを管理するコンテナ
class UiPanel : public UiElement {
public:
    UiPanel();
    explicit UiPanel(const char* background_file_name);
    virtual ~UiPanel();

    // ボタン追加（レイヤー順にソート）
    UiButton* AddButton(std::unique_ptr<UiButton> button);

    // スプライト追加（レイヤー順にソート）
    UiElement* AddSprite(const char* file_name,
        DirectX::XMFLOAT2 position,
        DirectX::XMFLOAT2 size,
        DirectX::XMFLOAT2 sprite_position,
        DirectX::XMFLOAT2 sprite_size,
        int layer,
        bool is_valid = true);

    // 更新処理
    void Update(int* clicked_button_id);

    // 描画処理
    void Render(const RenderContext& rc,
        int background_mode = MenuBackgroundMode::kBackgroundVisible,
        bool freeze_effects = false);

    // スプライト色変更
    void ChangeSpritesColor(const DirectX::XMFLOAT4& color);

    // アルファトランジション設定（パネル全体に適用）
    void SetAlphaTransition(bool enable, float target_alpha, float progress);

    // アルファトランジション有効/無効のみ設定
    void SetAlphaTransition(bool enable) {
        enable_alpha_transition_ = enable;
    }

    // 個別設定用
    void SetTargetAlpha(float target_alpha) {
        target_alpha_ = target_alpha;
    }

    void SetAlphaProgress(float progress) {
        alpha_progress_ = progress;
    }

    // ゲッター
    bool IsActive() const { return is_active_; }
    const ButtonInputState& GetInputState() const { return input_state_; }
    UiButton* GetActiveButton() const { return active_button_; }

    // セッター
    void SetActive(bool is_active) { is_active_ = is_active; }
    void SetBackgroundPosition(const DirectX::XMFLOAT2& position) {
        background_position_ = position;
    }
    void SetBackgroundSize(const DirectX::XMFLOAT2& size) {
        background_size_ = size;
    }
    void SetBackgroundColor(const DirectX::XMFLOAT4& color) {
        background_color_ = color;
    }

private:
    // ボタンをレイヤー順に挿入
    void InsertButtonSorted(std::unique_ptr<UiButton> button);

    // 入力状態更新
    void UpdateInputState();

    // クリックされたボタンのIDを取得
    int GetClickedButtonId();

    // カーソルのスクリーン座標取得
    static DirectX::XMFLOAT2 GetCursorScreenPosition();

    std::vector<std::unique_ptr<UiButton>> buttons_;
    std::vector<std::unique_ptr<UiElement>> sprites_;

    std::unique_ptr<UiElement> background_element_;
    DirectX::XMFLOAT2 background_position_ = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 background_size_;
    DirectX::XMFLOAT4 background_color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    bool has_background_;

    bool is_active_ = false;
    ButtonInputState input_state_;

    UiButton* active_button_ = nullptr;
    DirectX::XMFLOAT2 active_button_position_ = { 0.0f, 0.0f };
    DirectX::XMFLOAT2 active_button_size_ = { 0.0f, 0.0f };
};