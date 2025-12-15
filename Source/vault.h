#pragma once
#include "game_object.h"
#include "input_manager.h"
#include "k_lerp.h"
#include <imgui_logger.h>

class Vault : public GameObject {
public:
	Vault(const char* model) : GameObject(model) {

	}

	void Update(float elapsed_time) override {
		if (InputManager::Instance().IsKeyDown(VK_SPACE)) {
			auto * tw = TweenManager::Instance().AddTween<Vector3Tween>(
				&position_, // lerp対象変数のポインタ
				position_, // 開始値
				DirectX::XMFLOAT3{ position_.x + 1.0f, position_.y + 0.0f, position_.z + 2.0f }, // 終了値
				1.2f, // 継続時間（秒）
				EaseType::EaseInElastic // イージングタイプ
			);
			//tw->SetLoop(true);
			//tw->SetYoyo(true); // 往復設定
			tw->SetOnComplete([this]() { // 完了時コールバック
				ImGuiLogger::Instance().AddLog("Vault Tween Complete!");
				});
		}

		GameObject::Update(elapsed_time);
	}
};
