#ifndef UI_ELEMENT_H_
#define UI_ELEMENT_H_

#include <DirectXMath.h>
#include <memory>
#include <k_lerp.h>
#include "render_layer.h"

class Sprite;
struct ID3D11DeviceContext;

/**
 * @brief UI上に描画される基本要素クラス
 *
 * スプライト描画、位置・サイズ管理、描画レイヤー、
 * アルファ値およびアルファ乗算のトランジション機能を提供する。
 * 他のUI要素はこのクラスを継承して実装されることを想定している。
 */
class UiElement {
public:
    /**
     * @brief UiElement のコンストラクタ
     *
     * @param file_name 使用するスプライトのファイル名
     * @param position UI上での表示位置
     * @param size UI要素の表示サイズ
     * @param sprite_position スプライト内での切り取り開始位置
     * @param sprite_size スプライト内での切り取りサイズ
     * @param layer 描画レイヤー
     * @param is_valid 要素が有効かどうか
     */
    UiElement(const char* file_name = "",
        DirectX::XMFLOAT2 position = { 0, 0 },
        DirectX::XMFLOAT2 size = { 0, 0 },
        DirectX::XMFLOAT2 sprite_position = { 0, 0 },
        DirectX::XMFLOAT2 sprite_size = { 0, 0 },
        int layer = RenderLayer::kDefault,
        bool is_valid = true);

    /**
     * @brief 仮想デストラクタ
     */
    virtual ~UiElement() = default;

    /**
     * @brief UI要素を描画する
     *
     * @param dc DirectX デバイスコンテキスト
     */
    virtual void Render(ID3D11DeviceContext* dc);

    /**
     * @brief スプライトを設定する
     *
     * @param file_name スプライトのファイル名
     */
    void SetSprite(const char* file_name);

    /**
     * @brief 現在設定されているスプライトを取得する
     *
     * @return Sprite ポインタ（nullptr の可能性あり）
     */
    Sprite* GetSprite() const;

    /**
     * @brief アルファ値のトランジションを開始する
     *
     * @param target_alpha 目標アルファ値
     * @param duration 遷移時間（秒）
     * @param ease イージングタイプ
     */
    void StartAlphaTransition(float target_alpha,
        float duration,
        EaseType ease = EaseType::EaseOutQuad);

    /**
     * @brief アルファ乗算値のトランジションを開始する
     *
     * @param target_multiplier 目標乗算値
     * @param duration 遷移時間（秒）
     * @param ease イージングタイプ
     */
    void StartAlphaMultiplierTransition(float target_multiplier,
        float duration,
        EaseType ease = EaseType::EaseOutQuad);

    /**
     * @brief アルファ値を即座に設定する
     *
     * トランジションを介さずに即時反映される。
     *
     * @param alpha 設定するアルファ値
     */
    void SetAlphaImmediate(float alpha);

    /**
     * @brief アルファ乗算値を設定する
     *
     * @param multiplier 乗算値
     */
    void SetAlphaMultiplier(float multiplier);

    /**
     * @brief UI要素の位置を設定する
     *
     * @param position 新しい位置
     */
    virtual void SetPosition(const DirectX::XMFLOAT2& position) { position_ = position; }

    /**
     * @brief UI要素のサイズを設定する
     *
     * @param size 新しいサイズ
     */
    void SetSize(const DirectX::XMFLOAT2& size) { size_ = size; }

    /**
     * @brief UI要素の色を設定する
     *
     * @param color RGBAカラー
     */
    void SetColor(const DirectX::XMFLOAT4& color) { color_ = color; }

    /**
     * @brief UI要素の有効・無効を設定する
     *
     * @param is_valid true の場合描画対象となる
     */
    void SetValid(bool is_valid) { is_valid_ = is_valid; }

    /**
     * @brief 描画レイヤーを設定する
     *
     * @param render_layer レイヤー値
     */
    void SetRenderLayer(int render_layer) { render_layer_ = render_layer; }

    /**
     * @brief スプライト内の表示開始位置を設定する
     *
     * @param sprite_position スプライト内座標
     */
    void SetSpritePosition(const DirectX::XMFLOAT2& sprite_position) {
        sprite_position_ = sprite_position;
    }

    /**
     * @brief スプライト内の表示サイズを設定する
     *
     * @param sprite_size スプライトサイズ
     */
    void SetSpriteSize(const DirectX::XMFLOAT2& sprite_size) {
        sprite_size_ = sprite_size;
    }

    /**
     * @brief 現在の位置を取得する
     *
     * @return 位置
     */
    const DirectX::XMFLOAT2& GetPosition() const { return position_; }

    /**
     * @brief 現在のサイズを取得する
     *
     * @return サイズ
     */
    const DirectX::XMFLOAT2& GetSize() const { return size_; }

    /**
     * @brief 描画レイヤーを取得する
     *
     * @return レイヤー値
     */
    int GetRenderLayer() const { return render_layer_; }

    /**
     * @brief 有効状態かどうかを取得する
     *
     * @return true の場合有効
     */
    bool IsValid() const { return is_valid_; }

protected:
    /**
     * @brief 実際に描画に使用するアルファ値を計算する
     *
     * current_alpha_ と alpha_multiplier_ を考慮する。
     *
     * @return 計算後のアルファ値
     */
    float CalculateAlpha() const;

    /** スプライト */
    std::unique_ptr<Sprite> sprite_;

    /** UI要素の位置 */
    DirectX::XMFLOAT2 position_;

    /** UI要素のサイズ */
    DirectX::XMFLOAT2 size_;

    /** スプライト内の表示開始位置 */
    DirectX::XMFLOAT2 sprite_position_;

    /** スプライト内の表示サイズ */
    DirectX::XMFLOAT2 sprite_size_;

    /** 描画カラー（RGBA） */
    DirectX::XMFLOAT4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

    /** 描画レイヤー */
    int render_layer_;

    /** 有効フラグ */
    bool is_valid_;

    /** サイズ調整用オフセット */
    DirectX::XMFLOAT2 size_offset_ = { 0.0f, 0.0f };

    /** 現在のアルファ値 */
    float current_alpha_ = 1.0f;

    /** アルファ乗算値 */
    float alpha_multiplier_ = 1.0f;

    /** アルファ値用トゥイーン */
    FloatTween* alpha_tween_ = nullptr;

    /** アルファ乗算用トゥイーン */
    FloatTween* multiplier_tween_ = nullptr;
};

#endif
