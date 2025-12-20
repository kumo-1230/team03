#include "System/Graphics.h"
#include "scene_title.h"
#include "System/Input.h"
#include "scene_game.h"
#include "scene_manager.h"
#include "scene_loading.h"
#include "common.h"
#include "System/Audio.h"
#include "input_manager.h"
#include "render_layer.h"
#include <world.h>
#include "camera_controller.h"
#include <system_cursor.h>
#include <custom_cursor.h>

SceneTitle::SceneTitle() {}

void SceneTitle::Initialize() {
    Graphics& graphics = Graphics::Instance();
    auto* device = graphics.GetDevice();

    auto& world = World::Instance();
	world.Clear();

    sprFadeLoad = std::make_unique<Sprite>(device, "Data/Sprite/load_background.png");

    titleStartMenu = std::make_unique<UiPanel>();

    // ロゴスプライト追加
    auto* spr_logo = titleStartMenu->AddSprite();
    spr_logo->SetSprite("Data/Sprite/game_logo_new.png");
    spr_logo->SetPosition(DirectX::XMFLOAT2{ SCREEN_W * 0.2f - 512 * 0.5f, 100 });
    spr_logo->SetSize(DirectX::XMFLOAT2{ 601, 224 });
    spr_logo->SetRenderLayer(RenderLayer::kDefault);
    spr_logo->SetSpritePosition({ 0, 0 });
    spr_logo->SetSpriteSize({ 601, 224 });

    // スタートボタン追加
    auto* startButton = titleStartMenu->AddButton(
        "Data/Sprite/gamestart_text_new.png",
        DirectX::XMFLOAT2{ SCREEN_W * 0.2f - 351 * 0.5f - 100,
                          SCREEN_H * 0.8f - 96 * 0.5f - 80 },
        DirectX::XMFLOAT2{ 351, 96 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 351, 96 },
        0,  // action_id
        [this]() {  // on_click_callback
            if (clickSE) {
                clickSE->Stop();
                clickSE->Play(false);
            }
            SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame()));
        },
        RenderLayer::kButton
    );
    startButton->SetRenderLayer(RenderLayer::kDefault);

    // ホバー時のコールバック設定
    startButton->SetOnHoverCallback([this]() {
        if (onCursorSE) {
            onCursorSE->Stop();
            onCursorSE->Play(false);
        }
        });

    // チュートリアルボタン追加
    auto* tutorialButton = titleStartMenu->AddButton(
        "Data/Sprite/tutorial_text_new.png",
        DirectX::XMFLOAT2{ SCREEN_W * 0.2f - 633 * 0.5f + 40,
                          SCREEN_H * 0.9f - 96 * 0.5f - 30 },
        DirectX::XMFLOAT2{ 633, 96 },
        DirectX::XMFLOAT2{ 0, 0 },
        DirectX::XMFLOAT2{ 633, 96 },
        1,  // action_id
        [this]() {  // on_click_callback
            if (clickSE) {
                clickSE->Stop();
                clickSE->Play(false);
            }
            //シーン作ってない
            //SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTutorial()));
        },
        RenderLayer::kButton
    );

    // ホバー時のコールバック設定
    tutorialButton->SetOnHoverCallback([this]() {
        if (onCursorSE) {
            onCursorSE->Stop();
            onCursorSE->Play(false);
        }
        });

    // パネルをフェードイン
    titleStartMenu->SetActive(true);
    titleStartMenu->FadeIn(1.0f);

    // オーディオ読み込み
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


    // カメラ設定
    //camera = std::make_unique<CameraController>();
    //DirectX::XMFLOAT3 eye{ 0, 0, 0 };
    //DirectX::XMFLOAT3 focus{ 1, 0, 0 };
    //camera->SetLookAt(eye, focus, DirectX::XMFLOAT3(0, 1, 0));
    //camera->SetPerspectiveFov(
    //    DirectX::XMConvertToRadians(45),
    //    graphics.GetScreenWidth() / graphics.GetScreenHeight(),
    //    0.1f,
    //    100000.0f
    //);

    camera_controller_ = world.CreateObject<CameraController>();
    camera_controller_->GetCamera()->SetEye({ 0, 0, 0 });
    camera_controller_->GetCamera()->SetFocus({ 0, 0, 1 });

    // ライト設定
    {
        DirectionalLight directionalLight;
        DirectX::XMFLOAT3 dir = { 0.3f, -1.0f, 0.3f };
        DirectX::XMVECTOR Dir = DirectX::XMLoadFloat3(&dir);
        Dir = DirectX::XMVector3Normalize(Dir);
        DirectX::XMStoreFloat3(&directionalLight.direction, Dir);
        directionalLight.color = { 1.5f, 1.5f, 1.5f };
        lightManager.SetDirectionalLight(directionalLight);

        PointLight mapLight;
        mapLight.position = { 0, 1, 0 };
        mapLight.range = 12.0f;
        mapLight.color = { 1.0f, 0.1f, 0.1f };
        mapLight.intensity = 8.0f;
        mapLight.priority = 10;
        lightManager.AddPointLight(mapLight);

        DirectX::XMFLOAT3 playerPos = { 0, 0, 0 };
        playerPos.y += 1.0f;
        DirectX::XMFLOAT3 spotDirection = camera_controller_->GetCamera()->GetFront();

        lightManager.SetPlayerSpotLight(
            playerPos, spotDirection,
            20.0f, 25.0f, 40.0f,
            { 1.0f, 0.95f, 0.85f }, 8.0f
        );
    }

    // カーソル設定（タイトル画面ではカスタムカーソルを表示）
    SystemCursor::Hide();           // システムカーソルを非表示
    CustomCursor::Instance().Show(); // カスタムカーソルを表示
    CustomCursor::Instance().FadeIn(1.0f); // フェードインで登場
}

