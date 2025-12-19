#ifndef CYLINDER_COLLIDER_H_
#define CYLINDER_COLLIDER_H_

#include "collider.h"

// ============================================================================
// Cylinder Collider (Cylinder implementation)
// ============================================================================
class CylinderCollider : public Collider {
public:
    CylinderCollider(float radius = 0.5f, float height = 2.0f) 
        : Collider(ColliderType::kCylinder), radius_(radius), height_(height) {}

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

    void GetCylinderSegment(DirectX::XMFLOAT3& start, DirectX::XMFLOAT3& end) const;

private:
    float radius_;
    float height_;
};

#endif  // CYLINDER_COLLIDER_H_
