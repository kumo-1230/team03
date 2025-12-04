#include "System/Graphics.h"
#include "SceneTitle.h"
#include "System/Input.h"
#include "SceneGame.h"
#include "SceneManager.h"
#include "SceneLoading.h"
#include "CursorManager.h"
#include "common.h"
#include "System/Audio.h"
#include "SceneTutorial.h"
#include "Cursor.h"

SceneTitle::SceneTitle() {}

void SceneTitle::Initialize() {
    Graphics& graphics = Graphics::Instance();

    sprTitle = std::make_unique<Sprite>("Data/Sprite/game_logo_new.png");
    sprButtonBack = std::make_unique<Sprite>("Data/Sprite/title_square.png");
    sprFadeLoad = std::make_unique<Sprite>("Data/Sprite/load_background.png");

    titleStartMenu = std::make_unique<UiPanel>("");

    titleStartMenu->AddButton(std::make_unique<UiButton>(
        "Data/Sprite/gamestart_text_new.png",
        DirectX::XMFLOAT2{ SCREEN_W * 0.2f - 351 * 0.5f - 100,
                          SCREEN_H * 0.8f - 96 * 0.5f - 80 },
        DirectX::XMFLOAT2{ 351, 96 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 351, 96 },
        0,
        0,
        [this]() {
            if (clickSE) clickSE->Play(false);
            SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame()));
        },
        true
    ));

    titleStartMenu->AddButton(std::make_unique<UiButton>(
        "Data/Sprite/tutorial_text_new.png",
        DirectX::XMFLOAT2{ SCREEN_W * 0.2f - 633 * 0.5f + 40,
                          SCREEN_H * 0.9f - 96 * 0.5f - 30 },
        DirectX::XMFLOAT2{ 633, 96 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 633, 96 },
        0,
        1,
        [this]() {
            if (clickSE) clickSE->Play(false);
            SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTutorial()));
        },
        true
    ));

    clickSE = Audio::Instance().LoadAudioSource("Data/Sound/title/SE_title_click.wav");
    onCursorSE = Audio::Instance().LoadAudioSource("Data/Sound/title/SE_title_cursor.wav");
    onStartSE = Audio::Instance().LoadAudioSource("Data/Sound/title/SE_title_zoom.wav");
    backGroundMusic = Audio::Instance().LoadAudioSource("Data/Sound/title/BGM_title.wav");

    skyMap = std::make_unique<sky_map>(graphics.GetDevice(),
        L"Data/SkyMapSprite/game_background3.hdr");

    backGroundMusic->SetVolume(1.0f);
    onCursorSE->SetVolume(20);
    onStartSE->SetVolume(20);
    clickSE->SetVolume(20);

    titleStartMenu->SetActive(true);

    camera = std::make_unique<Camera>();
    DirectX::XMFLOAT3 eye{ 0, 0, 0 };
    DirectX::XMFLOAT3 focus{ 1, 0, 0 };
    camera->SetLookAt(eye, focus, DirectX::XMFLOAT3(0, 1, 0));
    camera->SetPerspectibeFov(
        DirectX::XMConvertToRadians(45),
        graphics.GetScreenWidth() / graphics.GetScreenHeight(),
        0.1f,
        100000.0f
    );

    cameraController = std::make_unique<CameraController>();
    cameraController->SetEye({ 0, 0, 0 });
    cameraController->SetTarget({ 0, 0, 1 });

    lastHoveredButtonId = -1;
}

void SceneTitle::Finalize() {
    cameraController.reset();
    backGroundMusic->Stop();
    clickSE->Stop();
    onStartSE->Stop();
    onCursorSE->Stop();
    delete backGroundMusic;
    delete clickSE;
    delete onStartSE;
    delete onCursorSE;
}

void SceneTitle::Update(float elapsedTime) {
    backGroundMusic->Play(true);
    Cursor::Instance().Update(elapsedTime);
    cameraController->Updeate(elapsedTime, camera.get(), 0, 0);

    int clicked_button_id = -1;
    titleStartMenu->Update(&clicked_button_id);

    int hovered_button_id = -1;
    const auto& input_state = titleStartMenu->GetInputState();
    if (input_state.is_button_held || input_state.is_button_down) {
        for (int i = 0; i < 2; ++i) {
            if (titleStartMenu->IsActive()) {
                hovered_button_id = i;
                break;
            }
        }
    }

    if (hovered_button_id != lastHoveredButtonId && hovered_button_id != -1) {
        if (onCursorSE) onCursorSE->Play(false);
        lastHoveredButtonId = hovered_button_id;
    }

    if (hovered_button_id == -1) {
        lastHoveredButtonId = -1;
    }
}

void SceneTitle::Render() {
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    RenderState* renderState = graphics.GetRenderState();

    RenderContext rc;
    rc.deviceContext = dc;
    rc.renderState = graphics.GetRenderState();
    ModelRenderer* modelRenderer = graphics.GetModelRenderer();

    rc.view = camera->GetView();
    rc.projection = camera->GetProjection();
    DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
    DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
    DirectX::XMMATRIX VP = V * P;
    DirectX::XMFLOAT4X4 vp;
    DirectX::XMStoreFloat4x4(&vp, VP);
    DirectX::XMFLOAT3 Cpos = camera->GetEye();

    skyMap->blit(rc, vp, { Cpos.x, Cpos.y, Cpos.z, 1.0f });

    sprTitle->Render(rc, 50, 50, 0, 656, 188, 0, 1, 1, 1, 1);
    titleStartMenu->Render(rc, MenuBackgroundMode::kBackgroundVisible, false);
    Cursor::Instance().Render(rc);
}

void SceneTitle::DrawGUI() {}