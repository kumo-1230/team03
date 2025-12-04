#pragma once
#include "System/Graphics.h"
#include <memory>
#include <DirectXMath.h>
#include <System/Sprite.h>
#include "render_layer.h"

enum class UiRenderMode {
    kNormal,           // í èÌï\é¶
    kHalfTransparent,  // îºìßñæ
    kInvisible         // îÒï\é¶
};

class UiElement {
public:
    UiElement(const char* file_name,
        DirectX::XMFLOAT2 position,
        DirectX::XMFLOAT2 size,
        DirectX::XMFLOAT2 sprite_position,
        DirectX::XMFLOAT2 sprite_size,
        int layer,
        bool is_valid = true);

    virtual ~UiElement() = default;

    virtual void Render(const RenderContext& rc);

    DirectX::XMFLOAT2 GetPosition() const { return position_; }
    DirectX::XMFLOAT2 GetSize() const { return size_; }
    DirectX::XMFLOAT2 GetSpritePosition() const { return sprite_position_; }
    DirectX::XMFLOAT2 GetSpriteSize() const { return sprite_size_; }
    int GetLayer() const { return layer_; }
    bool IsValid() const { return is_valid_; }
    DirectX::XMFLOAT4 GetColor() const { return color_; }
    UiRenderMode GetRenderMode() const { return render_mode_; }

    virtual void SetPosition(const DirectX::XMFLOAT2& position) {
        position_ = position;
    }
    virtual void SetSize(const DirectX::XMFLOAT2& size) {
        size_ = size;
    }
    virtual void SetSpritePosition(const DirectX::XMFLOAT2& sprite_position) {
        sprite_position_ = sprite_position;
    }
    virtual void SetSpriteSize(const DirectX::XMFLOAT2& sprite_size) {
        sprite_size_ = sprite_size;
    }
    virtual void SetValid(bool is_valid) {
        is_valid_ = is_valid;
    }
    virtual void SetColor(const DirectX::XMFLOAT4& color) {
        color_ = color;
    }
    virtual void SetRenderMode(UiRenderMode mode) {
        render_mode_ = mode;
    }

    void SetAlphaTransition(bool enable) {
        enable_alpha_transition_ = enable;
    }
    void SetTargetAlpha(float target_alpha) {
        target_alpha_ = target_alpha;
    }
    void SetAlphaProgress(float alpha_progress) {
        alpha_progress_ = alpha_progress;
    }

protected:
    virtual float CalculateAlpha() const;

    static float Lerp(float start, float end, float t) {
        return start + (end - start) * t;
    }

    std::unique_ptr<Sprite> sprite_;
    DirectX::XMFLOAT2 position_;
    DirectX::XMFLOAT2 size_;
    DirectX::XMFLOAT2 sprite_position_;
    DirectX::XMFLOAT2 sprite_size_;
    DirectX::XMFLOAT4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    int layer_ = RenderLayer::kDefault;
    bool is_valid_ = true;
    UiRenderMode render_mode_ = UiRenderMode::kNormal;
    float size_offset_ = 0.0f;

    bool enable_alpha_transition_ = false;
    float base_alpha_ = 1.0f;
    float target_alpha_ = 1.0f;
    float alpha_progress_ = 0.0f;
};