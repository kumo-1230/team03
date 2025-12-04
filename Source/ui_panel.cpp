#include "ui_panel.h"
#include "System/Graphics.h"
#include "common.h"
#include <algorithm>
#include <ranges>
#include <KeyInput.h>
#include <render_layer.h>

UiPanel::UiPanel()
    : UiElement("", DirectX::XMFLOAT2{ 0.0f, 0.0f }, DirectX::XMFLOAT2{ 0.0f, 0.0f },
        DirectX::XMFLOAT2{ 0.0f, 0.0f }, DirectX::XMFLOAT2{ 0.0f, 0.0f }, 0, true),
    has_background_(false),
    background_size_{ SCREEN_W - 20.0f, SCREEN_H - 20.0f } {
}

UiPanel::UiPanel(const char* background_file_name)
    : UiElement("", DirectX::XMFLOAT2{ 0.0f, 0.0f }, DirectX::XMFLOAT2{ 0.0f, 0.0f },
        DirectX::XMFLOAT2{ 0.0f, 0.0f }, DirectX::XMFLOAT2{ 0.0f, 0.0f }, 0, true),
    has_background_(true),
    background_size_{ SCREEN_W - 20.0f, SCREEN_H - 20.0f } {
    background_element_ = std::make_unique<UiElement>(
        background_file_name,
        DirectX::XMFLOAT2{ 0.0f, 0.0f },  // position
        background_size_,                // size
        DirectX::XMFLOAT2{ 0.0f, 0.0f },  // sprite_position
        DirectX::XMFLOAT2{ 1.0f, 1.0f },  // sprite_size (normalized)
        RenderLayer::kBackGround,                              // layer (îwåiÇ»ÇÃÇ≈ç≈â∫ëw)
        true);                           // is_valid
}

UiPanel::~UiPanel() {}

UiButton* UiPanel::AddButton(std::unique_ptr<UiButton> button) {
    if (!button) {
        return nullptr;
    }
    UiButton* button_ptr = button.get();
    InsertButtonSorted(std::move(button));
    return button_ptr;
}

void UiPanel::InsertButtonSorted(std::unique_ptr<UiButton> button) {
    const int layer = button->GetRenderLayer();
    auto insert_position = std::ranges::find_if(buttons_, [layer](const auto& btn) {
        return layer < btn->GetRenderLayer();
        });
    buttons_.insert(insert_position, std::move(button));
}

UiElement* UiPanel::AddSprite(const char* file_name, DirectX::XMFLOAT2 position,
    DirectX::XMFLOAT2 size, DirectX::XMFLOAT2 sprite_position,
    DirectX::XMFLOAT2 sprite_size, int layer, bool is_valid) {
    auto new_sprite = std::make_unique<UiElement>(
        file_name, position, size, sprite_position, sprite_size, layer, is_valid);

    UiElement* sprite_ptr = new_sprite.get();

    auto insert_position = std::ranges::find_if(sprites_, [layer](const auto& spr) {
        return layer < spr->GetRenderLayer();
        });

    sprites_.insert(insert_position, std::move(new_sprite));

    return sprite_ptr;
}

void UiPanel::Update(int* clicked_button_id) {
    if (!clicked_button_id) {
        return;
    }

    if (KeyInput::Instance().GetKeyDown(VK_F1)) {
        is_active_ = !is_active_;
    }

    if (!is_active_) {
        *clicked_button_id = -1;
        return;
    }

    const DirectX::XMFLOAT2 cursor_position = GetCursorScreenPosition();
    constexpr DirectX::XMFLOAT2 kCursorSize{ 10.0f, 10.0f };

    for (auto& button : buttons_) {
        button->Update(cursor_position, kCursorSize);
    }

    UpdateInputState();

    if (KeyInput::Instance().GetKeyDown(VK_LBUTTON)) {
        *clicked_button_id = GetClickedButtonId();
        if (*clicked_button_id != -1 && active_button_) {
            active_button_->ExecuteOnClick();
        }
    }
    else {
        *clicked_button_id = -1;
    }
}

