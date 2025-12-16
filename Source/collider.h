#ifndef COLLIDER_H_
#define COLLIDER_H_

#include <vector>
#include <DirectXMath.h>

class GameObject;

// コライダーの種類を表す列挙型
enum class ColliderType {
    kSphere,   // 球体コライダー
    kBox,      // ボックスコライダー
    kCapsule   // カプセルコライダー
};

// コライダーの基底クラス
class Collider {
public:
    Collider() = default;
    virtual ~Collider() = default;

    // コライダーの種類を取得
    virtual ColliderType GetType() const = 0;

    // 衝突判定（シンプル版）
    virtual bool CheckCollision(const Collider* other, GameObject*& out_other) const = 0;

    // 衝突判定（剛体用、補正ベクトル付き）
    virtual bool CheckRigidbodyCollision(
        const Collider* other,
        DirectX::XMFLOAT3& out_correction,
        GameObject*& out_other) const = 0;

    // ワールド座標での中心位置を取得
    DirectX::XMFLOAT3 GetWorldCenter() const;

    // オーナーのGameObjectを設定
    void SetOwner(GameObject* owner) { owner_ = owner; }

    // オーナーのGameObjectを取得
    GameObject* GetOwner() const { return owner_; }

    // オフセット位置の取得・設定
    const DirectX::XMFLOAT3& GetOffset() const { return offset_; }
    void SetOffset(const DirectX::XMFLOAT3& offset) { offset_ = offset; }
    void SetOffset(float x, float y, float z) { offset_ = { x, y, z }; }

    // 有効/無効の取得・設定
    bool IsEnabled() const { return is_enabled_; }
    void SetEnabled(bool enabled) { is_enabled_ = enabled; }

    virtual void GetDebugDrawInfo(
        DirectX::XMFLOAT4X4& transform,
        DirectX::XMFLOAT3& size) const = 0;

protected:
    GameObject* owner_ = nullptr;                      // 所有者
    DirectX::XMFLOAT3 offset_ = { 0.0f, 0.0f, 0.0f };   // ローカル座標でのオフセット
    bool is_enabled_ = true;                           // 有効フラグ
};

// 球体コライダー
class SphereCollider : public Collider {
public:
    explicit SphereCollider(float radius = 1.0f) : radius_(radius) {}
    ~SphereCollider() override = default;

    ColliderType GetType() const override { return ColliderType::kSphere; }

    bool CheckCollision(const Collider* other, GameObject*& out_other) const override;

    bool CheckRigidbodyCollision(
        const Collider* other,
        DirectX::XMFLOAT3& out_correction,
        GameObject*& out_other) const override;

    // 半径の取得・設定
    float GetRadius() const { return radius_; }
    void SetRadius(float radius) { radius_ = radius; }

    void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform, DirectX::XMFLOAT3& size) const;

private:
    float radius_;  // 球体の半径
};

// ボックスコライダー
class BoxCollider : public Collider {
public:
    explicit BoxCollider(const DirectX::XMFLOAT3& size = { 1.0f, 1.0f, 1.0f })
        : size_(size) {
    }
    ~BoxCollider() override = default;

    ColliderType GetType() const override { return ColliderType::kBox; }

    bool CheckCollision(const Collider* other, GameObject*& out_other) const override;

    bool CheckRigidbodyCollision(
        const Collider* other,
        DirectX::XMFLOAT3& out_correction,
        GameObject*& out_other) const override;

    const DirectX::XMFLOAT3& GetSize() const { return size_; }
    void SetSize(const DirectX::XMFLOAT3& size) { size_ = size; }
    void SetSize(float x, float y, float z) { size_ = { x, y, z }; }

    std::vector<DirectX::XMFLOAT3> GetWorldVertices() const;

    void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform, DirectX::XMFLOAT3& size) const;

private:
    DirectX::XMFLOAT3 size_;  // ボックスのサイズ（幅、高さ、奥行き）
};

// カプセルコライダー
class CapsuleCollider : public Collider {
public:
    explicit CapsuleCollider(float height = 2.0f, float radius = 0.5f)
        : height_(height), radius_(radius) {
    }
    ~CapsuleCollider() override = default;

    ColliderType GetType() const override { return ColliderType::kCapsule; }

    bool CheckCollision(const Collider* other, GameObject*& out_other) const override;

    bool CheckRigidbodyCollision(
        const Collider* other,
        DirectX::XMFLOAT3& out_correction,
        GameObject*& out_other) const override;

    // 高さの取得・設定
    float GetHeight() const { return height_; }
    void SetHeight(float height) { height_ = height; }

    // 半径の取得・設定
    float GetRadius() const { return radius_; }
    void SetRadius(float radius) { radius_ = radius; }

    // カプセルの中心線（セグメント）の開始点と終了点を取得
    void GetCapsuleSegment(DirectX::XMFLOAT3& start, DirectX::XMFLOAT3& end) const;

    void GetDebugDrawInfo(DirectX::XMFLOAT4X4& transform, DirectX::XMFLOAT3& size) const;

private:
    float height_;  // カプセルの高さ（全体の高さ）
    float radius_;  // カプセルの半径
};

#endif  // COLLIDER_H_