#include "collider.h"
#include "game_object.h"
#include <algorithm>

DirectX::XMFLOAT3 Collider::GetWorldCenter() const {
    if (!owner_) return offset_;

    DirectX::XMMATRIX world_transform = owner_->GetWorldTransformMatrix();
    DirectX::XMVECTOR offset_vec = DirectX::XMLoadFloat3(&offset_);
    DirectX::XMVECTOR world_center = DirectX::XMVector3TransformCoord(offset_vec, world_transform);

    DirectX::XMFLOAT3 center;
    DirectX::XMStoreFloat3(&center, world_center);
    return center;
}

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

void CapsuleCollider::GetCapsuleSegment(DirectX::XMFLOAT3& start, DirectX::XMFLOAT3& end) const {
    float half_height = (height_ - radius_ * 2.0f) * 0.5f;

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

static DirectX::XMVECTOR ClosestPointOnSegment(
    DirectX::FXMVECTOR point,
    DirectX::FXMVECTOR seg_start,
    DirectX::FXMVECTOR seg_end) {
    DirectX::XMVECTOR ab = DirectX::XMVectorSubtract(seg_end, seg_start);
    DirectX::XMVECTOR ap = DirectX::XMVectorSubtract(point, seg_start);

    float t = DirectX::XMVectorGetX(DirectX::XMVector3Dot(ap, ab)) /
        DirectX::XMVectorGetX(DirectX::XMVector3Dot(ab, ab));

    t = max(0.0f, min(1.0f, t));

    return DirectX::XMVectorAdd(seg_start, DirectX::XMVectorScale(ab, t));
}

static bool CheckSphereVsSphereSimple(
    const SphereCollider* a,
    const SphereCollider* b) {
    DirectX::XMFLOAT3 center_a = a->GetWorldCenter();
    DirectX::XMFLOAT3 center_b = b->GetWorldCenter();

    DirectX::XMVECTOR pos_a = DirectX::XMLoadFloat3(&center_a);
    DirectX::XMVECTOR pos_b = DirectX::XMLoadFloat3(&center_b);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(pos_b, pos_a);

    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
    float radius_sum = a->GetRadius() + b->GetRadius();

    return distance < radius_sum;
}

static bool CheckSphereVsSphereRigidbody(
    const SphereCollider* a,
    const SphereCollider* b,
    DirectX::XMFLOAT3& out_correction) {
    DirectX::XMFLOAT3 center_a = a->GetWorldCenter();
    DirectX::XMFLOAT3 center_b = b->GetWorldCenter();

    DirectX::XMVECTOR pos_a = DirectX::XMLoadFloat3(&center_a);
    DirectX::XMVECTOR pos_b = DirectX::XMLoadFloat3(&center_b);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(pos_b, pos_a);

    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
    float radius_sum = a->GetRadius() + b->GetRadius();

    if (distance < radius_sum) {
        float penetration = radius_sum - distance;
        if (distance > 0.0001f) {
            DirectX::XMVECTOR normal = DirectX::XMVector3Normalize(diff);
            DirectX::XMVECTOR correction = DirectX::XMVectorScale(normal, penetration);
            DirectX::XMStoreFloat3(&out_correction, correction);
        }
        else {
            out_correction = { 0, penetration, 0 };
        }
        return true;
    }

    return false;
}

static bool CheckSphereVsBoxSimple(
    const SphereCollider* sphere,
    const BoxCollider* box) {
    DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
    DirectX::XMFLOAT3 box_center = box->GetWorldCenter();
    DirectX::XMFLOAT3 box_size = box->GetSize();

    DirectX::XMFLOAT3 half_size = {
        box_size.x * 0.5f,
        box_size.y * 0.5f,
        box_size.z * 0.5f };

    DirectX::XMFLOAT3 box_min = {
        box_center.x - half_size.x,
        box_center.y - half_size.y,
        box_center.z - half_size.z };
    DirectX::XMFLOAT3 box_max = {
        box_center.x + half_size.x,
        box_center.y + half_size.y,
        box_center.z + half_size.z };

    DirectX::XMFLOAT3 closest = {
        max(box_min.x, min(sphere_center.x, box_max.x)),
        max(box_min.y, min(sphere_center.y, box_max.y)),
        max(box_min.z, min(sphere_center.z, box_max.z)) };

    DirectX::XMVECTOR sphere_center_vec = DirectX::XMLoadFloat3(&sphere_center);
    DirectX::XMVECTOR closest_vec = DirectX::XMLoadFloat3(&closest);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(sphere_center_vec, closest_vec);

    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));

    return distance < sphere->GetRadius();
}

