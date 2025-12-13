#ifndef PLAYER_H_
#define PLAYER_H_

#include "game_object.h"
#include "Camera.h"
#include <memory>

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