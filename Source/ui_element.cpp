#include "ui_element.h"
#include <System/Sprite.h>
#include <System/graphics.h>

UiElement::UiElement(const char* file_name, DirectX::XMFLOAT2 position,
    DirectX::XMFLOAT2 size, DirectX::XMFLOAT2 sprite_position,
    DirectX::XMFLOAT2 sprite_size, int layer, bool is_valid)
    : position_(position),
    size_(size),
    sprite_position_(sprite_position),
    sprite_size_(sprite_size),
    render_layer_(layer),
    is_valid_(is_valid&& file_name&& file_name[0] != '\0') {
    if (is_valid_) {
        sprite_ = std::make_unique<Sprite>(Graphics::Instance().GetDevice(), file_name);
    }
}

void UiElement::Render(ID3D11DeviceContext* dc) {
    if (!is_valid_) {
        return;
    }

    const float final_alpha = CalculateAlpha();
    if (final_alpha <= 0.0f) {
        return;
    }

    sprite_->Render(dc,
        position_.x + size_offset_.x,
        position_.y + size_offset_.y,
        0.0f,
        size_.x + size_offset_.x,
        size_.y + size_offset_.y,
        sprite_position_.x,
        sprite_position_.y,
        sprite_size_.x,
        sprite_size_.y,
        0.0f,
        color_.x,
        color_.y,
        color_.z,
        final_alpha);
}

void UiElement::SetSprite(const char* file_name) {
    if (!file_name || file_name[0] == '\0') {
        is_valid_ = false;
        sprite_ = nullptr;
        return;
    }
    sprite_ = std::make_unique<Sprite>(Graphics::Instance().GetDevice(), file_name);
    is_valid_ = true;
}

Sprite* UiElement::GetSprite() const {
    return sprite_.get();
}

void UiElement::StartAlphaTransition(float target_alpha, float duration, EaseType ease) {
    if (alpha_tween_) {
        alpha_tween_->Stop();
    }
    alpha_tween_ = TweenManager::Instance().AddTween<FloatTween>(
        &current_alpha_,
        current_alpha_,
        target_alpha,
        duration,
        ease
    );
}

void UiElement::StartAlphaMultiplierTransition(float target_multiplier,
    float duration, EaseType ease) {
    if (multiplier_tween_) {
        multiplier_tween_->Stop();
    }
    multiplier_tween_ = TweenManager::Instance().AddTween<FloatTween>(
        &alpha_multiplier_,
        alpha_multiplier_,
        target_multiplier,
        duration,
        ease
    );
}

void UiElement::SetAlphaImmediate(float alpha) {
    current_alpha_ = alpha;
    if (alpha_tween_) {
        alpha_tween_->Stop();
        alpha_tween_ = nullptr;
    }
}

void UiElement::SetAlphaMultiplier(float multiplier) {
    alpha_multiplier_ = multiplier;
    if (multiplier_tween_) {
        multiplier_tween_->Stop();
        multiplier_tween_ = nullptr;
    }
}

float UiElement::CalculateAlpha() const {
    return current_alpha_ * alpha_multiplier_ * color_.w;
}