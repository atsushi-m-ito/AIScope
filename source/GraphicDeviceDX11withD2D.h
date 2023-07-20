#pragma once

#include "targetver.h"
#include <Windows.h>
#include "IGraphicDevice.h"
////////////////////////////////////////////
//DirectXヘッダー
#include <d3d11.h>

#pragma comment( lib, "d3d11.lib" )


#include <dxgi1_2.h>
#include <D2d1_1.h>
#include <d2d1effects.h>
#pragma comment( lib, "D2d1.lib" )
#ifndef WITHOUT_DWRITE
#include <dwrite.h>
#pragma comment( lib, "Dwrite.lib" )
#endif

#include "RenderDX11.h"


class GraphicDeviceDX11withD2D : public IGraphicDevice
{
private:
	// DirectX 11
	HWND m_hWnd;

	int m_vsync; //pSwapChain->Presentメソッドの第一引数.0なら垂直同期しない, 1なら垂直同期.

	D3D_DRIVER_TYPE         m_driverType;
	D3D_FEATURE_LEVEL       m_featureLevel;
	ID3D11Device*           m_pD3DDevice;
	ID3D11DeviceContext*    m_pD3DDeviceContext;
	IDXGISwapChain1*         m_pSwapChain;
	ID3D11RenderTargetView* m_pRenderTargetView;
	ID3D11Texture2D*          m_pDepthStencil;		// 深度/ステンシル
	ID3D11DepthStencilView*   m_pDepthStencilView;	// 深度/ステンシル・ビュー

    ID3D11Texture2D*          m_pRenderTarget_HDR;
    ID3D11RenderTargetView* m_pRenderTargetView_HDR;
    ID3D11ShaderResourceView* m_pTextureRV_HDR;

	//Direct2D and DirectWrite
	ID2D1Factory1* m_d2dFactory;
	ID2D1Device* m_d2dDevice;
	ID2D1DeviceContext* m_d2dDeviceContext;
#ifndef WITHOUT_DWRITE
	IDWriteFactory* m_dwriteFactory;
#endif

	UINT m_buffer_count;
	const DXGI_FORMAT m_buffer_format;

    const int m_HDR_flag;

	int	m_width;
	int m_height;
	bool is_resized = false;


public:
	GraphicDeviceDX11withD2D(HWND hWnd, DXGI_FORMAT dxgi_format) :
		m_hWnd(hWnd),
		m_driverType(D3D_DRIVER_TYPE_NULL),
		m_featureLevel(D3D_FEATURE_LEVEL_11_0),
		m_pD3DDevice(NULL),
		m_pD3DDeviceContext(NULL),
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
        m_pTextureRV_HDR(NULL),
#ifndef WITHOUT_DWRITE
		m_dwriteFactory(NULL),
#endif
		m_d2dFactory(NULL),
		m_d2dDevice(NULL),
		m_d2dDeviceContext(NULL)
    {}; 

	~GraphicDeviceDX11withD2D(){
		TerminateDX11();
		
		mTerminateDX11();
		
		
	};

	int InitializeWindow(int width, int height, int init_windowmode, int vsync_mode){

		//D2Dの初期化//
		CreateDeviceD2D();

		//DX11の初期化//
		int result = mInitializeDX11( width, height, init_windowmode);
		if (result){
			return result;
		}
		
		//common設定(将来的に廃止予定)//
		DX11_CORE dx11_core;
		dx11_core.pD3DDevice = m_pD3DDevice;
		dx11_core.pD3DDeviceContext = m_pD3DDeviceContext;
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
		if (is_resized) {
			mResizeBuffer(m_width, m_height);
		}

		m_pD3DDeviceContext->OMSetRenderTargets(1, &m_pRenderTargetView, m_pDepthStencilView);
	};
	
	void EndRendering()
	{
		m_pSwapChain->Present(m_vsync, 0);	//第一引数が0なら垂直同期しない, 1なら垂直同期.
		//ID3DSwapChain1以降だとPresentの処理によってRenderTargetViewが外れるので上記のBeginRenderingで再設定が必要//
		//CheckRtvDX11();
	};

	ID2D1DeviceContext* GetD2DDeviceContext() {
		m_d2dDeviceContext->AddRef();
		return m_d2dDeviceContext;
	}


	void Resize(int width, int height)
	{
		m_width = width;
		m_height = height;
		is_resized = true;
	}

