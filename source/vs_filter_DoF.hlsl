
//--------------------------------------------------------------------------------------
// Filter for Depth of Field
//--------------------------------------------------------------------------------------



struct PS_INPUT
{
    float4 pos : SV_POSITION;	
	float2 tex : TEXCOORD;
};

//--------------------------------------------------------------------------------------
// Vertex Shader
//--------------------------------------------------------------------------------------
PS_INPUT main(float3 pos : POSITION, float2 tex : TEX )
{
	PS_INPUT output;
	output.pos = float4(pos, 1.0f);
	output.tex = tex;// float2((pos.x + 1.0f) / 2.0f, (-pos.y + 1.0f) / 2.0f);
    return output;
}
