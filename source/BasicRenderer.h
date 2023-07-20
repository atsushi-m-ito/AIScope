//=====================================
// Dx9やOpenGL1.5までの基本レンダリングを行う.
//
//
// 
//=====================================

#ifndef __BasicRenderer_h__
#define __BasicRenderer_h__


#pragma once


#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <math.h>


#include "sdkdx11.h"
#include "listTemplate.h"
#include "vec3.h"
#include "mat44.h"
#include "aidx_vertexbuffer.h"

#include "debugprint.h"


#define SAFE_RELEASE(p) if(p){(p)->Release(); (p)=NULL;}


class BasicRenderer{
public:

//DX11共用グローバルオブジェクト--------------------

	ID3D11Device*           m_pD3DDevice;
	ID3D11DeviceContext*    m_pD3DDeviceContext;

	CommonShader* m_comShader;
	//CommonTexture* m_comTexture;

//DX11 shader---------------------------------------
	

	ID3D11VertexShader*     m_pVertexShader;
	ID3D11PixelShader*      m_pPixelShader;
	ID3D11InputLayout*      m_pVertexLayout;
	//ID3D11Buffer*           m_pVertexBuffer;
	ID3D11Buffer*           m_pIndexBuffer;
	ID3D11Buffer*           m_pInstanceBuffer;
	
	AIDX11::VertexBuffer m_vertexBuffer;

	//UINT m_vertexBufSz;
	UINT m_indexBufSz;
	UINT m_instanceBufSz;
	UINT m_instance_stride;
	
	//UINT			m_vertex_stride;
	DXGI_FORMAT		m_index_format;

	mat44f m_ViewMatrix;
	mat44f m_ProjectionMatrix;


	union {
		ID3D11Buffer* m_pConstantBuffers[2];
		struct {
			ID3D11Buffer*			m_pWVPBuffer;
			ID3D11Buffer*			m_pLightBuffer;
		};
	};


	//材質カラー設定=====================;
	float m_ambient_color[4];		//ライトがあるときの色
	

	//BLEND設定==========================;

	//void EnableFlagClear



public:
	

	BasicRenderer(ID3D11Device* pD3DDevice,	ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader, UINT vertexBufSz, UINT indexBufSz);
	virtual ~BasicRenderer();

	void SetViewAndProjectionMatrix(const float* pViewMatrix, const float* pProjectionMatrix);


//protected://将来的にはprotectedにする(VNTRendererのみの制限)//
	void UpdateVertexBuffer(LPVOID pBuffer, UINT count, UINT stride);
	void UpdateIndexBuffer(LPVOID pBuffer, UINT count, UINT stride);
	void UpdateInstanceBuffer(LPVOID pBuffer, UINT count, UINT stride);

	void DrawIndexed(UINT index_count, D3D_PRIMITIVE_TOPOLOGY primitive_topology);
	//void Draw(UINT index_count, D3D_PRIMITIVE_TOPOLOGY primitive_topology, bool is_instanced);
	void Draw(UINT indexCountPerInstance, D3D_PRIMITIVE_TOPOLOGY primitive_topology, UINT instanceCount);
	
	
};


//exeと同じフォルダにあるファイルパスへ直す.
int GetAttachedFileName(TCHAR* filepath, const TCHAR *filename);


class RendererWrapper : public BasicRenderer {
private:

public:

	RendererWrapper(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		BasicRenderer(pD3DDevice, pD3DDeviceContext, comShader, 0, 0)
	{

	}

	//virtual ~RendererWrapper() {};


	bool LoadVertexShader(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel,
		const D3D11_INPUT_ELEMENT_DESC* inputElements, UINT numElements) {
		m_pVertexShader = m_comShader->LoadVertexShader(szFileName, szEntryPoint, szShaderModel, inputElements, numElements, &m_pVertexLayout);

		return (m_pVertexShader != NULL);
	}

	bool LoadPixelShader(const TCHAR* szFileName, LPCSTR szEntryPoint, LPCSTR szShaderModel) {
		m_pPixelShader = m_comShader->LoadPixelShader(szFileName, szEntryPoint, szShaderModel);
		return (m_pPixelShader != NULL);
	}


	void BeginDraw(D3D_PRIMITIVE_TOPOLOGY primitive_topology) {


		// Set the input layout
		m_pD3DDeviceContext->IASetInputLayout(m_pVertexLayout);
		// Set primitive topology
		m_pD3DDeviceContext->IASetPrimitiveTopology(primitive_topology);

		
		/*
		//UINT stride = sizeof(VERTEX_F3F3UI1);
		m_pD3DDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_vertex_stride, &offset);
		*/
		m_vertexBuffer.Attach();
		m_pD3DDeviceContext->IASetIndexBuffer(m_pIndexBuffer, m_index_format, 0);


		//動的な頂点バッファ(複数インスタンス用)の更新 (UpdateSubresourceを使う場合).
		//目的のバッファはUsage = D3D11_USAGE_DEFAULT, CPUAccessFlags = 0.
		//第三引数がNULLの時はVertexBuffer(VRAM)と同じサイズをコピーするので,
		//コピー元(CPU)はVertexBuffer以上のサイズが確保されていなければならない.
		//このとき第五・第六引数は無視される.
		//	m_pD3DDeviceContext->UpdateSubresource( m_pVertexBuffer, 0, NULL, positions, 0, 0);//sizeof(vec3f) * particle_count
		//結果: Map/UnmapとUpdateSubresourceでは殆ど速度差はない.


		// Render materials
		m_pD3DDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);
		m_pD3DDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers);

