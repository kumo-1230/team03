#include "Pose.h"
#include "common.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "SceneGame.h"
#include "SceneTitle.h"
#include "KeyInput.h"
#include "System/Audio.h"
#include "SceneTutorial.h"
#include "System/Graphics.h"

Pose::Pose() {
    Initializer();
}

Pose::~Pose() {
    delete setPause;
    delete pauseBackSE;
    delete pauseSelectSE;
}

void Pose::Initializer() {
	auto* device = Graphics::Instance().GetDevice();
    sprSettingBack = std::make_unique<Sprite>(device, "./Data/Sprite/setting_default.png");
    sprPoseBack = std::make_unique<Sprite>(device, "./Data/Sprite/pause_default.png");

    setPause = Audio::Instance().LoadAudioSource("Data/Sound/Game/SE_game_pause.wav");
    setPause->SetVolume(1.0f);
    pauseBackSE = Audio::Instance().LoadAudioSource("Data/Sound/Pause/SE_setting_back.wav");
    pauseSelectSE = Audio::Instance().LoadAudioSource("Data/Sound/Pause/SE_setting_select.wav");

    poseMenu = std::make_unique<UiPanel>();

    poseButtons[0] = poseMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/pause_button.png",
        DirectX::XMFLOAT2{ SCREEN_W_CENTER - 417.0f / 2.0f, 50 },
        DirectX::XMFLOAT2{ 417, 476 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 417, 476 },
        0, 0,
        [this]() {
            poseMenu->SetActive(false);
            settingMenu->SetActive(true);
            numBank = -1;
        },
        true
    ));

    poseButtons[1] = poseMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/pause_button.png",
        DirectX::XMFLOAT2{ SCREEN_W_CENTER - 417.0f / 2.0f, SCREEN_H_CENTER },
        DirectX::XMFLOAT2{ 417, 476 },
        DirectX::XMFLOAT2{ 0, 476 },
        DirectX::XMFLOAT2{ 417, 476 },
        0, 1,
        [this]() {
            PoseOff();
            fadeIn = false;
        },
        true
    ));

    poseButtons[2] = poseMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/pause_button.png",
        DirectX::XMFLOAT2{ SCREEN_W_CENTER + 417, 0 },
        DirectX::XMFLOAT2{ 417, 476 },
        DirectX::XMFLOAT2{ 0, 476 * 2 },
        DirectX::XMFLOAT2{ 417, 476 },
        0, 2,
        [this]() {
            if (PoseOff()) {
                SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle()));
            }
            fadeIn = false;
        },
        true
    ));

    poseButtons[3] = poseMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/pause_button.png",
        DirectX::XMFLOAT2{ SCREEN_W_CENTER + 417, SCREEN_H_CENTER },
        DirectX::XMFLOAT2{ 417, 476 },
        DirectX::XMFLOAT2{ 0, 476 * 3 },
        DirectX::XMFLOAT2{ 417, 476 },
        0, 3,
        [this]() {
            if (PoseOff()) {
                if (tutorial)
                    SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTutorial()));
                else
                    SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame()));
            }
            fadeIn = false;
        },
        true
    ));

    poseMenu->SetActive(false);

    settingMenu = std::make_unique<UiPanel>();

    settingButtons[0] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 900, 145 },
        DirectX::XMFLOAT2{ 110, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 0,
        [this]() {
            sensitivity = static_cast<int>(SENSITIVITY_TYPE::HIGH);
            buttonSensitivity = 0;
        },
        true
    ));

    settingButtons[1] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 1150, 145 },
        DirectX::XMFLOAT2{ 110, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 1,
        [this]() {
            sensitivity = static_cast<int>(SENSITIVITY_TYPE::NORMALE);
            buttonSensitivity = 1;
        },
        true
    ));

    settingButtons[2] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 1425, 145 },
        DirectX::XMFLOAT2{ 110, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 2,
        [this]() {
            sensitivity = static_cast<int>(SENSITIVITY_TYPE::LOW);
            buttonSensitivity = 2;
        },
        true
    ));

    settingButtons[3] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 960, 390 },
        DirectX::XMFLOAT2{ 165, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 3,
        [this]() {
            fov = static_cast<int>(FOV_TYPE::HIGH);
            buttonFov = 3;
        },
        true
    ));

    settingButtons[4] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 1290, 390 },
        DirectX::XMFLOAT2{ 220, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 4,
        [this]() {
            fov = static_cast<int>(FOV_TYPE::NORMALE);
            buttonFov = 4;
        },
        true
    ));

    settingButtons[5] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 960, 640 },
        DirectX::XMFLOAT2{ 165, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 5,
        [this]() {
            shake = static_cast<int>(SHAKE_TYPE::ON);
            buttonShake = 5;
        },
        true
    ));

    settingButtons[6] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 1320, 640 },
        DirectX::XMFLOAT2{ 165, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 6,
        [this]() {
            shake = static_cast<int>(SHAKE_TYPE::OFF);
            buttonShake = 6;
        },
        true
    ));

    settingButtons[7] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 840, 940 },
        DirectX::XMFLOAT2{ 165, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 7,
        [this]() {
            drunkenness = static_cast<int>(DRUNKENNESS_TYPE::HIGH);
            buttonDrunkenness = 7;
            shake = static_cast<int>(SHAKE_TYPE::OFF);
            buttonShake = 6;
        },
        true
    ));

    settingButtons[8] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 1120, 940 },
        DirectX::XMFLOAT2{ 165, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 8,
        [this]() {
            drunkenness = static_cast<int>(DRUNKENNESS_TYPE::LOW);
            buttonDrunkenness = 8;
        },
        true
    ));

    settingButtons[9] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/empty.png",
        DirectX::XMFLOAT2{ 1400, 940 },
        DirectX::XMFLOAT2{ 165, 64 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 0, 0 },
        0, 9,
        [this]() {
            drunkenness = static_cast<int>(DRUNKENNESS_TYPE::NONE);
            buttonDrunkenness = 9;
        },
        true
    ));

    settingButtons[10] = settingMenu->AddButton(std::make_unique<UiButton>(
        "./Data/Sprite/setting_back.png",
        DirectX::XMFLOAT2{ SCREEN_W - 246.0f, 0 },
        DirectX::XMFLOAT2{ 246, 105 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 246, 105 },
        0, 10,
        [this]() {
            poseMenu->SetActive(true);
            settingMenu->SetActive(false);
        },
        true
    ));

    settingMenu->SetActive(false);

    sensitivity = static_cast<int>(SENSITIVITY_TYPE::NORMALE);
    fov = static_cast<int>(FOV_TYPE::NORMALE);
    shake = static_cast<int>(SHAKE_TYPE::ON);
    drunkenness = static_cast<int>(DRUNKENNESS_TYPE::NONE);

    buttonSensitivity = 1;
    buttonFov = 4;
    buttonShake = 5;
    buttonDrunkenness = 9;

    sprSetting = std::make_unique<Sprite>(device, "Data/Sprite/setting_switch.png");

    lerp = std::make_unique<Lerp>(0.1f);
    lerpButton = std::make_unique<Lerp>(0.1f);
}

