#pragma once
#include "ui_element.h"
#include <functional>
#include <DirectXMath.h>

// UIボタンクラス
// ユーザーインタラクションを処理するボタン要素
class UiButton : public UiElement {
public:
    // コンストラクタ（ヒットエリアは描画エリアと同じ）
    UiButton(const char* file_name,
        DirectX::XMFLOAT2 position,
        DirectX::XMFLOAT2 size,
        DirectX::XMFLOAT2 sprite_position,
        DirectX::XMFLOAT2 sprite_size,
        int layer,
        int action_id,
        std::function<void()> on_click_callback = nullptr,
        bool is_valid = true);

    // コンストラクタ（ヒットエリアを個別に指定）
    UiButton(const char* file_name,
        DirectX::XMFLOAT2 position,
        DirectX::XMFLOAT2 size,
        DirectX::XMFLOAT2 sprite_position,
        DirectX::XMFLOAT2 sprite_size,
        DirectX::XMFLOAT2 hit_area_position,
        DirectX::XMFLOAT2 hit_area_size,
        int layer,
        int action_id,
        std::function<void()> on_click_callback = nullptr,
        bool is_valid = true);

    virtual ~UiButton() = default;

    // 更新処理（カーソル位置との判定）
    void Update(const DirectX::XMFLOAT2& cursor_position,
        const DirectX::XMFLOAT2& cursor_size);

    // 衝突判定
    bool TestHit(const DirectX::XMFLOAT2& point_position,
        const DirectX::XMFLOAT2& point_size) const;

    // 位置設定（ヒットエリアも連動して移動）
    void SetPosition(const DirectX::XMFLOAT2& position) override;

    // クリックコールバックを実行
    void ExecuteOnClick();

    // コールバック設定
    void SetOnClickCallback(std::function<void()> callback);
    void SetOnHoverCallback(std::function<void()> callback);

    // ゲッター
    int GetActionId() const { return action_id_; }
    bool IsHovered() const { return is_hovered_; }
    DirectX::XMFLOAT2 GetHitAreaPosition() const { return hit_area_position_; }
    DirectX::XMFLOAT2 GetHitAreaSize() const { return hit_area_size_; }

    // セッター
    void SetHitAreaPosition(const DirectX::XMFLOAT2& position) {
        hit_area_position_ = position;
    }
    void SetHitAreaSize(const DirectX::XMFLOAT2& size) {
        hit_area_size_ = size;
    }

private:
    DirectX::XMFLOAT2 hit_area_position_;  // ヒットエリアの位置
    DirectX::XMFLOAT2 hit_area_size_;      // ヒットエリアのサイズ
    int action_id_;                         // アクションID
    bool is_hovered_;                       // ホバー状態

    std::function<void()> on_click_callback_;   // クリック時のコールバック
    std::function<void()> on_hover_callback_;   // ホバー時のコールバック
};