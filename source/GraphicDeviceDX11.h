#pragma once

#define WIN32_LEAN_AND_MEAN
#include "IGraphicDevice.h"
#include <Windows.h>
////////////////////////////////////////////
//DirectXヘッダー
#include <d3d11.h>
#pragma comment( lib, "d3d11.lib" )

#include "RenderDX11.h"


class GraphicDeviceDX11 : public IGraphicDevice
{
private:
	// DirectX 11
	HWND m_hWnd;

	int m_vsync; //pSwapChain->Presentメソッドの第一引数.0なら垂直同期しない, 1なら垂直同期.

	D3D_DRIVER_TYPE         m_driverType;
	D3D_FEATURE_LEVEL       m_featureLevel;
	ID3D11Device*           m_pD3DDevice;
	ID3D11DeviceContext*    m_pDeviceContext;
	IDXGISwapChain*         m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11Texture2D*          m_pDepthStencil;		// 深度/ステンシル
	ID3D11DepthStencilView*   m_pDepthStencilView;	// 深度/ステンシル・ビュー

    ID3D11Texture2D*          m_pRenderTarget_HDR;
    ID3D11RenderTargetView* m_pRenderTargetView_HDR;
    ID3D11ShaderResourceView* m_pTextureRV_HDR;

	UINT m_buffer_count;
	const DXGI_FORMAT m_buffer_format;

    const int m_HDR_flag;

public:
	GraphicDeviceDX11(HWND hWnd, DXGI_FORMAT dxgi_format) :
		m_hWnd(hWnd),
		m_driverType(D3D_DRIVER_TYPE_NULL),
		m_featureLevel(D3D_FEATURE_LEVEL_11_0),
		m_pD3DDevice(NULL),
		m_pDeviceContext(NULL),
		m_pSwapChain(NULL),
		m_pRenderTargetView(NULL),
		m_pDepthStencil(NULL),		// 深度/ステンシル
		m_pDepthStencilView(NULL),	// 深度/ステンシル・ビュー		
		m_buffer_count(2),	//3D vision 用に2を選択
        //m_buffer_format(DXGI_FORMAT_R16G16B16A16_FLOAT),//DXGI_FORMAT_R16G16B16A16_UNORM,DXGI_FORMAT_R8G8B8A8_UNORM)DXGI_FORMAT_R16G16B16A16_FLOAT
		//m_buffer_format(DXGI_FORMAT_R8G8B8A8_UNORM),//DXGI_FORMAT_R16G16B16A16_UNORM,DXGI_FORMAT_R8G8B8A8_UNORM)DXGI_FORMAT_R16G16B16A16_FLOAT
		m_buffer_format(dxgi_format),
        m_HDR_flag(0),
        m_pRenderTarget_HDR(NULL),
        m_pRenderTargetView_HDR(NULL),
        m_pTextureRV_HDR(NULL)
    {}; 

	~GraphicDeviceDX11(){
		TerminateDX11();
		
		mTerminateDX11();
		
		
	};

	int InitializeWindow(int width, int height, int init_windowmode, int vsync_mode){

		//DX11の初期化//
		int result = mInitializeDX11( width, height, init_windowmode);
		if (result){
			return result;
		}

		//common設定(将来的に廃止予定)//
		DX11_CORE dx11_core;
		dx11_core.pD3DDevice = m_pD3DDevice;
		dx11_core.pD3DDeviceContext = m_pDeviceContext;
		dx11_core.pSwapChain = m_pSwapChain;
		dx11_core.pRenderTargetView = m_pRenderTargetView;
		dx11_core.pDepthStencilView = m_pDepthStencilView;
		InitDX11(dx11_core);


		//垂直同期設定
		if (vsync_mode == 1){
			m_vsync = 1;
		} else{
			m_vsync = 0;
		}
		return 0;
	};

	void BeginRendering()
	{
		m_pDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	};

	void EndRendering()
	{
		m_pSwapChain->Present(m_vsync, 0);	//第一引数が0なら垂直同期しない, 1なら垂直同期.
	};


