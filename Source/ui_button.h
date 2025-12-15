#ifndef UI_BUTTON_H_
#define UI_BUTTON_H_

#include "ui_element.h"
#include <DirectXMath.h>
#include <functional>
#include "render_layer.h"

struct ButtonVisualConfig {
    DirectX::XMFLOAT2 normal_size_offset;
    DirectX::XMFLOAT2 hover_size_offset;
    float normal_alpha;
    float hover_alpha;

    static ButtonVisualConfig Default() {
        return {
          {0.0f, 0.0f},
          {5.0f, 5.0f},
          0.7f,
          1.0f
        };
    }
};

class UiButton : public UiElement {
public:
    UiButton(const char* file_name = "",
        DirectX::XMFLOAT2 position = { 0, 0 },
        DirectX::XMFLOAT2 size = { 0, 0 },
        DirectX::XMFLOAT2 sprite_position = { 0, 0 },
        DirectX::XMFLOAT2 sprite_size = {0, 0},
        int action_id = 0,
        std::function<void()> on_click_callback = nullptr,
        int layer = RenderLayer::kButton,
        bool is_valid = true);

    UiButton(const char* file_name = "",
        DirectX::XMFLOAT2 position = { 0, 0 },
        DirectX::XMFLOAT2 size = { 0, 0 },
        DirectX::XMFLOAT2 sprite_position = { 0, 0 },
        DirectX::XMFLOAT2 sprite_size = { 0, 0 },
        DirectX::XMFLOAT2 hit_area_position = { 0, 0 },
        DirectX::XMFLOAT2 hit_area_size = { 0, 0 },
        int action_id = 0,
        std::function<void()> on_click_callback = nullptr,
        int layer = RenderLayer::kButton,
        bool is_valid = true);

    void UpdateHoverState(const DirectX::XMFLOAT2& cursor_position);
    void ExecuteOnClick();
    void SetPosition(const DirectX::XMFLOAT2& position) override;
    void SetOnClickCallback(std::function<void()> callback);
    void SetOnHoverCallback(std::function<void()> callback);
    void SetVisualConfig(const ButtonVisualConfig& config);

    int GetActionId() const { return action_id_; }
    bool IsHovered() const { return is_hovered_; }

private:
    bool TestHit(const DirectX::XMFLOAT2& point_position) const;
    void ApplyVisualEffects();

    DirectX::XMFLOAT2 hit_area_position_;
    DirectX::XMFLOAT2 hit_area_size_;
    int action_id_;
    bool is_hovered_;
    ButtonVisualConfig visual_config_;
    std::function<void()> on_click_callback_;
    std::function<void()> on_hover_callback_;
};

#endif