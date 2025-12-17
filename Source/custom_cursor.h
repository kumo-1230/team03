#ifndef CUSTOM_CURSOR_H_
#define CUSTOM_CURSOR_H_

#include <DirectXMath.h>
#include <d3d11.h>
#include <memory>
#include <functional>
#include <vector>

class Sprite;
class Lerp;

/**
 * @class CustomCursor
 * @brief カスタムカーソルの描画・アニメーション管理を行うシングルトンクラス
 *
 * スプライトベースのカスタムカーソルを描画し、パーティクルエフェクト、
 * フェードイン/アウトなどの視覚効果を提供します。
 */
class CustomCursor
{
private:
    /**
     * @struct Particle
     * @brief カーソルのパーティクル情報
     */
    struct Particle
    {
        int type;                                 ///< パーティクルの種類
        std::unique_ptr<Lerp> lifetime_lerp;      ///< 寿命管理用Lerp
        DirectX::XMFLOAT2 position;               ///< パーティクルの位置
        float angle = 0.0f;                       ///< 回転角度
    };

public:
    /**
     * @brief シングルトンインスタンスを取得
     * @return CustomCursor& インスタンスへの参照
     */
    static CustomCursor& Instance();

    /**
     * @brief カスタムカーソルを初期化
     * @param sprite_path スプライト画像のパス（デフォルト: "Data/Sprite/new_cursor.png"）
     */
    void Initialize(const char* sprite_path = "Data/Sprite/new_cursor.png");

    /**
     * @brief カーソルの更新処理
     * @param elapsed_time 前フレームからの経過時間(秒)
     *
     * カーソル位置の追跡、パーティクルの更新、フェード処理を実行します。
     */
    void Update(float elapsed_time);

    /**
     * @brief カーソルを描画
     * @param dc Direct3Dデバイスコンテキスト
     */
    void Render(ID3D11DeviceContext* dc);

    /**
     * @brief カスタムカーソルを表示
     */
    void Show();

    /**
     * @brief カスタムカーソルを非表示
     */
    void Hide();

    /**
     * @brief フェードアウトを開始
     * @param duration フェード時間(秒)
     * @param on_complete フェード完了時のコールバック関数（オプション）
     */
    void FadeOut(float duration, std::function<void()> on_complete = nullptr);

    /**
     * @brief フェードインを開始
     * @param duration フェード時間(秒)
     * @param on_complete フェード完了時のコールバック関数（オプション）
     */
    void FadeIn(float duration, std::function<void()> on_complete = nullptr);

    /**
     * @brief アルファ値を即座に設定
     * @param alpha アルファ値（0.0f～1.0f）
     */
    void SetAlpha(float alpha);

    /**
     * @brief パーティクル生成間隔を設定
     * @param frames 生成間隔（フレーム数）
     */
    void SetParticleSpawnInterval(int frames);

    /**
     * @brief パーティクル生成を有効/無効化
     * @param enable trueで有効、falseで無効
     */
    void EnableParticles(bool enable);

    /**
     * @brief カーソルの表示状態を取得
     * @return bool 表示中の場合true
     */
    bool IsVisible() const;

    /**
     * @brief フェード中かどうかを取得
     * @return bool フェード中の場合true
     */
    bool IsFading() const;

    /**
     * @brief カーソルの現在位置を取得
     * @return DirectX::XMFLOAT2 カーソル位置
     */
    DirectX::XMFLOAT2 GetPosition() const;

private:
    /**
     * @brief コンストラクタ（シングルトンのためprivate）
     */
    CustomCursor() = default;

    /**
     * @brief デストラクタ
     */
    ~CustomCursor() = default;

    /**
     * @brief コピーコンストラクタ（削除）
     */
    CustomCursor(const CustomCursor&) = delete;

    /**
     * @brief 代入演算子（削除）
     */
    CustomCursor& operator=(const CustomCursor&) = delete;

    /**
     * @brief パーティクルの更新処理
     * @param elapsed_time 経過時間
     */
    void UpdateParticles(float elapsed_time);

    /**
     * @brief フェード処理の更新
     * @param elapsed_time 経過時間
     */
    void UpdateFade(float elapsed_time);

    /**
     * @brief パーティクルを生成すべきか判定
     * @return bool 生成する場合true
     */
    bool ShouldSpawnParticle();

    /**
     * @brief パーティクルを生成
     */
    void SpawnParticle();

    /**
     * @brief 現在のアルファ値を取得（フェード考慮）
     * @return float 現在のアルファ値
     */
    float GetCurrentAlpha() const;

    /**
     * @brief パーティクルを描画
     * @param dc デバイスコンテキスト
     * @param alpha 基本アルファ値
     */
    void RenderParticles(ID3D11DeviceContext* dc, float alpha);

    /**
     * @brief カーソル本体を描画
     * @param dc デバイスコンテキスト
     * @param alpha アルファ値
     */
    void RenderCursor(ID3D11DeviceContext* dc, float alpha);

    /**
     * @brief フェード処理を開始
     * @param start_alpha 開始アルファ値
     * @param target_alpha 目標アルファ値
     * @param duration フェード時間
     * @param callback 完了時のコールバック
     */
    void StartFade(float start_alpha, float target_alpha, float duration, std::function<void()> callback);

    /**
     * @brief フェード処理を停止
     */
    void StopFade();

private:
    std::unique_ptr<Sprite> sprite_;              ///< カーソルスプライト
    DirectX::XMFLOAT2 position_{};                ///< カーソル位置

    bool is_visible_ = true;                      ///< 表示状態
    float alpha_ = 1.0f;                          ///< アルファ値

    bool is_fading_ = false;                      ///< フェード中フラグ
    float fade_start_alpha_ = 1.0f;               ///< フェード開始アルファ値
    float fade_target_alpha_ = 1.0f;              ///< フェード目標アルファ値
    std::unique_ptr<Lerp> fade_lerp_;             ///< フェード用Lerp
    std::function<void()> on_fade_complete_;      ///< フェード完了時コールバック

    std::vector<Particle> particles_;             ///< パーティクルリスト
    int particle_timer_ = 0;                      ///< パーティクル生成タイマー
    int particle_spawn_interval_ = 30;            ///< パーティクル生成間隔
    bool particles_enabled_ = true;               ///< パーティクル有効フラグ
};

#endif  // CUSTOM_CURSOR_H_