void Pose::Update(float elapsedTime) {
    {
        if (KeyInput::Instance().GetKeyDown('P') && onPose == false) {
            onPose = true;
            fadeIn = true;

            poseMenu->SetActive(true);
            settingMenu->SetActive(false);
            lerp->SetAmount(0.0f);
            poseMenu->SetAlphaTransition(true);
            poseMenu->SetTargetAlpha(0.5f);

            setPause->Stop();
            setPause->Play(false);
        }
        else if (KeyInput::Instance().GetKeyDown('P') && onPose == true) {
            fadeOut = true;
            fadeIn = false;

            setPause->Stop();
            setPause->Play(false);
        }
        if (fadeOut) PoseOff();
    }

    if (!onPose) return;

    if (fadeIn) {
        lerp->Update(elapsedTime, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::ADD);
        alhpa = lerp->GetOffset(Lerp::EASING_TYPE::Normal, 0.0f, 1.0f);
        poseMenu->SetAlphaProgress(lerp->GetProgress(Lerp::EASING_TYPE::Normal));
        lerpButton->SetAmount(0.0f);
        if (lerp->GetAmount() >= lerp->GetMaxAmount()) {
            poseMenu->SetAlphaTransition(false);
        }
    }
    else {
        lerp->Update(elapsedTime, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::Subtract);
        lerpButton->Update(elapsedTime, Lerp::RESET_TYPE::Fixed, Lerp::ADD_TYPE::ADD);
        alhpa = lerp->GetOffset(Lerp::EASING_TYPE::Normal, 0.0f, 1.0f);
        poseMenu->SetAlphaTransition(true);
        poseMenu->SetTargetAlpha(0.0f);
        poseMenu->SetAlphaProgress(lerpButton->GetProgress(Lerp::EASING_TYPE::Normal));
    }

    if (poseMenu->IsActive()) {
        poseMenu->Update(&num);

        if (num != -1) {
            pauseSelectSE->Stop();
            pauseSelectSE->Play(false);
        }
    }
    else if (settingMenu->IsActive()) {
        settingMenu->Update(&num);

        const auto& input_state = settingMenu->GetInputState();
        if (input_state.is_button_held || input_state.is_button_down) {
            for (int i = 0; i < 11; ++i) {
                if (settingButtons[i] && settingButtons[i]->IsHovered()) {
                    holdNum = settingButtons[i]->GetActionId();
                    break;
                }
            }
        }

        if (num != -1) {
            pauseSelectSE->Stop();
            pauseSelectSE->Play(false);
        }
    }

    num = -1;
}

