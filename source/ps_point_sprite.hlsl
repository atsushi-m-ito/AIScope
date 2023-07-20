
//--------------------------------------------------------------------------------------
// File: PointSprite.fx
// position, WVPBuffer
//--------------------------------------------------------------------------------------

//texture
Texture2D tex2d : register(t0);
SamplerState samp : register(s0);


struct PS_INPUT_point
{
    float4 pos : SV_POSITION;
    float alpha : ALPHA;
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
float4 main(PS_INPUT_point input) : SV_Target
{
    static const float4 materialColor = {0.2f, 0.3f, 0.5f, 1.0f/16.0f};

    float4 color;
    //float coef = 1.0 / (input.pos.z*input.pos.z);
    
    //const float coef = input.pos.w*input.pos.w;
    //const float coef = 1.0f;// (input.pos.w * input.pos.w);// *input.pos.w);
    /*const float coef = 1.0f / (input.pos.z + 1.0f);// *input.pos.w);// *input.pos.w);
    //const float coef = 1.0f / (input.pos.z*input.pos.z*input.pos.z*input.pos.z);
    color.rgb = materialColor.rgb * coef * coef;
    color.a = materialColor.a;// *coef*coef;
    */

    color.rgb = materialColor.rgb;
    color.a = materialColor.a *input.alpha;



    return color;
}