	void mResizeBuffer(int width, int height){
		mReleaseRenderTarget();
		m_d2dDeviceContext->SetTarget(nullptr);

		HRESULT hr = m_pSwapChain->ResizeBuffers(m_buffer_count, width, height, m_buffer_format, 0);
		
		//D2Dのレンダーターゲットに合わせてbitmapの作成//
		mCreateDeviceSwapChainBitmap(m_pSwapChain, m_d2dDeviceContext);

		mCreateRenderTarget();


		//common設定(将来的に廃止予定)//
		DX11_CORE dx11_core;
		dx11_core.pD3DDevice = m_pD3DDevice;
		dx11_core.pD3DDeviceContext = m_pD3DDeviceContext;
		dx11_core.pSwapChain = m_pSwapChain;
		dx11_core.pRenderTargetView = m_pRenderTargetView;
		dx11_core.pDepthStencilView = m_pDepthStencilView;
		ResetDX11(dx11_core);

	};

private:

	/*
	VisualStudio2015のDirectX11のUWPアプリのサンプルから抜粋して一部改変
	*/
	// Direct3D デバイスに依存しないリソースを構成します。
	void CreateDeviceD2D()
	{
		// Direct2D リソースを初期化します。
		
		D2D1_FACTORY_OPTIONS options;
		ZeroMemory(&options, sizeof(D2D1_FACTORY_OPTIONS));

#if defined(_DEBUG)
		// プロジェクトがデバッグ ビルドに含まれている場合は、Direct2D デバッグを SDK レイヤーを介して有効にします。
		options.debugLevel = D2D1_DEBUG_LEVEL_INFORMATION;
#endif

		// Direct2D ファクトリを初期化します。
		//HRESULT hr = 
		D2D1CreateFactory(
				D2D1_FACTORY_TYPE_SINGLE_THREADED,
				__uuidof(ID2D1Factory1),
				&options,
				(LPVOID*)&m_d2dFactory		
		);

#ifndef WITHOUT_DWRITE
		// DirectWrite ファクトリを初期化します。
		DWriteCreateFactory(
				DWRITE_FACTORY_TYPE_SHARED,
				__uuidof(IDWriteFactory),
			(IUnknown**)&m_dwriteFactory
		);
#endif
	}

	int mInitializeDX11(int width, int height, int init_windowmode) {

		HRESULT hr = S_OK;
		const HWND hWnd = m_hWnd;

		m_width = width;
		m_height = height;


		//このフラグは、カラー チャネルの順序が API の既定値とは異なるサーフェスのサポートを追加します。
		// これは、Direct2D との互換性を保持するために必要です。
		UINT creationFlags = D3D11_CREATE_DEVICE_BGRA_SUPPORT;

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
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_10_1,
			D3D_FEATURE_LEVEL_10_0,
			D3D_FEATURE_LEVEL_9_3,
			D3D_FEATURE_LEVEL_9_2,
			D3D_FEATURE_LEVEL_9_1
		};



		for (UINT driverTypeIndex = 0; driverTypeIndex < numDriverTypes; driverTypeIndex++)
		{
			m_driverType = driverTypes[driverTypeIndex];
			hr = D3D11CreateDevice(NULL, m_driverType, 0,
				creationFlags, featureLevels, ARRAYSIZE(featureLevels),
				D3D11_SDK_VERSION, &m_pD3DDevice, &m_featureLevel, &m_pD3DDeviceContext);
			if (SUCCEEDED(hr))
				break;
		}
		

		//D2DのDeviceContextの作成//
		IDXGIDevice1* dxgiDevice;
		hr = m_pD3DDevice->QueryInterface(__uuidof(IDXGIDevice1), (LPVOID*)&dxgiDevice);
		if (FAILED(hr)) {
			MessageBox(NULL,
				L"m_pD3DDevice->QueryInterface() failed.",
				NULL, MB_OK);
			return -1;
		}

		m_d2dFactory->CreateDevice(dxgiDevice, &m_d2dDevice);
		if (FAILED(hr)) {
			MessageBox(NULL,
				L"m_d2dFactory->CreateDevice() failed.",
				NULL, MB_OK);
			dxgiDevice->Release();
			return -1;
		}

		m_d2dDevice->CreateDeviceContext(D2D1_DEVICE_CONTEXT_OPTIONS_NONE, &m_d2dDeviceContext);
		if (FAILED(hr)) {
			MessageBox(NULL,
				L"m_d2dDevice->CreateDeviceContext() failed.",
				NULL, MB_OK);
			dxgiDevice->Release();
			return -1;
		}

		//D3DとD2Dの共通SwapChaneを生成//
		//Visual studio2015のUWPのサンプルより//
		//既存の Direct3D デバイスと同じアダプターを使用して、新規作成します。
		
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc = { 0 };
			