void SceneTitle::Finalize() {
    backGroundMusic->Stop();
    clickSE->Stop();
    onStartSE->Stop();
    onCursorSE->Stop();
    delete backGroundMusic;
    delete clickSE;
    delete onStartSE;
    delete onCursorSE;

    // カーソルをリセット（次のシーンのために）
    CustomCursor::Instance().Hide();

    World::Instance().Clear();
}

void SceneTitle::Update(float elapsedTime) {
    backGroundMusic->Play(true);

    // UIパネルを更新
    titleStartMenu->Update();

    // イベント処理
    //auto events = titleStartMenu->GetPendingEvents();
    //for (const auto& event : events) {
    //    switch (event.type) {
    //    case UiEventType::Click:
    //        // クリックイベントの処理（コールバックで既に処理されている）
    //        break;
    //    case UiEventType::HoverBegin:
    //        // ホバー開始時の追加処理（必要なら）
    //        break;
    //    case UiEventType::HoverEnd:
    //        // ホバー終了時の追加処理
    //        break;
    //    default:
    //        break;
    //    }
    //}
}

void SceneTitle::Render() {
    Graphics& graphics = Graphics::Instance();
    ID3D11DeviceContext* dc = graphics.GetDeviceContext();
    RenderState* rs = graphics.GetRenderState();
    RenderContext rc;
    rc.deviceContext = dc;
    rc.renderState = rs;
    rc.camera = camera_controller_->GetCamera();
    rc.lightManager = &lightManager;

    // レンダーステート設定
    dc->RSSetState(rs->GetRasterizerState(RasterizerState::SolidCullNone));
    dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::TestAndWrite), 0);
    ID3D11SamplerState* s = rs->GetSamplerState(SamplerState::LinearClamp);
    dc->PSSetSamplers(0, 1, &s);
    const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    dc->OMSetBlendState(rs->GetBlendState(BlendState::Opaque), blendFactor, 0xffffffff);

    // スカイマップ描画
    DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.camera->GetView());
    DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.camera->GetProjection());
    DirectX::XMMATRIX VP = V * P;
    DirectX::XMFLOAT4X4 vp;
    DirectX::XMStoreFloat4x4(&vp, VP);
    DirectX::XMFLOAT3 Cpos = camera_controller_->GetCamera()->GetEye();
    skyMap->blit(rc, vp, { Cpos.x, Cpos.y, Cpos.z, 1.0f });

    // UI描画
    dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
    dc->OMSetBlendState(rs->GetBlendState(BlendState::Transparency), blendFactor, 0xffffffff);
    dc->PSSetSamplers(0, 1, &s);

    titleStartMenu->Render(dc, MenuBackgroundMode::kBackgroundVisible, false);

}

void SceneTitle::DrawGUI() {}