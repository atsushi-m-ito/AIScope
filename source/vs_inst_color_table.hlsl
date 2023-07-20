
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

cbuffer ColorTable : register(b1) {
	float4 color_table[9];
}

cbuffer ColorRange : register(b2) {
	float4 color_range;
}

//--------------------------------------------------------------------------------------

struct VS_INSTANCE_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
	float4 inst_pos : INS_POS;
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
	output.pos = float4(input.pos + input.inst_pos.xyz, 1.0f);
    output.pos = mul( output.pos, world );
    output.pos = mul( output.pos, view );
	output.pos = mul(output.pos, projection);
	float4 normal_world = mul(float4(input.normal, 0.0f), world);
	output.normal = mul(normal_world, view).xyz;
	
	//output.color = float4(0.f, 1.f, 0.0f, 1.0f);
	//output.color = color_table[4];
	//output.color = color_range;
	
	float color_z = (input.inst_pos.z - color_range.x) / color_range.y * color_range.z;
	if (color_z > color_range.z) {
		output.color = color_table[int(color_range.z)];
	} else if (color_z <= 0.0f) {
		output.color = color_table[0];
	} else {
		int i_color = int(color_z);
		color_z = color_z - float(i_color);
		output.color = color_table[i_color] * (1.0f- color_z) + color_table[i_color + 1] * color_z;
	}
	
	
    return output;
}
