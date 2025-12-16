#pragma once
#include "ui_panel.h"
#include "Lerp.h"
#include "System/AudioSource.h"
#include <memory>

struct ID3D11DeviceContext;
class Sprite;

class Pause {
public:
    enum class SENSITIVITY_TYPE {
        LOW = 0,
        NORMAL,
        HIGH,
    };

    enum class FOV_TYPE {
        NORMALE = 0,
        HIGH,
    };

    enum class SHAKE_TYPE {
        ON = 0,
        OFF
    };

    enum class DRUNKENNESS_TYPE {
        NONE = 0,
        LOW,
        HIGH,
    };

    static Pause& Instance() {
        static Pause pose;
        return pose;
    }

    // ポーズ状態制御
    void SetPose(bool b) { on_pause_ = b; }
    bool IsOnPose() const { return on_pause_; }

    // 設定値取得
    int GetSenitivity() const { return sensitivity; }
    int GetFov() const { return fov; }
    int GetShake() const { return shake; }
    int GetDrunkenness() const { return drunkenness; }

    // チュートリアル設定
    void SetTutorial(bool b) { tutorial = b; }
    bool IsTutorial() const { return tutorial; }

    // 更新・描画
    void Update(float elapsedTime);
    void Render(ID3D11DeviceContext* dc);

private:
    Pause();
    ~Pause();
    Pause(const Pause&) = delete;
    Pause& operator=(const Pause&) = delete;

    void Initialize();
    void PoseOffComplete();  // フェード完了後の処理

    // ポーズ状態
    bool on_pause_{ false };
    bool is_fade_in_{ false };
    bool is_fade_out_{ false };
    bool tutorial{ false };

    // UI パネル
    std::unique_ptr<UiPanel> pose_panel_{ nullptr };
    std::unique_ptr<UiPanel> settingMenu{ nullptr };

    // スプライト
    std::unique_ptr<Sprite> sprSetting{ nullptr };

    // オーディオ
    AudioSource* se_pause_{ nullptr };
    AudioSource* se_pause_back_{ nullptr };
    AudioSource* se_pause_select_{ nullptr };

    // 設定値
    int sensitivity{ 0 };
    int fov{ 0 };
    int shake{ 0 };
    int drunkenness{ 0 };

    int buttonSensitivity{ 0 };
    int buttonFov{ 0 };
    int buttonShake{ 0 };
    int buttonDrunkenness{ 0 };

    // 選択管理
    int holdNum{ -1 };
    int num{ -1 };
    int num_bank{ -1 };

    // アルファ値（必要に応じて）
    float alpha{ 0.0f };

    // ボタン配列
    UiButton* pose_buttons_[4]{ nullptr };
    UiButton* settingButtons[11]{ nullptr };
};