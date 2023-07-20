

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
};

//texture
Texture2D tex2d : register(t0);
SamplerState samp : register(s0);


//--------------------------------------------------------------------------------------

struct GS_INPUT_point
{
    float3 pos : POSITION;
    float alpha : ALPHA;
};

struct PS_INPUT_point
{
    float4 pos : SV_POSITION;
    float alpha : ALPHA;
};

//--------------------------------------------------------------------------------------
// Geometry Shader
//--------------------------------------------------------------------------------------
[maxvertexcount(4)]
void main(point GS_INPUT_point input[1], 
    inout TriangleStream<PS_INPUT_point> ParticleStream)
{

    float4 pos = float4(input[0].pos, 1.0);
    pos = mul(pos, world);
    pos = mul(pos, view);


    float4 posLT = pos + float4(-ParticleSize.x, ParticleSize.y, 0.0, 0.0) * pos.w;
    float4 posLB = pos + float4(-ParticleSize.x, -ParticleSize.y, 0.0, 0.0) * pos.w;
    float4 posRT = pos + float4(ParticleSize.x, ParticleSize.y, 0.0, 0.0) * pos.w;
    float4 posRB = pos + float4(ParticleSize.x, -ParticleSize.y, 0.0, 0.0) * pos.w;

    // 左上の点の位置(射影座標系)を計算して出力.
        /*  PS_INPUT output; */
    PS_INPUT_point output;
    output.alpha = input[0].alpha;
    
    output.pos = mul(posLT, projection);
    //    output.tex = float2(0.0, 0.0);
    ParticleStream.Append(output);

    // 右上の点の位置(射影座標系)を計算して出力.
    output.pos = mul(posRT, projection);
    //    output.tex = float2(1.0, 0.0);
    ParticleStream.Append(output);

    // 左下の点の位置(射影座標系)を計算して出力.
    output.pos = mul(posLB, projection);
    //    output.tex = float2(0.0, 1.0);
    ParticleStream.Append(output);

    // 右下の点の位置(射影座標系)を計算して出力.
    output.pos = mul(posRB, projection);
    //    output.tex = float2(1.0, 1.0);
    ParticleStream.Append(output);

    ParticleStream.RestartStrip();

}
