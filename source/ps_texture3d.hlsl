

//texture
Texture3D tex3d : register(t0);
SamplerState samp : register(s0);


struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
	float3 tex : TEXCOORD;         // テクスチャ座標
};



//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------

float4 main(PS_INPUT input) : SV_Target
{
	//static const float3 lightDirection = { 0.0f,0.0f,1.0f }; //{ 0.288675f, -0.288675f, 0.288675f };	//(0.5 * 3^{-0.5})
	//static const float3 materialColor = { 1.0f, 0.0f, 1.0f };

	//float scale = 0.5f - dot(input.normal, lightDirection);
	//scale = max(scale, 0.0f);

	float4 color = tex3d.Sample(samp, input.tex);  // * vMeshColor;
	////color.a = 1.0f;
	//float4 color = float4(input.tex, 1.0f);  // * vMeshColor;

	return color;
}
