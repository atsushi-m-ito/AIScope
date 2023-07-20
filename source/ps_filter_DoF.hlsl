

//texture
Texture2D tex2d : register(t0);
Texture2D<float> depth2d : register(t1);
SamplerState samp : register(s0);

cbuffer WVPBuffer : register(b0)
{
	float2 texel_size;  // パーティクルの大きさ
	float2 dummy;
}


struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float2 tex : TEXCOORD;         // テクスチャ座標
};



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 main(PS_INPUT input) : SV_Target
{
	
	const float forcus_depth = 0.88;

	
	float2 tex = input.tex;


	float depth = depth2d.Sample(samp, input.tex);
	if (depth == 1.0f) {
		return tex2d.Sample(samp, tex);
	}

	/*
	float ratio = 1.0f / (1.0f + 30.f * abs(depth - forcus_depth));
	float ratio0 = 1.0f / (1.0f + 30.f * abs(1.0f - forcus_depth));
	ratio = (ratio - ratio0) / (1.0f - ratio0);
	*/
	/*
	float ratio = 1.0f / (1.0f + 10000.f * (depth - forcus_depth)*(depth - forcus_depth));
	float ratio0 = 1.0f / (1.0f + 10000.f * (1.0f - forcus_depth)*(1.0f - forcus_depth));
	ratio = (ratio - ratio0) / (1.0f - ratio0);
	*/

	//float ratio = max(-(depth - forcus_depth)*(depth - forcus_depth) * 100.f + 1.0f, 0.0f);
	float ratio = max(-abs(depth - forcus_depth) * 30.f + 1.0f, 0.0f);


	
	
	
	float4 color1 = float4(0.0f, 0.0f, 0.0f, 0.0f);
	const int width = 5;
	
	float tx = tex.x - texel_size.x * (float)width;
	int i;
	for (i = -width; i < 0; ++i) {
		color1 += tex2d.Sample(samp, float2(tx, tex.y));
		tx += texel_size.x;
	}

	float4 color0 = tex2d.Sample(samp, tex);
	color1 += color0;

	for (i = 1; i <= width; ++i) {
		tx += texel_size.x; 
		color1 += tex2d.Sample(samp, float2(tx, tex.y));		
	}
	




	float4 color = ratio * color0 + (1.0f - ratio) * color1 / (float)(width * 2 + 1);
	//float4 color = float4(ratio, ratio, ratio, 1.0f);
	return color;
}
