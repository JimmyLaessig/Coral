/*!
 * Per-frame draw parameters
 */
struct FrameParams 
{
    float4x4 viewProjectionMatrix;
    float3 lightDirection;
    float3 lightColor;
    float3 ambientColor;
};

/*!
 * Per-instance draw parameters
 */
struct InstanceParams
{
    float4x4 modelMatrix;
    float3x3 normalMatrix;
};

/*!
 * Uniform buffer containing FrameParams for the frame
 */
[[vk::binding(0, 0)]]
ConstantBuffer<FrameParams> frameParams;

/*!
 * Sampler containing the base color texture
 */
[[vk::binding(4, 0)]]
Sampler2D baseColorTexture;

/*!
 * Pixel shader input
 */
struct PSInput
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

/*!
 * Pixel shader
 */
[shader("fragment")]
float4 main(PSInput input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 L = normalize(frameParams.lightDirection.xyz);

    float NdotL      = max(dot(N, L), 0.f);
    float3 baseColor = baseColorTexture.Sample(input.uv).rgb;
    float3 litColor  = baseColor * (frameParams.lightColor.xyz * NdotL + frameParams.ambientColor);
    return float4(litColor, 1.f);
}
