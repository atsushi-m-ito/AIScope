#pragma once



class ReadableDepth {
private:
	DX11_CORE* m_dx11_core;
	D3D11_TEXTURE2D_DESC m_previous_descBackBuffer{ 0 };
	//ID3D11RenderTargetView* m_filterRTV = nullptr;
	//ID3D11ShaderResourceView* m_renderSRV = nullptr;
	ID3D11DepthStencilView* m_filterDSV = nullptr;
	ID3D11ShaderResourceView* m_depthSRV = nullptr;
	ID3D11Texture2D* filterDepthStencilBuffer = nullptr;

	ID3D11RenderTargetView* org_RTV = nullptr;
	ID3D11DepthStencilView* org_DSV = nullptr;


public:
	ReadableDepth(DX11_CORE* dx11_core) :
		m_dx11_core(dx11_core)		
	{	}

	~ReadableDepth() {
		//SAFE_RELEASE(m_filterRTV);
		//SAFE_RELEASE(m_renderSRV);
		SAFE_RELEASE(m_filterDSV);
		SAFE_RELEASE(m_depthSRV);
		SAFE_RELEASE(filterDepthStencilBuffer);
	}

private:
	bool IsEqual(D3D11_TEXTURE2D_DESC a, D3D11_TEXTURE2D_DESC b) {
		if (a.ArraySize != b.ArraySize) return false;
		if (a.Format != b.Format) return false;
		if (a.Height != b.Height) return false;
		if (a.Width != b.Width) return false;
		if (a.MipLevels != b.MipLevels) return false;
		if (a.MiscFlags != b.MiscFlags) return false;
		if (a.SampleDesc.Count != b.SampleDesc.Count) return false;
		if (a.SampleDesc.Quality != b.SampleDesc.Quality) return false;
		return true;
	}

public:

	ID3D11ShaderResourceView* Begin() {

		//(2)スクリーンフォーマットを取得//
		ID3D11Texture2D* pBackBuffer = NULL;
		HRESULT hr = m_dx11_core->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr)) return nullptr;

		D3D11_TEXTURE2D_DESC descBackBuffer;
		pBackBuffer->GetDesc(&descBackBuffer);
		pBackBuffer->Release();

		//サイズが異なったらrendertargetを作成しなおす//
		if (!IsEqual(m_previous_descBackBuffer, descBackBuffer)) {
			m_previous_descBackBuffer = descBackBuffer;
			/*
			//オフスクリーンレンダリング用のレンダリングターゲット(texture)バッファをGPU上に作成//
			D3D11_TEXTURE2D_DESC Texture2DDesc;
			Texture2DDesc.ArraySize = descBackBuffer.ArraySize;
			Texture2DDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			Texture2DDesc.CPUAccessFlags = 0;
			Texture2DDesc.Format = descBackBuffer.Format;
			Texture2DDesc.Height = descBackBuffer.Height;
			Texture2DDesc.Width = descBackBuffer.Width;
			Texture2DDesc.MipLevels = descBackBuffer.MipLevels;
			Texture2DDesc.MiscFlags = descBackBuffer.MiscFlags;
			Texture2DDesc.SampleDesc.Count = descBackBuffer.SampleDesc.Count;
			Texture2DDesc.SampleDesc.Quality = descBackBuffer.SampleDesc.Quality;
			Texture2DDesc.Usage = D3D11_USAGE_DEFAULT;               //GPUで書き込むにはD3D11_USAGE_DEFAULTにする必要がある//

			ID3D11Texture2D* hCaptureTexture;
			hr = m_dx11_core->pD3DDevice->CreateTexture2D(&Texture2DDesc, 0, &hCaptureTexture);

			SAFE_RELEASE(m_filterRTV);
			hr = m_dx11_core->pD3DDevice->CreateRenderTargetView(hCaptureTexture, NULL, &m_filterRTV);

			{
				SAFE_RELEASE(m_renderSRV);
				D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
				memset(&srv_desc, 0, sizeof(srv_desc));
				srv_desc.Format = descBackBuffer.Format;
				srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MipLevels = 1;
				hr = m_dx11_core->pD3DDevice->CreateShaderResourceView(hCaptureTexture, &srv_desc, &m_renderSRV);
			}

			hCaptureTexture->Release();
			*/

			//オフスクリーンレンダリング用のレンダリングターゲット(深度/ステンシル)バッファをGPU上に作成//
			D3D11_TEXTURE2D_DESC descDepth;
			descDepth.Width = descBackBuffer.Width;   // 幅
			descDepth.Height = descBackBuffer.Height;  // 高さ
			descDepth.MipLevels = 1;       // ミップマップ・レベル数
			descDepth.ArraySize = 1;       // 配列サイズ
			descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;  // フォーマットShaderResourceにするために使えるフォーマットは限られる//
			descDepth.SampleDesc.Count = descBackBuffer.SampleDesc.Count;  // マルチサンプリングの設定
			descDepth.SampleDesc.Quality = descBackBuffer.SampleDesc.Quality;  // マルチサンプリングの品質
			descDepth.Usage = D3D11_USAGE_DEFAULT;      // デフォルト使用法
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // 深度/ステンシルとして使用
			descDepth.CPUAccessFlags = 0;   // CPUからはアクセスしない
			descDepth.MiscFlags = 0;   // その他の設定なし

			//2Dテクスチャの作成//
			hr = m_dx11_core->pD3DDevice->CreateTexture2D(&descDepth, NULL, &filterDepthStencilBuffer);

			SAFE_RELEASE(m_filterDSV);
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//バッファ作成時とは異なるフォーマットにする(RenderTargetとして使えるフォーマットの制限のため)
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;	//マルチサンプリングならD3D10_DSV_DIMENSION_TEXTURE2DMS
			descDSV.Flags = 0;		//DX11
			descDSV.Texture2D.MipSlice = 0;		//マルチサンプリングならTexture2DMS
			// 深度/ステンシル・ビューの作成//
			hr = m_dx11_core->pD3DDevice->CreateDepthStencilView(filterDepthStencilBuffer, &descDSV, &m_filterDSV);

			{
				SAFE_RELEASE(m_depthSRV);
				D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
				memset(&srv_desc, 0, sizeof(srv_desc));
				srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;//バッファ作成時とは異なるフォーマットにする(ShaderResourceとして使えるフォーマットの制限のため)
				srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MipLevels = 1;
				hr = m_dx11_core->pD3DDevice->CreateShaderResourceView(filterDepthStencilBuffer, &srv_desc, &m_depthSRV);
			}

			//filterDepthStencilBuffer->Release();

		}

		//(3.5)レンダリングターゲットの差し替え//
		m_dx11_core->pD3DDeviceContext->OMGetRenderTargets(1, &org_RTV, &org_DSV);
		m_dx11_core->pD3DDeviceContext->OMSetRenderTargets(1, &org_RTV, m_filterDSV);


		//BackBufferのクリア//
		//m_dx11_core->pD3DDeviceContext->ClearRenderTargetView(m_filterRTV, rendering_property.bg_color);
		//DepthBufferのクリア//
		m_dx11_core->pD3DDeviceContext->ClearDepthStencilView(m_filterDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);

		return m_depthSRV;
	}

	void End() {
		if (org_DSV == nullptr) return;
		//RenderingCoreDX11(data_list, screen_w, screen_h, range_x, range_y, range_w, range_h, view_matrix, focus_distance, rendering_property);


		//レンダリングターゲットを戻す//
		m_dx11_core->pD3DDeviceContext->OMSetRenderTargets(1, &org_RTV, org_DSV);
		SAFE_RELEASE(org_RTV);
		SAFE_RELEASE(org_DSV);
	}



};



