#include "collider.h"
#include "game_object.h"

//DirectX::XMFLOAT3 Collider::GetWorldCenter() const {
//    if (!owner_) return offset_;
//
//    DirectX::XMMATRIX world_transform = owner_->GetWorldTransformMatrix();
//    DirectX::XMVECTOR offset_vec = DirectX::XMLoadFloat3(&offset_);
//    DirectX::XMVECTOR world_center = DirectX::XMVector3TransformCoord(offset_vec, world_transform);
//
//    DirectX::XMFLOAT3 center;
//    DirectX::XMStoreFloat3(&center, world_center);
//    return center;
//}

DirectX::XMFLOAT3 Collider::GetWorldCenter() const {
    if (!owner_) return offset_;

    DirectX::XMMATRIX world_transform = owner_->GetWorldTransformMatrix();
    DirectX::XMVECTOR offset_vec = DirectX::XMLoadFloat3(&offset_);

    DirectX::XMVECTOR scale, rotation, translation;
    DirectX::XMMatrixDecompose(&scale, &rotation, &translation, world_transform);

    DirectX::XMMATRIX rotation_matrix = DirectX::XMMatrixRotationQuaternion(rotation);
    DirectX::XMMATRIX translation_matrix = DirectX::XMMatrixTranslationFromVector(translation);
    DirectX::XMMATRIX transform_no_scale = rotation_matrix * translation_matrix;

    DirectX::XMVECTOR world_center = DirectX::XMVector3TransformCoord(offset_vec, transform_no_scale);

    DirectX::XMFLOAT3 center;
    DirectX::XMStoreFloat3(&center, world_center);
    return center;
}