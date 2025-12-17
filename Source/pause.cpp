#include "pause.h"
#include "common.h"
#include "scene_manager.h"
#include "scene_loading.h"
#include "scene_game.h"
#include "scene_title.h"
#include "input_manager.h"
#include "System/audio.h"
#include "System/graphics.h"
#include "render_layer.h"
#include <imgui_logger.h>
#include <custom_cursor.h>
#include <system_cursor.h>

Pause::Pause() {
    Initialize();
}

Pause::~Pause() {
    delete se_pause_;
    delete se_pause_back_;
    delete se_pause_select_;
}

void Pause::Initialize() {
    auto* device = Graphics::Instance().GetDevice();

    se_pause_ = Audio::Instance().LoadAudioSource("Data/Sound/Game/SE_game_pause.wav");
    se_pause_->SetVolume(1.0f);
    se_pause_back_ = Audio::Instance().LoadAudioSource("Data/Sound/Pause/SE_setting_back.wav");
    se_pause_select_ = Audio::Instance().LoadAudioSource("Data/Sound/Pause/SE_setting_select.wav");

    // ポーズメニュー作成（背景付き）
    pose_panel_ = std::make_unique<UiPanel>("./Data/Sprite/setting_default.png");

    pose_buttons_[0] = pose_panel_->AddButton(
        "./Data/Sprite/pause_button.png",
        DirectX::XMFLOAT2{ SCREEN_W_CENTER - 417.0f / 2.0f, 50 },
        DirectX::XMFLOAT2{ 417, 476 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 417, 476 },
        0  // action_id
    );

    // ホバー時のSE再生
    pose_buttons_[0]->SetOnHoverCallback([this]() {
        if (se_pause_select_) {
            se_pause_select_->Stop();
            se_pause_select_->Play(false);
        }
        });

    pose_buttons_[0]->SetOnClickCallback([this]() {
		ImGuiLogger::Instance().AddLog(u8"ポーズメニューの設定ボタンがクリックされました。");
		});

    pose_panel_->SetActive(false);
}

void Pause::Update(float elapsedTime) {
    auto& input = InputManager::Instance();

    // ポーズの切り替え
    {
        // ポーズ開始
        if (input.IsKeyDown('P') && !on_pause_) {
            on_pause_ = true;
            is_fade_in_ = true;
            pose_panel_->SetActive(true);
            pose_panel_->FadeIn(0.3f);

            se_pause_->Stop();
            se_pause_->Play(false);

            // カスタムカーソルをフェードインで表示
            CustomCursor::Instance().FadeIn(0.3f);
            SystemCursor::Hide();  // システムカーソルは非表示
        }
        // ポーズ解除
        else if (input.IsKeyDown('P') && on_pause_) {
            is_fade_out_ = true;
            is_fade_in_ = false;

            se_pause_->Stop();
            se_pause_->Play(false);

			pose_panel_->FadeOut(0.3f);

            // カスタムカーソルをフェードアウト
            CustomCursor::Instance().FadeOut(0.3f, [this]() {
                // フェード完了後の処理
                PoseOffComplete();
                });
        }
    }

    if (!on_pause_) return;

    // UIパネルの更新
    pose_panel_->Update();
}

void Pause::Render(ID3D11DeviceContext* dc) {
    if (!on_pause_) return;

    pose_panel_->Render(dc, MenuBackgroundMode::kBackgroundVisible, false);
}

void Pause::PoseOffComplete() {
    on_pause_ = false;
    is_fade_in_ = true;
    is_fade_out_ = false;
    num_bank = -1;

    pose_panel_->SetActive(false);
    if (settingMenu) {
        settingMenu->SetActive(false);
    }

    // カーソルを画面中央に移動
    SystemCursor::CenterCursor();
}