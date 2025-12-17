#include "Collision.h"
#include <algorithm>


//球同士の当たり判定
bool Collision::IntersectSphereVsSphere(
	const DirectX::XMFLOAT3& positionA,
	float radiusA,
	const DirectX::XMFLOAT3& positionB,
	float radiusB,
	DirectX::XMFLOAT3& outPositionB)
{
	{
		//A->Bの単位ベクトルを算出
		//型変換
		DirectX::XMVECTOR PositionA = DirectX::XMLoadFloat3(&positionA);
		DirectX::XMVECTOR PositionB = DirectX::XMLoadFloat3(&positionB);
		//ベクトルを求める
		DirectX::XMVECTOR Vec		= DirectX::XMVectorSubtract(PositionB, PositionA);
		//長さを求める
		DirectX::XMVECTOR LengthSq	= DirectX::XMVector3Length(Vec);
		float lengthSq;
		//アドレスを入れる
		DirectX::XMStoreFloat(&lengthSq, LengthSq);

		//距離判定
		float range = radiusA + radiusB;
		if (range < lengthSq)
		{
			return false;
		}

		//正規化
		DirectX::XMVECTOR outB = DirectX::XMVector3Normalize(Vec);
		//押し出す距離
		outB = DirectX::XMVectorScale(outB, range);
		//押し出す場所
		outB = DirectX::XMVectorAdd(PositionA, outB);
		//押し出す
		DirectX::XMStoreFloat3(&outPositionB, outB);

		return true;
	}

}

//円柱同士の当たり判定
bool Collision::IntersectCylinderVsCylinder(const DirectX::XMFLOAT3& positionA, float radiusA, float heightA, const DirectX::XMFLOAT3& positionB, float radiusB, float heightB, DirectX::XMFLOAT3& outPositionB)
{
	//Aの足元がBの頭より上なら当たっていない
	if (positionA.y > positionB.y + heightB)
	{
		return false;
	}

	//Aの頭がBの足元よりしたなら当たっていない
	if (positionA.y + heightA < positionB.y)
	{
		return false;
	}

	//XZ平面での範囲チェック
	float vx = positionB.x - positionA.x;
	float vz = positionB.z - positionA.z;
	float range = radiusA + radiusB;
	float dist = vx * vx + vz * vz;
	if (dist > range * range)
	{
		return false;
	}

	//単位ベクトル化
	dist = sqrtf(dist);
	vx /= dist;
	vz /= dist;
	float push = range - dist;

	//AがBを押し出す
	outPositionB.x = positionB.x + vx * push;
	outPositionB.y = positionB.y;
	outPositionB.z = positionB.z + vz * push;

	return true;
}

//円柱同士の当たり判定
bool Collision::IntersectCylinderVsCylinder(const DirectX::XMFLOAT3& positionA, float radiusA, float heightA, const DirectX::XMFLOAT3& positionB, float radiusB, float heightB)
{
	//Aの足元がBの頭より上なら当たっていない
	if (positionA.y > positionB.y + heightB)
	{
		return false;
	}

	//Aの頭がBの足元よりしたなら当たっていない
	if (positionA.y + heightA < positionB.y)
	{
		return false;
	}

	//XZ平面での範囲チェック
	float vx = positionB.x - positionA.x;
	float vz = positionB.z - positionA.z;
	float range = radiusA + radiusB;
	float dist = vx * vx + vz * vz;
	if (dist > range * range)
	{
		return false;
	}

	//単位ベクトル化
	dist = sqrtf(dist);
	vx /= dist;
	vz /= dist;
	float push = range - dist;

	return true;
}

//球と円柱の交差判定
bool Collision::IntersectSphereVsCylinder(const DirectX::XMFLOAT3& sphereP, float sphereR, const DirectX::XMFLOAT3& cylinderP, float cylinderR, float cylinderH, DirectX::XMFLOAT3& outCylinderP)
{
	//Aの足元がBの頭より上なら当たっていない
	if (sphereP.y > cylinderP.y + cylinderH)
	{
		return false;
	}

	//Aの頭がBの足元よりしたなら当たっていない
	if (sphereP.y + sphereR < cylinderP.y)
	{
		return false;
	}

	//XZ平面での範囲チェック
	float vx = cylinderP.x - sphereP.x;
	float vz = cylinderP.z - sphereP.z;
	float range = sphereR + cylinderR;
	float dist = vx * vx + vz * vz;
	if (dist > range * range)
	{
		return false;
	}

	//単位ベクトル化
	dist = sqrtf(dist);
	vx /= dist;
	vz /= dist;
	float push = range - dist;

	//AがBを押し出す
	outCylinderP.x = cylinderP.x + vx * push;
	outCylinderP.y = cylinderP.y;
	outCylinderP.z = cylinderP.z + vz * push;

	return true;
}

