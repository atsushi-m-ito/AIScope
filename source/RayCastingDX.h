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
#include "CrossSection.h"



class RayCastingDX {
private:
	static constexpr double VIEW_NEAR = 0.1;
protected:


	RendererWrapper renderer;
	//DX11 shader---------------------------------------
	/*
	ID3D11VertexShader* m_pVertexShaderTex;
	ID3D11PixelShader* m_pPixelShaderTex;

	ID3D11Buffer* m_pColorBuffer;
	*/


	ID3D11SamplerState* m_pSampler = nullptr;
	ID3D11SamplerState* m_pSampler_cm = nullptr;	

	ID3D11Texture3D* m_pTexture3D = nullptr;
	ID3D11ShaderResourceView* m_pTextureRV = nullptr;

	ID3D11Texture1D* m_pTexture_cm = nullptr;
	ID3D11ShaderResourceView* m_pTextureRV_cm = nullptr;


	StateManagerDX11 m_state_manager;

	ID3D11Buffer* m_pCB_num_marching = nullptr;
	ID3D11Buffer* m_pCB_cross_section = nullptr;
	

	std::vector<VERTEX_F3F3F3> m_vbo_vertex;

	std::vector<uint32_t> m_vbo_indexes;





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
		GetAttachedFileName(filepath, _T("vs_raycasting.cso"));
		renderer.LoadVertexShader(filepath, "main", "vs_4_1", layout, numElements);


		//Pixel Shader-----------------------------------------

		GetAttachedFileName(filepath, _T("ps_raycasting.cso"));
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
		blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_INV_SRC_ALPHA;// D3D11_BLEND_INV_SRC_ALPHA;
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
		rasterizerState.CullMode = D3D11_CULL_FRONT;
		rasterizerState.FrontCounterClockwise = TRUE;
		rasterizerState.DepthBias = 0;
		rasterizerState.DepthBiasClamp = 0;
		rasterizerState.SlopeScaledDepthBias = 0;
		rasterizerState.DepthClipEnable = FALSE;
		rasterizerState.ScissorEnable = FALSE;
		rasterizerState.MultisampleEnable = FALSE;
		rasterizerState.AntialiasedLineEnable = FALSE;

		m_state_manager.SetRasterizerState(&rasterizerState);



		// 3Dテクスチャのサンプリング(ポリゴン貼り付け時の補間)方法の指定.
		D3D11_SAMPLER_DESC sampDesc;
		ZeroMemory(&sampDesc, sizeof(sampDesc));
		sampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;// D3D11_FILTER_ANISOTROPIC;// D3D11_FILTER_MIN_MAG_MIP_LINEAR;
		sampDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;//D3D11_TEXTURE_ADDRESS_BORDER
		sampDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
		sampDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
		sampDesc.BorderColor[0] = 0.0f;
		sampDesc.BorderColor[1] = 0.0f;
		sampDesc.BorderColor[2] = 0.0f;
		sampDesc.BorderColor[3] = 0.0f;
		sampDesc.MinLOD = 0;
		sampDesc.MaxLOD = D3D11_FLOAT32_MAX;
		auto hr = renderer.m_pD3DDevice->CreateSamplerState(&sampDesc, &m_pSampler);
		if (FAILED(hr))
			return hr;
		
		// カラーマップテクスチャのサンプリング(ポリゴン貼り付け時の補間)方法の指定.
		{
			D3D11_SAMPLER_DESC desc;
			ZeroMemory(&desc, sizeof(desc));
			desc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;// D3D11_FILTER_ANISOTROPIC;// D3D11_FILTER_MIN_MAG_MIP_LINEAR;
			desc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
			desc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;//this is needed to create even if 1D.
			desc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
			desc.ComparisonFunc = D3D11_COMPARISON_NEVER;
			desc.MinLOD = 0;
			desc.MaxLOD = D3D11_FLOAT32_MAX;
			auto hr = renderer.m_pD3DDevice->CreateSamplerState(&desc, &m_pSampler_cm);
			if (FAILED(hr))
				return hr;
		}

