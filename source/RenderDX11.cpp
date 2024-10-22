
#pragma warning(disable : 4996)

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
#define _USE_MATH_DEFINES

#include <tchar.h>
#include <string.h>
#include <math.h>

#include "RenderDX11.h"

#include "windib2.h"
#include "WriteBMP.h"

#include "mod.h"
#include "visual.h"
#include "camera.h"
#include "atomic_color.h"


#include "SphereRendererDX11.h"
#include "CylinderRendererDX11.h"
#include "SphereColorTableRendererDX11.h"
#include "CylinderColorTableRendererDX11.h"
#include "PointRendererDX11.h"
#include "PointSpriteDX11.h"
#include "VNTRenderer.h"
#include "KrbRendererDX11.h"
#include "BoxRendererDX11.h"
#include "Tex3DRendererDX11.h"
#include "CylinderStripRendererDX11.h"
#include "RayCastingDX.h"
#include "RayCastingDX_depth.h"
#include "filterRGBDepth.h"



#include "MDLoader.h"
#include "filelist.h"


#include "setting.h"


extern double VIEW_ANGLE;
extern double VIEW_NEAR;
extern double VIEW_FAR;



//extern MasterFileList g_files;		//開いたファイルのリスト.
extern bool g_updateFlag;	//レンダリングをしないようにLockする。メモリ転送など.
extern int g_filter_mode;

namespace {
	//DirectX11 system//////////////////////////////
	DX11_CORE s_dx11_core;
	CommonShader* comShader = nullptr;
	
	SphereRendererDX11* m_sphere_renderer = nullptr;
	CylinderRendererDX11* m_cylinder_renderer = nullptr;
	SphereColorTableRendererDX11* m_sphere_color_table_renderer = nullptr;
	PointRendererDX11* m_pointRendererDX11 = nullptr;
	PointSpriteDX11* m_pointSpriteDX11 = nullptr;
	VNTRenderer* m_vntRenderer = nullptr;
	Tex3DRenderer* m_tex3dRenderer = nullptr;
	KrbRendererDX11* m_karabaRendererDX11 = nullptr;
	BoxRendererDX11* m_boxRendererDX11 = nullptr;
	CylinderStripRendererDX11* m_strip_renderer = nullptr;
	RayCastingDX* m_rayCastingRenderer = nullptr;
	RayCastingDX_depth* m_rayCastingRenderer_depth = nullptr;

	constexpr const bool USE_RAYCASTING = true;

	//polygon//////////////////////////////////////////////
	vec3f* flier_vertex;
	int flier_vertex_count;
	WORD* flier_index;
	int flier_index_count;

	///////////////////////////////////////////////////////////////////
	//変換行列用ライブラリ
	///////////////////////////////////////////////////////////////////
	void SetProjectionPerspectiveMatrix(float angle, float wph, float z_near, float z_far, mat44f* pProjectionMatrix);
	void SetProjectionRange(int projection_mode, int image_w, int image_h, int range_x, int range_y, int range_w, int range_h, double distance, mat44f* projection_matrix);

	StateManagerDX11* state_manager_dx = nullptr;

	enum FILTER_MODE {
		FILTER_NONE = 0,
		FILTER_DEPTH_OF_FIELD = 1            //ぼかし//
	};
	


	FilterRGBandDepth* s_filterDoF = nullptr;
	ReadableDepth* s_readable_depth = nullptr;
	ID3D11ShaderResourceView* m_depth_view = nullptr;

	bool IsAny3(const bool* is_corss_section_effect) {
		return is_corss_section_effect[0] || is_corss_section_effect[1] || is_corss_section_effect[2];
	}


	bool IsCrossSectForAtom(const RenderingProperty& prop) {
		return IsAny3(prop.is_corss_section_effect) && (prop.corss_section_target & 0b10);
	}

}


///////////////////////////////////////////////////////////////////
//以下はユーザー定義のコールバック関数
///////////////////////////////////////////////////////////////////

void InitDX11(DX11_CORE& dx11_core){
	
	s_dx11_core = dx11_core;
	
	//Shader共用管理クラス
	comShader = new CommonShader(dx11_core.pD3DDevice);
	
	state_manager_dx = new StateManagerDX11(dx11_core.pD3DDevice, dx11_core.pD3DDeviceContext);

	// ブレンド・ステート・オブジェクトの作成
	// パーティクル用ブレンド・ステート・オブジェクト
	D3D11_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(D3D11_BLEND_DESC));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendOp = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlend = D3D11_BLEND_SRC_ALPHA;//D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D11_BLEND_ONE;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D11_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D11_BLEND_ZERO;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D11_COLOR_WRITE_ENABLE_ALL;

	//	BlendFunc(&blendDesc, DOG_SRC_ALPHA, DOG_ONE_MINUS_SRC_ALPHA);

	state_manager_dx->SetBlendState(&blendDesc);


	//深度バッファを書き込みしない.
	// デプス・ステート・オブジェクトの作成
	// パーティクル用デプス・ステート・オブジェクト
	//ポイントスプライトではデプスバッファのチェックだけして、更新はしない.
	D3D11_DEPTH_STENCIL_DESC depthStencilState;
	ZeroMemory(&depthStencilState, sizeof(D3D11_DEPTH_STENCIL_DESC));
	depthStencilState.DepthEnable = TRUE;
	depthStencilState.DepthWriteMask = D3D11_DEPTH_WRITE_MASK_ALL;	//深度バッファを書き込みする.
	depthStencilState.DepthFunc = D3D11_COMPARISON_LESS;
	depthStencilState.StencilEnable = FALSE;
	depthStencilState.StencilReadMask = D3D11_DEFAULT_STENCIL_READ_MASK;
	depthStencilState.StencilWriteMask = D3D11_DEFAULT_STENCIL_WRITE_MASK;
	//	depthStencilState.StencilFunc = D3D11_STENCIL_OP_KEEP;
	//	depthStencilState.StencilDepthFailOp = D3D11_STENCIL_OP_KEEP;
	//	depthStencilState.StencilPassOp = D3D11_STENCIL_OP_KEEP;
	//	depthStencilState.StencilFailOp = D3D11_COMPARISON_ALWAYS;
	state_manager_dx->SetDepthStencilState(&depthStencilState);


	// ラスタライザの作成//
	//カリング等を制御//
	D3D11_RASTERIZER_DESC rasterizerState;
	ZeroMemory(&rasterizerState, sizeof(D3D11_RASTERIZER_DESC));
	rasterizerState.FillMode = D3D11_FILL_SOLID;
	rasterizerState.CullMode = D3D11_CULL_BACK;
	rasterizerState.FrontCounterClockwise = TRUE;
	rasterizerState.DepthBias = 0;
	rasterizerState.DepthBiasClamp = 0;
	rasterizerState.SlopeScaledDepthBias = 0;
	rasterizerState.DepthClipEnable = FALSE;
	rasterizerState.ScissorEnable = FALSE;
	rasterizerState.MultisampleEnable = FALSE;
	rasterizerState.AntialiasedLineEnable = FALSE;
	state_manager_dx->SetRasterizerState(&rasterizerState);


	s_filterDoF = new FilterRGBandDepth(&s_dx11_core, comShader);

}

