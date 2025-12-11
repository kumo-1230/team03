#include "bidirectional_reflectance_distribution_function.hlsli"

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 w_position : POSITION;
    float4 w_normal : NORMAL;
    float4 w_tangent : TANGENT;
    float2 texcoord : TEXCOORD;
};

struct PointLight
{
    float3 position;
    float range;
    float3 color;
    float intensity;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 view_projection;
    float4 light_direction;
    float4 camera_position;
    float ambient_intensity;
    float exposure;
    int point_light_count;
    float pad_scene;
    PointLight point_lights[8];
};

struct texture_info
{
    int index;
    int texcoord;
};

struct normal_texture_info
{
    int index;
    int texcoord;
    float scale;
};

struct occlusion_texture_info
{
    int index;
    int texcoord;
    float strength;
};

struct pbr_metallic_roughness
{
    float4 basecolor_factor;
    texture_info basecolor_texture;
    float metallic_factor;
    float roughness_factor;
    texture_info metallic_roughness_texture;
};

struct material_constants
{
    float3 emissive_factor;
    int alpha_mode;
    float alpha_cutoff;
    int double_sided;
    
    pbr_metallic_roughness pbr_metallic_roughness;
    
    normal_texture_info normal_texture;
    occlusion_texture_info occlusion_texture;
    texture_info emissive_texture;
};

cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b0)
{
    row_major float4x4 world;
    int material;
    bool has_tangent;
    int skin;
    int pad;
};

StructuredBuffer<material_constants> materials : register(t0);

#define BASECOLOR_TEXTURE 0
#define METALLIC_ROUGHNESS_TEXTURE 1
#define NORMAL_TEXTURE 2
#define EMISSIVE_TEXTURE 3
#define OCCLUSION_TEXTURE 4
Texture2D<float4> material_textures[5] : register(t1);

#define POINT 0
#define LINEAR 1
#define ANISOTROPIC 2
SamplerState sampler_states[3] : register(s0);

float3 tone_mapping_aces(float3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return saturate((x * (a * x + b)) / (x * (c * x + d) + e));
}

float3 calculate_ambient_ibl(float3 N, float3 V, float3 basecolor, float metallic, float roughness, float intensity)
{
    const float3 ambient_sky = float3(0.15, 0.18, 0.22);
    const float3 ambient_ground = float3(0.05, 0.04, 0.03);
    
    float sky_factor = N.y * 0.5 + 0.5;
    float3 ambient_color = lerp(ambient_ground, ambient_sky, sky_factor);
    
    float3 f0 = lerp(0.04, basecolor, metallic);
    float NoV = max(0.0, dot(N, V));
    float3 fresnel = f0 + (max(1.0 - roughness, f0) - f0) * pow(1.0 - NoV, 5.0);
    
    float3 kD = 1.0 - fresnel;
    kD *= 1.0 - metallic;
    
    float3 diffuse_ibl = kD * basecolor * ambient_color;
    float3 specular_ibl = fresnel * ambient_color * (1.0 - roughness * 0.5);
    
    return (diffuse_ibl + specular_ibl) * intensity;
}

float3 calculate_rim_light(float3 N, float3 V, float3 basecolor, float intensity)
{
    float rim = 1.0 - max(0.0, dot(N, V));
    rim = pow(rim, 4.0);
    return basecolor * rim * intensity * 0.3;
}

