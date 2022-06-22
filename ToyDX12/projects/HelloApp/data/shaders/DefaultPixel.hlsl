struct VSOutput
{
    float4 PosCS : SV_POSITION;
    float3 NormalWS : NORMAL;
};

float4 main(VSOutput psInput) : SV_TARGET
{
    return float4(psInput.NormalWS.xyz, 1.0);
}