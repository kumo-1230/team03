#include "SceneResult.h"
#include "System/Graphics.h"
#include "CursorManager.h"
#include "common.h"
#include "KeyInput.h"
#include "SceneManager.h"
#include "SceneTitle.h"
#include "SceneLoading.h"
#include "Cursor.h"
#include "System/Audio.h"

SceneResult::SceneResult()
{
	Graphics& graphics = Graphics::Instance();
	//カメラ初期化
	camera.reset(new Camera);

	DirectX::XMFLOAT3 eye{ 0,0,0 };
	DirectX::XMFLOAT3 focus{ 1,0,0 };

	camera->SetLookAt(
		eye,
		focus,
		DirectX::XMFLOAT3(0, 1, 0)
	);
	camera->SetPerspectibeFov(
		DirectX::XMConvertToRadians(CAMERA_FOV),
		graphics.GetScreenWidth() / graphics.GetScreenHeight(),
		0.1f,
		100000.0f
	);

	//カメラコントローラー初期化
	cameraController = std::make_unique<CameraController>();
	cameraController->SetEye({ 0,0,0 });
	cameraController->SetTarget({ 0,0,1 });

}

SceneResult::~SceneResult()
{
}

void SceneResult::Update(float elapsedTime)
{
	Cursor::Instance().Update(elapsedTime);

	if (KeyInput::Instance().GetKeyDown(VK_LBUTTON))
	{
		SceneManager::Instance().ChangeScene(new SceneLoading(new SceneTitle()));
	}

	cameraController->Updeate(elapsedTime, camera.get(), 0, 0);
}

void SceneResult::Render()
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
		Cursor::Instance().Render(rc);
	}

}
