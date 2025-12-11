#pragma once
#include <DirectXMath.h>

#pragma pack(push, 1)

struct PBR_TextureInfo {
    int index;
    int texcoord;
};

struct PBR_NormalTextureInfo {
    int index;
    int texcoord;
    float scale;
};

struct PBR_OcclusionTextureInfo {
    int index;
    int texcoord;
    float strength;
};

struct PBR_MetallicRoughness {
    DirectX::XMFLOAT4 basecolorFactor;
    PBR_TextureInfo basecolorTexture;
    float metallicFactor;
    float roughnessFactor;
    PBR_TextureInfo metallicRoughnessTexture;
};

struct PBR_MaterialConstants {
    DirectX::XMFLOAT3 emissiveFactor;
    int alphaMode;
    float alphaCutoff;
    int doubleSided;
    PBR_MetallicRoughness pbrMetallicRoughness;
    PBR_NormalTextureInfo normalTexture;
    PBR_OcclusionTextureInfo occlusionTexture;
    PBR_TextureInfo emissiveTexture;
};

#pragma pack(pop)

static_assert(sizeof(PBR_TextureInfo) == 8, "PBR_TextureInfo must be 8 bytes");
static_assert(sizeof(PBR_NormalTextureInfo) == 12, "PBR_NormalTextureInfo must be 12 bytes");
static_assert(sizeof(PBR_OcclusionTextureInfo) == 12, "PBR_OcclusionTextureInfo must be 12 bytes");
static_assert(sizeof(PBR_MetallicRoughness) == 40, "PBR_MetallicRoughness must be 40 bytes");
static_assert(sizeof(PBR_MaterialConstants) == 96, "PBR_MaterialConstants must be 96 bytes");