
//--------------------------------------------------------------------------------------
// File: PointSprite.fx
// position, WVPBuffer
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer WVPBuffer : register( b0 )
{
	matrix world;
	matrix view;
	matrix projection;
	float2 ParticleSize;  // パーティクルの大きさ
	float2 dummy;
}


struct VS_INPUT_point_color
{
    float3 pos : POSITION;
	float4 color : COLOR;
};


struct GS_INPUT_point_color
{
    float4 pos4 : POSITION4;
	float4 color : COLOR;
};
/*
GS_INPUT_point_color main( VS_INPUT_point_color input)
{
	GS_INPUT_point_color output;
	output.pos = input.pos;
	output.color = input.color;
	
	return output;
}
*/
GS_INPUT_point_color main(VS_INPUT_point_color input)
{
    GS_INPUT_point_color output;
    float4 pos = float4(input.pos, 1.0f);
    pos = mul(pos, world);
    pos = mul(pos, view);
    output.pos4 = pos;
    output.color = input.color;

    return output;
}
