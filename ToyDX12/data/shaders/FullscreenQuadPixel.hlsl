struct VSOutput
{
    float4 Pos : SV_POSITION;
    float2 TexCoord : TEXCOORD;
};

float4 main(VSOutput psInput) : SV_TARGET
{
    return float4(psInput.TexCoord, 0.0, 1.0);
}