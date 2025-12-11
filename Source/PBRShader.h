#pragma once
#include <d3d11.h>
#include <wrl.h>
#include <DirectXMath.h>
#include <vector>
#include <System/RenderContext.h>
#include "System/Shader.h"
#include "System/Model.h"
#include <memory>

class PBRShader : public Shader {
public:
    PBRShader(ID3D11Device* device);
    ~PBRShader() override = default;

    void Begin(const RenderContext& rc) override;
    void Update(const RenderContext& rc, const Model::Mesh& mesh) override;
    void End(const RenderContext& rc) override;

private:
    Microsoft::WRL::ComPtr<ID3D11VertexShader> vertexShader;
    Microsoft::WRL::ComPtr<ID3D11PixelShader> pixelShader;
    Microsoft::WRL::ComPtr<ID3D11InputLayout> inputLayout;

    struct CbMesh {
        DirectX::XMFLOAT4X4 world;
        int material;
        int hasTangent;
        int skin;
        int pad;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> meshConstantBuffer;

    struct CbScene {
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMFLOAT4 lightDirection;
        DirectX::XMFLOAT4 cameraPosition;
    };
    Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;

    Microsoft::WRL::ComPtr<ID3D11Buffer> materialStructuredBuffer;
    Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> materialStructuredBufferSRV;

    Microsoft::WRL::ComPtr<ID3D11SamplerState> samplerStates[3];
};