		swapChainDesc.Width = width;		// ウィンドウのサイズと一致させます。
		swapChainDesc.Height = height;
		swapChainDesc.Format = m_buffer_format;
		swapChainDesc.Stereo = false;
		swapChainDesc.SampleDesc.Count = 1;								// マルチサンプリングは使いません。
		swapChainDesc.SampleDesc.Quality = 0;
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = m_buffer_count;						// 遅延を最小限に抑えるにはダブル バッファーを使用します。//3D vision の為にも2を設定.//
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;	// Windows ストア アプリはすべて、この SwapEffect を使用する必要があります。
		swapChainDesc.Flags = 0;
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;// DXGI_SCALING_NONE is not supported by windows 7?;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		


		IDXGIAdapter* dxgiAdapter;
		hr = dxgiDevice->GetAdapter(&dxgiAdapter);

		IDXGIFactory2* dxgiFactory;
		hr = dxgiAdapter->GetParent(IID_PPV_ARGS(&dxgiFactory));
		if (FAILED(hr)) {
			MessageBox(NULL,
				L"dxgiAdapter->GetParent() failed.",
				NULL, MB_OK);
			dxgiDevice->Release();
			return -1;
		}

		hr = dxgiFactory->CreateSwapChainForHwnd(m_pD3DDevice, hWnd, &swapChainDesc, NULL, NULL, &m_pSwapChain);
		if (FAILED(hr)) {
			MessageBox(NULL,
				L"dxgiFactory->CreateSwapChainForHwnd() failed.",
				NULL, MB_OK);
			dxgiFactory->Release();
			dxgiDevice->Release();
			return -1;
		}

			// DXGI が 1 度に複数のフレームをキュー処理していないことを確認します。これにより、遅延が減少し、
			// アプリケーションが各 VSync の後でのみレンダリングすることが保証され、消費電力が最小限に抑えられます。

		//hr = dxgiDevice->SetMaximumFrameLatency(1);
		

		dxgiFactory->Release();
		dxgiDevice->Release();
		
		
		//D2Dのレンダーターゲットに合わせてbitmapの作成//
		mCreateDeviceSwapChainBitmap(m_pSwapChain, m_d2dDeviceContext);
		

		//SwapChainのバックバッファからRenderTarget関連を作成//
		mCreateRenderTarget();

        if (m_HDR_flag){
            mCreateRenderTargetHDR();
        }

		return 0;
	};


	void mCreateDeviceSwapChainBitmap(IDXGISwapChain1* swapchain, ID2D1DeviceContext* target) {

		IDXGISurface* surface;
		HRESULT hr = swapchain->GetBuffer(0, __uuidof(IDXGISurface), (LPVOID*)&surface);
		if (FAILED(hr)) {
			MessageBox(NULL,
				L"swapchain->GetBuffer() failed.",
				NULL, MB_OK);			
			return;
		}

		D2D1_BITMAP_PROPERTIES1 const props = D2D1::BitmapProperties1(
			D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
			//D2D1::PixelFormat(DXGI_FORMAT_B8G8R8A8_UNORM, D2D1_ALPHA_MODE_IGNORE));
			D2D1::PixelFormat(m_buffer_format, D2D1_ALPHA_MODE_IGNORE));

		ID2D1Bitmap1* bitmap;
		hr = target->CreateBitmapFromDxgiSurface(surface,
			props,
			&bitmap);
		if (FAILED(hr))	{
			MessageBox(NULL,
				L"CreateBitmapFromDxgiSurface() failed.",
				NULL, MB_OK);
			surface->Release();
			return ;
		}

		target->SetTarget(bitmap);
		bitmap->Release();
		surface->Release();
	}


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

		m_pD3DDeviceContext->OMSetRenderTargets(0, NULL, NULL);

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

		if (m_pD3DDeviceContext) {
			m_pD3DDeviceContext->ClearState();
			m_pD3DDeviceContext->Flush();
		}
/*
		if (m_pDepthStencilView) m_pDepthStencilView->Release();
		if (m_pDepthStencil) m_pDepthStencil->Release();

		if (m_pRenderTargetView) m_pRenderTargetView->Release();
		*/
		mReleaseRenderTarget();

		SAFE_RELEASE(m_pSwapChain);
		SAFE_RELEASE(m_pD3DDeviceContext);
		SAFE_RELEASE(m_pD3DDevice);


		
		SAFE_RELEASE(m_d2dDeviceContext);
		SAFE_RELEASE(m_d2dDevice);
#ifndef WITHOUT_DWRITE
		SAFE_RELEASE(m_dwriteFactory); 
#endif
		SAFE_RELEASE(m_d2dFactory);
		
	}



};
