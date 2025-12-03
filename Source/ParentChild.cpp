#include "ParentChild.h"

void ParentChild::MakeParentAndChild(
	const DirectX::XMFLOAT4X4& ParentTransform,
	DirectX::XMFLOAT4X4& ChildTransform)
{
	using namespace DirectX;
	XMMATRIX PW = XMLoadFloat4x4(&ParentTransform);	//親のワールド座標
	XMMATRIX CG = XMLoadFloat4x4(&ChildTransform);	//子供のグローバル座標
	XMMATRIX CW = CG * PW;							//子供のワールド座標
	XMStoreFloat4x4(&ChildTransform, CW);			//適応
}

void ParentChild::MakeParentAndChild(
	const DirectX::XMFLOAT4X4& ParentTransform,
	const DirectX::XMFLOAT3& ChildPos,
	const DirectX::XMFLOAT3& ChildScale,
	const DirectX::XMFLOAT3& ChildAngle,
	DirectX::XMFLOAT4X4& ChildTransform)
{
	using namespace DirectX;
	//武器のローカル行列を計算
	XMMATRIX S = XMMatrixScaling(ChildScale.x,ChildScale.y,ChildScale.z);
	XMMATRIX R = XMMatrixRotationRollPitchYaw(ChildAngle.x, ChildAngle.y, ChildAngle.z);
	XMMATRIX T = XMMatrixTranslation(ChildPos.x,ChildPos.y,ChildPos.z);
	// 親のワールド行列を読み込み
	XMMATRIX PW = XMLoadFloat4x4(&ParentTransform);

	// 子の最終ワールド行列（親にアタッチして離す）
	XMMATRIX CW = S * R * T * PW;

	XMStoreFloat4x4(&ChildTransform, CW);
}

void ParentChild::MakeParentAndChild(
	const DirectX::XMFLOAT4X4& ParentTransform,
	const DirectX::XMFLOAT3& ChildPos,
	const DirectX::XMFLOAT3& ChildScale,
	const DirectX::XMFLOAT4& ChildRotation,
	DirectX::XMFLOAT4X4& ChildTransform)
{
	using namespace DirectX;
	//武器のローカル行列を計算
	XMMATRIX S = XMMatrixScaling(ChildScale.x, ChildScale.y, ChildScale.z);

	DirectX::XMVECTOR Q = DirectX::XMLoadFloat4(&ChildRotation);
	Q = DirectX::XMQuaternionNormalize(Q);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(Q);
	
	XMMATRIX T = XMMatrixTranslation(ChildPos.x, ChildPos.y, ChildPos.z);
	// 親のワールド行列を読み込み
	XMMATRIX PW = XMLoadFloat4x4(&ParentTransform);

	// 子の最終ワールド行列（親にアタッチして離す）
	XMMATRIX CW = S * R * T * PW;

	XMStoreFloat4x4(&ChildTransform, CW);
}
