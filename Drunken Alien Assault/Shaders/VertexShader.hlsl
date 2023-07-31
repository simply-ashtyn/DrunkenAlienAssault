//#pragma pack_matrix(row_major)
//// an ultra simple hlsl vertex shader
//
//cbuffer sceneData
//{
//    GW::MATH::GVECTORF lightDirection, lightColor;
//    GW::MATH::GMATRIXF viewMatrix, ProjectionMatrix;
//};
//cbuffer meshData
//{
//    GW::MATH::GMATRIXF worldMatrix;
//    OBJ_ATTRIBUTES material;
//};
//
//float4 main(float4 inputVertex : POSITION) : SV_POSITION
//{
//    float4 ans = mul(inputVertex, worldMatrix);
//    ans = mul(ans, viewMatrix);
//    ans = mul(ans, projectionMatrix);
//    return ans;
//}





#pragma pack_matrix(row_major)

struct objectVert
{
    float3 POS;
    float3 UVW;
    float3 NRM;
};
struct OutputToRasterizer
{
    float4 posH : SV_POSITION;
    float3 posW : WORLD;
    float3 normW : NORMAL;
};
struct OBJ_ATTRIBUTES
{
    float3 Kd;
    float d;
    float3 Ks;
    float Ns;
    float3 Ka;
    float sharpness;
    float3 Tf;
    float Ni;
    float3 Ke;
    uint illum;
};
cbuffer sceneData : register(b0)
{
    float4 lightDirection, lightColor, lightAmbient, cameraPos;
    matrix viewMatrix, ProjectionMatrix;
};
cbuffer meshData : register(b1)
{
    matrix worldMatrix;
    OBJ_ATTRIBUTES material;
};

OutputToRasterizer main(float3 inputVertex : POS, float3 inputUV : UVW, float3 inputNorm : NRM)
{
    OutputToRasterizer finalAns;

    float4 ans = float4(inputVertex, 1.0f);
    ans = mul(ans, worldMatrix);
    ans = mul(ans, viewMatrix);
    ans = mul(ans, ProjectionMatrix);
    float3 ans3 = mul(ans, worldMatrix);
    float3 normAns = mul((inputNorm), (float3x3) worldMatrix);
    finalAns.posH = ans;
    finalAns.posW = ans3;
    finalAns.normW = normAns;


    return finalAns;
}