#pragma once

#include <DirectXMath.h>

class ParentChild
{
public:
	static void MakeParentAndChild(
		const DirectX::XMFLOAT4X4& ParentTransform,
		DirectX::XMFLOAT4X4& ChildTransform
	);

	static void MakeParentAndChild(
		const DirectX::XMFLOAT4X4& ParentTransform,
		const DirectX::XMFLOAT3& ChildPos,
		const DirectX::XMFLOAT3& ChildScale,
		const DirectX::XMFLOAT3& ChildAngle,
		DirectX::XMFLOAT4X4& ChildTransform
	);

	static void MakeParentAndChild(
		const DirectX::XMFLOAT4X4& ParentTransform,
		const DirectX::XMFLOAT3& ChildPos,
		const DirectX::XMFLOAT3& ChildScale,
		const DirectX::XMFLOAT4& ChildRotation,
		DirectX::XMFLOAT4X4& ChildTransform
	);
};