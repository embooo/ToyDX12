// Padding to 256 bytes is implicit 
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    float4x4 gView;
    float4x4 gProj;
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
    float3 NormalOS : NORMAL;

};

struct VSOutput
{
    float4 PosCS : SV_POSITION;
    float4 PosWS : POSITION;
    float3 NormalWS : NORMAL;
};

VSOutput main(VSInput input)
{
    float4x4 wvp = mul(mul(gWorld, gView), gProj);
    VSOutput output;

    output.PosCS = mul(float4(input.Pos, 1.0), wvp);
    output.NormalWS = mul(float4(input.NormalOS, 0.0), gWorld);
    output.PosWS =  mul(float4(input.Pos, 1.0), gWorld);

    return output;
}