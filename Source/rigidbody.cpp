#include "rigidbody.h"
#include "collider.h"
#include "game_object.h"

void Rigidbody::ApplyGravity(float elapsed_time, const DirectX::XMFLOAT3& gravity) {
    if (!is_enabled_ || is_kinematic_ || !use_gravity_ || !owner_) return;

    DirectX::XMFLOAT3 velocity = owner_->GetVelocity();
    velocity.x += gravity.x * elapsed_time;
    velocity.y += gravity.y * elapsed_time;
    velocity.z += gravity.z * elapsed_time;
    owner_->SetVelocity(velocity);
}

void Rigidbody::ApplyDrag(float elapsed_time) {
    if (!is_enabled_ || is_kinematic_ || !owner_ || drag_ <= 0.0f) return;

    DirectX::XMFLOAT3 velocity = owner_->GetVelocity();
    float drag_factor = 1.0f / (1.0f + drag_ * elapsed_time);
    velocity.x *= drag_factor;
    velocity.y *= drag_factor;
    velocity.z *= drag_factor;
    owner_->SetVelocity(velocity);
}

void Rigidbody::ClampVelocity() {
    if (!owner_ || max_speed_ < 0.0f) return;

    DirectX::XMFLOAT3 velocity = owner_->GetVelocity();
    DirectX::XMVECTOR velocity_vec = DirectX::XMLoadFloat3(&velocity);
    float speed = DirectX::XMVectorGetX(DirectX::XMVector3Length(velocity_vec));

    if (speed > max_speed_) {
        DirectX::XMVECTOR normalized = DirectX::XMVector3Normalize(velocity_vec);
        DirectX::XMVECTOR clamped = DirectX::XMVectorScale(normalized, max_speed_);
        DirectX::XMStoreFloat3(&velocity, clamped);
        owner_->SetVelocity(velocity);
    }
}

void Rigidbody::ResolveCollisions() {
    if (!is_enabled_ || is_kinematic_ || !owner_) return;

    const std::vector<Collider*>& my_colliders = owner_->GetColliders();
    if (my_colliders.empty()) return;

    // World内のすべてのオブジェクトと衝突判定
    // ※この実装ではWorld::DetectCollisions()から呼ばれることを想定
    // 個別に他のオブジェクトのコライダーと判定する場合は外部から呼び出す
}

void Rigidbody::ResolveCollision(const Collider* my_collider, const Collider* other_collider) {
    if (!is_enabled_ || is_kinematic_ || !owner_ || !my_collider || !other_collider) return;

    DirectX::XMFLOAT3 correction;
    GameObject* other = nullptr;

    // 衝突判定と補正ベクトルを取得
    if (!my_collider->CheckRigidbodyCollision(other_collider, correction, other)) return;

    DirectX::XMFLOAT3 my_velocity = owner_->GetVelocity();

    // 速度の二乗を計算（静止判定）
    float my_speed_sq = my_velocity.x * my_velocity.x +
        my_velocity.y * my_velocity.y +
        my_velocity.z * my_velocity.z;

    // 位置補正を適用
    DirectX::XMFLOAT3 current_pos = owner_->GetWorldPosition();
    owner_->SetWorldPosition(
        current_pos.x + correction.x,
        current_pos.y + correction.y,
        current_pos.z + correction.z);

    // 衝突法線を計算
    DirectX::XMVECTOR correction_vec = DirectX::XMLoadFloat3(&correction);
    float correction_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(correction_vec));

    if (correction_length < 0.0001f) return;

    DirectX::XMVECTOR correction_normalized = DirectX::XMVector3Normalize(correction_vec);
    DirectX::XMFLOAT3 normal;
    DirectX::XMStoreFloat3(&normal, correction_normalized);

    // 法線方向の速度成分を計算
    float dot = my_velocity.x * normal.x +
        my_velocity.y * normal.y +
        my_velocity.z * normal.z;

    // 法線方向に進んでいる場合のみ反射
    if (dot < 0.0f) {
        // 反射速度を計算（反発係数を考慮）
        float reflection_factor = -(1.0f + bounciness_) * dot;

        my_velocity.x += normal.x * reflection_factor;
        my_velocity.y += normal.y * reflection_factor;
        my_velocity.z += normal.z * reflection_factor;

        // 摩擦を適用（接線方向の速度を減衰）
        if (friction_ > 0.0f) {
            // 接線方向の速度を計算
            DirectX::XMFLOAT3 tangent_velocity;
            tangent_velocity.x = my_velocity.x - normal.x * (my_velocity.x * normal.x +
                my_velocity.y * normal.y +
                my_velocity.z * normal.z);
            tangent_velocity.y = my_velocity.y - normal.y * (my_velocity.x * normal.x +
                my_velocity.y * normal.y +
                my_velocity.z * normal.z);
            tangent_velocity.z = my_velocity.z - normal.z * (my_velocity.x * normal.x +
                my_velocity.y * normal.y +
                my_velocity.z * normal.z);

            // 摩擦による減衰を適用
            float friction_factor = 1.0f - friction_;
            my_velocity.x = normal.x * (my_velocity.x * normal.x +
                my_velocity.y * normal.y +
                my_velocity.z * normal.z) +
                tangent_velocity.x * friction_factor;
            my_velocity.y = normal.y * (my_velocity.x * normal.x +
                my_velocity.y * normal.y +
                my_velocity.z * normal.z) +
                tangent_velocity.y * friction_factor;
            my_velocity.z = normal.z * (my_velocity.x * normal.x +
                my_velocity.y * normal.y +
                my_velocity.z * normal.z) +
                tangent_velocity.z * friction_factor;
        }

        // 速度を低速時は0にする（安定化）
        float final_speed_sq = my_velocity.x * my_velocity.x +
            my_velocity.y * my_velocity.y +
            my_velocity.z * my_velocity.z;

        if (final_speed_sq < 0.0001f) {
            my_velocity = { 0.0f, 0.0f, 0.0f };
        }

        owner_->SetVelocity(my_velocity);
    }

    // 速度制限を適用
    ClampVelocity();
}