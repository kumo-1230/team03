#ifndef PLAYER_H_
#define PLAYER_H_

#include "game_object.h"
#include "Camera.h"
#include <memory>
#include <collider.h>
#include "rigidbody.h"
#include "System/Graphics.h"

class Player : public GameObject {
public:
    Player()
        : GameObject(),
        move_speed_(5.0f),
        camera_(nullptr),
        camera_height_(1.0f),
        mouse_sensitivity_(0.25f),
        screen_width_(static_cast<int>(Graphics::Instance().GetScreenWidth())),
        screen_height_(static_cast<int>(Graphics::Instance().GetScreenHeight())),
        window_handle_(Graphics::Instance().GetWindowHandle()),
        camera_follow_enabled_(true),
        camera_yaw_(0.0f),
        camera_pitch_(0.0f) {
    }

    Player(const char* model_filepath, const DirectX::XMFLOAT3& pos = { 0.0f, 0.0f, 0.0f })
        : GameObject(model_filepath, pos),
        move_speed_(5.0f),
        camera_(nullptr),
        camera_height_(1.0f),
        mouse_sensitivity_(0.25f),
        screen_width_(static_cast<int>(Graphics::Instance().GetScreenWidth())),
        screen_height_(static_cast<int>(Graphics::Instance().GetScreenHeight())),
        window_handle_(Graphics::Instance().GetWindowHandle()),
        camera_follow_enabled_(true),
        camera_yaw_(0.0f),
        camera_pitch_(0.0f) {
    }

    virtual ~Player() = default;

    void Update(float elapsed_time) override {
        UpdateCamera(elapsed_time);
        HandleInput(elapsed_time);
        GameObject::Update(elapsed_time);
		RenderDebugInfo();
    }

    void SetCamera(Camera* camera) {
        camera_ = camera;
    }

    Camera* GetCamera() const {
        return camera_;
    }

    void SetMoveSpeed(float speed) {
        move_speed_ = speed;
    }

    float GetMoveSpeed() const {
        return move_speed_;
    }

    void SetMouseSensitivity(float sensitivity) {
        mouse_sensitivity_ = sensitivity;
    }

    void SetScreenSize(int width, int height) {
        screen_width_ = width;
        screen_height_ = height;
    }

    void SetWindowHandle(HWND hwnd) {
        window_handle_ = hwnd;
    }

    void SetCameraHeight(float height) {
        camera_height_ = height;
    }

    void SetCameraFollowEnabled(bool enabled) {
        camera_follow_enabled_ = enabled;

        if (!enabled) {
            ShowCursor(TRUE);
        }
    }

    bool IsCameraFollowEnabled() const {
        return camera_follow_enabled_;
    }

private:
    void HandleInput(float elapsed_time) {
        DirectX::XMFLOAT3 moveDirection = { 0.0f, 0.0f, 0.0f };

        float forward_x = sinf(camera_yaw_);
        float forward_z = cosf(camera_yaw_);
        float right_x = cosf(camera_yaw_);
        float right_z = -sinf(camera_yaw_);

        if (GetAsyncKeyState('W') & 0x8000) {
            moveDirection.x += forward_x;
            moveDirection.z += forward_z;
        }
        if (GetAsyncKeyState('S') & 0x8000) {
            moveDirection.x -= forward_x;
            moveDirection.z -= forward_z;
        }
        if (GetAsyncKeyState('D') & 0x8000) {
            moveDirection.x += right_x;
            moveDirection.z += right_z;
        }
        if (GetAsyncKeyState('A') & 0x8000) {
            moveDirection.x -= right_x;
            moveDirection.z -= right_z;
        }

        DirectX::XMVECTOR moveDirVec = DirectX::XMLoadFloat3(&moveDirection);
        float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(moveDirVec));

