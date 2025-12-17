#include "aabb_collider.h"
#include "sphere_collider.h"
#include "box_collider.h"
#include "capsule_collider.h"
#include "game_object.h"
#include "collision_detection.h"

void AABBCollider::GetWorldBounds(DirectX::XMFLOAT3& out_min, DirectX::XMFLOAT3& out_max) const {
    DirectX::XMFLOAT3 center = GetWorldCenter();
    DirectX::XMFLOAT3 half_extents = {
        size_.x * 0.5f,
        size_.y * 0.5f,
        size_.z * 0.5f
    };

    out_min = {
        center.x - half_extents.x,
        center.y - half_extents.y,
        center.z - half_extents.z
    };

    out_max = {
        center.x + half_extents.x,
        center.y + half_extents.y,
        center.z + half_extents.z
    };
}

bool AABBCollider::CheckCollision(const Collider* other, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CollisionDetection::CheckSphereVsAABBSimple(
            static_cast<const SphereCollider*>(other), this);
        break;
    case ColliderType::kBox:
        has_collision = CollisionDetection::CheckBoxVsAABBSimple(
            static_cast<const BoxCollider*>(other), this);
        break;
    case ColliderType::kAABB:
        has_collision = CollisionDetection::CheckAABBVsAABBSimple(
            this, static_cast<const AABBCollider*>(other));
        break;
    case ColliderType::kCapsule:
        has_collision = CollisionDetection::CheckAABBVsCylinderSimple(
            this, static_cast<const CapsuleCollider*>(other));
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

bool AABBCollider::CheckRigidbodyCollision(
    const Collider* other, 
    DirectX::XMFLOAT3& out_correction, 
    GameObject*& out_other) const {
    
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CollisionDetection::CheckSphereVsAABBRigidbody(
            static_cast<const SphereCollider*>(other), this, out_correction);
        if (has_collision) {
            out_correction.x = -out_correction.x;
            out_correction.y = -out_correction.y;
            out_correction.z = -out_correction.z;
        }
        break;
    case ColliderType::kBox:
        has_collision = CollisionDetection::CheckBoxVsAABBRigidbody(
            static_cast<const BoxCollider*>(other), this, out_correction);
        if (has_collision) {
            out_correction.x = -out_correction.x;
            out_correction.y = -out_correction.y;
            out_correction.z = -out_correction.z;
        }
        break;
    case ColliderType::kAABB:
        has_collision = CollisionDetection::CheckAABBVsAABBRigidbody(
            this, static_cast<const AABBCollider*>(other), out_correction);
        break;
    case ColliderType::kCapsule:
        has_collision = CollisionDetection::CheckAABBVsCylinderRigidbody(
            this, static_cast<const CapsuleCollider*>(other), out_correction);
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

void AABBCollider::GetDebugDrawInfo(
    DirectX::XMFLOAT4X4& transform,
    DirectX::XMFLOAT3& size) const {

    DirectX::XMFLOAT3 world_center = GetWorldCenter();

    transform = {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        world_center.x, world_center.y, world_center.z, 1
    };

    size = { size_.x * 0.5f, size_.y * 0.5f, size_.z * 0.5f };
}
