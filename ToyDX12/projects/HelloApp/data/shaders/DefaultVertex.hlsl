// Padding to 256 bytes is implicit 
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbPerPass : register(b1)
{
    // Matrices
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float3   gEyePosWS;
    float pad0;
    float gNear;
    float gFar;
    float gDeltaTime;
    float gTotalTime;
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
    float3 PosOS : POSITION;
    float3 NormalOS : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

struct VSOutput
{
    float4 PosCS : SV_POSITION;
    float4 PosWS : POSITION;
    float3 NormalWS : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

VSOutput main(VSInput input)
{
    float4x4 wvp = mul(mul(gWorld, gView), gProj);
    VSOutput output;

    output.PosCS = mul(float4(input.PosOS, 1.0), wvp);
    output.PosCS = output.PosCS;
    output.NormalWS = mul(float4(input.NormalOS, 0.0), gWorld).xyz;
    output.PosWS =  mul(float4(input.PosOS, 1.0), gWorld);
    output.Tangent = input.Tangent;
    output.TexCoord = input.TexCoord;

    return output;
}