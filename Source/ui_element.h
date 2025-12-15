#ifndef UI_ELEMENT_H_
#define UI_ELEMENT_H_

#include <DirectXMath.h>
#include <memory>
#include <k_lerp.h>
#include "render_layer.h"

class Sprite;
struct ID3D11DeviceContext;

class UiElement {
public:
    UiElement(const char* file_name = "", DirectX::XMFLOAT2 position = { 0, 0 },
        DirectX::XMFLOAT2 size = { 0, 0 }, DirectX::XMFLOAT2 sprite_position = { 0, 0 },
        DirectX::XMFLOAT2 sprite_size = { 0, 0 }, int layer = RenderLayer::kDefault,
        bool is_valid = true);
    virtual ~UiElement() = default;

    virtual void Render(ID3D11DeviceContext* dc);
    void SetSprite(const char* file_name);
    Sprite* GetSprite() const;

    void StartAlphaTransition(float target_alpha, float duration,
        EaseType ease = EaseType::EaseOutQuad);
    void StartAlphaMultiplierTransition(float target_multiplier, float duration,
        EaseType ease = EaseType::EaseOutQuad);
    void SetAlphaImmediate(float alpha);
    void SetAlphaMultiplier(float multiplier);

    virtual void SetPosition(const DirectX::XMFLOAT2& position) { position_ = position; }
    void SetSize(const DirectX::XMFLOAT2& size) { size_ = size; }
    void SetColor(const DirectX::XMFLOAT4& color) { color_ = color; }
    void SetValid(bool is_valid) { is_valid_ = is_valid; }
    void SetRenderLayer(int render_layer) { render_layer_ = render_layer; }
    void SetSpritePosition(const DirectX::XMFLOAT2& sprite_position) {
        sprite_position_ = sprite_position;
	}
    void SetSpriteSize(const DirectX::XMFLOAT2& sprite_size) {
        sprite_size_ = sprite_size;
    }

    const DirectX::XMFLOAT2& GetPosition() const { return position_; }
    const DirectX::XMFLOAT2& GetSize() const { return size_; }
    int GetRenderLayer() const { return render_layer_; }
    bool IsValid() const { return is_valid_; }

protected:
    float CalculateAlpha() const;

    std::unique_ptr<Sprite> sprite_;
    DirectX::XMFLOAT2 position_;
    DirectX::XMFLOAT2 size_;
    DirectX::XMFLOAT2 sprite_position_;
    DirectX::XMFLOAT2 sprite_size_;
    DirectX::XMFLOAT4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };
    int render_layer_;
    bool is_valid_;
    DirectX::XMFLOAT2 size_offset_ = { 0.0f, 0.0f };

    float current_alpha_ = 1.0f;
    float alpha_multiplier_ = 1.0f;
    FloatTween* alpha_tween_ = nullptr;
    FloatTween* multiplier_tween_ = nullptr;
};

#endif