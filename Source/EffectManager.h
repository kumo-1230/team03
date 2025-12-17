#pragma once

#include <DirectXMath.h>
#include <Effekseer.h>
#include <EffekseerRendererDX11.h>

//エフェクトマネージャー
class EffectManager
{
private:
	Effekseer::ManagerRef effekseerManager;
	EffekseerRenderer::RendererRef effekseerRender;

private:
	EffectManager() {}
	~EffectManager() {}
public:

	//初期化
	void Initialize();

	//終了化
	void Finalize();

public:
	//唯一インスタンス取得
	static EffectManager& Instance()
	{
		static EffectManager instance;
		return instance;
	}

	//更新処理
	void Update(float elapsedTime);

	//描画処理
	void Render(const DirectX::XMFLOAT4X4& view, const DirectX::XMFLOAT4X4& projection);

	//Effecksserマネージャー取得
	Effekseer::ManagerRef GetEffekseerManager() { return effekseerManager; }
};