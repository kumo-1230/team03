#include "Camera.h"
#include "mathUtils.h"

//指定方向を向く
void Camera::SetLookAt(const DirectX::XMFLOAT3& eye, const DirectX::XMFLOAT3& focus, const DirectX::XMFLOAT3& up)
{
	//視点、注視点、上方向～ビュー行列制作
	DirectX::XMVECTOR Eye   = DirectX::XMLoadFloat3(&eye);
	DirectX::XMVECTOR Focus = DirectX::XMLoadFloat3(&focus);
	DirectX::XMVECTOR Up    = DirectX::XMLoadFloat3(&up);
	DirectX::XMMATRIX View  = DirectX::XMMatrixLookAtLH(Eye, Focus, Up);
	DirectX::XMStoreFloat4x4(&view, View);

	//カメラ方向を取り出す
	this->right.x = GetView()._11;
	this->right.y = GetView()._21;
	this->right.z = GetView()._31;

	this->up.x    = GetView()._12;
	this->up.y    = GetView()._22;
	this->up.z    = GetView()._32;

	this->front.x = GetView()._13;
	this->front.y = GetView()._23;
	this->front.z = GetView()._33;

	//視点、注視点を保存
	this->eye     = eye;
	this->focus   = focus;
}

//パースペクティブ設定
void Camera::SetPerspectibeFov(float fovY, float aspect, float nearZ, float farZ)
{
	//画角,画面比率、クリップ距離からプロジェクション行列作成
	DirectX::XMMATRIX Projection = DirectX::XMMatrixPerspectiveFovLH(fovY,aspect,nearZ,farZ);

	DirectX::XMStoreFloat4x4(&projection, Projection);
}

void Camera::CameraShakeRange(float elapsedTime, float t, float Rmin, float Rmax, float range)
{
	//注視点を揺らすオフセット値を求める
	DirectX::XMFLOAT3 shake;
	shake.x = MathUtils::RandomRangeFloat(Rmin, Rmax) + t * range;
	shake.y = MathUtils::RandomRangeFloat(Rmin, Rmax) + t * range;
	shake.z = MathUtils::RandomRangeFloat(Rmin, Rmax) + t * range;

	//注視点に揺れ値を加える
	DirectX::XMFLOAT3 Focus = focus;
	Focus.x += shake.x * elapsedTime;
	Focus.y += shake.y * elapsedTime;
	Focus.z += shake.z * elapsedTime;

	SetLookAt(eye, Focus, up);
}
