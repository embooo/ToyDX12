#include "Common.hlsl"
#include "PBR_Common.hlsl"

#define USE_DIR_LIGHT
//#define USE_TBN
#define GAMMA_CORRECT

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
    float LdotH;
    float MaterialRoughness;
};

struct PointLight
{
    float3 dirWS;
    float3 posWS;
    float  attenuation;
    float  radius;
};

float4 main(VSOutput psInput) : SV_TARGET
{
    float2 uv = psInput.TexCoord;
    float3 posWS = psInput.PosWS.xyz;

    float3 E = gEyePosWS;//mul(float4(0,0,0,1.0), gInvView).xyz;
    
    float3 N = normalize(psInput.NormalWS);
    float3 T = normalize(psInput.Tangent.xyz);
    float3 V = normalize(E - posWS);
    
    
#ifdef USE_TBN
    float3 N_NormalMap = normalize(gNormalMap.Sample(gsamLinearWrap, uv).xyz) * 2.0 - 1.0;
    N = normalize(ComputeNormalWS(T, N, N_NormalMap));
#endif

    float4 baseColor = saturate(gBaseColorMap.Sample(gsamLinearWrap, uv) * gBaseColor);

#ifdef GAMMA_CORRECT
    baseColor = pow(baseColor, 2.2);
#endif
    float4 metalRough = gMetallicRoughnessMap.Sample(gsamLinearWrap, uv);
    float metallic = metalRough.b * gMetallic;
    float perceptualRoughness = metalRough.g * gRoughness;

    PointLight light;
    light.dirWS = float3(0, 1, 0);
    light.posWS = float3(0,0,-20);
    light.attenuation = 1.0f;
    light.radius = 1.0f;

#ifdef USE_DIR_LIGHT
    float3 L = normalize(light.dirWS);
#else
    float3 d = posWS - light.posWS;
    light.radius = 2.0f;
    light.attenuation = 1.0f / (dot(d,d) + (light.radius*light.radius) / 2.0f);
    
    float3 L = normalize(light.posWS - posWS);
#endif
    float3 H = normalize(V + L);

    PBRData pbrData;
    pbrData.NdotL = max(0.001, dot(N, L));
    pbrData.NdotH = saturate(dot(N, H));
    pbrData.NdotV = saturate(dot(N, V));
    pbrData.HdotV = saturate(dot(H, V));
    pbrData.LdotH = saturate(dot(L, H));

    pbrData.MaterialRoughness = perceptualRoughness * perceptualRoughness;

    // Diffuse BRDF
    float F0 = 0.04;
    float3 cDiff =  lerp(baseColor.rgb * (1 - F0), 0, metallic);

    float3 diffuse = (cDiff / PI) * pbrData.NdotL;

    // Specular BRDF : https://www.khronos.org/assets/uploads/developers/library/2017-gtc/glTF-2.0-and-PBR-GTC_May17.pdf
    float3 f0 = lerp(float3(0.04, 0.04, 0.04), baseColor.rgb, metallic);
    float3 F = Schlick(f0, pbrData.LdotH);
    float G = SmithGeometricOcclusion(pbrData.NdotL, pbrData.NdotV, pbrData.MaterialRoughness);
    
    float D = GGX(pbrData.NdotH, pbrData.MaterialRoughness);

    float3 specular = (F * G * D) / (4.0f * pbrData.NdotL * pbrData.NdotV);

    // Ambient
    float3 ambient = baseColor.rgb * 0.27;

    float3 color =  20 * (lerp(diffuse, specular, Schlick(f0, pbrData.HdotV))  * pbrData.NdotL) * light.radius * light.attenuation;
    color = saturate(color);
#ifdef GAMMA_CORRECT
    color = pow(color, 1.0f/2.2);
#endif
    
    float4 debugProps = float4(pbrData.NdotL, pbrData.MaterialRoughness, perceptualRoughness, metallic);
    
    return float4(color , 1.0);
    //return float4(N, 1);
    //return float4(N, 1);
    
    //return float4(pbrData.NdotL,pbrData.NdotL,pbrData.NdotL, 1.0);
    //return float4(f0, 1.0);

    //return float4(pbrData.NdotL,pbrData.NdotL,pbrData.NdotL, 1.0);
}