float4 main(VS_OUT pin) : SV_TARGET
{
    const float GAMMA = 2.2;
    
    const material_constants m = materials[material];
    
    float4 basecolor_factor = m.pbr_metallic_roughness.basecolor_factor;
    const int basecolor_texture = m.pbr_metallic_roughness.basecolor_texture.index;
    if (basecolor_texture > -1)
    {
        float4 sampled = material_textures[BASECOLOR_TEXTURE].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        basecolor_factor *= sampled;
    }
    
    float3 emmisive_factor = m.emissive_factor;
    const int emissive_texture = m.emissive_texture.index;
    if (emissive_texture > -1)
    {
        float4 sampled = material_textures[EMISSIVE_TEXTURE].Sample(sampler_states[ANISOTROPIC], pin.texcoord);
        sampled.rgb = pow(sampled.rgb, GAMMA);
        emmisive_factor *= sampled.rgb;
    }
    
    float roughness_factor = m.pbr_metallic_roughness.roughness_factor;
    float metallic_factor = m.pbr_metallic_roughness.metallic_factor;
    const int metallic_roughness_texture = m.pbr_metallic_roughness.metallic_roughness_texture.index;
    if (metallic_roughness_texture > -1)
    {
        float4 sampled = material_textures[METALLIC_ROUGHNESS_TEXTURE].Sample(sampler_states[LINEAR], pin.texcoord);
        roughness_factor *= sampled.g;
        metallic_factor *= sampled.b;
    }
    
    float occlusion_factor = 1.0;
    const int occlusion_texture = m.occlusion_texture.index;
    if (occlusion_texture > -1)
    {
        float4 sampled = material_textures[OCCLUSION_TEXTURE].Sample(sampler_states[LINEAR], pin.texcoord);
        occlusion_factor *= sampled.r;
    }
    const float occlusion_strength = m.occlusion_texture.strength;
    
    const float3 f0 = lerp(0.04, basecolor_factor.rgb, metallic_factor);
    const float3 f90 = 1.0;
    const float alpha_roughness = roughness_factor * roughness_factor;
    const float3 c_diff = lerp(basecolor_factor.rgb, 0.0, metallic_factor);
    
    const float3 P = pin.w_position.xyz;
    const float3 V = normalize(camera_position.xyz - pin.w_position.xyz);
    
    float3 N = normalize(pin.w_normal.xyz);
    float3 T = has_tangent ? normalize(pin.w_tangent.xyz) : float3(1, 0, 0);
    float sigma = has_tangent ? pin.w_tangent.w : 1.0;
    T = normalize(T - N * dot(N, T));
    float3 B = normalize(cross(N, T) * sigma);
    
    const int normal_texture = m.normal_texture.index;
    if (normal_texture > -1)
    {
        float4 sampled = material_textures[NORMAL_TEXTURE].Sample(sampler_states[LINEAR], pin.texcoord);
        float3 normal_factor = sampled.xyz;
        normal_factor = (normal_factor * 2.0) - 1.0;
        normal_factor = normalize(normal_factor * float3(m.normal_texture.scale, m.normal_texture.scale, 1.0));
        N = normalize((normal_factor.x * T) + (normal_factor.y * B) + (normal_factor.z * N));
    }
    
    float3 diffuse = 0;
    float3 specular = 0;
    
    float3 L = normalize(-light_direction.xyz);
    float3 Li = float3(1.0, 1.0, 1.0);
    const float NoL = max(0.0, dot(N, L));
    const float NoV = max(0.0, dot(N, V));
    if (NoL > 0.0 || NoV > 0.0)
    {
        const float3 H = normalize(V + L);
        const float NoH = max(0.0, dot(N, H));
        const float HoV = max(0.0, dot(H, V));
        
        diffuse += Li * NoL * brdf_lambertian(f0, f90, c_diff, HoV);
        specular += Li * NoL * brdf_specular_ggx(f0, f90, alpha_roughness, HoV, NoL, NoV, NoH);
    }
    
    for (int i = 0; i < point_light_count; ++i)
    {
        float3 light_vec = point_lights[i].position - P;
        float distance = length(light_vec);
        float attenuation = saturate(1.0 - (distance / point_lights[i].range));
        attenuation *= attenuation;
        
        if (attenuation > 0.0)
        {
            float3 point_L = normalize(light_vec);
            float3 point_Li = point_lights[i].color * point_lights[i].intensity * attenuation;
            
            const float point_NoL = max(0.0, dot(N, point_L));
            if (point_NoL > 0.0)
            {
                const float3 point_H = normalize(V + point_L);
                const float point_NoH = max(0.0, dot(N, point_H));
                const float point_HoV = max(0.0, dot(point_H, V));
                
                diffuse += point_Li * point_NoL * brdf_lambertian(f0, f90, c_diff, point_HoV);
                specular += point_Li * point_NoL * brdf_specular_ggx(f0, f90, alpha_roughness, point_HoV, point_NoL, NoV, point_NoH);
            }
        }
    }
    
    float3 ambient_ibl = calculate_ambient_ibl(N, V, basecolor_factor.rgb, metallic_factor, roughness_factor, ambient_intensity);
    float3 rim_light = calculate_rim_light(N, V, basecolor_factor.rgb, ambient_intensity);
    
    float3 emmisive = emmisive_factor;
    diffuse = lerp(diffuse, diffuse * occlusion_factor, occlusion_strength);
    specular = lerp(specular, specular * occlusion_factor, occlusion_strength);
    ambient_ibl = lerp(ambient_ibl, ambient_ibl * occlusion_factor, occlusion_strength);
    
    float3 Lo = (diffuse + specular + ambient_ibl + rim_light + emmisive) * exposure;
    
    Lo = tone_mapping_aces(Lo);
    Lo = pow(Lo, 1.0 / GAMMA);
    
    return float4(Lo, basecolor_factor.a);
}