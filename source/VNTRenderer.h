
#ifndef __VNTRenderer_h__
#define __VNTRenderer_h__


#pragma once


#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <math.h>


#include "vec4.h"
#include "BasicRenderer.h"
#include "material_info.h"

#include "TextureManagerDX11.h"



class VNTRenderer : public BasicRenderer{
protected:


//DX11 shader---------------------------------------
	
	ID3D11VertexShader* m_pVertexShaderTex = nullptr;
	ID3D11PixelShader* m_pPixelShaderTex = nullptr;
	ID3D11Buffer* m_pColorBuffer = nullptr;
	ID3D11ShaderResourceView*	m_pTextureRVArray = nullptr;
	ID3D11SamplerState*     m_pSampler = nullptr;
    TextureManager m_texture_managrer;	
	StateManagerDX11 m_state_manager;

public:
	
	
	VNTRenderer(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		BasicRenderer(pD3DDevice, pD3DDeviceContext, comShader, 0, 0),
		m_texture_managrer(pD3DDevice),
		m_state_manager(pD3DDevice, pD3DDeviceContext)
	{
		InitShader();
	}

	virtual ~VNTRenderer() {
		SAFE_RELEASE(m_pColorBuffer);
		SAFE_RELEASE(m_pVertexShaderTex);
		SAFE_RELEASE(m_pPixelShaderTex);
		SAFE_RELEASE(m_pSampler);
	}

	


	void VNTRenderer::DrawMaterials(int material_count, MATERIAL_INFO* materials, mat44f* pworld_matrix) {

		m_state_manager.StoreState();

		// Set the input layout
		m_pD3DDeviceContext->IASetInputLayout(m_pVertexLayout);

		// Set primitive topology
		m_pD3DDeviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);



		//バッファのセット.	
		UINT offset = 0;
		//m_pD3DDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_vertex_stride, &offset);
		m_vertexBuffer.Attach();
		m_pD3DDeviceContext->IASetIndexBuffer(m_pIndexBuffer, m_index_format, 0);


		m_pD3DDeviceContext->GSSetShader(NULL, NULL, 0);

		m_pD3DDeviceContext->PSSetSamplers(0, 1, &m_pSampler);



		for (int i = 0; i < material_count; i++) {
			if (materials[i].texture_name) {
				//テクスチャでの表示

				// Render materials
				m_pD3DDeviceContext->VSSetShader(m_pVertexShaderTex, NULL, 0);
				m_pD3DDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers);

				//m_dx11_core->pD3DDeviceContext->PSSetSamplers(0, 1, &m_pSampler);

				m_pD3DDeviceContext->PSSetShader(m_pPixelShaderTex, NULL, 0);

				int width, height;
				ID3D11ShaderResourceView* pTextureRV = m_texture_managrer.LoadTexture(materials[i].texture_name, &width, &height);
				m_pD3DDeviceContext->PSSetShaderResources(0, 1, &pTextureRV);
				m_pD3DDeviceContext->DrawIndexed(materials[i].indexCount, materials[i].indexOffset, 0);

			} else {    //マテリアルカラー指定での表示

				//colorを定数バッファとして送る.
				D3D11_MAPPED_SUBRESOURCE mappedResource;
				m_pD3DDeviceContext->Map(m_pColorBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
				CopyMemory((float*)mappedResource.pData, materials[i].diffuse_color, sizeof(float) * 4);
				m_pD3DDeviceContext->Unmap(m_pColorBuffer, 0);

				m_pD3DDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);
				m_pD3DDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers);

				m_pD3DDeviceContext->PSSetConstantBuffers(1, 1, &m_pColorBuffer);

				m_pD3DDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);
				m_pD3DDeviceContext->DrawIndexed(materials[i].indexCount, materials[i].indexOffset, 0);
			}
		}
		m_pD3DDeviceContext->Flush();

		m_state_manager.RestoreState();

	}


	//ラッパー//
	void VNTRenderer::DrawMaterials(int material_count, MATERIAL_INFO* materials, vec3f* position) {
		mat44f world_matrix;
		world_matrix.SetIdentity();

		//world (position) matrix.
		world_matrix.m41 = position->x;
		world_matrix.m42 = position->y;
		world_matrix.m43 = position->z;

		DrawMaterials(material_count, materials, &world_matrix);

	}


