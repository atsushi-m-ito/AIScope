// PointRendererGL.h: PointRendererGL クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#ifndef PointRendererGL_h
#define PointRendererGL_h

#pragma once


#include "MDLoader.h"	//ATOMS_DATAのためだけに必要//
#include "BOND_INFO.h"

#include "sdkwgl.h"
#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "io64.h"
//#include "mod.h"



#include "visual.h"



class PointRendererGL  
{
private:



//OpenGL用頂点データ
	int m_vram_size;
	int m_vbo_vcount;
	vec3f* m_vbo_positions;
	BYTE* m_vbo_colors;
	
	unsigned int *m_color_table;	//原子ごとの色が入る

	ATOMS_DATA* m_prev_render_dat;	//前回レンダリングしたデータ
	//void CreateSphereArray(ATOMS_DATA* dat, BOND_INFO* bond, int show_atom, int show_bond, int visual_atomcolor);
	void CreateSphere(int start, int count, ATOMS_DATA* dat, BOND_INFO* bond);
	
	
	void CheckColorByBondNum( ATOMS_DATA* dat);
	void CheckColorByBondNum2( ATOMS_DATA* dat, BOND_INFO* bond);
	void CheckColorByAtomKind( ATOMS_DATA* dat);
	void CheckColorByPressure( ATOMS_DATA* dat, BOND_INFO* bond );
	
	//表示モード
//	int m_visual_atom;	//原子の表示サイズ

//	int m_visual_atompoly;	//原子のポリゴン数(緯度方向の分割数。2以上。経度方向の分割数は緯度方向の倍になる)

	VISUAL_SETTING m_prev_vis;




//ROWDOG///////////////////////
	/*
	void UpdateVertexBuffer(LPVOID pBuffer, UINT count, UINT stride);
	void UpdateIndexBuffer(LPVOID pBuffer, UINT count, UINT stride);
	virtual void Draw(UINT index_count, vec3f* position);
*/
	float Jmax;

public:
	
	PointRendererGL();
	virtual ~PointRendererGL();


	void Draw(ATOMS_DATA* dat, BOND_INFO* bond, const VISUAL_SETTING& vis);

	void SetMode(int type, int mode, ATOMS_DATA* dat);
	void ClearVRAM();	

	
};



#endif // PointRendererGL

