#pragma once
#include <DirectXMath.h>
#include "System/ShapeRenderer.h"
#include "System/ModelRenderer.h"
#include <imgui.h>

#define DEBUG

class Obj
{
protected:
	std::unique_ptr<Model> model = nullptr;
	DirectX::XMFLOAT3	position = { 0,0,0 };
	DirectX::XMFLOAT3	angle = { 0,0,0 };
	DirectX::XMFLOAT3	scale = { 1,1,1 };
	DirectX::XMFLOAT4	rotation = {};
	DirectX::XMFLOAT4X4 transform = {
		//スケールx
		1, 0, 0, 0,
		//スケールy
		0, 1, 0, 0,
		//回転
		0, 0, 1, 0,
		//位置
		0, 0, 0, 1
	};

	DirectX::XMFLOAT4X4 parentTransform;

public:

	Obj();
	virtual ~Obj();

	//初期化
	void Initialize();

	//終了化
	void Finalize();

public:

	/////////////////////////////////////////////

	//位置取得
	const DirectX::XMFLOAT3& GetPosition() const { return position; }

	//位置設定
	void SetPosition(const DirectX::XMFLOAT3& position) { this->position = position; }

	//回転取得
	const DirectX::XMFLOAT3& GetAngle() const { return angle; }

	//回転設定
	void SetAngle(const DirectX::XMFLOAT3& angle) { this->angle = angle; }

	//スケール取得
	const DirectX::XMFLOAT3& GetScale() const { return scale; }

	//スケール取得
	void SetScale(const DirectX::XMFLOAT3& scale) { this->scale = scale; }

	const DirectX::XMFLOAT4X4& GetTransform() { return transform; }

	DirectX::XMFLOAT4X4& GetTransformAddress() { return transform; }

	void SetRotation(const DirectX::XMVECTOR& Vec, const DirectX::XMFLOAT3 Forward);

	void SetRotation(const DirectX::XMFLOAT4& r) { rotation = r; }

	const DirectX::XMFLOAT4& GetRotation() { return rotation; }

	void SetParentTransform(DirectX::XMFLOAT4X4 t) { parentTransform = t; }

	/////////////////////////////////////////////

public:
	//アップデート
	virtual void Update(float elapsedTime);

	//レンダー
	virtual void Render(const RenderContext& rc, ModelRenderer* renderer);

	//行列処理
	void UpdateTransfomEuler();

	//行列処理
	void UpdateTransfomQuaternion();

	//デバッグピリミティブ描画
	virtual void RenderDebugPrimitive(const RenderContext& rc, ShapeRenderer* renderer) {};

	//GUI描画
	virtual void DrawDebugGUI() {};

};
