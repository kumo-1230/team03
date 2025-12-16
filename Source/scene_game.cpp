#include "System/Graphics.h"

#include "scene_game.h"
#include "common.h"
#include "System/Input.h"
#include "scene_manager.h"
#include "scene_title.h"
#include "System/ModelRenderer.h"
#include "ScoreRender.h"
#include "pause.h"
#include "CursorManager.h"
#include "mathUtils.h"
#include "Screen.h"
#include "System/Audio.h"
#include "EffectManager.h"
#include "Cursor.h"
#include <PBRShader.h>
#include <world.h>
#include <System/ResourceManager.h>
#include <Player.h>
#include <k_lerp.h>
#include <input_manager.h>
#include <vault.h>
#include "collider.h"
#include "camera.h"

SceneGame::SceneGame()
{
	int i = 0;
}

// 初期化
void SceneGame::Initialize()
{
	Graphics& graphics = Graphics::Instance();
	auto* dv = graphics.GetDevice();

	World& world = World::Instance();
	world.Clear(); // ワールドを初期化 (全シーンのFinalizeで呼ぶが、一応ここでも初期化)

	// プレイヤー初期化
	player_ = world.CreateObject<Player>();// 引数はmodelパスだが、FPSなのでモデルなし
	player_->AddCollider<BoxCollider>(DirectX::XMFLOAT3{ 1.0f, 2.0f, 1.0f });

	// カメラ初期化
	{
		camera_controller_ = world.CreateObject<CameraController>();
		camera_controller_->SetMode(CameraMode::kFirstPerson); // カメラの設定
		camera_controller_->SetTarget(player_); // カメラがプレイヤーを追従する

		player_->SetCameraController(camera_controller_); // プレイヤーが視点によって移動方向を決めるのでSetが必要
	}

	sky_map_ = std::make_unique<sky_map>(dv, L"Data/SkyMapSprite/game_background3.hdr");

	// 車オブジェクト
	obj_ = world.CreateObject<Vault>("Data/Model/mech_drone/mech_drone.glb");
	obj_->SetPosition(0, 0, 1);
	obj_->AddCollider<BoxCollider>(DirectX::XMFLOAT3{ 1.0f, 1.0f, 1.0f });

	// ロボットオブジェクト
	world.CreateObject("Data/Model/mech_drone/mech_drone2.glb", DirectX::XMFLOAT3{ 0, 0, 2 }, DirectX::XMFLOAT3{ 0, 0, 0 }, DirectX::XMFLOAT3{ 10.0f, 10.0f, 10.0f })
		->SetParent(obj_);

	//world.CreateObject("Data/Model/mech_drone/mech_drone.glb", {0, -1, 0});
	//world.CreateObject()->SetModel("Data/Model/mech_drone/mech_drone.glb"); しても同じ
	// (ResourceManager経由で読み込まれるので、同じモデルは一度しか読み込まれない)
	//obj_ = と変数に保存しておく必要はない 後にScene::Update（）で操作したいときのみ使用

	// 親子関係の使用例 (InitでSetParentするだけでだけで、毎フレーム座標追従処理は自動)
	if (0) {
		// 親オブジェクト
		auto* car = World::Instance().CreateObject("car.glb");
		car->SetPosition(10, 0, 5); // ワールド座標 (10, 0, 5)

		// 子オブジェクト（タイヤ）
		auto* wheel = World::Instance().CreateObject("wheel.glb");
		wheel->SetLocalPosition(1, 0, 1); // 車体からの相対位置
		wheel->SetParent(car, false); // 現在の座標をローカル座標として扱う

		// または、既にワールド座標にあるオブジェクトを親に設定
		auto* weapon = World::Instance().CreateObject("gun.glb");
		weapon->SetPosition(11, 1, 5); // ワールド座標
		weapon->SetParent(car, true); // ワールド座標を維持（自動的にローカル座標に変換）

		// 後からワールド座標で設定
		wheel->SetWorldPosition(15, 0, 10); // ワールド座標で指定→自動的にローカル座標に変換される

		// 親を動かすと子も連動
		car->SetPosition(20, 0, 10);
		// wheel のワールド座標は自動的に (21, 0, 11) になる
	}

	Pause::Instance().SetTutorial(false);

	// ライト設定
	{
		DirectionalLight directionalLight;
		DirectX::XMFLOAT3 dir = { 0.3f, -1.0f, 0.3f };
		DirectX::XMVECTOR Dir = DirectX::XMLoadFloat3(&dir);
		Dir = DirectX::XMVector3Normalize(Dir);
		DirectX::XMStoreFloat3(&directionalLight.direction, Dir);
		directionalLight.color = { 1.5f, 1.5f, 1.5f };
		light_manager_.SetDirectionalLight(directionalLight);

		PointLight mapLight;
		mapLight.position = { 0, 1, 0 };
		mapLight.range = 12.0f;
		mapLight.color = { 0.0f, 0.0f, 1.0f };
		mapLight.intensity = 8.0f;
		mapLight.priority = 10;
		light_manager_.AddPointLight(mapLight);

		DirectX::XMFLOAT3 playerPos = player_->GetWorldPositionFloat3();
		playerPos.y += 1.0f;
		DirectX::XMFLOAT3 spotDirection = camera_controller_->GetCamera()->GetFront();

		light_manager_.SetPlayerSpotLight(
			playerPos, spotDirection,
			20.0f, 25.0f, 40.0f,
			{ 1.0f, 0.95f, 0.85f }, 8.0f
		);
	}

	// オーディオ
	{
		bgm_ = Audio::Instance().LoadAudioSource("Data/Sound/Game/BGM_game.wav");
		bgm_->SetVolume(0.4f);
	}
}
// 終了化
void SceneGame::Finalize()
{
	bgm_->Stop();

	EffectManager::Instance().Initialize();

	delete bgm_;

	World::Instance().Clear();
}

