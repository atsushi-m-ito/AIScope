//////////////////////////////////////////////////////////////
//
//		Wrapper of DirectX11
//
//////////////////////////////////////////////////////////////

#ifndef __sdkdx11_h__
#define __sdkdx11_h__


#pragma once




////////////////////////////////////////////
//DirectXヘッダー
#include <d3d11.h>

#pragma comment( lib, "d3d11.lib" )


//d3dx11_43.dllを必ず遅延読み込みにすること.
//dll名の末尾の数字はSDKとランタイムのバージョンに依存する.


/////////////////////////////////////

#include "CommonShader.h"

#include "release.h"

typedef ID3D11ShaderResourceView*	ROW_TEXTURE;



struct DX11_CORE{
	ID3D11Device*           pD3DDevice;
	ID3D11DeviceContext*    pD3DDeviceContext;
	IDXGISwapChain*         pSwapChain;
	ID3D11RenderTargetView* pRenderTargetView;
	ID3D11DepthStencilView* pDepthStencilView;	// 深度/ステンシル・ビュー

    ID3D11RenderTargetView* pRenderTargetView_HDR;
};





struct TEXTURE_INFO{
	int					width;
	int					height;
};





#endif	//__sdkdx11_h__