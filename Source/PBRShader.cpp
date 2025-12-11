#include "PBRShader.h"
#include "PBRMaterialConstants.h"
#include <System/GpuResourceUtils.h>

PBRShader::PBRShader(ID3D11Device* device) {
    OutputDebugStringA("PBRShader constructor START\n");

    // サイズチェック
    _ASSERT_EXPR(sizeof(CbPointLight) == 32, "CbPointLight size must be 32 bytes");
    _ASSERT_EXPR(sizeof(CbSpotLight) == 64, "CbSpotLight size must be 64 bytes");
    _ASSERT_EXPR(sizeof(CbScene) == 624, "CbScene size must be 624 bytes");

    OutputDebugStringA("Size checks passed\n");

    // 入力レイアウトの定義
    OutputDebugStringA("Creating input layout\n");
    D3D11_INPUT_ELEMENT_DESC inputElementDesc[] = {
        { "POSITION",     0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "NORMAL",       0, DXGI_FORMAT_R32G32B32_FLOAT,    0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TANGENT",      0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD",     0, DXGI_FORMAT_R32G32_FLOAT,       0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_WEIGHTS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "BONE_INDICES", 0, DXGI_FORMAT_R32G32B32A32_UINT,  0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
    };

    // 頂点シェーダーの読み込み
    OutputDebugStringA("Loading vertex shader\n");
    GpuResourceUtils::LoadVertexShader(
        device,
        "Data/Shader/pbr_model_vs.cso",
        inputElementDesc,
        _countof(inputElementDesc),
        inputLayout.GetAddressOf(),
        vertexShader.GetAddressOf()
    );

    // ピクセルシェーダーの読み込み
    OutputDebugStringA("Loading pixel shader\n");
    GpuResourceUtils::LoadPixelShader(
        device,
        "Data/Shader/pbr_model_ps_maya_style.cso",
        pixelShader.GetAddressOf()
    );

    // メッシュ定数バッファの作成
    OutputDebugStringA("Creating mesh constant buffer\n");
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(CbMesh);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        HRESULT hr = device->CreateBuffer(&desc, nullptr, meshConstantBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create mesh constant buffer");

        OutputDebugStringA("Mesh constant buffer created\n");
    }

    // シーン定数バッファの作成
    OutputDebugStringA("Creating scene constant buffer\n");
    {
        D3D11_BUFFER_DESC desc = {};
        desc.ByteWidth = sizeof(CbScene);
        desc.Usage = D3D11_USAGE_DEFAULT;
        desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        desc.CPUAccessFlags = 0;
        desc.MiscFlags = 0;
        desc.StructureByteStride = 0;

        HRESULT hr = device->CreateBuffer(&desc, nullptr, sceneConstantBuffer.GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create scene constant buffer");

        OutputDebugStringA("Scene constant buffer created\n");
    }

    // サンプラーステートの作成
    OutputDebugStringA("Creating sampler states\n");
    {
        D3D11_SAMPLER_DESC desc = {};
        desc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
        desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
        desc.MinLOD = 0;
        desc.MaxLOD = D3D11_FLOAT32_MAX;

        // POINT サンプラー
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_POINT;
        HRESULT hr = device->CreateSamplerState(&desc, samplerStates[0].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create POINT sampler");

        // LINEAR サンプラー
        desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
        hr = device->CreateSamplerState(&desc, samplerStates[1].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create LINEAR sampler");

        // ANISOTROPIC サンプラー
        desc.Filter = D3D11_FILTER_ANISOTROPIC;
        desc.MaxAnisotropy = 16;
        hr = device->CreateSamplerState(&desc, samplerStates[2].GetAddressOf());
        _ASSERT_EXPR(SUCCEEDED(hr), "Failed to create ANISOTROPIC sampler");
    }

    OutputDebugStringA("PBRShader constructor END\n");

    // 注意: materialStructuredBufferSRV は Model 読み込み時に設定されます
}

void PBRShader::Begin(const RenderContext& rc) {
    ID3D11DeviceContext* dc = rc.deviceContext;

    // シェーダーと入力レイアウトの設定
    dc->VSSetShader(vertexShader.Get(), nullptr, 0);
    dc->PSSetShader(pixelShader.Get(), nullptr, 0);
    dc->IASetInputLayout(inputLayout.Get());

    // シーン定数バッファの準備
    CbScene cbScene = {};
    memset(&cbScene, 0, sizeof(CbScene));

    // ビュープロジェクション行列の設定
    DirectX::XMMATRIX V = DirectX::XMLoadFloat4x4(&rc.camera->GetView());
    DirectX::XMMATRIX P = DirectX::XMLoadFloat4x4(&rc.camera->GetProjection());
    DirectX::XMMATRIX VP = V * P;
    DirectX::XMStoreFloat4x4(&cbScene.viewProjection, VP);

    // ディレクショナルライトの設定
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

    // カメラ位置の設定
    DirectX::XMFLOAT3 eye = rc.camera->GetEye();
    cbScene.cameraPosition = DirectX::XMFLOAT4(eye.x, eye.y, eye.z, 1.0f);

    // アンビエントとエクスポージャーの設定
    cbScene.ambientIntensity = 0.2f;
    cbScene.exposure = 0.8f;

    // ライトの設定
    if (rc.lightManager) {
        // ポイントライトの取得と設定
        std::vector<PointLight> nearestLights = rc.lightManager->GetNearestPointLights(eye, 8);
        cbScene.pointLightCount = static_cast<int>(nearestLights.size());

        for (size_t i = 0; i < nearestLights.size() && i < 8; ++i) {
            cbScene.pointLights[i].position = nearestLights[i].position;
            cbScene.pointLights[i].range = nearestLights[i].range;
            cbScene.pointLights[i].color = nearestLights[i].color;
            cbScene.pointLights[i].intensity = nearestLights[i].intensity;
        }

        // スポットライトの取得と設定
        std::vector<SpotLight> nearestSpotLights = rc.lightManager->GetNearestSpotLights(eye, 4);
        cbScene.spotLightCount = static_cast<int>(nearestSpotLights.size());

        for (size_t i = 0; i < nearestSpotLights.size() && i < 4; ++i) {
            cbScene.spotLights[i].position = nearestSpotLights[i].position;
            cbScene.spotLights[i].range = nearestSpotLights[i].range;
            cbScene.spotLights[i].direction = nearestSpotLights[i].direction;
            cbScene.spotLights[i].innerConeAngle = DirectX::XMConvertToRadians(nearestSpotLights[i].innerConeAngle);
            cbScene.spotLights[i].color = nearestSpotLights[i].color;
            cbScene.spotLights[i].outerConeAngle = DirectX::XMConvertToRadians(nearestSpotLights[i].outerConeAngle);
            cbScene.spotLights[i].intensity = nearestSpotLights[i].intensity;
            cbScene.spotLights[i].pad = DirectX::XMFLOAT3(0.0f, 0.0f, 0.0f);
        }
    }
    else {
        cbScene.pointLightCount = 0;
        cbScene.spotLightCount = 0;
    }

    // 定数バッファの更新と設定
    dc->UpdateSubresource(sceneConstantBuffer.Get(), 0, 0, &cbScene, 0, 0);
    dc->VSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());
    dc->PSSetConstantBuffers(1, 1, sceneConstantBuffer.GetAddressOf());

    // サンプラーステートの設定
    ID3D11SamplerState* samplers[3] = {
        samplerStates[0].Get(),
        samplerStates[1].Get(),
        samplerStates[2].Get()
    };
    dc->PSSetSamplers(0, 3, samplers);

    // マテリアル構造化バッファの設定
    if (materialStructuredBufferSRV) {
        dc->PSSetShaderResources(0, 1, materialStructuredBufferSRV.GetAddressOf());
    }
    else {
        // マテリアルバッファが未設定の場合は警告（初回のみ）
        static bool warned = false;
        if (!warned) {
            OutputDebugStringA("Warning: materialStructuredBufferSRV is not set\n");
            warned = true;
        }
    }
}

void PBRShader::Update(const RenderContext& rc, const Model::Mesh& mesh) {
    ID3D11DeviceContext* dc = rc.deviceContext;

    // メッシュ定数バッファの準備
    CbMesh cbMesh;
    cbMesh.world = mesh.node->worldTransform;
    cbMesh.material = mesh.materialIndex;
    cbMesh.hasTangent = 1;
    cbMesh.skin = -1;
    cbMesh.pad = 0;

    // 定数バッファの更新と設定
    dc->UpdateSubresource(meshConstantBuffer.Get(), 0, 0, &cbMesh, 0, 0);
    dc->VSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());
    dc->PSSetConstantBuffers(0, 1, meshConstantBuffer.GetAddressOf());

    // マテリアルテクスチャの設定
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

    // シェーダーリソースのクリア
    ID3D11ShaderResourceView* nullSrvs[6] = { nullptr };
    dc->PSSetShaderResources(0, 6, nullSrvs);
}