#ifndef SPHERE_COLLIDER_H_
#define SPHERE_COLLIDER_H_

#include "collider.h"

// ============================================================================
// Sphere Collider
// ============================================================================
class SphereCollider : public Collider {
public:
    SphereCollider(float radius = 1.0f) 
        : Collider(ColliderType::kSphere), radius_(radius) {}

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

#endif  // SPHERE_COLLIDER_H_
