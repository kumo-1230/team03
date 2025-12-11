#include "PBRShader.h"
#include "PBRMaterialConstants.h"
#include <System/GpuResourceUtils.h>
#include <sstream>

PBRShader::PBRShader(ID3D11Device* device) {
    char buffer[1024];
    sprintf_s(buffer, "=== PBR Shader Material Constants Size Debug ===\n");
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "PBR_TextureInfo: %zu bytes\n", sizeof(PBR_TextureInfo));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "PBR_NormalTextureInfo: %zu bytes\n", sizeof(PBR_NormalTextureInfo));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "PBR_OcclusionTextureInfo: %zu bytes\n", sizeof(PBR_OcclusionTextureInfo));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "PBR_MetallicRoughness: %zu bytes\n", sizeof(PBR_MetallicRoughness));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "PBR_MaterialConstants: %zu bytes (Expected: 96)\n", sizeof(PBR_MaterialConstants));
    OutputDebugStringA(buffer);

    sprintf_s(buffer, "\n=== Member Offsets ===\n");
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "emissiveFactor offset: %zu\n", offsetof(PBR_MaterialConstants, emissiveFactor));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "alphaMode offset: %zu\n", offsetof(PBR_MaterialConstants, alphaMode));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "alphaCutoff offset: %zu\n", offsetof(PBR_MaterialConstants, alphaCutoff));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "doubleSided offset: %zu\n", offsetof(PBR_MaterialConstants, doubleSided));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "pbrMetallicRoughness offset: %zu\n", offsetof(PBR_MaterialConstants, pbrMetallicRoughness));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "normalTexture offset: %zu\n", offsetof(PBR_MaterialConstants, normalTexture));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "occlusionTexture offset: %zu\n", offsetof(PBR_MaterialConstants, occlusionTexture));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "emissiveTexture offset: %zu\n", offsetof(PBR_MaterialConstants, emissiveTexture));
    OutputDebugStringA(buffer);
    sprintf_s(buffer, "================================================\n");
    OutputDebugStringA(buffer);

    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_INDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    GpuResourceUtils::LoadVertexShader(
        device,
        "Data/Shader/pbr_model_vs.cso",
        inputElementDesc,
        _countof(inputElementDesc),
        inputLayout.GetAddressOf(),
        vertexShader.GetAddressOf()
    );

    GpuResourceUtils::LoadPixelShader(
        device,
        "Data/Shader/pbr_model_ps.cso",
        pixelShader.GetAddressOf()
    );

    GpuResourceUtils::CreateConstantBuffer(
        device,
        sizeof(CbMesh),
        meshConstantBuffer.GetAddressOf()
    );

    GpuResourceUtils::CreateConstantBuffer(
        device,
        sizeof(CbScene),
        sceneConstantBuffer.GetAddressOf()
    );

    {
        D3D11_SAMPLER_DESC desc = {};
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MinLOD = 0;
        desc.MaxLOD = D3D11_FLOAT32_MAX;
        HRESULT hr = device->CreateSamplerState(&desc, samplerStates[0].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create POINT sampler");

        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        hr = device->CreateSamplerState(&desc, samplerStates[1].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create LINEAR sampler");

        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        desc.MaxAnisotropy = 16;
        hr = device->CreateSamplerState(&desc, samplerStates[2].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create ANISOTROPIC sampler");
    }
}

void PBRShader::Begin(const RenderContext& rc) {
    ID3D11DeviceContext* dc = rc.deviceContext;

    dc->VSSetShader(vertexShader.Get(), nullptr, 0);
    dc->PSSetShader(pixelShader.Get(), nullptr, 0);
    dc->IASetInputLayout(inputLayout.Get());

    CbScene cbScene;

    DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.camera->GetView());
    DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.camera->GetProjection());
    DirectX::XMMATRIX VP = V * P;
    DirectX::XMStoreFloat4x4(&cbScene.viewProjection, VP);

    if (rc.lightManager) {
        const DirectionalLight& light = rc.lightManager->GetDirectionalLight();
        cbScene.lightDirection = DirectX::XMFLOAT4(
            light.direction.x,
            light.direction.y,
            light.direction.z,
            0.0f
        );
    }
    else {
        cbScene.lightDirection = DirectX::XMFLOAT4(0.0f, -1.0f, 0.0f, 0.0f);
    }

    DirectX::XMFLOAT3 eye = rc.camera->GetEye();
    cbScene.cameraPosition = DirectX::XMFLOAT4(eye.x, eye.y, eye.z, 1.0f);

    dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &cbScene, 0, 0);
    dc->VSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());
    dc->PSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());

    dc->PSSetShaderResources(0, 1, materialStructuredBufferSRV.GetAddressOf());

    ID3D11SamplerState* samplers[3] = {
        samplerStates[0].Get(),
        samplerStates[1].Get(),
        samplerStates[2].Get()
    };
    dc->PSSetSamplers(0, 3, samplers);
}

void PBRShader::Update(const RenderContext& rc, const Model::Mesh& mesh) {
    ID3D11DeviceContext* dc = rc.deviceContext;

    CbMesh cbMesh;
    cbMesh.world = mesh.node->worldTransform;
    cbMesh.material = mesh.materialIndex;
    cbMesh.hasTangent = 1;
    cbMesh.skin = -1;
    cbMesh.pad = 0;

    dc->UpdateSubresource(meshConstantBuffer.Get(), 0, 0, &cbMesh, 0, 0);
    dc->VSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());
    dc->PSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());

    ID3D11ShaderResourceView* srvs[5] = {
        mesh.material->baseMap.Get(),
        mesh.material->metalnessRoughnessMap.Get(),
        mesh.material->normalMap.Get(),
        mesh.material->emissiveMap.Get(),
        mesh.material->occlusionMap.Get()
    };
    dc->PSSetShaderResources(1, 5, srvs);
}

void PBRShader::End(const RenderContext& rc) {
    ID3D11DeviceContext* dc = rc.deviceContext;

    ID3D11ShaderResourceView* nullSrvs[6] = { nullptr };
    dc->PSSetShaderResources(0, 6, nullSrvs);
}