void ResetDX11(DX11_CORE& dx11_core){

	s_dx11_core = dx11_core;

}

void TerminateDX11(){

	if(m_pointRendererDX11){ delete m_pointRendererDX11;}
	//if(m_atomRendererDX11){ delete m_atomRendererDX11;}
	
	delete state_manager_dx;
	delete s_filterDoF;

	if(comShader){ delete comShader;}

}


template <typename F, typename D>
static void Transpose(F* matrix44, const D* source44){
	matrix44[0] = (F)source44[0];
	matrix44[1] = (F)source44[4];
	matrix44[2] = (F)source44[8];
	matrix44[3] = (F)source44[12];
	matrix44[4] = (F)source44[1];
	matrix44[5] = (F)source44[5];
	matrix44[6] = (F)source44[9];
	matrix44[7] = (F)source44[13];
	matrix44[8] = (F)source44[2];
	matrix44[9] = (F)source44[6];
	matrix44[10] = (F)source44[10];
	matrix44[11] = (F)source44[14];
	matrix44[12] = (F)source44[3];
	matrix44[13] = (F)source44[7];
	matrix44[14] = (F)source44[11];
	matrix44[15] = (F)source44[15];

}

void RenderingOneDX11_2(LawData& data, const float* view_matrix_t, const float* projection_matrix, const double focus_distance, const int screen_w, const int screen_h, const RenderingProperty& rendering_property, DX11_CORE* dx11_core) {
	//各ファイルごとのレンダリング処理//

	//visual setting//
	VISUAL_SETTING vis;
	vis.atom = rendering_property.atom_draw;
	vis.atom_poly = rendering_property.atom_poly;
	vis.atom_radius = rendering_property.atom_radius;
	vis.atom_color = rendering_property.atom_color;
	vis.bond = rendering_property.bond_draw;
	vis.bond_poly = rendering_property.bond_poly;
	vis.history_frame = g_visual_history_frame;
	vis.trajectory_width = g_visual_trajectory_width;

	unsigned int box_color = (unsigned int)((1.0f - rendering_property.bg_color[0])*255.f)
		| ((unsigned int)((1.0f - rendering_property.bg_color[1])*255.f) << 8)
		| ((unsigned int)((1.0f - rendering_property.bg_color[2])*255.f) << 16)
		| (0xFF000000);


	unsigned int trajectory_color = (unsigned int)((g_visual_trajectory_color[0])*255.f)
		| ((unsigned int)((g_visual_trajectory_color[1])*255.f) << 8)
		| ((unsigned int)((g_visual_trajectory_color[2])*255.f) << 16)
		| (0xFF000000);

	if (m_boxRendererDX11 == NULL) m_boxRendererDX11 = new BoxRendererDX11(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);
	m_boxRendererDX11->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
	state_manager_dx->StoreState();



	auto CrossSectionChecker = [&rendering_property](vec3f r, uint32_t color) {
		int res = -1;
		for (int i = 0; i < 3; ++i) {
			if (rendering_property.is_corss_section_effect[i]) {
				const vec3d dr{
					r.x - rendering_property.corss_sections[i].point.x,
					r.y - rendering_property.corss_sections[i].point.y,
					r.z - rendering_property.corss_sections[i].point.z };
				const double inner = dr.x * rendering_property.corss_sections[i].normal.x
					+ dr.y * rendering_property.corss_sections[i].normal.y
					+ dr.z * rendering_property.corss_sections[i].normal.z;

				//int res1 = (inner < 0.0) ? 1 : 0;
				int res1 = (inner < rendering_property.atom_radius) ? 1 : 0;

				if (res == -1) {
					res = res1;
				} else if (rendering_property.corss_section_mode_and_or[i]) {
					res |= res1;
				} else {
					res &= res1;
				}
			}
		}
		return (res > 0) ? true : false;
		};

	switch (data.type) {
	case FILETYPE_ATOM:
		{
			if (rendering_property.atom_draw == VISUAL_ATOM_POINT) {


				//レンダリング処理///////////////////////////////////////////

				if (m_pointRendererDX11 == NULL) m_pointRendererDX11 = new PointRendererDX11(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);

				m_pointRendererDX11->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
				ATOMS_DATA* dat = data.GetDataPointer();

				m_pointRendererDX11->Draw(dat, data.GetBond(), vis);

				if (rendering_property.box_draw) {
					mat33d axis;
					vec3d org;
					data.GetBoxSize(&axis, &org);
					m_boxRendererDX11->Draw(axis, org, 12, box_color);
				}

			} else {
				
				if (rendering_property.atom_draw == VISUAL_ATOM_SPHERE) {

				

					//レンダリング処理///////////////////////////////////////////
					if (vis.atom_color == VISUAL_ATOMCOLOR_HEIGHT) {

						if (m_sphere_color_table_renderer == nullptr) m_sphere_color_table_renderer = new SphereColorTableRendererDX11(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);

						
						ATOMS_DATA* dat = data.GetDataPointer();

						//color map of JET//
						static const float color_table[9 * 4] = { 0.0, 0.0, 0.5647058823529412, 1.0,  //#000090
							0.0, 0.0588235294117647, 1.0, 1.0,					//#000fff
							0.0, 0.5647058823529412, 1.0, 1.0,					//#0090ff
							0.0588235294117647, 1.0, 0.9333333333333333, 1.0,	//#0fffee
							0.5647058823529412, 1.0, 0.4392156862745098,1.0,	//#90ff70
							1.0, 0.9333333333333333, 0.0, 1.0,					//#ffee00
							1.0, 0.4392156862745098, 0.0, 1.0,					//#ff7000
							0.9333333333333333, 0.0, 0.0, 1.0,					//#ee0000
							0.4980392156862745, 0.0, 0.0, 1.0 };				//#7f0000
						
						mat33d axis;
						vec3d org;
						data.GetBoxSize(&axis, &org);

						const float height_center = g_color_hight_center > 0.0 ? g_color_hight_center : axis.c.z / 2.0;
						const float height_range = g_color_hight_range > 0.0 ? g_color_hight_range : axis.c.z / 2.0;

						m_sphere_color_table_renderer->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
						if (IsCrossSectForAtom(rendering_property)){
							m_sphere_color_table_renderer->DrawFunc(dat->pcnt, dat->r, color_table, 9,
								(float)(height_center - height_range), (float)(height_center + height_range), vis, CrossSectionChecker);
						}else{
							m_sphere_color_table_renderer->Draw(dat->pcnt, dat->r, color_table, 9,
								(float)(height_center - height_range), (float)(height_center + height_range), vis);
						}
					} else {

						if (vis.atom_color == VISUAL_ATOMCOLOR_PRESSURE) {
							MessageBox(NULL, _T("warning"), _T("color mode with pressure is not supported in this version\n"), MB_OK);
						}

						if (m_sphere_renderer == NULL) m_sphere_renderer = new SphereRendererDX11(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);


						ATOMS_DATA* dat = data.GetDataPointer();

						const uint32_t* color_table = (vis.atom_color == VISUAL_ATOMCOLOR_BONDNUM) ? (const uint32_t*)bondnum_colors :
							(const uint32_t*)atom_colors;
						const int* color_indexes = (vis.atom_color == VISUAL_ATOMCOLOR_BONDNUM) ? (data.GetBond()->coords) :
							dat->knd;

						m_sphere_renderer->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
						
						if (IsCrossSectForAtom(rendering_property)){
							//m_rayCastingRenderer->SetCrossSection(3, rendering_property.corss_sections, rendering_property.is_corss_section_effect, rendering_property.corss_section_mode_and_or);
							m_sphere_renderer->DrawFunc(dat->pcnt, dat->r, color_indexes, color_table, vis, CrossSectionChecker);
						} else {
							m_sphere_renderer->Draw(dat->pcnt, dat->r, color_indexes, color_table, vis);
						}
					}
				}

				if (vis.bond) {
					if (vis.atom_color == VISUAL_ATOMCOLOR_HEIGHT) {

					} else {

						if (m_cylinder_renderer == NULL) m_cylinder_renderer = new CylinderRendererDX11(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);


						ATOMS_DATA* dat = data.GetDataPointer();

						const uint32_t* color_table = (vis.atom_color == VISUAL_ATOMCOLOR_BONDNUM) ? (const uint32_t*)bondnum_colors :
							(const uint32_t*)atom_colors;
						const int* color_indexes = (vis.atom_color == VISUAL_ATOMCOLOR_BONDNUM) ? (data.GetBond()->coords) :
							dat->knd;

						m_cylinder_renderer->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
						m_cylinder_renderer->Draw(data.GetBond()->count, data.GetBond()->bvector, dat->r, color_indexes, color_table, vis);
					}
				}
				

				if (rendering_property.box_draw) {
					mat33d axis;
					vec3d org;
					data.GetBoxSize(&axis, &org);
					m_boxRendererDX11->Draw(axis, org, 12, box_color);
				}

			}
		}
		break;

	case FILETYPE_FIELD:
		if (rendering_property.field_render_mode == 1) {
			if (m_rayCastingRenderer == NULL) m_rayCastingRenderer = new RayCastingDX(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);
			m_rayCastingRenderer->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
			auto* field = data.GetField();

			m_rayCastingRenderer->SetCrossSection(3, rendering_property.corss_sections, rendering_property.is_corss_section_effect, rendering_property.corss_section_mode_and_or);

			RayCastingDX::ColorRange range;
			range.range_min = rendering_property.field_range_min;
			range.range_max = rendering_property.field_range_max;
			range.alpha_min = rendering_property.field_alpha_min;
			range.alpha_max = rendering_property.field_alpha_max;
			m_rayCastingRenderer->DrawField(field, range, view_matrix_t);

			if (rendering_property.box_draw) {
				m_boxRendererDX11->Draw(field->boxaxis, field->boxorg, 12, box_color);
			}
		}else if (rendering_property.field_render_mode == 10) {
			if (m_rayCastingRenderer_depth == NULL) m_rayCastingRenderer_depth = new RayCastingDX_depth(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);
			m_rayCastingRenderer_depth->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
			auto* field = data.GetField();

			m_rayCastingRenderer_depth->SetCrossSection(3, rendering_property.corss_sections, rendering_property.is_corss_section_effect, rendering_property.corss_section_mode_and_or);

			RayCastingDX_depth::ColorRange range;
			range.range_min = rendering_property.field_range_min;
			range.range_max = rendering_property.field_range_max;
			range.alpha_min = rendering_property.field_alpha_min;
			range.alpha_max = rendering_property.field_alpha_max;
			m_rayCastingRenderer_depth->DrawField(field, range, view_matrix_t, projection_matrix, screen_w, screen_h, m_depth_view);

			if (rendering_property.box_draw) {
				m_boxRendererDX11->Draw(field->boxaxis, field->boxorg, 12, box_color);
			}
		}else{
			if (m_tex3dRenderer == NULL) m_tex3dRenderer = new Tex3DRenderer(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);
			m_tex3dRenderer->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
			auto* field = data.GetField();

			m_tex3dRenderer->DrawField(field, view_matrix_t);

			if (rendering_property.box_draw) {
				m_boxRendererDX11->Draw(field->boxaxis, field->boxorg, 12, box_color);
			}
		}
		break;


	case FILETYPE_KARABA:
		{

			//レンダリング処理///////////////////////////////////////////

			if (m_karabaRendererDX11 == NULL) m_karabaRendererDX11 = new KrbRendererDX11(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);

			m_karabaRendererDX11->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
			KRB_INFO* dat = data.GetDataPointerKRB();

			if (IsCrossSectForAtom(rendering_property)) {
				m_karabaRendererDX11->DrawFunc(dat, trajectory_color, vis, CrossSectionChecker);
			} else {
				m_karabaRendererDX11->Draw(dat, trajectory_color, vis);
			}

			if (rendering_property.box_draw) {
				mat33d axis;
				vec3d org;
				data.GetBoxSize(&axis, &org);
				m_boxRendererDX11->Draw(axis, org, 12, box_color);
			}

		}
		break;


	case FILETYPE_TRAJ:
		{

			const uint32_t trajectory_color = (unsigned int)((g_visual_trajectory_color[0])*255.f)
				| ((unsigned int)((g_visual_trajectory_color[1])*255.f) << 8)
				| ((unsigned int)((g_visual_trajectory_color[2])*255.f) << 16)
				| (0xFF000000);


			const TrajectoryData* traj = data.GetTrajectory();
			if (m_strip_renderer == NULL) m_strip_renderer = new CylinderStripRendererDX11(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);
			m_strip_renderer->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
			m_strip_renderer->Draw(traj->positions, traj->strip_indexes, traj->strip_lengths, traj->num_strips, trajectory_color, vis);

			if (vis.atom == VISUAL_ATOM_SPHERE) {
				
				std::vector<vec3f> positions;
				std::vector<int> color_indexes;
				int k = 0;
				for (int i = 0; i < traj->num_strips; ++i) {
					positions.push_back(traj->positions[traj->strip_indexes[k]]);
					k += traj->strip_lengths[i];
					positions.push_back(traj->positions[traj->strip_indexes[k-1]]);
					
					color_indexes.push_back(0);
					color_indexes.push_back(1);
				}
				
				if (m_sphere_renderer == NULL) m_sphere_renderer = new SphereRendererDX11(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);
				m_sphere_renderer->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);

				if (positions.empty()) {
					int ci[1] = { 0 };
					m_sphere_renderer->Draw(1, traj->positions, ci, (const uint32_t*)atom_colors, vis);
				} else {
					m_sphere_renderer->Draw(positions.size(), &(positions[0]), &(color_indexes[0]), (const uint32_t*)atom_colors, vis);
				}
			}

			if (rendering_property.box_draw) {
				mat33d axis;
				vec3d org;
				data.GetBoxSize(&axis, &org);
				m_boxRendererDX11->Draw(axis, org, 12, box_color);
			}
		}
		break;

	case FILETYPE_PMD:
		{

			if (m_vntRenderer == NULL) {
				m_vntRenderer = new VNTRenderer(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);
			}

			m_vntRenderer->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);


			MATERIAL_INFO* materials;
			IPolyRepository* pmd = data.GetPolyRepository();
			int num_materials = pmd->GetMaterialList(&materials);
			UINT vertex_stride;
			UINT index_stride;
			LPVOID vertex_buffer;
			int vertex_count = pmd->GetVertexBuffer(&vertex_buffer, &vertex_stride);
			LPVOID index_buffer;
			int index_count = pmd->GetIndexBuffer(&index_buffer, &index_stride);


			vec3f r = { 0.0f, 0.0f, 0.0f };
			if (num_materials > 0) {
				m_vntRenderer->UpdateVertexBuffer(vertex_buffer, vertex_count, vertex_stride);//頂点データをVRAMに転送
				m_vntRenderer->UpdateIndexBuffer(index_buffer, index_count, index_stride);	//インデックスデータをVRAMに転送
				m_vntRenderer->DrawMaterials(num_materials, materials, &r);
			}



		}
		break;
	case FILETYPE_VMEC:
		{


			//レンダリング処理///////////////////////////////////////////

			if (m_pointSpriteDX11 == NULL) {
				m_pointSpriteDX11 = new PointSpriteDX11(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);
			}

			m_pointSpriteDX11->SetViewAndProjectionMatrix(view_matrix_t, projection_matrix);
			VMEC_Repository* vmec = data.GetVmec();
			float* values = vmec->GetFieldPointer();
			size_t count = vmec->GetNumParticles();
			float range_max[3], range_min[3];
			float alpha;
			vmec->GetEffectiveRange(range_max, range_min, &alpha);
			int stride = vmec->GetStride();

			mat33d boxaxis;
			vec3d boxorg;
			vmec->GetBoxaxis(&boxaxis, &boxorg);
			//double width = 0.001 * sqrt((boxaxis.a)*(boxaxis.a) + (boxaxis.b)*(boxaxis.b) + (boxaxis.c)*(boxaxis.c));
			double width = 0.001 * sqrt((boxaxis.a)*(boxaxis.a) + (boxaxis.b)*(boxaxis.b) + (boxaxis.c)*(boxaxis.c));

			m_pointSpriteDX11->SetPointSize((float)width);

			m_pointSpriteDX11->Draw(values, count, stride, range_max, range_min, alpha);

		}
		break;

	}
	state_manager_dx->RestoreState();


}