		m_pD3DDeviceContext->GSSetShader(NULL, NULL, 0);
		//	m_pD3DDeviceContext->GSSetShader( m_pGeometryShader, NULL, 0 );
		//	m_pD3DDeviceContext->GSSetConstantBuffers(0, 1, m_pConstantBuffers);

		m_pD3DDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);
		//	m_pD3DDeviceContext->PSSetShaderResources( 0, 1, &(m_pTextureRV) );
		//	m_pD3DDeviceContext->PSSetSamplers( 0, 1, &m_pSampler );
	}

	void EndDraw(UINT indexCountPerInstance, UINT instanceCount) {
		if (instanceCount > 0) {
			// インスタンスvertex buffer
			const UINT offset = 0;
			m_pD3DDeviceContext->IASetVertexBuffers(1, 1, &m_pInstanceBuffer, &m_instance_stride, &offset);

			m_pD3DDeviceContext->DrawIndexedInstanced(indexCountPerInstance, instanceCount, 0, 0, 0);
		} else {
			m_pD3DDeviceContext->DrawIndexed(indexCountPerInstance, 0, 0);
		}
		m_pD3DDeviceContext->Flush();

	}

};


class StateManagerDX11 {
public:
	StateManagerDX11(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext) :
		m_D3DDevice(pD3DDevice), 
		m_D3DDeviceContext(pD3DDeviceContext),
		org_pBlendState(NULL),
		org_pDepthStencilState(NULL),
		org_pRasterizerState(NULL),
		m_pBlendState(NULL),
		m_pDepthStencilState(NULL),
		m_pRasterizerState(NULL),
		is_set_blend(false),
		is_set_depth_stencil(false),
		is_set_rasterizer(false)
	{	}

	virtual ~StateManagerDX11() {
		SAFE_RELEASE(m_pBlendState);
		SAFE_RELEASE(m_pDepthStencilState);
		SAFE_RELEASE(m_pRasterizerState);
	}

	bool SetBlendState(const D3D11_BLEND_DESC* blendDesc) {
		HRESULT hr = m_D3DDevice->CreateBlendState(blendDesc, &m_pBlendState);
		return SUCCEEDED(hr);
	}

	bool SetDepthStencilState(const D3D11_DEPTH_STENCIL_DESC* depthStencilState) {
		HRESULT	hr = m_D3DDevice->CreateDepthStencilState(depthStencilState, &m_pDepthStencilState);
		return SUCCEEDED(hr);
	}

	bool SetRasterizerState(const D3D11_RASTERIZER_DESC* rasterizerState) {
		HRESULT	hr = m_D3DDevice->CreateRasterizerState(rasterizerState, &m_pRasterizerState);
		return SUCCEEDED(hr);
	}


	void StoreState() {

		//Blend設定///////////////////////////////
		//デフォルトのブレンドステート.
		if (m_pBlendState) {
			is_set_blend = true;
			m_D3DDeviceContext->OMGetBlendState(&org_pBlendState, org_BlendFactor, &org_SampleMask);
			m_D3DDeviceContext->OMSetBlendState(m_pBlendState, org_BlendFactor, org_SampleMask);
		}

		//ステンシル設定////////////////////////////////////
		//デフォルトのステンシルステート.
		if (m_pDepthStencilState) {
			is_set_depth_stencil = true;
			m_D3DDeviceContext->OMGetDepthStencilState(&org_pDepthStencilState, &org_StencilRef);
			m_D3DDeviceContext->OMSetDepthStencilState(m_pDepthStencilState, 0);
		}

		//ラスタライザ設定////////////////////////////////////
		//デフォルトのラスタライザ.
		if (m_pRasterizerState) {
			is_set_rasterizer = true;
			m_D3DDeviceContext->RSGetState(&org_pRasterizerState);
			m_D3DDeviceContext->RSSetState(m_pRasterizerState);
		}
	}

	void RestoreState() {

		//Blend設定解除///////////////////////////////
		if (is_set_blend) {
			m_D3DDeviceContext->OMSetBlendState(org_pBlendState, org_BlendFactor, org_SampleMask);
		}
		if (is_set_depth_stencil) {
			m_D3DDeviceContext->OMSetDepthStencilState(org_pDepthStencilState, org_StencilRef);
		}
		if (is_set_rasterizer) {
			m_D3DDeviceContext->RSSetState(org_pRasterizerState);
		}

		is_set_blend = false; 
		is_set_depth_stencil = false;
		is_set_rasterizer = false;

	}

private:
	ID3D11Device* m_D3DDevice;
	ID3D11DeviceContext* m_D3DDeviceContext;

	ID3D11BlendState*		org_pBlendState;
	FLOAT org_BlendFactor[4];
	UINT org_SampleMask;

	ID3D11DepthStencilState*		org_pDepthStencilState;
	UINT org_StencilRef;

	ID3D11RasterizerState* org_pRasterizerState;

	ID3D11BlendState* m_pBlendState;
	ID3D11DepthStencilState* m_pDepthStencilState;
	ID3D11RasterizerState* m_pRasterizerState;

	bool is_set_blend;
	bool is_set_depth_stencil;
	bool is_set_rasterizer;

};



#endif    // __BasicRenderer_h__

