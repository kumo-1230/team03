struct VS_IN
{
    float4 position : POSITION;
    float4 normal : NORMAL;
    float4 tangent : TANGENT;
    float2 texcoord : TEXCOORD;
    float4 bone_weights : BONE_WEIGHTS;
    uint4 bone_indices : BONE_INDICES;
};

struct VS_OUT
{
    float4 position : SV_POSITION;
    float4 w_position : POSITION;
    float4 w_normal : NORMAL;
    float4 w_tangent : TANGENT;
    float2 texcoord : TEXCOORD;
};

cbuffer PRIMITIVE_CONSTANT_BUFFER : register(b0)
{
    row_major float4x4 world;
    int material;
    bool has_tangent;
    int skin;
    int pad;
};

cbuffer SCENE_CONSTANT_BUFFER : register(b1)
{
    row_major float4x4 view_projection;
    float4 light_direction;
    float4 camera_position;
};

VS_OUT main(VS_IN vin)
{
    VS_OUT vout;
    
    vin.position.w = 1;
    vout.position = mul(vin.position, mul(world, view_projection));
    vout.w_position = mul(vin.position, world);
    
    vin.normal.w = 0;
    vout.w_normal = normalize(mul(vin.normal, world));
    
    float sigma = vin.tangent.w;
    vin.tangent.w = 0;
    vout.w_tangent = normalize(mul(vin.tangent, world));
    vout.w_tangent.w = sigma;
    
    vout.texcoord = vin.texcoord;
    
    return vout;
}