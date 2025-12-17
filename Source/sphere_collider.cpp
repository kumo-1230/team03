#include "sphere_collider.h"
#include "box_collider.h"
#include "aabb_collider.h"
#include "capsule_collider.h"
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
    case ColliderType::kAabb:
        has_collision = CollisionDetection::CheckSphereVsAabbSimple(
            this, static_cast<const AabbCollider*>(other));
        break;
    case ColliderType::kCapsule:
        has_collision = CollisionDetection::CheckSphereVsCylinderSimple(
            this, static_cast<const CapsuleCollider*>(other));
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
    case ColliderType::kAabb:
        has_collision = CollisionDetection::CheckSphereVsAabbRigidbody(
            this, static_cast<const AabbCollider*>(other), out_correction);
        break;
    case ColliderType::kCapsule:
        has_collision = CollisionDetection::CheckSphereVsCylinderRigidbody(
            this, static_cast<const CapsuleCollider*>(other), out_correction);
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
