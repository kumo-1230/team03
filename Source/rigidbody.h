#ifndef RIGIDBODY_H_
#define RIGIDBODY_H_

#include <DirectXMath.h>

// 前方宣言
class GameObject;
class Collider;

// 物理演算を管理するリジッドボディクラス
class Rigidbody {
public:
    Rigidbody() = default;
    virtual ~Rigidbody() = default;

    // 重力を適用
    void ApplyGravity(float elapsed_time, const DirectX::XMFLOAT3& gravity);

    // 抵抗（空気抵抗・摩擦）を適用
    void ApplyDrag(float elapsed_time);

    // 衝突応答を解決（複数コライダー対応）
    void ResolveCollisions();

    // オーナーの設定・取得
    void SetOwner(GameObject* owner) { owner_ = owner; }
    GameObject* GetOwner() const { return owner_; }

    // 有効/無効の設定・取得
    bool IsEnabled() const { return is_enabled_; }
    void SetEnabled(bool enabled) { is_enabled_ = enabled; }

    // Kinematic（物理演算の影響を受けない）の設定・取得
    bool IsKinematic() const { return is_kinematic_; }
    void SetKinematic(bool kinematic) { is_kinematic_ = kinematic; }

    // 重力の使用設定・取得
    bool IsUseGravity() const { return use_gravity_; }
    void SetUseGravity(bool use_gravity) { use_gravity_ = use_gravity; }

    // 抵抗係数の設定・取得
    float GetDrag() const { return drag_; }
    void SetDrag(float drag) { drag_ = drag; }

    // 質量の設定・取得
    float GetMass() const { return mass_; }
    void SetMass(float mass) { mass_ = mass > 0.0f ? mass : 1.0f; }

    // 反発係数の設定・取得
    float GetBounciness() const { return bounciness_; }
    void SetBounciness(float bounciness) {
        bounciness_ = bounciness < 0.0f ? 0.0f : (bounciness > 1.0f ? 1.0f : bounciness);
    }

    // 摩擦係数の設定・取得
    float GetFriction() const { return friction_; }
    void SetFriction(float friction) {
        friction_ = friction < 0.0f ? 0.0f : (friction > 1.0f ? 1.0f : friction);
    }

    // 速度制限の設定・取得
    float GetMaxSpeed() const { return max_speed_; }
    void SetMaxSpeed(float max_speed) { max_speed_ = max_speed; }

    // 速度を制限内に収める
    void ClampVelocity();

private:
    // 単一コライダーペアの衝突を解決
    void ResolveCollision(const Collider* my_collider, const Collider* other_collider);

    GameObject* owner_ = nullptr;       // 所有者
    bool is_enabled_ = true;            // 有効フラグ
    bool is_kinematic_ = false;         // Kinematicフラグ
    bool use_gravity_ = true;           // 重力使用フラグ
    float drag_ = 0.0f;                 // 抵抗係数
    float mass_ = 1.0f;                 // 質量
    float bounciness_ = 0.0f;           // 反発係数（0.0 = 完全非弾性, 1.0 = 完全弾性）
    float friction_ = 0.5f;             // 摩擦係数（0.0 = 摩擦なし, 1.0 = 最大摩擦）
    float max_speed_ = 100.0f;          // 最大速度（負の値で無制限）
};

#endif  // RIGIDBODY_H_