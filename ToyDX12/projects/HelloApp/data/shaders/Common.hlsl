float3 ComputeNormalWS(float3 T, float3 N, float3 N_NormalMap)
{
    float3x3 TBN = float3x3(T, cross(T, N), N);
    
    return mul(TBN, N_NormalMap);
}