#include "capsule_collider.h"
#include "sphere_collider.h"
#include "box_collider.h"
#include "aabb_collider.h"
#include "game_object.h"
#include "collision_detection.h"

void CapsuleCollider::GetCylinderSegment(DirectX::XMFLOAT3& start, DirectX::XMFLOAT3& end) const {
    float half_height = height_ * 0.5f;

    if (!owner_) {
        DirectX::XMFLOAT3 center = offset_;
        start = { center.x, center.y - half_height, center.z };
        end = { center.x, center.y + half_height, center.z };
        return;
    }

    DirectX::XMVECTOR local_up = DirectX::XMVectorSet(0, half_height, 0, 0);
    DirectX::XMMATRIX world_transform = owner_->GetWorldTransformMatrix();
    DirectX::XMVECTOR offset_vec = DirectX::XMLoadFloat3(&offset_);
    DirectX::XMVECTOR world_center = DirectX::XMVector3TransformCoord(offset_vec, world_transform);
    DirectX::XMVECTOR world_up = DirectX::XMVector3TransformNormal(local_up, world_transform);
    DirectX::XMVECTOR start_vec = DirectX::XMVectorSubtract(world_center, world_up);
    DirectX::XMVECTOR end_vec = DirectX::XMVectorAdd(world_center, world_up);

    DirectX::XMStoreFloat3(&start, start_vec);
    DirectX::XMStoreFloat3(&end, end_vec);
}

bool CapsuleCollider::CheckCollision(const Collider* other, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CollisionDetection::CheckSphereVsCylinderSimple(
            static_cast<const SphereCollider*>(other), this);
        break;
    case ColliderType::kBox:
        has_collision = CollisionDetection::CheckBoxVsCylinderSimple(
            static_cast<const BoxCollider*>(other), this);
        break;
    case ColliderType::kAABB:
        has_collision = CollisionDetection::CheckAABBVsCylinderSimple(
            static_cast<const AABBCollider*>(other), this);
        break;
    case ColliderType::kCapsule:
        has_collision = CollisionDetection::CheckCylinderVsCylinderSimple(
            this, static_cast<const CapsuleCollider*>(other));
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

bool CapsuleCollider::CheckRigidbodyCollision(
    const Collider* other, 
    DirectX::XMFLOAT3& out_correction, 
    GameObject*& out_other) const {
    
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CollisionDetection::CheckSphereVsCylinderRigidbody(
            static_cast<const SphereCollider*>(other), this, out_correction);
        if (has_collision) {
            out_correction.x = -out_correction.x;
            out_correction.y = -out_correction.y;
            out_correction.z = -out_correction.z;
        }
        break;
    case ColliderType::kBox:
        has_collision = CollisionDetection::CheckBoxVsCylinderRigidbody(
            static_cast<const BoxCollider*>(other), this, out_correction);
        if (has_collision) {
            out_correction.x = -out_correction.x;
            out_correction.y = -out_correction.y;
            out_correction.z = -out_correction.z;
        }
        break;
    case ColliderType::kAABB:
        has_collision = CollisionDetection::CheckAABBVsCylinderRigidbody(
            static_cast<const AABBCollider*>(other), this, out_correction);
        if (has_collision) {
            out_correction.x = -out_correction.x;
            out_correction.y = -out_correction.y;
            out_correction.z = -out_correction.z;
        }
        break;
    case ColliderType::kCapsule:
        has_collision = CollisionDetection::CheckCylinderVsCylinderRigidbody(
            this, static_cast<const CapsuleCollider*>(other), out_correction);
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

void CapsuleCollider::GetDebugDrawInfo(
    DirectX::XMFLOAT4X4& transform,
    DirectX::XMFLOAT3& size) const {

    if (owner_) {
        transform = owner_->GetWorldTransformFloat4X4();

        if (offset_.x != 0.0f || offset_.y != 0.0f || offset_.z != 0.0f) {
            DirectX::XMMATRIX world = DirectX::XMLoadFloat4x4(&transform);
            DirectX::XMVECTOR offset_vec = DirectX::XMLoadFloat3(&offset_);
            DirectX::XMVECTOR world_offset = DirectX::XMVector3TransformCoord(
                offset_vec, world);

            DirectX::XMStoreFloat3(
                reinterpret_cast<DirectX::XMFLOAT3*>(&transform._41),
                world_offset);
        }
    }
    else {
        transform = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            offset_.x, offset_.y, offset_.z, 1
        };
    }

    size = { radius_, height_, radius_ };
}
