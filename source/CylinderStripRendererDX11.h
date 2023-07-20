#pragma once
#include "SphereRendererDX11.h"
#include "GeometryCylinder.h"

extern size_t g_max_vram_size;

template <typename GEO>
class StripMaker {
public:

	StripMaker(GEO& geometry) :
		m_geometry(geometry),
		is_begin_point(true),
		add_numbber(0)
	{

	}


	void Push(const vec3f& point, const uint32_t color) {

		if (add_numbber == 0) {
			add_numbber++;
		}else if (add_numbber == 1) {
			m_geometry.PushFirst(m_point_org, point,
				m_color_org,
				m_vertexes,
				m_indexes);
			add_numbber++;
		} else {
			m_geometry.PushSecond(m_point_org, point,
				m_color_org,
				m_vertexes,
				m_indexes);
		}

		m_point_org = point;
		m_color_org = color;

	}

	//区切る//
	void Split() {
		if (add_numbber > 1) {
			m_geometry.PushLast(m_point_org,
				m_color_org,
				m_vertexes,
				m_indexes);
		}
		add_numbber = 0;
	}


	void GetSize(size_t* v_size, size_t* i_size) {
		*v_size = m_vertexes.size();
		*i_size = m_indexes.size();
	}


	VERTEX_F3F3UI1* GetVertexBuffer() {
		return &(m_vertexes[0]);
	}

	int* GetIndexBuffer() {
		return &(m_indexes[0]);
	}

	void Clear() {
		is_begin_point = true;
		m_vertexes.clear();
		m_indexes.clear();
		add_numbber = 0;
	}

private:
	GEO& m_geometry;
	vec3f m_point_org;
	uint32_t m_color_org;

	std::vector<VERTEX_F3F3UI1> m_vertexes;
	std::vector<int> m_indexes;

	bool is_begin_point;
	int add_numbber;

};


class CylinderStripRendererDX11  {
private:

	RendererWrapper renderer;

	int m_vram_size = 0;
	unsigned int m_index_type;

	//ポリゴンモデル用のバッファ
	GeometryCylinder* m_geometry_bond = nullptr;
	StripMaker<GeometryCylinder>* m_maker = nullptr;

	VISUAL_SETTING m_prev_vis;

	int m_visual_bond= VISUAL_BOND_PIPE;	//ボンドの表示種類

public:


	CylinderStripRendererDX11(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		renderer(pD3DDevice, pD3DDeviceContext, comShader)		
	{
		InitShader();
	}


	virtual ~CylinderStripRendererDX11() {
		delete m_geometry_bond;
		delete m_maker;
	}



	void Draw(const vec3f* r, const int* strip_indexes, const int* strip_lengths, const int num_strips, const uint32_t color, const VISUAL_SETTING& vis) {


		if (num_strips == 0) return;
		if (r == NULL) return;


		if ((vis.bond_poly != m_prev_vis.bond_poly) || (vis.trajectory_width != m_prev_vis.trajectory_width)) {
			//ポリゴンモデルの初期化

			if (m_geometry_bond) delete m_geometry_bond;
			m_geometry_bond = new GeometryCylinder(vis.trajectory_width, vis.bond_poly);

			if (m_maker) {
				delete m_maker;
			}
			m_maker = new StripMaker<GeometryCylinder>(*m_geometry_bond);
		}

		//バッファの事前確保//

		//頂点数の計算//
		size_t num_draw_once = CalcNumDrawOnce(m_geometry_bond, g_max_vram_size);
		size_t v_size_limit = num_draw_once * m_geometry_bond->GetNumVertexes();
		size_t i_size_limit = num_draw_once * m_geometry_bond->GetNumIndexes();
		renderer.UpdateVertexBuffer(NULL, v_size_limit, sizeof(VERTEX_F3F3UI1));	//頂点データをVRAMに転送
		renderer.UpdateIndexBuffer(NULL, i_size_limit, sizeof(int));	//インデックスデータをVRAMに転送



		StripMaker<GeometryCylinder>& maker = *m_maker;
		maker.Clear();

		int k = 0;
		for (int i = 0; i < num_strips; ++i) {
			for (int j = 0; j < strip_lengths[i]; ++j) {
				int target = strip_indexes[k];
				maker.Push(r[target], color);
				k++;
			}
			maker.Split();

			size_t v_size, i_size;
			maker.GetSize(&v_size, &i_size);
			if ((v_size >= v_size_limit) || (i_size >= i_size_limit) ||
				((i == num_strips - 1) && (v_size > 0) && (i_size > 0))) {

				renderer.UpdateVertexBuffer(maker.GetVertexBuffer(), v_size, sizeof(VERTEX_F3F3UI1));	//頂点データをVRAMに転送
				renderer.UpdateIndexBuffer(maker.GetIndexBuffer(), i_size, sizeof(int));	//インデックスデータをVRAMに転送

				renderer.DrawIndexed(i_size, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
				maker.Clear();
			}

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



	



};


