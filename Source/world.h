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
            "基底クラスがGameObjectであるオブジェクトを<>で指定してください");
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

            // 累積補正ベクトルと衝突情報を保存
            DirectX::XMFLOAT3 total_correction = { 0.0f, 0.0f, 0.0f };
            DirectX::XMFLOAT3 average_normal = { 0.0f, 0.0f, 0.0f };
            int collision_count = 0;
            bool has_any_collision = false;

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
                            // 補正ベクトルを累積
                            total_correction.x += correction.x;
                            total_correction.y += correction.y;
                            total_correction.z += correction.z;

                            // 法線ベクトルを累積（後で速度反射に使用）
                            DirectX::XMVECTOR correction_vec = DirectX::XMLoadFloat3(&correction);
                            float correction_length = DirectX::XMVectorGetX(DirectX::XMVector3Length(correction_vec));

                            if (correction_length > 0.0001f) {
                                DirectX::XMVECTOR normal_vec = DirectX::XMVector3Normalize(correction_vec);
                                DirectX::XMFLOAT3 normal;
                                DirectX::XMStoreFloat3(&normal, normal_vec);

                                average_normal.x += normal.x;
                                average_normal.y += normal.y;
                                average_normal.z += normal.z;
                                collision_count++;
                            }

                            has_any_collision = true;
                        }
                    }
                }
            }

            // 衝突があった場合、累積した補正と速度反射を適用
            if (has_any_collision) {
                // 位置補正（累積した補正ベクトルを一度に適用）
                DirectX::XMFLOAT3 current_pos = obj_a->GetWorldPositionFloat3();
                obj_a->SetWorldPosition(
                    current_pos.x + total_correction.x,
                    current_pos.y + total_correction.y,
                    current_pos.z + total_correction.z);

                // 速度反射（平均法線ベクトルを使用）
                if (collision_count > 0) {
                    // 平均法線を計算
                    average_normal.x /= static_cast<float>(collision_count);
                    average_normal.y /= static_cast<float>(collision_count);
                    average_normal.z /= static_cast<float>(collision_count);

                    // 正規化
                    DirectX::XMVECTOR avg_normal_vec = DirectX::XMLoadFloat3(&average_normal);
                    float length = DirectX::XMVectorGetX(DirectX::XMVector3Length(avg_normal_vec));

                    if (length > 0.0001f) {
                        avg_normal_vec = DirectX::XMVector3Normalize(avg_normal_vec);
                        DirectX::XMStoreFloat3(&average_normal, avg_normal_vec);

                        // 速度反射処理
                        DirectX::XMFLOAT3 velocity = obj_a->GetVelocityFloat3();

                        // 法線方向の速度成分を計算
                        float dot = velocity.x * average_normal.x +
                            velocity.y * average_normal.y +
                            velocity.z * average_normal.z;

                        if (dot < 0.0f) {
                            // 反発係数を適用した反射
                            float bounciness = rb_a->GetBounciness();
                            float reflection_factor = -(1.0f + bounciness) * dot;

                            velocity.x += average_normal.x * reflection_factor;
                            velocity.y += average_normal.y * reflection_factor;
                            velocity.z += average_normal.z * reflection_factor;

                            // 摩擦の適用
                            float friction = rb_a->GetFriction();
                            if (friction > 0.0f) {
                                // 反射後の法線方向速度を再計算
                                float normal_velocity = velocity.x * average_normal.x +
                                    velocity.y * average_normal.y +
                                    velocity.z * average_normal.z;

                                // 接線方向の速度を計算
                                DirectX::XMFLOAT3 tangent_velocity = {
                                    velocity.x - average_normal.x * normal_velocity,
                                    velocity.y - average_normal.y * normal_velocity,
                                    velocity.z - average_normal.z * normal_velocity
                                };

                                // 摩擦係数を適用
                                float friction_factor = 1.0f - friction;
                                velocity.x = average_normal.x * normal_velocity + tangent_velocity.x * friction_factor;
                                velocity.y = average_normal.y * normal_velocity + tangent_velocity.y * friction_factor;
                                velocity.z = average_normal.z * normal_velocity + tangent_velocity.z * friction_factor;
                            }

                            // 速度が非常に小さい場合は0にする（安定化）
                            float speed_sq = velocity.x * velocity.x +
                                velocity.y * velocity.y +
                                velocity.z * velocity.z;
                            if (speed_sq < 0.0001f) {  // 速度の二乗が0.01以下（速度0.1以下）
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
    std::vector<std::unique_ptr<GameObject>> game_objects_;
    DirectX::XMFLOAT3 gravity_;
};

#endif  // WORLD_H_