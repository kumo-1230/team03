#include "System/Graphics.h"
#include "PlayerOld.h"
#include "common.h"
#include "System/Input.h"
#include "SceneManager.h"
#include "SceneResult.h"
#include "SceneTitle.h"
#include "System/ModelRenderer.h"
#include "ScoreRender.h"
#include "Pose.h"
#include "CursorManager.h"
#include "mathUtils.h"
#include "Screen.h"
#include "System/Audio.h"
#include "SceneLoading.h"
#include "SceneGame.h"
#include "EffectManager.h"
#include "Cursor.h"
#include "SceneTutorial.h"

SceneTutorial::SceneTutorial() {}

void SceneTutorial::Initialize() {
    Graphics& graphics = Graphics::Instance();

    BGMTutorial = Audio::Instance().LoadAudioSource("Data/Sound/Tutorial/BGM_tutorial.wav");
    BGMTutorial->Play(true);

    skyMap = std::make_unique<sky_map>(graphics.GetDevice(),
        L"Data/SkyMapSprite/game_background3.hdr");
    player = std::make_unique<PlayerOld>();

    cameraController = std::make_unique<CameraController>();

    camera = std::make_unique<Camera>();
    DirectX::XMFLOAT3 eye = player->GetPosition();
    DirectX::XMFLOAT3 focus{};
    focus.x = sinf(player->GetAngle().y);
    focus.z = cosf(player->GetAngle().y);
    float fov = 80.0f;
    camera->SetLookAt(eye, focus, DirectX::XMFLOAT3(0, 1, 0));
    camera->SetPerspectiveFov(
        DirectX::XMConvertToRadians(fov),
        graphics.GetScreenWidth() / graphics.GetScreenHeight(),
        0.1f,
        100000.0f
    );

    tutorialMenu = std::make_unique<UiPanel>();

    Pose::Instance().SetTutorial(true);
}

void SceneTutorial::Finalize() {
    player->Finalize();
    BGMTutorial->Stop();
    EffectManager::Instance().Initialize();
    delete BGMTutorial;
}

void SceneTutorial::Update(float elapsedTime) {
    Cursor::Instance().Update(elapsedTime);
    cameraController->Update(elapsedTime, camera.get(), 0, 0);

    int clicked_button_id = -1;
    if (tutorialMenu) {
        tutorialMenu->Update(&clicked_button_id);
    }
}

void SceneTutorial::Render() {
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    ShapeRenderer* shapeRenderer = graphics.GetShapeRenderer();
    ModelRenderer* modelRenderer = graphics.GetModelRenderer();

    ID3D11Device* device = graphics.GetDevice();
    D3D11_RASTERIZER_DESC rasterDesc{};
    rasterDesc.FillMode = D3D11_FILL_SOLID;
    rasterDesc.CullMode = D3D11_CULL_BACK;
    rasterDesc.FrontCounterClockwise = FALSE;
    rasterDesc.DepthClipEnable = TRUE;
    ID3D11RasterizerState* rasterState = nullptr;
    device->CreateRasterizerState(&rasterDesc, &rasterState);
    dc->RSSetState(rasterState);
    rasterState->Release();

    RenderContext rc;
    rc.deviceContext = dc;
    rc.renderState = graphics.GetRenderState();
    rc.camera = camera.get();

    DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.camera->GetView());
    DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.camera->GetProjection());
    DirectX::XMMATRIX VP = V * P;
    DirectX::XMFLOAT4X4 vp;
    DirectX::XMStoreFloat4x4(&vp, VP);
    DirectX::XMFLOAT3 Cpos = camera->GetEye();

    skyMap->blit(rc, vp, { Cpos.x, Cpos.y, Cpos.z, 1.0f });

    if (tutorialMenu) {
        tutorialMenu->Render(dc, MenuBackgroundMode::kBackgroundVisible, false);
    }

    Cursor::Instance().Render(dc);
}

void SceneTutorial::DrawGUI() {}