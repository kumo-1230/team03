#pragma once

#include <memory>
#include <vector>
#include <map>
#include <wrl.h>
#include <d3d11.h>
#include <DirectXMath.h>
#include "RenderContext.h"
#include "Model.h"
#include "Shader.h"

enum class ShaderId
{
    Basic,
    Lambert,
    PBR,
    Max
};

class ModelRenderer
{
public:
    ModelRenderer(ID3D11Device* device);
    ~ModelRenderer() = default;

    // 描画予約
    void Draw(ShaderId shaderId, std::shared_ptr<Model> model);

    // 描画実行
    void Render(const RenderContext& rc);

private:
    struct CbScene
    {
        DirectX::XMFLOAT4X4 viewProjection;
        DirectX::XMFLOAT3   lightDirection;
        float               pad1;
        DirectX::XMFLOAT3   lightColor;
        float               pad2;
        DirectX::XMFLOAT3   cameraPosition;
        float               pad3;
    };

    struct CbSkeleton
    {
        DirectX::XMFLOAT4X4 boneTransforms[256];
    };

    struct DrawInfo
    {
        ShaderId shaderId;
        std::shared_ptr<Model> model;
    };

    struct TransparencyDrawInfo
    {
        ShaderId shaderId = ShaderId::Basic;
        const Model::Mesh* mesh;
        float distance;
    };

    Microsoft::WRL::ComPtr<ID3D11Buffer> sceneConstantBuffer;
    Microsoft::WRL::ComPtr<ID3D11Buffer> skeletonConstantBuffer;

    std::unique_ptr<Shader> shaders[static_cast<int>(ShaderId::Max)];
    std::vector<DrawInfo> drawInfos;
    std::vector<TransparencyDrawInfo> transparencyDrawInfos;

    // PBRシェーダー用: モデルごとのStructuredBuffer管理
    std::map<Model*, Microsoft::WRL::ComPtr<ID3D11Buffer>> materialStructuredBuffers;
    std::map<Model*, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> materialStructuredBufferSRVs;

    ID3D11Device* device; // StructuredBuffer作成用
};