#ifndef GAME_OBJECT_H_
#define GAME_OBJECT_H_

#include <memory>
#include <vector>
#include <DirectXMath.h>
#include "System/Model.h"
#include "System/ResourceManager.h"

// 前方宣言
class Collider;
class Rigidbody;
class ModelRenderer;
struct RenderContext;

// 階層関係のタイプ
enum class HierarchyType {
    kNone,           // 親子関係なし
    kTransformOnly,  // トランスフォームのみ継承
    kFull           // 完全な親子関係（アクティブ状態も継承）
};

// ゲームオブジェクトの基底クラス
class GameObject {
public:
    GameObject() = default;
    virtual ~GameObject();

    // 更新処理
    virtual void Update(float elapsed_time);

    // 描画処理
    virtual void Render(const RenderContext& rc, ModelRenderer* model_renderer);

    // トランスフォーム更新
    void UpdateTransform();

    // ========================================
    // 親子関係
    // ========================================
    void SetParent(GameObject* parent, bool keep_world_position = false);
    void SetParentTransformOnly(GameObject* parent, bool keep_world_position = false);
    void DetachFromParent();
    GameObject* GetParent() const { return parent_; }
    const std::vector<GameObject*>& GetChildren() const { return children_; }

    // ========================================
    // 位置・回転・スケール
    // ========================================
    // ローカル座標
    void SetLocalPosition(const DirectX::XMFLOAT3& pos);
    void SetLocalPosition(float x, float y, float z);
    void SetPosition(DirectX::FXMVECTOR v);
    const DirectX::XMFLOAT3& GetLocalPosition() const { return position_; }

    // ワールド座標
    void SetWorldPosition(const DirectX::XMFLOAT3& world_pos);
    void SetWorldPosition(float x, float y, float z);
    DirectX::XMFLOAT3 GetWorldPosition() const;

    // 回転（ラジアン）
    void SetAngle(DirectX::FXMVECTOR v);
    const DirectX::XMFLOAT3& GetAngle() const { return angle_; }

    // 回転（度数法）
    void SetAngleDegree(const DirectX::XMFLOAT3& deg);
    void SetAngleDegree(float x, float y, float z);
    DirectX::XMFLOAT3 GetAngleDegree() const;

    // スケール
    void SetScale(DirectX::FXMVECTOR v);
    void SetScale(const DirectX::XMFLOAT3& scale) { scale_ = scale; UpdateTransform(); }
    void SetScale(float x, float y, float z) { scale_ = { x, y, z }; UpdateTransform(); }
    const DirectX::XMFLOAT3& GetScale() const { return scale_; }

    // ========================================
    // 移動・回転操作
    // ========================================
    void Move(const DirectX::XMFLOAT3& offset);
    void Move(float x, float y, float z);
    void Move(DirectX::FXMVECTOR v);
    void MoveForward(float distance);
    void MoveRight(float distance);
    void MoveUp(float distance);

    void Rotate(const DirectX::XMFLOAT3& delta);
    void Rotate(float x, float y, float z);
    void RotateDegree(float x, float y, float z);
    void LookAt(const DirectX::XMFLOAT3& target);
    void LookAt(DirectX::FXMVECTOR target);

    // ========================================
    // 方向ベクトル
    // ========================================
    DirectX::XMVECTOR GetForward() const;
    DirectX::XMVECTOR GetRight() const;
    DirectX::XMVECTOR GetUp() const;
    DirectX::XMFLOAT3 GetForwardFloat3() const;
    DirectX::XMFLOAT3 GetRightFloat3() const;
    DirectX::XMFLOAT3 GetUpFloat3() const;

    // ========================================
    // 速度
    // ========================================
    void SetVelocity(DirectX::FXMVECTOR v);
    void SetVelocity(const DirectX::XMFLOAT3& vel) { velocity_ = vel; }
    void SetVelocity(float x, float y, float z) { velocity_ = { x, y, z }; }
    const DirectX::XMFLOAT3& GetVelocity() const { return velocity_; }

    void AddVelocity(const DirectX::XMFLOAT3& vel);
    void AddVelocity(float x, float y, float z);
    void AddVelocity(DirectX::FXMVECTOR v);

