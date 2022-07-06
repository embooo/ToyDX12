cbuffer cbMaterial : register(b1)
{
    float4 gDiffuseFactor;	    // DiffuseAlbedo
	float3 gSpecularFactor;		// F0
	float  gGlossinessFactor;	// 1 - Roughness
};

cbuffer cbPerPass : register(b2)
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

struct VSOutput
{
    float4 PosCS : SV_POSITION;
    float4 PosWS : POSITION;
    float3 NormalOS : NORMAL;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

float4 main(VSOutput psInput) : SV_TARGET
{
    float3 N = normalize(psInput.NormalOS);
    float3 V = normalize(gEyePosWS - psInput.PosWS);
    float3 L = normalize(float3(-2,2,-2) - psInput.PosWS);

    float NdotL = max(0, dot(N, L));

    float3 color = gDiffuseFactor * NdotL;

    return float4(color, 1.0);
}