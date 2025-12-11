#include "System/Graphics.h"

#include "SceneGame.h"
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
#include "EffectManager.h"
#include "Cursor.h"
#include <KeyInput.h>
#include <PBRShader.h>
#include <world.h>
#include <System/ResourceManager.h>
#include <Player.h>

SceneGame::SceneGame()
{
	int i = 0;
}

// 初期化
void SceneGame::Initialize()
{
	OutputDebugStringA("=== SceneGame::Initialize START ===\n");

	World& world = World::Instance();
	world.Clear();
	OutputDebugStringA("World cleared\n");

	Graphics& graphics = Graphics::Instance();
	auto* dv = graphics.GetDevice();

	OutputDebugStringA("About to load BGM\n");
	BGM = Audio::Instance().LoadAudioSource("Data/Sound/Game/BGM_game.wav");
	BGM->SetVolume(0.4f);
	OutputDebugStringA("BGM loaded\n");

	OutputDebugStringA("Creating player\n");
	player_ = world.CreateObject<Player>();
	OutputDebugStringA("Player created\n");

	// カメラ初期化
	{
		OutputDebugStringA("Initializing camera\n");
		camera = std::make_unique<Camera>();
		DirectX::XMFLOAT3 eye = player_->GetPosition();
		DirectX::XMFLOAT3 focus{};
		focus.x = sinf(player_->GetAngle().y);
		focus.z = cosf(player_->GetAngle().y);

		float fov = 80.0f;
		camera->SetLookAt(eye, focus, DirectX::XMFLOAT3(0, 1, 0));
		camera->SetPerspectiveFov(
			DirectX::XMConvertToRadians(fov),
			graphics.GetScreenWidth() / graphics.GetScreenHeight(),
			0.1f,
			100000.0f
		);
		OutputDebugStringA("Camera initialized\n");
	}

	OutputDebugStringA("Loading skymap\n");
	skyMap = std::make_unique<sky_map>(dv, L"Data/SkyMapSprite/game_background3.hdr");
	OutputDebugStringA("Skymap loaded\n");

	player_->SetCamera(camera.get());
	OutputDebugStringA("Camera set to player\n");

	OutputDebugStringA("Loading first model\n");
	obj = world.CreateObject("Data/Model/mech_drone/mech_drone.glb");
	OutputDebugStringA("First model loaded\n");

	obj->SetPosition(0, -1, 0);
	OutputDebugStringA("First model position set\n");

<<<<<<< Updated upstream
	OutputDebugStringA("Loading second model\n");
	world.CreateObject("Data/Model/mech_drone/mech_drone2.glb", DirectX::XMFLOAT3{ 0, 0, 0 });
	OutputDebugStringA("Second model loaded\n");
=======
	//{
	//	auto aaa = world.CreateObject("Data/Model/mech_drone/aaa.glb");
	//	aaa->SetPosition(0, 1, 0);
	//}

	{
		//world.CreateObject("Data/Model/2.glb");
	}



	//world.CreateObject("Data/Model/mech_drone/mech_drone.glb", {0, -1, 0});
	//world.CreateObject()->SetModel("Data/Model/mech_drone/mech_drone.glb"); しても同じ
	// (ResourceManager経由で読み込まれるので、同じモデルは一度しか読み込まれない)
	//obj = と変数に保存しておく必要はない 後にScene::Update（）で操作したいときのみ使用

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
>>>>>>> Stashed changes

	Pose::Instance().SetTutorial(false);
	OutputDebugStringA("Pose tutorial disabled\n");

	// ライト設定
	{
		OutputDebugStringA("Setting directional light\n");
		DirectionalLight directionalLight;
		DirectX::XMFLOAT3 dir = { 0.3f, -1.0f, 0.3f };
		DirectX::XMVECTOR Dir = DirectX::XMLoadFloat3(&dir);
		Dir = DirectX::XMVector3Normalize(Dir);
		DirectX::XMStoreFloat3(&directionalLight.direction, Dir);
		directionalLight.color = { 1.5f, 1.5f, 1.5f };
		lightManager_.SetDirectionalLight(directionalLight);
		OutputDebugStringA("Directional light set\n");

		OutputDebugStringA("Adding point light\n");
		PointLight mapLight;
		mapLight.position = { 0, 1, 0 };
		mapLight.range = 12.0f;
		mapLight.color = { 1.0f, 0.1f, 0.1f };
		mapLight.intensity = 8.0f;
		mapLight.priority = 10;
		lightManager_.AddPointLight(mapLight);
		OutputDebugStringA("Point light added\n");

		OutputDebugStringA("Setting player spot light\n");
		DirectX::XMFLOAT3 playerPos = player_->GetPosition();
		playerPos.y += 1.0f;
		DirectX::XMFLOAT3 spotDirection = camera->GetFront();

		lightManager_.SetPlayerSpotLight(
			playerPos, spotDirection,
			20.0f, 25.0f, 40.0f,
			{ 1.0f, 0.95f, 0.85f }, 8.0f
		);
		OutputDebugStringA("Player spot light set\n");
	}

	OutputDebugStringA("=== SceneGame::Initialize END ===\n");
}
// 終了化
void SceneGame::Finalize()
{
	//player_->Finalize();
	BGM->Stop();

	EffectManager::Instance().Initialize();

	delete BGM;

	World::Instance().Clear();
}

