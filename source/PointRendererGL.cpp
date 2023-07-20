// PointRendererGL.cpp: PointRendererGL クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "PointRendererGL.h"

#include "atomic_color.h"

#define _USE_MATH_DEFINES
#include <math.h>

#include <string.h>


extern size_t g_max_vram_size;


PointRendererGL::PointRendererGL() : 
	m_vram_size( 0 ),	
	m_prev_render_dat( NULL ),
	m_vbo_positions( NULL ),
	m_vbo_colors( NULL ),
	m_color_table( NULL )	
{
	
	memset(&m_prev_vis, 0, sizeof(VISUAL_SETTING));


}


PointRendererGL::~PointRendererGL(){

	delete [] m_vbo_positions;
	delete [] m_vbo_colors;
	
	delete [] m_color_table;

}


/////////////////////////////////////////////////////////////////
//
//Rendering by OpenGL
//
/////////////////////////////////////////////////////////////////


//VBOの設定
void PointRendererGL::Draw(ATOMS_DATA* dat, BOND_INFO* bond, const VISUAL_SETTING& vis){	//int j_mode, float Jmax
	

	
	if (dat == NULL) return;
	
	
	//vboのセット
	int redraw_flag = 0;
	if(vis.atom_poly != m_prev_vis.atom_poly){
		redraw_flag = 1;
	}

	//原子表示色のモード設定//
	if(vis.atom_color != m_prev_vis.atom_color){
		redraw_flag = 1;
	}

	if(dat != m_prev_render_dat){
		redraw_flag = 1;
	}

	
	//カラーテーブル作り
	if(vis.atom_color ==  VISUAL_ATOMCOLOR_BONDNUM){
		//if(dat->bondInsideCnt == -1){
			CheckColorByBondNum2(dat, bond);
		//}else{
		//	CheckColorByBondNum(dat);
		//}
	}else if(vis.atom_color ==  VISUAL_ATOMCOLOR_PRESSURE){
		//if(dat->bondInsideCnt == -1){
			CheckColorByPressure(dat, bond);
		//}else{
		//	CheckColorByBondNum(dat);
		//}
	}else{	//MODE_COLOR_ATOMIC
		CheckColorByAtomKind(dat);
	}



	
	//GL描画の前処理//
	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_ALPHA_TEST);

	glPointSize(2.0);
	//glColor4f( 0.7f, 0.7f, 0.7f, 1.0f );
	glDisable(GL_CULL_FACE);
		
	glEnableClientState(GL_VERTEX_ARRAY);
	//glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_COLOR_MATERIAL);//頂点カラーを反映
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		
	
	//原子球の描画//
	if(vis.atom){
		const int count = dat->pcnt;
		
		//頂点数の計算//
		size_t num_draw_once = g_max_vram_size / (3 * sizeof(float) + sizeof(unsigned int));
		if(count > num_draw_once){//頂点のバッファサイズを超える場合は複数回に分けて描画する//
			redraw_flag = 1;
		}

		
		size_t i_start = 0;
		while(i_start < count){

			if(i_start + num_draw_once > count){ num_draw_once = count - i_start;};

			
			if(redraw_flag){

				CreateSphere(i_start, num_draw_once, dat, bond);
			}
			
			if(m_vbo_vcount){
			
				glVertexPointer(3, GL_FLOAT, 0, (float*)m_vbo_positions);
				//glNormalPointer(GL_FLOAT, 0, (float*)m_vbo_normals);
				glColorPointer(4, GL_UNSIGNED_BYTE, 0, (BYTE*)m_vbo_colors);	//初期値は必ず4である
		
				glDrawArrays(GL_POINTS, 0, m_vbo_vcount);
				GLenum err = glGetError();
				if(err){
					char buf[1024];
					sprintf(buf, "%s", gluErrorString(err));
					printf("%s\n", buf);
				}

				glFlush();
			}

			
			i_start += num_draw_once;
		}
	}

	
	//GL描画の後処理//
	glDisableClientState(GL_VERTEX_ARRAY);
	//glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisable(GL_COLOR_MATERIAL);//頂点カラーを反映
	//glEnable(GL_CULL_FACE);
		
	glPointSize(1.0);

	glDisable(GL_ALPHA_TEST);
	glDisable(GL_BLEND);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	
	m_prev_render_dat = dat;
	m_prev_vis = vis;
}


void PointRendererGL::CreateSphere(int start, int count, ATOMS_DATA* dat, BOND_INFO* bond){

	
	//vboサイズ
	int vbo_v_count = count;
	int need_vram_sz = vbo_v_count * (3 * sizeof(float) + sizeof(unsigned int));

	
	
	if(m_vram_size < need_vram_sz){
		m_vram_size = need_vram_sz;
		delete [] m_vbo_positions;
		delete [] m_vbo_colors;
		m_vbo_positions = new vec3f[vbo_v_count];
		m_vbo_colors = new BYTE[vbo_v_count*4];
		
	}
	

	m_vbo_vcount = 0;
	
	//原子(球)のセット//
	const int i_end = start + count;
	for(int i = start; i < i_end; i++){
		if(m_color_table[i] & 0xFF000000){	//完全透過は表示しない//
			m_vbo_positions[m_vbo_vcount] = dat->r[i];
			memcpy(m_vbo_colors + m_vbo_vcount * 4, m_color_table + i, 4);

			m_vbo_vcount ++;
		}
	}


	
}

///////////////////////////////////////////////
//
//VBOにポリゴンをセットする機能
//
////////////////////////////////////////////////
	

//原子番号を元にカラーを計算
void PointRendererGL::CheckColorByAtomKind( ATOMS_DATA* dat){
	
	

	delete [] m_color_table;
	m_color_table = new unsigned int[dat->pcnt];
	
	//色を参照
    if (dat->knd){
        for (int i = 0; i < dat->pcnt; i++){
            m_color_table[i] = *(((unsigned int*)atom_colors) + dat->knd[i]);
        }
    } else{
        for (int i = 0; i < dat->pcnt; i++){
            m_color_table[i] = *(((unsigned int*)atom_colors));
        }
    }
}


//ボンド数を元にカラーを計算
void PointRendererGL::CheckColorByBondNum2( ATOMS_DATA* dat, BOND_INFO* bond){
	
	

	delete [] m_color_table;
	m_color_table = new unsigned int[dat->pcnt];
	memset(m_color_table, 0, sizeof(unsigned int) * dat->pcnt);

	//bond数を計算
	
	//色を参照
	for(int i = 0; i < dat->pcnt; i++){
		if(bond->coords[i] >= COLOR_LIMIT_BOND){
			m_color_table[i] = *(((unsigned int*)bondnum_colors) + COLOR_LIMIT_BOND - 1);
		}else{
			m_color_table[i] = *(((unsigned int*)bondnum_colors) + bond->coords[i]);
		}
	}
}


//ボンド数を元にカラーを計算
void PointRendererGL::CheckColorByPressure( ATOMS_DATA* dat, BOND_INFO* bond){
	
	

	delete [] m_color_table;
	m_color_table = new unsigned int[dat->pcnt];
	memset(m_color_table, 0, sizeof(unsigned int) * dat->pcnt);

	//bond数を計算
	
	//色を参照
	for(int i = 0; i < dat->pcnt; i++){
		m_color_table[i] = bond->coords[i];
	}
}

void PointRendererGL::ClearVRAM(){
	m_prev_render_dat = NULL;

}