static bool CheckSphereVsBoxRigidbody(
    const SphereCollider* sphere,
    const BoxCollider* box,
    DirectX::XMFLOAT3& out_correction) {
    DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
    DirectX::XMFLOAT3 box_center = box->GetWorldCenter();
    DirectX::XMFLOAT3 box_size = box->GetSize();

    DirectX::XMFLOAT3 half_size = {
        box_size.x * 0.5f,
        box_size.y * 0.5f,
        box_size.z * 0.5f };

    DirectX::XMFLOAT3 box_min = {
        box_center.x - half_size.x,
        box_center.y - half_size.y,
        box_center.z - half_size.z };
    DirectX::XMFLOAT3 box_max = {
        box_center.x + half_size.x,
        box_center.y + half_size.y,
        box_center.z + half_size.z };

    DirectX::XMFLOAT3 closest = {
        max(box_min.x, min(sphere_center.x, box_max.x)),
        max(box_min.y, min(sphere_center.y, box_max.y)),
        max(box_min.z, min(sphere_center.z, box_max.z)) };

    DirectX::XMVECTOR sphere_center_vec = DirectX::XMLoadFloat3(&sphere_center);
    DirectX::XMVECTOR closest_vec = DirectX::XMLoadFloat3(&closest);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(sphere_center_vec, closest_vec);

    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));

    if (distance < sphere->GetRadius()) {
        float penetration = sphere->GetRadius() - distance;
        if (distance > 0.0001f) {
            DirectX::XMVECTOR normal = DirectX::XMVector3Normalize(diff);
            DirectX::XMVECTOR correction = DirectX::XMVectorScale(normal, penetration);
            DirectX::XMStoreFloat3(&out_correction, correction);
        }
        else {
            out_correction = { 0, penetration, 0 };
        }
        return true;
    }

    return false;
}

static bool CheckSphereVsCapsuleSimple(
    const SphereCollider* sphere,
    const CapsuleCollider* capsule) {
    DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
    DirectX::XMFLOAT3 cap_start, cap_end;
    capsule->GetCapsuleSegment(cap_start, cap_end);

    DirectX::XMVECTOR sphere_pos = DirectX::XMLoadFloat3(&sphere_center);
    DirectX::XMVECTOR seg_start = DirectX::XMLoadFloat3(&cap_start);
    DirectX::XMVECTOR seg_end = DirectX::XMLoadFloat3(&cap_end);

    DirectX::XMVECTOR closest = ClosestPointOnSegment(sphere_pos, seg_start, seg_end);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(sphere_pos, closest);

    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
    float radius_sum = sphere->GetRadius() + capsule->GetRadius();

    return distance < radius_sum;
}

static bool CheckSphereVsCapsuleRigidbody(
    const SphereCollider* sphere,
    const CapsuleCollider* capsule,
    DirectX::XMFLOAT3& out_correction) {
    DirectX::XMFLOAT3 sphere_center = sphere->GetWorldCenter();
    DirectX::XMFLOAT3 cap_start, cap_end;
    capsule->GetCapsuleSegment(cap_start, cap_end);

    DirectX::XMVECTOR sphere_pos = DirectX::XMLoadFloat3(&sphere_center);
    DirectX::XMVECTOR seg_start = DirectX::XMLoadFloat3(&cap_start);
    DirectX::XMVECTOR seg_end = DirectX::XMLoadFloat3(&cap_end);

    DirectX::XMVECTOR closest = ClosestPointOnSegment(sphere_pos, seg_start, seg_end);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(sphere_pos, closest);

    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
    float radius_sum = sphere->GetRadius() + capsule->GetRadius();

    if (distance < radius_sum) {
        float penetration = radius_sum - distance;
        if (distance > 0.0001f) {
            DirectX::XMVECTOR normal = DirectX::XMVector3Normalize(diff);
            DirectX::XMVECTOR correction = DirectX::XMVectorScale(normal, penetration);
            DirectX::XMStoreFloat3(&out_correction, correction);
        }
        else {
            out_correction = { 0, penetration, 0 };
        }
        return true;
    }

    return false;
}

