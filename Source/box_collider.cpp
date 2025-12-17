#include "box_collider.h"
#include "sphere_collider.h"
#include "aabb_collider.h"
#include "capsule_collider.h"
#include "game_object.h"
#include "collision_detection.h"

std::vector<DirectX::XMFLOAT3> BoxCollider::GetWorldVertices() const {
    std::vector<DirectX::XMFLOAT3> vertices(8);

    if (!owner_) {
        DirectX::XMFLOAT3 half_size = { size_.x * 0.5f, size_.y * 0.5f, size_.z * 0.5f };
        vertices[0] = { -half_size.x, -half_size.y, -half_size.z };
        vertices[1] = { half_size.x, -half_size.y, -half_size.z };
        vertices[2] = { half_size.x, half_size.y, -half_size.z };
        vertices[3] = { -half_size.x, half_size.y, -half_size.z };
        vertices[4] = { -half_size.x, -half_size.y, half_size.z };
        vertices[5] = { half_size.x, -half_size.y, half_size.z };
        vertices[6] = { half_size.x, half_size.y, half_size.z };
        vertices[7] = { -half_size.x, half_size.y, half_size.z };
        return vertices;
    }

    DirectX::XMMATRIX world_transform = owner_->GetWorldTransformMatrix();
    DirectX::XMFLOAT3 half_size = { size_.x * 0.5f, size_.y * 0.5f, size_.z * 0.5f };
    DirectX::XMFLOAT3 local_vertices[8] = {
        {-half_size.x, -half_size.y, -half_size.z},
        {half_size.x, -half_size.y, -half_size.z},
        {half_size.x, half_size.y, -half_size.z},
        {-half_size.x, half_size.y, -half_size.z},
        {-half_size.x, -half_size.y, half_size.z},
        {half_size.x, -half_size.y, half_size.z},
        {half_size.x, half_size.y, half_size.z},
        {-half_size.x, half_size.y, half_size.z},
    };

    DirectX::XMVECTOR offset_vec = DirectX::XMLoadFloat3(&offset_);
    DirectX::XMMATRIX offset_transform = DirectX::XMMatrixTranslationFromVector(offset_vec);
    DirectX::XMMATRIX final_transform = offset_transform * world_transform;

    for (int i = 0; i < 8; ++i) {
        DirectX::XMVECTOR local_vec = DirectX::XMLoadFloat3(&local_vertices[i]);
        DirectX::XMVECTOR world_vec = DirectX::XMVector3TransformCoord(local_vec, final_transform);
        DirectX::XMStoreFloat3(&vertices[i], world_vec);
    }

    return vertices;
}

bool BoxCollider::CheckCollision(const Collider* other, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CollisionDetection::CheckSphereVsBoxSimple(
            static_cast<const SphereCollider*>(other), this);
        break;
    case ColliderType::kBox:
        has_collision = CollisionDetection::CheckBoxVsBoxSimple(
            this, static_cast<const BoxCollider*>(other));
        break;
    case ColliderType::kAABB:
        has_collision = CollisionDetection::CheckBoxVsAABBSimple(
            this, static_cast<const AABBCollider*>(other));
        break;
    case ColliderType::kCapsule:
        has_collision = CollisionDetection::CheckBoxVsCylinderSimple(
            this, static_cast<const CapsuleCollider*>(other));
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

bool BoxCollider::CheckRigidbodyCollision(
    const Collider* other, 
    DirectX::XMFLOAT3& out_correction, 
    GameObject*& out_other) const {
    
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CollisionDetection::CheckSphereVsBoxRigidbody(
            static_cast<const SphereCollider*>(other), this, out_correction);
        if (has_collision) {
            out_correction.x = -out_correction.x;
            out_correction.y = -out_correction.y;
            out_correction.z = -out_correction.z;
        }
        break;
    case ColliderType::kBox:
        has_collision = CollisionDetection::CheckBoxVsBoxRigidbody(
            this, static_cast<const BoxCollider*>(other), out_correction);
        break;
    case ColliderType::kAABB:
        has_collision = CollisionDetection::CheckBoxVsAABBRigidbody(
            this, static_cast<const AABBCollider*>(other), out_correction);
        break;
    case ColliderType::kCapsule:
        has_collision = CollisionDetection::CheckBoxVsCylinderRigidbody(
            this, static_cast<const CapsuleCollider*>(other), out_correction);
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

void BoxCollider::GetDebugDrawInfo(
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

    size = { size_.x * 0.5f, size_.y * 0.5f, size_.z * 0.5f };
}
