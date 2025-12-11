#pragma once

#include <vector>
#include <memory>
#include <algorithm>
#include "game_object.h"
#include "System/ModelRenderer.h"

class World {
public:
    static World& Instance() {
        static World instance;
        return instance;
    }

    template<typename T, typename... Args>
    T* CreateObject(Args&&... args) {
        static_assert(std::is_base_of<GameObject, T>::value, "基底クラスがGameObjectであるオブジェクトを<>で指定してください");
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
        for (auto& obj : game_objects_) {
            if (obj && obj->IsActive()) {
                obj->Update(elapsed_time);
            }
        }

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

private:
    World() = default;
    ~World() = default;
    World(const World&) = delete;
    World& operator=(const World&) = delete;

    void RemoveInactiveObjects() {
        game_objects_.erase(
            std::remove_if(game_objects_.begin(), game_objects_.end(),
                [](const std::unique_ptr<GameObject>& obj) {
                    return !obj || !obj->IsActive();
                }),
            game_objects_.end()
        );
    }

    std::vector<std::unique_ptr<GameObject>> game_objects_;
};