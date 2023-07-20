//=====================================
// 複数インスタンス表示クラス
//
//
// 
//=====================================
#pragma once



#include <d3d11.h>
#include <vector>

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <algorithm>

#include "atomic_color.h"


//#pragma warning(disable : 4996)

extern size_t g_max_vram_size;

#include "trajectory2.h"	//ATOMS_DATAのためだけに必要//

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
#include "GeometrySphere.h"
#include "GeometryCylinder.h"

#include "aidx_constantbuffer.h"


class SphereColorTableRendererDX11 {
private:
	
	RendererWrapper renderer;
		
	int m_vram_size=0;
	int m_vbo_vcount=0;
	VERTEX_F3F3* m_vbo_vertex=nullptr;
	int m_vbo_icount=0;
	int* m_vbo_indexes = nullptr;
	unsigned int m_index_type;
	std::vector<VERTEX_F4> m_instances;
	
	//ポリゴンモデル用のバッファ
	GeometrySphere* m_geometry_sphere = nullptr;

	VISUAL_SETTING m_prev_vis;
		
	AIDX11::ConstantBuffer m_CB_color_table;
	AIDX11::ConstantBuffer m_CB_range;

public:

	SphereColorTableRendererDX11(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		renderer(pD3DDevice, pD3DDeviceContext, comShader),
		m_CB_color_table(pD3DDevice, pD3DDeviceContext, 1),
		m_CB_range(pD3DDevice, pD3DDeviceContext, 2)
	{
		InitShader();
	}


	~SphereColorTableRendererDX11() {
		delete[] m_vbo_vertex;
		delete[] m_vbo_indexes;
		delete m_geometry_sphere;
	}


	//Instanceを描画//
	template <class FUNC>
	void DrawFunc(int num_particles, const vec3f* r, const float* color_table, const int num_colors, float height_min, float height_max, const VISUAL_SETTING& vis, FUNC func) {

		if (num_particles <= 0) return;
		if (r == NULL) return;

		//vboのセット
		if ((vis.atom_poly != m_prev_vis.atom_poly) || (vis.atom_radius != m_prev_vis.atom_radius)) {
			//ポリゴンモデルの初期化

			if (m_geometry_sphere) delete m_geometry_sphere;
			m_geometry_sphere = new GeometrySphere(vis.atom_radius, vis.atom_poly, vis.atom_poly * 2);
			CreateSphere();
		}


		//原子球の描画//
		if (vis.atom) {

			renderer.UpdateVertexBuffer(m_vbo_vertex, m_vbo_vcount, sizeof(VERTEX_F3F3));	//頂点データをVRAMに転送
			renderer.UpdateIndexBuffer(m_vbo_indexes, m_vbo_icount, sizeof(int));	//インデックスデータをVRAMに転送

			m_CB_color_table.Update(color_table, num_colors, sizeof(float) * 4);

			m_CB_range.Update(&(VERTEX_F4(height_min, height_max - height_min, (float)(num_colors - 1), 0.0)), 1, sizeof(float) * 4);
			//m_CB_range.Update(&(VERTEX_F4(1.0, 0.0, 1.0, 1.0)), 1, sizeof(float) * 4);

			//一度に書く頂点数の計算//
			size_t num_draw_once = std::min<size_t>((g_max_vram_size / (sizeof(VERTEX_F3UI1))), num_particles);

			size_t i_start = 0;
			while (i_start < num_particles) {

				if (i_start + num_draw_once > num_particles) { num_draw_once = num_particles - i_start; };

				//原子の位置と色のセット//
				m_instances.clear();
				const int i_end = i_start + num_draw_once;
				for (int i = i_start; i < i_end; ++i) {
					if (func(r[i], 0xFFFFFFFF) ){
						m_instances.push_back(VERTEX_F4(r[i], r[i].z));
					}
				}
				UINT valid_count = m_instances.size();

				if (!m_instances.empty()) {
					renderer.UpdateInstanceBuffer((LPVOID) & (m_instances[0]), valid_count, sizeof(VERTEX_F3UI1));
					//renderer.Draw(m_vbo_icount, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST, valid_count);
					renderer.BeginDraw(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
					m_CB_color_table.AttachVS();
					m_CB_range.AttachVS();
					renderer.EndDraw(m_vbo_icount, valid_count);

				}

				i_start += num_draw_once;
			}

		}


		m_prev_vis = vis;

	}

	//Instanceを描画//
	void Draw(int num_particles, const vec3f* r, const float* color_table, const int num_colors, float height_min, float height_max, const VISUAL_SETTING& vis) {
		DrawFunc(num_particles, r, color_table, num_colors, height_min, height_max, vis, [](vec3f r, uint32_t color) {return true; });
	}

	void SetViewAndProjectionMatrix(const float* pViewMatrix, const float* pProjectionMatrix) {
		renderer.SetViewAndProjectionMatrix(pViewMatrix, pProjectionMatrix);
	}

private:

	void InitShader() {
		TCHAR filepath[MAX_PATH];
		D3D11_INPUT_ELEMENT_DESC layout[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			//{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },		
			{ "INS_POS", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D11_INPUT_PER_INSTANCE_DATA, 1 },


		};
		UINT numElements = ARRAYSIZE(layout);
		GetAttachedFileName(filepath, _T("vs_inst_color_table.cso"));
		renderer.LoadVertexShader(filepath, "main", "vs_4_0", layout, numElements);


		//Pixel Shader-----------------------------------------
		GetAttachedFileName(filepath, _T("ps_color.cso"));
		renderer.LoadPixelShader(filepath, "main", "ps_4_0");
	}


	void CreateSphere() {

		const int vbo_v_count = m_geometry_sphere->GetNumVertexes();
		const int vbo_i_count = m_geometry_sphere->GetNumIndexes();
		//vboサイズ
		int need_vram_sz = vbo_v_count * sizeof(VERTEX_F3F3) + vbo_i_count * sizeof(int);

		if (m_vram_size < need_vram_sz) {
			m_vram_size = need_vram_sz;

			delete[] m_vbo_vertex;
			delete[] m_vbo_indexes;
			m_vbo_vertex = new VERTEX_F3F3[vbo_v_count];
			m_vbo_indexes = new int[vbo_i_count];
		}

		m_vbo_vcount = 0;
		m_vbo_icount = 0;

		m_geometry_sphere->SetSphere(vec3f(0.0, 0.0, 0.0),
			m_vbo_vcount,	//頂点番号のoffset
			m_vbo_vertex,
			m_vbo_indexes);


		m_vbo_vcount += vbo_v_count;
		m_vbo_icount += vbo_i_count;

	}

};





