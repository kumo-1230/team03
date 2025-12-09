#include "PBRShader.h"
#include <System/GpuResourceUtils.h>

PBRShader::PBRShader(ID3D11Device* device) {
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
}

void PBRShader::Begin(const RenderContext& rc) {
    ID3D11DeviceContext* dc = rc.deviceContext;

    dc->VSSetShader(vertexShader.Get(), nullptr, 0);
    dc->PSSetShader(pixelShader.Get(), nullptr, 0);
    dc->IASetInputLayout(inputLayout.Get());
}

void PBRShader::Update(const RenderContext& rc, const Model::Mesh& mesh) {
    ID3D11DeviceContext* dc = rc.deviceContext;

    CbMesh cbMesh;
    cbMesh.world = mesh.node->worldTransform;
    cbMesh.materialIndex = mesh.materialIndex;
    cbMesh.hasTangent = 1;
    cbMesh.skin = -1;
    cbMesh.pad = 0;

    dc->UpdateSubresource(meshConstantBuffer.Get(), 0, 0, &cbMesh, 0, 0);
    dc->VSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());
    dc->PSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());

    ID3D11ShaderResourceView* srvs[] = {
        mesh.material->baseMap.Get(),
        mesh.material->metalnessRoughnessMap.Get(),
        mesh.material->normalMap.Get(),
        mesh.material->emissiveMap.Get(),
        mesh.material->occlusionMap.Get()
    };
    dc->PSSetShaderResources(1, _countof(srvs), srvs);
}

void PBRShader::End(const RenderContext& rc) {
    ID3D11DeviceContext* dc = rc.deviceContext;

    ID3D11ShaderResourceView* nullSrvs[5] = { nullptr };
    dc->PSSetShaderResources(1, _countof(nullSrvs), nullSrvs);
}