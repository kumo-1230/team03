#ifndef PLAYER_H_
#define PLAYER_H_

#include "game_object.h"
#include <collider.h>
#include "rigidbody.h"

class CameraController;

class Player : public GameObject {
public:
    virtual ~Player() = default;

    void Update(float elapsed_time) override {
        if (target_) {
            if (GetDistanceTo(target_) < 5.0f) {
                Log(u8"‹——£‚ª5ˆÈ“à");
            }
            else {
				Log(u8"‹——£‚ª5‚æ‚è‘å‚«‚¢");
            }
        }
        HandleInput(elapsed_time);
        RenderDebugInfo();
    }

    void SetCameraController(CameraController* camera) {
        camera_controller_ = camera;
    }

    void SetTargetObject(GameObject* target) {
        target_ = target;
	}

    CameraController* GetCameraController() const {
        return camera_controller_;
    }

    void SetMoveSpeed(float speed) {
        move_speed_ = speed;
    }

    float GetMoveSpeed() const {
        return move_speed_;
    }

private:
    void HandleInput(float elapsed_time) {
        DirectX::XMFLOAT3 move_dir = { 0.0f, 0.0f, 0.0f };

        if (camera_controller_ && 
            (camera_controller_->GetMode() == CameraMode::kFirstPerson || camera_controller_->GetMode() == CameraMode::kThirdPerson)) {
            Camera* cam = camera_controller_->GetCamera();
            DirectX::XMFLOAT3 forward = cam->GetFront();
            DirectX::XMFLOAT3 right = cam->GetRight();

            forward.y = 0.0f;
            right.y = 0.0f;

            DirectX::XMVECTOR forward_vec = DirectX::XMLoadFloat3(&forward);
            DirectX::XMVECTOR right_vec = DirectX::XMLoadFloat3(&right);
            forward_vec = DirectX::XMVector3Normalize(forward_vec);
            right_vec = DirectX::XMVector3Normalize(right_vec);

            DirectX::XMVECTOR move_vec = DirectX::XMVectorZero();

            if (GetAsyncKeyState('W') & 0x8000) {
                move_vec = DirectX::XMVectorAdd(move_vec, forward_vec);
            }
            if (GetAsyncKeyState('S') & 0x8000) {
                move_vec = DirectX::XMVectorSubtract(move_vec, forward_vec);
            }
            if (GetAsyncKeyState('D') & 0x8000) {
                move_vec = DirectX::XMVectorAdd(move_vec, right_vec);
            }
            if (GetAsyncKeyState('A') & 0x8000) {
                move_vec = DirectX::XMVectorSubtract(move_vec, right_vec);
            }

            DirectX::XMVECTOR length_vec = DirectX::XMVector3Length(move_vec);
            float length = DirectX::XMVectorGetX(length_vec);

            if (length > 0.0f) {
                move_vec = DirectX::XMVector3Normalize(move_vec);
                DirectX::XMStoreFloat3(&move_dir, move_vec);

                SetVelocity(
                    move_dir.x * move_speed_,
                    0.0f,
                    move_dir.z * move_speed_
                );

                float angle_y = atan2f(move_dir.x, move_dir.z);
                SetAngle(0.0f, angle_y, 0.0f);
            }
            else {
                SetVelocity(0.0f, 0.0f, 0.0f);
            }
        }
    }

    void RenderDebugInfo() {
        if (!ImGui::Begin("Player Debug Info", nullptr,
            ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::End();
            return;
        }

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f),
            "=== PLAYER DEBUG INFO ===");
        ImGui::Separator();

        if (ImGui::CollapsingHeader("Basic Info",
            ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Active: %s", IsActive() ? "TRUE" : "FALSE");
            ImGui::Text("Move Speed: %.3f", move_speed_);
            ImGui::Text("Camera: %s",
                camera_controller_ ? "CONNECTED" : "NOT SET");
        }

        if (ImGui::CollapsingHeader("Position",
            ImGuiTreeNodeFlags_DefaultOpen)) {
            DirectX::XMFLOAT3 world_pos = GetWorldPositionFloat3();
            ImGui::Text("World Position:");
            ImGui::Indent();
            ImGui::Text("X: %.3f", world_pos.x);
            ImGui::Text("Y: %.3f", world_pos.y);
            ImGui::Text("Z: %.3f", world_pos.z);
            ImGui::Unindent();
        }

        if (ImGui::CollapsingHeader("Velocity",
            ImGuiTreeNodeFlags_DefaultOpen)) {
            DirectX::XMFLOAT3 velocity = GetVelocityFloat3();
            ImGui::Text("Velocity:");
            ImGui::Indent();
            ImGui::Text("X: %.3f", velocity.x);
            ImGui::Text("Y: %.3f", velocity.y);
            ImGui::Text("Z: %.3f", velocity.z);
            ImGui::Unindent();
            ImGui::Text("Speed: %.3f", GetSpeed());
        }

        ImGui::End();
    }

    float move_speed_;
    CameraController* camera_controller_;
	GameObject* target_;
};

#endif