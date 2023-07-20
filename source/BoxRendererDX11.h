
#ifndef BoxRendererDX11_h
#define BoxRendererDX11_h

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




class BoxRendererDX11{
private:


	RendererWrapper renderer;

    VERTEX_F3UI1* m_vbo_vertex = nullptr;
    int* m_vbo_indexes = nullptr;


public:


	BoxRendererDX11(ID3D11Device* pD3DDevice, ID3D11DeviceContext* pD3DDeviceContext, CommonShader* comShader) :
		renderer(pD3DDevice, pD3DDeviceContext, comShader)
	{
		InitShader();
	}


    virtual ~BoxRendererDX11() {
		delete[] m_vbo_vertex;
		delete[] m_vbo_indexes;

	}



    void Draw(const mat33d& axis, const vec3d& org, int size, unsigned int color) {

        if (size != 12) {
            return;
        }

        const int vbo_v_count = 8;
        const int vbo_i_count = 12 * 2;
        if (m_vbo_vertex == NULL) {
            m_vbo_vertex = new VERTEX_F3UI1[vbo_v_count];
            m_vbo_indexes = new int[vbo_i_count];

            //    UpdateVertexBuffer(NULL, vbo_v_count*16, sizeof(VERTEX_F3UI1));	//頂点データのバッファだけ確保
                //UpdateIndexBuffer(NULL, vbo_i_count * 16, sizeof(int));	//インデックスデータのバッファだけ確保
        }

        mat33f axis_f(axis);
        vec3f org_f{ (float)org.x, (float)org.y, (float)org.z };

        m_vbo_vertex[0].position = org_f;
        m_vbo_vertex[1].position = org_f + axis_f.a;
        m_vbo_vertex[2].position = org_f + axis_f.b;
        m_vbo_vertex[3].position = org_f + axis_f.a + axis_f.b;

        m_vbo_vertex[4].position = m_vbo_vertex[0].position + axis_f.c;
        m_vbo_vertex[5].position = m_vbo_vertex[1].position + axis_f.c;
        m_vbo_vertex[6].position = m_vbo_vertex[2].position + axis_f.c;
        m_vbo_vertex[7].position = m_vbo_vertex[3].position + axis_f.c;



        m_vbo_indexes[0] = 0;   m_vbo_indexes[1] = 1;
        m_vbo_indexes[2] = 1;   m_vbo_indexes[3] = 3;
        m_vbo_indexes[4] = 3;   m_vbo_indexes[5] = 2;
        m_vbo_indexes[6] = 2;   m_vbo_indexes[7] = 0;

        m_vbo_indexes[8] = 4;   m_vbo_indexes[9] = 5;
        m_vbo_indexes[10] = 5;   m_vbo_indexes[11] = 7;
        m_vbo_indexes[12] = 7;   m_vbo_indexes[13] = 6;
        m_vbo_indexes[14] = 6;   m_vbo_indexes[15] = 4;

        m_vbo_indexes[16] = 0;   m_vbo_indexes[17] = 4;
        m_vbo_indexes[18] = 1;   m_vbo_indexes[19] = 5;
        m_vbo_indexes[20] = 2;   m_vbo_indexes[21] = 6;
        m_vbo_indexes[22] = 3;   m_vbo_indexes[23] = 7;

        for (int i = 0; i < vbo_v_count; i++) {
            m_vbo_vertex[i].color = color;
        }


        renderer.UpdateVertexBuffer(m_vbo_vertex, vbo_v_count, sizeof(VERTEX_F3UI1));	//頂点データをVRAMに転送
        renderer.UpdateIndexBuffer(m_vbo_indexes, vbo_i_count, sizeof(int));	//インデックスデータをVRAMに転送

        renderer.DrawIndexed(vbo_i_count, D3D_PRIMITIVE_TOPOLOGY_LINELIST);

    }


	void SetViewAndProjectionMatrix(const float* pViewMatrix, const float* pProjectionMatrix) {
		renderer.SetViewAndProjectionMatrix(pViewMatrix, pProjectionMatrix);		
	}
	
private:

	HRESULT InitShader() {

		// Define the input layout for Vertex Shader
		D3D11_INPUT_ELEMENT_DESC layout[] = {
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
			{ "COLOR", 0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
		};
		UINT numElements = ARRAYSIZE(layout);
		TCHAR filepath[MAX_PATH];
		GetAttachedFileName(filepath, _T("vs_point_color_to_ps.cso"));
		renderer.LoadVertexShader(filepath, "main", "vs_4_0", layout, numElements);

		//Pixel Shader-----------------------------------------
		GetAttachedFileName(filepath, _T("ps_point_color.cso"));
		renderer.LoadPixelShader(filepath, "main", "ps_4_0");

		return S_OK;
	}


};




#endif

