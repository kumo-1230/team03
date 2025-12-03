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

SceneTitle::SceneTitle()
{
}

//初期化
void SceneTitle::Initialize()
{
	Graphics& graphics = Graphics::Instance();

	//スプライト初期化
	sprTitle = std::make_unique<Sprite>("Data/Sprite/game_logo_new.png");
	sprButtonBack = std::make_unique<Sprite>("Data/Sprite/title_square.png");
	sprFadeLoad = std::make_unique<Sprite>("Data/Sprite/load_background.png");
	//sprCursor = std::make_unique<Sprite>("Data/Sprite/cursor.png");

	//タイトル画面の分岐ボタン
	titleStartMenu.reset(new Menu());
	titleStartMenu->SetButton("Data/Sprite/gamestart_text_new.png", { SCREEN_W * 0.2f - 351 * 0.5f - 100,SCREEN_H * 0.8 - 96 * 0.5f - 80 }, { 351,96 }, { 0,0 }, { 351,96 }, 0, 0, true);
	titleStartMenu->SetButton("Data/Sprite/tutorial_text_new.png", { SCREEN_W * 0.2f - 633 * 0.5f + 40,SCREEN_H * 0.9 - 96 * 0.5f - 30 }, { 633,96 }, { 0,0 }, { 633, 96 }, 0, 1, true);

	clickSE = Audio::Instance().LoadAudioSource("Data/Sound/title/SE_title_click.wav");
	onCursorSE = Audio::Instance().LoadAudioSource("Data/Sound/title/SE_title_cursor.wav");
	onStartSE = Audio::Instance().LoadAudioSource("Data/Sound/title/SE_title_zoom.wav");
	backGroundMusic = Audio::Instance().LoadAudioSource("Data/Sound/title/BGM_title.wav");

	skyMap = std::make_unique<sky_map>(graphics.GetDevice(), L"Data/SkyMapSprite/game_background3.hdr");

	backGroundMusic->SetVolume(1.0f);
	onCursorSE->SetVolume(20);
	onStartSE->SetVolume(20);
	clickSE->SetVolume(20);

	titleStartMenu->SetMenuStart(true);

	//カメラ初期化
	camera.reset(new Camera);

	DirectX::XMFLOAT3 eye{0,0,0};
	DirectX::XMFLOAT3 focus{1,0,0};

	camera->SetLookAt(
		eye,
		focus,
		DirectX::XMFLOAT3(0, 1, 0)
	);
	camera->SetPerspectibeFov(
		DirectX::XMConvertToRadians(45),
		graphics.GetScreenWidth() / graphics.GetScreenHeight(),
		0.1f,
		100000.0f
	);

	//カメラコントローラー初期化
	cameraController = std::make_unique<CameraController>();
	cameraController->SetEye({ 0,0,0 });
	cameraController->SetTarget({ 0,0,1 });

}

//終了化
void SceneTitle::Finalize()
{
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

//更新処理
void SceneTitle::Update(float elapsedTime)
{
	backGroundMusic->Play(true);

	Cursor::Instance().Update(elapsedTime);
	cameraController->Updeate(elapsedTime, camera.get(), 0, 0);

	if (KeyInput::Instance().GetKeyDown(VK_LBUTTON))
	{
			SceneManager::Instance().ChangeScene(new SceneLoading(new SceneGame()));
	}
}

//描画処理
void SceneTitle::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	RenderState* renderState = graphics.GetRenderState();

	//描画準備
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = graphics.GetRenderState();
	ModelRenderer* modelRenderer = graphics.GetModelRenderer();

	//カメラパラメータ設定
	rc.view = camera->GetView();
	rc.projection = camera->GetProjection();


	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
	DirectX::XMMATRIX VP = V * P;
	DirectX::XMFLOAT4X4 vp;
	DirectX::XMStoreFloat4x4(&vp, VP);
	DirectX::XMFLOAT3 Cpos = camera->GetEye();

	//スカイマップ描画
	skyMap->blit(rc, vp, { Cpos.x,Cpos.y,Cpos.z,1.0f });

	//2Dスプライト描画
	{
		sprTitle->Render(rc, 50, 50, 0, 656, 188, 0, 1, 1, 1, 1);

		titleStartMenu->Render(rc, MENU::BACK_OFF,false);

		Cursor::Instance().Render(rc);
	}
}


void SceneTitle::DrawGUI()
{
}
