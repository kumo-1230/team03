#ifndef UI_PANEL_H_
#define UI_PANEL_H_

#include "ui_button.h"
#include "ui_element.h"
#include "ui_event.h"
#include <vector>
#include <memory>
#include "k_lerp.h"

class UiPanel {
public:
    UiPanel();
    explicit UiPanel(const char* background_file_name);
    virtual ~UiPanel();

    template<typename... Args>
    UiButton* AddButton(Args&&... args) {
        auto new_button = std::make_unique<UiButton>(std::forward<Args>(args)...);

        UiButton* button_ptr = new_button.get();

        const int layer = new_button->GetRenderLayer();
        auto insert_position = std::ranges::find_if(buttons_, [layer](const auto& btn) {
            return layer < btn->GetRenderLayer();
            });

        buttons_.insert(insert_position, std::move(new_button));

        return button_ptr;
    }

    /*
     (const char* file_name = "", DirectX::XMFLOAT2 position = { 0, 0 },
       DirectX::XMFLOAT2 size = { 0, 0 }, DirectX::XMFLOAT2 sprite_position = { 0, 0 },
       DirectX::XMFLOAT2 sprite_size = { 0, 0 }, int layer = RenderLayer::kDefault,
       bool is_valid = true)
    */
    template<typename... Args>
    UiElement* AddSprite(Args&&... args) {
        auto new_sprite = std::make_unique<UiElement>(std::forward<Args>(args)...);

        UiElement* sprite_ptr = new_sprite.get();

        const int layer = new_sprite->GetRenderLayer();
        auto insert_position = std::ranges::find_if(sprites_, [layer](const auto& spr) {
            return layer < spr->GetRenderLayer();
            });

        sprites_.insert(insert_position, std::move(new_sprite));

        return sprite_ptr;
    }

    // 更新
    void Update();

    // レンダリング
    void Render(ID3D11DeviceContext* dc, int background_mode = MenuBackgroundMode::kBackgroundVisible,
        bool freeze_effects = false);

    // パネル全体のフェードイン/アウト
    void FadeIn(float duration = 0.5f, EaseType ease = EaseType::EaseOutCubic);
    void FadeOut(float duration = 0.5f, EaseType ease = EaseType::EaseInCubic);

    // イベント取得
    std::vector<UiEvent> GetPendingEvents();

    // スプライトの色変更
    void ChangeSpritesColor(const DirectX::XMFLOAT4& color);

    // 背景サイズ設定
    void SetBackgroundSize(const DirectX::XMFLOAT2& size);

    // アクティブ状態
    void SetActive(bool active) { is_active_ = active; }
    bool IsActive() const { return is_active_; }

    // 背景色設定
    void SetBackgroundColor(const DirectX::XMFLOAT4& color) { background_color_ = color; }

protected:
    void HandleInput();
    void UpdateButtonStates();
    void EnqueueEvent(UiEventType type, UiButton* button);
    void InsertButtonSorted(std::unique_ptr<UiButton> button);

    std::vector<std::unique_ptr<UiButton>> buttons_;
    std::vector<std::unique_ptr<UiElement>> sprites_;
    std::unique_ptr<UiElement> background_element_;
    std::vector<UiEvent> pending_events_;

    UiButton* hovered_button_ = nullptr;
    UiButton* pressed_button_ = nullptr;

    bool has_background_;
    DirectX::XMFLOAT2 background_size_;
    DirectX::XMFLOAT4 background_color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    bool is_active_ = true;
};

#endif // UI_PANEL_H_