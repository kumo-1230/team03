#include "EffectManager.h"
#include "System/Graphics.h"

void EffectManager::Initialize()
{
	Graphics& graphics = Graphics::Instance();

	//Effekseerレンダラ生成
	effekseerRender = EffekseerRendererDX11::Renderer::Create(graphics.GetDevice(), graphics.GetDeviceContext(), 2048);

	//Effekseerマネージャー生成
	effekseerManager = Effekseer::Manager::Create(2048);

	//Effekseerレンダラの各種設定(特別なカスタマイズをしない場合は定型に以下の設定でOK)
	effekseerManager->SetSpriteRenderer(effekseerRender->CreateSpriteRenderer());
	effekseerManager->SetRibbonRenderer(effekseerRender->CreateRibbonRenderer());
	effekseerManager->SetRingRenderer(effekseerRender->CreateRingRenderer());
	effekseerManager->SetTrackRenderer(effekseerRender->CreateTrackRenderer());
	effekseerManager->SetModelRenderer(effekseerRender->CreateModelRenderer());
	//Efeekseer内でのローダー設定(特別なカスタマイズをしない場合は以下の設定でOK)
	effekseerManager->SetTextureLoader(effekseerRender->CreateTextureLoader());
	effekseerManager->SetModelLoader(effekseerRender->CreateModelLoader());
	effekseerManager->SetMaterialLoader(effekseerRender->CreateMaterialLoader());

	//Effekseerを左手座標系で計算する
	effekseerManager->SetCoordinateSystem(Effekseer::CoordinateSystem::LH);
}

//終了化
void EffectManager::Finalize()
{
	//スマートポインタで破棄されるから何もしない
}

//更新処理
void EffectManager::Update(float elapsedTime)
{
	//エフェクト更新処理
	effekseerManager->Update(elapsedTime * 60.0f);
}

void EffectManager::Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection)
{
	//ビュー&プロジェクション行列をEffekseerレンダラに設定
	effekseerRender->SetCameraMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&view));
	effekseerRender->SetProjectionMatrix(*reinterpret_cast<const Effekseer::Matrix44*>(&projection));

	//Effekseer描画開始
	effekseerRender->BeginRendering();

	//Effekseer描画実行
	//マネージャー単位で描画するので描画順を制御する場合はマネージャーを複数作成し、
	//Dreaw()関数を実行する順序で制御できそう
	effekseerManager->Draw();

	//Effekseer描画終了
	effekseerRender->EndRendering();
}
