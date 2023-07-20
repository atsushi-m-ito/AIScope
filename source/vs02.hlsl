cbuffer WVPBuffer : register( b0 )
{
	matrix world;
	matrix view;
	matrix projection;
	float4 m_ambient_color;
}

struct VS_INPUT
{
    float3 pos : POSITION;
    float3 normal : NORMAL;
//    float2 tex : TEXCOORD0;
};


struct PS_INPUT
{
    float4 pos : SV_POSITION;
    float3 normal : NORMAL;
//    float3 color : COLOR;
};

PS_INPUT main( VS_INPUT input)
{
	PS_INPUT output;
	output.pos = float4(input.pos, 1.0f);
    output.pos = mul( output.pos, world );
    output.pos = mul( output.pos, view );
    output.pos = mul( output.pos, projection );
    output.normal = (mul( float4(input.normal,0.0f), world )).xyz;
//	output.color = float3(1.0f, 0.0f, 1.0f);    
    return output;
}
