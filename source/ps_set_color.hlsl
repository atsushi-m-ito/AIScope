
cbuffer ColorBuffer : register(b1){
	float4 m_ambient_color;
}


struct PS_INPUT
{
	float4 pos : SV_POSITION;
	float3 normal : NORMAL;
//	float2 tex : TEXCOORD;         // テクスチャ座標
};


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main(PS_INPUT input) : SV_Target
{
	static const float3 lightDirection = {0.0f,-0.7f,0.7f};//{ 0.288675f, -0.288675f, 0.288675f };	//(0.5 * 3^{-0.5})
	static const float3 materialColor = { 1.0f, 0.0f, 1.0f };

	float scale = 0.5f - dot(input.normal, lightDirection);
	scale = max(scale, 0.0f);

	float4 color = float4(m_ambient_color.xyz * scale, m_ambient_color.w);
    //float4 color = float4(materialColor, 1.0f);

		return color;
}
