#include "PBR_Common.hlsl"

cbuffer cbMaterial : register(b1)
{
    float4 gBaseColor;
    float gMetallic;
    float gRoughness;
};

cbuffer cbPerPass : register(b2)
{
    // Matrices
    float4x4 gView;
    float4x4 gInvView;
    float4x4 gProj;
    float4x4 gInvProj;
    float3 gEyePosWS;
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
    float3 NormalWS : NORMAL1;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

Texture2D gBaseColorMap : register(t0);
Texture2D gMetallicRoughnessMap : register(t1);
Texture2D gNormalMap : register(t2);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

struct PBRData
{
    float NdotL;
    float NdotH;
    float NdotV;
    float HdotV;
    float MaterialRoughness;
};

struct PointLight
{
    float3 posWS;
};

float4 main(VSOutput psInput) : SV_TARGET
{
    float2 uv = psInput.TexCoord;
    float3 posWS = psInput.PosWS;
    float3 normalWS = gNormalMap.Sample(gsamLinearWrap, uv);
    
    float4 baseColor = gBaseColorMap.Sample(gsamLinearWrap, uv) * gBaseColor;
    float4 metalRough = gMetallicRoughnessMap.Sample(gsamLinearWrap, uv);
    float metallic = metalRough.b * gMetallic;
    float perceptualRoughness = metalRough.g * gRoughness;
    
    PointLight light;
    light.posWS = float3(0, 0, 0);
    
    float3 L = normalize(light.posWS - posWS);
    float3 V = normalize(gEyePosWS - posWS);
    float3 H = normalize(V + L);
    
    PBRData pbrData;
    pbrData.NdotL = saturate(dot(normalWS, L));
    pbrData.NdotH = saturate(dot(normalWS, H));
    pbrData.NdotV = abs(dot(normalWS, V)) + 0.001;
    pbrData.HdotV = saturate(dot(H, V));
    pbrData.MaterialRoughness = perceptualRoughness * perceptualRoughness;
    
    
    // Diffuse BRDF
    float3 cDiff = lerp(baseColor.rgb * (1 - 0.04), 0, metallic);
    float brdfDiffuse = (1 / PI) * pbrData.NdotL;
    
    float3 diffuse = cDiff * brdfDiffuse;
    
    // Specular BRDF : https://www.khronos.org/assets/uploads/developers/library/2017-gtc/glTF-2.0-and-PBR-GTC_May17.pdf
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), baseColor.rgb, metallic);
    float3 F = Schlick(f0, pbrData.HdotV);
    float G = SmithGeometricOcclusion(pbrData.NdotV, pbrData.MaterialRoughness);
    float D = GGX(pbrData.NdotH, pbrData.MaterialRoughness);
    
    float3 brdfSpecular = (F * G * D) / (4 * pbrData.NdotL * pbrData.NdotV);
   
    float3 color = (diffuse + brdfSpecular) * pbrData.NdotL;
    
    return float4(color, 1.0);
}