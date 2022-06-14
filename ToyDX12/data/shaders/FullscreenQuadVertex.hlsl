// Padding to 256 bytes is implicit 
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorldViewProj;
};

// Other syntax (since shader model 5.1)
// struct PerObjectConstants
// {
//     float4x4 gWorldViewProj;
//     uint matIndex;
// };
// ConstantBuffer<PerObjectConstants> gObjConstants : register(b0);

struct VSInput
{
    float3 Pos : POSITION;
    float2 TexCoord : TEXCOORD0;
};

struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

VSOutput main(VSInput input)
{
    VSOutput output;
    output.Pos = float4(input.Pos, 1.0);
    output.TexCoord = input.TexCoord;
    
    return output;
}