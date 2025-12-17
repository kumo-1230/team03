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
            "基底クラスがGameObjectであるクラスを<>で指定してください");
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

    // モデルパスと初期位置を指定して作成
    GameObject* CreateObject(const char* model_path, const DirectX::XMFLOAT3& position) {
        auto obj = std::make_unique<GameObject>();
        GameObject* ptr = obj.get();
        ptr->SetModel(model_path);
        ptr->SetLocalPosition(position);
        game_objects_.emplace_back(std::move(obj));
        return ptr;
    }

    GameObject* CreateObject(const char* model_path, float x, float y, float z) {
        return CreateObject(model_path, DirectX::XMFLOAT3(x, y, z));
    }

    GameObject* CreateObject(const char* model_path, DirectX::FXMVECTOR position) {
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, position);
        return CreateObject(model_path, pos);
    }

    // カスタム型と初期位置を指定して作成
    template<typename T>
    T* CreateObject(const DirectX::XMFLOAT3& position) {
        static_assert(std::is_base_of<GameObject, T>::value,
            "基底クラスがGameObjectであるクラスを<>で指定してください");
        auto obj = std::make_unique<T>();
        T* ptr = obj.get();
        ptr->SetLocalPosition(position);
        game_objects_.emplace_back(std::move(obj));
        return ptr;
    }

    template<typename T>
    T* CreateObject(float x, float y, float z) {
        return CreateObject<T>(DirectX::XMFLOAT3(x, y, z));
    }

    template<typename T>
    T* CreateObject(DirectX::FXMVECTOR position) {
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, position);
        return CreateObject<T>(pos);
    }

    // カスタム型、モデルパス、初期位置を指定して作成
    template<typename T>
    T* CreateObject(const char* model_path, const DirectX::XMFLOAT3& position) {
        static_assert(std::is_base_of<GameObject, T>::value,
            "基底クラスがGameObjectであるオブジェクトを<>で指定してください");
        auto obj = std::make_unique<T>();
        T* ptr = obj.get();
        ptr->SetModel(model_path);
        ptr->SetLocalPosition(position);
        game_objects_.emplace_back(std::move(obj));
        return ptr;
    }

    template<typename T>
    T* CreateObject(const char* model_path, float x, float y, float z) {
        return CreateObject<T>(model_path, DirectX::XMFLOAT3(x, y, z));
    }

    template<typename T>
    T* CreateObject(const char* model_path, DirectX::FXMVECTOR position) {
        DirectX::XMFLOAT3 pos;
        DirectX::XMStoreFloat3(&pos, position);
        return CreateObject<T>(model_path, pos);
    }

    void Update(float elapsed_time) {
        ApplyPhysics(elapsed_time);

        for (auto& obj : game_objects_) {
            if (obj && obj->IsActive()) {
                obj->Update(elapsed_time);
                obj->UpdatePosition(elapsed_time);
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

    void SetDebugDrawColliders(bool enable) { debug_draw_colliders_ = enable; }
    bool GetDebugDrawColliders() const { return debug_draw_colliders_; }

    void DrawDebugPrimitives(ShapeRenderer* shape_renderer) {
        if (!debug_draw_colliders_ || !shape_renderer) return;

        for (const auto& obj : game_objects_) {
            if (!obj || !obj->IsActive()) continue;

            const std::vector<Collider*>& colliders = obj->GetColliders();
            if (colliders.empty()) continue;

            const DirectX::XMFLOAT4X4& world_transform = obj->GetWorldTransformFloat4X4();
            const DirectX::XMFLOAT3& obj_angle = obj->GetAngle();

            for (Collider* collider : colliders) {
                if (!collider || !collider->IsEnabled()) continue;

                DirectX::XMFLOAT4X4 transform;
                DirectX::XMFLOAT3 size;
                collider->GetDebugDrawInfo(transform, size);

                DirectX::XMFLOAT3 position = {
                    transform._41,
                    transform._42,
                    transform._43
                };

                constexpr DirectX::XMFLOAT4 color = { 0.0f, 1.0f, 0.0f, 1.0f };

                switch (collider->GetType()) {
                case ColliderType::kSphere:
                    shape_renderer->DrawSphere(position, size.x, color);
                    break;

                case ColliderType::kBox:
                    shape_renderer->DrawBox(position, obj_angle, size, color);
                    break;

                case ColliderType::kAabb:
                    shape_renderer->DrawBox(position, { 0.0f, 0.0f, 0.0f }, size, color);
                    break;

                case ColliderType::kCapsule:
                    shape_renderer->DrawCapsule(transform, size.x, size.y, color);
                    break;
                }
            }
        }
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
        std::vector<CollisionPair> current_collisions;
        current_collisions.reserve(previous_collisions_.size() + 16); // メモリ事前確保

        const size_t object_count = game_objects_.size();

        for (size_t i = 0; i < object_count; ++i) {
            GameObject* obj_a = game_objects_[i].get();
            if (!obj_a || !obj_a->IsActive()) continue;

            const std::vector<Collider*>& colliders_a = obj_a->GetColliders();
            if (colliders_a.empty()) continue;

            Rigidbody* rb_a = obj_a->GetRigidbody();
            const bool has_rigidbody = rb_a && rb_a->IsEnabled() && !rb_a->IsKinematic();

            // Rigidbody用変数（条件付きで初期化）
            DirectX::XMFLOAT3 total_correction = { 0.0f, 0.0f, 0.0f };
            DirectX::XMFLOAT3 average_normal = { 0.0f, 0.0f, 0.0f };
            int collision_count = 0;
            bool has_any_collision = false;

            for (size_t j = i + 1; j < object_count; ++j) {
                GameObject* obj_b = game_objects_[j].get();
                if (!obj_b || !obj_b->IsActive()) continue;

                const std::vector<Collider*>& colliders_b = obj_b->GetColliders();
                if (colliders_b.empty()) continue;

                bool pair_collided = false;

                // コライダーペアの衝突判定
                if (has_rigidbody) {
                    // Rigidbody用：物理演算付き
                    for (Collider* col_a : colliders_a) {
                        if (!col_a || !col_a->IsEnabled()) continue;

                        for (Collider* col_b : colliders_b) {
                            if (!col_b || !col_b->IsEnabled()) continue;

                            DirectX::XMFLOAT3 correction;
                            GameObject* other = nullptr;
                            if (col_a->CheckRigidbodyCollision(col_b, correction, other)) {
                                // 補正ベクトル累積
                                total_correction.x += correction.x;
                                total_correction.y += correction.y;
                                total_correction.z += correction.z;

                                // 法線ベクトル累積（SIMD最適化）
                                DirectX::XMVECTOR correction_vec = DirectX::XMLoadFloat3(&correction);
                                DirectX::XMVECTOR length_sq_vec = DirectX::XMVector3LengthSq(correction_vec);
                                float length_sq = DirectX::XMVectorGetX(length_sq_vec);

                                if (length_sq > 0.00000001f) { // 0.0001f の二乗
                                    DirectX::XMVECTOR normal_vec = DirectX::XMVector3Normalize(correction_vec);
                                    DirectX::XMFLOAT3 normal;
                                    DirectX::XMStoreFloat3(&normal, normal_vec);

                                    average_normal.x += normal.x;
                                    average_normal.y += normal.y;
                                    average_normal.z += normal.z;
                                    collision_count++;
                                }

                                pair_collided = true;
                                has_any_collision = true;
                            }
                        }
                    }
                }
                else {
                    // Rigidbodyなし：単純判定のみ
                    for (Collider* col_a : colliders_a) {
                        if (!col_a || !col_a->IsEnabled()) continue;

                        for (Collider* col_b : colliders_b) {
                            if (!col_b || !col_b->IsEnabled()) continue;

                            GameObject* other = nullptr;
                            if (col_a->CheckCollision(col_b, other)) {
                                pair_collided = true;
                                break; // 1つでも衝突したらペア判定終了
                            }
                        }
                        if (pair_collided) break; // 早期脱出
                    }
                }

                // コールバック処理
                if (pair_collided) {
                    CollisionPair pair{ obj_a, obj_b };
                    current_collisions.push_back(pair);

                    // 前フレーム衝突チェック（線形探索を最適化）
                    const bool was_colliding = std::find(
                        previous_collisions_.begin(),
                        previous_collisions_.end(),
                        pair
                    ) != previous_collisions_.end();

                    if (was_colliding) {
                        obj_a->OnCollisionStay(obj_b);
                        obj_b->OnCollisionStay(obj_a);
                    }
                    else {
                        obj_a->OnCollisionEnter(obj_b);
                        obj_b->OnCollisionEnter(obj_a);
                    }
                }
            }

            // Rigidbody物理演算適用
            if (has_rigidbody && has_any_collision) {
                // 位置補正
                DirectX::XMVECTOR current_pos = obj_a->GetWorldPositionVector();
                DirectX::XMVECTOR correction_vec = DirectX::XMLoadFloat3(&total_correction);
                DirectX::XMVECTOR new_pos = DirectX::XMVectorAdd(current_pos, correction_vec);
                obj_a->SetWorldPosition(new_pos);

                // 速度反射
                if (collision_count > 0) {
                    // 平均法線計算（SIMD化）
                    const float inv_count = 1.0f / static_cast<float>(collision_count);
                    DirectX::XMVECTOR avg_normal_vec = DirectX::XMLoadFloat3(&average_normal);
                    avg_normal_vec = DirectX::XMVectorScale(avg_normal_vec, inv_count);

                    DirectX::XMVECTOR length_sq_vec = DirectX::XMVector3LengthSq(avg_normal_vec);
                    float length_sq = DirectX::XMVectorGetX(length_sq_vec);

                    if (length_sq > 0.00000001f) {
                        avg_normal_vec = DirectX::XMVector3Normalize(avg_normal_vec);

                        // 速度ベクトル取得
                        DirectX::XMVECTOR velocity_vec = DirectX::XMLoadFloat3(&obj_a->GetVelocityFloat3());

                        // 法線方向の速度成分（内積）
                        DirectX::XMVECTOR dot_vec = DirectX::XMVector3Dot(velocity_vec, avg_normal_vec);
                        float dot = DirectX::XMVectorGetX(dot_vec);

                        if (dot < 0.0f) {
                            const float bounciness = rb_a->GetBounciness();
                            const float reflection_factor = -(1.0f + bounciness) * dot;

                            // 反射ベクトル計算（SIMD）
                            DirectX::XMVECTOR reflection = DirectX::XMVectorScale(avg_normal_vec, reflection_factor);
                            velocity_vec = DirectX::XMVectorAdd(velocity_vec, reflection);

                            // 摩擦適用
                            const float friction = rb_a->GetFriction();
                            if (friction > 0.0f) {
                                // 法線速度再計算
                                DirectX::XMVECTOR normal_dot = DirectX::XMVector3Dot(velocity_vec, avg_normal_vec);
                                DirectX::XMVECTOR normal_velocity_vec = DirectX::XMVectorScale(
                                    avg_normal_vec,
                                    DirectX::XMVectorGetX(normal_dot)
                                );

                                // 接線速度
                                DirectX::XMVECTOR tangent_velocity = DirectX::XMVectorSubtract(
                                    velocity_vec,
                                    normal_velocity_vec
                                );

                                // 摩擦係数適用
                                const float friction_factor = 1.0f - friction;
                                tangent_velocity = DirectX::XMVectorScale(tangent_velocity, friction_factor);
                                velocity_vec = DirectX::XMVectorAdd(normal_velocity_vec, tangent_velocity);
                            }

                            // 速度安定化（二乗比較で平方根回避）
                            DirectX::XMVECTOR speed_sq_vec = DirectX::XMVector3LengthSq(velocity_vec);
                            if (DirectX::XMVectorGetX(speed_sq_vec) < 0.0001f) {
                                velocity_vec = DirectX::XMVectorZero();
                            }

                            obj_a->SetVelocity(velocity_vec);
                            rb_a->ClampVelocity();
                        }
                    }
                }
            }
        }

        // OnCollisionExit処理
        for (const auto& prev_pair : previous_collisions_) {
            const bool still_colliding = std::find(
                current_collisions.begin(),
                current_collisions.end(),
                prev_pair
            ) != current_collisions.end();

            if (!still_colliding) {
                if (prev_pair.obj_a && prev_pair.obj_a->IsActive() &&
                    prev_pair.obj_b && prev_pair.obj_b->IsActive()) {
                    prev_pair.obj_a->OnCollisionExit(prev_pair.obj_b);
                    prev_pair.obj_b->OnCollisionExit(prev_pair.obj_a);
                }
            }
        }

        previous_collisions_ = std::move(current_collisions);
    }

    struct CollisionPair {
        GameObject* obj_a;
        GameObject* obj_b;

        bool operator==(const CollisionPair& other) const {
            return (obj_a == other.obj_a && obj_b == other.obj_b) ||
                (obj_a == other.obj_b && obj_b == other.obj_a);
        }
    };

	bool debug_draw_colliders_ = _DEBUG;
    std::vector<CollisionPair> previous_collisions_;
    std::vector<std::unique_ptr<GameObject>> game_objects_;
    DirectX::XMFLOAT3 gravity_;
};

#endif  // WORLD_H_