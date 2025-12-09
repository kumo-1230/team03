#include "Character.h"
#include "RayCast.h"
#include "stage.h"

//行列更新処理
void Character::UpdateTransfom()
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

void Character::RenderDebugPrimitive(const RenderContext& rc, ShapeRenderer* renderer)
{
	////球のデバッグ
	//renderer->RenderSphere(rc, position, radius, DirectX::XMFLOAT4(0, 0, 0, 1));
	//円柱のデバッグ
	renderer->DrawCapsule(transform, radius, height, DirectX::XMFLOAT4(0, 0, 0, 1));
}


bool Character::ApplyDamage(float dmage, EnemyManager* enemyManager)
{
	//ダメージが0の場合は健康状態を変更する必要がない
	if (dmage == 0) return false;

	//死亡している場合は健康状態を変更しない
	if (HP <= 0) return false;

	//ダメージ処理
	HP -= dmage;

	if (HP == 0)
	{
	}
	else
	{
	}

	//健康状態を変更した場合はtrueを返す
	return true;
}

bool Character::ApplyDamage(float dmage, float invincidleTime, EnemyManager* enemyManager)
{
	//ダメージが0の場合は健康状態を変更する必要がない
	if (dmage == 0) return false;

	//死亡している場合は健康状態を変更しない
	if (HP <= 0) return false;

	if (invincidleTimer > 0) return false;

	invincidleTimer = invincidleTime;

	//ダメージ処理
	HP -= dmage;

	if (HP == 0)
	{
	}
	else
	{
	}

	//健康状態を変更した場合はtrueを返す
	return true;
}

//衝撃を与える
void Character::AddImpulse(const DirectX::XMFLOAT3& impulse)
{
	velocity.x += impulse.x;
	velocity.y += impulse.y;
	velocity.z += impulse.z;
}

void Character::UpdateVelocity(float elapsedTime, const Stage* stage)
{

	//垂直速力更新
	UpdateVerticalVelocity(elapsedTime);

	//水平速力更新処理
	UpdateHorizontalVelocity(elapsedTime);

	//垂直移動更新処理
	UpdateVerticalMove(elapsedTime,stage);

	//水平移動更新処理
	UpdateHorizontalMove(elapsedTime,stage);

	{
	////重力処理
	//velocity.y += gravity * elapsedTime;

	////移動処理
	//position.y += velocity.y * elapsedTime;

	////地面判定
	//if (position.y < 0.0f)
	//{
	//	position.y = 0.0f;
	//	velocity.y = 0.0f;
	//	if (isGround == false)
	//	{
	//		isGround = true;
	//		OnLanding();
	//	}
	//}
	//else
	//{
	//	isGround = false;
	//}
	}
}


//速力更新
void Character::UpdateVerticalVelocity(float elapsedTime)
{
	//重力処理
	velocity.y += gravity * elapsedTime;

}

//垂直移動更新処理
void Character::UpdateVerticalMove(float elapsedTime,const Stage* stage)
{
	//移動処理
	float MoveY = velocity.y * elapsedTime;

	position.y += MoveY;

	//地面判定
	if (position.y < 0.0f)
	{
		position.y = 0.0f;
		velocity.y = 0.0f;
		if (isGround == false)
		{
			isGround = true;
		}
	}
	else
	{
		isGround = false;
	}

}

void Character::UpdateHorizontalVelocity(float elapsedTime)
{
	//XZ平面の速力を減速する
	float length = sqrtf( velocity.x * velocity.x + velocity.z * velocity.z);
	if (length > 0.0f)
	{
		//摩擦力
		float friction = this->friction * elapsedTime;

		//っ空中にいる間は摩擦力を減らす
		if (isGround == false) friction *= airControl;

		//摩擦による横方向の減速処理
		if (length > friction)
		{
			//if (isGround)
			//{
			//	velocity.x -= friction;
			//	velocity.z -= friction;
			//}

			float vx = velocity.x / length;
			float vz = velocity.z / length;
			velocity.x = vx * (length - friction);
			velocity.z = vz * (length - friction);
		}
		//横方向の速力が摩擦力いかになったら速力を無効か
		else
		{
			velocity.x = 0.0f;
			velocity.z = 0.0f;
		}
	}

	//XZ平面の速力を加速する
	if (length <= MaxMoveSpeed)
	{
		//移動ベクトルがゼロでないなら
		float moveVecLength = sqrtf(moveVecX * moveVecX + moveVecZ * moveVecZ);
		if (moveVecLength > 0.0f)
		{
			//加速力
			float acceleration = this->acceleration * elapsedTime;

			if (isGround == false) acceleration *= airControl;

			//移動ベクトルによる加速処理
			velocity.x += (moveVecX / moveVecLength) * acceleration;
			velocity.z += (moveVecZ / moveVecLength) * acceleration;

			//最大加速度制限
			float length = sqrtf(velocity.x * velocity.x + velocity.z * velocity.z);
			if (length > MaxMoveSpeed)
			{
				velocity.x = (velocity.x / length) * MaxMoveSpeed;
				velocity.z = (velocity.z / length) * MaxMoveSpeed;
			}
		}
	}



	//移動ベクトルをリセット
	moveVecX = 0;
	moveVecZ = 0;
}

void Character::UpdateHorizontalMove(float elapsedTime, const Stage* stage)
{
	position.x += velocity.x * elapsedTime;
	position.z += velocity.z * elapsedTime;
}

void Character::HitImpact(DirectX::XMFLOAT3 thisPos, DirectX::XMFLOAT3 targetPos, float power, Character* Target)
{
	//吹き飛ばす
	DirectX::XMFLOAT3 impulse;
	DirectX::XMVECTOR ThisPos = DirectX::XMLoadFloat3(&thisPos);
	DirectX::XMVECTOR TargetPos = DirectX::XMLoadFloat3(&targetPos);
	DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(TargetPos, ThisPos);
	//単位化
	Vec = DirectX::XMVector3Normalize(Vec);
	//スラカー倍する
	DirectX::XMStoreFloat3(&impulse, Vec);
	impulse.x *= power;
	impulse.y += 0.0f;
	impulse.z *= power;

	Target->AddImpulse(impulse);

	//ヒットエフェクト
	DirectX::XMFLOAT3 e = Target->GetPosition();
	e.y += Target->GetHeight() * 0.5f;
	//hitEffect->Play(e);

	////ヒットSE再生
	//hitSE->Play(false);
}
