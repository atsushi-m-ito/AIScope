// LMCRendererGL.h: LMCRendererGL クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#ifndef LMCRendererGL_h
#define LMCRendererGL_h

#pragma once

#include <stdio.h>

#include "LMC_Holder.h"
#include "visual.h"

#include "sdkwgl.h"
#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "mod.h"


#include "visual.h"

#define KIND_ALL (255)


class LMCRendererGL  
{
private:



//OpenGL用頂点データ
	int m_vbo_vcount;
	vec3f* m_vbo_positions;
	vec3f* m_vbo_normals;
	BYTE* m_vbo_colors;
	int m_vbo_icount;
	int* m_vbo_indexes;
	unsigned int m_index_type;

	
	VISUAL_SETTING m_prev_vis;
	int m_prev_step;	//前回レンダリングしたデータ
	
//ポリゴンモデル用のバッファ
	vec3f* m_sphere_pos;
	vec3f* m_sphere_nor;
	int* m_sphere_idx;
	int m_sphere_vcnt, m_sphere_icnt;
	void InitSphere(double radius, int slices, int stacks);
	void SetSphere(vec3f& orgin, unsigned int onecolor, int index_offset, vec3f* positions, vec3f* normals, BYTE* colors, int* indexes);
	void CreateSphereArray(LMC_INT* dat, int grid_x, int grid_y, int grid_z, int num_elements, float* boxaxis );

	int CountValidGridData( LMC_INT* dat, int num_grid, int num_elements);

	

public:

	LMCRendererGL();
	virtual ~LMCRendererGL();


	void Draw(LMC_INT* dat, int grid_x, int grid_y, int grid_z, int num_elements, float* boxaxis, VISUAL_SETTING* vis, int step);

	//void SetMode(int type, int mode, ATOMS_DATA* dat);
	//void ClearVRAM();	


	
//	void SetViewMatrixPointer(mat44f* pViewMatrix );
//	void SetProjectionMatrixPointer(mat44f* pProjectionMatrix );

	
};





#endif // LMCRendererGL

