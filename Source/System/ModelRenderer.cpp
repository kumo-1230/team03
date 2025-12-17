#include "ModelRenderer.h"
#include "BasicShader.h"
#include "LambertShader.h"
#include "PBRShader.h"
#include "PBRMaterialConstants.h"
#include "Misc.h"
#include "GpuResourceUtils.h"
#include <algorithm>

// ModelRenderer.cpp のコンストラクタを修正
ModelRenderer::ModelRenderer(ID3D11Device* device)
    : device(device)
{
    OutputDebugStringA("ModelRenderer constructor START\n");

    GpuResourceUtils::CreateConstantBuffer(
        device,
        sizeof(CbScene),
        sceneConstantBuffer.GetAddressOf());
    OutputDebugStringA("Scene constant buffer created\n");

    GpuResourceUtils::CreateConstantBuffer(
        device,
        sizeof(CbSkeleton),
        skeletonConstantBuffer.GetAddressOf());
    OutputDebugStringA("Skeleton constant buffer created\n");

    OutputDebugStringA("Creating BasicShader\n");
    shaders[static_cast<int>(ShaderId::Basic)] = std::make_unique<BasicShader>(device);
    OutputDebugStringA("BasicShader created\n");

    OutputDebugStringA("Creating LambertShader\n");
    shaders[static_cast<int>(ShaderId::Lambert)] = std::make_unique<LambertShader>(device);
    OutputDebugStringA("LambertShader created\n");

    OutputDebugStringA("Creating PBRShader\n");
    shaders[static_cast<int>(ShaderId::PBR)] = std::make_unique<PBRShader>(device);
    OutputDebugStringA("PBRShader created\n");

    OutputDebugStringA("ModelRenderer constructor END\n");
}

