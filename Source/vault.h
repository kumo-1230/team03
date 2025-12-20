#pragma once
#include "game_object.h"
#include "input_manager.h"
#include "k_lerp.h"
#include <imgui_logger.h>

class Vault : public GameObject {
public:
	//Vault(const char* model) : GameObject() {

	//}

	void Update(float elapsed_time) override 
	{
		if (InputManager::Instance().IsKeyDown('G')) {
			auto* tw = TweenManager::Instance().AddTween<Float3Tween>(
				&position_, // lerp対象変数のポインタ
				position_, // 開始値
				DirectX::XMFLOAT3{ position_.x + 1.0f, position_.y + 0.0f, position_.z + 2.0f }, // 終了値
				1.2f, // 継続時間（秒）
				EaseType::EaseInElastic // イージングタイプ
			);
			//tw->SetLoop(true);
			//tw->SetYoyo(true); 
			tw->SetOnComplete([this]() { // 完了時コールバック
				Log(u8"Vault: lerp Complete!");
				});
		}
	}

	void OnCollisionEnter(GameObject* other) override {
		Log("Vault: Collision Enter!");

		// 衝突相手の情報を取得
		if (other) {
			DirectX::XMFLOAT3 other_pos = other->GetWorldPositionFloat3();
			Log("Other object position: (%.2f, %.2f, %.2f)",
				other_pos.x, other_pos.y, other_pos.z);
			Log("This object position: (%.2f, %.2f, %.2f)",
				position_.x, position_.y, position_.z);
		}
	}

	void OnCollisionStay(GameObject* other) override {
		//Log("Vault: Collision Stay!");
	}

	void OnCollisionExit(GameObject* other) override {
		Log("Vault: Collision Exit!");
	}
};
