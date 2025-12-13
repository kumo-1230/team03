#include "game_object.h"
#include "collider.h"
#include "rigidbody.h"
#include <algorithm>
#include "System/ModelRenderer.h"
#include "PBRShader.h"
#include "System/Shader.h"

GameObject::~GameObject() {
    // コライダーの削除
    for (Collider* collider : colliders_) {
        if (collider) {
            delete collider;
        }
    }
    colliders_.clear();

    // リジッドボディの削除
    if (rigidbody_) {
        delete rigidbody_;
        rigidbody_ = nullptr;
    }

    // 子オブジェクトの親参照をクリア
    for (GameObject* child : children_) {
        if (child) {
            child->parent_ = nullptr;
        }
    }
}

void GameObject::Update(float elapsed_time) {
    if (!IsActiveInHierarchy()) return;

    elapsed_time_ += elapsed_time;

    position_.x += velocity_.x * elapsed_time;
    position_.y += velocity_.y * elapsed_time;
    position_.z += velocity_.z * elapsed_time;

    UpdateTransform();

    for (GameObject* child : children_) {
        if (child) {
            child->Update(elapsed_time);
        }
    }
}

void GameObject::Render(const RenderContext& rc, ModelRenderer* model_renderer) {
    if (!IsActiveInHierarchy() || !model_) return;

    model_renderer->Draw(ShaderId::PBR, model_);

    for (GameObject* child : children_) {
        if (child) {
            child->Render(rc, model_renderer);
        }
    }
}

void GameObject::SetParentTransformOnly(GameObject* parent, bool keep_world_position) {
    if (keep_world_position && parent) {
        DirectX::XMFLOAT3 world_pos = GetWorldPosition();
        DirectX::XMFLOAT3 world_rot = angle_;
        DirectX::XMFLOAT3 world_scale = scale_;

        DetachFromParent();

        parent_ = parent;
        hierarchy_type_ = HierarchyType::kTransformOnly;
        parent_->children_.push_back(this);

        DirectX::XMMATRIX parent_world_inv = DirectX::XMMatrixInverse(nullptr, parent->GetWorldTransformM());
        DirectX::XMVECTOR world_pos_v = DirectX::XMLoadFloat3(&world_pos);
        DirectX::XMVECTOR local_pos_v = DirectX::XMVector3TransformCoord(world_pos_v, parent_world_inv);
        DirectX::XMStoreFloat3(&position_, local_pos_v);
    }
    else {
        DetachFromParent();

        parent_ = parent;
        hierarchy_type_ = HierarchyType::kTransformOnly;

        if (parent_) {
            parent_->children_.push_back(this);
        }
    }

    UpdateTransform();
}

void GameObject::SetParent(GameObject* parent, bool keep_world_position) {
    if (keep_world_position && parent) {
        DirectX::XMFLOAT3 world_pos = GetWorldPosition();
        DirectX::XMFLOAT3 world_rot = angle_;
        DirectX::XMFLOAT3 world_scale = scale_;

        DetachFromParent();

        parent_ = parent;
        hierarchy_type_ = HierarchyType::kFull;
        parent_->children_.push_back(this);

        DirectX::XMMATRIX parent_world_inv = DirectX::XMMatrixInverse(nullptr, parent->GetWorldTransformM());
        DirectX::XMVECTOR world_pos_v = DirectX::XMLoadFloat3(&world_pos);
        DirectX::XMVECTOR local_pos_v = DirectX::XMVector3TransformCoord(world_pos_v, parent_world_inv);
        DirectX::XMStoreFloat3(&position_, local_pos_v);
    }
    else {
        DetachFromParent();

        parent_ = parent;
        hierarchy_type_ = HierarchyType::kFull;

        if (parent_) {
            parent_->children_.push_back(this);
        }
    }

    UpdateTransform();
}

void GameObject::DetachFromParent() {
    if (parent_) {
        auto& siblings = parent_->children_;
        siblings.erase(
            std::remove(siblings.begin(), siblings.end(), this),
            siblings.end());
        parent_ = nullptr;
    }
    hierarchy_type_ = HierarchyType::kNone;
}

bool GameObject::IsActiveInHierarchy() const {
    if (!active_) return false;

    if (hierarchy_type_ == HierarchyType::kFull && parent_) {
        return parent_->IsActiveInHierarchy();
    }

    return true;
}

DirectX::XMFLOAT3 GameObject::GetWorldPosition() const {
    DirectX::XMFLOAT3 world_pos;
    DirectX::XMMATRIX world = GetWorldTransformM();
    DirectX::XMStoreFloat3(&world_pos, world.r[3]);
    return world_pos;
}

