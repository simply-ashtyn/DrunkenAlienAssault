#pragma pack_matrix(row_major)

struct objectVert
{
    float3 POS;
    float3 UVW;
    float3 NRM;
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
struct InputToRasterizer
{
    float4 posH : SV_POSITION;
    float3 posW : WORLD;
    float3 normW : NORMAL;
};

float4 main(InputToRasterizer inputData) : SV_TARGET
{
    //return float4(material.Kd, 1.0f);
    
    float3 inputNormalized = normalize(inputData.normW);
    float3 lightNormalized = normalize(lightDirection);

    float3 viewDir = normalize(cameraPos - inputData.posW);
    float3 halfVec = normalize((-lightNormalized) + viewDir);
    float intensity = max(pow(clamp(dot(inputNormalized, halfVec), 0, 1), material.Ns), 0);
    float reflectedLight = lightColor * intensity * material.Ks;

    float lightRatio = clamp(dot(-lightNormalized, inputNormalized), -1, 1.1f);

    float3 lightIndirect = float3(lightAmbient * material.Ka);

    float3 lightDirect = float3(lightRatio * lightColor.xyz);

    float3 finalColor = saturate(lightIndirect + lightDirect) * material.Kd + reflectedLight;

    return float4(finalColor, material.d);
}