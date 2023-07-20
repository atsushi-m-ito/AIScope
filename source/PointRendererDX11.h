// PointRendererDX11.h: PointRendererDX11 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#ifndef PointRendererDX11_h
#define PointRendererDX11_h

#pragma once

#include <d3d11.h>


#include "MDLoader.h"	//ATOMS_DATAのためだけに必要//
#include "BOND_INFO.h"

//#include "sdkwgl.h"
#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "io64.h"
#include "mod.h"


#include "visual.h"


#include "BasicRenderer.h"
#include "VertexTypes.h"

extern size_t g_max_vram_size;



class BasePointRendererDX11 : public BasicRenderer {

protected:
	ID3D11GeometryShader*     m_pGeometryShader = nullptr;
	ID3D11Buffer* m_cbPoint = nullptr;

	StateManagerDX11 m_state_manager;

	struct vec2f {
		float	x;
		float	y;
	} m_pPointSize{ 0.5f,0.5f };


	enum POINT_SPRITE_LAYOUT {
		RGB_FLOAT_COLOR_U8,
		RGB_FLOAT_COLOR_FLOAT
	};

	
public:


	BasePointRendererDX11(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		BasicRenderer(pD3DDevice, pD3DDeviceContext, comShader, 0, 0),
		m_state_manager(pD3DDevice, pD3DDeviceContext)
	{
	}


	virtual ~BasePointRendererDX11() {
		if (m_pGeometryShader) m_comShader->UnloadShader((LPVOID)m_pGeometryShader);
		SAFE_RELEASE(m_cbPoint);
	}



	void SetPointSize(float width) {
		m_pPointSize.x = width;
		m_pPointSize.y = width;
	}

protected:

	HRESULT InitShader(POINT_SPRITE_LAYOUT layout_mode) {
		TCHAR filepath[MAX_PATH];

		//Vertex Shader--------------------------------------
		// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout_u8[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		};
		D3D11_INPUT_ELEMENT_DESC layout_float[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 }

		};
		D3D11_INPUT_ELEMENT_DESC* layout=nullptr;
		UINT numElements=0;
		switch (layout_mode) {
		case RGB_FLOAT_COLOR_U8:
			layout = layout_u8;
			numElements = ARRAYSIZE(layout_u8);
			break;
		case RGB_FLOAT_COLOR_FLOAT:
			layout = layout_float;
			numElements = ARRAYSIZE(layout_float);
			break;
		}

		GetAttachedFileName(filepath, _T("vs_point_color.cso"));
		//GetAttachedFileName(filepath, _T("vs_point_color_to_ps.cso"));
		m_pVertexShader = m_comShader->LoadVertexShader(filepath, "main", "vs_4_0", layout, numElements, &m_pVertexLayout);


		//Geometry Shader-----------------------------------------
		GetAttachedFileName(filepath, _T("gs_point_color.cso"));
		m_pGeometryShader = m_comShader->LoadGeometryShader(filepath, "main", "gs_4_0");


		//Pixel Shader-----------------------------------------
		GetAttachedFileName(filepath, _T("ps_point_color.cso"));
		m_pPixelShader = m_comShader->LoadPixelShader(filepath, "main", "ps_4_0");

		//-----------------------------------------------------------------------
		//Constant bufferの作成
		//ポイントスプライトの点の大きさ//
		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = sizeof(mat44f) + sizeof(float) * 4;	//16の倍数である必要がある//
		bd.Usage = D3D11_USAGE_DYNAMIC;
		bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
		bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;	//CPUからの書き換え
		bd.MiscFlags = 0;
		bd.StructureByteStride = 0;

		HRESULT hr = m_pD3DDevice->CreateBuffer(&bd, NULL, &m_cbPoint);
		if (FAILED(hr))
			return hr;


		// ブレンド・ステート・オブジェクトの作成
		// パーティクル用ブレンド・ステート・オブジェクト
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;//D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
		blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

		m_state_manager.SetBlendState(&blendDesc);

		//深度バッファを書き込みしない.
		// デプス・ステート・オブジェクトの作成
		// パーティクル用デプス・ステート・オブジェクト
		//ポイントスプライトではデプスバッファのチェックだけして、更新はしない.
		D3D11_DEPTH_STENCIL_DESC depthStencilState;
		ZeroMemory(&depthStencilState, sizeof(D3D11_DEPTH_STENCIL_DESC));
		depthStencilState.DepthEnable = TRUE;
		depthStencilState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ZERO;	//深度バッファを書き込みしない.
		depthStencilState.DepthFunc = D3D11_COMPARISON_LESS;
		depthStencilState.StencilEnable = FALSE;
		depthStencilState.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
		depthStencilState.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;

		m_state_manager.SetDepthStencilState(&depthStencilState);

		// ラスタライザの作成//
		//カリング等を制御//
		D3D11_RASTERIZER_DESC rasterizerState;
		ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));
		rasterizerState.FillMode = D3D11_FILL_SOLID;
		rasterizerState.CullMode = D3D11_CULL_NONE;//両面//
		rasterizerState.FrontCounterClockwise = TRUE;
		rasterizerState.DepthBias = 0;
		rasterizerState.DepthBiasClamp = 0;
		rasterizerState.SlopeScaledDepthBias = 0;
		rasterizerState.DepthClipEnable = FALSE;
		rasterizerState.ScissorEnable = FALSE;
		rasterizerState.MultisampleEnable = FALSE;
		rasterizerState.AntialiasedLineEnable = FALSE;

		m_state_manager.SetRasterizerState(&rasterizerState);


		return S_OK;
	}

	
	
	virtual void DrawOnlyVertex(UINT particle_count, D3D_PRIMITIVE_TOPOLOGY primitive_topology)
	{

		m_state_manager.StoreState();

		//変換行列
		mat44f world_matrix;
		world_matrix.SetIdentity();


		//定数バッファの書き込み.
		D3D11_MAPPED_SUBRESOURCE mappedResource;
		m_pD3DDeviceContext->Map(m_cbPoint, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
		CopyMemory(mappedResource.pData, &m_ProjectionMatrix, sizeof(mat44f));
		CopyMemory((mat44f*)mappedResource.pData + 1, &m_pPointSize, sizeof(vec2f));
		m_pD3DDeviceContext->Unmap(m_cbPoint, 0);


		//バッファのセット.	
		// Set the input layout
		m_pD3DDeviceContext->IASetInputLayout(m_pVertexLayout);
		// Set primitive topology
		m_pD3DDeviceContext->IASetPrimitiveTopology(primitive_topology);// D3D_PRIMITIVE_TOPOLOGY_POINTLIST );

		UINT offset = 0;
		m_vertexBuffer.Attach();


		// Render materials
		m_pD3DDeviceContext->VSSetShader(m_pVertexShader, NULL, 0);
		m_pD3DDeviceContext->VSSetConstantBuffers(0, 1, m_pConstantBuffers);

		m_pD3DDeviceContext->GSSetShader(m_pGeometryShader, NULL, 0);
		m_pD3DDeviceContext->GSSetConstantBuffers(1, 1, &m_cbPoint);

		m_pD3DDeviceContext->PSSetShader(m_pPixelShader, NULL, 0);
		//	m_dx11_core->pD3DDeviceContext->PSSetShaderResources( 0, 1, &(m_pTextureRV) );
		//m_dx11_core->pD3DDeviceContext->PSSetSamplers( 0, 1, &m_pSampler );
		m_pD3DDeviceContext->Draw(particle_count, 0);
		m_pD3DDeviceContext->Flush();


		m_state_manager.RestoreState();
	}


};