DirectX::XMFLOAT4X4 GameObject::GetWorldTransform() const {
    DirectX::XMFLOAT4X4 world_transform;
    DirectX::XMStoreFloat4x4(&world_transform, GetWorldTransformM());
    return world_transform;
}

DirectX::XMMATRIX GameObject::GetWorldTransformM() const {
    DirectX::XMMATRIX local_transform = DirectX::XMLoadFloat4x4(&transform_);

    if (parent_ && hierarchy_type_ != HierarchyType::kNone) {
        DirectX::XMMATRIX parent_world = parent_->GetWorldTransformM();
        return local_transform * parent_world;
    }

    return local_transform;
}

void GameObject::SetPosition(DirectX::FXMVECTOR v) {
    DirectX::XMFLOAT3 pos;
    DirectX::XMStoreFloat3(&pos, v);
    SetLocalPosition(pos);
}

void GameObject::SetLocalPosition(const DirectX::XMFLOAT3& pos) {
    position_ = pos;
    UpdateTransform();
}

void GameObject::SetLocalPosition(float x, float y, float z) {
    position_ = { x, y, z };
    UpdateTransform();
}

void GameObject::SetWorldPosition(const DirectX::XMFLOAT3& world_pos) {
    if (parent_ && hierarchy_type_ != HierarchyType::kNone) {
        DirectX::XMMATRIX parent_world_inv = DirectX::XMMatrixInverse(nullptr, parent_->GetWorldTransformM());
        DirectX::XMVECTOR world_pos_v = DirectX::XMLoadFloat3(&world_pos);
        DirectX::XMVECTOR local_pos_v = DirectX::XMVector3TransformCoord(world_pos_v, parent_world_inv);
        DirectX::XMStoreFloat3(&position_, local_pos_v);
    }
    else {
        position_ = world_pos;
    }
    UpdateTransform();
}

void GameObject::SetWorldPosition(float x, float y, float z) {
    SetWorldPosition(DirectX::XMFLOAT3(x, y, z));
}

void GameObject::SetAngle(DirectX::FXMVECTOR v) {
    DirectX::XMStoreFloat3(&angle_, v);
    UpdateTransform();
}

void GameObject::SetAngleDegree(const DirectX::XMFLOAT3& deg) {
    angle_.x = DirectX::XMConvertToRadians(deg.x);
    angle_.y = DirectX::XMConvertToRadians(deg.y);
    angle_.z = DirectX::XMConvertToRadians(deg.z);
    UpdateTransform();
}

void GameObject::SetAngleDegree(float x, float y, float z) {
    angle_.x = DirectX::XMConvertToRadians(x);
    angle_.y = DirectX::XMConvertToRadians(y);
    angle_.z = DirectX::XMConvertToRadians(z);
    UpdateTransform();
}

DirectX::XMFLOAT3 GameObject::GetAngleDegree() const {
    return {
        DirectX::XMConvertToDegrees(angle_.x),
        DirectX::XMConvertToDegrees(angle_.y),
        DirectX::XMConvertToDegrees(angle_.z) };
}

void GameObject::SetScale(DirectX::FXMVECTOR v) {
    DirectX::XMStoreFloat3(&scale_, v);
    UpdateTransform();
}

void GameObject::SetVelocity(DirectX::FXMVECTOR v) {
    DirectX::XMStoreFloat3(&velocity_, v);
}

void GameObject::AddVelocity(const DirectX::XMFLOAT3& vel) {
    velocity_.x += vel.x;
    velocity_.y += vel.y;
    velocity_.z += vel.z;
}

void GameObject::AddVelocity(float x, float y, float z) {
    velocity_.x += x;
    velocity_.y += y;
    velocity_.z += z;
}

void GameObject::AddVelocity(DirectX::FXMVECTOR v) {
    DirectX::XMVECTOR current = DirectX::XMLoadFloat3(&velocity_);
    DirectX::XMVECTOR result = DirectX::XMVectorAdd(current, v);
    DirectX::XMStoreFloat3(&velocity_, result);
}

void GameObject::SetModel(const char* filepath) {
    model_ = ResourceManager::Instance().LoadModel(filepath);
    UpdateTransform();
}

void GameObject::SetModel(const std::shared_ptr<Model>& model) {
    model_ = model;
    UpdateTransform();
}

void GameObject::SetCollider(Collider* collider) {
    if (collider) {
        RemoveAllColliders();
        collider->SetOwner(this);
        colliders_.push_back(collider);
    }
}

void GameObject::RemoveCollider(Collider* collider) {
    auto it = std::find(colliders_.begin(), colliders_.end(), collider);
    if (it != colliders_.end()) {
        delete* it;
        colliders_.erase(it);
    }
}

void GameObject::RemoveAllColliders() {
    for (Collider* collider : colliders_) {
        if (collider) {
            delete collider;
        }
    }
    colliders_.clear();
}

