
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
	//float2 ParticleSize;  // パーティクルの大きさ
	//float2 dummy;
}


struct VS_INPUT_point_color
{
    float3 pos : POSITION;
	float4 color : COLOR;
};

struct PS_INPUT_point_color
{
    float4 pos : SV_POSITION;
    float4 color : COLOR;
};

PS_INPUT_point_color main(VS_INPUT_point_color input)
{
    PS_INPUT_point_color output;
    float4 pos = float4(input.pos, 1.0f);
    pos = mul(pos, world);
    pos = mul(pos, view);
    pos = mul(pos, projection);
    output.pos = pos;
    output.color = input.color;

    return output;
}
