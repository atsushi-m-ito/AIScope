#pragma once


#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <math.h>
#include <float.h>
#include <vector>
#include <algorithm>


#include "vec4.h"
#include "BasicRenderer.h"
#include "material_info.h"
#include "Field3DLoader.h"
#include "VertexTypes.h"
#include "visual.h"
#include "TextureManagerDX11.h"
#include "FieldRGBA.h"



class Tex3DRenderer {
protected:


	RendererWrapper renderer;
	//DX11 shader---------------------------------------
	/*
	ID3D11VertexShader* m_pVertexShaderTex;
	ID3D11PixelShader* m_pPixelShaderTex;

	ID3D11Buffer* m_pColorBuffer;
	*/

	
	ID3D11SamplerState* m_pSampler = nullptr;

	ID3D11Texture3D* m_pTexture3D = nullptr;
	ID3D11ShaderResourceView* m_pTextureRV = nullptr;

	StateManagerDX11 m_state_manager;
	
	
	std::vector<VERTEX_F3F3F3> m_vbo_vertex;
	
	std::vector<uint32_t> m_vbo_indexes;



	FieldRGBA<float> field_rgba;


protected:

	HRESULT InitShader() {
		TCHAR filepath[MAX_PATH];

		//Vertex Shader--------------------------------------

			// Define the input layout
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{//namae, buffer内のインデックス, 形式, バッファ種類, 先頭からの格納場所, 頂点orインスタンス, インスタンスの繰り返し.
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);
		GetAttachedFileName(filepath, _T("vs_texture3d.cso"));
		renderer.LoadVertexShader(filepath, "main", "vs_4_1", layout, numElements);


		//Pixel Shader-----------------------------------------

		GetAttachedFileName(filepath, _T("ps_texture3d.cso"));
		renderer.LoadPixelShader(filepath, "main", "ps_4_1");



		// ブレンド・ステート・オブジェクトの作成
		// パーティクル用ブレンド・ステート・オブジェクト
		D3D11_BLEND_DESC blendDesc;
		ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
		blendDesc.AlphaToCoverageEnable = FALSE;
		blendDesc.IndependentBlendEnable = FALSE;
		blendDesc.RenderTarget[0].BlendEnable = TRUE;
		blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;// D11_BLEND_SRC_ALPHA;// D3D11_BLEND_ONE;//3D11_BLEND_SRC_ALPHA;
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;
		blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
		blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ONE;
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
		auto hr = renderer.m_pD3DDevice->CreateSamplerState(&sampDesc, &m_pSampler);
		if (FAILED(hr))
			return hr;



		return S_OK;
	}




	ID3D11ShaderResourceView* mSetTexture3D(const BYTE* rgba, int width_x, int width_y, int width_z) {



		//既に読み込まれていないかチェック//
		//読み込み済みならResourceViewポインタを返す//
		if (m_pTextureRV) return m_pTextureRV;


		//まだデータがセットされていないときはセットする//


		//DirectXにリソースとして登録//
		D3D11_TEXTURE3D_DESC Texture3DDesc;
		Texture3DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;   //textureの場合//
		Texture3DDesc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
		Texture3DDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		Texture3DDesc.Width = width_x;
		Texture3DDesc.Height = width_y;
		Texture3DDesc.Depth = width_z;
		Texture3DDesc.MipLevels = 1;
		Texture3DDesc.MiscFlags = 0;
		Texture3DDesc.Usage = D3D11_USAGE_DEFAULT;


		D3D11_SUBRESOURCE_DATA resource;
		resource.pSysMem = rgba;
		resource.SysMemPitch = width_x * 4;
		resource.SysMemSlicePitch = width_x * width_y * 4;  //3Dテクスチャではざっくりwidth*height*4の意味//

#ifdef _1DEBUG
		test_rgba = new BYTE[width_x * width_y * width_z * 4];
		resource.pSysMem = test_rgba;//rgba;
		for (int iz = 0; iz < width_z; ++iz) {
			for (int iy = 0; iy < width_y; ++iy) {
				for (int ix = 0; ix < width_x; ++ix) {
					const int index = ix + width_x * (iy + width_y * iz);
					test_rgba[index * 4] = (ix * 256) / width_x;
					test_rgba[index * 4+1] = (iy * 256) / width_y;
					test_rgba[index * 4+2] = (iz * 256) / width_z;
					test_rgba[index * 4 + 3] = 255;
				}
			}
		}
#endif // _DEBUG


		HRESULT hr = renderer.m_pD3DDevice->CreateTexture3D(&Texture3DDesc, &resource, &m_pTexture3D);
		if (FAILED(hr)) {
			return nullptr;
		}


		hr = renderer.m_pD3DDevice->CreateShaderResourceView(m_pTexture3D, NULL, &m_pTextureRV);
		if (FAILED(hr)) {
			return nullptr;
		}

		return m_pTextureRV;
	}

