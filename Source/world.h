#ifndef WORLD_H_
#define WORLD_H_

#include <vector>
#include <memory>
#include <algorithm>
#include <DirectXMath.h>
#include "game_object.h"
#include "collider.h"
#include "rigidbody.h"
#include "System/ModelRenderer.h"

class World {
public:
    static World& Instance() {
        static World instance;
        return instance;
    }

    template<typename T, typename... Args>
    T* CreateObject(Args&&... args) {
        static_assert(std::is_base_of<GameObject, T>::value,
            "基底クラスがGameObjectであるオブジェクトを<>で指定してください");
        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        T* ptr = obj.get();
        game_objects_.emplace_back(std::move(obj));
        return ptr;
    }

    template<typename... Args>
    GameObject* CreateObject(Args&&... args) {
        auto obj = std::make_unique<GameObject>(std::forward<Args>(args)...);
        GameObject* ptr = obj.get();
        game_objects_.emplace_back(std::move(obj));
        return ptr;
    }

    GameObject* CreateObject() {
        auto obj = std::make_unique<GameObject>();
        GameObject* ptr = obj.get();
        game_objects_.emplace_back(std::move(obj));
        return ptr;
    }

    void Update(float elapsed_time) {
        ApplyPhysics(elapsed_time);

        for (auto& obj : game_objects_) {
            if (obj && obj->IsActive()) {
                obj->Update(elapsed_time);
            }
        }

        DetectCollisions();
        RemoveInactiveObjects();
    }

    void Render(const RenderContext& rc, ModelRenderer* model_renderer) {
        for (auto& obj : game_objects_) {
            if (obj && obj->IsActive()) {
                obj->Render(rc, model_renderer);
            }
        }
        if (model_renderer) {
            model_renderer->Render(rc);
        }
    }

    void Clear() {
        game_objects_.clear();
    }

    size_t GetGameObjectCount() const {
        return game_objects_.size();
    }

    GameObject* GetGameObject(size_t index) {
        if (index < game_objects_.size()) {
            return game_objects_[index].get();
        }
        return nullptr;
    }

    void DestroyGameObject(GameObject* obj) {
        if (!obj) return;
        obj->Destroy();
    }

    void SetGravity(const DirectX::XMFLOAT3& gravity) {
        gravity_ = gravity;
    }

    const DirectX::XMFLOAT3& GetGravity() const {
        return gravity_;
    }

private:
    World() : gravity_(0.0f, -9.8f, 0.0f) {}
    ~World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    void RemoveInactiveObjects() {
        game_objects_.erase(
            std::remove_if(game_objects_.begin(), game_objects_.end(),
                [](const std::unique_ptr<GameObject>& obj) {
                    return !obj || !obj->IsActive();
                }),
            game_objects_.end());
    }

    void ApplyPhysics(float elapsed_time) {
        for (auto& obj : game_objects_) {
            if (!obj || !obj->IsActive()) continue;

            Rigidbody* rb = obj->GetRigidbody();
            if (rb && rb->IsEnabled()) {
                rb->ApplyGravity(elapsed_time, gravity_);
                rb->ApplyDrag(elapsed_time);
            }
        }
    }

    void DetectCollisions() {
        for (size_t i = 0; i < game_objects_.size(); ++i) {
            GameObject* obj_a = game_objects_[i].get();
            if (!obj_a || !obj_a->IsActive()) continue;

            Rigidbody* rb_a = obj_a->GetRigidbody();
            if (!rb_a || !rb_a->IsEnabled() || rb_a->IsKinematic()) continue;

            const std::vector<Collider*>& colliders_a = obj_a->GetColliders();
            if (colliders_a.empty()) continue;

            for (size_t j = i + 1; j < game_objects_.size(); ++j) {
                GameObject* obj_b = game_objects_[j].get();
                if (!obj_b || !obj_b->IsActive()) continue;

                const std::vector<Collider*>& colliders_b = obj_b->GetColliders();
                if (colliders_b.empty()) continue;

                // すべてのコライダーペアで衝突判定
                for (Collider* col_a : colliders_a) {
                    if (!col_a || !col_a->IsEnabled()) continue;

                    for (Collider* col_b : colliders_b) {
                        if (!col_b || !col_b->IsEnabled()) continue;

                        // 衝突判定と補正
                        DirectX::XMFLOAT3 correction;
                        GameObject* other = nullptr;
                        if (col_a->CheckRigidbodyCollision(col_b, correction, other)) {
                            // 位置補正
                            DirectX::XMFLOAT3 current_pos = obj_a->GetWorldPosition();
                            obj_a->SetWorldPosition(
                                current_pos.x + correction.x,
                                current_pos.y + correction.y,
                                current_pos.z + correction.z);

                            // 速度反射（反発係数と摩擦を考慮）
                            DirectX::XMFLOAT3 velocity = obj_a->GetVelocity();
                            DirectX::XMVECTOR correction_vec = DirectX::XMLoadFloat3(&correction);
                            float correction_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(correction_vec));

                            if (correction_length > 0.0001f) {
                                DirectX::XMVECTOR normal_vec = DirectX::XMVector3Normalize(correction_vec);
                                DirectX::XMFLOAT3 normal;
                                DirectX::XMStoreFloat3(&normal, normal_vec);

                                float dot = velocity.x * normal.x +
                                    velocity.y * normal.y +
                                    velocity.z * normal.z;

                                if (dot < 0.0f) {
                                    float bounciness = rb_a->GetBounciness();
                                    float reflection_factor = -(1.0f + bounciness) * dot;

                                    velocity.x += normal.x * reflection_factor;
                                    velocity.y += normal.y * reflection_factor;
                                    velocity.z += normal.z * reflection_factor;

                                    // 摩擦の適用
                                    float friction = rb_a->GetFriction();
                                    if (friction > 0.0f) {
                                        float normal_velocity = velocity.x * normal.x +
                                            velocity.y * normal.y +
                                            velocity.z * normal.z;

                                        DirectX::XMFLOAT3 tangent_velocity = {
                                            velocity.x - normal.x * normal_velocity,
                                            velocity.y - normal.y * normal_velocity,
                                            velocity.z - normal.z * normal_velocity };

                                        float friction_factor = 1.0f - friction;
                                        velocity.x = normal.x * normal_velocity + tangent_velocity.x * friction_factor;
                                        velocity.y = normal.y * normal_velocity + tangent_velocity.y * friction_factor;
                                        velocity.z = normal.z * normal_velocity + tangent_velocity.z * friction_factor;
                                    }

                                    // 速度が非常に小さい場合は0にする（安定化）
                                    float speed_sq = velocity.x * velocity.x +
                                        velocity.y * velocity.y +
                                        velocity.z * velocity.z;
                                    if (speed_sq < 0.0001f) {
                                        velocity = { 0.0f, 0.0f, 0.0f };
                                    }

                                    obj_a->SetVelocity(velocity);
                                    rb_a->ClampVelocity();
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<std::unique_ptr<GameObject>> game_objects_;
    DirectX::XMFLOAT3 gravity_;
};

#endif  // WORLD_H_