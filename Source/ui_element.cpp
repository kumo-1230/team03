#include "ui_element.h"
#include <render_layer.h>

UiElement::UiElement(const char* file_name, DirectX::XMFLOAT2 position,
    DirectX::XMFLOAT2 size, DirectX::XMFLOAT2 sprite_position,
    DirectX::XMFLOAT2 sprite_size, int layer, bool is_valid)
    : position_(position),
    size_(size),
    sprite_position_(sprite_position),
    sprite_size_(sprite_size),
    layer_(layer) {
    if (!file_name || file_name[0] == '\0') {
        is_valid_ = false;
    }
    else {
        is_valid_ = is_valid;
    }

    if (is_valid_) {
        sprite_ = std::make_unique<Sprite>(file_name);
    }
    else {
        int a = 3;
    }
}

void UiElement::Render(const RenderContext& rc) {
    if (render_mode_ == UiRenderMode::kInvisible || !is_valid_) {
        return;
    }

    DirectX::XMFLOAT4 final_color = color_;
    final_color.w = CalculateAlpha();

    sprite_->Render(rc,
        position_.x + size_offset_,
        position_.y + size_offset_,
        0.0f,
        size_.x + size_offset_,
        size_.y + size_offset_,
        sprite_position_.x,
        sprite_position_.y,
        sprite_size_.x,
        sprite_size_.y,
        0.0f,
        final_color.x,
        final_color.y,
        final_color.z,
        final_color.w);
}

float UiElement::CalculateAlpha() const {
    if (!enable_alpha_transition_) {
        switch (render_mode_) {
        case UiRenderMode::kNormal:
            return 1.0f;
        case UiRenderMode::kHalfTransparent:
            return 0.5f;
        case UiRenderMode::kInvisible:
            return 0.0f;
        default:
            return 1.0f;
        }
    }
    return Lerp(base_alpha_, target_alpha_, alpha_progress_);
}