protected:



	HRESULT InitShader() {
		TCHAR filepath[MAX_PATH];

		//Vertex Shader--------------------------------------

			// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{//namae, buffer内のインデックス, 形式, バッファ種類, 先頭からの格納場所, 頂点orインスタンス, インスタンスの繰り返し.
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);
		GetAttachedFileName(filepath, _T("vs_texture.cso"));
		m_pVertexShaderTex = m_comShader->LoadVertexShader(filepath, "main", "vs_4_0", layout, numElements, &m_pVertexLayout);

		GetAttachedFileName(filepath, _T("vs_pnt2pn.cso"));
		m_pVertexShader = m_comShader->LoadVertexShader(filepath, "main", "vs_4_0", layout, numElements, &m_pVertexLayout);


		//Pixel Shader-----------------------------------------

		GetAttachedFileName(filepath, _T("ps_set_color.cso"));
		m_pPixelShader = m_comShader->LoadPixelShader(filepath, "main", "ps_4_0");

		GetAttachedFileName(filepath, _T("ps_texture.cso"));
		m_pPixelShaderTex = m_comShader->LoadPixelShader(filepath, "main", "ps_4_0");


		//-----------------------------------------------------------------------
		//Constant bufferの作成
		//constant bufferとは、カメラ位置やワールド変換行列など,
		//CPUからもGPUの各スレッドからも高速にアクセスするバッファ.
		//ワールド行列:モデルをglobal空間に設置するときの位置(平行移動)や向き(回転)をセットする行列.
		//ビュー行列:カメラから見た座標系へ変換.
		//射影行列:2次元座標へ変換


			//projection, view, world matrixes
		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = sizeof(mat44f) * 3;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//CPUからの書き換え
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;
		//SUBRESOURCEによる初期値は設定しない

		HRESULT hr = m_pD3DDevice->CreateBuffer(&bd, NULL, &m_pWVPBuffer);
		if (FAILED(hr))
			return hr;


		bd.ByteWidth = sizeof(float) * 4;
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//CPUからの書き換え
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;
		//SUBRESOURCEによる初期値は設定しない

		hr = m_pD3DDevice->CreateBuffer(&bd, NULL, &m_pColorBuffer);
		if (FAILED(hr))
			return hr;

		/*	//Light position.
			bd.ByteWidth = sizeof( float )*4;
			hr = pD3DDevice->CreateBuffer( &bd, NULL, &m_pLightBuffer );
			if( FAILED( hr ) )
				return hr;
		*/


		// ラスタライザの作成//
		//カリング等を制御//
		D3D11_RASTERIZER_DESC rasterizerState;
		ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerState.FillMode = D3D11_FILL_SOLID;    //D3D11_FILL_SOLID or D3D11_FILL_WIREFRAME//
		rasterizerState.CullMode = D3D11_CULL_NONE;     //D3D11_CULL_NONE or D3D11_CULL_BACK or D3D11_CULL_FRONT
		rasterizerState.FrontCounterClockwise = TRUE;
		rasterizerState.DepthBias = 0;
		rasterizerState.DepthBiasClamp = 0;
		rasterizerState.SlopeScaledDepthBias = 0;
		rasterizerState.DepthClipEnable = FALSE;
		rasterizerState.ScissorEnable = FALSE;
		rasterizerState.MultisampleEnable = FALSE;
		rasterizerState.AntialiasedLineEnable = FALSE;

		m_state_manager.SetRasterizerState(&rasterizerState);


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
		hr = m_pD3DDevice->CreateSamplerState(&sampDesc, &m_pSampler);
		if (FAILED(hr))
			return hr;



		return S_OK;
	}


};



#endif    // __VNTRenderer_h__

