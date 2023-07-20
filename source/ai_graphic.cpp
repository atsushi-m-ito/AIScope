#if 0

#include "ai_graphic.h"


static int sge_type = 0;	//初期値//



struct DX11_CORE{
	ID3D11Device*			pd3dDevice;
	ID3D11DeviceContext*	pd3dDC;
	IDXGISwapChain*			pSwapChain;
	ID3D11RenderTargetView*	pRenderTargetView;
	ID3D11DepthStencilView*	pDepthStencilView;	// 深度/ステンシル・ビュー
//	CommonShader*			comShader;
//	CommonTexture*			comTexture;
};

DX11_CORE* pdx11core = NULL;



int aigInitialize(int graphictype, void* params){


	return 0;

}


///////////////////////////////////////
//以下は selective graphic engine (SGE)
///////////////////////////////////////

int sgeInitialize(int graphictype, void* params){

	sge_type = graphictype;

	switch(sge_type){
	case SGE_TYPE_GL15:
	
	
		break;
	case SGE_TYPE_DX11:
		
		pdx11core = (DX11_CORE*)params;
		break;
	}
	return 0;
}

//////////////////////////////////////////////
//
//RenderTargetViewとDepthStencilViewのクリア
//
/////////////////////////////////////////////////
int sgeClearScreenBuffer(float* color4){

	switch(sge_type){
	case SGE_TYPE_GL15:

		glClearColor(color4[0], color4[1], color4[2], color4[3]);	//画面クリア時の背景色を設定
		//glClearColor(0.0, color4[1], color4[2], color4[3]);	//画面クリア時の背景色を設定
		
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//画面を背景色でクリア

		break;
	case SGE_TYPE_DX11:
	
		
		pdx11core->pd3dDC->ClearRenderTargetView( pdx11core->pRenderTargetView, color4 );
	    
		//DepthBufferのクリア
		pdx11core->pd3dDC->ClearDepthStencilView( pdx11core->pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);
	
		break;
	}

	return 0;
}

#endif