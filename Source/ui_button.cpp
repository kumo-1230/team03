#include "ui_button.h"

UiButton::UiButton(const char* file_name, DirectX::XMFLOAT2 position,
    DirectX::XMFLOAT2 size, DirectX::XMFLOAT2 sprite_position,
    DirectX::XMFLOAT2 sprite_size, int layer, int action_id,
    std::function<void()> on_click_callback,
    bool is_valid)
    : UiElement(file_name, position, size, sprite_position, sprite_size,
        layer, is_valid),
    hit_area_position_(position),
    hit_area_size_(size),
    action_id_(action_id),
    is_hovered_(false),
    on_click_callback_(std::move(on_click_callback)) {
}

UiButton::UiButton(const char* file_name, DirectX::XMFLOAT2 position,
    DirectX::XMFLOAT2 size, DirectX::XMFLOAT2 sprite_position,
    DirectX::XMFLOAT2 sprite_size,
    DirectX::XMFLOAT2 hit_area_position,
    DirectX::XMFLOAT2 hit_area_size, int layer, int action_id,
    std::function<void()> on_click_callback,
    bool is_valid)
    : UiElement(file_name, position, size, sprite_position, sprite_size,
        layer, is_valid),
    hit_area_position_(hit_area_position),
    hit_area_size_(hit_area_size),
    action_id_(action_id),
    is_hovered_(false),
    on_click_callback_(std::move(on_click_callback)) {
}

void UiButton::Update(const DirectX::XMFLOAT2& cursor_position,
    const DirectX::XMFLOAT2& cursor_size) {
    if (!is_valid_) {
        is_hovered_ = false;
        return;
    }

    const bool was_hovered = is_hovered_;
    is_hovered_ = TestHit(cursor_position, cursor_size);

    if (is_hovered_) {
        SetRenderMode(UiRenderMode::kNormal);
        size_offset_ = -1.0f;
    }
    else {
        SetRenderMode(UiRenderMode::kHalfTransparent);
        size_offset_ = 0.0f;
    }

    // ホバー状態が変化したときのコールバック
    if (is_hovered_ && !was_hovered && on_hover_callback_) {
        on_hover_callback_();
    }
}

bool UiButton::TestHit(const DirectX::XMFLOAT2& point_position,
    const DirectX::XMFLOAT2& point_size) const {
    // AABB (Axis-Aligned Bounding Box) 衝突判定
    const float hit_right = hit_area_position_.x + hit_area_size_.x;
    const float hit_bottom = hit_area_position_.y + hit_area_size_.y;
    const float point_right = point_position.x + point_size.x;
    const float point_bottom = point_position.y + point_size.y;

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