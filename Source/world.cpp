#include "world.h"

World& World::Instance() {
    static World instance;
    return instance;
}

World::World() : gravity_(0.0f, -9.8f, 0.0f) {}

GameObject* World::CreateObject(
    const char* model_filepath,
    const DirectX::XMFLOAT3& pos,
    const DirectX::XMFLOAT3& rotation,
    const DirectX::XMFLOAT3& scale) {
    auto obj = std::make_unique<GameObject>(model_filepath, pos, rotation, scale);
    GameObject* ptr = obj.get();
    game_objects_.emplace_back(std::move(obj));
    return ptr;
}

void World::Update(float elapsed_time) {
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

void World::Render(const RenderContext& rc, ModelRenderer* model_renderer) {
    for (auto& obj : game_objects_) {
        if (obj && obj->IsActive()) {
            obj->Render(rc, model_renderer);
        }
    }

    if (model_renderer) {
        model_renderer->Render(rc);
    }
}

void World::Clear() {
    game_objects_.clear();
}

size_t World::GetGameObjectCount() const {
    return game_objects_.size();
}

GameObject* World::GetGameObject(size_t index) {
    if (index < game_objects_.size()) {
        return game_objects_[index].get();
    }
    return nullptr;
}

void World::DestroyGameObject(GameObject* obj) {
    if (!obj) return;
    obj->Destroy();
}

void World::SetGravity(const DirectX::XMFLOAT3& gravity) {
    gravity_ = gravity;
}

const DirectX::XMFLOAT3& World::GetGravity() const {
    return gravity_;
}

void World::SetDebugDrawColliders(bool enable) {
    debug_draw_colliders_ = enable;
}

bool World::GetDebugDrawColliders() const {
    return debug_draw_colliders_;
}

void World::DrawDebugPrimitives(ShapeRenderer* shape_renderer) {
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

            case ColliderType::kAABB:
                shape_renderer->DrawBox(position, { 0.0f, 0.0f, 0.0f }, size, color);
                break;

            case ColliderType::kCapsule:
                shape_renderer->DrawCapsule(transform, size.x, size.y, color);
                break;
            }
        }
    }
}

void World::RemoveInactiveObjects() {
    game_objects_.erase(
        std::remove_if(game_objects_.begin(), game_objects_.end(),
            [](const std::unique_ptr<GameObject>& obj) {
                return !obj || !obj->IsActive();
            }),
        game_objects_.end());
}

void World::ApplyPhysics(float elapsed_time) {
    for (auto& obj : game_objects_) {
        if (!obj || !obj->IsActive()) continue;

        Rigidbody* rb = obj->GetRigidbody();
        if (rb && rb->IsEnabled()) {
            rb->ApplyGravity(elapsed_time, gravity_);
            rb->ApplyDrag(elapsed_time);
        }
    }
}

