
//--------------------------------------------------------------------------------------
// File: shader02.fx
// position, normal, WVPBuffer
//--------------------------------------------------------------------------------------


struct PS_INPUT
{
	//SV_POSITION�Z�}���e�B�N�X���w�肷��ƁA���W�͈͂�(-1,1)�ł͂Ȃ��Ȃ�B�V�X�e���̎d�l�ł���A���[�U�[���g�������Ƃ��ɂ́A�ʓrPOSITION�Z�}���e�B�N�X�œn���ׂ�//
    float4 pos : SV_POSITION;  
    float3 normal : NORMAL;
    float4 color : COLOR;
};

//--------------------------------------------------------------------------------------
// Pixel Shader
//--------------------------------------------------------------------------------------
float4 main( PS_INPUT input ) : SV_Target
{
	/*
	static const float3 lightDirection = {0.0f, 0.0f, 0.5f};	//(0.5 * 3^{-0.5})
	
	float scale = 0.5f - dot(input.normal, lightDirection);
	scale = max(scale, 0.0f);
	*/

	static const float3 lightDirection = { 0.0f, 0.0f, -1.0f };

	//float scale = dot(input.normal, lightDirection);
	//scale = (scale - 1.0f) * 1.4f + 1.0f;//
	float scale = dot(input.normal, lightDirection) + 0.25f;
	float4 color = float4(input.color.xyz * scale, input.color.w);
	
    return color;
}

