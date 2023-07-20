// PointRendererDX11.h: PointRendererDX11 クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#pragma once

#include <d3d11.h>
extern size_t g_max_vram_size;


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

#include "PointRendererDX11.h"

class PointSpriteDX11 : public BasePointRendererDX11{
private:

    
    struct mVERTEX {
    vec3f position;
    float color[4];
    };
    


private:




    int m_vram_size = 0;
    int m_vbo_vcount = 0;
    mVERTEX* m_vbo_vertex = nullptr;

    

    float* m_prev_render_dat = nullptr;	//前回レンダリングしたデータ
    
public:


    PointSpriteDX11(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
        BasePointRendererDX11(pD3DDevice, pD3DDeviceContext, comShader)
    {
        SetPointSize(1.0f);
        InitShader(RGB_FLOAT_COLOR_FLOAT);
    }


    virtual ~PointSpriteDX11() {
        delete[] m_vbo_vertex;
    }




	//VBOの設定
	void Draw(float* values, const int count, const int stride, const float* range_max, const float* range_min, float alpha) {	//int j_mode, float Jmax

		if (values == NULL) return;
		if (count <= 0) return;



		//vboのセット
		int redraw_flag = 0;


		if (values != m_prev_render_dat) {
			redraw_flag = 1;
		}



		//点の描画//
		{


			//頂点数の計算//
			size_t num_draw_once = g_max_vram_size / (sizeof(vec3f));
			if (count > num_draw_once) {//頂点のバッファサイズを超える場合は複数回に分けて描画する//
				redraw_flag = 1;
			}

			size_t i_start = 0;
			while (i_start < count) {

				if (i_start + num_draw_once > count) { num_draw_once = count - i_start; };


				if (redraw_flag) {

					CreateSphere(i_start, num_draw_once, stride, values, range_max, range_min, alpha);
				}



				if (m_vbo_vcount) {
					//   UpdateVertexBuffer(points + i_start, num_draw_once, sizeof(vec3f));	//頂点データをVRAMに転送

					UpdateVertexBuffer(m_vbo_vertex, m_vbo_vcount, sizeof(mVERTEX));	//頂点データをVRAMに転送
					//UpdateIndexBuffer(m_vbo_indexes, m_vbo_icount, sizeof(int));	//インデックスデータをVRAMに転送

					DrawOnlyVertex(m_vbo_vcount, D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

				}

				i_start += num_draw_once;
			}

		}




		m_prev_render_dat = values;

	}

private:


    void CreateSphere(int start, int count, int stride, float* values, const float* range_max, const float* range_min, float alpha) {

        //vboサイズ
        int vbo_v_count = count;
        int need_vram_sz = vbo_v_count * sizeof(mVERTEX);



        if (m_vram_size < need_vram_sz) {
            m_vram_size = need_vram_sz;

            delete[] m_vbo_vertex;
            m_vbo_vertex = new mVERTEX[vbo_v_count];
        }



        m_vbo_vcount = 0;

        const int stride_f = stride / sizeof(float);

        float range_delta[3];
        for (int i = 0; i < 3; i++) {
            range_delta[i] = range_max[i] - range_min[i];
        }


        //点データのセット
        const int i_end = start + count;
        int a = sizeof(mVERTEX);

        if (stride_f == 4) {
            for (int i = start; i < i_end; i++) {

                m_vbo_vertex[m_vbo_vcount].position.x = values[i * stride_f];
                m_vbo_vertex[m_vbo_vcount].position.y = values[i * stride_f + 1];
                m_vbo_vertex[m_vbo_vcount].position.z = values[i * stride_f + 2];
                // m_vbo_vertex[m_vbo_vcount].alpha = alpha[i];
                float f = (values[i * stride_f + 3] - range_min[0]) / (range_delta[0]);
                m_vbo_vertex[m_vbo_vcount].color[0] = 0.4f * f;
                m_vbo_vertex[m_vbo_vcount].color[1] = 0.8f * f;
                m_vbo_vertex[m_vbo_vcount].color[2] = f;
                m_vbo_vertex[m_vbo_vcount].color[3] = alpha;
                m_vbo_vcount++;
            }

        } else if (stride_f == 5) {
            for (int i = start; i < i_end; i++) {

                m_vbo_vertex[m_vbo_vcount].position.x = values[i * stride_f];
                m_vbo_vertex[m_vbo_vcount].position.y = values[i * stride_f + 1];
                m_vbo_vertex[m_vbo_vcount].position.z = values[i * stride_f + 2];
                // m_vbo_vertex[m_vbo_vcount].alpha = alpha[i];
                const float f0 = (values[i * stride_f + 3] - range_min[0]) / (range_delta[0]);
                const float f1 = (values[i * stride_f + 4] - range_min[1]) / (range_delta[1]);
                /*
                m_vbo_vertex[m_vbo_vcount].color[0] = f0 + f1 * 0.2f;
                m_vbo_vertex[m_vbo_vcount].color[1] = (f0*f0 + f1*f1) * 0.5;
                m_vbo_vertex[m_vbo_vcount].color[2] = f1 * 1.4 + f0 * 0.2f;
                */
                m_vbo_vertex[m_vbo_vcount].color[0] = 1.0 * (f0 * f0 + f1 * f1);
                m_vbo_vertex[m_vbo_vcount].color[1] = f0;
                m_vbo_vertex[m_vbo_vcount].color[2] = 1.1f * f1 + 0.3 * f0;

                m_vbo_vertex[m_vbo_vcount].color[3] = alpha;
                m_vbo_vcount++;
            }
        } else if (stride_f == 6) {
            for (int i = start; i < i_end; i++) {

                m_vbo_vertex[m_vbo_vcount].position.x = values[i * stride_f];
                m_vbo_vertex[m_vbo_vcount].position.y = values[i * stride_f + 1];
                m_vbo_vertex[m_vbo_vcount].position.z = values[i * stride_f + 2];
                // m_vbo_vertex[m_vbo_vcount].alpha = alpha[i];
                const float f0 = (values[i * stride_f + 3] - range_min[0]) / (range_delta[0]);
                const float f1 = (values[i * stride_f + 4] - range_min[1]) / (range_delta[1]);
                const float f2 = (values[i * stride_f + 5] - range_min[2]) / (range_delta[2]);
                m_vbo_vertex[m_vbo_vcount].color[0] = f0;
                m_vbo_vertex[m_vbo_vcount].color[1] = f1;
                m_vbo_vertex[m_vbo_vcount].color[2] = f2;
                m_vbo_vertex[m_vbo_vcount].color[3] = alpha;
                m_vbo_vcount++;
            }
        }
    }


};



