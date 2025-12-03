#pragma once
#include "System/ModelRenderer.h"
#include "System/ShapeRenderer.h"
#include <memory>
#include <vector>

class Player;

class Stage
{
protected:
	std::shared_ptr<Model> model;
	DirectX::XMFLOAT3	position = { 0,0,0 };
	DirectX::XMFLOAT3	angle = { 0,0,0 };
	DirectX::XMFLOAT3	scale = { 1,1,1 };
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
	//タイルモード
	int mode;
	int nowType;

	bool isGrap;

	float radius;
public:
	Stage();
	virtual ~Stage();

	//初期化
	virtual void Initialize() {};

	void setPosition(DirectX::XMFLOAT3 pos) {this->position = pos;}
	void setScale(DirectX::XMFLOAT3 sca){this->scale = sca;}
	void setAngle(DirectX::XMFLOAT3 ang) { angle = ang; }
	void setIsGrap(bool isG) { isGrap = isG; }
	bool getIsGrap() { return isGrap; }

	float GetRadius() { return radius; }
	DirectX::XMFLOAT3 GetAngle() { return angle; }

	int getType() { return nowType; }

	DirectX::XMFLOAT3& getPosition() { return position; }

	void UpdateTransform();

	virtual void RenderDebug(RenderContext& rc, ShapeRenderer* renderer) {};

	virtual void MakeUpdate(float elapsedTime,const Player* player) {};

	virtual void stopEffect() {};
protected:

public:

	//更新処理
	virtual void Update(float elapsedTime,const Player* player);

	//描画処理
	virtual void Render(const RenderContext& rc, ModelRenderer* renderer);

	virtual void Event() {};

	const DirectX::XMFLOAT4X4& GetTransform() const { return transform; }
	const Model* GetModel()const { return model.get(); }
};