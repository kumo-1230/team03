#include "System/Input.h"
#include "CameraController.h"
#include <Windows.h>
#include "common.h"
#include "System/Graphics.h"

//更新処理
void CameraController::Update(float elapsedTime, Camera* camera, float ax, float ay)
{
	{
		//POINT center;
		//center.x = SCREEN_W * 0.5f;
		//center.y = SCREEN_H * 0.5f;

		////現在のカーソル位置を取得
		//POINT cursor;
		//GetCursorPos(&cursor);

		////クライアント座標に変換
		//ScreenToClient(Graphics::Instance().GetWindowHandle(), &cursor);

		//GamePad& gamePad = Input::Instance().GetGamePad();
		//float ax = gamePad.GetAxisRX();
		//float ay = gamePad.GetAxisRY();
		//float ax = (float)(cursor.x - center.x);
		//float ay = (float)(cursor.y - center.y);
	}
	
	//カメラ回転速度
	//float speed = ACCESS_PROPERTY_SET_GUID * elapsedTime;
	float speed = rollSpeed * elapsedTime;

	//スティックの入力値に併せてx,y軸を回転
	//X軸の回転操作
	angle.y += speed * ax;
	//X軸の回転操作
	angle.x += speed * ay;

	//X軸のカメラ回転を制限
	if (angle.x > maxAngleX)
	{
		angle.x = maxAngleX;
	}
	if (angle.x < minAngleX)
	{
		angle.x = minAngleX;
	}

	//Y軸の回転値を-3.14～3.14に収まるように
	if (angle.y < -DirectX::XM_PI)
	{
		angle.y += DirectX::XM_2PI;
	}
	if (angle.y > DirectX::XM_PI)
	{
		angle.y -= DirectX::XM_2PI;
	}

	//カメラ回転地を回転行列に変換
	DirectX::XMMATRIX Transeform = DirectX::XMMatrixRotationRollPitchYaw(angle.x, angle.y, angle.z);

	//回転行列から前方向ベクトルを取り出す
	DirectX::XMVECTOR Front = Transeform.r[2];
	DirectX::XMFLOAT3 front;
	DirectX::XMStoreFloat3(&front, Front);

	//注視点から後ろベクトル方向に一定距離離れたカメラ視点を求める
	//eye.x = target.x - front.x;
	//eye.y = target.y - front.y;
	//eye.z = target.z - front.z;
	front.x = target.x + front.x;
	front.y = target.y + front.y + 0.5f;
	front.z = target.z + front.z;

	camera->SetLookAt(eye, front, DirectX::XMFLOAT3(0, 1, 0));
}