static bool CheckBoxVsBoxSimple(
    const BoxCollider* a,
    const BoxCollider* b) {
    DirectX::XMFLOAT3 center_a = a->GetWorldCenter();
    DirectX::XMFLOAT3 center_b = b->GetWorldCenter();
    DirectX::XMFLOAT3 size_a = a->GetSize();
    DirectX::XMFLOAT3 size_b = b->GetSize();

    DirectX::XMFLOAT3 half_a = { size_a.x * 0.5f, size_a.y * 0.5f, size_a.z * 0.5f };
    DirectX::XMFLOAT3 half_b = { size_b.x * 0.5f, size_b.y * 0.5f, size_b.z * 0.5f };

    DirectX::XMFLOAT3 diff = {
        center_b.x - center_a.x,
        center_b.y - center_a.y,
        center_b.z - center_a.z };

    float overlap_x = (half_a.x + half_b.x) - abs(diff.x);
    float overlap_y = (half_a.y + half_b.y) - abs(diff.y);
    float overlap_z = (half_a.z + half_b.z) - abs(diff.z);

    return overlap_x > 0 && overlap_y > 0 && overlap_z > 0;
}

static bool CheckBoxVsBoxRigidbody(
    const BoxCollider* a,
    const BoxCollider* b,
    DirectX::XMFLOAT3& out_correction) {
    DirectX::XMFLOAT3 center_a = a->GetWorldCenter();
    DirectX::XMFLOAT3 center_b = b->GetWorldCenter();
    DirectX::XMFLOAT3 size_a = a->GetSize();
    DirectX::XMFLOAT3 size_b = b->GetSize();

    DirectX::XMFLOAT3 half_a = { size_a.x * 0.5f, size_a.y * 0.5f, size_a.z * 0.5f };
    DirectX::XMFLOAT3 half_b = { size_b.x * 0.5f, size_b.y * 0.5f, size_b.z * 0.5f };

    DirectX::XMFLOAT3 diff = {
        center_b.x - center_a.x,
        center_b.y - center_a.y,
        center_b.z - center_a.z };

    float overlap_x = (half_a.x + half_b.x) - abs(diff.x);
    float overlap_y = (half_a.y + half_b.y) - abs(diff.y);
    float overlap_z = (half_a.z + half_b.z) - abs(diff.z);

    if (overlap_x > 0 && overlap_y > 0 && overlap_z > 0) {
        if (overlap_x < overlap_y && overlap_x < overlap_z) {
            out_correction = {
                (diff.x > 0 ? 1.0f : -1.0f) * overlap_x,
                0,
                0 };
        }
        else if (overlap_y < overlap_z) {
            out_correction = {
                0,
                (diff.y > 0 ? 1.0f : -1.0f) * overlap_y,
                0 };
        }
        else {
            out_correction = {
                0,
                0,
                (diff.z > 0 ? 1.0f : -1.0f) * overlap_z };
        }
        return true;
    }

    return false;
}

static bool CheckCapsuleVsCapsuleSimple(
    const CapsuleCollider* a,
    const CapsuleCollider* b) {
    DirectX::XMFLOAT3 a_start, a_end, b_start, b_end;
    a->GetCapsuleSegment(a_start, a_end);
    b->GetCapsuleSegment(b_start, b_end);

    DirectX::XMVECTOR a_start_vec = DirectX::XMLoadFloat3(&a_start);
    DirectX::XMVECTOR a_end_vec = DirectX::XMLoadFloat3(&a_end);
    DirectX::XMVECTOR b_start_vec = DirectX::XMLoadFloat3(&b_start);
    DirectX::XMVECTOR b_end_vec = DirectX::XMLoadFloat3(&b_end);

    DirectX::XMVECTOR closest_a = ClosestPointOnSegment(b_start_vec, a_start_vec, a_end_vec);
    DirectX::XMVECTOR closest_b = ClosestPointOnSegment(closest_a, b_start_vec, b_end_vec);
    closest_a = ClosestPointOnSegment(closest_b, a_start_vec, a_end_vec);

    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(closest_b, closest_a);
    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
    float radius_sum = a->GetRadius() + b->GetRadius();

    return distance < radius_sum;
}

