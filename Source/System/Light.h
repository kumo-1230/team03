#pragma once

#include <DirectXMath.h>
#include <vector>
#include <algorithm>
#include "imgui.h"
#include "ShapeRenderer.h"

struct DirectionalLight {
    DirectX::XMFLOAT3 direction = { 0, -1, 0 };
    DirectX::XMFLOAT3 color = { 1, 1, 1 };
};

struct PointLight {
    DirectX::XMFLOAT3 position = { 0, 0, 0 };
    float range = 10.0f;
    DirectX::XMFLOAT3 color = { 1, 1, 1 };
    float intensity = 1.0f;
    int priority = 0;
    bool enabled = true;
};

struct SpotLight {
    DirectX::XMFLOAT3 position = { 0, 0, 0 };
    float range = 10.0f;
    DirectX::XMFLOAT3 direction = { 0, -1, 0 };
    float innerConeAngle = 30.0f;
    DirectX::XMFLOAT3 color = { 1, 1, 1 };
    float outerConeAngle = 45.0f;
    float intensity = 1.0f;
    int priority = 0;
    bool enabled = true;
    float pad = 0.0f;
};

class LightManager {
public:
    void SetDirectionalLight(DirectionalLight& light) {
        directionalLight = light;
    }

    const DirectionalLight& GetDirectionalLight() const {
        return directionalLight;
    }

    int AddPointLight(const PointLight& light) {
        pointLights.push_back(light);
        return static_cast<int>(pointLights.size()) - 1;
    }

    void RemovePointLight(int index) {
        if (index >= 0 && index < static_cast<int>(pointLights.size())) {
            pointLights.erase(pointLights.begin() + index);
            if (playerLightIndex == index) {
                playerLightIndex = -1;
            }
            else if (playerLightIndex > index) {
                playerLightIndex--;
            }
        }
    }

    void UpdatePointLight(int index, const PointLight& light) {
        if (index >= 0 && index < static_cast<int>(pointLights.size())) {
            pointLights[index] = light;
        }
    }

    int AddSpotLight(const SpotLight& light) {
        spotLights.push_back(light);
        return static_cast<int>(spotLights.size()) - 1;
    }

    void RemoveSpotLight(int index) {
        if (index >= 0 && index < static_cast<int>(spotLights.size())) {
            spotLights.erase(spotLights.begin() + index);
            if (playerSpotLightIndex == index) {
                playerSpotLightIndex = -1;
            }
            else if (playerSpotLightIndex > index) {
                playerSpotLightIndex--;
            }
        }
    }

    void UpdateSpotLight(int index, const SpotLight& light) {
        if (index >= 0 && index < static_cast<int>(spotLights.size())) {
            spotLights[index] = light;
        }
    }

    void SetPlayerLight(const DirectX::XMFLOAT3& position, float range = 15.0f,
        const DirectX::XMFLOAT3& color = { 1.0f, 0.9f, 0.8f },
        float intensity = 5.0f) {
        PointLight light;
        light.position = position;
        light.range = range;
        light.color = color;
        light.intensity = intensity;
        light.priority = 1000;
        light.enabled = true;

        if (playerLightIndex == -1) {
            playerLightIndex = AddPointLight(light);
        }
        else {
            UpdatePointLight(playerLightIndex, light);
        }
    }

    void SetPlayerSpotLight(const DirectX::XMFLOAT3& position, const DirectX::XMFLOAT3& direction,
        float range = 20.0f, float innerAngle = 25.0f, float outerAngle = 40.0f,
        const DirectX::XMFLOAT3& color = { 1.0f, 0.95f, 0.85f },
        float intensity = 8.0f) {
        SpotLight light = {};
        light.position = position;
        light.direction = direction;
        light.range = range;
        light.innerConeAngle = innerAngle;
        light.outerConeAngle = outerAngle;
        light.color = color;
        light.intensity = intensity;
        light.priority = 1000;
        light.enabled = true;
        light.pad = 0.0f;

        if (playerSpotLightIndex == -1) {
            playerSpotLightIndex = AddSpotLight(light);
        }
        else {
            UpdateSpotLight(playerSpotLightIndex, light);
        }
    }

