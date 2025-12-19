#include "sphere_collider.h"
#include "box_collider.h"
#include "aabb_collider.h"
#include "cylinder_collider.h"
#include "game_object.h"
#include "collision_detection.h"

bool SphereCollider::CheckCollision(const Collider* other, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CollisionDetection::CheckSphereVsSphereSimple(
            this, static_cast<const SphereCollider*>(other));
        break;
    case ColliderType::kBox:
        has_collision = CollisionDetection::CheckSphereVsBoxSimple(
            this, static_cast<const BoxCollider*>(other));
        break;
    case ColliderType::kAABB:
        has_collision = CollisionDetection::CheckSphereVsAABBSimple(
            this, static_cast<const AABBCollider*>(other));
        break;
    case ColliderType::kCylinder:
        has_collision = CollisionDetection::CheckSphereVsCylinderSimple(
            this, static_cast<const CylinderCollider*>(other));
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

bool SphereCollider::CheckRigidbodyCollision(
    const Collider* other, 
    DirectX::XMFLOAT3& out_correction, 
    GameObject*& out_other) const {
    
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CollisionDetection::CheckSphereVsSphereRigidbody(
            this, static_cast<const SphereCollider*>(other), out_correction);
        break;
    case ColliderType::kBox:
        has_collision = CollisionDetection::CheckSphereVsBoxRigidbody(
            this, static_cast<const BoxCollider*>(other), out_correction);
        break;
    case ColliderType::kAABB:
        has_collision = CollisionDetection::CheckSphereVsAABBRigidbody(
            this, static_cast<const AABBCollider*>(other), out_correction);
        break;
    case ColliderType::kCylinder:
        has_collision = CollisionDetection::CheckSphereVsCylinderRigidbody(
            this, static_cast<const CylinderCollider*>(other), out_correction);
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

void SphereCollider::GetDebugDrawInfo(
    DirectX::XMFLOAT4X4& transform,
    DirectX::XMFLOAT3& size) const {

    DirectX::XMFLOAT3 world_center = GetWorldCenter();

    transform = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        world_center.x, world_center.y, world_center.z, 1
    };

    size = { radius_, radius_, radius_ };
}
