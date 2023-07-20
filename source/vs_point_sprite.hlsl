
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
    float2 ParticleSize;  // パーティクルの大きさ
    float2 dummy;
}


struct VS_INPUT_point
{
    float3 pos : POSITION;
    float alpha : ALPHA;
};


struct GS_INPUT_point
{
    float3 pos : POSITION;
    float alpha : ALPHA;
};




GS_INPUT_point main(VS_INPUT_point input)
{
    GS_INPUT_point output;
    output.pos = input.pos;
    output.alpha = input.alpha;

    return output;
}

