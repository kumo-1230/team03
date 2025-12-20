#include "ui_panel.h"
#include "ui_event.h"
#include "System/graphics.h"
#include "common.h"
#include <algorithm>
#include <ranges>
#include "input_manager.h"
#include "render_layer.h"

UiPanel::UiPanel()
{
}

UiPanel::UiPanel(const char* background_file_name) 
{
    background_element_ = std::make_unique<UiElement>(
        background_file_name,
        DirectX::XMFLOAT2{ 0.0f, 0.0f },
        background_size_,
        DirectX::XMFLOAT2{ 0.0f, 0.0f },
        DirectX::XMFLOAT2{ 1.0f, 1.0f },
        RenderLayer::kBackGround,
        true);
	has_background_ = true;
}

UiPanel::~UiPanel() {}

void UiPanel::InsertButtonSorted(std::unique_ptr<UiButton> button) {
    const int layer = button->GetRenderLayer();
    auto insert_position = std::ranges::find_if(buttons_, [layer](const auto& btn) {
        return layer < btn->GetRenderLayer();
        });
    buttons_.insert(insert_position, std::move(button));
}

void UiPanel::Update() {
    if (!is_active_) {
        return;
    }

    HandleInput();
}

void UiPanel::HandleInput() {
    const DirectX::XMFLOAT2 cursor_position = InputManager::Instance().GetCursorPosition();

    // すべてのボタンのホバー状態を更新
    UpdateButtonStates();

    // マウスボタンが押された瞬間
    if (InputManager::Instance().IsMouseButtonDown(0)) { // Left button
        if (hovered_button_) {
            pressed_button_ = hovered_button_;
            EnqueueEvent(UiEventType::PressBegin, pressed_button_);
        }
    }

    // マウスボタンが離された瞬間
    if (InputManager::Instance().IsMouseButtonUp(0)) {
        if (pressed_button_) {
            EnqueueEvent(UiEventType::PressEnd, pressed_button_);

            // 押したボタンと同じボタン上で離された場合はクリック
            if (pressed_button_ == hovered_button_) {
                EnqueueEvent(UiEventType::Click, pressed_button_);
                pressed_button_->ExecuteOnClick();
            }

            pressed_button_ = nullptr;
        }
    }
}

void UiPanel::UpdateButtonStates() {
    const DirectX::XMFLOAT2 cursor_position = InputManager::Instance().GetCursorPosition();
    UiButton* new_hovered_button = nullptr;

    // 逆順で判定（上のレイヤーが優先）
    for (auto& button : std::ranges::reverse_view(buttons_)) {
        if (!button->IsValid()) {
            continue;
        }

        button->UpdateHoverState(cursor_position);

        if (button->IsHovered() && !new_hovered_button) {
            new_hovered_button = button.get();
        }
    }

    // ホバー状態の変化を検出してイベント生成
    if (new_hovered_button != hovered_button_) {
        if (hovered_button_) {
            EnqueueEvent(UiEventType::HoverEnd, hovered_button_);
        }
        if (new_hovered_button) {
            EnqueueEvent(UiEventType::HoverBegin, new_hovered_button);
        }
        hovered_button_ = new_hovered_button;
    }
}

void UiPanel::EnqueueEvent(UiEventType type, UiButton* button) {
    if (button) {
        pending_events_.emplace_back(type, button, button->GetActionId());
    }
}

std::vector<UiEvent> UiPanel::GetPendingEvents() {
    std::vector<UiEvent> events = std::move(pending_events_);
    pending_events_.clear();
    return events;
}

void UiPanel::Render(ID3D11DeviceContext* dc, int background_mode,
    bool freeze_effects) {
    if (!is_active_) {
        return;
    }

    // 背景のレンダリング
    if (has_background_ && background_element_ &&
        background_mode == MenuBackgroundMode::kBackgroundVisible) {
        background_element_->SetColor(background_color_);
        background_element_->Render(dc);
    }

    // スプライトのレンダリング
    for (auto& sprite : sprites_) {
        if (!sprite->IsValid()) {
            continue;
        }
        sprite->Render(dc);
    }

    // ボタンのレンダリング
    for (auto& button : buttons_) {
        if (!button->IsValid()) {
            continue;
        }
        button->Render(dc);
    }
}

void UiPanel::ChangeColor(const DirectX::XMFLOAT4& color) {
    if (background_element_) {
        background_element_->SetColor(color);
    }

    for (auto& sprite : sprites_) {
        sprite->SetColor(color);
    }

    for (auto& button : buttons_) {
        button->SetColor(color);
    }
}

void UiPanel::FadeIn(float duration, EaseType ease) {
    // 係数を使ってパネル全体をフェード
    if (background_element_) {
        background_element_->StartAlphaMultiplierTransition(1.0f, duration, ease);
    }

    for (auto& sprite : sprites_) {
        sprite->StartAlphaMultiplierTransition(1.0f, duration, ease);
    }

    for (auto& button : buttons_) {
        button->StartAlphaMultiplierTransition(1.0f, duration, ease);
    }
}

void UiPanel::FadeOut(float duration, EaseType ease) {
    // 係数を使ってパネル全体をフェード
    if (background_element_) {
        background_element_->StartAlphaMultiplierTransition(0.0f, duration, ease);
    }

    for (auto& sprite : sprites_) {
        sprite->StartAlphaMultiplierTransition(0.0f, duration, ease);
    }

    for (auto& button : buttons_) {
        button->StartAlphaMultiplierTransition(0.0f, duration, ease);
    }
}

void UiPanel::SetBackgroundSize(const DirectX::XMFLOAT2& size) {
    background_size_ = size;
    if (background_element_) {
        background_element_->SetSize(size);
    }
}

void UiPanel::SetAlphaMultiplier(float alpha) {
    if (background_element_) {
        background_element_->SetAlphaMultiplier(alpha);
	}

    for (auto& sprite : sprites_) {
        sprite->SetAlphaMultiplier(alpha);
    }

    for (auto& button : buttons_) {
        button->SetAlphaMultiplier(alpha);
    }
}