        if (length > 0.0f) {
            moveDirVec = DirectX::XMVector3Normalize(moveDirVec);
            DirectX::XMStoreFloat3(&moveDirection, moveDirVec);

            SetVelocity(
                moveDirection.x * move_speed_,
                0.0f,
                moveDirection.z * move_speed_
            );

            float angle_y = atan2f(moveDirection.x, moveDirection.z);
            SetAngle(0.0f, angle_y, 0.0f);
        }
        else {
            SetVelocity(0.0f, 0.0f, 0.0f);
        }
    }

    void RenderDebugInfo() {
        if (!ImGui::Begin("Player Debug Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::End();
            return;
        }

        ImGui::TextColored(ImVec4(1.0f, 1.0f, 0.0f, 1.0f), "=== PLAYER DEBUG INFO ===");
        ImGui::Separator();

        // === 基本情報 ===
        if (ImGui::CollapsingHeader("Basic Info", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Active: %s", IsActive() ? "TRUE" : "FALSE");
            ImGui::Text("Active in Hierarchy: %s", IsActiveInHierarchy() ? "TRUE" : "FALSE");
            ImGui::Text("Elapsed Time: %.3f", GetElapsedTime());
        }

        // === 位置情報 ===
        if (ImGui::CollapsingHeader("Position", ImGuiTreeNodeFlags_DefaultOpen)) {
            DirectX::XMFLOAT3 local_pos = GetLocalPosition();
            ImGui::Text("Local Position:");
            ImGui::Indent();
            ImGui::Text("X: %.3f", local_pos.x);
            ImGui::Text("Y: %.3f", local_pos.y);
            ImGui::Text("Z: %.3f", local_pos.z);
            ImGui::Unindent();

            DirectX::XMFLOAT3 world_pos = GetWorldPosition();
            ImGui::Text("World Position:");
            ImGui::Indent();
            ImGui::Text("X: %.3f", world_pos.x);
            ImGui::Text("Y: %.3f", world_pos.y);
            ImGui::Text("Z: %.3f", world_pos.z);
            ImGui::Unindent();
        }

        // === 回転情報 ===
        if (ImGui::CollapsingHeader("Rotation", ImGuiTreeNodeFlags_DefaultOpen)) {
            DirectX::XMFLOAT3 angle_rad = GetAngle();
            ImGui::Text("Angle (Radians):");
            ImGui::Indent();
            ImGui::Text("X: %.3f", angle_rad.x);
            ImGui::Text("Y: %.3f", angle_rad.y);
            ImGui::Text("Z: %.3f", angle_rad.z);
            ImGui::Unindent();

            DirectX::XMFLOAT3 angle_deg = GetAngleDegree();
            ImGui::Text("Angle (Degrees):");
            ImGui::Indent();
            ImGui::Text("X: %.3f", angle_deg.x);
            ImGui::Text("Y: %.3f", angle_deg.y);
            ImGui::Text("Z: %.3f", angle_deg.z);
            ImGui::Unindent();
        }

        // === スケール情報 ===
        if (ImGui::CollapsingHeader("Scale")) {
            DirectX::XMFLOAT3 scale = GetScale();
            ImGui::Text("Scale:");
            ImGui::Indent();
            ImGui::Text("X: %.3f", scale.x);
            ImGui::Text("Y: %.3f", scale.y);
            ImGui::Text("Z: %.3f", scale.z);
            ImGui::Unindent();
        }

        // === 速度情報 ===
        if (ImGui::CollapsingHeader("Velocity", ImGuiTreeNodeFlags_DefaultOpen)) {
            DirectX::XMFLOAT3 velocity = GetVelocity();
            ImGui::Text("Velocity:");
            ImGui::Indent();
            ImGui::Text("X: %.3f", velocity.x);
            ImGui::Text("Y: %.3f", velocity.y);
            ImGui::Text("Z: %.3f", velocity.z);
            ImGui::Unindent();

            float speed = GetSpeed();
            ImGui::Text("Speed: %.3f", speed);
        }

        // === 方向ベクトル ===
        if (ImGui::CollapsingHeader("Direction Vectors")) {
            DirectX::XMFLOAT3 forward = GetForwardFloat3();
            ImGui::Text("Forward:");
            ImGui::Indent();
            ImGui::Text("X: %.3f", forward.x);
            ImGui::Text("Y: %.3f", forward.y);
            ImGui::Text("Z: %.3f", forward.z);
            ImGui::Unindent();

            DirectX::XMFLOAT3 right = GetRightFloat3();
            ImGui::Text("Right:");
            ImGui::Indent();
            ImGui::Text("X: %.3f", right.x);
            ImGui::Text("Y: %.3f", right.y);
            ImGui::Text("Z: %.3f", right.z);
            ImGui::Unindent();

            DirectX::XMFLOAT3 up = GetUpFloat3();
            ImGui::Text("Up:");
            ImGui::Indent();
            ImGui::Text("X: %.3f", up.x);
            ImGui::Text("Y: %.3f", up.y);
            ImGui::Text("Z: %.3f", up.z);
            ImGui::Unindent();
        }

        // === 親子関係 ===
        if (ImGui::CollapsingHeader("Hierarchy")) {
            GameObject* parent = GetParent();
            ImGui::Text("Parent: %s", parent ? "YES" : "NONE");

            const std::vector<GameObject*>& children = GetChildren();
            ImGui::Text("Children Count: %zu", children.size());

            if (!children.empty() && ImGui::TreeNode("Children List")) {
                for (size_t i = 0; i < children.size(); ++i) {
                    if (children[i]) {
                        DirectX::XMFLOAT3 child_pos = children[i]->GetWorldPosition();
                        ImGui::Text("[%zu] Pos(%.1f, %.1f, %.1f)",
                            i, child_pos.x, child_pos.y, child_pos.z);
                    }
                }
                ImGui::TreePop();
            }
        }

        // === Rigidbody情報 ===
        if (ImGui::CollapsingHeader("Rigidbody")) {
            Rigidbody* rb = GetRigidbody();
            if (rb) {
                ImGui::Text("Rigidbody: ATTACHED");
                ImGui::Indent();
                ImGui::Text("Enabled: %s", rb->IsEnabled() ? "TRUE" : "FALSE");
                ImGui::Text("Kinematic: %s", rb->IsKinematic() ? "TRUE" : "FALSE");
                ImGui::Text("Use Gravity: %s", rb->IsUseGravity() ? "TRUE" : "FALSE");
                ImGui::Text("Mass: %.3f", rb->GetMass());
                ImGui::Text("Bounciness: %.3f", rb->GetBounciness());
                ImGui::Text("Friction: %.3f", rb->GetFriction());
                ImGui::Text("Drag: %.3f", rb->GetDrag());
                ImGui::Text("Max Speed: %.3f", rb->GetMaxSpeed());
                ImGui::Unindent();
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Rigidbody: NONE");
            }
        }

        // === Collider情報 ===
        if (ImGui::CollapsingHeader("Colliders")) {
            const std::vector<Collider*>& colliders = GetColliders();
            ImGui::Text("Collider Count: %zu", colliders.size());

            if (!colliders.empty() && ImGui::TreeNode("Collider List")) {
                for (size_t i = 0; i < colliders.size(); ++i) {
                    if (colliders[i]) {
                        ImGui::PushID(static_cast<int>(i));

                        const char* type_name = "Unknown";
                        switch (colliders[i]->GetType()) {
                        case ColliderType::kSphere: type_name = "Sphere"; break;
                        case ColliderType::kBox: type_name = "Box"; break;
                        case ColliderType::kCapsule: type_name = "Capsule"; break;
                        }

                        if (ImGui::TreeNode("Collider", "[%zu] %s", i, type_name)) {
                            ImGui::Text("Enabled: %s", colliders[i]->IsEnabled() ? "TRUE" : "FALSE");

                            DirectX::XMFLOAT3 offset = colliders[i]->GetOffset();
                            ImGui::Text("Offset: (%.3f, %.3f, %.3f)",
                                offset.x, offset.y, offset.z);

                            DirectX::XMFLOAT3 world_center = colliders[i]->GetWorldCenter();
                            ImGui::Text("World Center: (%.3f, %.3f, %.3f)",
                                world_center.x, world_center.y, world_center.z);

                            // 型ごとの詳細情報
                            if (colliders[i]->GetType() == ColliderType::kSphere) {
                                SphereCollider* sphere = static_cast<SphereCollider*>(colliders[i]);
                                ImGui::Text("Radius: %.3f", sphere->GetRadius());
                            }
                            else if (colliders[i]->GetType() == ColliderType::kBox) {
                                BoxCollider* box = static_cast<BoxCollider*>(colliders[i]);
                                DirectX::XMFLOAT3 size = box->GetSize();
                                ImGui::Text("Size: (%.3f, %.3f, %.3f)",
                                    size.x, size.y, size.z);
                            }
                            else if (colliders[i]->GetType() == ColliderType::kCapsule) {
                                CapsuleCollider* capsule = static_cast<CapsuleCollider*>(colliders[i]);
                                ImGui::Text("Radius: %.3f", capsule->GetRadius());
                                ImGui::Text("Height: %.3f", capsule->GetHeight());
                            }

                            ImGui::TreePop();
                        }

                        ImGui::PopID();
                    }
                }
                ImGui::TreePop();
            }
        }

        // === プレイヤー固有情報 ===
        if (ImGui::CollapsingHeader("Player Specific", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Move Speed: %.3f", move_speed_);
            ImGui::Text("Mouse Sensitivity: %.3f", mouse_sensitivity_);
            ImGui::Text("Camera Height: %.3f", camera_height_);
            ImGui::Text("Camera Follow: %s", camera_follow_enabled_ ? "ENABLED" : "DISABLED");
            ImGui::Text("Camera Yaw: %.3f (%.1f deg)", camera_yaw_,
                DirectX::XMConvertToDegrees(camera_yaw_));
            ImGui::Text("Camera Pitch: %.3f (%.1f deg)", camera_pitch_,
                DirectX::XMConvertToDegrees(camera_pitch_));
            ImGui::Text("Screen Size: %d x %d", screen_width_, screen_height_);

            if (camera_) {
                ImGui::Separator();
                ImGui::Text("Camera Info:");
                ImGui::Indent();
                ImGui::Text("Eye: (%.2f, %.2f, %.2f)",
                    camera_->eye.x, camera_->eye.y, camera_->eye.z);
                ImGui::Text("Focus: (%.2f, %.2f, %.2f)",
                    camera_->focus.x, camera_->focus.y, camera_->focus.z);
                ImGui::Unindent();
            }
            else {
                ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Camera: NOT SET");
            }
        }

        // === トランスフォーム行列 ===
        if (ImGui::CollapsingHeader("Transform Matrix")) {
            DirectX::XMFLOAT4X4 local_transform = GetTransform();
            ImGui::Text("Local Transform:");
            ImGui::Indent();
            for (int row = 0; row < 4; ++row) {
                ImGui::Text("[%.3f, %.3f, %.3f, %.3f]",
                    local_transform.m[row][0],
                    local_transform.m[row][1],
                    local_transform.m[row][2],
                    local_transform.m[row][3]);
            }
            ImGui::Unindent();

            DirectX::XMFLOAT4X4 world_transform = GetWorldTransform();
            ImGui::Text("World Transform:");
            ImGui::Indent();
            for (int row = 0; row < 4; ++row) {
                ImGui::Text("[%.3f, %.3f, %.3f, %.3f]",
                    world_transform.m[row][0],
                    world_transform.m[row][1],
                    world_transform.m[row][2],
                    world_transform.m[row][3]);
            }
            ImGui::Unindent();
        }

        ImGui::End();
    }

    void UpdateCamera(float elapsed_time) {
        if (!camera_ || !window_handle_ || !camera_follow_enabled_) return;

        POINT center;
        center.x = static_cast<LONG>(screen_width_ / 2);
        center.y = static_cast<LONG>(screen_height_ / 2);

        POINT cursor;
        GetCursorPos(&cursor);
        ScreenToClient(window_handle_, &cursor);

        float ax = static_cast<float>(cursor.x - center.x);
        float ay = static_cast<float>(cursor.y - center.y);

        if (ax != 0.0f || ay != 0.0f) {
            camera_yaw_ += ax * mouse_sensitivity_ * elapsed_time;
            camera_pitch_ += ay * mouse_sensitivity_ * elapsed_time;

            const float pitch_max = DirectX::XM_PIDIV2 - 0.1f;
            const float pitch_min = -DirectX::XM_PIDIV2 + 0.1f;
            if (camera_pitch_ > pitch_max) camera_pitch_ = pitch_max;
            if (camera_pitch_ < pitch_min) camera_pitch_ = pitch_min;

            if (camera_yaw_ < -DirectX::XM_PI) {
                camera_yaw_ += DirectX::XM_2PI;
            }
            if (camera_yaw_ > DirectX::XM_PI) {
                camera_yaw_ -= DirectX::XM_2PI;
            }

            POINT screenCenter{ center.x, center.y };
            ClientToScreen(window_handle_, &screenCenter);
            SetCursorPos(screenCenter.x, screenCenter.y);
        }

        DirectX::XMFLOAT3 playerPos = GetWorldPosition();

        camera_->eye.x = playerPos.x;
        camera_->eye.y = playerPos.y + camera_height_;
        camera_->eye.z = playerPos.z;

        DirectX::XMMATRIX transform = DirectX::XMMatrixRotationRollPitchYaw(camera_pitch_, camera_yaw_, 0.0f);

        DirectX::XMVECTOR frontVec = transform.r[2];
        DirectX::XMFLOAT3 front;
        DirectX::XMStoreFloat3(&front, frontVec);

        camera_->focus.x = camera_->eye.x + front.x;
        camera_->focus.y = camera_->eye.y + front.y;
        camera_->focus.z = camera_->eye.z + front.z;

        DirectX::XMFLOAT3 up(0.0f, 1.0f, 0.0f);
        camera_->SetLookAt(camera_->eye, camera_->focus, up);

        ShowCursor(FALSE);
    }

    float move_speed_;
    Camera* camera_;
    float camera_height_;
    float mouse_sensitivity_;
    int screen_width_;
    int screen_height_;
    HWND window_handle_;
    bool camera_follow_enabled_;
    float camera_yaw_;
    float camera_pitch_;
};

#endif