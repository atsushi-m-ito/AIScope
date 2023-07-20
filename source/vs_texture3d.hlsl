
//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer WVPBuffer : register(b0)
{
	matrix world;
	matrix view;
	matrix projection;
};

//--------------------------------------------------------------------------------------
struct VS_INPUT
{
	float3 pos : POSITION;
	float3 normal : NORMAL;
	float3 tex : TEXCOORD;
};

struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 tex : TEXCOORD;         // テクスチャ座標
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	output.pos = float4(input.pos, 1.0f);
	output.pos = mul(output.pos, world);
	output.pos = mul(output.pos, view);
	output.pos = mul(output.pos, projection);

	float4 normal_world = mul(float4(input.normal, 0.0f), world);
	output.normal = mul(normal_world, view).xyz;

	output.tex = input.tex;
	return output;
}

