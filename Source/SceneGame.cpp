#include "System/Graphics.h"

#include "SceneGame.h"
#include "Player.h"
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
#include "EffectManager.h"
#include "Cursor.h"
#include <KeyInput.h>

SceneGame::SceneGame()
{
	int i = 0;
}

// 初期化
void SceneGame::Initialize()
{
	Graphics& graphics = Graphics::Instance();
	//ステージ初期化
	//スプライト初期化

	BGM					= Audio::Instance().LoadAudioSource("Data/Sound/Game/BGM_game.wav");

	BGM->SetVolume(0.4f);

	skyMap				= std::make_unique<sky_map>(graphics.GetDevice(),L"Data/SkyMapSprite/game_background3.hdr");
	player				= std::make_unique<Player>();
	//カメラコントローラー初期化
	cameraController	= std::make_unique<CameraController>();
	//カメラ初期化
	camera				= std::make_unique <Camera>();

	DirectX::XMFLOAT3 eye = player->GetPosition();
	DirectX::XMFLOAT3 focus{};
	focus.x = sinf(player->GetAngle().y);
	focus.z = cosf(player->GetAngle().y);

	float fov = 80.0f;
	camera->SetLookAt(
		eye,
		focus,
		DirectX::XMFLOAT3(0, 1, 0)
	);
	camera->SetPerspectibeFov(
		DirectX::XMConvertToRadians(fov),
		graphics.GetScreenWidth() / graphics.GetScreenHeight(),
		0.1f,
		100000.0f
	);

	Pose::Instance().SetTutorial(false);
}

// 終了化
void SceneGame::Finalize()
{
	player->Finalize();
	BGM->Stop();

	EffectManager::Instance().Initialize();

	delete BGM;
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	if (gameLimit < 0)
	{
		{
			SceneManager::Instance().ChangeScene(new SceneTitle());
			return;
		}
		return;
	}
	Pose::Instance().Update(elapsedTime);

	if (Pose::Instance().GetPose())
	{
		BGM->SetVolume(0.05f);
		Cursor::Instance().Update(elapsedTime);
		return;
	}
	else
	{
		BGM->SetVolume(0.4f);
	}

	GamePad& gamePad = Input::Instance().GetGamePad();

	static float velocity = 0.0f;

	{
		//カメラコントローラー更新処理
		//DirectX::XMFLOAT3 target = player->GetPosition();

		DirectX::XMFLOAT3 PPos = player->GetPosition();
		cameraController->SetEye(PPos);
		cameraController->SetTarget(PPos);

		POINT center;
		center.x = static_cast<LONG>(SCREEN_W * 0.5f);
		center.y = static_cast<LONG>(SCREEN_H * 0.5f);

		//現在のカーソル位置を取得
		POINT cursor;
		GetCursorPos(&cursor);

		//クライアント座標に変換
		ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

		float sensitivity = 0.0f;
		if (Pose::Instance().GetSenitivity() == static_cast<int>(Pose::SENSITIVITY_TYPE::LOW)) sensitivity = 0.1f;
		if (Pose::Instance().GetSenitivity() == static_cast<int>(Pose::SENSITIVITY_TYPE::NORMALE)) sensitivity = 0.25f;
		if (Pose::Instance().GetSenitivity() == static_cast<int>(Pose::SENSITIVITY_TYPE::HIGH)) sensitivity = 0.5f;

		float ax = (float)(cursor.x - center.x) * sensitivity;
		float ay = (float)(cursor.y - center.y) * sensitivity;

		//cameraController->Updeate(elapsedTime, camera.get(), ax, ay);

		{
			cameraController->Updeate(elapsedTime, camera.get(), ax, ay);
		}

		{
			CursorManager::ChangeCursorShow(false);
		}
		// カーソルを中央に戻す
		{
			POINT screenCenter{ (LONG)(SCREEN_W / 2), (LONG)(SCREEN_H / 2) };
			ClientToScreen(Graphics::Instance().GetWindowHandle(), &screenCenter);
			SetCursorPos(screenCenter.x, screenCenter.y);
		}

		{
			if (KeyInput::Instance().GetKeyHold(VK_LBUTTON))
			{

				gameLimit -= 1 * elapsedTime * 0.2f;
			}
			else
				gameLimit -= 1 * elapsedTime;

			player->Update(elapsedTime, camera.get());
		}

		//エネミー更新処理
		//enemyManager->Update(elapsedTime, player.get(), stageManager->GetFloor());
		//starManager->Update(elapsedTime);

		EffectManager::Instance().Update(elapsedTime);
	}
}

// 描画処理
void SceneGame::Render()
{
	/*if (gameLimit < 0 || player->GetHP() <= 0.0f)
	{
		return;
	}*/

	Graphics& graphics           = Graphics::Instance();
	ID3D11DeviceContext* dc      = graphics.GetDeviceContext();
	ShapeRenderer* shapeRenderer = graphics.GetShapeRenderer();
	ModelRenderer* modelRenderer = graphics.GetModelRenderer();

	// ======= バックフェイスカリング設定ここから =======
	ID3D11Device* device = graphics.GetDevice();

	D3D11_RASTERIZER_DESC rasterDesc{};
	rasterDesc.FillMode = D3D11_FILL_SOLID;
	rasterDesc.CullMode = D3D11_CULL_BACK;           // 背面カリング
	rasterDesc.FrontCounterClockwise = FALSE;        // 時計回りを表面とみなす
	rasterDesc.DepthClipEnable = TRUE;

	ID3D11RasterizerState* rasterState = nullptr;
	device->CreateRasterizerState(&rasterDesc, &rasterState);
	dc->RSSetState(rasterState);
	rasterState->Release(); // 参照カウント減らしておく（安全）
	// ======= バックフェイスカリング設定ここまで =======


	// 描画準備
	RenderContext rc;
	rc.deviceContext  = dc;
	rc.lightDirection = { 0.0f, -1.0f, 0.0f };	// ライト方向（下方向）
	rc.renderState    = graphics.GetRenderState();

	//カメラパラメータ設定
	rc.view           = camera->GetView();
	rc.projection     = camera->GetProjection();

	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.view);
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.projection);
	DirectX::XMMATRIX VP = V * P;
	DirectX::XMFLOAT4X4 vp;
	DirectX::XMStoreFloat4x4(&vp, VP);
	DirectX::XMFLOAT3 Cpos = camera->GetEye();

	//スカイマップ描画
	skyMap->blit(rc, vp, { Cpos.x,Cpos.y,Cpos.z,1.0f });

	// 3Dモデル描画
	{
		player->Render(rc, modelRenderer);

		EffectManager::Instance().Render(rc.view, rc.projection);
	}

	// 3Dデバッグ描画
	{
	}

	// 2Dスプライト描画
	{

	}
}

// GUI描画
void SceneGame::DrawGUI()
{
	//プレイヤーデバッグ描画
	//player->DrawDebugGUI();
//	player->DrawDebugGUI();
//#ifdef DEBUG
//	//なんかのポジションを取ってくる
//	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
//	//表示場所
//	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Once);
//	//大きさ
//	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
//
//	if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
//	{
//		//トランスフォーム
//		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
//		{
//			ImGui::InputFloat("UIAngle", &UIangle);
//
//			ImGui::InputFloat("mapX", &mapx);
//			ImGui::InputFloat("mapY", &mapy);
//
//			ImGui::InputFloat("mapOffsetX", &mapOffsetX);
//			ImGui::InputFloat("mapOffsetY", &mapOffsetY);
//
//			ImGui::InputFloat2("fiverPos", &fiverPos.x);
//		}
//	}
//	ImGui::End();
//#endif
}