void UiPanel::UpdateInputState() {
    const DirectX::XMFLOAT2 cursor_position = GetCursorScreenPosition();
    constexpr DirectX::XMFLOAT2 kCursorSize{ 10.0f, 10.0f };
    bool any_button_hit = false;

    for (auto& button : std::ranges::reverse_view(buttons_)) {
        if (!button->IsValid()) {
            continue;
        }

        if (button->TestHit(cursor_position, kCursorSize)) {
            any_button_hit = true;

            if (!input_state_.is_button_held && !input_state_.is_button_down) {
                input_state_.is_button_down = true;
                input_state_.is_button_held = false;
                input_state_.is_button_up = false;
                active_button_ = button.get();
                active_button_position_ = button->GetPosition();
                active_button_size_ = button->GetSize();
            }
            else {
                input_state_.is_button_down = false;
                input_state_.is_button_held = true;
                input_state_.is_button_up = false;
            }
            return;
        }
    }

    if (!any_button_hit) {
        if (input_state_.is_button_held) {
            input_state_.is_button_up = true;
            input_state_.is_button_held = false;
            input_state_.is_button_down = false;
            if (active_button_) {
                active_button_position_ = active_button_->GetPosition();
                active_button_size_ = { 0.0f, 0.0f };
                active_button_ = nullptr;
            }
        }
        else {
            input_state_.is_button_up = false;
            input_state_.is_button_held = false;
            input_state_.is_button_down = false;
        }
    }
}

int UiPanel::GetClickedButtonId() {
    const DirectX::XMFLOAT2 cursor_position = GetCursorScreenPosition();
    constexpr DirectX::XMFLOAT2 kCursorSize{ 10.0f, 10.0f };

    for (auto& button : std::ranges::reverse_view(buttons_)) {
        if (button->IsValid() && button->TestHit(cursor_position, kCursorSize)) {
            return button->GetActionId();
        }
    }
    return -1;
}

void UiPanel::Render(const RenderContext& rc, int background_mode,
    bool freeze_effects) {
    if (!is_active_) {
        return;
    }

    const float panel_alpha = CalculateAlpha();

    if (has_background_ && background_element_ &&
        background_mode == MenuBackgroundMode::kBackgroundVisible) {
        DirectX::XMFLOAT4 bg_color = background_color_;
        bg_color.w *= panel_alpha; 

        if (!freeze_effects) {
            background_element_->SetAlphaTransition(enable_alpha_transition_);
            background_element_->SetTargetAlpha(target_alpha_);
            background_element_->SetAlphaProgress(alpha_progress_);
        }
        background_element_->SetColor(bg_color);
        background_element_->Render(rc);
    }

    for (auto& sprite : sprites_) {
        if (!sprite->IsValid()) {
            continue;
        }

        if (!freeze_effects) {
            sprite->SetAlphaTransition(enable_alpha_transition_);
            sprite->SetTargetAlpha(target_alpha_);
            sprite->SetAlphaProgress(alpha_progress_);
        }
        sprite->Render(rc);
    }

    for (auto& button : buttons_) {
        if (!button->IsValid()) {
            continue;
        }

        if (!freeze_effects) {
            button->SetAlphaTransition(enable_alpha_transition_);
            button->SetTargetAlpha(target_alpha_);
            button->SetAlphaProgress(alpha_progress_);
        }
        button->Render(rc);
    }
}

void UiPanel::ChangeSpritesColor(const DirectX::XMFLOAT4& color) {
    for (auto& sprite : sprites_) {
        sprite->SetColor(color);
    }
}

void UiPanel::SetAlphaTransition(bool enable, float target_alpha, float progress) {
    enable_alpha_transition_ = enable;
    target_alpha_ = target_alpha;
    alpha_progress_ = progress;
}

DirectX::XMFLOAT2 UiPanel::GetCursorScreenPosition() {
    POINT cursor;
    GetCursorPos(&cursor);
    ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

    constexpr float kCursorHalfSize = 5.0f;
    return { static_cast<float>(cursor.x) - kCursorHalfSize,
            static_cast<float>(cursor.y) - kCursorHalfSize };
}