void Draw2D(DX11_CORE* dx11_core) {

	if (m_vntRenderer == NULL) {
		m_vntRenderer = new VNTRenderer(dx11_core->pD3DDevice, dx11_core->pD3DDeviceContext, comShader);
	}
	
	mat44f mat_identify;
	mat_identify.SetIdentity();
	m_vntRenderer->SetViewAndProjectionMatrix((float*)&mat_identify, (float*)&mat_identify);

	//WCHAR test_texture[] = _T("C:\\physics\\VisualStudio\\AIScope3\\対応ファイル仕様書\\pmd\\ion-boy_facetxt.bmp");
	WCHAR test_texture[] = _T("C:\\physics\\VisualStudio\\AIScope3\\AIScope3_vs2015_git\\resource\\button0.png");
	//MATERIAL_INFO* materials;
	MATERIAL_INFO materials[1];
	materials[0].indexCount = 6;
	materials[0].indexOffset = 0;
	materials[0].textureid = -1;
	materials[0].texture_name = test_texture;
	//materials[0].texture_name = NULL;
	materials[0].diffuse_color[0] = 0.5f;
	materials[0].diffuse_color[1] = 0.5f;
	materials[0].diffuse_color[2] = 0.5f;
	materials[0].diffuse_color[3] = 1.0f;
	//IPolyRepository* pmd = data.GetPolyRepository();
	int num_materials = 1;// pmd->GetMaterialList(&materials);
	UINT vertex_stride = sizeof(float) * 8;
	UINT index_stride = 4;

	struct RectF {
		float x0;
		float y0;
		float x1;
		float y1;
	}; 

	RectF button_position = {-1.0f,-1.0f, -0.9f, -0.9f};
	RectF button_coords = { 0.0f,0.0f, 1.0f, 1.0f };

	VERTEX_F3F3F2 vertexes[4];
	vertexes[2].position.Set(button_position.x1, button_position.y0, 0.0);
	vertexes[2].normal.Set(0.0, 0.0, -1.0);
	vertexes[2].coord_u = button_coords.x1;
	vertexes[2].coord_v = button_coords.y1;
	
	vertexes[3].position.Set(button_position.x1, button_position.y1, 0.0);
	vertexes[3].normal.Set(0.0, 0.0, -1.0);
	vertexes[3].coord_u = button_coords.x1;
	vertexes[3].coord_v = button_coords.y0;

	vertexes[1].position.Set(button_position.x0, button_position.y1,0.0);
	vertexes[1].normal.Set(0.0, 0.0, -1.0);
	vertexes[1].coord_u = button_coords.x0;
	vertexes[1].coord_v = button_coords.y0;

	vertexes[0].position.Set(button_position.x0, button_position.y0, 0.0);
	vertexes[0].normal.Set(0.0, 0.0, -1.0);
	vertexes[0].coord_u = button_coords.x0;
	vertexes[0].coord_v = button_coords.y1;

	LPVOID vertex_buffer = (LPVOID)vertexes;
	int vertex_count = 4;// pmd->GetVertexBuffer(&vertex_buffer, &vertex_stride);
	//LPVOID index_buffer;
	int index_buffer[6] = { 0,1,2, 1,2,3 };
	int index_count = 6;// pmd->GetIndexBuffer(&index_buffer, &index_stride);


	vec3f r = { 0.0f, 0.0f, 0.0f };
	if (num_materials > 0) {
		m_vntRenderer->UpdateVertexBuffer(vertex_buffer, vertex_count, vertex_stride);//頂点データをVRAMに転送
		m_vntRenderer->UpdateIndexBuffer(index_buffer, index_count, index_stride);	//インデックスデータをVRAMに転送
		m_vntRenderer->DrawMaterials(num_materials, materials, &r);
	}



}

