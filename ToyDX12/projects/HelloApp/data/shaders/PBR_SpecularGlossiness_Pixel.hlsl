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
    float3 NormalWS : NORMAL1;
    float3 Tangent : TANGENT;
    float2 TexCoord : TEXCOORD;
};

// Specular/Glossiness Workflow
Texture2D gDiffuseMap : register(t0);     
Texture2D gSpecGlossMap : register(t1);

Texture2D gBaseColor : register(t2);     
Texture2D gMetallicRoughness : register(t3);

Texture2D gNormalMap : register(t2);

SamplerState gsamPointWrap : register(s0);
SamplerState gsamPointClamp : register(s1);
SamplerState gsamLinearWrap : register(s2);
SamplerState gsamLinearClamp : register(s3);
SamplerState gsamAnisotropicWrap : register(s4);
SamplerState gsamAnisotropicClamp : register(s5);

float NormalDistributionFunction(float NdotH, float m)
{
    // NdotH : angle between micro-facet normal H and macro normal N
    // m = roughness [0, 1]; Amount/Fraction of microfacets with normal H that make an angle NdotH with the macro-normal N

    return ((m + 8.0) / 8.0) * pow(NdotH, m);
}

float Schlick(float F0, float cosTheta)
{
    return F0 + (1 - F0) * pow((1 - cosTheta), 5);
}

float Fresnel(float F0, float3 cDiffuse, float HdotV, float NdotH, float roughness)
{
    // HdotV : angle between micro-facet normal H and view direction V
    return Schlick(F0, HdotV) * NormalDistributionFunction(NdotH, roughness);
}


float4 main(VSOutput psInput) : SV_TARGET
{
    float3 E = gEyePosWS;           // Eye position
    float3 PL = float3(-1,2,1); // Point light position
    float3 p = psInput.PosWS.xyz;       // Point on surface position 
    float3 N = normalize(psInput.NormalOS) * 2 - 1;
    
    float3 V = normalize(E - p);
    float3 L = normalize(PL - p);
    float3 H = normalize(L + V); // Halfway vector; micro-facet normal   
    
    // Temporary way to calculate Tangent
    float3 Q1 = ddx(p);
	float3 Q2 = ddy(p);
    
	float2 st1 = ddx(psInput.TexCoord);
	float2 st2 = ddy(psInput.TexCoord);
    float3 T = normalize(Q1*st2.x - Q2*st1.x);
    ////////////////////////////////////////////
    float3x3 TBN = float3x3(T, cross(T, psInput.NormalOS), normalize(psInput.NormalOS));

    float3 N_Map = gNormalMap.Sample(gsamLinearWrap, psInput.TexCoord).rgb;
    N = normalize(mul(N_Map, TBN));
    float NdotL = max(0, dot(N, L));
    float NdotH = max(0, dot(N, H));
    float HdotV = max(0, dot(H, V));

    // Specular = BaseColor * Metalness
    // Albedo   = BaseColor * (1-Metallness);

    // Input textures
    float4 diffuse   = gDiffuseFactor * gDiffuseMap.Sample(gsamLinearWrap, psInput.TexCoord);

    float4 specGloss = gSpecGlossMap.Sample(gsamLinearWrap, psInput.TexCoord);
    float3 specular  = gSpecularFactor * specGloss.rgb;
    float glossiness = gGlossinessFactor * specGloss.a; 
    float alpha  = (1 - glossiness) * (1 - glossiness);

    // BRDF inputs
    // https://github.com/KhronosGroup/glTF/blob/main/extensions/2.0/Archived/KHR_materials_pbrSpecularGlossiness/README.md#specular---glossiness
    float F0    = specular.r; // Percentage of incoming light that is reflected when the angle between light and surface normal is 0 (NdotL = 0; light hits the surface perpendicularly)

    float roughness = 1 - glossiness;
    float3 AL = float3(0.27, 0.27, 0.27);// Quantity of ambient light  
    float3 BL = float3(0.8, 0.8, 0.8);// Quantity of incoming light
    float3 md = diffuse.rgb;     // Diffuse albedo   

    float3 cAmbient  = AL * md;
    float3 cDiffuse  = BL * NdotL;
    float3 cSpecular = md + Fresnel(F0, cDiffuse, HdotV, NdotH, roughness) ;
    
    float3 color = cAmbient + cDiffuse * cSpecular ;//+ cDiffuse;// + cSpecular;

    return float4(color, 1.0);
}