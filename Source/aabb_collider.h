#ifndef AABB_COLLIDER_H_
#define AABB_COLLIDER_H_

#include "collider.h"

// ============================================================================
// AABB Collider (Axis-Aligned Bounding Box)
// ============================================================================
class AABBCollider : public Collider {
public:
    AABBCollider(const DirectX::XMFLOAT3& size = { 1.0f, 1.0f, 1.0f }) 
        : Collider(ColliderType::kAabb), size_(size) {}
    AABBCollider(float x, float y, float z) 
        : Collider(ColliderType::kAabb), size_({ x, y, z }) {}

    bool CheckCollision(const Collider* other, GameObject*& out_other) const override;
    bool CheckRigidbodyCollision(const Collider* other, 
                                DirectX::XMFLOAT3& out_correction, 
                                GameObject*& out_other) const override;
    void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform, 
                         DirectX::XMFLOAT3& size) const override;

    const DirectX::XMFLOAT3& GetSize() const { return size_; }
    void SetSize(const DirectX::XMFLOAT3& size) { size_ = size; }
    void SetSize(float x, float y, float z) { size_ = { x, y, z }; }

    void GetWorldBounds(DirectX::XMFLOAT3& out_min, DirectX::XMFLOAT3& out_max) const;

private:
    DirectX::XMFLOAT3 size_;
};

#endif  // AABB_COLLIDER_H_