		{
			D3D11_BUFFER_DESC bd;
			bd.ByteWidth = 64;	//16の倍数である必要がある//
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// D3D11_CPU_ACCESS_WRITE;	//CPUからの書き換え
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			//SUBRESOURCEによる初期値は設定しない

			hr = renderer.m_pD3DDevice->CreateBuffer(&bd, nullptr, &m_pCB_num_marching);
			if (FAILED(hr))
				return hr;
		}

		
		{
			D3D11_BUFFER_DESC bd;
			bd.ByteWidth = 8*9*4;// 56 * 4;// 16 * 9;	//16の倍数である必要がある//
			bd.Usage = D3D11_USAGE_DYNAMIC;
			bd.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
			bd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;// D3D11_CPU_ACCESS_WRITE;	//CPUからの書き換え
			bd.MiscFlags = 0;
			bd.StructureByteStride = 0;
			//SUBRESOURCEによる初期値は設定しない

			hr = renderer.m_pD3DDevice->CreateBuffer(&bd, nullptr, &m_pCB_cross_section);
			if (FAILED(hr))
				return hr;
		}
		

		return S_OK;
	}




	ID3D11ShaderResourceView* mSetTexture3D(const float* field, int width_x, int width_y, int width_z) {



		//既に読み込まれていないかチェック//
		//読み込み済みならResourceViewポインタを返す//
		if (m_pTextureRV) return m_pTextureRV;


		//まだデータがセットされていないときはセットする//


		//DirectXにリソースとして登録//
		D3D11_TEXTURE3D_DESC Texture3DDesc;
		Texture3DDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;   //textureの場合//
		Texture3DDesc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
		Texture3DDesc.Format = DXGI_FORMAT_R32_FLOAT;
		Texture3DDesc.Width = width_x;
		Texture3DDesc.Height = width_y;
		Texture3DDesc.Depth = width_z;
		Texture3DDesc.MipLevels = 1;
		Texture3DDesc.MiscFlags = 0;
		Texture3DDesc.Usage = D3D11_USAGE_DEFAULT;


		D3D11_SUBRESOURCE_DATA resource;
		resource.pSysMem = field;
		resource.SysMemPitch = width_x * 4;
		resource.SysMemSlicePitch = width_x * width_y * 4;  //3Dテクスチャではざっくりwidth*height*4の意味//


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


	ID3D11ShaderResourceView* mSetTextureColorMap(const uint8_t* color_map_rgba, int num_colors) {

		//既に読み込まれていないかチェック//
		//読み込み済みならResourceViewポインタを返す//
		if (m_pTextureRV_cm) return m_pTextureRV_cm;


		//まだデータがセットされていないときはセットする//


		//DirectXにリソースとして登録//
		D3D11_TEXTURE1D_DESC desc;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;   //textureの場合//
		desc.CPUAccessFlags = 0;//D3D11_CPU_ACCESS_WRITE;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.Width = num_colors;
		desc.ArraySize = 1;
		desc.MipLevels = 1;
		desc.MiscFlags = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;


		D3D11_SUBRESOURCE_DATA resource;
		resource.pSysMem = color_map_rgba;
		resource.SysMemPitch = num_colors * 4;
		resource.SysMemSlicePitch = num_colors * 4;  //3Dテクスチャではざっくりwidth*height*4の意味//


		HRESULT hr = renderer.m_pD3DDevice->CreateTexture1D(&desc, &resource, &m_pTexture_cm);
		if (FAILED(hr)) {
			return nullptr;
		}


		hr = renderer.m_pD3DDevice->CreateShaderResourceView(m_pTexture_cm, NULL, &m_pTextureRV_cm);
		if (FAILED(hr)) {
			return nullptr;
		}

		return m_pTextureRV_cm;
	}

	struct RUVW {
		vec3d pos;
		vec3d uvw;
		double angle_order;
		bool operator<(const RUVW& b) {
			return this->angle_order < b.angle_order;
		}
	};

	void mAddV(const vec3d& v, const vec3d& normal, const vec3f& uvw) {
		m_vbo_vertex.emplace_back(VERTEX_F3F3F3(
			{ (float)(v.x), (float)(v.y), (float)(v.z) },
			{ (float)normal.x, (float)normal.y, (float)normal.z },
			uvw.x, uvw.y, uvw.z));
	}


	int CreateRaySurface(const mat33d& boxaxis, const vec3d& boxorg, const float* view_matrix_t, double& delta_layer) {

		const vec3d camera_pos = vec3d(view_matrix_t[0], view_matrix_t[1], view_matrix_t[2]) * (double)(-view_matrix_t[3])
			+ vec3d(view_matrix_t[4], view_matrix_t[5], view_matrix_t[6]) * (double)(-view_matrix_t[7])
			+ vec3d(view_matrix_t[8], view_matrix_t[9], view_matrix_t[10]) * (double)(-view_matrix_t[11]);
		
		const vec3d camera_direction(view_matrix_t[8], view_matrix_t[9], view_matrix_t[10]);
		
		const mat33d inv_M_abc(Inverse(mat33d( boxaxis.a.x, boxaxis.b.x, boxaxis.c.x, boxaxis.a.y, boxaxis.b.y, boxaxis.c.y, boxaxis.a.z, boxaxis.b.z, boxaxis.c.z )));
		
		vec3d box_vertex[8];
		vec3f uvw[8];
		vec3d ray[8];
		double nearest_screen_normal = DBL_MAX;
		double far_screen_normal = 0.0;
		for (uint32_t i = 0; i < 8; ++i) {
			vec3d v = boxorg
				+ ((i & 0x1) ? boxaxis.a : vec3d{ 0.0 ,0.0 ,0.0 })
				+ ((i & 0x2) ? boxaxis.b : vec3d{ 0.0 ,0.0 ,0.0 })
				+ ((i & 0x4) ? boxaxis.c : vec3d{ 0.0 ,0.0 ,0.0 });
			box_vertex[i] = v;

			uvw[i] = vec3f{ (i & 0x1) ? 1.0f : 0.0f, (i & 0x2) ? 1.0f : 0.0f, (i & 0x4) ? 1.0f : 0.0f };

			const vec3d v_from_com = v - camera_pos;
			const double dz_com = v_from_com * camera_direction;
			if (nearest_screen_normal > dz_com) {
				nearest_screen_normal = dz_com;
			}
			if (far_screen_normal < dz_com) {
				far_screen_normal = dz_com;
			}
			//ray[i] = v - camera_pos;
			//ray[i].Normalize();
			//uvw空間に変換: デカルト座標のray = Matrix(a,b,c) * uwv座標のray//
			//ray[i] = delta_layer * (inv_M_abc * Unit(v_from_com));		
			//ray[i] = delta_layer * Unit(camera_direction);
			//ray[i] = (inv_M_abc * Unit(v_from_com));
			ray[i] = Unit(v_from_com);

		}

		/*視線方向の距離がカメラに最も近い点をスクリーンにする.
		それ以外の点をスクリーン上に射影する.
		そうすることでレイキャスティングの開始点が面によって異なる問題を防ぐ*/
		if (nearest_screen_normal < VIEW_NEAR) {
			nearest_screen_normal = VIEW_NEAR;
		}

		const int num_marching = (int)ceil((far_screen_normal - nearest_screen_normal) / delta_layer);


		for (uint32_t i = 0; i < 8; ++i) {
			vec3d v = box_vertex[i];
			const vec3d v_from_com = v - camera_pos;
			const double dz_com = v_from_com * camera_direction;
			if (dz_com == 0.0) continue;
			box_vertex[i] = v_from_com * (nearest_screen_normal / dz_com) + camera_pos;
			
			//点の移動に合わせてuvwの座標も引き戻す//
			uvw[i] += (vec3f)(inv_M_abc * (box_vertex[i] - v));
			
			//const double dz_u = Unit(v_from_com) * camera_direction;
			//const double factor = delta_layer / dz_u;
			//const double factor = 1.0/ dz_u;
			//ray[i] *= factor;
		}
		
		

		mAddV(box_vertex[0], ray[0], uvw[0]);
		mAddV(box_vertex[1], ray[1], uvw[1]);
		mAddV(box_vertex[2], ray[2], uvw[2]);
		mAddV(box_vertex[3], ray[3], uvw[3]);
		mAddV(box_vertex[4], ray[4], uvw[4]);
		mAddV(box_vertex[5], ray[5], uvw[5]);
		mAddV(box_vertex[6], ray[6], uvw[6]);
		mAddV(box_vertex[7], ray[7], uvw[7]);
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

		m_vbo_indexes.push_back(0);
		m_vbo_indexes.push_back(4);
		m_vbo_indexes.push_back(1);
		m_vbo_indexes.push_back(4);
		m_vbo_indexes.push_back(5);
		m_vbo_indexes.push_back(1);
		m_vbo_indexes.push_back(2);
		m_vbo_indexes.push_back(3);
		m_vbo_indexes.push_back(6);
		m_vbo_indexes.push_back(3);
		m_vbo_indexes.push_back(7);
		m_vbo_indexes.push_back(6);

		m_vbo_indexes.push_back(0);
		m_vbo_indexes.push_back(2);
		m_vbo_indexes.push_back(4);
		m_vbo_indexes.push_back(2);
		m_vbo_indexes.push_back(6);
		m_vbo_indexes.push_back(4);

		m_vbo_indexes.push_back(1);
		m_vbo_indexes.push_back(5);
		m_vbo_indexes.push_back(3);
		m_vbo_indexes.push_back(5);
		m_vbo_indexes.push_back(7);
		m_vbo_indexes.push_back(3);

		return num_marching;
	}

	//0-5 is box boundary and 6-8 is optional cross section//
	CrossSection cross_section[9] = { 0.0f };

	int mSetBoxBoundary(const mat33d& boxaxis, const vec3d& boxorg, CrossSection* cross_section) {
		vec4f point;
		point.Set(boxorg.x, boxorg.y, boxorg.z, 0.0f);
		//point.Set(0.0f, 0.0f, 0.0f, 0.0f);
		cross_section[0].point = point;
		cross_section[0].normal.Set(-boxaxis.a.x, -boxaxis.a.y, -boxaxis.a.z,.0f);
		cross_section[1].point = point;
		cross_section[1].normal.Set(-boxaxis.b.x, -boxaxis.b.y, -boxaxis.b.z,0.0f);
		cross_section[2].point = point;
		cross_section[2].normal.Set(-boxaxis.c.x, -boxaxis.c.y, -boxaxis.c.z,0.0f);

		const vec3d v2(boxaxis.a + boxaxis.b + boxaxis.c + boxorg);//
		point.Set(v2.x, v2.y, v2.z,0.f);
		cross_section[3].point = point;
		cross_section[3].normal.Set(boxaxis.a.x, boxaxis.a.y, boxaxis.a.z, 0.0f);
		cross_section[4].point = point;
		cross_section[4].normal.Set(boxaxis.b.x, boxaxis.b.y, boxaxis.b.z, 0.0f);
		cross_section[5].point = point;
		cross_section[5].normal.Set(boxaxis.c.x, boxaxis.c.y, boxaxis.c.z, 0.0f);
		/*
		//user definied cross section
		const vec3d v3((boxaxis.a + boxaxis.b + boxaxis.c)*0.5 + boxorg);//
		cross_section[6].point.Set(v3.x, v3.y, v3.z, 0.0f);
		cross_section[6].normal.Set(1.0f, -1.0f, 0.0f, 0.0f);
		*/
		return 6;
	}

public:

	RayCastingDX(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		renderer(pD3DDevice, pD3DDeviceContext, comShader),
		m_state_manager(pD3DDevice, pD3DDeviceContext)
	{
		InitShader();

		for (int i = 0; i < 9; ++i) {
			cross_section[i].point.Clear();
			cross_section[i].normal.Clear();
		}
		
	}
	virtual ~RayCastingDX() {
		m_pTextureRV->Release();
		m_pTextureRV_cm->Release();
		m_pTexture3D->Release();
		m_pTexture_cm->Release();
		m_pSampler->Release();
		m_pSampler_cm->Release();
		m_pCB_num_marching->Release();
		m_pCB_cross_section->Release();

	}

	void SetViewAndProjectionMatrix(const float* pViewMatrix, const float* pProjectionMatrix) {
		renderer.SetViewAndProjectionMatrix(pViewMatrix, pProjectionMatrix);
	}

	//user definied cross section//
	void SetCrossSection(int num_cross_sections, const CrossSection* user_sections, const bool* is_effective, const int* mode_and_or) {
		const int i_end = max(num_cross_sections, 3);
		for (int i = 0; i < i_end; i++) {
			if (is_effective[i]) {
				cross_section[i + 6] = user_sections[i];
				cross_section[i + 6].normal.w = (mode_and_or[i] > 0) ? 1.0f : 0.0f;
			}
			else {
				cross_section[i + 6].point.Set(0.0f, 0.0f, 0.0f, 0.0f);
				cross_section[i + 6].normal.Set(0.0f, 0.0f, 0.0f, 0.0f);
			}
		}
		for (int i = i_end; i < 3; i++) {
			cross_section[i + 6].point.Set(0.0f, 0.0f, 0.0f, 0.0f);
			cross_section[i + 6].normal.Set(0.0f, 0.0f, 0.0f, 0.0f);
		}
	}

	struct ColorRange {
		float range_min = 1.0f - 6;
		float range_max = 1.0f;
		float alpha_min = 1.0f - 6;
		float alpha_max = 1.0f;
	};

	void DrawField(const FIELD3D_FRAME* field, ColorRange range, const float* view_matrix_t){

		m_vbo_vertex.clear();
		m_vbo_indexes.clear();

		const mat33d& boxaxis = field->boxaxis;

		//double delta_layer = 0.2*std::min<double>(std::min<double>((field->boxaxis.a.x / field->grid_x), (field->boxaxis.b.y / field->grid_y)), (field->boxaxis.c.z / field->grid_z));
		double delta_layer = 0.5 * std::min<double>(std::min<double>((field->boxaxis.a.x / field->grid_x), (field->boxaxis.b.y / field->grid_y)), (field->boxaxis.c.z / field->grid_z));
		int num_marching = CreateRaySurface(field->boxaxis, field->boxorg, view_matrix_t, delta_layer);
		
		int num_cross_section = mSetBoxBoundary(field->boxaxis, field->boxorg, cross_section);

		ID3D11ShaderResourceView* pTextureRV = mSetTexture3D(field->fieldb, field->grid_x, field->grid_y, field->grid_z);

		//color map//
		
		const int COLOR_MAP_SPECULAR_SIZE = 10;
		const uint8_t color_map_specular[COLOR_MAP_SPECULAR_SIZE * 4] = {
			38, 0, 106, 255,
			76, 0, 211, 255,
			0, 0, 230, 255,
			0, 140, 120, 255,
			0, 180, 60, 255,
			0, 250, 0, 255,
			60, 180, 0, 255,
			120, 120, 0, 255,
			180, 60, 0, 255,
			255, 0, 0, 255
		};

		const uint8_t color_map_JET_SIZE = 9;
		const uint8_t color_map_JET[color_map_JET_SIZE * 4] = {
							0x00, 0x00, 0x90, 0xFF,
							0x00, 0x0f, 0xff, 0xFF,
							0x00, 0x90, 0xff, 0xFF,
							0x0f, 0xff, 0xee, 0xFF,
							0x90, 0xff, 0x70, 0xFF,
							0xff, 0xee, 0x00, 0xFF,
							0xff, 0x70, 0x00, 0xFF,
							0xee, 0x00, 0x00, 0xFF,
							0x7f, 0x00, 0x00, 0xFF, };

			
		//ID3D11ShaderResourceView* pTextureRV_cm = mSetTextureColorMap(color_map_specular, COLOR_MAP_SPECULAR_SIZE);
		ID3D11ShaderResourceView* pTextureRV_cm = mSetTextureColorMap(color_map_JET, color_map_JET_SIZE);

		if (m_vbo_indexes.empty()) return;

		m_state_manager.StoreState();


		//バッファのセット.	
		UINT offset = 0;
		//m_pD3DDeviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &m_vertex_stride, &offset);
		renderer.UpdateVertexBuffer(&m_vbo_vertex[0], m_vbo_vertex.size(), sizeof(VERTEX_F3F3F3));	//頂点データをVRAMに転送
		renderer.UpdateIndexBuffer(&m_vbo_indexes[0], m_vbo_indexes.size(), sizeof(uint32_t));	//インデックスデータをVRAMに転送

		renderer.m_pD3DDeviceContext->PSSetSamplers(0, 1, &m_pSampler);
		renderer.m_pD3DDeviceContext->PSSetSamplers(1, 1, &m_pSampler_cm);

		renderer.m_pD3DDeviceContext->PSSetShaderResources(0, 1, &pTextureRV);
		renderer.m_pD3DDeviceContext->PSSetShaderResources(1, 1, &pTextureRV_cm);
		
		//set the tranformed uvw unit vectors.
		renderer.m_pD3DDeviceContext->PSSetConstantBuffers(0, 1, &m_pCB_num_marching);
		{
			//convert from max to (1/log(max/min))
			range.range_max = (float)(log(range.range_max / range.range_min));
			range.alpha_max = (float)(log(range.alpha_max / range.alpha_min));

			//定数バッファの書き込み.
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			renderer.m_pD3DDeviceContext->Map(m_pCB_num_marching, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			CopyMemory(mappedResource.pData, &num_marching, sizeof(int));
			CopyMemory((uint8_t*)(mappedResource.pData) + 4, &range, sizeof(float) * 4);
			const mat33d inv_M_abc(Inverse(mat33d(boxaxis)));
			//mat33f inv_M_abcf(inv_M_abc.m11, inv_M_abc.m12, inv_M_abc.m13, inv_M_abc.m21, inv_M_abc.m22, inv_M_abc.m23, inv_M_abc.m31, inv_M_abc.m32, inv_M_abc.m33);
			mat33f inv_M_abcf(inv_M_abc.m11, inv_M_abc.m12, inv_M_abc.m13, inv_M_abc.m21, inv_M_abc.m22, inv_M_abc.m23, inv_M_abc.m31, inv_M_abc.m32, inv_M_abc.m33);
			//inv_M_abcf *= delta_layer;
			const float delta_ray_f = (float)delta_layer;
			//float f3[3]; f3[0] = delta_layer * inv_M_abc.m11; f3[1] = delta_layer * inv_M_abc.m22; f3[2] = delta_layer * inv_M_abc.m33;
			//float f3[3]; f3[0] = 1.0f; f3[1] = 1.0f; f3[2] = 1.0f;
			CopyMemory((uint8_t*)(mappedResource.pData) + 4 * 5, &delta_ray_f, sizeof(float));
			CopyMemory((uint8_t*)(mappedResource.pData) + 4 * 6, &inv_M_abcf, sizeof(float) * 9);
			renderer.m_pD3DDeviceContext->Unmap(m_pCB_num_marching, 0);
		}
		
		//set the box boundary and cross section
		renderer.m_pD3DDeviceContext->PSSetConstantBuffers(1, 1, &m_pCB_cross_section);
		{
			//定数バッファの書き込み.
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			renderer.m_pD3DDeviceContext->Map(m_pCB_cross_section, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
			CopyMemory(mappedResource.pData, &(cross_section[0]), sizeof(float)* 8*9 );
			renderer.m_pD3DDeviceContext->Unmap(m_pCB_cross_section, 0);
		}
		

		renderer.DrawIndexed(m_vbo_indexes.size(), D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);


		m_state_manager.RestoreState();

	}


};



