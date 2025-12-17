#include "Obj.h"
#include <algorithm>

Obj::Obj()
{
	Initialize();
}

Obj::~Obj()
{

}

void Obj::Initialize()
{
	UpdateTransfomEuler();
	DirectX::XMVECTOR Q = DirectX::XMQuaternionRotationRollPitchYaw(angle.x, angle.y, angle.z);
	DirectX::XMStoreFloat4(&rotation, Q);
}

void Obj::Finalize()
{
}


//行列更新処理
void Obj::UpdateTransfomEuler()
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

void Obj::UpdateTransfomQuaternion()
{
	//スケール行列制作
	DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale.x, scale.y, scale.z);

	DirectX::XMVECTOR Q = DirectX::XMLoadFloat4(&rotation);
	Q = DirectX::XMQuaternionNormalize(Q);
	DirectX::XMMATRIX R = DirectX::XMMatrixRotationQuaternion(Q);

	//位置行列を作成
	DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position.x, position.y, position.z);

	//三つの行列を組み合わせてワールド行列を作成
	DirectX::XMMATRIX W = S * R * T;

	//計算したワールド行列を取り出す
	DirectX::XMStoreFloat4x4(&transform, W);

}

//向いてほしい方向のベクトル
//オブジェクトの前方向
void Obj::SetRotation(const DirectX::XMVECTOR& Vec, const DirectX::XMFLOAT3 Forward)
{
    using namespace DirectX;

    // 正規化された方向ベクトル
    XMVECTOR V = XMVector3Normalize(Vec);
    XMVECTOR F = XMVector3Normalize(XMLoadFloat3(&Forward));

    // 回転軸と角度を求める
    XMVECTOR AXIS = XMVector3Cross(F, V);
    float dot = XMVectorGetX(XMVector3Dot(F, V));
    dot = std::clamp(dot, -1.0f, 1.0f);

    float axisLenSq = XMVectorGetX(XMVector3LengthSq(AXIS));

    XMVECTOR Q;

    if (axisLenSq < 1e-6f)
    {
        if (dot > 0.0f)
        {
            // 同じ方向なら回転なし
            Q = XMQuaternionIdentity();
        }
        else
        {
            // 反対方向ならY軸180度回転（Z軸前提）
            Q = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), XM_PI);
        }
    }
    else
    {
        float angle = acosf(dot);
        Q = XMQuaternionRotationAxis(XMVector3Normalize(AXIS), angle);
    }

    // 正規化して保存
    XMStoreFloat4(&rotation, XMQuaternionNormalize(Q));
}
//更新処理
void Obj::Update(float elapsdTime)
{
}

//描画処理
void Obj::Render(const RenderContext& rc, ModelRenderer* renderer)
{
    if (model != nullptr)
    {
        renderer->Draw(ShaderId::Lambert, model);
    }
}