class PointRendererDX11 : public BasePointRendererDX11 {
	
private:

	int m_vram_size = 0;
	int m_vbo_vcount = 0;
	VERTEX_F3UI1* m_vbo_vertex = nullptr;
	
	unsigned int *m_color_table = nullptr;	//原子ごとの色が入る

	ATOMS_DATA* m_prev_render_dat = nullptr;	//前回レンダリングしたデータ
	
	VISUAL_SETTING m_prev_vis;

public:

	PointRendererDX11(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		BasePointRendererDX11(pD3DDevice, pD3DDeviceContext, comShader)
	{
		SetPointSize(0.5f);
		InitShader(RGB_FLOAT_COLOR_U8);
	}


	virtual ~PointRendererDX11() {
		delete[] m_vbo_vertex;
		delete[] m_color_table;
	}


	//VBOの設定
	void Draw(ATOMS_DATA* dat, BOND_INFO* bond, const VISUAL_SETTING& vis) {	//int j_mode, float Jmax



		if (dat == NULL) return;


		//vboのセット
		int redraw_flag = 1;
		if (vis.atom_poly != m_prev_vis.atom_poly) {
			redraw_flag = 1;
		}

		//原子表示色のモード設定//
		if (vis.atom_color != m_prev_vis.atom_color) {
			redraw_flag = 1;
		}

		if (dat != m_prev_render_dat) {
			redraw_flag = 1;
		}


		//カラーテーブル作り
		if (redraw_flag) {
			if (vis.atom_color == VISUAL_ATOMCOLOR_BONDNUM) {
				//if(dat->bondInsideCnt == -1){
				CheckColorByBondNum2(dat, bond);
				//}else{
				//	CheckColorByBondNum(dat);
				//}
			} else if (vis.atom_color == VISUAL_ATOMCOLOR_PRESSURE) {
				//if(dat->bondInsideCnt == -1){
				CheckColorByPressure(dat, bond);
				//}else{
				//	CheckColorByBondNum(dat);
				//}
			} else {	//MODE_COLOR_ATOMIC
				CheckColorByAtomKind(dat);
			}
		}


		//原子球の描画//
		if (vis.atom) {
			const int count = dat->pcnt;
			if (vis.bond) { redraw_flag = 1; }

			//頂点数の計算//
			size_t num_draw_once = g_max_vram_size / (sizeof(VERTEX_F3UI1));
			if (count > num_draw_once) {//頂点のバッファサイズを超える場合は複数回に分けて描画する//
				redraw_flag = 1;
			}

			size_t i_start = 0;
			while (i_start < count) {

				if (i_start + num_draw_once > count) { num_draw_once = count - i_start; };


				if (redraw_flag) {

					CreateSphere(i_start, num_draw_once, dat, bond);
				}


				if (m_vbo_vcount) {

					UpdateVertexBuffer(m_vbo_vertex, m_vbo_vcount, sizeof(VERTEX_F3UI1));	//頂点データをVRAMに転送

					DrawOnlyVertex(m_vbo_vcount, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);
				}

				i_start += num_draw_once;
			}

		}




		m_prev_render_dat = dat;
		m_prev_vis = vis;
	}

	

private:


	void CreateSphere(int start, int count, ATOMS_DATA* dat, BOND_INFO* bond) {

		//vboサイズ
		int vbo_v_count = count;
		int need_vram_sz = vbo_v_count * sizeof(VERTEX_F3UI1);



		if (m_vram_size < need_vram_sz) {
			m_vram_size = need_vram_sz;

			delete[] m_vbo_vertex;
			m_vbo_vertex = new VERTEX_F3UI1[vbo_v_count];
		}



		m_vbo_vcount = 0;




		//原子(球)のセット
		const int i_end = start + count;
		for (int i = start; i < i_end; i++) {
			if (m_color_table[i] & 0xFF000000) {	//完全透過は表示しない//
				m_vbo_vertex[m_vbo_vcount].position = dat->r[i];
				m_vbo_vertex[m_vbo_vcount].color = m_color_table[i];

				m_vbo_vcount++;
			}
		}


	}

	///////////////////////////////////////////////
	//
	//VBOにポリゴンをセットする機能
	//
	////////////////////////////////////////////////


	//原子番号を元にカラーを計算
	void CheckColorByAtomKind(ATOMS_DATA* dat) {



		delete[] m_color_table;
		m_color_table = new unsigned int[dat->pcnt];

		//色を参照
		if (dat->knd) {
			for (int i = 0; i < dat->pcnt; i++) {
				m_color_table[i] = *(((unsigned int*)atom_colors) + dat->knd[i]);
			}
		} else {
			for (int i = 0; i < dat->pcnt; i++) {
				m_color_table[i] = *(((unsigned int*)atom_colors));
			}
		}
	}


	//ボンド数を元にカラーを計算
	void CheckColorByBondNum2(ATOMS_DATA* dat, BOND_INFO* bond) {



		delete[] m_color_table;
		m_color_table = new unsigned int[dat->pcnt];
		memset(m_color_table, 0, sizeof(unsigned int) * dat->pcnt);

		//bond数を計算

		//色を参照
		for (int i = 0; i < dat->pcnt; i++) {
			if (bond->coords[i] >= COLOR_LIMIT_BOND) {
				m_color_table[i] = *(((unsigned int*)bondnum_colors) + COLOR_LIMIT_BOND - 1);
			} else {
				m_color_table[i] = *(((unsigned int*)bondnum_colors) + bond->coords[i]);
			}
		}
	}


	//ボンド数を元にカラーを計算
	void CheckColorByPressure(ATOMS_DATA* dat, BOND_INFO* bond) {



		delete[] m_color_table;
		m_color_table = new unsigned int[dat->pcnt];
		memset(m_color_table, 0, sizeof(unsigned int) * dat->pcnt);

		//bond数を計算

		//色を参照
		for (int i = 0; i < dat->pcnt; i++) {
			m_color_table[i] = bond->coords[i];
		}
	}

};





#endif // PointRendererDX11