void GameObject::SetRigidbody(Rigidbody* rigidbody) {
    if (rigidbody_) {
        delete rigidbody_;
    }
    rigidbody_ = rigidbody;
    if (rigidbody_) {
        rigidbody_->SetOwner(this);
    }
}

void GameObject::RemoveRigidbody() {
    if (rigidbody_) {
        delete rigidbody_;
        rigidbody_ = nullptr;
    }
}

void GameObject::Destroy() {
    active_ = false;

    std::vector<GameObject*> children_copy = children_;
    for (GameObject* child : children_copy) {
        if (child) {
            if (child->hierarchy_type_ == HierarchyType::kFull) {
                child->Destroy();
            }
            else {
                child->DetachFromParent();
            }
        }
    }
}

DirectX::XMVECTOR GameObject::GetForward() const {
    DirectX::XMMATRIX m = GetWorldTransformM();
    return DirectX::XMVector3Normalize(m.r[2]);
}

DirectX::XMVECTOR GameObject::GetRight() const {
    DirectX::XMMATRIX m = GetWorldTransformM();
    return DirectX::XMVector3Normalize(m.r[0]);
}

DirectX::XMVECTOR GameObject::GetUp() const {
    DirectX::XMMATRIX m = GetWorldTransformM();
    return DirectX::XMVector3Normalize(m.r[1]);
}

DirectX::XMFLOAT3 GameObject::GetForwardFloat3() const {
    DirectX::XMMATRIX m = GetWorldTransformM();
    DirectX::XMVECTOR v = DirectX::XMVector3Normalize(m.r[2]);
    DirectX::XMFLOAT3 out;
    DirectX::XMStoreFloat3(&out, v);
    return out;
}

DirectX::XMFLOAT3 GameObject::GetRightFloat3() const {
    DirectX::XMMATRIX m = GetWorldTransformM();
    DirectX::XMVECTOR v = DirectX::XMVector3Normalize(m.r[0]);
    DirectX::XMFLOAT3 out;
    DirectX::XMStoreFloat3(&out, v);
    return out;
}

DirectX::XMFLOAT3 GameObject::GetUpFloat3() const {
    DirectX::XMMATRIX m = GetWorldTransformM();
    DirectX::XMVECTOR v = DirectX::XMVector3Normalize(m.r[1]);
    DirectX::XMFLOAT3 out;
    DirectX::XMStoreFloat3(&out, v);
    return out;
}

void GameObject::Move(const DirectX::XMFLOAT3& offset) {
    position_.x += offset.x;
    position_.y += offset.y;
    position_.z += offset.z;
    UpdateTransform();
}

void GameObject::Move(float x, float y, float z) {
    position_.x += x;
    position_.y += y;
    position_.z += z;
    UpdateTransform();
}

void GameObject::Move(DirectX::FXMVECTOR v) {
    DirectX::XMVECTOR current = DirectX::XMLoadFloat3(&position_);
    DirectX::XMVECTOR result = DirectX::XMVectorAdd(current, v);
    DirectX::XMStoreFloat3(&position_, result);
    UpdateTransform();
}

void GameObject::MoveForward(float distance) {
    DirectX::XMVECTOR forward = GetForward();
    DirectX::XMVECTOR offset = DirectX::XMVectorScale(forward, distance);
    Move(offset);
}

void GameObject::MoveRight(float distance) {
    DirectX::XMVECTOR right = GetRight();
    DirectX::XMVECTOR offset = DirectX::XMVectorScale(right, distance);
    Move(offset);
}

void GameObject::MoveUp(float distance) {
    DirectX::XMVECTOR up = GetUp();
    DirectX::XMVECTOR offset = DirectX::XMVectorScale(up, distance);
    Move(offset);
}

void GameObject::Rotate(const DirectX::XMFLOAT3& delta) {
    angle_.x += delta.x;
    angle_.y += delta.y;
    angle_.z += delta.z;
    UpdateTransform();
}

void GameObject::Rotate(float x, float y, float z) {
    angle_.x += x;
    angle_.y += y;
    angle_.z += z;
    UpdateTransform();
}

void GameObject::RotateDegree(float x, float y, float z) {
    angle_.x += DirectX::XMConvertToRadians(x);
    angle_.y += DirectX::XMConvertToRadians(y);
    angle_.z += DirectX::XMConvertToRadians(z);
    UpdateTransform();
}

void GameObject::LookAt(const DirectX::XMFLOAT3& target) {
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position_);
    DirectX::XMVECTOR tar = DirectX::XMLoadFloat3(&target);
    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(tar, pos);

    float x = DirectX::XMVectorGetX(dir);
    float z = DirectX::XMVectorGetZ(dir);
    angle_.y = atan2f(x, z);
    UpdateTransform();
}

