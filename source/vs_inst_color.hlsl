
//--------------------------------------------------------------------------------------
// File: shader02.fx
// position, normal, WVPBuffer
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer WVPBuffer : register( b0 )
{
	matrix world;
	matrix view;
	matrix projection;
	float4 m_ambient_color;
}
//--------------------------------------------------------------------------------------

struct VS_INSTANCE_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
//	float4 color : COLOR;
//	float4 inst_pos_color : IPOS;
	float3 inst_pos : INS_POS;
	float4 inst_color : INS_COLOR;
};

struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
	float4 color : COLOR;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main( VS_INSTANCE_INPUT input )
{
	PS_INPUT output;
	output.pos = float4(input.pos + input.inst_pos, 1.0f);
    output.pos = mul( output.pos, world );
    output.pos = mul( output.pos, view );
	output.pos = mul(output.pos, projection);
	float4 normal_world = mul(float4(input.normal, 0.0f), world);
	output.normal = mul(normal_world, view).xyz;
	//output.color = input.color;
	output.color = input.inst_color;
    return output;
}