void ClearRenderingTargetDX11(const float* bg_color) {
	DX11_CORE* dx11_core = &s_dx11_core;

	//BackBufferのクリア
	dx11_core->pD3DDeviceContext->ClearRenderTargetView(dx11_core->pRenderTargetView, bg_color);

	//DepthBufferのクリア
	dx11_core->pD3DDeviceContext->ClearDepthStencilView(dx11_core->pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

}

bool HasField(std::vector<LawData>& data_list) {
	for (auto& a : data_list) {
		if (a.type == FILETYPE_FIELD) {
			return true;
		}
	}
	return false;
}

void RenderingCoreDX11(std::vector<LawData>& data_list,  int screen_w, int screen_h, int range_x, int range_y, int range_w, int range_h, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property)
{
	DX11_CORE* dx11_core = &s_dx11_core;

	
	//変換行列計算
	float view_matrix_t[16];
	Transpose<float, double>(view_matrix_t, view_matrix);

	mat44f projection_matrix;

	const int num_files =  data_list.size();
	if ((rendering_property.multiview_mode == 0) || (num_files == 1)) {
		//粒子とfieldを重ねたrendering
		// Setup the viewport
		D3D11_VIEWPORT vp;
		vp.Width = (FLOAT)range_w;
		vp.Height = (FLOAT)range_h;
		vp.MinDepth = 0.0f;
		vp.MaxDepth = 1.0f;
		vp.TopLeftX = 0;
		vp.TopLeftY = 0;
		dx11_core->pD3DDeviceContext->RSSetViewports(1, &vp);

		const int image_w = screen_w;
		const int image_h = screen_h;

		SetProjectionRange(rendering_property.projection_mode, image_w, image_h, range_x, range_y, range_w, range_h, focus_distance, &projection_matrix);


		if (HasField(data_list) && (num_files > 1)) {
			//Fieldと粒子を重ねたレンダリング//
			//field以外を先にレンダリングしてから、その上にfieldを重ねる//
			//ただし、fieldの積分ではzバッファの値の深さで止める//
			// 
			//レンダリング先のDepthを読見込み可能にする//
			if(s_readable_depth==nullptr) s_readable_depth = new ReadableDepth(dx11_core);
			m_depth_view = s_readable_depth->Begin();

			{//先に粒子をrendering
				RenderingProperty rendering_property2{ rendering_property };
				rendering_property2.box_draw = 0;
				for (auto& a : data_list) {
					if (a.type == FILETYPE_FIELD) continue;
					RenderingOneDX11_2(a, view_matrix_t, (float*)&projection_matrix, focus_distance, range_w, range_h, rendering_property2, dx11_core);
				}
			}
			
			s_readable_depth->End();
			
			dx11_core->pD3DDeviceContext->ClearDepthStencilView(dx11_core->pDepthStencilView, D3D11_CLEAR_DEPTH, 1.0f, 0);

			//続いてfieldをrendering//
			for (auto& a : data_list) {
				if (a.type == FILETYPE_FIELD) {
					RenderingProperty rendering_property2{ rendering_property };
					rendering_property2.field_render_mode = 10;//Fieldと粒子を重ねたレンダリング//
					RenderingOneDX11_2(a, view_matrix_t, (float*)&projection_matrix, focus_distance, range_w, range_h, rendering_property2, dx11_core);
					break;
				}
			}
			

		}else{
			//通常のレンダリング//
			for (auto& a : data_list) {
				RenderingOneDX11_2(a, view_matrix_t, (float*)&projection_matrix, focus_distance, range_w, range_h, rendering_property, dx11_core);
			}			
		}
	} else {


		//スクリーンサイズ比からイメージを並べる数を計算//
		double dnx = sqrt((double)num_files * (double)screen_w / (double)screen_h);
		int multiview_nx = (int)(dnx)+1;
		if (multiview_nx > num_files) { multiview_nx = num_files; }
		int multiview_ny = num_files / multiview_nx;
		if (num_files > multiview_nx * multiview_ny) { multiview_ny++; }


		int nx = 0;
		int ny = 0;// multiview_ny - 1;
		for (auto it = data_list.begin(), it_end = data_list.end(); it != it_end; ++it) {


			//一枚当たりのイメージサイズ//
			const int image_x = (screen_w * nx) / multiview_nx;
			const int image_w = (screen_w * (nx + 1)) / multiview_nx - image_x;
			const int image_y = (screen_h * ny) / multiview_ny;
			const int image_h = (screen_h * (ny + 1)) / multiview_ny - image_y;

			//要求描画範囲とオーバーラップがあるか確認//
			if (range_x + range_w > image_x) {
				if (image_x + image_w > range_x) {
					if (range_y + range_h > image_y) {
						if (image_y + image_h > range_y) {

							//オーバーラップ部分の範囲(ただしimage_x,yからの相対位置)
							const int real_range_x = max(range_x - image_x, 0);
							const int real_range_w = min(range_x + range_w - image_x, image_w) - real_range_x;
							const int real_range_y = max(range_y - image_y, 0);
							const int real_range_h = min(range_y + range_h - image_y, image_h) - real_range_y;

							//オーバーラップ部分の範囲(ただしrange_x,yからの相対位置)
							const int view_x = max(0, image_x - range_x);
							const int view_w = min(range_w, image_x + image_w - range_x) - view_x;
							const int view_y = max(0, image_y - range_y);
							const int view_h = min(range_h, image_y + image_h - range_y) - view_y;

							//glViewport(view_x, view_y, view_w, view_h);
							D3D11_VIEWPORT vp;
							vp.Width = (FLOAT)view_w;
							vp.Height = (FLOAT)view_h;
							vp.MinDepth = 0.0f;
							vp.MaxDepth = 1.0f;
							vp.TopLeftX = view_x;
							vp.TopLeftY = view_y;
							dx11_core->pD3DDeviceContext->RSSetViewports(1, &vp);


							SetProjectionRange(rendering_property.projection_mode, image_w, image_h, real_range_x, real_range_y, real_range_w, real_range_h, focus_distance, &projection_matrix);
							//SetProjectionGL( image_w, image_h, cam->GetForcusDistance() );
							//RenderingOneGL15(fi, view_matrix, focus_distance, rendering_property);
							RenderingOneDX11_2(*it, view_matrix_t, (float*)&projection_matrix, focus_distance, range_w, range_h, rendering_property, dx11_core);

						}
					}
				}
			}
			nx++;
			if (nx == multiview_nx) {
				nx = 0;
				//ny--;
				ny++;
			}


		}

	}

	/*
	操作ボタンの表示
	Draw2D(dx11_core);
	*/
}

void CheckRtvDX11() {
#ifdef _DEBUG
	{
		ID3D11RenderTargetView* org_RTV;
		ID3D11DepthStencilView* org_DSV;
		s_dx11_core.pD3DDeviceContext->OMGetRenderTargets(1, &org_RTV, &org_DSV);
		SAFE_RELEASE(org_RTV);
		SAFE_RELEASE(org_DSV);
	}
#endif
}

void RenderingDX11(std::vector<LawData>& data_list, int screen_w, int screen_h, int range_x, int range_y, int range_w, int range_h, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property) {
	
	
	switch ((g_filter_mode == 1) ? FILTER_DEPTH_OF_FIELD : FILTER_NONE) {
	case FILTER_NONE:
		ClearRenderingTargetDX11(rendering_property.bg_color);
		RenderingCoreDX11(data_list, screen_w, screen_h, range_x, range_y, range_w, range_h, view_matrix, focus_distance, rendering_property);
		break;
	case FILTER_DEPTH_OF_FIELD:
		s_filterDoF->Render(data_list, screen_w, screen_h, range_x, range_y, range_w, range_h, view_matrix, focus_distance, rendering_property);
		break;
	}
	
}


///////////////////////////////////////////////////////////////////
//変換行列用ライブラリ
///////////////////////////////////////////////////////////////////

namespace {
	void SetProjectionPerspectiveMatrix(float angle, float wph, float z_near, float z_far, mat44f* pProjectionMatrix) {
		pProjectionMatrix->Clear();

		pProjectionMatrix->m11 = 1.0f / tan(angle);
		pProjectionMatrix->m22 = pProjectionMatrix->m11 * wph;
		pProjectionMatrix->m33 = z_far / (z_far - z_near);
		pProjectionMatrix->m43 = -z_near * pProjectionMatrix->m33;
		pProjectionMatrix->m34 = 1.0f;
		pProjectionMatrix->m44 = 0.0f;


	}

	void SetProjectionMatrixFrustnum(double left, double right, double bottom, double top,
		double zNear, double zFar, mat44f* pProjectionMatrix) {

		pProjectionMatrix->Clear();

		const double width = right - left;
		const double height = top - bottom;
		const double zwidth = zFar - zNear;

		pProjectionMatrix->m11 = (float)(2.0*zNear / width);
		pProjectionMatrix->m31 = -(float)((right + left) / width);
		pProjectionMatrix->m22 = (float)(2.0*zNear / height);
		pProjectionMatrix->m32 = -(float)((top + bottom) / height);
		//pProjectionMatrix->m33 = (float)((zFar + zNear) / zwidth);
		//pProjectionMatrix->m43 = -(float)((zFar*zNear) / zwidth);
		pProjectionMatrix->m33 = (float)((zFar) / zwidth);
		pProjectionMatrix->m43 = -(float)((zFar*zNear) / zwidth);
		pProjectionMatrix->m34 = 1.0f;
		pProjectionMatrix->m44 = 0.0f;

		/* OpenGL(右手座標系)でのPorjectionMaxtixの定義に加えて,
		 z座標のみ反転する次のmatrixを掛けたもの

		  ( 1  0  0  0
			0  1  0  0
			0  0 -1  0
			0  0  0  1  )
		 */



	}


	void SetProjectionMatrixOrtho(double left, double right, double bottom, double top,
		double zNear, double zFar, mat44f* pProjectionMatrix) {

		pProjectionMatrix->Clear();

		const double width = right - left;
		const double height = top - bottom;
		const double zwidth = zFar - zNear;

		pProjectionMatrix->m11 = (float)(2.0 / width);
		pProjectionMatrix->m41 = -(float)((right + left) / width);
		pProjectionMatrix->m22 = (float)(2.0 / height);
		pProjectionMatrix->m42 = -(float)((top + bottom) / height);
		
		pProjectionMatrix->m33 = (float)(1.0 / zwidth);
		pProjectionMatrix->m43 = -(float)((zNear) / zwidth);
		pProjectionMatrix->m44 = 1.0f;
		
		
		
		/* OpenGL(右手座標系)でのPorjectionMaxtixの定義に加えて,
		z座標のみ反転する次のmatrixを掛けたもの

		( 1  0  0  0
		0  1  0  0
		0  0 -1  0
		0  0  0  1  )
		*/
	}




	//一つのデータの可視化に関して、描画範囲をPrjection範囲として設定//
	void SetProjectionRange(int projection_mode, int image_w, int image_h, int range_x, int range_y, int range_w, int range_h, double distance, mat44f* projection_matrix) {

		//射影変換の設定===================================//
		if (projection_mode == 1) {  //正射影//
			double top = distance * tan(VIEW_ANGLE);
			double right = top * (double)image_w / (double)image_h;

			double dh = top * 2.0 * ((double)range_h / (double)image_h);
			double dw = right * 2.0 * ((double)range_w / (double)image_w);

			double btm = top * (-1.0 + 2.0 * ((double)range_y / (double)image_h));
			double left = right * (-1.0 + 2.0 * ((double)range_x / (double)image_w));

			//SetProjectionMatrixOrtho(left, left + dw, btm, btm + dh, VIEW_NEAR*distance, VIEW_FAR * distance, projection_matrix);
			double top2 = top * (1.0 - 2.0 * ((double)range_y / (double)image_h));
			SetProjectionMatrixOrtho(left, left + dw, top2 - dh, top2, VIEW_NEAR*distance, VIEW_FAR * distance/5.0, projection_matrix);

		} else {
			double top = VIEW_NEAR * distance * tan(VIEW_ANGLE);
			double right = top * (double)image_w / (double)image_h;

			double dh = top * 2.0 * ((double)range_h / (double)image_h);
			double dw = right * 2.0 * ((double)range_w / (double)image_w);

			double btm = top * (-1.0 + 2.0 * ((double)range_y / (double)image_h));
			double left = right * (-1.0 + 2.0 * ((double)range_x / (double)image_w));


			//SetProjectionMatrixFrustnum(left, left + dw, btm, btm + dh, VIEW_NEAR*distance, VIEW_FAR * distance, projection_matrix);
			double top2 = top * (1.0 - 2.0 * ((double)range_y / (double)image_h));
			SetProjectionMatrixFrustnum(left, left + dw, top2 - dh, top2, VIEW_NEAR*distance, VIEW_FAR * distance, projection_matrix);
		}
		//===================================射影変換//
	}
}


//color mapを出力//
void OutputColorMap() {

	FILE* fp = _tfopen(_T("color_map_height.bmp"), _T("wb"));
	if (!fp) {
		printf("color map用bmpファイルの作成に失敗しました\n");
		return;
	}

	const int width = 32;
	const int height = 256;
	WriteBMP wbmp(width, height, 3);
	BYTE colorRGBA[width * height * 4];

	//color map of JET//
	static const float color_table[9 * 4] = { 0.0, 0.0, 0.5647058823529412, 1.0,  //#000090
		0.0, 0.0588235294117647, 1.0, 1.0,					//#000fff
		0.0, 0.5647058823529412, 1.0, 1.0,					//#0090ff
		0.0588235294117647, 1.0, 0.9333333333333333, 1.0,	//#0fffee
		0.5647058823529412, 1.0, 0.4392156862745098,1.0,	//#90ff70
		1.0, 0.9333333333333333, 0.0, 1.0,					//#ffee00
		1.0, 0.4392156862745098, 0.0, 1.0,					//#ff7000
		0.9333333333333333, 0.0, 0.0, 1.0,					//#ee0000
		0.4980392156862745, 0.0, 0.0, 1.0 };				//#7f0000

	for (int iy = 0; iy < height; ++iy) {
		float y = (float)(height - iy - 1)/(float)(height) * 8.0;
		int t = (int)y;
		float dy = y - (float)t;
		for (int k = 0; k < 4; ++k) {
			colorRGBA[iy * width*4 + k] = (int)(255.0 * (color_table[t * 4 + k] * (1.0f - dy) + color_table[(t + 1) * 4 + k] * dy));
		}
		for (int ix = 1; ix < width; ++ix) {
			colorRGBA[(iy * width + ix) * 4] = colorRGBA[iy * width * 4];
			colorRGBA[(iy * width + ix) * 4 + 1] = colorRGBA[iy * width * 4 + 1];
			colorRGBA[(iy * width + ix) * 4 + 2] = colorRGBA[iy * width * 4 + 2];
			colorRGBA[(iy * width + ix) * 4 + 3] = colorRGBA[iy * width * 4 + 3];
		}
	}

	wbmp.CreateDIBfromRGBA(colorRGBA, width * 4, 0, 0, width, height);
	wbmp.WriteDIB(fp);
	fclose(fp);
	//delete[] colorRGBA;

}


//DirectXで描画した内容をビットマップ画像として保存//
void OutputDX11(std::vector<LawData>& data_list, const TCHAR *fName, int width, int height, int magnify, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property) {

	DX11_CORE* dx11_core = &s_dx11_core;

	FILE* fp = _tfopen(fName, _T("wb"));
	if (!fp) {
		printf("bmpファイルの作成に失敗しました\n");
		return;
	}

	WriteBMP wbmp(width * magnify, height * magnify, 3);




	//(2)スクリーンフォーマットを取得//
	ID3D11Texture2D* pBackBuffer = NULL;
	HRESULT hr = dx11_core->pSwapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (LPVOID*)&pBackBuffer);
	if (FAILED(hr)) return;

	D3D11_TEXTURE2D_DESC descBackBuffer;
	pBackBuffer->GetDesc(&descBackBuffer);
	pBackBuffer->Release();


	//(3)CPU読み出し可能なバッファをGPU上に作成//
	D3D11_TEXTURE2D_DESC Texture2DDesc;
	Texture2DDesc.ArraySize = 1;
	Texture2DDesc.BindFlags = 0;
	Texture2DDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
	Texture2DDesc.Format = descBackBuffer.Format;
	Texture2DDesc.Height = descBackBuffer.Height;
	Texture2DDesc.Width = descBackBuffer.Width;
	Texture2DDesc.MipLevels = 1;
	Texture2DDesc.MiscFlags = 0;
	Texture2DDesc.SampleDesc.Count = 1;
	Texture2DDesc.SampleDesc.Quality = 0;
	Texture2DDesc.Usage = D3D11_USAGE_STAGING;

	ID3D11Texture2D *hCaptureTexture;
	dx11_core->pD3DDevice->CreateTexture2D(&Texture2DDesc, 0, &hCaptureTexture);

	BYTE *buffer = NULL;

	//int offset_y = 0;

	for (int y = 0; y < magnify; y++) {
		//int offset_x = 0;
		for (int x = 0; x < magnify; x++) {


			//(1)DirectX11でレンダリングする関数.フリップはしない//
			//ClearRenderingTargetDX11();
			RenderingDX11(data_list, width*magnify, height*magnify, width*x, height*y, width, height, view_matrix, focus_distance, rendering_property);



			//(4)作成したCPU読み込み可能バッファにGPU上でデータをコピー//
			ID3D11Resource *hResource;
			dx11_core->pRenderTargetView->GetResource(&hResource);
			dx11_core->pD3DDeviceContext->CopyResource(hCaptureTexture, hResource);
			hResource->Release();
			//SaveWICTextureToFile

			//(5)GPU上の読み込み可能バッファのメモリアドレスのマップを開く//
			D3D11_MAPPED_SUBRESOURCE mappedResource;
			dx11_core->pD3DDeviceContext->Map(hCaptureTexture, 0, D3D11_MAP_READ, 0, &mappedResource);

			//(6)CPU上のメモリにバッファを確保//
			int src_stride = mappedResource.RowPitch;
			if (buffer == NULL) {
				buffer = new BYTE[src_stride * height];
			}

			//(7)GPU上の読み込み可能バッファからCPU上のバッファへ転送, マップを閉じる//
			CopyMemory(buffer, mappedResource.pData, src_stride * height);
			dx11_core->pD3DDeviceContext->Unmap(hCaptureTexture, 0);


			//(8)CPU上のバッファの画像をファイル書き出し//
			if (descBackBuffer.Format == DXGI_FORMAT_R16G16B16A16_FLOAT) {
				wbmp.CreateDIBfromRGBA_FP16(buffer, src_stride, width * x, height * y, width, height);
			} else if (descBackBuffer.Format == DXGI_FORMAT_R8G8B8A8_UNORM) {
				wbmp.CreateDIBfromRGBA(buffer, src_stride, width * x, height * y, width, height);
			}


		}


	}
	hCaptureTexture->Release();

	wbmp.WriteDIB(fp);

	delete[] buffer;
	fclose(fp);

	if (rendering_property.atom_color == VISUAL_ATOMCOLOR_HEIGHT) {
		OutputColorMap();
	}

}
