#include "game_object.h"
#include "collider.h"
#include "rigidbody.h"
#include <algorithm>
#include "System/ModelRenderer.h"
#include "PBRShader.h"
#include "System/Shader.h"

GameObject::~GameObject() {
    for (Collider* collider : colliders_) {
        delete collider;
    }
    colliders_.clear();

    delete rigidbody_;

    for (GameObject* child : children_) {
        if (child) {
            child->parent_ = nullptr;
        }
    }
}

void GameObject::Update(float elapsed_time) {
    if (!IsActiveInHierarchy()) return;

    elapsed_time_ += elapsed_time;

    DirectX::XMVECTOR pos_vec = DirectX::XMLoadFloat3(&position_);
    DirectX::XMVECTOR vel_vec = DirectX::XMLoadFloat3(&velocity_);
    DirectX::XMVECTOR scaled_vel = DirectX::XMVectorScale(vel_vec, elapsed_time);
    pos_vec = DirectX::XMVectorAdd(pos_vec, scaled_vel);
    DirectX::XMStoreFloat3(&position_, pos_vec);

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

void GameObject::SetParentTransformOnly(GameObject* parent,
    bool keep_world_position) {
    if (keep_world_position && parent) {
        // ワールド座標を保存
        DirectX::XMVECTOR world_pos = GetWorldPositionVector();

        DetachFromParent();

        parent_ = parent;
        hierarchy_type_ = HierarchyType::kTransformOnly;
        parent_->children_.push_back(this);

        // ローカル座標に変換
        DirectX::XMMATRIX parent_world_inv = DirectX::XMMatrixInverse(
            nullptr, parent->GetWorldTransformMatrix());
        DirectX::XMVECTOR local_pos = DirectX::XMVector3TransformCoord(
            world_pos, parent_world_inv);
        DirectX::XMStoreFloat3(&position_, local_pos);
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
        // ワールド座標を保存
        DirectX::XMVECTOR world_pos = GetWorldPositionVector();

        DetachFromParent();

        parent_ = parent;
        hierarchy_type_ = HierarchyType::kFull;
        parent_->children_.push_back(this);

        // ローカル座標に変換
        DirectX::XMMATRIX parent_world_inv = DirectX::XMMatrixInverse(
            nullptr, parent->GetWorldTransformMatrix());
        DirectX::XMVECTOR local_pos = DirectX::XMVector3TransformCoord(
            world_pos, parent_world_inv);
        DirectX::XMStoreFloat3(&position_, local_pos);
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

DirectX::XMVECTOR GameObject::GetWorldPositionVector() const {
    DirectX::XMMATRIX world = GetWorldTransformMatrix();
    return world.r[3];
}

DirectX::XMFLOAT3 GameObject::GetWorldPositionFloat3() const {
    DirectX::XMFLOAT3 world_pos;
    DirectX::XMStoreFloat3(&world_pos, GetWorldPositionVector());
    return world_pos;
}

DirectX::XMFLOAT4X4 GameObject::GetWorldTransformFloat4X4() const {
    DirectX::XMFLOAT4X4 world_transform;
    DirectX::XMStoreFloat4x4(&world_transform, GetWorldTransformMatrix());
    return world_transform;
}

DirectX::XMMATRIX GameObject::GetWorldTransformMatrix() const {
    DirectX::XMMATRIX local_transform = DirectX::XMLoadFloat4x4(&transform_);

    if (parent_ && hierarchy_type_ != HierarchyType::kNone) {
        DirectX::XMMATRIX parent_world = parent_->GetWorldTransformMatrix();
        return local_transform * parent_world;
    }

    return local_transform;
}

void GameObject::SetLocalPosition(const DirectX::XMFLOAT3& pos) {
    position_ = pos;
    UpdateTransform();
}

void GameObject::SetLocalPosition(float x, float y, float z) {
    position_ = { x, y, z };
    UpdateTransform();
}

void GameObject::SetLocalPosition(DirectX::FXMVECTOR v) {
    DirectX::XMStoreFloat3(&position_, v);
    UpdateTransform();
}

void GameObject::SetWorldPosition(const DirectX::XMFLOAT3& world_pos) {
    SetWorldPosition(DirectX::XMLoadFloat3(&world_pos));
}

void GameObject::SetWorldPosition(float x, float y, float z) {
    SetWorldPosition(DirectX::XMVectorSet(x, y, z, 1.0f));
}

void GameObject::SetWorldPosition(DirectX::FXMVECTOR v) {
    if (parent_ && hierarchy_type_ != HierarchyType::kNone) {
        DirectX::XMMATRIX parent_world_inv = DirectX::XMMatrixInverse(
            nullptr, parent_->GetWorldTransformMatrix());
        DirectX::XMVECTOR local_pos = DirectX::XMVector3TransformCoord(
            v, parent_world_inv);
        DirectX::XMStoreFloat3(&position_, local_pos);
    }
    else {
        DirectX::XMStoreFloat3(&position_, v);
    }
    UpdateTransform();
}

void GameObject::SetAngle(DirectX::FXMVECTOR v) {
    DirectX::XMStoreFloat3(&angle_, v);
    UpdateTransform();
}

void GameObject::SetAngle(const DirectX::XMFLOAT3& angle) {
    angle_ = angle;
    UpdateTransform();
}

void GameObject::SetAngle(float x, float y, float z) {
    angle_ = { x, y, z };
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
    AddVelocity(DirectX::XMLoadFloat3(&vel));
}

void GameObject::AddVelocity(float x, float y, float z) {
    AddVelocity(DirectX::XMVectorSet(x, y, z, 0.0f));
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

void GameObject::RemoveCollider(Collider* collider) {
    auto it = std::find(colliders_.begin(), colliders_.end(), collider);
    if (it != colliders_.end()) {
        delete* it;
        colliders_.erase(it);
    }
}

void GameObject::RemoveAllColliders() {
    for (Collider* collider : colliders_) {
        delete collider;
    }
    colliders_.clear();
}

void GameObject::SetRigidbody(Rigidbody* rigidbody) {
    delete rigidbody_;
    rigidbody_ = rigidbody;
    if (rigidbody_) {
        rigidbody_->SetOwner(this);
    }
}

Rigidbody* GameObject::AddRigidbody() {
    delete rigidbody_;

    rigidbody_ = new Rigidbody();
    rigidbody_->SetOwner(this);

    return rigidbody_;
}

void GameObject::RemoveRigidbody() {
    delete rigidbody_;
    rigidbody_ = nullptr;
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

DirectX::XMVECTOR GameObject::GetForwardVector() const {
    DirectX::XMMATRIX world_matrix = GetWorldTransformMatrix();
    return DirectX::XMVector3Normalize(world_matrix.r[2]);
}

DirectX::XMVECTOR GameObject::GetRightVector() const {
    DirectX::XMMATRIX world_matrix = GetWorldTransformMatrix();
    return DirectX::XMVector3Normalize(world_matrix.r[0]);
}

DirectX::XMVECTOR GameObject::GetUpVector() const {
    DirectX::XMMATRIX world_matrix = GetWorldTransformMatrix();
    return DirectX::XMVector3Normalize(world_matrix.r[1]);
}

DirectX::XMFLOAT3 GameObject::GetForwardFloat3() const {
    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, GetForwardVector());
    return result;
}

DirectX::XMFLOAT3 GameObject::GetRightFloat3() const {
    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, GetRightVector());
    return result;
}

DirectX::XMFLOAT3 GameObject::GetUpFloat3() const {
    DirectX::XMFLOAT3 result;
    DirectX::XMStoreFloat3(&result, GetUpVector());
    return result;
}

void GameObject::Move(const DirectX::XMFLOAT3& offset) {
    Move(DirectX::XMLoadFloat3(&offset));
}

void GameObject::Move(float x, float y, float z) {
    Move(DirectX::XMVectorSet(x, y, z, 0.0f));
}

void GameObject::Move(DirectX::FXMVECTOR v) {
    DirectX::XMVECTOR current = DirectX::XMLoadFloat3(&position_);
    DirectX::XMVECTOR result = DirectX::XMVectorAdd(current, v);
    DirectX::XMStoreFloat3(&position_, result);
    UpdateTransform();
}

void GameObject::MoveForward(float distance) {
    DirectX::XMVECTOR forward = GetForwardVector();
    DirectX::XMVECTOR offset = DirectX::XMVectorScale(forward, distance);
    Move(offset);
}

void GameObject::MoveRight(float distance) {
    DirectX::XMVECTOR right = GetRightVector();
    DirectX::XMVECTOR offset = DirectX::XMVectorScale(right, distance);
    Move(offset);
}

void GameObject::MoveUp(float distance) {
    DirectX::XMVECTOR up = GetUpVector();
    DirectX::XMVECTOR offset = DirectX::XMVectorScale(up, distance);
    Move(offset);
}

void GameObject::Rotate(const DirectX::XMFLOAT3& delta) {
    DirectX::XMVECTOR current = DirectX::XMLoadFloat3(&angle_);
    DirectX::XMVECTOR delta_vec = DirectX::XMLoadFloat3(&delta);
    DirectX::XMVECTOR result = DirectX::XMVectorAdd(current, delta_vec);
    DirectX::XMStoreFloat3(&angle_, result);
    UpdateTransform();
}

void GameObject::Rotate(float x, float y, float z) {
    Rotate(DirectX::XMFLOAT3(x, y, z));
}

void GameObject::RotateDegree(float x, float y, float z) {
    Rotate(DirectX::XMConvertToRadians(x)
        , DirectX::XMConvertToRadians(y)
        , DirectX::XMConvertToRadians(z));
}

void GameObject::LookAt(const DirectX::XMFLOAT3& target) {
    LookAt(DirectX::XMLoadFloat3(&target));
}

void GameObject::LookAt(DirectX::FXMVECTOR target) {
    DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position_);
    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(target, pos);

    float x = DirectX::XMVectorGetX(dir);
    float z = DirectX::XMVectorGetZ(dir);
    angle_.y = atan2f(x, z);
    UpdateTransform();
}

float GameObject::GetDistanceTo(const GameObject* other) const {
    if (!other) return 0.0f;

    DirectX::XMVECTOR pos1 = GetWorldPositionVector();
    DirectX::XMVECTOR pos2 = other->GetWorldPositionVector();
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(pos2, pos1);
    DirectX::XMVECTOR length = DirectX::XMVector3Length(diff);
    return DirectX::XMVectorGetX(length);
}

float GameObject::GetDistanceTo(const DirectX::XMFLOAT3& point) const {
    DirectX::XMVECTOR pos1 = GetWorldPositionVector();
    DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&point);
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(pos2, pos1);
    DirectX::XMVECTOR length = DirectX::XMVector3Length(diff);
    return DirectX::XMVectorGetX(length);
}

float GameObject::GetDistanceSquaredTo(const GameObject* other) const {
    if (!other) return 0.0f;

    // SIMD最適化: XMVector3LengthSqを使用
    DirectX::XMVECTOR pos1 = GetWorldPositionVector();
    DirectX::XMVECTOR pos2 = other->GetWorldPositionVector();
    DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(pos2, pos1);
    DirectX::XMVECTOR length_sq = DirectX::XMVector3LengthSq(diff);
    return DirectX::XMVectorGetX(length_sq);
}

DirectX::XMVECTOR GameObject::GetDirectionTo(const GameObject* other) const {
    if (!other) return DirectX::XMVectorZero();

    DirectX::XMVECTOR pos1 = GetWorldPositionVector();
    DirectX::XMVECTOR pos2 = other->GetWorldPositionVector();
    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(pos2, pos1);
    return DirectX::XMVector3Normalize(dir);
}

DirectX::XMVECTOR GameObject::GetDirectionTo(
    const DirectX::XMFLOAT3& point) const {
    DirectX::XMVECTOR pos1 = GetWorldPositionVector();
    DirectX::XMVECTOR pos2 = DirectX::XMLoadFloat3(&point);
    DirectX::XMVECTOR dir = DirectX::XMVectorSubtract(pos2, pos1);
    return DirectX::XMVector3Normalize(dir);
}

bool GameObject::IsInRange(const GameObject* other, float range) const {
    if (!other) return false;
    return GetDistanceSquaredTo(other) <= (range * range);
}

void GameObject::SetTransform(const DirectX::XMFLOAT4X4& transform) {
    transform_ = transform;
    if (model_) {
        DirectX::XMFLOAT4X4 world_transform = GetWorldTransformFloat4X4();
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
    DirectX::XMVECTOR vel = DirectX::XMLoadFloat3(&velocity_);
    DirectX::XMVECTOR length = DirectX::XMVector3Length(vel);
    return DirectX::XMVectorGetX(length);
}

void GameObject::SetSpeed(float speed) {
    DirectX::XMVECTOR vel = DirectX::XMLoadFloat3(&velocity_);
    DirectX::XMVECTOR normalized = DirectX::XMVector3Normalize(vel);
    DirectX::XMVECTOR result = DirectX::XMVectorScale(normalized, speed);
    DirectX::XMStoreFloat3(&velocity_, result);
}

void GameObject::UpdateTransform() {
    DirectX::XMMATRIX scale_matrix = DirectX::XMMatrixScaling(
        scale_.x, scale_.y, scale_.z);
    DirectX::XMMATRIX rotation_matrix = DirectX::XMMatrixRotationRollPitchYaw(
        angle_.x, angle_.y, angle_.z);
    DirectX::XMMATRIX translation_matrix = DirectX::XMMatrixTranslation(
        position_.x, position_.y, position_.z);
    DirectX::XMMATRIX world_matrix = scale_matrix * rotation_matrix *
        translation_matrix;
    DirectX::XMStoreFloat4x4(&transform_, world_matrix);

    if (model_) {
        DirectX::XMFLOAT4X4 world_transform = GetWorldTransformFloat4X4();
        model_->UpdateTransform(world_transform);
    }

    for (GameObject* child : children_) {
        if (child) {
            child->UpdateTransform();
        }
    }
}