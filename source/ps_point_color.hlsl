
//--------------------------------------------------------------------------------------
// File: PointSprite.fx
// position, WVPBuffer
//--------------------------------------------------------------------------------------

//texture
Texture2D tex2d : register( t0 );
SamplerState samp : register( s0 );


struct PS_INPUT_point_color
{
    float4 pos : SV_POSITION;
	float4 color : COLOR;
};


//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
/*
float4 PS( PS_INPUT input ) : SV_Target
{
    return tex2d.Sample( samp, input.tex );  // * vMeshColor;
}
*/
float4 main( PS_INPUT_point_color input ) : SV_Target
{
	//static const float4 materialColor = {1.0f, 0.7f, 0.7f, 0.3f};

	float4 color;
	//float coef = 1.0 / (input.pos.z*input.pos.z);
	float coef = input.pos.w*input.pos.w;
	
	color.rgb = input.color.rgb;
	/*
	color.g = materialColor.g * coef;
	color.b = materialColor.b * coef;
	color.a = materialColor.a;
	*/
	//color.rgb = input.color.rgb;
	color.a = input.color.a;
	
    return color;
}
