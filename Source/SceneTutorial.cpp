#include "System/Graphics.h"

#include "SceneTutorial.h"
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
#include "SceneLoading.h"
#include "SceneGame.h"
#include "EffectManager.h"
#include "Cursor.h"


SceneTutorial::SceneTutorial()
{
	int i = 0;
}

// 初期化
void SceneTutorial::Initialize()
{
	Graphics& graphics = Graphics::Instance();
	//ステージ初期化
	//スプライト初期化

	BGMTutorial        = Audio::Instance().LoadAudioSource("Data/Sound/Tutorial/BGM_tutorial.wav");
	BGMTutorial->Play(true);

	skyMap = std::make_unique<sky_map>(graphics.GetDevice(), L"Data/SkyMapSprite/game_background3.hdr");
	player = std::make_unique<Player>();
	//カメラコントローラー初期化
	cameraController = std::make_unique<CameraController>();
	//カメラ初期化
	camera = std::make_unique <Camera>();

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

	Pose::Instance().SetTutorial(true);
}

// 終了化
void SceneTutorial::Finalize()
{
	player->Finalize();

	BGMTutorial->Stop();

	EffectManager::Instance().Initialize();

	delete BGMTutorial;
}

// 更新処理
void SceneTutorial::Update(float elapsedTime)
{
}

// 描画処理
void SceneTutorial::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
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

	// 3Dモデル描画
	{
	}

	// 3Dデバッグ描画
	{
	}

	// 2Dスプライト描画
	{
	}
}

// GUI描画
void SceneTutorial::DrawGUI()
{
}
