//=====================================
// 複数インスタンス表示クラス
//
//
// 
//=====================================
#pragma once

#ifndef KrbRendererDX11_h
#define KrbRendererDX11_h

#include <d3d11.h>
#include <unordered_map>



#include "MDLoader.h"	//ATOMS_DATAのためだけに必要//
#include "BOND_INFO.h"

//#include "sdkwgl.h"
#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "io64.h"
#include "mod.h"


#include "visual.h"
#include "krb_reader.h"


#include "BasicRenderer.h"
#include "VertexTypes.h"
#include "GeometrySphere.h"
#include "GeometryCylinder.h"





extern size_t g_max_vram_size;


class KrbRendererDX11{
private:

	RendererWrapper renderer;


	int m_vram_size = 0;
	int m_vbo_vcount = 0;
	VERTEX_F3F3UI1* m_vbo_vertex = nullptr;
	int m_vbo_icount = 0;
	int* m_vbo_indexes = nullptr;
	unsigned int m_index_type;

	//unsigned int *m_color_table;	//原子ごとの色が入る

    KRB_INFO m_prev_krb_info;	//前回レンダリングしたデータ

	
//ポリゴンモデル用のバッファ
	
	GeometrySphere* m_geometry_sphere = nullptr;
	GeometryCylinder* m_geometry_bond = nullptr;
	


	struct DRAWN_PARTICLE {
		vec3f position;
		unsigned int color_table;
	};
	std::unordered_map<size_t, DRAWN_PARTICLE> m_particles;

	VISUAL_SETTING m_prev_vis;

	int m_visual_bond = VISUAL_BOND_PIPE;	//ボンドの表示種類
	


public:


