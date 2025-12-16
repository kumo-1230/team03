#ifndef CAMERA_CONTROLLER_H_
#define CAMERA_CONTROLLER_H_

#include "game_object.h"
#include "Camera.h"
#include "System/Graphics.h"
#include <Windows.h>

enum class CameraMode {
    kFirstPerson,
    kThirdPerson,
    kFree,
    kOrbit
};

class CameraController : public GameObject {
public:
    CameraController()
        : GameObject(),
        camera_(),
        mode_(CameraMode::kFirstPerson),
        target_(nullptr),
        mouse_sensitivity_(0.25f),
        camera_pitch_(0.0f),
        camera_yaw_(0.0f),
        offset_({ 0.0f, 1.0f, 0.0f }),
        distance_(5.0f),
        move_speed_(10.0f),
        fov_(DirectX::XMConvertToRadians(90.0f)),
        near_clip_(0.1f),
        far_clip_(10000.0f) {
        UpdateProjection();
    }

    virtual ~CameraController() = default;

    void Update(float elapsed_time) override {
        switch (mode_) {
        case CameraMode::kFirstPerson:
            UpdateFirstPerson(elapsed_time);
            break;
        case CameraMode::kThirdPerson:
            UpdateThirdPerson(elapsed_time);
            break;
        case CameraMode::kFree:
            UpdateFree(elapsed_time);
            break;
        case CameraMode::kOrbit:
            UpdateOrbit(elapsed_time);
            break;
        }
    }

    Camera* GetCamera() { return &camera_; }
    const Camera* GetCamera() const { return &camera_; }

    void SetMode(CameraMode mode) { mode_ = mode; }
    CameraMode GetMode() const { return mode_; }

    void SetTarget(GameObject* target) { target_ = target; }
    GameObject* GetTarget() const { return target_; }

    void SetFov(float fov) {
        fov_ = fov;
        UpdateProjection();
    }

    void SetClipPlanes(float near_clip, float far_clip) {
        near_clip_ = near_clip;
        far_clip_ = far_clip;
        UpdateProjection();
    }

    float GetFov() const { return fov_; }
    float GetNearClip() const { return near_clip_; }
    float GetFarClip() const { return far_clip_; }

    void SetMouseSensitivity(float sensitivity) {
        mouse_sensitivity_ = sensitivity;
    }

    void SetOffset(const DirectX::XMFLOAT3& offset) { offset_ = offset; }
    DirectX::XMFLOAT3 GetOffset() const { return offset_; }

    void SetDistance(float distance) { distance_ = distance; }
    float GetDistance() const { return distance_; }

    void SetMoveSpeed(float speed) { move_speed_ = speed; }
    float GetMoveSpeed() const { return move_speed_; }

private:
    void UpdateProjection() {
        float aspect = Graphics::Instance().GetScreenWidth() /
            Graphics::Instance().GetScreenHeight();
        camera_.SetPerspectiveFov(fov_, aspect, near_clip_, far_clip_);
    }
    void UpdateFirstPerson(float elapsed_time) {
        if (!target_) return;

        HandleMouseLook(elapsed_time);

        DirectX::XMVECTOR target_pos = target_->GetWorldPositionVector();
        DirectX::XMVECTOR offset = DirectX::XMLoadFloat3(&offset_);
        DirectX::XMVECTOR pos_vec = DirectX::XMVectorAdd(target_pos, offset);

        DirectX::XMFLOAT3 eye;
        DirectX::XMStoreFloat3(&eye, pos_vec);

        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(
            camera_pitch_, camera_yaw_, 0.0f);
        DirectX::XMVECTOR forward_vec = rotation.r[2];

        DirectX::XMVECTOR focus_vec = DirectX::XMVectorAdd(pos_vec, forward_vec);
        DirectX::XMFLOAT3 focus;
        DirectX::XMStoreFloat3(&focus, focus_vec);

        camera_.SetLookAt(eye, focus, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
        SetLocalPosition(eye);
    }

    void UpdateThirdPerson(float elapsed_time) {
        if (!target_) return;

        HandleMouseLook(elapsed_time);

        DirectX::XMVECTOR target_pos = target_->GetWorldPositionVector();
        DirectX::XMVECTOR offset = DirectX::XMLoadFloat3(&offset_);
        DirectX::XMVECTOR focus_vec = DirectX::XMVectorAdd(target_pos, offset);

        DirectX::XMFLOAT3 focus;
        DirectX::XMStoreFloat3(&focus, focus_vec);

        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(
            camera_pitch_, camera_yaw_, 0.0f);
        DirectX::XMVECTOR forward_vec = rotation.r[2];
        DirectX::XMVECTOR scaled_forward = DirectX::XMVectorScale(forward_vec, distance_);

        DirectX::XMVECTOR pos_vec = DirectX::XMVectorSubtract(focus_vec, scaled_forward);
        DirectX::XMFLOAT3 eye;
        DirectX::XMStoreFloat3(&eye, pos_vec);

        camera_.SetLookAt(eye, focus, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
        SetLocalPosition(eye);
    }

    void UpdateFree(float elapsed_time) {
        HandleMouseLook(elapsed_time);

        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(
            camera_pitch_, camera_yaw_, 0.0f);

        DirectX::XMVECTOR forward_vec = rotation.r[2];
        DirectX::XMVECTOR right_vec = rotation.r[0];

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
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            move_vec = DirectX::XMVectorAdd(move_vec, DirectX::g_XMIdentityR1);
        }
        if (GetAsyncKeyState(VK_SHIFT) & 0x8000) {
            move_vec = DirectX::XMVectorSubtract(move_vec, DirectX::g_XMIdentityR1);
        }

        DirectX::XMVECTOR length_vec = DirectX::XMVector3Length(move_vec);
        float length = DirectX::XMVectorGetX(length_vec);

        if (length > 0.0f) {
            move_vec = DirectX::XMVector3Normalize(move_vec);
            move_vec = DirectX::XMVectorScale(move_vec, move_speed_ * elapsed_time);

            DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&position_);
            pos_vec = DirectX::XMVectorAdd(pos_vec, move_vec);
            DirectX::XMStoreFloat3(&position_, pos_vec);
        }

        DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&position_);
        DirectX::XMVECTOR focus_vec = DirectX::XMVectorAdd(pos_vec, forward_vec);

