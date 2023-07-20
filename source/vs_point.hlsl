
//--------------------------------------------------------------------------------------
// File: PointSprite.fx
// position, WVPBuffer
//--------------------------------------------------------------------------------------

//--------------------------------------------------------------------------------------
// Constant Buffer Variables
//--------------------------------------------------------------------------------------
cbuffer WVPBuffer : register(b0)
{
    matrix world;
    matrix view;
    matrix projection;
    //float2 ParticleSize;  // パーティクルの大きさ
    //float2 dummy;
}


struct VS_INPUT_point
{
    float3 pos : POSITION;
    //float4 color : COLOR;
};


struct GS_INPUT_point
{
    float3 pos : POSITION;
    //float4 color : COLOR;
};

struct PS_INPUT_point
{
    float4 pos : SV_POSITION;
    //float4 color : COLOR;
};

PS_INPUT_point main(VS_INPUT_point input)
{
    PS_INPUT_point output;

    output.pos = float4(input.pos, 1.0f);
    output.pos = mul(output.pos, world);
    output.pos = mul(output.pos, view);
    //float z = output.pos.z;
    output.pos = mul(output.pos, projection);
    //output.pos.w = z;
    return output;
}





