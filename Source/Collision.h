#pragma once

#include <DirectXMath.h>

//ƒRƒŠƒWƒ‡ƒ“
class Collision
{
public:
	//‹…‚Æ‹…‚ÌŒğ·”»’è(‰~’Œ“¯m)
	static bool IntersectSphereVsSphere(
		const DirectX::XMFLOAT3& positionA,
		float radiusA,
		const DirectX::XMFLOAT3& positionB,
		float radiusB,
		DirectX::XMFLOAT3& outPositionB
	);
	//‰~’Œ“¯m‚ÌŒğ·”»’è
	static bool IntersectCylinderVsCylinder(
		const DirectX::XMFLOAT3& positionA,
		float radiusA,
		float heightA,
		const DirectX::XMFLOAT3& positionB,
		float radiusB,
		float heightB,
		DirectX::XMFLOAT3& outPositionB
	);
	//‰~’Œ“¯m‚ÌŒğ·”»’è
	static bool IntersectCylinderVsCylinder(
		const DirectX::XMFLOAT3& positionA,
		float radiusA,
		float heightA,
		const DirectX::XMFLOAT3& positionB,
		float radiusB,
		float heightB
	);

	//‹…‚Æ‰~’Œ‚ÌŒğ·”»’è
	static bool IntersectSphereVsCylinder(
		const DirectX::XMFLOAT3& sphereP,
		float sphereR,
		const DirectX::XMFLOAT3& cylinderP,
		float cylinderR,
		float cylinderH,
		DirectX::XMFLOAT3& outCylinderP
	);

	//lŠp‚ÆlŠp
	static bool IntersectBoxVsBox(
		const DirectX::XMFLOAT3& posA,
		const DirectX::XMFLOAT3& lengthA,
		const DirectX::XMFLOAT3& posB,
		const DirectX::XMFLOAT3& lengthB,
		DirectX::XMFLOAT3& outPosition
	);

	//lŠp‚Æ‰~’Œ
	static bool IntersectBoxVsCylinder(
		const DirectX::XMFLOAT3& posA,
		const DirectX::XMFLOAT3& lengthA,
		const DirectX::XMFLOAT3& cylinderP,
		float cylinderR,
		float cylinderH,
		DirectX::XMFLOAT3& outPosition
	);



	//‹…‚Æ‰~’Œ‚ÌŒğ·”»’è
	static bool IntersectSphereVsCylinder(
		const DirectX::XMFLOAT3& sphereP,
		float sphereR,
		const DirectX::XMFLOAT3& cylinderP,
		float cylinderR,
		float cylinderH
	);

	//‹…‚Æ‹…‚ÌŒğ·”»’è(‰~’Œ“¯m)
	static bool IntersectSphereVsSphere(
		const DirectX::XMFLOAT3& positionA,
		float radiusA,
		const DirectX::XMFLOAT3& positionB,
		float radiusB
	)
	{
		DirectX::XMFLOAT3 outPos{};
		return IntersectSphereVsSphere(positionA, radiusA, positionB, radiusB,outPos);
	}

	//lŠp‚ÆlŠp
	static bool IntersectBoxVsBox(
		const DirectX::XMFLOAT3& posA,
		const DirectX::XMFLOAT3& lengthA,
		const DirectX::XMFLOAT3& posB,
		const DirectX::XMFLOAT3& lengthB
	)
	{
		DirectX::XMFLOAT3 OutPos{};
		return IntersectBoxVsBox(posA, lengthA, posB, lengthB, OutPos);
	}

	//lŠp‚Æ‰~’Œ
	static bool IntersectBoxVsCylinder(
		const DirectX::XMFLOAT3& posA,
		const DirectX::XMFLOAT3& lengthA,
		const DirectX::XMFLOAT3& cylinderP,
		float cylinderR,
		float cylinderH
	)
	{
		DirectX::XMFLOAT3 OutPos{};
		return IntersectBoxVsCylinder(posA, lengthA, cylinderP, cylinderR, cylinderH, OutPos);
	}

};