	struct RUVW {
		vec3d pos;
		vec3d uvw;
		double angle_order;
		bool operator<(const RUVW& b) {
			return this->angle_order < b.angle_order;
		}
	};

	int CreateLayeredPolygon(const mat33d& boxaxis, const vec3d& boxorg, const float* view_matrix_t, double delta_layer) {

		vec3d camera_dir_x{ (double)view_matrix_t[0], (double)view_matrix_t[1], (double)view_matrix_t[2] }; 
		vec3d camera_dir_y{ (double)view_matrix_t[4], (double)view_matrix_t[5], (double)view_matrix_t[6] };
		vec3d camera_dir_z{ (double)view_matrix_t[8], (double)view_matrix_t[9], (double)view_matrix_t[10] };
		vec3d center = (boxaxis.a + boxaxis.b + boxaxis.c)/2.0 + boxorg ;
		
		double far_z = -DBL_MAX;
		double near_z = + DBL_MAX;
		vec3d box_vertex[8];
		for (uint32_t i = 0; i < 8; ++i) {
			vec3d v = boxorg
				+ ((i & 0x1) ? boxaxis.a : vec3d{ 0.0 ,0.0 ,0.0 })
				+ ((i & 0x2) ? boxaxis.b : vec3d{ 0.0 ,0.0 ,0.0 })
				+ ((i & 0x4) ? boxaxis.c : vec3d{ 0.0 ,0.0 ,0.0 });
			box_vertex[i] = v;
			double distance = (v - center) * camera_dir_z;
			
			if (far_z < distance) far_z = distance;
			if (near_z > distance) near_z = distance;
		} 

		//m_vbo_vertex.clear();
		//m_vbo_indexes.clear();
		//const double length = far_z - near_z;
		//const double dz = (far_z - near_z) / (double)num_slice;
		const int num_slice = (int)((far_z - near_z) / delta_layer);
		//const vec3d zero_pos = center + camera_dir_z * near_z;
		
		std::vector<RUVW> cross_point;
		uint32_t num_vertex = 0;
		for (int i = 0; i < num_slice; ++i) {
			double weight = ((double)(num_slice-i) - 0.5) / (double)num_slice;
			const vec3d surface_o = (weight * far_z + (1.0 - weight) * near_z) * camera_dir_z + center;
			
			cross_point.clear();
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[0], boxaxis.c, { 0.0,0.0,0.0 }, { 0.0,0.0,1.0 }, cross_point);
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[1], boxaxis.c, { 1.0,0.0,0.0 }, { 0.0,0.0,1.0 }, cross_point);
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[2], boxaxis.c, { 0.0,1.0,0.0 }, { 0.0,0.0,1.0 }, cross_point);
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[3], boxaxis.c, { 1.0,1.0,0.0 }, { 0.0,0.0,1.0 }, cross_point);

			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[0], boxaxis.b, { 0.0,0.0,0.0 }, { 0.0,1.0,0.0 }, cross_point);
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[1], boxaxis.b, { 1.0,0.0,0.0 }, { 0.0,1.0,0.0 }, cross_point);
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[4], boxaxis.b, { 0.0,0.0,1.0 }, { 0.0,1.0,0.0 }, cross_point);
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[5], boxaxis.b, { 1.0,0.0,1.0 }, { 0.0,1.0,0.0 }, cross_point);

			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[0], boxaxis.a, { 0.0,0.0,0.0 }, { 1.0,0.0,0.0 }, cross_point);
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[2], boxaxis.a, { 0.0,1.0,0.0 }, { 1.0,0.0,0.0 }, cross_point);
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[4], boxaxis.a, { 0.0,0.0,1.0 }, { 1.0,0.0,0.0 }, cross_point);
			mAddCrossPoint(surface_o, camera_dir_z, box_vertex[6], boxaxis.a, { 0.0,1.0,1.0 }, { 1.0,0.0,0.0 }, cross_point);

			if(cross_point.size() >= 3){
				
				//ordering by angle around camera_z//
				for (auto&& v : cross_point) {
					vec3d nv = v.pos - center;
					nv.Normalize();
					double cos_a = nv * camera_dir_x;
					double cos_b = nv * camera_dir_y;
					v.angle_order = (cos_b >= 0.0) ? (1.0 - cos_a) : cos_a + 3.0;
				}

				std::sort(std::begin(cross_point), std::end(cross_point));

			
				for (const auto& v : cross_point) {
					m_vbo_vertex.emplace_back(VERTEX_F3F3F3({ (float)(v.pos.x), (float)(v.pos.y), (float)(v.pos.z) }, { (float)camera_dir_z.x, (float)camera_dir_z.y, (float)camera_dir_z.z }, v.uvw.x, v.uvw.y, v.uvw.z));					
				}
				const int i_end = (int)(cross_point.size());
				for (int i = 2; i < i_end; ++i) {
					m_vbo_indexes.push_back(num_vertex);
					m_vbo_indexes.push_back(num_vertex + i - 1);
					m_vbo_indexes.push_back(num_vertex + i);
				}

				num_vertex += i_end;

			}
		}
		return 0;
	}

	double mCrossPoint(const vec3d& surface_o, const vec3d& surface_normal, const vec3d& line_p, const vec3d& line_vec) {
		return ((surface_o - line_p) * surface_normal) / (line_vec * surface_normal);
	}

	void mAddCrossPoint(const vec3d& surface_o, const vec3d& surface_normal, const vec3d& line_p, const vec3d& line_vec, const vec3d& uvw_p, const vec3d& uvw_v, std::vector<RUVW>& cross_point) {
		double t = mCrossPoint(surface_o, surface_normal, line_p, line_vec);
		if ((0.0 <= t) && (t < 1.0)) {			
			cross_point.push_back({ line_p + t * line_vec, uvw_p + t * uvw_v, 0.0 });
		}
	}


	int CreateCrosssection(const mat33d& boxaxis, const vec3d& boxorg) {

		vec3d box_vertex[8];
		vec3f uvw[8];
		for (uint32_t i = 0; i < 8; ++i) {
			vec3d v = boxorg
				+ ((i & 0x1) ? boxaxis.a : vec3d{ 0.0 ,0.0 ,0.0 })
				+ ((i & 0x2) ? boxaxis.b : vec3d{ 0.0 ,0.0 ,0.0 })
				+ ((i & 0x4) ? boxaxis.c : vec3d{ 0.0 ,0.0 ,0.0 });
			box_vertex[i] = v;

			uvw[i] = vec3f{ (i & 0x1) ? 1.0f : 0.0f, (i & 0x2) ? 1.0f : 0.0f, (i & 0x4) ? 1.0f : 0.0f };
		}

		//m_vbo_vertex.clear();
		//m_vbo_indexes.clear();


		vec3d normal_c = boxaxis.c;
		normal_c.Normalize();
		mAddV(box_vertex[0], -normal_c, uvw[0]);
		mAddV(box_vertex[1], -normal_c, uvw[1]);
		mAddV(box_vertex[2], -normal_c, uvw[2]);
		mAddV(box_vertex[3], -normal_c, uvw[3]);
		mAddV(box_vertex[4], normal_c, uvw[4]);
		mAddV(box_vertex[5], normal_c, uvw[5]);
		mAddV(box_vertex[6], normal_c, uvw[6]);
		mAddV(box_vertex[7], normal_c, uvw[7]); 
		m_vbo_indexes.push_back(0);
		m_vbo_indexes.push_back(1);
		m_vbo_indexes.push_back(2);
		m_vbo_indexes.push_back(2);
		m_vbo_indexes.push_back(1);
		m_vbo_indexes.push_back(3);

		m_vbo_indexes.push_back(4);
		m_vbo_indexes.push_back(6);
		m_vbo_indexes.push_back(5);
		m_vbo_indexes.push_back(6);
		m_vbo_indexes.push_back(7);
		m_vbo_indexes.push_back(5);


		vec3d normal_b = boxaxis.b;
		normal_b.Normalize();
		mAddV(box_vertex[0], -normal_b, uvw[0]);
		mAddV(box_vertex[1], -normal_b, uvw[1]);
		mAddV(box_vertex[2], normal_b, uvw[2]);
		mAddV(box_vertex[3], normal_b, uvw[3]);
		mAddV(box_vertex[4], -normal_b, uvw[4]);
		mAddV(box_vertex[5], -normal_b, uvw[5]);
		mAddV(box_vertex[6], normal_b, uvw[6]);
		mAddV(box_vertex[7], normal_b, uvw[7]);
		m_vbo_indexes.push_back(0 + 8);
		m_vbo_indexes.push_back(4 + 8);
		m_vbo_indexes.push_back(1 + 8);
		m_vbo_indexes.push_back(4 + 8);
		m_vbo_indexes.push_back(5 + 8);
		m_vbo_indexes.push_back(1 + 8);
		m_vbo_indexes.push_back(2 + 8);
		m_vbo_indexes.push_back(3 + 8);
		m_vbo_indexes.push_back(6 + 8);
		m_vbo_indexes.push_back(3 + 8);
		m_vbo_indexes.push_back(7 + 8);
		m_vbo_indexes.push_back(6 + 8);

		vec3d normal_a = boxaxis.a;
		normal_a.Normalize();
		mAddV(box_vertex[0], -normal_a, uvw[0]);
		mAddV(box_vertex[1], normal_a, uvw[1]);
		mAddV(box_vertex[2], -normal_a, uvw[2]);
		mAddV(box_vertex[3], normal_a, uvw[3]);
		mAddV(box_vertex[4], -normal_a, uvw[4]);
		mAddV(box_vertex[5], normal_a, uvw[5]);
		mAddV(box_vertex[6], -normal_a, uvw[6]);
		mAddV(box_vertex[7], normal_a, uvw[7]);

		m_vbo_indexes.push_back(0 + 16);
		m_vbo_indexes.push_back(2 + 16);
		m_vbo_indexes.push_back(4 + 16);
		m_vbo_indexes.push_back(2 + 16);
		m_vbo_indexes.push_back(6 + 16);
		m_vbo_indexes.push_back(4 + 16);

		m_vbo_indexes.push_back(1 + 16);
		m_vbo_indexes.push_back(5 + 16);
		m_vbo_indexes.push_back(3 + 16);
		m_vbo_indexes.push_back(5 + 16);
		m_vbo_indexes.push_back(7 + 16);
		m_vbo_indexes.push_back(3 + 16);

		return 0;
	}

	void mAddV(const vec3d& v, const vec3d& normal, const vec3f& uvw) {
		m_vbo_vertex.emplace_back(VERTEX_F3F3F3(
			{ (float)(v.x), (float)(v.y), (float)(v.z) },
			{ (float)normal.x, (float)normal.y, (float)normal.z },
			uvw.x, uvw.y, uvw.z));
	}

