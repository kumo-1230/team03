#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <System/RenderContext.h>
#include "System/Shader.h"
#include "System/Model.h"
#include <memory>
#include "System/Light.h"

class PBRShader : public Shader {
public:
    PBRShader(ID3D11Device* device);
    ~PBRShader() override {
        OutputDebugStringA("PBRShader destructor called\n");
    }

    void Begin(const RenderContext& rc) override;
    void Update(const RenderContext& rc, const Model::Mesh& mesh) override;
    void End(const RenderContext& rc) override;

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    // メッシュ定数バッファ (register(b0))
    struct CbMesh {
        DirectX::XMFLOAT4X4 world;
        int material;
        int hasTangent;
        int skin;
        int pad;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> meshConstantBuffer;

    // ポイントライト構造体 - HLSLと完全一致させる
    struct CbPointLight {
        DirectX::XMFLOAT3 position;
        float range;
        DirectX::XMFLOAT3 color;
        float intensity;
    };

    // スポットライト構造体 - HLSLと完全一致させる
    struct CbSpotLight {
        DirectX::XMFLOAT3 position;
        float range;
        DirectX::XMFLOAT3 direction;
        float innerConeAngle;
        DirectX::XMFLOAT3 color;
        float outerConeAngle;
        float intensity;
        DirectX::XMFLOAT3 pad;
    };

    // シーン定数バッファ (register(b1))
    struct CbScene {
        DirectX::XMFLOAT4X4 viewProjection;  // 64 bytes
        DirectX::XMFLOAT4 lightDirection;    // 16 bytes
        DirectX::XMFLOAT4 cameraPosition;    // 16 bytes
        float ambientIntensity;              // 4 bytes
        float exposure;                      // 4 bytes
        int pointLightCount;                 // 4 bytes
        int spotLightCount;                  // 4 bytes
        CbPointLight pointLights[8];         // 32 * 8 = 256 bytes
        CbSpotLight spotLights[4];           // 64 * 4 = 256 bytes
        // Total: 64 + 16 + 16 + 16 + 256 + 256 = 624 bytes
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer> materialStructuredBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialStructuredBufferSRV;

    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[3];
};