void GameObject::LookAt(DirectX::FXMVECTOR target) {
    DirectX::XMFLOAT3 t;
    DirectX::XMStoreFloat3(&t, target);
    LookAt(t);
}

float GameObject::GetDistanceTo(const GameObject* other) const {
    if (!other) return 0.0f;
    DirectX::XMFLOAT3 world_pos1 = GetWorldPosition();
    DirectX::XMFLOAT3 world_pos2 = other->GetWorldPosition();
    DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&world_pos1);
    DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&world_pos2);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(p2, p1);
    DirectX::XMVECTOR length = DirectX::XMVector3Length(diff);
    return DirectX::XMVectorGetX(length);
}

float GameObject::GetDistanceTo(const DirectX::XMFLOAT3& point) const {
    DirectX::XMFLOAT3 world_pos = GetWorldPosition();
    DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&world_pos);
    DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&point);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(p2, p1);
    DirectX::XMVECTOR length = DirectX::XMVector3Length(diff);
    return DirectX::XMVectorGetX(length);
}

float GameObject::GetDistanceSquaredTo(const GameObject* other) const {
    if (!other) return 0.0f;
    DirectX::XMFLOAT3 world_pos1 = GetWorldPosition();
    DirectX::XMFLOAT3 world_pos2 = other->GetWorldPosition();
    float dx = world_pos2.x - world_pos1.x;
    float dy = world_pos2.y - world_pos1.y;
    float dz = world_pos2.z - world_pos1.z;
    return dx * dx + dy * dy + dz * dz;
}

DirectX::XMVECTOR GameObject::GetDirectionTo(const GameObject* other) const {
    if (!other) return DirectX::XMVectorZero();
    DirectX::XMFLOAT3 world_pos1 = GetWorldPosition();
    DirectX::XMFLOAT3 world_pos2 = other->GetWorldPosition();
    DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&world_pos1);
    DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&world_pos2);
    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(p2, p1);
    return DirectX::XMVector3Normalize(dir);
}

DirectX::XMVECTOR GameObject::GetDirectionTo(const DirectX::XMFLOAT3& point) const {
    DirectX::XMFLOAT3 world_pos = GetWorldPosition();
    DirectX::XMVECTOR p1 = DirectX::XMLoadFloat3(&world_pos);
    DirectX::XMVECTOR p2 = DirectX::XMLoadFloat3(&point);
    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(p2, p1);
    return DirectX::XMVector3Normalize(dir);
}

bool GameObject::IsInRange(const GameObject* other, float range) const {
    if (!other) return false;
    return GetDistanceSquaredTo(other) <= (range * range);
}

void GameObject::SetTransform(const DirectX::XMFLOAT4X4& transform) {
    transform_ = transform;
    if (model_) {
        DirectX::XMFLOAT4X4 world_transform = GetWorldTransform();
        model_->UpdateTransform(world_transform);
    }
}

void GameObject::ApplyImpulse(const DirectX::XMFLOAT3& impulse) {
    AddVelocity(impulse);
}

void GameObject::ApplyImpulse(DirectX::FXMVECTOR impulse) {
    AddVelocity(impulse);
}

void GameObject::StopMovement() {
    velocity_ = { 0.0f, 0.0f, 0.0f };
}

float GameObject::GetSpeed() const {
    DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&velocity_);
    DirectX::XMVECTOR length = DirectX::XMVector3Length(v);
    return DirectX::XMVectorGetX(length);
}

void GameObject::SetSpeed(float speed) {
    DirectX::XMVECTOR v = DirectX::XMLoadFloat3(&velocity_);
    DirectX::XMVECTOR normalized = DirectX::XMVector3Normalize(v);
    DirectX::XMVECTOR result = DirectX::XMVectorScale(normalized, speed);
    DirectX::XMStoreFloat3(&velocity_, result);
}

void GameObject::UpdateTransform() {
    DirectX::XMMATRIX S = DirectX::XMMatrixScaling(scale_.x, scale_.y, scale_.z);
    DirectX::XMMATRIX R = DirectX::XMMatrixRotationRollPitchYaw(angle_.x, angle_.y, angle_.z);
    DirectX::XMMATRIX T = DirectX::XMMatrixTranslation(position_.x, position_.y, position_.z);
    DirectX::XMMATRIX W = S * R * T;
    DirectX::XMStoreFloat4x4(&transform_, W);

    if (model_) {
        DirectX::XMFLOAT4X4 world_transform = GetWorldTransform();
        model_->UpdateTransform(world_transform);
    }

    for (GameObject* child : children_) {
        if (child) {
            child->UpdateTransform();
        }
    }
}