void Pose::Render(ID3D11DeviceContext* dc) {
    if (!onPose) return;

    sprPoseBack->Render(dc, 0, 0, 0, SCREEN_W, SCREEN_H, 0.0f, 1, 1, 1, alhpa);

    if (poseMenu->IsActive()) {
        poseMenu->Render(dc, MenuBackgroundMode::kBackgroundVisible, false);
    }
    else if (settingMenu->IsActive()) {
        sprSettingBack->Render(dc, 0, 0, 0, SCREEN_W, SCREEN_H, 0.0f, 1, 1, 1, alhpa);

        int renderMode = 0;

        for (int i = 0; i < 11; ++i) {
            if (!settingButtons[i]) continue;

            UiButton* b = settingButtons[i];
            int mode = b->GetActionId();

            if (mode == holdNum) {
                renderMode = 1;
            }

            if (mode <= 2 && mode == buttonSensitivity) {
                renderMode = 2;
            }
            else if (mode <= 4 && mode == buttonFov) {
                renderMode = 2;
            }
            else if (mode <= 6 && mode == buttonShake) {
                renderMode = 2;
            }
            else if (mode <= 9 && mode == buttonDrunkenness) {
                renderMode = 2;
            }

            if (i != 10) {
                DirectX::XMFLOAT2 pos = b->GetPosition();
                sprSetting->Render(dc, pos.x, pos.y + 5.0f, 0.0f,
                    56, 56,
                    56.0f * renderMode, 0, 56, 56,
                    0.0f,
                    1, 1, 1, alhpa);
            }

            renderMode = 0;
        }
        settingMenu->Render(dc, MenuBackgroundMode::kBackgroundVisible, false);
    }
}

bool Pose::PoseOff() {
    if (lerp->GetAmount() <= 0.0f) {
        onPose = false;
        fadeIn = true;
        fadeOut = false;
        numBank = -1;
        poseMenu->SetActive(false);
        settingMenu->SetActive(false);

        POINT screenCenter{ (LONG)(SCREEN_W / 2), (LONG)(SCREEN_H / 2) };
        ClientToScreen(Graphics::Instance().GetWindowHandle(), &screenCenter);
        SetCursorPos(screenCenter.x, screenCenter.y);
        return true;
    }
    return false;
}