class FilterRGBandDepth {
private:
	DX11_CORE* m_dx11_core;
	D3D11_TEXTURE2D_DESC m_previous_descBackBuffer{ 0 };
	ID3D11RenderTargetView* m_filterRTV;
	ID3D11DepthStencilView* m_filterDSV;
	ID3D11ShaderResourceView* m_renderSRV;
	ID3D11ShaderResourceView* m_depthSRV;
	ID3D11SamplerState* m_pSampler;
	ID3D11Buffer* m_texel_width_CB;

	RendererWrapper renderer;

public:
	FilterRGBandDepth(DX11_CORE* dx11_core, CommonShader* comShader) :
		renderer(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader),
		m_dx11_core(dx11_core),
		m_filterRTV(NULL),
		m_renderSRV(NULL),
		m_filterDSV(NULL),
		m_depthSRV(NULL),
		m_pSampler(NULL),
		m_texel_width_CB(NULL)
	{

		//シェーダー//
		TCHAR filepath[MAX_PATH];
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEX", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }
		};
		UINT numElements = ARRAYSIZE(layout);
		GetAttachedFileName(filepath, _T("vs_filter_DoF.cso"));
		renderer.LoadVertexShader(filepath, "main", "vs_4_0", layout, numElements);


		//Pixel Shader-----------------------------------------
		GetAttachedFileName(filepath, _T("ps_filter_DoF.cso"));
		renderer.LoadPixelShader(filepath, "main", "ps_4_0");


		// テクスチャのサンプリング(ポリゴン貼り付け時の補間)方法の指定.
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		HRESULT hr = m_dx11_core->pD3DDevice->CreateSamplerState(&sampDesc, &m_pSampler);


		//テクセル幅(1/画面幅)を伝えるための定数バッファ//
		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = sizeof(float) * 4;	//16の倍数である必要がある//
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//CPUからの書き換え
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		hr = m_dx11_core->pD3DDevice->CreateBuffer(&bd, NULL, &m_texel_width_CB);
	}

	~FilterRGBandDepth() {
		SAFE_RELEASE(m_filterRTV);
		SAFE_RELEASE(m_renderSRV);
		SAFE_RELEASE(m_pSampler);
		SAFE_RELEASE(m_texel_width_CB);
		SAFE_RELEASE(m_filterDSV);
		SAFE_RELEASE(m_depthSRV);
	}

private:
	bool IsEqual(D3D11_TEXTURE2D_DESC a, D3D11_TEXTURE2D_DESC b) {
		if (a.ArraySize != b.ArraySize) return false;
		if (a.Format != b.Format) return false;
		if (a.Height != b.Height) return false;
		if (a.Width != b.Width) return false;
		if (a.MipLevels != b.MipLevels) return false;
		if (a.MiscFlags != b.MiscFlags) return false;
		if (a.SampleDesc.Count != b.SampleDesc.Count) return false;
		if (a.SampleDesc.Quality != b.SampleDesc.Quality) return false;
		return true;
	}

