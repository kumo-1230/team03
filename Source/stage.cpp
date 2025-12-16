#include "stage.h"

//コンストラクタ
Stage::Stage()
{
}

Stage::~Stage()
{
}

//ポジション更新
void Stage::UpdateTransform()
{
	//スケール行列制作
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

	//回転行列を制作
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);

	//位置行列を作成
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	//三つの行列を組み合わせてワールド行列を作成
	DirectX::XMMATRIX W = S * R * T;

	//計算したワールド行列を取り出す
	DirectX::XMStoreFloat4x4(&transform, W);
}

void Stage::Update(float elapsedTime, const PlayerOld* player)
{
}

//描画処理
void Stage::Render(const RenderContext& rc, ModelRenderer* renderer)
{
	//レンダラにモデルを描画してもらう
	//renderer->Render(rc, transform, model.get(), ShaderId::Lambert);

	// 新Rendererの書き方
	renderer->Draw(ShaderId::Lambert, model);

}