	void Resize(int width, int height)
	{

		mReleaseRenderTarget();

		HRESULT hr = m_pSwapChain->ResizeBuffers(m_buffer_count, width, height, m_buffer_format, 0);

		mCreateRenderTarget();


		//common設定(将来的に廃止予定)//
		DX11_CORE dx11_core;
		dx11_core.pD3DDevice = m_pD3DDevice;
		dx11_core.pD3DDeviceContext = m_pDeviceContext;
		dx11_core.pSwapChain = m_pSwapChain;
		dx11_core.pRenderTargetView = m_pRenderTargetView;
		dx11_core.pDepthStencilView = m_pDepthStencilView;
		ResetDX11(dx11_core);

	};

private:
	int mInitializeDX11(int width, int height, int init_windowmode){

		HRESULT hr = S_OK;
		const HWND hWnd = m_hWnd;

		/*
		RECT rc;
		GetClientRect(hWnd, &rc);
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;
		*/
		UINT createDeviceFlags = 0;
		
		//Windows 8.1 is not supported
/*		#ifdef _DEBUG
		createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
		#endif
*/		
		D3D_DRIVER_TYPE driverTypes[] =
		{
			D3D_DRIVER_TYPE_HARDWARE,		//HAL
			D3D_DRIVER_TYPE_WARP,			//高機能ソフトウェアレンダリング.
			D3D_DRIVER_TYPE_REFERENCE,		//reference
		};
		UINT numDriverTypes = ARRAYSIZE(driverTypes);

		D3D_FEATURE_LEVEL featureLevels[] =
		{
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
		};
		UINT numFeatureLevels = ARRAYSIZE(featureLevels);

		DXGI_SWAP_CHAIN_DESC sd;
		ZeroMemory(&sd, sizeof(sd));
		sd.BufferCount = m_buffer_count;				//3D vision の為には2を設定.
		sd.BufferDesc.Width = width;
		sd.BufferDesc.Height = height;
		sd.BufferDesc.Format = m_buffer_format;
		sd.BufferDesc.RefreshRate.Numerator = 60;
		sd.BufferDesc.RefreshRate.Denominator = 1;
		sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		sd.OutputWindow = hWnd;
		sd.SampleDesc.Count = 1;
		sd.SampleDesc.Quality = 0;
		if (init_windowmode){
			sd.Windowed = TRUE;	//TRUE(window mode), FALSE(fullscreen)
		}
		else{
			sd.Windowed = FALSE;	//TRUE(window mode), FALSE(fullscreen)
		}

		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			m_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDeviceAndSwapChain(NULL, m_driverType, NULL, createDeviceFlags, featureLevels, numFeatureLevels,
				D3D11_SDK_VERSION, &sd, &m_pSwapChain, &m_pD3DDevice, &m_featureLevel, &m_pDeviceContext);
			if (SUCCEEDED(hr))
				break;
		}
		if (FAILED(hr))
			return hr;


		//SwapChainのバックバッファからRenderTarget関連を作成//
		mCreateRenderTarget();

        if (m_HDR_flag){
            mCreateRenderTargetHDR();
        }

