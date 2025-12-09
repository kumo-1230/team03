#include "Player.h"
#include "System/Input.h"
#include <imgui.h>
#include "Collision.h"
#include "EffectManager.h"
#include "System/Audio.h"
#include "stage.h"
#include "RayCast.h"
#include <random>
#include "System/Graphics.h"
#include "KeyInput.h"
#include "ParentChild.h"
#include "Screen.h"
#include "common.h"
#include "DirectXCommon.h"
#include "mathUtils.h"
#include "Pose.h"


#define DEBUG
#define ROPE_MIN (100)

Player::Player()
{
	hitEffect = new Effect("Data/Effect/Hit.efk");

	Initialize();
}

Player::~Player()
{
}

void Player::Initialize()
{
	//モデルが大きいからスケーリング
	scale.x = scale.y = scale.z = 0.01f;

	HP = 10;
	position      = { Player_StartPosX,0,0 };
}

void Player::Finalize()
{
}

//更新処理
void Player::Update(float elapsedTime, Camera* camera)
{
	//カメラ初期化
	Graphics& graphics = Graphics::Instance();

	float fov = 0;
	if (Pose::Instance().GetFov() == static_cast<int>(Pose::FOV_TYPE::NORMALE)) fov = FOV;
	if (Pose::Instance().GetFov() == static_cast<int>(Pose::FOV_TYPE::HIGH)) fov = FOV_H;

	//エフェクト更新処理
	EffectManager::Instance().Update(elapsedTime);

	UpdateTransfom();
}

//描画処理
void Player::Render(const RenderContext& rc, ModelRenderer* renderer)
{
	//エフェクト更新処理
	EffectManager::Instance().Render(rc.camera->view, rc.camera->projection);
}

//デバッグ用GUI描画
void Player::DrawDebugGUI()
{
//#ifdef DEBUG
//	//なんかのポジションを取ってくる
//	ImVec2 pos = ImGui::GetMainViewport()->GetWorkPos();
//	//表示場所
//	ImGui::SetNextWindowPos(ImVec2(pos.x, pos.y), ImGuiCond_Once);
//	//大きさ
//	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
//
//	if (ImGui::Begin("Player", nullptr, ImGuiWindowFlags_None))
//	{
//		//トランスフォーム
//		if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen))
//		{
//			//位置
//			ImGui::InputFloat3("Position", &position.x);
//
//			//回転
//			DirectX::XMFLOAT3 a;
//
//			a.x = DirectX::XMConvertToDegrees(angle.x);
//			a.y = DirectX::XMConvertToDegrees(angle.y);
//			a.z = DirectX::XMConvertToDegrees(angle.z);
//			ImGui::InputFloat3("Angle", &a.x);
//			angle.x = DirectX::XMConvertToRadians(a.x);
//			angle.y = DirectX::XMConvertToRadians(a.y);
//			angle.z = DirectX::XMConvertToRadians(a.z);
//
//			ImGui::InputInt("SCORE",&Score);
//
//			ImGui::InputFloat3("old", &oldPos.x);
//			ImGui::InputFloat3("current", &currentPos.x);
//			ImGui::InputInt("lengthBank", &lengthBank);
//			ImGui::Checkbox("isGrap", &isGrap);
//			//スケー
//
//			ImGui::InputInt("combo", &comboNum);
//			ImGui::InputFloat("comboTimer", &comboTimer);
//
//			ImGui::InputFloat("addSpeed", &addSpeed);
//
//			ImGui::InputFloat3("velocity", &velocity.x);
//
//			ImGui::InputFloat("time", &rainbowTime);
//
//		}
//	}
//	ImGui::End();
//
//#endif // DEBUG
}

void Player::RenderDebugPrimitive(const RenderContext& rc, ShapeRenderer* renderer)
{
	//基底クラスの関数呼び出し
	//Character::RenderDebugPrimitive(rc, renderer);

	//弾丸デバッグプリミティブ描画
	//projectileManager.RenderDebugPrimitive(rc, renderer);

	//renderer->RenderSphere(rc, workPos, 0.005f, DirectX::XMFLOAT4(1, 0, 0, 1));
}
