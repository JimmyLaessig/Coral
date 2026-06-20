// ------------------------------------------------------------
// Uniforms
// ------------------------------------------------------------

struct Matrices 
{
    float4x4 modelViewProjectionMatrix;
    float3x3 normalMatrix;
};

struct LightData
{
    float4 lightDirection;
    float4 lightColor;
};

ConstantBuffer<Matrices> matrices   : register(b0);
ConstantBuffer<LightData> lightData : register(b1);

Sampler2D baseColorTexture    : register(t2);

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
[shader("vertex")]
VSOutput vsMain(VSInput input)
{
    VSOutput output;
    output.position = mul(matrices.modelViewProjectionMatrix, float4(input.position, 1.0));
    output.normal   = mul(matrices.normalMatrix, input.normal);
    output.uv       = input.uv;
    return output;
}

// ------------------------------------------------------------
// Fragment Shader
// ------------------------------------------------------------
[shader("fragment")]
float4 psMain(VSOutput input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 L = normalize(lightData.lightDirection.xyz);

    float NdotL = max(dot(N, L), 0.f);

    float3 baseColor = baseColorTexture.Sample(input.uv).rgb;
    float3 litColor  = baseColor * lightData.lightColor.xyz * NdotL;

    return float4(litColor, 1.f);
}
