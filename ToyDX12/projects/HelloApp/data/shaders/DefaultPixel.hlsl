struct VSOutput
{
    float4 PosCS : SV_POSITION;
    float4 PosWS : POSITION;
    float3 NormalWS : NORMAL;
};

float4 main(VSOutput psInput) : SV_TARGET
{
    return float4(normalize(psInput.NormalWS.xyz), 1.0);
}