
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
	float3 ray : RAY;
	float3 tex : TEXCOORD;         // テクスチャ座標
	float3 org_pos : POSITION0;
	float4 proj_pos : POSITION1;
	float4 ray_z : Z_VIEW_RAY;
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
	output.proj_pos = output.pos;

	output.ray = input.normal;
	output.tex = input.tex;

	//output.view_ray = mul(mul(float4(input.normal, 0.0f), view), projection).xyz;
	output.ray_z.z = mul(float4(input.normal, 0.0f), view).z;
	output.ray_z.x = projection._33;
	//output.ray_z.y = -projection._43 / projection._33;
	output.ray_z.y = projection._43;
	output.ray_z.w = vpos.z;
	

	return output;
}

