struct VSOutput
{
    float4 PosCS : SV_POSITION;
    float4 PosWS : POSITION;
    float3 NormalOS : NORMAL;
    float3 NormalWS : NORMAL1;
    float4 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

float4 main(VSOutput psInput) : SV_TARGET
{
    return float4(psInput.TexCoord, 0.0, 1.0);
}