void World::DetectCollisions() {
    std::vector<CollisionPair> current_collisions;
    current_collisions.reserve(previous_collisions_.size() + 16);

    const size_t object_count = game_objects_.size();

    for (size_t i = 0; i < object_count; ++i) {
        GameObject* obj_a = game_objects_[i].get();
        if (!obj_a || !obj_a->IsActive()) continue;

        const std::vector<Collider*>& colliders_a = obj_a->GetColliders();
        if (colliders_a.empty()) continue;

        Rigidbody* rb_a = obj_a->GetRigidbody();
        const bool has_rigidbody = rb_a && rb_a->IsEnabled() && !rb_a->IsKinematic();

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

            if (has_rigidbody) {
                for (Collider* col_a : colliders_a) {
                    if (!col_a || !col_a->IsEnabled()) continue;

                    for (Collider* col_b : colliders_b) {
                        if (!col_b || !col_b->IsEnabled()) continue;

                        DirectX::XMFLOAT3 correction;
                        GameObject* other = nullptr;
                        if (col_a->CheckRigidbodyCollision(col_b, correction, other)) {
                            total_correction.x += correction.x;
                            total_correction.y += correction.y;
                            total_correction.z += correction.z;

                            DirectX::XMVECTOR correction_vec = DirectX::XMLoadFloat3(&correction);
                            DirectX::XMVECTOR length_sq_vec = DirectX::XMVector3LengthSq(correction_vec);
                            float length_sq = DirectX::XMVectorGetX(length_sq_vec);

                            if (length_sq > 0.00000001f) {
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
                for (Collider* col_a : colliders_a) {
                    if (!col_a || !col_a->IsEnabled()) continue;

                    for (Collider* col_b : colliders_b) {
                        if (!col_b || !col_b->IsEnabled()) continue;

                        GameObject* other = nullptr;
                        if (col_a->CheckCollision(col_b, other)) {
                            pair_collided = true;
                            break;
                        }
                    }
                    if (pair_collided) break;
                }
            }

            if (pair_collided) {
                CollisionPair pair{ obj_a, obj_b };
                current_collisions.push_back(pair);

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

        if (has_rigidbody && has_any_collision) {
            DirectX::XMVECTOR current_pos = obj_a->GetWorldPositionVector();
            DirectX::XMVECTOR correction_vec = DirectX::XMLoadFloat3(&total_correction);
            DirectX::XMVECTOR new_pos = DirectX::XMVectorAdd(current_pos, correction_vec);
            obj_a->SetWorldPositionVector(new_pos);

            if (collision_count > 0) {
                const float inv_count = 1.0f / static_cast<float>(collision_count);
                DirectX::XMVECTOR avg_normal_vec = DirectX::XMLoadFloat3(&average_normal);
                avg_normal_vec = DirectX::XMVectorScale(avg_normal_vec, inv_count);

                DirectX::XMVECTOR length_sq_vec = DirectX::XMVector3LengthSq(avg_normal_vec);
                float length_sq = DirectX::XMVectorGetX(length_sq_vec);

                if (length_sq > 0.00000001f) {
                    avg_normal_vec = DirectX::XMVector3Normalize(avg_normal_vec);

                    DirectX::XMVECTOR velocity_vec = DirectX::XMLoadFloat3(&obj_a->GetVelocityFloat3());

                    DirectX::XMVECTOR dot_vec = DirectX::XMVector3Dot(velocity_vec, avg_normal_vec);
                    float dot = DirectX::XMVectorGetX(dot_vec);

                    if (dot < 0.0f) {
                        const float bounciness = rb_a->GetBounciness();
                        const float reflection_factor = -(1.0f + bounciness) * dot;

                        DirectX::XMVECTOR reflection = DirectX::XMVectorScale(avg_normal_vec, reflection_factor);
                        velocity_vec = DirectX::XMVectorAdd(velocity_vec, reflection);

                        const float friction = rb_a->GetFriction();
                        if (friction > 0.0f) {
                            DirectX::XMVECTOR normal_dot = DirectX::XMVector3Dot(velocity_vec, avg_normal_vec);
                            DirectX::XMVECTOR normal_velocity_vec = DirectX::XMVectorScale(
                                avg_normal_vec,
                                DirectX::XMVectorGetX(normal_dot)
                            );

                            DirectX::XMVECTOR tangent_velocity = DirectX::XMVectorSubtract(
                                velocity_vec,
                                normal_velocity_vec
                            );

                            const float friction_factor = 1.0f - friction;
                            tangent_velocity = DirectX::XMVectorScale(tangent_velocity, friction_factor);
                            velocity_vec = DirectX::XMVectorAdd(normal_velocity_vec, tangent_velocity);
                        }

                        DirectX::XMVECTOR speed_sq_vec = DirectX::XMVector3LengthSq(velocity_vec);
                        if (DirectX::XMVectorGetX(speed_sq_vec) < 0.0001f) {
                            velocity_vec = DirectX::XMVectorZero();
                        }

                        obj_a->SetVelocityVector(velocity_vec);
                        rb_a->ClampVelocity();
                    }
                }
            }
        }
    }

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

bool World::CollisionPair::operator==(const CollisionPair& other) const {
    return (obj_a == other.obj_a && obj_b == other.obj_b) ||
        (obj_a == other.obj_b && obj_b == other.obj_a);
}