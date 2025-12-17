#include "ui_button.h"
#include <System/Sprite.h>

UiButton::UiButton(const char* file_name,
    DirectX::XMFLOAT2 position,
    DirectX::XMFLOAT2 size,
    DirectX::XMFLOAT2 sprite_position,
    DirectX::XMFLOAT2 sprite_size,
    int action_id,
    std::function<void()> on_click_callback,
    int layer,
    bool is_valid)
    : UiElement(file_name, position, size, sprite_position, sprite_size,
        layer, is_valid),
    hit_area_position_(position),
    hit_area_size_(size),
    action_id_(action_id),
    is_hovered_(false),
    visual_config_(ButtonVisualConfig::Default()),
    on_click_callback_(std::move(on_click_callback)) {

    // 初期状態のアルファ値を normal_alpha に設定
    SetAlphaImmediate(visual_config_.normal_alpha);
}

UiButton::UiButton(const char* file_name,
    DirectX::XMFLOAT2 position,
    DirectX::XMFLOAT2 size,
    DirectX::XMFLOAT2 sprite_position,
    DirectX::XMFLOAT2 sprite_size,
    DirectX::XMFLOAT2 hit_area_position,
    DirectX::XMFLOAT2 hit_area_size,
    int action_id,
    std::function<void()> on_click_callback,
    int layer,
    bool is_valid)
    : UiElement(file_name, position, size, sprite_position, sprite_size,
        layer, is_valid),
    hit_area_position_(hit_area_position),
    hit_area_size_(hit_area_size),
    action_id_(action_id),
    is_hovered_(false),
    visual_config_(ButtonVisualConfig::Default()),
    on_click_callback_(std::move(on_click_callback)) {

    // 初期状態のアルファ値を normal_alpha に設定
    SetAlphaImmediate(visual_config_.normal_alpha);
}

void UiButton::UpdateHoverState(const DirectX::XMFLOAT2& cursor_position) {
    if (!is_valid_) {
        if (is_hovered_) { 
            is_hovered_ = false;
            ApplyVisualEffects();
        }
        return;
    }
    const bool was_hovered = is_hovered_;
    is_hovered_ = TestHit(cursor_position);

    if (was_hovered != is_hovered_) {
        ApplyVisualEffects();
    }

    if (is_hovered_ && !was_hovered && on_hover_callback_) {
        on_hover_callback_();
    }
}

void UiButton::ApplyVisualEffects() {
    if (is_hovered_) {
        size_offset_ = visual_config_.hover_size_offset;

        char buf[256];
        sprintf_s(buf, "Hover: current_alpha_=%.2f, alpha_multiplier_=%.2f, final=%.2f\n",
            current_alpha_, alpha_multiplier_, CalculateAlpha());
        OutputDebugStringA(buf);
        StartAlphaTransition(
            visual_config_.hover_alpha,
            0.2f,
            EaseType::EaseOutQuad
        );
    }
    else {
        size_offset_ = visual_config_.normal_size_offset;

        char buf[256];
        sprintf_s(buf, "Normal: current_alpha_=%.2f, alpha_multiplier_=%.2f, final=%.2f\n",
            current_alpha_, alpha_multiplier_, CalculateAlpha());
        OutputDebugStringA(buf);
        StartAlphaTransition(
            visual_config_.normal_alpha,
            0.2f,
            EaseType::EaseInQuad
        );
    }
}

bool UiButton::TestHit(const DirectX::XMFLOAT2& point_position) const {
    constexpr float kCursorSize = 10.0f;

    const float hit_right = hit_area_position_.x + hit_area_size_.x;
    const float hit_bottom = hit_area_position_.y + hit_area_size_.y;
    const float point_right = point_position.x + kCursorSize;
    const float point_bottom = point_position.y + kCursorSize;

    return !(hit_right < point_position.x ||
        hit_area_position_.x > point_right ||
        hit_bottom < point_position.y ||
        hit_area_position_.y > point_bottom);
}

void UiButton::SetPosition(const DirectX::XMFLOAT2& position) {
    const DirectX::XMFLOAT2 offset = {
        position.x - position_.x,
        position.y - position_.y
    };

    position_ = position;
    hit_area_position_.x += offset.x;
    hit_area_position_.y += offset.y;
}

void UiButton::ExecuteOnClick() {
    if (on_click_callback_) {
        on_click_callback_();
    }
}

void UiButton::SetOnClickCallback(std::function<void()> callback) {
    on_click_callback_ = std::move(callback);
}

void UiButton::SetOnHoverCallback(std::function<void()> callback) {
    on_hover_callback_ = std::move(callback);
}

void UiButton::SetVisualConfig(const ButtonVisualConfig& config) {
    visual_config_ = config;
    ApplyVisualEffects();
}