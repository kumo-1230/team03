#ifndef UI_BUTTON_H_
#define UI_BUTTON_H_

#include "ui_element.h"
#include <DirectXMath.h>
#include <functional>
#include "render_layer.h"

/**
 * @brief ボタンの見た目に関する設定構造体
 *
 * ホバー状態／通常状態でのサイズ変化およびアルファ値を定義する。
 */
struct ButtonVisualConfig {
    /** 通常時のサイズオフセット */
    DirectX::XMFLOAT2 normal_size_offset;

    /** ホバー時のサイズオフセット */
    DirectX::XMFLOAT2 hover_size_offset;

    /** 通常時のアルファ値 */
    float normal_alpha;

    /** ホバー時のアルファ値 */
    float hover_alpha;

    /**
     * @brief デフォルトのボタン表示設定を取得する
     *
     * @return デフォルト設定
     */
    static ButtonVisualConfig Default() {
        return {
            {0.0f, 0.0f},
            {5.0f, 5.0f},
            0.7f,
            1.0f
        };
    }
};

/**
 * @brief クリック可能なUIボタンクラス
 *
 * マウスカーソルによるホバー判定、クリック判定を行い、
 * 視覚効果（サイズ・アルファ変化）およびコールバック実行を管理する。
 */
class UiButton : public UiElement {
public:
    /**
     * @brief UiButton コンストラクタ（ヒット領域がサイズと同一の場合）
     *
     * @param file_name スプライトのファイル名
     * @param position 表示位置
     * @param size 表示サイズ
     * @param sprite_position スプライト内切り取り位置
     * @param sprite_size スプライト内切り取りサイズ
     * @param action_id ボタンに紐づくアクションID
     * @param on_click_callback クリック時に実行されるコールバック
     * @param layer 描画レイヤー
     * @param is_valid 有効フラグ
     */
    UiButton(const char* file_name = "",
        DirectX::XMFLOAT2 position = { 0, 0 },
        DirectX::XMFLOAT2 size = { 0, 0 },
        DirectX::XMFLOAT2 sprite_position = { 0, 0 },
        DirectX::XMFLOAT2 sprite_size = { 0, 0 },
        int action_id = 0,
        std::function<void()> on_click_callback = nullptr,
        int layer = RenderLayer::kButton,
        bool is_valid = true);

    /**
     * @brief UiButton コンストラクタ（ヒット領域を個別指定）
     *
     * 見た目と当たり判定を分離したい場合に使用する。
     *
     * @param file_name スプライトのファイル名
     * @param position 表示位置
     * @param size 表示サイズ
     * @param sprite_position スプライト内切り取り位置
     * @param sprite_size スプライト内切り取りサイズ
     * @param hit_area_position ヒット領域の位置
     * @param hit_area_size ヒット領域のサイズ
     * @param action_id ボタンに紐づくアクションID
     * @param on_click_callback クリック時に実行されるコールバック
     * @param layer 描画レイヤー
     * @param is_valid 有効フラグ
     */
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

    /**
     * @brief カーソル位置からホバー状態を更新する
     *
     * @param cursor_position 現在のカーソル位置
     */
    void UpdateHoverState(const DirectX::XMFLOAT2& cursor_position);

    /**
     * @brief クリック時の処理を実行する
     *
     * 登録されたクリックコールバックが呼び出される。
     */
    void ExecuteOnClick();

    /**
     * @brief ボタン位置を設定する
     *
     * ヒット領域の位置も同時に更新される。
     *
     * @param position 新しい位置
     */
    void SetPosition(const DirectX::XMFLOAT2& position) override;

    /**
     * @brief クリック時のコールバックを設定する
     *
     * @param callback 実行する関数
     */
    void SetOnClickCallback(std::function<void()> callback);

    /**
     * @brief ホバー開始時のコールバックを設定する
     *
     * @param callback 実行する関数
     */
    void SetOnHoverCallback(std::function<void()> callback);

    /**
     * @brief ボタンの視覚効果設定を変更する
     *
     * @param config 視覚設定
     */
    void SetVisualConfig(const ButtonVisualConfig& config);

    /**
     * @brief アクションIDを取得する
     *
     * @return アクションID
     */
    int GetActionId() const { return action_id_; }

    /**
     * @brief 現在ホバー状態かどうかを取得する
     *
     * @return true の場合ホバー中
     */
    bool IsHovered() const { return is_hovered_; }

private:
    /**
     * @brief 指定された位置がヒット領域内か判定する
     *
     * @param point_position 判定する位置
     * @return true の場合ヒット
     */
    bool TestHit(const DirectX::XMFLOAT2& point_position) const;

    /**
     * @brief 現在の状態に応じた視覚効果を適用する
     *
     * サイズオフセットおよびアルファ値の変更を行う。
     */
    void ApplyVisualEffects();

    /** ヒット領域の位置 */
    DirectX::XMFLOAT2 hit_area_position_;

    /** ヒット領域のサイズ */
    DirectX::XMFLOAT2 hit_area_size_;

    /** ボタン識別用アクションID */
    int action_id_;

    /** ホバー状態フラグ */
    bool is_hovered_;

    /** 視覚効果設定 */
    ButtonVisualConfig visual_config_;

    /** クリック時コールバック */
    std::function<void()> on_click_callback_;

    /** ホバー開始時コールバック */
    std::function<void()> on_hover_callback_;
};

#endif
