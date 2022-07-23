static const float PI = 3.14159265f;

float NormalDistributionFunction(float NdotH, float m)
{
    // NdotH : angle between micro-facet normal H and macro normal N
    // m = roughness [0, 1]; Amount/Fraction of microfacets with normal H that make an angle NdotH with the macro-normal N
    return ((m + 8.0) / 8.0) * pow(NdotH, m);
}

float GGX(float NdotH, float r)
{
    const float r2 = r * r;
    const float d = (NdotH * NdotH) * (r2 - 1) + 1;
    
    return (r2 * r2) / (PI * d * d);
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

float SmithGeometricOcclusion(float NdotV, float r)
{
    const float r2 = r * r;
    return (2 * r2) / (NdotV + sqrt(r2 + (1 - r2) * (NdotV * NdotV)));
}
