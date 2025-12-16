#ifndef COLLIDER_H_
#define COLLIDER_H_

#include <DirectXMath.h>
#include <vector>

class GameObject;

enum class ColliderType {
    kSphere,
    kBox,
    kCapsule,
    kAabb  // Axis-Aligned Bounding Box
};

// ============================================================================
// Base Collider Class
// ============================================================================
class Collider {
public:
    Collider(ColliderType type) : type_(type), owner_(nullptr), enabled_(true) {}
    virtual ~Collider() = default;

    // 衝突判定メソッド
    virtual bool CheckCollision(const Collider* other, GameObject*& out_other) const = 0;
    virtual bool CheckRigidbodyCollision(const Collider* other,
        DirectX::XMFLOAT3& out_correction,
        GameObject*& out_other) const = 0;

    // デバッグ描画用情報取得
    virtual void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform,
        DirectX::XMFLOAT3& size) const = 0;

    // Getters/Setters
    ColliderType GetType() const { return type_; }
    GameObject* GetOwner() const { return owner_; }
    void SetOwner(GameObject* owner) { owner_ = owner; }

    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }

    const DirectX::XMFLOAT3& GetOffset() const { return offset_; }
    void SetOffset(const DirectX::XMFLOAT3& offset) { offset_ = offset; }
    void SetOffset(float x, float y, float z) { offset_ = { x, y, z }; }

    // ワールド座標系での中心位置を取得
    DirectX::XMFLOAT3 GetWorldCenter() const;

protected:
    ColliderType type_;
    GameObject* owner_;
    DirectX::XMFLOAT3 offset_ = { 0.0f, 0.0f, 0.0f };
    bool enabled_;
};

// ============================================================================
// Sphere Collider
// ============================================================================
class SphereCollider : public Collider {
public:
    SphereCollider(float radius = 1.0f)
        : Collider(ColliderType::kSphere), radius_(radius) {
    }

    bool CheckCollision(const Collider* other, GameObject*& out_other) const override;
    bool CheckRigidbodyCollision(const Collider* other,
        DirectX::XMFLOAT3& out_correction,
        GameObject*& out_other) const override;
    void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform,
        DirectX::XMFLOAT3& size) const override;

    float GetRadius() const { return radius_; }
    void SetRadius(float radius) { radius_ = radius; }

private:
    float radius_;
};

// ============================================================================
// Box Collider (OBB - Oriented Bounding Box)
// ============================================================================
class BoxCollider : public Collider {
public:
    BoxCollider(const DirectX::XMFLOAT3& size = { 1.0f, 1.0f, 1.0f })
        : Collider(ColliderType::kBox), size_(size) {
    }
    BoxCollider(float x, float y, float z)
        : Collider(ColliderType::kBox), size_({ x, y, z }) {
    }

    bool CheckCollision(const Collider* other, GameObject*& out_other) const override;
    bool CheckRigidbodyCollision(const Collider* other,
        DirectX::XMFLOAT3& out_correction,
        GameObject*& out_other) const override;
    void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform,
        DirectX::XMFLOAT3& size) const override;

    const DirectX::XMFLOAT3& GetSize() const { return size_; }
    void SetSize(const DirectX::XMFLOAT3& size) { size_ = size; }
    void SetSize(float x, float y, float z) { size_ = { x, y, z }; }

    // OBB用のワールド座標系頂点取得
    std::vector<DirectX::XMFLOAT3> GetWorldVertices() const;

private:
    DirectX::XMFLOAT3 size_;
};

// ============================================================================
// AABB Collider (Axis-Aligned Bounding Box)
// ============================================================================
class AABBCollider : public Collider {
public:
    AABBCollider(const DirectX::XMFLOAT3& size = { 1.0f, 1.0f, 1.0f })
        : Collider(ColliderType::kAabb), size_(size) {
    }
    AABBCollider(float x, float y, float z)
        : Collider(ColliderType::kAabb), size_({ x, y, z }) {
    }

    bool CheckCollision(const Collider* other, GameObject*& out_other) const override;
    bool CheckRigidbodyCollision(const Collider* other,
        DirectX::XMFLOAT3& out_correction,
        GameObject*& out_other) const override;
    void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform,
        DirectX::XMFLOAT3& size) const override;

    const DirectX::XMFLOAT3& GetSize() const { return size_; }
    void SetSize(const DirectX::XMFLOAT3& size) { size_ = size; }
    void SetSize(float x, float y, float z) { size_ = { x, y, z }; }

    // AABB用の最小/最大座標取得
    void GetWorldBounds(DirectX::XMFLOAT3& out_min, DirectX::XMFLOAT3& out_max) const;

private:
    DirectX::XMFLOAT3 size_;
};

// ============================================================================
// Capsule Collider (Cylinder implementation)
// ============================================================================
class CapsuleCollider : public Collider {
public:
    CapsuleCollider(float radius = 0.5f, float height = 2.0f)
        : Collider(ColliderType::kCapsule), radius_(radius), height_(height) {
    }

    bool CheckCollision(const Collider* other, GameObject*& out_other) const override;
    bool CheckRigidbodyCollision(const Collider* other,
        DirectX::XMFLOAT3& out_correction,
        GameObject*& out_other) const override;
    void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform,
        DirectX::XMFLOAT3& size) const override;

    float GetRadius() const { return radius_; }
    void SetRadius(float radius) { radius_ = radius; }

    float GetHeight() const { return height_; }
    void SetHeight(float height) { height_ = height; }

    // カプセル(円柱)の中心軸セグメント取得
    void GetCylinderSegment(DirectX::XMFLOAT3& start, DirectX::XMFLOAT3& end) const;

private:
    float radius_;
    float height_;
};

#endif  // COLLIDER_H_