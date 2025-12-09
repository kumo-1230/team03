#include "SceneLoading.h"
#include "System/Graphics.h"
#include "System/Input.h"
#include "SceneManager.h"

//ローディングスレッド
void SceneLoading::LoadingThread(SceneLoading* scene)
{
	//COM関連の初期化でスレッドマイに呼ぶ必要がある
	CoInitialize(nullptr);

	//次のシーンの初期化を行う
	scene->nextScene->Initialize();

	//スレッドが終わる前にCOM関連書終了化
	CoUninitialize();

	//次のシーンの準備完了設定
	scene->nextScene->SetReady();
}

void SceneLoading::Initialize()
{
	//スプライト初期化
	auto* device = Graphics::Instance().GetDevice();
	load_Back = std::make_unique<Sprite>(device, "Data/Sprite/load_background.png");
	load_Sprite = std::make_unique<Sprite>(device, "Data/Sprite/load_mapchip.png");

	//スレッド開始
	thread = new std::thread(LoadingThread,this);
}

void SceneLoading::Finalize()
{
	// スレッド終了待ち
	if (thread)
	{
		if (thread->joinable())
		{
			thread->join();
		}
		delete thread;
		thread = nullptr;
	}
}

void SceneLoading::Update(float elapsedTime)
{
	count += elapsedTime * 1;
	timer += elapsedTime * 1;
	if (count >= 0.2f)
	{
		sprPos++;
		count = 0;
	}
	if (sprPos >= 12.0f)
	{
		sprPos = 0;
	}
	//次のシーンの準備が完了したらシーン切り替え
	if (nextScene->IsReady()/*&&timer > 3.0f*/)
	{
		if (thread && thread->joinable())
		{
			thread->join();
		}
		SceneManager::Instance().ChangeScene(nextScene);
	}
}

void SceneLoading::Render()
{
	Graphics& graphics = Graphics::Instance();
	ID3D11DeviceContext* dc = graphics.GetDeviceContext();
	RenderState* rs = graphics.GetRenderState();
	dc->OMSetBlendState(rs->GetBlendState(BlendState::Transparency), nullptr, 0xffffffff);
	ID3D11SamplerState* s = rs->GetSamplerState(SamplerState::LinearClamp);
	dc->PSSetSamplers(0, 1, &s);
	dc->OMSetDepthStencilState(rs->GetDepthStencilState(DepthState::NoTestNoWrite), 0);
	dc->RSSetState(rs->GetRasterizerState(RasterizerState::SolidCullNone));


	//2Dスプライト描画
	{
		//画面右下にローディング描画
		load_Back->Render(dc, 0, 0, 0, 1920, 1080, 0, 1, 1, 1, 1);

		load_Sprite->Render(dc, 1520, 680, 0, 400, 400, sprPos * 400, 400,400,400, 0, 1, 1, 1, 1);
	}
}

void SceneLoading::DrawGUI()
{
}