static bool CheckCapsuleVsCapsuleRigidbody(
    const CapsuleCollider* a,
    const CapsuleCollider* b,
    DirectX::XMFLOAT3& out_correction) {
    DirectX::XMFLOAT3 a_start, a_end, b_start, b_end;
    a->GetCapsuleSegment(a_start, a_end);
    b->GetCapsuleSegment(b_start, b_end);

    DirectX::XMVECTOR a_start_vec = DirectX::XMLoadFloat3(&a_start);
    DirectX::XMVECTOR a_end_vec = DirectX::XMLoadFloat3(&a_end);
    DirectX::XMVECTOR b_start_vec = DirectX::XMLoadFloat3(&b_start);
    DirectX::XMVECTOR b_end_vec = DirectX::XMLoadFloat3(&b_end);

    DirectX::XMVECTOR closest_a = ClosestPointOnSegment(b_start_vec, a_start_vec, a_end_vec);
    DirectX::XMVECTOR closest_b = ClosestPointOnSegment(closest_a, b_start_vec, b_end_vec);
    closest_a = ClosestPointOnSegment(closest_b, a_start_vec, a_end_vec);

    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(closest_b, closest_a);
    float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));
    float radius_sum = a->GetRadius() + b->GetRadius();

    if (distance < radius_sum) {
        float penetration = radius_sum - distance;
        if (distance > 0.0001f) {
            DirectX::XMVECTOR normal = DirectX::XMVector3Normalize(diff);
            DirectX::XMVECTOR correction = DirectX::XMVectorScale(normal, penetration);
            DirectX::XMStoreFloat3(&out_correction, correction);
        }
        else {
            out_correction = { 0, penetration, 0 };
        }
        return true;
    }

    return false;
}

bool SphereCollider::CheckCollision(const Collider* other, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CheckSphereVsSphereSimple(this, static_cast<const SphereCollider*>(other));
        break;
    case ColliderType::kBox:
        has_collision = CheckSphereVsBoxSimple(this, static_cast<const BoxCollider*>(other));
        break;
    case ColliderType::kCapsule:
        has_collision = CheckSphereVsCapsuleSimple(this, static_cast<const CapsuleCollider*>(other));
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

bool SphereCollider::CheckRigidbodyCollision(const Collider* other, DirectX::XMFLOAT3& out_correction, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CheckSphereVsSphereRigidbody(this, static_cast<const SphereCollider*>(other), out_correction);
        break;
    case ColliderType::kBox:
        has_collision = CheckSphereVsBoxRigidbody(this, static_cast<const BoxCollider*>(other), out_correction);
        break;
    case ColliderType::kCapsule:
        has_collision = CheckSphereVsCapsuleRigidbody(this, static_cast<const CapsuleCollider*>(other), out_correction);
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

bool BoxCollider::CheckCollision(const Collider* other, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CheckSphereVsBoxSimple(static_cast<const SphereCollider*>(other), this);
        break;
    case ColliderType::kBox:
        has_collision = CheckBoxVsBoxSimple(this, static_cast<const BoxCollider*>(other));
        break;
    case ColliderType::kCapsule:
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

bool BoxCollider::CheckRigidbodyCollision(const Collider* other, DirectX::XMFLOAT3& out_correction, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CheckSphereVsBoxRigidbody(static_cast<const SphereCollider*>(other), this, out_correction);
        if (has_collision) {
            out_correction.x = -out_correction.x;
            out_correction.y = -out_correction.y;
            out_correction.z = -out_correction.z;
        }
        break;
    case ColliderType::kBox:
        has_collision = CheckBoxVsBoxRigidbody(this, static_cast<const BoxCollider*>(other), out_correction);
        break;
    case ColliderType::kCapsule:
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

bool CapsuleCollider::CheckCollision(const Collider* other, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CheckSphereVsCapsuleSimple(static_cast<const SphereCollider*>(other), this);
        break;
    case ColliderType::kBox:
        break;
    case ColliderType::kCapsule:
        has_collision = CheckCapsuleVsCapsuleSimple(this, static_cast<const CapsuleCollider*>(other));
        break;
    }

    if (has_collision) {
        out_other = other->GetOwner();
    }

    return has_collision;
}

bool CapsuleCollider::CheckRigidbodyCollision(const Collider* other, DirectX::XMFLOAT3& out_correction, GameObject*& out_other) const {
    if (!other || !other->IsEnabled() || !IsEnabled()) {
        return false;
    }

    bool has_collision = false;

    switch (other->GetType()) {
    case ColliderType::kSphere:
        has_collision = CheckSphereVsCapsuleRigidbody(static_cast<const SphereCollider*>(other), this, out_correction);
        if (has_collision) {
            out_correction.x = -out_correction.x;
            out_correction.y = -out_correction.y;
            out_correction.z = -out_correction.z;
        }
        break;
    case ColliderType::kBox:
        break;
    case ColliderType::kCapsule:
        has_collision = CheckCapsuleVsCapsuleRigidbody(this, static_cast<const CapsuleCollider*>(other), out_correction);
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

    size = size_;
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