// 更新処理
void SceneGame::Update(float elapsed_time)
{
	if (camera_controller_) {
		// カメラモード更新
		if (InputManager::Instance().IsKeyDown('1')) {
			camera_controller_->SetMode(CameraMode::kFirstPerson);
		}
		if (InputManager::Instance().IsKeyDown('2')) {
			camera_controller_->SetMode(CameraMode::kThirdPerson);
			camera_controller_->SetDistance(5.0f);
		}
		if (InputManager::Instance().IsKeyDown('3')) {
			camera_controller_->SetMode(CameraMode::kFree);
		}
		if (InputManager::Instance().IsKeyDown('4')) {
			camera_controller_->SetMode(CameraMode::kOrbit);
		}
	}

	if (game_limit_ < 0)
	{
		{
			SceneManager::Instance().ChangeScene(new SceneTitle());
			return;
		}
		return;
	}
	Pause::Instance().Update(elapsed_time);

	if (Pause::Instance().IsOnPose())
	{
		bgm_->SetVolume(0.05f);
		Cursor::Instance().Update(elapsed_time);
		return;
	}
	else
	{
		bgm_->SetVolume(0.4f);
	}

	GamePad& gamePad = Input::Instance().GetGamePad();
	static float velocity = 0.0f;

	// 自機ライティング更新
	{
		if (player_ && camera_controller_) {
			DirectX::XMFLOAT3 playerPos = player_->GetWorldPositionFloat3();
			playerPos.y += 1.0f;
			light_manager_.SetPlayerSpotLight(playerPos, camera_controller_->GetCamera()->GetFront());
		}
	}

	{
		{
			if (InputManager::Instance().IsKeyHeld(VK_LBUTTON))
			{
				game_limit_ -= 1 * elapsed_time * 0.2f;
			}
			else
				game_limit_ -= 1 * elapsed_time;
		}
		EffectManager::Instance().Update(elapsed_time);
	}

	World::Instance().Update(elapsed_time);
}