        DirectX::XMFLOAT3 eye, focus;
        DirectX::XMStoreFloat3(&eye, pos_vec);
        DirectX::XMStoreFloat3(&focus, focus_vec);

        camera_.SetLookAt(eye, focus, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
    }

    void UpdateOrbit(float elapsed_time) {
        if (!target_) return;

        HandleMouseLook(elapsed_time);

        DirectX::XMFLOAT3 focus = target_->GetWorldPositionFloat3();

        if (GetAsyncKeyState(VK_MBUTTON) & 0x8000) {
            float scroll = 0.0f;
            if (GetAsyncKeyState(VK_UP) & 0x8000) scroll = 1.0f;
            if (GetAsyncKeyState(VK_DOWN) & 0x8000) scroll = -1.0f;
            distance_ -= scroll * elapsed_time * 10.0f;
            distance_ = (distance_ < 1.0f) ? 1.0f :
                (distance_ > 100.0f) ? 100.0f : distance_;
        }

        DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationRollPitchYaw(
            camera_pitch_, camera_yaw_, 0.0f);
        DirectX::XMVECTOR forward_vec = rotation.r[2];
        DirectX::XMVECTOR scaled_forward = DirectX::XMVectorScale(forward_vec, distance_);

        DirectX::XMVECTOR focus_vec = DirectX::XMLoadFloat3(&focus);
        DirectX::XMVECTOR pos_vec = DirectX::XMVectorSubtract(focus_vec, scaled_forward);

        DirectX::XMFLOAT3 eye;
        DirectX::XMStoreFloat3(&eye, pos_vec);

        camera_.SetLookAt(eye, focus, DirectX::XMFLOAT3(0.0f, 1.0f, 0.0f));
        SetLocalPosition(eye);
    }

    void HandleMouseLook(float elapsed_time) {
        HWND hwnd = Graphics::Instance().GetWindowHandle();
        if (!hwnd) return;

        int screen_w = static_cast<int>(Graphics::Instance().GetScreenWidth());
        int screen_h = static_cast<int>(Graphics::Instance().GetScreenHeight());

        LONG center_x = screen_w >> 1;
        LONG center_y = screen_h >> 1;

        POINT cursor;
        GetCursorPos(&cursor);
        ScreenToClient(hwnd, &cursor);

        float ax = static_cast<float>(cursor.x - center_x);
        float ay = static_cast<float>(cursor.y - center_y);

        if (ax != 0.0f || ay != 0.0f) {
            camera_yaw_ += ax * mouse_sensitivity_ * elapsed_time;
            camera_pitch_ += ay * mouse_sensitivity_ * elapsed_time;

            const float pitch_max = DirectX::XM_PIDIV2 - 0.1f;
            const float pitch_min = -DirectX::XM_PIDIV2 + 0.1f;
            camera_pitch_ = (camera_pitch_ > pitch_max) ? pitch_max :
                (camera_pitch_ < pitch_min) ? pitch_min : camera_pitch_;

            if (camera_yaw_ < -DirectX::XM_PI) camera_yaw_ += DirectX::XM_2PI;
            else if (camera_yaw_ > DirectX::XM_PI) camera_yaw_ -= DirectX::XM_2PI;

            POINT screen_center{ center_x, center_y };
            ClientToScreen(hwnd, &screen_center);
            SetCursorPos(screen_center.x, screen_center.y);
        }
    }

    Camera camera_;

    CameraMode mode_;
    GameObject* target_;

    float mouse_sensitivity_;

    float camera_pitch_;
    float camera_yaw_;

    DirectX::XMFLOAT3 offset_;
    float distance_;
    float move_speed_;

    float fov_;
    float near_clip_;
    float far_clip_;
};

#endif