public:
	void Render(std::vector<LawData>& data_list, int screen_w, int screen_h, int range_x, int range_y, int range_w, int range_h, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property) {

		//(2)スクリーンフォーマットを取得//
		ID3D11Texture2D* pBackBuffer = NULL;
		HRESULT hr = m_dx11_core->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
		if (FAILED(hr)) return;

		D3D11_TEXTURE2D_DESC descBackBuffer;
		pBackBuffer->GetDesc(&descBackBuffer);
		pBackBuffer->Release();

		//サイズが異なったらrendertargetを作成しなおす//
		if (!IsEqual(m_previous_descBackBuffer, descBackBuffer)) {
			m_previous_descBackBuffer = descBackBuffer;

			//オフスクリーンレンダリング用のレンダリングターゲット(texture)バッファをGPU上に作成//
			D3D11_TEXTURE2D_DESC Texture2DDesc;
			Texture2DDesc.ArraySize = descBackBuffer.ArraySize;
			Texture2DDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
			Texture2DDesc.CPUAccessFlags = 0;
			Texture2DDesc.Format = descBackBuffer.Format;
			Texture2DDesc.Height = descBackBuffer.Height;
			Texture2DDesc.Width = descBackBuffer.Width;
			Texture2DDesc.MipLevels = descBackBuffer.MipLevels;
			Texture2DDesc.MiscFlags = descBackBuffer.MiscFlags;
			Texture2DDesc.SampleDesc.Count = descBackBuffer.SampleDesc.Count;
			Texture2DDesc.SampleDesc.Quality = descBackBuffer.SampleDesc.Quality;
			Texture2DDesc.Usage = D3D11_USAGE_DEFAULT;               //GPUで書き込むにはD3D11_USAGE_DEFAULTにする必要がある//

			ID3D11Texture2D* hCaptureTexture;
			hr = m_dx11_core->pD3DDevice->CreateTexture2D(&Texture2DDesc, 0, &hCaptureTexture);

			SAFE_RELEASE(m_filterRTV);
			hr = m_dx11_core->pD3DDevice->CreateRenderTargetView(hCaptureTexture, NULL, &m_filterRTV);

			{
				SAFE_RELEASE(m_renderSRV);
				D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
				memset(&srv_desc, 0, sizeof(srv_desc));
				srv_desc.Format = descBackBuffer.Format;
				srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MipLevels = 1;
				hr = m_dx11_core->pD3DDevice->CreateShaderResourceView(hCaptureTexture, &srv_desc, &m_renderSRV);
			}

			hCaptureTexture->Release();


			//オフスクリーンレンダリング用のレンダリングターゲット(深度/ステンシル)バッファをGPU上に作成//
			D3D11_TEXTURE2D_DESC descDepth;
			descDepth.Width = descBackBuffer.Width;   // 幅
			descDepth.Height = descBackBuffer.Height;  // 高さ
			descDepth.MipLevels = 1;       // ミップマップ・レベル数
			descDepth.ArraySize = 1;       // 配列サイズ
			descDepth.Format = DXGI_FORMAT_R24G8_TYPELESS;  // フォーマットShaderResourceにするために使えるフォーマットは限られる//
			descDepth.SampleDesc.Count = descBackBuffer.SampleDesc.Count;  // マルチサンプリングの設定
			descDepth.SampleDesc.Quality = descBackBuffer.SampleDesc.Quality;  // マルチサンプリングの品質
			descDepth.Usage = D3D11_USAGE_DEFAULT;      // デフォルト使用法
			descDepth.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE; // 深度/ステンシルとして使用
			descDepth.CPUAccessFlags = 0;   // CPUからはアクセスしない
			descDepth.MiscFlags = 0;   // その他の設定なし
			ID3D11Texture2D* filterDepthStencilBuffer;
			//2Dテクスチャの作成//
			hr = m_dx11_core->pD3DDevice->CreateTexture2D(&descDepth, NULL, &filterDepthStencilBuffer);

			SAFE_RELEASE(m_filterDSV);
			D3D11_DEPTH_STENCIL_VIEW_DESC descDSV;
			descDSV.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;//バッファ作成時とは異なるフォーマットにする(RenderTargetとして使えるフォーマットの制限のため)
			descDSV.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;	//マルチサンプリングならD3D10_DSV_DIMENSION_TEXTURE2DMS
			descDSV.Flags = 0;		//DX11
			descDSV.Texture2D.MipSlice = 0;		//マルチサンプリングならTexture2DMS
			// 深度/ステンシル・ビューの作成//
			hr = m_dx11_core->pD3DDevice->CreateDepthStencilView(filterDepthStencilBuffer, &descDSV, &m_filterDSV);

			{
				SAFE_RELEASE(m_depthSRV);
				D3D11_SHADER_RESOURCE_VIEW_DESC srv_desc;
				memset(&srv_desc, 0, sizeof(srv_desc));
				srv_desc.Format = DXGI_FORMAT_R24_UNORM_X8_TYPELESS;//バッファ作成時とは異なるフォーマットにする(ShaderResourceとして使えるフォーマットの制限のため)
				srv_desc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
				srv_desc.Texture2D.MipLevels = 1;
				hr = m_dx11_core->pD3DDevice->CreateShaderResourceView(filterDepthStencilBuffer, &srv_desc, &m_depthSRV);
			}

			filterDepthStencilBuffer->Release();

		}

		//(3.5)レンダリングターゲットの差し替え//
		ID3D11RenderTargetView* org_RTV;
		ID3D11DepthStencilView* org_DSV;
		m_dx11_core->pD3DDeviceContext->OMGetRenderTargets(1, &org_RTV, &org_DSV);
		m_dx11_core->pD3DDeviceContext->OMSetRenderTargets(1, &m_filterRTV, m_filterDSV);


		//BackBufferのクリア//
		m_dx11_core->pD3DDeviceContext->ClearRenderTargetView(m_filterRTV, rendering_property.bg_color);
		//DepthBufferのクリア//
		m_dx11_core->pD3DDeviceContext->ClearDepthStencilView(m_filterDSV, D3D11_CLEAR_DEPTH, 1.0f, 0);
		RenderingCoreDX11(data_list, screen_w, screen_h, range_x, range_y, range_w, range_h, view_matrix, focus_distance, rendering_property);


		//レンダリングターゲットを戻す//
		m_dx11_core->pD3DDeviceContext->OMSetRenderTargets(1, &org_RTV, org_DSV);
		SAFE_RELEASE(org_RTV);
		SAFE_RELEASE(org_DSV);

		//ビューポートの設定(RenderingCoreDX11でマルチレンダリングの場合に備えて)//
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)range_w;
		vp.Height = (FLOAT)range_h;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		m_dx11_core->pD3DDeviceContext->RSSetViewports(1, &vp);


		ClearRenderingTargetDX11(rendering_property.bg_color);
		m_dx11_core->pD3DDeviceContext->PSSetSamplers(0, 1, &m_pSampler);

		m_dx11_core->pD3DDeviceContext->PSSetShaderResources(0, 1, &m_renderSRV);
		m_dx11_core->pD3DDeviceContext->PSSetShaderResources(1, 1, &m_depthSRV);


		VERTEX_F3F2 vertex[4];
		vertex[0].position.Set(-1.0f, -1.0f, .0f);
		vertex[1].position.Set(-1.0f, 1.0f, .0f);
		vertex[2].position.Set(1.0f, 1.0f, .0f);
		vertex[3].position.Set(1.0f, -1.0f, .0f);
		const float u_max = (float)range_w / (float)descBackBuffer.Width;//画面幅とテクスチャ幅が異なる場合の対策//
		const float v_max = (float)range_h / (float)descBackBuffer.Height;
		vertex[0].coord_u = 0.0f;
		vertex[0].coord_v = v_max;
		vertex[1].coord_u = 0.0f;
		vertex[1].coord_v = 0.0f;
		vertex[2].coord_u = u_max;
		vertex[2].coord_v = 0.0f;
		vertex[3].coord_u = u_max;
		vertex[3].coord_v = v_max;

		int indexes[6]{ 0,1,2,2,3,0 };

		renderer.UpdateVertexBuffer(vertex, 4, sizeof(VERTEX_F3F2));	//頂点データをVRAMに転送
		renderer.UpdateIndexBuffer(indexes, 6, sizeof(int));	//インデックスデータをVRAMに転送

		//texel幅の送信(//定数バッファの書き込み)//
		{
			float texel_width[2] = { 1.0f / (float)descBackBuffer.Width , 1.0f / (float)descBackBuffer.Height };
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			m_dx11_core->pD3DDeviceContext->Map(m_texel_width_CB, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			CopyMemory(mappedResource.pData, &texel_width, sizeof(float) * 2);
			m_dx11_core->pD3DDeviceContext->Unmap(m_texel_width_CB, 0);
			m_dx11_core->pD3DDeviceContext->PSSetConstantBuffers(0, 1, &m_texel_width_CB);
		}

		renderer.DrawIndexed(6, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	}
};
