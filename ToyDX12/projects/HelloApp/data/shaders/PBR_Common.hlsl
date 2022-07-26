static const float PI = 3.14159265f;

float NormalDistributionFunction(float NdotH, float m)
{
    // NdotH : angle between micro-facet normal H and macro normal N
    // m = roughness [0, 1]; Amount/Fraction of microfacets with normal H that make an angle NdotH with the macro-normal N
    return ((m + 8.0) / 8.0) * pow(NdotH, m);
}

float GGX(float NdotH, float r)
{
    float r2 = r * r;
    float d = ( (NdotH * (r2 - 1)) * NdotH + 1) ;

    return r2 / (PI * d * d);
}

float3 Schlick(float3 F0, float cosTheta)
{
    return F0 + (float3(1.0f, 1.0f, 1.0f) - F0) * pow((1.0f - cosTheta), 5.0f);
}

float3 Fresnel(float F0, float3 cDiffuse, float HdotV, float NdotH, float roughness)
{
    // HdotV : angle between micro-facet normal H and view direction V
    return Schlick(F0, HdotV) * NormalDistributionFunction(NdotH, roughness);
}

float SmithGeometricOcclusion(float NdotL, float NdotV, float r)
{
    float alpha = r * r;
    float alphaSq = alpha * alpha;

    float G1_NdotV = (2.0f * NdotV) / (NdotV + sqrt(alphaSq + (1 - alphaSq) * (NdotV * NdotV)));
    float G1_NdotL = (2.0f * NdotL) / (NdotL + sqrt(alphaSq + (1 - alphaSq) * (NdotL * NdotL)));

    return G1_NdotV * G1_NdotL;
}