		return 0;
	};


	int mCreateRenderTarget(){

		HRESULT hr = S_OK;

		// Create a render target view
		ID3D11Texture2D* pBackBuffer = NULL;
		hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr))
			return hr;

		//BackBuffer用の窓口(View)を作成.
		hr = m_pD3DDevice->CreateRenderTargetView(pBackBuffer, NULL, &m_pRenderTargetView);
		if (FAILED(hr))
			return hr;

		//バック・バッファの情報取得(for depthBuffer)
		D3D11_TEXTURE2D_DESC descBackBuffer;
		pBackBuffer->GetDesc(&descBackBuffer);

		//もう使わないのでリリース.
		pBackBuffer->Release();
		

		//depth_buffer(as 2D texture)の作成.
		D3D11_TEXTURE2D_DESC descDepth;
		descDepth.Width = descBackBuffer.Width;   // 幅
		descDepth.Height = descBackBuffer.Height;  // 高さ
		descDepth.MipLevels = 1;       // ミップマップ・レベル数
		descDepth.ArraySize = 1;       // 配列サイズ
		descDepth.Format = DXGI_FORMAT_D32_FLOAT;  // フォーマット(深度のみ)
		descDepth.SampleDesc.Count = descBackBuffer.SampleDesc.Count;  // マルチサンプリングの設定
		descDepth.SampleDesc.Quality = descBackBuffer.SampleDesc.Quality;  // マルチサンプリングの品質
		descDepth.Usage = D3D11_USAGE_DEFAULT;      // デフォルト使用法
		descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL; // 深度/ステンシルとして使用
		descDepth.CPUAccessFlags = 0;   // CPUからはアクセスしない
		descDepth.MiscFlags = 0;   // その他の設定なし
		hr = m_pD3DDevice->CreateTexture2D(
			&descDepth,         // 作成する2Dテクスチャの設定
			NULL,               // 
			&m_pDepthStencil);  // 作成したテクスチャを受け取る変数
		if (FAILED(hr))
			return hr;

		// 深度/ステンシル ビューの作成
		D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
		descDSV.Format = descDepth.Format;            // ビューのフォーマット
		descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;	//マルチサンプリングならD3D10_DSV_DIMENSION_TEXTURE2DMS
		descDSV.Flags = 0;		//DX11
		descDSV.Texture2D.MipSlice = 0;		//マルチサンプリングならTexture2DMS
		hr = m_pD3DDevice->CreateDepthStencilView(
			m_pDepthStencil,       // 深度/ステンシル・ビューを作るテクスチャ
			&descDSV,              // 深度/ステンシル・ビューの設定
			&m_pDepthStencilView); // 作成したビューを受け取る変数
		if (FAILED(hr))
			return hr;



		return 0;
	};


    int mCreateRenderTargetHDR(){



        //(2)スクリーンフォーマットを取得//
        ID3D11Texture2D* pBackBuffer = NULL;
        HRESULT hr = m_pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
        if (FAILED(hr)){
            return hr;
        }
    

        D3D11_TEXTURE2D_DESC descBackBuffer;
        pBackBuffer->GetDesc(&descBackBuffer);
        pBackBuffer->Release();


        //(3)CPU読み出し可能なバッファをGPU上に作成//
        D3D11_TEXTURE2D_DESC Texture2DDesc;
        Texture2DDesc.ArraySize = 1;
        Texture2DDesc.BindFlags = 0;
        Texture2DDesc.CPUAccessFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
		Texture2DDesc.Format = m_buffer_format;// DXGI_FORMAT_R16G16B16A16_FLOAT;
        Texture2DDesc.Height = descBackBuffer.Height;
        Texture2DDesc.Width = descBackBuffer.Width;
        Texture2DDesc.MipLevels = 1;
        Texture2DDesc.MiscFlags = 0;
        Texture2DDesc.SampleDesc.Count = 1;
        Texture2DDesc.SampleDesc.Quality = 0;
        Texture2DDesc.Usage = D3D11_USAGE_DEFAULT;;


        m_pD3DDevice->CreateTexture2D(&Texture2DDesc, 0, &m_pRenderTarget_HDR);

        
        hr = m_pD3DDevice->CreateRenderTargetView(m_pRenderTarget_HDR, NULL, &m_pRenderTargetView_HDR);
        if (FAILED(hr)){
            return hr;
        }

        hr = m_pD3DDevice->CreateShaderResourceView(m_pRenderTarget_HDR, NULL, &m_pTextureRV_HDR);
        if (FAILED(hr)){
            return NULL;
        }


        return 0;
    };


	int mReleaseRenderTarget(){

		m_pDeviceContext->OMSetRenderTargets(0, NULL, NULL);

		if (m_pDepthStencilView) {
			m_pDepthStencilView->Release();
			m_pDepthStencilView = NULL;
		}

		if (m_pDepthStencil){
			m_pDepthStencil->Release();
			m_pDepthStencil = NULL;
		}

		if (m_pRenderTargetView){
			m_pRenderTargetView->Release();
			m_pRenderTargetView = NULL;
		}


		return 0;
	}

	void mTerminateDX11(){

		BOOL fullscreen;
		m_pSwapChain->GetFullscreenState(&fullscreen, NULL);
		if (fullscreen){ m_pSwapChain->SetFullscreenState(FALSE, NULL); }

		if (m_pDeviceContext) {
			m_pDeviceContext->ClearState();
			m_pDeviceContext->Flush();
		}
/*
		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pDepthStencil) m_pDepthStencil->Release();

		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		*/
		mReleaseRenderTarget();

		if (m_pSwapChain) {
			m_pSwapChain->Release();
		}
		if (m_pDeviceContext) m_pDeviceContext->Release();
		if (m_pD3DDevice) m_pD3DDevice->Release();
	}



};