	KrbRendererDX11::KrbRendererDX11(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		renderer(pD3DDevice, pD3DDeviceContext, comShader)
	{
		InitShader();
	}


	virtual ~KrbRendererDX11() {

		delete[] m_vbo_vertex;
		delete[] m_vbo_indexes;

		delete m_geometry_sphere;
		delete m_geometry_bond;

	}





	//VBOの設定
	template<class FUNC>
	void DrawFunc(KRB_INFO* dat, uint32_t trajectory_color, const VISUAL_SETTING& vis, FUNC func) {	//int j_mode, float Jmax


		if (dat == NULL) return;


		//vboのセット
		int redraw_flag = 0;
		if ((vis.atom_poly != m_prev_vis.atom_poly) || (vis.atom_radius != m_prev_vis.atom_radius)) {
			//ポリゴンモデルの初期化
			if (m_geometry_sphere) delete m_geometry_sphere;
			m_geometry_sphere = new GeometrySphere(vis.atom_radius, vis.atom_poly, vis.atom_poly * 2);

			redraw_flag = 1;
		}

		//原子表示色のモード設定//
		//m_visual_atomcolor = vis.atom_color;
		if (vis.atom_color != m_prev_vis.atom_color) {
			redraw_flag = 1;
		}

		if ((vis.bond_poly != m_prev_vis.bond_poly) || (vis.bond != m_prev_vis.bond)) {
			//ポリゴンモデルの初期化

			if (m_geometry_bond) delete m_geometry_bond;
			m_geometry_bond = new GeometryCylinder(vis.trajectory_width, vis.bond_poly);

			redraw_flag = 1;
		}

		if (dat->particles != m_prev_krb_info.particles) {
			redraw_flag = 1;
		}
		if (m_prev_krb_info.num_step != dat->num_step) {
			redraw_flag = 1;
		}
		redraw_flag = 1;


		//カラーテーブル作り

		if (redraw_flag) {
			m_particles.clear();
		}

		//GPUバッファの事前確保//
		const unsigned int one_sphere_sz = ((m_geometry_sphere->GetNumVertexes()) * sizeof(VERTEX_F3F3UI1) + (m_geometry_sphere->GetNumIndexes()) * sizeof(int));
		const unsigned int one_bond_sz = ((m_geometry_bond->GetNumVertexes()) * sizeof(VERTEX_F3F3UI1) + (m_geometry_bond->GetNumIndexes()) * sizeof(int));
		const unsigned int vram_size_vertex = (unsigned int)((double)g_max_vram_size * ((double)((m_geometry_sphere->GetNumVertexes()) * sizeof(VERTEX_F3F3UI1)) / (double)one_sphere_sz));
		const unsigned int vram_size_index = g_max_vram_size - vram_size_vertex;
		const int vbo_v_count = vram_size_vertex / sizeof(VERTEX_F3F3UI1);
		const int vbo_i_count = vram_size_index / sizeof(int);
		{


			if (m_vbo_vertex == NULL) {
				m_vbo_vertex = new VERTEX_F3F3UI1[vbo_v_count];
				m_vbo_indexes = new int[vbo_i_count];
			}

			renderer.UpdateVertexBuffer(NULL, vbo_v_count, sizeof(VERTEX_F3F3UI1));	//頂点データのバッファだけ確保
			renderer.UpdateIndexBuffer(NULL, vbo_i_count, sizeof(int));	//インデックスデータのバッファだけ確保
		}


		//軌跡の描画//
		{
			const int i_end = dat->num_step;



			int i_start = 0;
			while (i_start < i_end) {


				int next_start = CreateBond(i_start, i_end, vbo_v_count, vbo_i_count, dat, trajectory_color, vis.history_frame, func);


				if (m_vbo_icount) {

					renderer.UpdateVertexBuffer(m_vbo_vertex, m_vbo_vcount, sizeof(VERTEX_F3F3UI1));	//頂点データをVRAMに転送
					renderer.UpdateIndexBuffer(m_vbo_indexes, m_vbo_icount, sizeof(int));	//インデックスデータをVRAMに転送


					renderer.DrawIndexed(m_vbo_icount, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				}

				i_start = next_start;
			}

		}



		//粒子の描画//
		if (vis.atom) {


			bool next_remained = true;
			while (next_remained) {


				next_remained = CreateSphere(vbo_v_count, vbo_i_count, func);



				if (m_vbo_icount) {

					renderer.UpdateVertexBuffer(m_vbo_vertex, m_vbo_vcount, sizeof(VERTEX_F3F3UI1));	//頂点データをVRAMに転送
					renderer.UpdateIndexBuffer(m_vbo_indexes, m_vbo_icount, sizeof(int));	//インデックスデータをVRAMに転送

					renderer.DrawIndexed(m_vbo_icount, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				}


			}
		}

		m_prev_krb_info = *dat;
		m_prev_vis = vis;
	}


	//VBOの設定
	void Draw(KRB_INFO* dat, uint32_t trajectory_color, const VISUAL_SETTING& vis) {	//int j_mode, float Jmax
		DrawFunc(dat, trajectory_color, vis, [](vec3f r, uint32_t color) {return true; });
	}

	void SetViewAndProjectionMatrix(const float* pViewMatrix, const float* pProjectionMatrix) {
		renderer.SetViewAndProjectionMatrix(pViewMatrix, pProjectionMatrix);
	}

private:
	

	void InitShader() {

		//Vertex Shader--------------------------------------

		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 24, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);
		TCHAR filepath[MAX_PATH];
		GetAttachedFileName(filepath, _T("vs_color.cso"));
		renderer.LoadVertexShader(filepath, "main", "vs_4_0", layout, numElements);

		//Pixel Shader-----------------------------------------
		GetAttachedFileName(filepath, _T("ps_color.cso"));
		renderer.LoadPixelShader(filepath, "main", "ps_4_0");

	}



	template<class FUNC>
	int CreateSphere(int limit_v_count, int limit_i_count, FUNC func) {

		int num_v_count = m_geometry_sphere->GetNumVertexes();
		int num_i_count = m_geometry_sphere->GetNumIndexes();

		m_vbo_vcount = 0;
		m_vbo_icount = 0;

		//原子(球)のセット//

		for (auto it = m_particles.begin(); it != m_particles.end(); ++it) {
			if ((it->second.color_table & 0xFF000000)   //完全透過は表示しない//
				&& func(it->second.position, it->second.color_table)){

				if (m_vbo_vcount + num_v_count > limit_v_count) {
					return true;
				}
				if (m_vbo_icount + num_i_count > limit_i_count) {
					return true;
				}

				m_geometry_sphere->SetSphere(it->second.position, it->second.color_table,
					m_vbo_vcount,	//頂点番号のoffset
					m_vbo_vertex + m_vbo_vcount,
					m_vbo_indexes + m_vbo_icount);

				m_vbo_vcount += num_v_count;
				m_vbo_icount += num_i_count;
			}
		}


		return false;

	}

	template<class FUNC>
	int CreateBond(int i_begin, int i_end, int limit_v_count, int limit_i_count, KRB_INFO* dat, uint32_t trajectory_color, int history_frame, FUNC func) {


		const int geo_v_count = m_geometry_bond->GetNumVertexes();
		const int geo_i_count = m_geometry_bond->GetNumIndexes();

		m_vbo_vcount = 0;
		m_vbo_icount = 0;


		float half_x = dat->box_axis.m11 * 0.5f;
		float half_y = dat->box_axis.m22 * 0.5f;
		float half_z = dat->box_axis.m33 * 0.5f;


		//ボンドのセット
		const int lower_visual_frame = (history_frame > 0 ? dat->num_step - history_frame : 0);


		const KRB_PARTICLE* particle = dat->particles;
		for (int i = i_begin; i < i_end; i++) {

			size_t id = particle[i].id;
			if (particle[i].simulation_mode == KRB_MODE_DEL) {
				m_particles.erase(id);

			} else {

				DRAWN_PARTICLE p;
				p.position.Set(particle[i].x, particle[i].y, particle[i].z);
				p.color_table = *(((unsigned int*)atom_colors) + particle[i].atomic_element);
				//p.color_table = trajectory_color;

				//最初の登場では色を決めて、開始点をバッファに積む//
				auto res = m_particles.insert(std::pair<size_t, DRAWN_PARTICLE>(id, p));
				if (!res.second) {

					//二回目以降なので軌道を描画//				
					vec3f next_pos;
					next_pos.Set(particle[i].x, particle[i].y, particle[i].z);
					if ((i >= lower_visual_frame) && func(next_pos, 0xFFFFFFFF)){

						vec3f dv = res.first->second.position - next_pos;
						if ((fabs(dv.x) < half_x) && (fabs(dv.y) < half_y) && (fabs(dv.z) < half_z)) {


							if (m_vbo_vcount + geo_v_count > limit_v_count) {
								return i;
							}
							if (m_vbo_icount + geo_i_count > limit_i_count) {
								return i;
							}

							m_geometry_bond->SetBond(next_pos, dv,
								//res.first->second.color_table, res.first->second.color_table, m_vbo_vcount,
								trajectory_color, trajectory_color, m_vbo_vcount,
								m_vbo_vertex + m_vbo_vcount,
								m_vbo_indexes + m_vbo_icount);

							m_vbo_vcount += geo_v_count;
							m_vbo_icount += geo_i_count;
						}
					}

					//位置の更新//
					res.first->second.position = next_pos;
				}
			}
		}

		return i_end;

	}




};


#endif    // __AtomRendererDX11_h__


