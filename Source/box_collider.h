#ifndef BOX_COLLIDER_H_
#define BOX_COLLIDER_H_

#include "collider.h"
#include <vector>

// ============================================================================
// Box Collider (OBB - Oriented Bounding Box)
// ============================================================================
class BoxCollider : public Collider {
public:
    BoxCollider(const DirectX::XMFLOAT3& size = { 1.0f, 1.0f, 1.0f }) 
        : Collider(ColliderType::kBox), size_(size) {}
    BoxCollider(float x, float y, float z) 
        : Collider(ColliderType::kBox), size_({ x, y, z }) {}

    bool CheckCollision(const Collider* other, GameObject*& out_other) const override;
    bool CheckRigidbodyCollision(const Collider* other, 
                                DirectX::XMFLOAT3& out_correction, 
                                GameObject*& out_other) const override;
    void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform, 
                         DirectX::XMFLOAT3& size) const override;

    const DirectX::XMFLOAT3& GetSize() const { return size_; }
    void SetSize(const DirectX::XMFLOAT3& size) { size_ = size; }
    void SetSize(float x, float y, float z) { size_ = { x, y, z }; }

    std::vector<DirectX::XMFLOAT3> GetWorldVertices() const;

private:
    DirectX::XMFLOAT3 size_;
};

#endif  // BOX_COLLIDER_H_