// 更新処理
void SceneGame::Update(float elapsedTime)
{
	//obj->SetPosition(0, MathUtils::RandomRangeFloat(0, 2.0f), 0);

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

	World::Instance().Update(elapsedTime);

	GamePad& gamePad = Input::Instance().GetGamePad();

	static float velocity = 0.0f;

	// 自機ライティング更新
	{
		DirectX::XMFLOAT3 playerPos = player_->GetPosition();
		playerPos.y += 1.0f;
		lightManager_.SetPlayerLight(playerPos, 15.0f, { 1.0f, 0.9f, 0.8f }, 5.0f);
	}


	{

		{
			if (KeyInput::Instance().GetKeyHold(VK_LBUTTON))
			{

				gameLimit -= 1 * elapsedTime * 0.2f;
			}
			else
				gameLimit -= 1 * elapsedTime;

			//player_->Update(elapsedTime, camera.get());
		}

		//エネミー更新処理
		//enemyManager->Update(elapsedTime, player_.get(), stageManager->GetFloor());
		//starManager->Update(elapsedTime);

		EffectManager::Instance().Update(elapsedTime);
	}

}

// 描画処理
void SceneGame::Render()
{
	/*if (gameLimit < 0 || player_->GetHP() <= 0.0f)
	{
		return;
	}*/

	Graphics& graphics           = Graphics::Instance();
	ID3D11DeviceContext* dc      = graphics.GetDeviceContext();
	ShapeRenderer* shapeRenderer = graphics.GetShapeRenderer();
	ModelRenderer* modelRenderer = graphics.GetModelRenderer();
	RenderState* rs = graphics.GetRenderState();

	// ======= バックフェイスカリング設定ここから =======
	ID3D11Device* device = graphics.GetDevice();

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
	rc.camera = camera.get();
	rc.lightManager = &lightManager_;

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
	DirectX::XMFLOAT3 Cpos = camera->GetEye();

	//スカイマップ描画
	skyMap->blit(rc, vp, { Cpos.x,Cpos.y,Cpos.z,1.0f });

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
		EffectManager::Instance().Render(rc.camera->view, rc.camera->projection);
	}

	// 3Dデバッグ描画
	{
	}

	// 2Dスプライト描画
	{
		dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
		dc->OMSetBlendState(rs->GetBlendState(BlendState::Transparency), blendFactor, 0xffffffff);

	}
}

// GUI描画
//void SceneGame::DrawGUI()
//{
//#ifdef _DEBUG
//	ImGui::Begin("GameDebug");
//	ImGui::Text("camera x, y, z: %.2f, %.2f, %.2f", camera->GetEye().x, camera->GetEye().y, camera->GetEye().z);
//
//	//model pos
//	ImGui::End();
//#endif
//	//プレイヤーデバッグ描画
//	//player_->DrawDebugGUI();
////	player_->DrawDebugGUI();
////#ifdef DEBUG
////	//なんかのポジションを取ってくる
////	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
////	//表示場所
////	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Once);
////	//大きさ
////	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
////
////	if (ImGui::Begin("PlayerOld", nullptr, ImGuiWindowFlags_None))
////	{
////		//トランスフォーム
////		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
////		{
////			ImGui::InputFloat("UIAngle", &UIangle);
////
////			ImGui::InputFloat("mapX", &mapx);
////			ImGui::InputFloat("mapY", &mapy);
////
////			ImGui::InputFloat("mapOffsetX", &mapOffsetX);
////			ImGui::InputFloat("mapOffsetY", &mapOffsetY);
////
////			ImGui::InputFloat2("fiverPos", &fiverPos.x);
////		}
////	}
////	ImGui::End();
////#endif
//}

void SceneGame::DrawGUI()
{
#ifdef _DEBUG
	ImGui::Begin("GameDebug", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

	if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
		DirectX::XMFLOAT3 camPos = camera->GetEye();
		ImGui::Text("Position: %.2f, %.2f, %.2f", camPos.x, camPos.y, camPos.z);

		DirectX::XMFLOAT3 camTarget = camera->focus;
		ImGui::Text("Target: %.2f, %.2f, %.2f", camTarget.x, camTarget.y, camTarget.z);

		DirectX::XMFLOAT3 camFront = camera->GetFront();
		ImGui::Text("Front: %.2f, %.2f, %.2f", camFront.x, camFront.y, camFront.z);
	}

	ImGui::End();
#endif
}