void ModelRenderer::Draw(ShaderId shaderId, std::shared_ptr<Model> model)
{
    DrawInfo& drawInfo = drawInfos.emplace_back();
    drawInfo.shaderId = shaderId;
    drawInfo.model = model;

    if (shaderId == ShaderId::PBR)
    {
        Model* modelPtr = model.get();

        if (materialStructuredBuffers.find(modelPtr) == materialStructuredBuffers.end())
        {
            const std::vector<Model::Material>& srcMaterials = model->GetMaterials();
            std::vector<PBR_MaterialConstants> materials;
            materials.reserve(srcMaterials.size());

            for (const auto& srcMat : srcMaterials) {
                PBR_MaterialConstants mat = {};

                mat.emissiveFactor = srcMat.emissiveColor;
                mat.alphaMode = static_cast<int>(srcMat.alphaMode);
                mat.alphaCutoff = srcMat.alphaCutoff;
                mat.doubleSided = 0;

                mat.pbrMetallicRoughness.basecolorFactor = DirectX::XMFLOAT4(
                    srcMat.baseColor.x, srcMat.baseColor.y,
                    srcMat.baseColor.z, srcMat.baseColor.w
                );
                mat.pbrMetallicRoughness.metallicFactor = srcMat.metalness;
                mat.pbrMetallicRoughness.roughnessFactor = srcMat.roughness;

                mat.pbrMetallicRoughness.basecolorTexture.index = srcMat.baseMap ? 0 : -1;
                mat.pbrMetallicRoughness.basecolorTexture.texcoord = 0;

                mat.pbrMetallicRoughness.metallicRoughnessTexture.index =
                    srcMat.metalnessRoughnessMap ? 0 : -1;
                mat.pbrMetallicRoughness.metallicRoughnessTexture.texcoord = 0;

                mat.normalTexture.index = srcMat.normalMap ? 0 : -1;
                mat.normalTexture.texcoord = 0;
                mat.normalTexture.scale = 1.0f;

                mat.emissiveTexture.index = srcMat.emissiveMap ? 0 : -1;
                mat.emissiveTexture.texcoord = 0;

                mat.occlusionTexture.index = srcMat.occlusionMap ? 0 : -1;
                mat.occlusionTexture.texcoord = 0;
                mat.occlusionTexture.strength = srcMat.occlusionStrength;

                materials.push_back(mat);
            }

            D3D11_BUFFER_DESC bufferDesc = {};
            bufferDesc.ByteWidth = sizeof(PBR_MaterialConstants) * static_cast<UINT>(materials.size());
            bufferDesc.Usage = D3D11_USAGE_DEFAULT;
            bufferDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
            bufferDesc.StructureByteStride = sizeof(PBR_MaterialConstants);
            bufferDesc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;

            D3D11_SUBRESOURCE_DATA initData = {};
            initData.pSysMem = materials.data();

            Microsoft::WRL::ComPtr<ID3D11Buffer> buffer;
            HRESULT hr = device->CreateBuffer(&bufferDesc, &initData, buffer.GetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create material structured buffer");

            D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
            srvDesc.Format = DXGI_FORMAT_UNKNOWN;
            srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
            srvDesc.Buffer.FirstElement = 0;
            srvDesc.Buffer.NumElements = static_cast<UINT>(materials.size());

            Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv;
            hr = device->CreateShaderResourceView(buffer.Get(), &srvDesc, srv.GetAddressOf());
            _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create material SRV");

            materialStructuredBuffers[modelPtr] = buffer;
            materialStructuredBufferSRVs[modelPtr] = srv;

            PBRShader* pbrShader = static_cast<PBRShader*>(shaders[static_cast<int>(ShaderId::PBR)].get());
            pbrShader->SetMaterialBufferSRV(srv);
            OutputDebugStringA("Material SRV set to PBRShader\n");
        }
    }
}
void ModelRenderer::Render(const RenderContext& rc)
{
    ID3D11DeviceContext* dc = rc.deviceContext;

    {
        static LightManager defaultLightManager;
        const LightManager* lightManager = rc.lightManager ? rc.lightManager : &defaultLightManager;

        CbScene cbScene{};
        DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.camera->GetView());
        DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.camera->GetProjection());
        DirectX::XMStoreFloat4x4(&cbScene.viewProjection, V * P);

        const DirectionalLight& directionalLight = lightManager->GetDirectionalLight();
        cbScene.lightDirection = directionalLight.direction;
        cbScene.lightColor = directionalLight.color;
        cbScene.cameraPosition = rc.camera->GetEye();

        dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &cbScene, 0, 0);
    }

    ID3D11Buffer* vsConstantBuffers[] = {
        skeletonConstantBuffer.Get(),
        sceneConstantBuffer.Get(),
    };
    ID3D11Buffer* psConstantBuffers[] = {
        sceneConstantBuffer.Get(),
    };
    dc->VSSetConstantBuffers(6, _countof(vsConstantBuffers), vsConstantBuffers);
    dc->PSSetConstantBuffers(7, _countof(psConstantBuffers), psConstantBuffers);

    ID3D11SamplerState* samplerStates[] = {
        rc.renderState->GetSamplerState(SamplerState::LinearWrap)
    };
    dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

    dc->OMSetDepthStencilState(rc.renderState->GetDepthStencilState(DepthState::TestAndWrite), 0);
    dc->RSSetState(rc.renderState->GetRasterizerState(RasterizerState::SolidCullBack));

    auto drawMesh = [&](const Model::Mesh& mesh, Shader* shader, ShaderId shaderId, Model* modelPtr)
        {
            UINT stride = sizeof(Model::Vertex);
            UINT offset = 0;
            dc->IASetVertexBuffers(0, 1, mesh.vertexBuffer.GetAddressOf(), &stride, &offset);
            dc->IASetIndexBuffer(mesh.indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
            dc->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

            CbSkeleton cbSkeleton{};
            if (mesh.bones.size() > 0) {
                for (size_t i = 0; i < mesh.bones.size(); ++i) {
                    const Model::Bone& bone = mesh.bones.at(i);
                    DirectX::XMMATRIX WorldTransform = DirectX::XMLoadFloat4x4(&bone.node->worldTransform);
                    DirectX::XMMATRIX OffsetTransform = DirectX::XMLoadFloat4x4(&bone.offsetTransform);
                    DirectX::XMMATRIX BoneTransform = OffsetTransform * WorldTransform;
                    DirectX::XMStoreFloat4x4(&cbSkeleton.boneTransforms[i], BoneTransform);
                }
            }
            else {
                cbSkeleton.boneTransforms[0] = mesh.node->worldTransform;
            }
            dc->UpdateSubresource(skeletonConstantBuffer.Get(), 0, 0, &cbSkeleton, 0, 0);

            if (shaderId == ShaderId::PBR) {
                auto it = materialStructuredBufferSRVs.find(modelPtr);
                if (it != materialStructuredBufferSRVs.end()) {
                    dc->PSSetShaderResources(0, 1, it->second.GetAddressOf());
                }
            }

            shader->Update(rc, mesh);

            dc->DrawIndexed(static_cast<UINT>(mesh.indices.size()), 0, 0);
        };

    DirectX::XMVECTOR CameraPosition = DirectX::XMLoadFloat3(&rc.camera->GetEye());
    DirectX::XMVECTOR CameraFront = DirectX::XMLoadFloat3(&rc.camera->GetFront());

    dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Opaque), nullptr, 0xFFFFFFFF);

    for (DrawInfo& drawInfo : drawInfos) {
        Shader* shader = shaders[static_cast<int>(drawInfo.shaderId)].get();
        shader->Begin(rc);

        for (const Model::Mesh& mesh : drawInfo.model->GetMeshes()) {
            if (mesh.material->alphaMode == Model::AlphaMode::Blend ||
                (mesh.material->baseColor.w > 0.01f && mesh.material->baseColor.w < 0.99f)) {
                TransparencyDrawInfo& transparencyDrawInfo = transparencyDrawInfos.emplace_back();
                transparencyDrawInfo.shaderId = drawInfo.shaderId;
                transparencyDrawInfo.mesh = &mesh;

                DirectX::XMVECTOR Position = DirectX::XMVectorSet(
                    mesh.node->worldTransform._41,
                    mesh.node->worldTransform._42,
                    mesh.node->worldTransform._43,
                    0.0f);
                DirectX::XMVECTOR Vec = DirectX::XMVectorSubtract(Position, CameraPosition);
                transparencyDrawInfo.distance = DirectX::XMVectorGetX(DirectX::XMVector3Dot(CameraFront, Vec));

                continue;
            }

            drawMesh(mesh, shader, drawInfo.shaderId, drawInfo.model.get());
        }

        shader->End(rc);
    }
    drawInfos.clear();

    dc->OMSetBlendState(rc.renderState->GetBlendState(BlendState::Transparency), nullptr, 0xFFFFFFFF);

    std::sort(transparencyDrawInfos.begin(), transparencyDrawInfos.end(),
        [](const TransparencyDrawInfo& lhs, const TransparencyDrawInfo& rhs) {
            return lhs.distance > rhs.distance;
        });

    for (const TransparencyDrawInfo& transparencyDrawInfo : transparencyDrawInfos) {
        Shader* shader = shaders[static_cast<int>(transparencyDrawInfo.shaderId)].get();

        shader->Begin(rc);

        drawMesh(*transparencyDrawInfo.mesh, shader, transparencyDrawInfo.shaderId, nullptr);

        shader->End(rc);
    }
    transparencyDrawInfos.clear();

    for (ID3D11Buffer*& vsConstantBuffer : vsConstantBuffers) { vsConstantBuffer = nullptr; }
    for (ID3D11Buffer*& psConstantBuffer : psConstantBuffers) { psConstantBuffer = nullptr; }
    dc->VSSetConstantBuffers(6, _countof(vsConstantBuffers), vsConstantBuffers);
    dc->PSSetConstantBuffers(7, _countof(psConstantBuffers), psConstantBuffers);

    for (ID3D11SamplerState*& samplerState : samplerStates) { samplerState = nullptr; }
    dc->PSSetSamplers(0, _countof(samplerStates), samplerStates);

    ID3D11ShaderResourceView* nullSrvs[6] = { nullptr };
    dc->PSSetShaderResources(0, 6, nullSrvs);
}