    void SetPlayerLightEnabled(bool enabled) {
        if (playerLightIndex >= 0 && playerLightIndex < static_cast<int>(pointLights.size())) {
            pointLights[playerLightIndex].enabled = enabled;
        }
    }

    void SetPlayerSpotLightEnabled(bool enabled) {
        if (playerSpotLightIndex >= 0 && playerSpotLightIndex < static_cast<int>(spotLights.size())) {
            spotLights[playerSpotLightIndex].enabled = enabled;
        }
    }

    std::vector<PointLight> GetNearestPointLights(const DirectX::XMFLOAT3& position, int maxCount = 8) const {
        struct LightDistance {
            float distance;
            int index;
        };

        std::vector<LightDistance> distances;
        distances.reserve(pointLights.size());

        for (size_t i = 0; i < pointLights.size(); ++i) {
            if (!pointLights[i].enabled) continue;

            DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
            DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&pointLights[i].position);
            DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(lightPos, pos);
            float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));

            if (distance > pointLights[i].range * 1.5f) {
                continue;
            }

            float priorityWeight = static_cast<float>(pointLights[i].priority) * 100.0f;
            float effectiveDistance = distance - priorityWeight;

            distances.push_back({ effectiveDistance, static_cast<int>(i) });
        }

        std::sort(distances.begin(), distances.end(),
            [](const LightDistance& a, const LightDistance& b) {
                return a.distance < b.distance;
            });

        std::vector<PointLight> result;
        const int count = (maxCount < static_cast<int>(distances.size())) ? maxCount : static_cast<int>(distances.size());
        result.reserve(count);

        for (int i = 0; i < count; ++i) {
            result.push_back(pointLights[distances[i].index]);
        }

        return result;
    }

    std::vector<SpotLight> GetNearestSpotLights(const DirectX::XMFLOAT3& position, int maxCount = 4) const {
        struct LightDistance {
            float distance;
            int index;
        };

        std::vector<LightDistance> distances;
        distances.reserve(spotLights.size());

        for (size_t i = 0; i < spotLights.size(); ++i) {
            if (!spotLights[i].enabled) continue;

            DirectX::XMVECTOR pos = DirectX::XMLoadFloat3(&position);
            DirectX::XMVECTOR lightPos = DirectX::XMLoadFloat3(&spotLights[i].position);
            DirectX::XMVECTOR diff = DirectX::XMVectorSubtract(lightPos, pos);
            float distance = DirectX::XMVectorGetX(DirectX::XMVector3Length(diff));

            if (distance > spotLights[i].range * 1.5f) {
                continue;
            }

            float priorityWeight = static_cast<float>(spotLights[i].priority) * 100.0f;
            float effectiveDistance = distance - priorityWeight;

            distances.push_back({ effectiveDistance, static_cast<int>(i) });
        }

        std::sort(distances.begin(), distances.end(),
            [](const LightDistance& a, const LightDistance& b) {
                return a.distance < b.distance;
            });

        std::vector<SpotLight> result;
        const int count = (maxCount < static_cast<int>(distances.size())) ? maxCount : static_cast<int>(distances.size());
        result.reserve(count);

        for (int i = 0; i < count; ++i) {
            result.push_back(spotLights[distances[i].index]);
        }

        return result;
    }

    const std::vector<PointLight>& GetAllPointLights() const {
        return pointLights;
    }

    const std::vector<SpotLight>& GetAllSpotLights() const {
        return spotLights;
    }

    void ClearPointLights() {
        pointLights.clear();
        playerLightIndex = -1;
    }

    void ClearSpotLights() {
        spotLights.clear();
        playerSpotLightIndex = -1;
    }

    void DrawDebugSpheres(ShapeRenderer* shapeRenderer) const {
        for (const auto& light : pointLights) {
            if (!light.enabled) continue;

            DirectX::XMFLOAT4 rangeColor(light.color.x, light.color.y, light.color.z, 0.3f);
            shapeRenderer->DrawSphere(light.position, light.range, rangeColor);

            DirectX::XMFLOAT4 markerColor(1.0f, 1.0f, 0.0f, 1.0f);
            shapeRenderer->DrawSphere(light.position, 0.2f, markerColor);
        }

        //for (const auto& light : spotLights) {
        //    if (!light.enabled) continue;

        //    DirectX::XMFLOAT4 rangeColor(light.color.x, light.color.y, light.color.z, 0.3f);
        //    shapeRenderer->DrawSphere(light.position, light.range, rangeColor);

        //    DirectX::XMFLOAT4 markerColor(1.0f, 0.5f, 0.0f, 1.0f);
        //    shapeRenderer->DrawSphere(light.position, 0.2f, markerColor);
        //}
    }

    void DrawGUI() const {
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x - 400, 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(380, 500), ImGuiCond_FirstUseEver);

        if (ImGui::Begin("Light Debug")) {
            ImGui::Text("Directional Light");
            ImGui::Separator();
            ImGui::Text("Direction: (%.2f, %.2f, %.2f)",
                directionalLight.direction.x,
                directionalLight.direction.y,
                directionalLight.direction.z);
            ImGui::Text("Color: (%.2f, %.2f, %.2f)",
                directionalLight.color.x,
                directionalLight.color.y,
                directionalLight.color.z);

            ImGui::Spacing();
            ImGui::Text("Point Lights: %d", static_cast<int>(pointLights.size()));
            ImGui::Separator();

            for (size_t i = 0; i < pointLights.size(); ++i) {
                const auto& light = pointLights[i];

                ImGui::PushID(static_cast<int>(i));

                bool isPlayerLight = (static_cast<int>(i) == playerLightIndex);
                if (isPlayerLight) {
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.3f, 0.7f, 0.3f, 0.8f));
                }

                char label[64];
                snprintf(label, sizeof(label), "Point Light %d %s",
                    static_cast<int>(i),
                    isPlayerLight ? "[Player]" : "");

                if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {
                    ImGui::Text("Enabled: %s", light.enabled ? "Yes" : "No");
                    ImGui::Text("Position: (%.2f, %.2f, %.2f)",
                        light.position.x, light.position.y, light.position.z);
                    ImGui::Text("Range: %.2f", light.range);
                    ImGui::Text("Intensity: %.2f", light.intensity);
                    ImGui::Text("Priority: %d", light.priority);
                    ImGui::ColorEdit3("Color", (float*)&light.color, ImGuiColorEditFlags_NoInputs);
                }

                if (isPlayerLight) {
                    ImGui::PopStyleColor();
                }

                ImGui::PopID();
            }

            ImGui::Spacing();
            ImGui::Text("Spot Lights: %d", static_cast<int>(spotLights.size()));
            ImGui::Separator();

            for (size_t i = 0; i < spotLights.size(); ++i) {
                const auto& light = spotLights[i];

                ImGui::PushID(static_cast<int>(i) + 10000);

                bool isPlayerLight = (static_cast<int>(i) == playerSpotLightIndex);
                if (isPlayerLight) {
                    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.7f, 0.5f, 0.3f, 0.8f));
                }

                char label[64];
                snprintf(label, sizeof(label), "Spot Light %d %s",
                    static_cast<int>(i),
                    isPlayerLight ? "[Player]" : "");

                if (ImGui::CollapsingHeader(label, ImGuiTreeNodeFlags_DefaultOpen)) {

                    if (isPlayerLight) {
                        ImGui::PopStyleColor();
                    }

                    ImGui::PopID();
                }
            }
            ImGui::End();
        }
    }

private:
    DirectionalLight directionalLight;
    std::vector<PointLight> pointLights;
    std::vector<SpotLight> spotLights;
    int playerLightIndex = -1;
    int playerSpotLightIndex = -1;
};