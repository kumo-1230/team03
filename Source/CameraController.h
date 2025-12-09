#pragma once
#include <DirectXMath.h>
#include "Camera.h"

class CameraController
{
private:
	DirectX::XMFLOAT3	eye		  = { 0,0,0 };//ポジション
	DirectX::XMFLOAT3	target    = { 0,0,0 };//注視点
	DirectX::XMFLOAT3	angle     = { 0,0,0 };//回転角度
	float				rollSpeed = DirectX::XMConvertToRadians(45);//回転速度
	float				range     = 10.0f;//距離
	float				maxAngleX = DirectX::XMConvertToRadians(89);
	float				minAngleX = DirectX::XMConvertToRadians(-89);
public:
	//更新処理
	void Update(float elapsedTime, Camera* camera, float ax, float ay);

	//ターゲット位置
	void SetTarget(const DirectX::XMFLOAT3& target) { this->target = target; }
	void SetEye(const DirectX::XMFLOAT3& eye) { this->eye = eye; }

	DirectX::XMFLOAT3 GetEye() { return eye; }
	DirectX::XMFLOAT3 GetTarget() { return target; }
};