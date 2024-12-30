
/// Input struct of the Vertex shader stage
struct VSInput
{
	[[vk::location(0)]] float3 Position : POSITION;
	[[vk::location(1)]] float3 Normal : NORMAL;
	[[vk::location(2)]] float2 Texcoord0 : TEXCOORD0;
};

/// Output struct of the Vertex shader stage / input struct of the Pixel shader stage
struct PSInput
{
	float4 Position : SV_POSITION;
	float3 WorldNormal : NORMAL;
	float2 Texcoord0: TEXCOORD0;
};


[[vk::binding(0, 0)]] cbuffer Matrices {
	float4x4 modelViewProjectionMatrix;
	float3x3 normalMatrix;
};

/// Output struct of the Pixel shader stage
struct PSOutput
{
	[[vk::location(0)]] float4 Color : COLOR0;
};


PSInput VS_main(VSInput input, uint VertexIndex: SV_VERTEXID)
{
	PSInput output;
	output.Position    = mul(modelViewProjectionMatrix, float4(input.Position, 1.f));
    output.WorldNormal = normalize(mul(normalMatrix, input.Normal));
	output.Texcoord0   = input.Texcoord0;

	return output;
}

struct LightData
{
    float3 color;
    float3 direction;
};

[[vk::binding(1, 0)]] cbuffer LightDataBuffer
{
    LightData lightData;
};


[[vk::binding(2, 0)]]
Texture2D colorTexture;
[[vk::binding(3, 0)]]
SamplerState colorSampler;


PSOutput PS_main(PSInput input)
{
	PSOutput output;
    float4 color = colorTexture.Sample(colorSampler, input.Texcoord0);
    output.Color.rgb = color.rgb * lightData.color * dot(normalize(input.WorldNormal), normalize(lightData.direction));
	output.Color.a	 = 1.f;
	
	return output;
}