#ifndef COLLIDER_H_
#define COLLIDER_H_

#include <DirectXMath.h>

class GameObject;

enum class ColliderType {
    kSphere,
    kBox,
    kCapsule,
    kAABB
};

// ============================================================================
// Base Collider Class
// ============================================================================
class Collider {
public:
    Collider(ColliderType type) : type_(type), owner_(nullptr), enabled_(true) {}
    virtual ~Collider() = default;

    virtual bool CheckCollision(const Collider* other, GameObject*& out_other) const = 0;
    virtual bool CheckRigidbodyCollision(const Collider* other, 
                                        DirectX::XMFLOAT3& out_correction, 
                                        GameObject*& out_other) const = 0;

    virtual void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform, 
                                  DirectX::XMFLOAT3& size) const = 0;

    // Getters/Setters
    ColliderType GetType() const { return type_; }
    GameObject* GetOwner() const { return owner_; }
    void SetOwner(GameObject* owner) { owner_ = owner; }
    
    bool IsEnabled() const { return enabled_; }
    void SetEnabled(bool enabled) { enabled_ = enabled; }

    const DirectX::XMFLOAT3& GetOffset() const { return offset_; }
    void SetOffset(const DirectX::XMFLOAT3& offset) { offset_ = offset; }
    void SetOffset(float x, float y, float z) { offset_ = { x, y, z }; }

    DirectX::XMFLOAT3 GetWorldCenter() const;

protected:
    ColliderType type_;
    GameObject* owner_;
    DirectX::XMFLOAT3 offset_ = { 0.0f, 0.0f, 0.0f };
    bool enabled_;
};

#endif  // COLLIDER_H_
