// ------------------------------------------------------------
// Uniforms
// ------------------------------------------------------------
Texture2D baseColorTexture;
SamplerState baseColorSampler;

cbuffer LightingParams : register(b0)
{
    float3 lightDirection;   // normalized
    float3 lightColor;
}

cbuffer TransformParams : register(b1)
{
    float4x4 modelViewProjectionMatrix;
    float3x3 normalMatrix;
}

// ------------------------------------------------------------
// Vertex Input / Output
// ------------------------------------------------------------
struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

struct VSOutput
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

// ------------------------------------------------------------
// Vertex Shader
// ------------------------------------------------------------
VSOutput vsMain(VSInput input)
{
    VSOutput output;
    output.position = mul(modelViewProjectionMatrix, float4(input.position, 1.0));
    output.normal   = mul(normalMatrix, input.normal);
    output.uv       = input.uv;
    return output;
}

// ------------------------------------------------------------
// Fragment Shader
// ------------------------------------------------------------
float4 psMain(VSOutput input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 L = normalize(lightDirection);

    float NdotL = max(dot(N, L), 0.0);

    float3 baseColor = baseColorTexture.Sample(baseColorSampler, input.uv).rgb;
    float3 litColor  = baseColor * lightColor * NdotL;

    return float4(litColor, 1.0);
}
