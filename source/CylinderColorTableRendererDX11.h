#pragma once



#include <d3d11.h>
#include <vector>


#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>
#include <algorithm>

#include "atomic_color.h"

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



class CylinderColorTableRendererDX11 {
private:

	RendererWrapper renderer;


	int m_vram_size = 0;
	int m_vbo_vcount = 0;
	VERTEX_F3F3UI1* m_vbo_vertex = nullptr;
	int m_vbo_icount = 0;
	int* m_vbo_indexes = nullptr;
	unsigned int m_index_type;

	//ポリゴンモデル用のバッファ
	GeometryCylinder* m_geometry_bond = nullptr;
	VISUAL_SETTING m_prev_vis;

	int m_visual_bond = VISUAL_BOND_PIPE;	//ボンドの表示種類


public:
	CylinderColorTableRendererDX11(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		renderer(pD3DDevice, pD3DDeviceContext, comShader)
	{
		InitShader();
	}

	virtual ~CylinderColorTableRendererDX11() {
		delete[] m_vbo_vertex;
		delete[] m_vbo_indexes;
		delete m_geometry_bond;
	}


	//VBOの設定//
	void Draw(int num_pairs, const VECTOR_IJ* bonds, const vec3f* r, const int* color_indexes, const uint32_t* color_table, const VISUAL_SETTING& vis) {

		if (num_pairs == 0) return;
		if (bonds == nullptr) return;

		if ((vis.bond_poly != m_prev_vis.bond_poly) || (vis.bond != m_prev_vis.bond)) {
			//ポリゴンモデルの初期化

			if (m_geometry_bond) delete m_geometry_bond;
			m_geometry_bond = new GeometryCylinder(vis.atom_radius * 0.25, vis.bond_poly);

		}

		//ボンドの描画//

		size_t num_draw_once = CalcNumDrawOnce(m_geometry_bond, g_max_vram_size);


		size_t i_start = 0;
		while (i_start < num_pairs) {

			if (i_start + num_draw_once > num_pairs) { num_draw_once = num_pairs - i_start; };


			CreateBond(i_start, num_draw_once, bonds, r, color_indexes, color_table);


			if (m_vbo_icount) {

				renderer.UpdateVertexBuffer(m_vbo_vertex, m_vbo_vcount, sizeof(VERTEX_F3F3UI1));	//頂点データをVRAMに転送
				renderer.UpdateIndexBuffer(m_vbo_indexes, m_vbo_icount, sizeof(int));	//インデックスデータをVRAMに転送


				renderer.DrawIndexed(m_vbo_icount, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
			}

			i_start += num_draw_once;
		}



		m_prev_vis = vis;
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
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },

		};
		UINT numElements = ARRAYSIZE(layout);
		GetAttachedFileName(filepath, _T("vs_color.cso"));
		renderer.LoadVertexShader(filepath, "main", "vs_4_0", layout, numElements);


		//Pixel Shader-----------------------------------------
		GetAttachedFileName(filepath, _T("ps_color.cso"));
		renderer.LoadPixelShader(filepath, "main", "ps_4_0");
	}



	void CreateBond(int start, int count, const VECTOR_IJ* bonds, const vec3f* r, const int* color_indexes, const uint32_t* color_table) {

		const int geo_v_count = m_geometry_bond->GetNumVertexes();
		const int geo_i_count = m_geometry_bond->GetNumIndexes();

		//vboサイズ
		int vbo_v_count = count * geo_v_count;
		int vbo_i_count = count * geo_i_count;
		int need_vram_sz = vbo_v_count * sizeof(VERTEX_F3F3UI1) + vbo_i_count * sizeof(int);


		if (m_vram_size < need_vram_sz) {
			m_vram_size = need_vram_sz;

			delete[] m_vbo_vertex;
			delete[] m_vbo_indexes;
			m_vbo_vertex = new VERTEX_F3F3UI1[vbo_v_count];
			m_vbo_indexes = new int[vbo_i_count];
		}



		m_vbo_vcount = 0;
		m_vbo_icount = 0;


		//ボンドのセット
		//const VECTOR_IJ* b = bond->bvector;
		const int i_end = start + count;
		for (int i = start; i < i_end; ++i) {

			uint32_t color_i = color_table[color_indexes[bonds[i].i]];
			uint32_t color_j = color_table[color_indexes[bonds[i].j]];
			if ((color_i & color_j) & 0xFF000000) {	//完全透過は表示しない//

				m_geometry_bond->SetBond(r[bonds[i].i], bonds[i].v,
					color_i,
					color_j,
					m_vbo_vcount,
					m_vbo_vertex + m_vbo_vcount,
					m_vbo_indexes + m_vbo_icount);

				m_vbo_vcount += geo_v_count;
				m_vbo_icount += geo_i_count;
			}
		}


	}
};