    void ApplyImpulse(const DirectX::XMFLOAT3& impulse);
    void ApplyImpulse(DirectX::FXMVECTOR impulse);
    void StopMovement();

    float GetSpeed() const;
    void SetSpeed(float speed);

    // ========================================
    // 距離・方向計算
    // ========================================
    float GetDistanceTo(const GameObject* other) const;
    float GetDistanceTo(const DirectX::XMFLOAT3& point) const;
    float GetDistanceSquaredTo(const GameObject* other) const;
    DirectX::XMVECTOR GetDirectionTo(const GameObject* other) const;
    DirectX::XMVECTOR GetDirectionTo(const DirectX::XMFLOAT3& point) const;
    bool IsInRange(const GameObject* other, float range) const;

    // ========================================
    // トランスフォーム
    // ========================================
    DirectX::XMFLOAT4X4 GetWorldTransform() const;
    DirectX::XMMATRIX GetWorldTransformM() const;
    void SetTransform(const DirectX::XMFLOAT4X4& transform);
    const DirectX::XMFLOAT4X4& GetTransform() const { return transform_; }

    // ========================================
    // モデル
    // ========================================
    void SetModel(const char* filepath);
    void SetModel(const std::shared_ptr<Model>& model);
    std::shared_ptr<Model> GetModel() const { return model_; }

    // ========================================
    // コライダー（複数対応）
    // ========================================
    template<typename T, typename... Args>
    T* AddCollider(Args&&... args) {
        static_assert(std::is_base_of<Collider, T>::value,
            "T must be derived from Collider");

        T* collider = new T(std::forward<Args>(args)...);
        collider->SetOwner(this);
        colliders_.push_back(collider);

        return collider;
    }

    void RemoveCollider(Collider* collider);
    void RemoveAllColliders();
    const std::vector<Collider*>& GetColliders() const { return colliders_; }
    Collider* GetCollider(size_t index = 0) const {
        return index < colliders_.size() ? colliders_[index] : nullptr;
    }
    size_t GetColliderCount() const { return colliders_.size(); }

    // 後方互換性のため
    void SetCollider(Collider* collider);

    // ========================================
    // リジッドボディ
    // ========================================
    template<typename T = Rigidbody, typename... Args>
    T* AddRigidbody(Args&&... args) {
        static_assert(std::is_base_of<Rigidbody, T>::value,
            "T must be derived from Rigidbody");

        if (rigidbody_) {
            delete rigidbody_;
        }

        T* rigidbody = new T(std::forward<Args>(args)...);
        rigidbody->SetOwner(this);
        rigidbody_ = rigidbody;

        return rigidbody;
    }

    void SetRigidbody(Rigidbody* rigidbody);
    Rigidbody* GetRigidbody() const { return rigidbody_; }
    void RemoveRigidbody();

    // ========================================
    // アクティブ状態
    // ========================================
    bool IsActive() const { return active_; }
    void SetActive(bool active) { active_ = active; }
    bool IsActiveInHierarchy() const;

    void Destroy();

    // ========================================
    // その他
    // ========================================
    float GetElapsedTime() const { return elapsed_time_; }
    HierarchyType GetHierarchyType() const { return hierarchy_type_; }

protected:
    // トランスフォーム
    DirectX::XMFLOAT3 position_ = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 angle_ = { 0.0f, 0.0f, 0.0f };
    DirectX::XMFLOAT3 scale_ = { 1.0f, 1.0f, 1.0f };
    DirectX::XMFLOAT4X4 transform_ = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        0, 0, 0, 1
    };

    // 物理
    DirectX::XMFLOAT3 velocity_ = { 0.0f, 0.0f, 0.0f };

    // コンポーネント
    std::vector<Collider*> colliders_;
    Rigidbody* rigidbody_ = nullptr;

    // モデル
    std::shared_ptr<Model> model_;

    // 階層構造
    GameObject* parent_ = nullptr;
    std::vector<GameObject*> children_;
    HierarchyType hierarchy_type_ = HierarchyType::kNone;

    // 状態
    bool active_ = true;
    float elapsed_time_ = 0.0f;
};

#endif  // GAME_OBJECT_H_