//球と円柱の当たり判定(out無し)
bool Collision::IntersectSphereVsCylinder(const DirectX::XMFLOAT3& sphereP, float sphereR, const DirectX::XMFLOAT3& cylinderP, float cylinderR, float cylinderH)
{
	//Aの足元がBの頭より上なら当たっていない
	if (sphereP.y > cylinderP.y + cylinderH)
	{
		return false;
	}

	//Aの頭がBの足元よりしたなら当たっていない
	if (sphereP.y + sphereR < cylinderP.y)
	{
		return false;
	}

	//XZ平面での範囲チェック
	float vx = cylinderP.x - sphereP.x;
	float vz = cylinderP.z - sphereP.z;
	float range = sphereR + cylinderR;
	float dist = vx * vx + vz * vz;
	if (dist > range * range)
	{
		return false;
	}

	//単位ベクトル化
	dist = sqrtf(dist);
	vx /= dist;
	vz /= dist;
	float push = range - dist;

	return true;
}

//四角と四角の当たり判定
bool Collision::IntersectBoxVsBox(
	const DirectX::XMFLOAT3& posA,
	const DirectX::XMFLOAT3& lengthA,
	const DirectX::XMFLOAT3& posB,
	const DirectX::XMFLOAT3& lengthB,
	DirectX::XMFLOAT3& outPosition)
{
	DirectX::XMFLOAT3 StartPosA;
	DirectX::XMFLOAT3 StartPosB;

	StartPosA.x = posA.x - (lengthA.x * 0.5f);
	StartPosA.z = posA.z - (lengthA.z * 0.5f);
	StartPosA.y = posA.y;

	StartPosB.x = posB.x - (lengthB.x * 0.5f);
	StartPosB.z = posB.z - (lengthB.z * 0.5f);
	StartPosB.y = posB.y;

	return !(StartPosA.x + lengthA.x < StartPosB.x || //BがAの外にあるか
		StartPosB.x + lengthB.x < StartPosA.x || //AがBの外にあるか
		StartPosA.y + lengthA.y < StartPosB.y || //BがAの外にあるか
		StartPosB.y + lengthB.y < StartPosA.y || //AがBの外にあるか
		StartPosA.z + lengthA.z < StartPosB.z || //BがAの外にあるか
		StartPosB.z + lengthB.z < StartPosA.z	  //AがBの外にあるか
		);
}

//四角と円柱の当たり判定
bool Collision::IntersectBoxVsCylinder(
	const DirectX::XMFLOAT3& posA,
	const DirectX::XMFLOAT3& lengthA,
	const DirectX::XMFLOAT3& cylinderP,
	float cylinderR,
	float cylinderH,
	DirectX::XMFLOAT3& outPosition
)
{

	if (isnan(outPosition.x))
	{
		int a = 0;
	}

	if (posA.y + lengthA.y < cylinderP.y) return false;
	if (cylinderP.y + cylinderH < posA.y) return false;

	{
		//四角の始まりから終わりの場所まで
		float boxMinX = posA.x - lengthA.x;
		float boxMaxX = posA.x + lengthA.x;
		float boxMinZ = posA.z - lengthA.z;
		float boxMaxZ = posA.z + lengthA.z;

		//円の中心から四角の最近接点を求める
		float closestX = std::max(boxMinX, std::min(cylinderP.x, boxMaxX));
		float closestZ = std::max(boxMinZ, std::min(cylinderP.z, boxMaxZ));

		//円の中心から四角の最接近点までのベクトルを取る
		float dx = cylinderP.x - closestX;
		float dz = cylinderP.z - closestZ;

		//円と四角形の距離が半径内なら衝突
		if ((dx * dx + dz * dz) <= (cylinderR * cylinderR))
		{
			float dist = { sqrtf(dx * dx + dz * dz) };
			dx /= dist;
			dz /= dist;
			float push = cylinderR - dist;

			outPosition.x = cylinderP.x + dx * push;
			outPosition.y = cylinderP.y;
			outPosition.z = cylinderP.z + dz * push;

			return true;
		}
		return false;
	}

}
