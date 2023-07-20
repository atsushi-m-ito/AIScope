// AtomRendererGL.h: AtomRendererGL クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#ifndef AtomRendererGL_h
#define AtomRendererGL_h

#pragma once


#include "MDLoader.h"	//ATOMS_DATAのためだけに必要//
#include "BOND_INFO.h"

#include "sdkwgl.h"
#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "io64.h"
#include "mod.h"


#include "visual.h"



class AtomRendererGL  
{
private:



//OpenGL用頂点データ
	int m_vram_size;
	int m_vbo_vcount;
	vec3f* m_vbo_positions;
	vec3f* m_vbo_normals;
	BYTE* m_vbo_colors;
	int m_vbo_icount;
	int* m_vbo_indexes;
	unsigned int m_index_type;

	unsigned int *m_color_table;	//原子ごとの色が入る

	ATOMS_DATA* m_prev_render_dat;	//前回レンダリングしたデータ
	//void CreateSphereArray(ATOMS_DATA* dat, BOND_INFO* bond, int show_atom, int show_bond, int atom_color);
	void CreateSphere(int start, int count, ATOMS_DATA* dat, BOND_INFO* bond);
	void CreateBond(int start, int count, ATOMS_DATA* dat, BOND_INFO* bond);


//ポリゴンモデル用のバッファ

	vec3f* m_sphere_pos;
	vec3f* m_sphere_nor;
	int* m_sphere_idx;
	int m_sphere_vcnt, m_sphere_icnt;
	void InitSphere(float radius, int slices, int stacks);
	void SetSphere(vec3f& orgin, unsigned int onecolor, int index_offset, vec3f* positions, vec3f* normals, BYTE* colors, int* indexes);
	
	
	vec3f* m_bond_pos;
	vec3f* m_bond_nor;
	int* m_bond_idx;
	int m_bond_vcnt, m_bond_icnt;
	void InitBond(float radius, int stacks);
	void InitCone(float radius, int stacks);
	void SetBond(vec3f& v1, vec3f& v2, unsigned int onecolor, unsigned int othercolor, int index_offset, vec3f* positions, vec3f* normals, BYTE* colors, int* indexes);
	void SetBond2(const vec3f& v1, const vec3f& dv, unsigned int onecolor, unsigned int othercolor, int index_offset, vec3f* positions, vec3f* normals, BYTE* colors, int* indexes);

	//void CheckColorByBondNum( ATOMS_DATA* dat);
	void CheckColorByBondNum2( ATOMS_DATA* dat, BOND_INFO* bond);
	void CheckColorByAtomKind( ATOMS_DATA* dat);
	void CheckColorByPressure( ATOMS_DATA* dat, BOND_INFO* bond );
	
	//表示モード
//	int m_visual_atom;	//原子の表示サイズ
//	int m_visual_atomcolor;	//原子の色(原子番号Z or ボンド数)
//	int m_visual_atompoly;	//原子のポリゴン数(緯度方向の分割数。2以上。経度方向の分割数は緯度方向の倍になる)

	VISUAL_SETTING m_prev_vis;

	int m_visual_bond;	//ボンドの表示種類
	



//ROWDOG///////////////////////
	/*
	void UpdateVertexBuffer(LPVOID pBuffer, UINT count, UINT stride);
	void UpdateIndexBuffer(LPVOID pBuffer, UINT count, UINT stride);
	virtual void Draw(UINT index_count, vec3f* position);
*/
	float Jmax;

public:
	
	AtomRendererGL();
	virtual ~AtomRendererGL();


	void Draw(ATOMS_DATA* dat, BOND_INFO* bond, const VISUAL_SETTING& vis);

	void SetMode(int type, int mode, ATOMS_DATA* dat);
	void ClearVRAM();	

	//void SetParam(int type, void* params);
	
//	void SetViewMatrixPointer(mat44f* pViewMatrix );
//	void SetProjectionMatrixPointer(mat44f* pProjectionMatrix );

	
};



#endif // AtomRendererGL

