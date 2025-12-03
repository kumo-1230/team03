#pragma once

#include <DirectXMath.h>
#include "System/ShapeRenderer.h"

class EnemyManager;
class Stage;

//キャラクター
class Character
{
protected:

	//重力
	float gravity = -30.0f;
	DirectX::XMFLOAT3 velocity = { 0,0,0 };

	//摩擦
	float friction = 15.0f;

protected:
	DirectX::XMFLOAT3	position  = { 0,0,0 };
	DirectX::XMFLOAT3	angle     = { 0,0,0 };
	DirectX::XMFLOAT3	scale     = { 1,1,1 };
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

	//半径
	float radius = 0.5f;

	//着地判定
	bool isGround = false;

	//高さ
	float height = 2.0f;

	//HP
	float HP = 5;

	//無敵時間
	float invincidleTimer = 1.0;

	//加速度
	float acceleration = 50.0f;
	//スピード上限値
	float MaxMoveSpeed = 5.0f;
	//移動方向X
	float moveVecX = 0.0f;
	//移動方向Z
	float moveVecZ = 0.0f;
	//空中制御
	float airControl = 0.3f;
	//どれだけ落下するか？
	float downhillOffset = 43.0f;

public:
	Character() {}
	virtual ~Character() {}

public:

	/////////////////////////////////////////////

	//位置取得
	const DirectX::XMFLOAT3& GetPosition() const        { return position; }

	//位置設定
	void SetPosition(const DirectX::XMFLOAT3& position) { this->position = position; }

	//回転取得
	const DirectX::XMFLOAT3& GetAngle() const           { return angle; }

	//回転設定
	void SetAngle(const DirectX::XMFLOAT3& angle)       { this->angle = angle; }

	//スケール取得
	const DirectX::XMFLOAT3& GetScale() const           { return scale; }

	//スケール取得
	void SetScale(const DirectX::XMFLOAT3& scale)       { this->scale = scale; }

	//半径取得
	float GetRadius() const { return radius; }

	//高さ取得
	float GetHeight() const { return height; }

	//HP取得
	float GetHP() const { return HP; }
	
	/////////////////////////////////////////////

public:
	//行列処理
	void UpdateTransfom();

	//デバッグピリミティブ描画
	virtual void RenderDebugPrimitive(const RenderContext& rc, ShapeRenderer* renderer);

	//地面に接しているか
	bool IsGround() const { return isGround; };

	//ダメージを与える
	bool ApplyDamage(float dmage, EnemyManager* enemyManager);

	//ダメージを与える
	bool ApplyDamage(float dmage ,float invincidleTime, EnemyManager* enemyManager);

	//ダメージを与える
	virtual bool ApplyDamage(float dmage, float invincidleTime) { return false; };

	//吹き飛ばし
	void HitImpact(DirectX::XMFLOAT3 thisPos,DirectX::XMFLOAT3 targetPos,float power,Character* Target);

	//衝撃を与える
	void AddImpulse(const DirectX::XMFLOAT3& impulse);

protected:
	//垂直速力更新
	void UpdateVerticalVelocity(float elapsedTime);

	//垂直移動更新処理
	virtual void UpdateVerticalMove(float elapsedTime, const Stage* stage);

	//水平速力更新処理
	void UpdateHorizontalVelocity(float elapsedTime);

	//水平移動更新処理
	virtual void UpdateHorizontalMove(float elapsedTime, const Stage* stage);

	//速力処理更新
	void UpdateVelocity(float elapsedTime, const Stage* stage);

};