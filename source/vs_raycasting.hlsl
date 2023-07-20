
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
//	float3 normal: NORMAL; 
	float3 ray : RAY;
	float3 tex : TEXCOORD;         // テクスチャ座標
	float3 org_pos : POSITION;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(VS_INPUT input)
{
	PS_INPUT output;
	float4 vpos = float4(input.pos, 1.0f);
	vpos = mul(vpos, world);	
	output.org_pos = vpos.xyz;
	vpos = mul(vpos, view);
	output.pos = mul(vpos, projection);

//	float4 normal_world = mul(float4(input.normal, 0.0f), world);
//	output.normal = mul(normal_world, view).xyz;
	//here, {view[4], view[8], view[12]} is position of camera in the space after translated by world and view matrixes.//
	//output.ray = vpos.xyz - float3(view._m03, view._m13, view._m23);
	output.ray = input.normal;

	output.tex = input.tex;
	return output;
}