// 描画処理
void SceneGame::Render()
{
	/*if (game_limit_ < 0 || player_->GetHP() <= 0.0f)
	{
		return;
	}*/

	Graphics& graphics           = Graphics::Instance();
	ID3D11DeviceContext* dc      = graphics.GetDeviceContext();
	ShapeRenderer* shapeRenderer = graphics.GetShapeRenderer();
	ModelRenderer* modelRenderer = graphics.GetModelRenderer();
	RenderState* rs = graphics.GetRenderState();
	ID3D11Device* device = graphics.GetDevice();

	// ======= バックフェイスカリング設定ここから =======
	// バックフェイスカリングはSolidCullBackを使うので、ここでは設定しない
	//D3D11_RASTERIZER_DESC rasterDesc{};
	//rasterDesc.FillMode = D3D11_FILL_SOLID;
	//rasterDesc.CullMode = D3D11_CULL_BACK;           // 背面カリング
	//rasterDesc.FrontCounterClockwise = FALSE;        // 時計回りを表面とみなす
	//rasterDesc.DepthClipEnable = TRUE;
	//ID3D11RasterizerState* rasterState = nullptr;
	//device->CreateRasterizerState(&rasterDesc, &rasterState);
	//dc->RSSetState(rasterState);
	//rasterState->Release(); // 参照カウント減らしておく（安全）
	// ======= バックフェイスカリング設定ここまで =======

	// 描画準備
	RenderContext rc;
	rc.deviceContext = dc;
	rc.renderState = rs;
	rc.camera = camera_controller_->GetCamera();
	rc.lightManager = &light_manager_;
	

	dc->RSSetState(rs->GetRasterizerState(RasterizerState::SolidCullNone));
	dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::TestAndWrite), 0);
	ID3D11SamplerState* s = rs->GetSamplerState(SamplerState::LinearClamp);
	dc->PSSetSamplers(0, 1, &s);
	const float blendFactor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
	dc->OMSetBlendState(rs->GetBlendState(BlendState::Opaque), blendFactor, 0xffffffff);

	DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.camera->GetView());
	DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.camera->GetProjection());
	DirectX::XMMATRIX VP = V * P;
	DirectX::XMFLOAT4X4 vp;
	DirectX::XMStoreFloat4x4(&vp, VP);
	DirectX::XMFLOAT3 Cpos = camera_controller_->GetWorldPositionFloat3();

	//スカイマップ描画
	sky_map_->blit(rc, vp, { Cpos.x,Cpos.y,Cpos.z,1.0f });

	// 3Dモデル描画
	{
		// バックフェイスカリングを有効化
		dc->RSSetState(rs->GetRasterizerState(RasterizerState::SolidCullBack));
		dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::TestAndWrite), 0);
		dc->OMSetBlendState(rs->GetBlendState(BlendState::Opaque), blendFactor, 0xffffffff);

		World::Instance().Render(rc, modelRenderer);

		// エフェクトは透明オブジェクトなので このState
		dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::TestOnly), 0);
		dc->OMSetBlendState(rs->GetBlendState(BlendState::Transparency), blendFactor, 0xffffffff);
		EffectManager::Instance().Render(rc.camera->GetView(), rc.camera->GetProjection());
	}

	// 3Dデバッグ描画
#ifdef _DEBUG
	{
		dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
		dc->OMSetBlendState(rs->GetBlendState(BlendState::Transparency), blendFactor, 0xffffffff);
		World::Instance().DrawDebugPrimitives(shapeRenderer);
		light_manager_.DrawDebugSpheres(shapeRenderer);
		shapeRenderer->Render(dc, rc.camera->GetView(), rc.camera->GetProjection());
	}
#endif

	// 2Dスプライト描画
	{
		dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
		dc->OMSetBlendState(rs->GetBlendState(BlendState::Transparency), blendFactor, 0xffffffff);
		Pause::Instance().Render(dc);

	}
}


void SceneGame::DrawGUI()
{
#ifdef _DEBUG
	ImGui::Begin("GameDebug", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
		DirectX::XMFLOAT3 camPos = camera_controller_->GetCamera()->GetEye();
		ImGui::Text("Position: %.2f, %.2f, %.2f", camPos.x, camPos.y, camPos.z);

		DirectX::XMFLOAT3 camTarget = camera_controller_->GetCamera()->GetFocus();
		ImGui::Text("Target: %.2f, %.2f, %.2f", camTarget.x, camTarget.y, camTarget.z);

		DirectX::XMFLOAT3 camFront = camera_controller_->GetCamera()->GetFront();
		ImGui::Text("Front: %.2f, %.2f, %.2f", camFront.x, camFront.y, camFront.z);
	}

	if (ImGui::CollapsingHeader("Debug Draw", ImGuiTreeNodeFlags_DefaultOpen)) {
		bool draw_colliders = World::Instance().GetDebugDrawColliders();
		if (ImGui::Checkbox("Draw Colliders", &draw_colliders)) {
			World::Instance().SetDebugDrawColliders(draw_colliders);
		}
	}

	ImGui::End();

	light_manager_.DrawGUI();
#endif
}