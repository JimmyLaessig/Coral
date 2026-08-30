
/*!
 * Camera draw parameters
 */
struct CameraParams 
{
    float4x4 viewProjectionMatrix;
};

/*!
 * Light draw parameters
 */
struct LightParams
{
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
ConstantBuffer<CameraParams> cameraParams;

/*!
 * Uniform buffer containing the current instance's parameters
 */
[[vk::binding(1, 0)]]
ConstantBuffer<InstanceParams> instanceParams;

/*!
 * Uniform buffer containing FrameParams for the frame
 */
[[vk::binding(2, 0)]]
ConstantBuffer<LightParams> lightParams;

/*!
 * Sampler containing the base color texture
 */
[[vk::binding(3, 0)]]
Sampler2D baseColorTexture;

/*!
 * Vertex shader input
 */
struct VSInput
{
    float3 position : POSITION;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

/*!
 * Vertex shader output
 */
struct VSOutput
{
    float4 position : SV_Position;
    float3 normal   : NORMAL;
    float2 uv       : TEXCOORD0;
};

/*!
 * Vertex Shader
 */
[shader("vertex")]
VSOutput vertexShader(VSInput input)
{
    float4x4 modelViewProjectionMatrix = mul(cameraParams.viewProjectionMatrix, instanceParams.modelMatrix);

    VSOutput output;
    output.position = mul(modelViewProjectionMatrix, float4(input.position, 1.0));
    output.normal   = normalize(input.normal);
    output.uv       = input.uv;
    return output;
}

/*!
 * Fragment shader
 */
[shader("fragment")]
float4 fragmentShader(VSOutput input) : SV_Target
{
    float3 N = normalize(input.normal);
    float3 L = normalize(lightParams.lightDirection.xyz);

    float NdotL      = max(dot(N, L), 0.f);
    float3 baseColor = baseColorTexture.Sample(input.uv).rgb;
    float3 litColor  = baseColor * (lightParams.lightColor.xyz * NdotL + lightParams.ambientColor);
    return float4(litColor, 1.f);
}