public:

	Tex3DRenderer(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		renderer(pD3DDevice, pD3DDeviceContext, comShader),
		m_state_manager(pD3DDevice, pD3DDeviceContext)
	{
		InitShader();
	}
	virtual ~Tex3DRenderer() {
		m_pTextureRV->Release();
		m_pTexture3D->Release();		
	}

	void SetViewAndProjectionMatrix(const float* pViewMatrix, const float* pProjectionMatrix) {
		renderer.SetViewAndProjectionMatrix(pViewMatrix, pProjectionMatrix);
	}




	void DrawField(const FIELD3D_FRAME* field, const float* view_matrix_t) {


		m_vbo_vertex.clear();
		m_vbo_indexes.clear();

		const double delta_layer = std::min<double>(std::min<double>((field->boxaxis.a.x / field->grid_x), (field->boxaxis.b.y / field->grid_y)), (field->boxaxis.c.z / field->grid_z));
		CreateLayeredPolygon(field->boxaxis, field->boxorg, view_matrix_t, delta_layer);
		//CreateCrosssection(field->boxaxis, field->boxorg);
		field_rgba.Convert(field->fieldb, field->grid_x, field->grid_y, field->grid_z);

		ID3D11ShaderResourceView* pTextureRV = mSetTexture3D(field_rgba.DataRGBA(), field->grid_x, field->grid_y, field->grid_z);

		if (m_vbo_indexes.empty()) return;

		m_state_manager.StoreState();


		//バッファのセット.	
		UINT offset = 0;
		//m_pD3DDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_vertex_stride, &offset);
		renderer.UpdateVertexBuffer(&m_vbo_vertex[0], m_vbo_vertex.size(), sizeof(VERTEX_F3F3F3));	//頂点データをVRAMに転送
		renderer.UpdateIndexBuffer(&m_vbo_indexes[0], m_vbo_indexes.size(), sizeof(uint32_t));	//インデックスデータをVRAMに転送

		renderer.m_pD3DDeviceContext->PSSetSamplers(0, 1, &m_pSampler);

		renderer.m_pD3DDeviceContext->PSSetShaderResources(0, 1, &pTextureRV);

		renderer.DrawIndexed(m_vbo_indexes.size(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		m_state_manager.RestoreState();

	}


};


