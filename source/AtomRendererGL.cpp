// AtomRendererGL.cpp: AtomRendererGL クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////

#include "AtomRendererGL.h"

#include "atomic_color.h"

#define _USE_MATH_DEFINES
#include <math.h>
#include <string.h>


#pragma warning(disable : 4996)


extern size_t g_max_vram_size;


#define		ATOM_RADIUS		(0.25)

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////



AtomRendererGL::AtomRendererGL() : 
	m_vram_size( 0 ),	
	m_prev_render_dat( NULL ),
	m_vbo_positions( NULL ),
	m_vbo_normals( NULL ),
	m_vbo_colors( NULL ),
	m_vbo_indexes( NULL ),
//	m_visual_atom( VISUAL_ATOM_SPHERE ),
	m_visual_bond( VISUAL_BOND_PIPE ),
	m_color_table( NULL ),
//	m_visual_atompoly( 4 ),
	m_sphere_pos( NULL ),
	m_sphere_nor( NULL ),
	m_sphere_idx( NULL ),
	m_bond_pos( NULL ),
	m_bond_nor( NULL ),
	m_bond_idx( NULL )
{
	
	memset(&m_prev_vis, 0, sizeof(VISUAL_SETTING));


}


AtomRendererGL::~AtomRendererGL(){

	delete [] m_vbo_positions;
	delete [] m_vbo_normals;
	delete [] m_vbo_colors;
	delete [] m_vbo_indexes;

	delete [] m_sphere_pos;
	delete [] m_sphere_nor;
	delete [] m_sphere_idx;
	delete [] m_bond_pos;
	delete [] m_bond_nor;
	delete [] m_bond_idx;
	delete [] m_color_table;

}


/////////////////////////////////////////////////////////////////
//
//Rendering by OpenGL
//
/////////////////////////////////////////////////////////////////


//VBOの設定
void AtomRendererGL::Draw(ATOMS_DATA* dat, BOND_INFO* bond, const VISUAL_SETTING& vis){	//int j_mode, float Jmax
	

	
	if (dat == NULL) return;
	

	//vboのセット
	int redraw_flag = 0;
	if((vis.atom_poly != m_prev_vis.atom_poly) || (vis.atom_radius != m_prev_vis.atom_radius)){
		//ポリゴンモデルの初期化
		InitSphere( vis.atom_radius, vis.atom_poly, vis.atom_poly*2);
		redraw_flag = 1;
	}

	//原子表示色のモード設定//
	//m_visual_atomcolor = vis.atom_color;
	if(vis.atom_color != m_prev_vis.atom_color){
		redraw_flag = 1;
	}

	if((vis.bond_poly != m_prev_vis.bond_poly)||(vis.bond != m_prev_vis.bond)){
		//ポリゴンモデルの初期化
		InitBond( vis.atom_radius * 0.25, vis.bond_poly);
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
	glDisable(GL_BLEND);
				
		glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
	glDisable(GL_CULL_FACE);
		
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);
	glEnableClientState(GL_COLOR_ARRAY);
	glEnable(GL_COLOR_MATERIAL);//頂点カラーを反映
	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		

	//原子球の描画//
	if(vis.atom){
		const int count = dat->pcnt;
		if(vis.bond){redraw_flag = 1;}

		
		//頂点数の計算//
		size_t num_draw_once = g_max_vram_size / (m_sphere_vcnt * (6 * sizeof(float) + sizeof(unsigned int)) + m_sphere_icnt * sizeof(int));
		if(count > num_draw_once){//頂点のバッファサイズを超える場合は複数回に分けて描画する//
			redraw_flag = 1;
		}
		
		size_t i_start = 0;
		while(i_start < count){

			if(i_start + num_draw_once > count){ num_draw_once = count - i_start;};

			
			if(redraw_flag){

				CreateSphere(i_start, num_draw_once, dat, bond);
			}
			
			
			if(m_vbo_icount){

				glVertexPointer(3, GL_FLOAT, 0, (float*)m_vbo_positions);
				glNormalPointer(GL_FLOAT, 0, (float*)m_vbo_normals);
				glColorPointer(4, GL_UNSIGNED_BYTE, 0, (BYTE*)m_vbo_colors);	//初期値は必ず4である
		
				glDrawElements(GL_TRIANGLES, m_vbo_icount, GL_UNSIGNED_INT, (GLuint*)m_vbo_indexes);
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

	
	//ボンドの描画//
	if(vis.bond){

		const int count = bond->count;

		if(vis.atom){redraw_flag = 1;}
		
		//頂点数の計算//
		size_t num_draw_once = g_max_vram_size / (m_bond_vcnt * (6 * sizeof(float) + sizeof(unsigned int)) + m_bond_icnt * sizeof(int));
		if(count > num_draw_once){//頂点のバッファサイズを超える場合は複数回に分けて描画する//
			redraw_flag = 1;
		}

		size_t i_start = 0;
		while(i_start < count){

			if(i_start + num_draw_once > count){ num_draw_once = count - i_start;};

			
			if(redraw_flag){

				CreateBond(i_start, num_draw_once, dat, bond);
			}
			

			if(m_vbo_icount){
	
				glVertexPointer(3, GL_FLOAT, 0, (float*)m_vbo_positions);
				glNormalPointer(GL_FLOAT, 0, (float*)m_vbo_normals);
				glColorPointer(4, GL_UNSIGNED_BYTE, 0, (BYTE*)m_vbo_colors);	//初期値は必ず4である
		
				glDrawElements(GL_TRIANGLES, m_vbo_icount, GL_UNSIGNED_INT, (GLuint*)m_vbo_indexes);
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
	glDisableClientState(GL_NORMAL_ARRAY);
	glDisableClientState(GL_COLOR_ARRAY);
	glDisable(GL_COLOR_MATERIAL);//頂点カラーを反映
	glEnable(GL_CULL_FACE);
	

	m_prev_render_dat = dat;
	m_prev_vis = vis;
}


void AtomRendererGL::CreateSphere(int start, int count, ATOMS_DATA* dat, BOND_INFO* bond){

	//vboサイズ
	int vbo_v_count = count * m_sphere_vcnt;
	int vbo_i_count = count * m_sphere_icnt;
	int need_vram_sz = vbo_v_count * (6 * sizeof(float) + sizeof(unsigned int)) + vbo_i_count * sizeof(int);

	
	if(m_vram_size < need_vram_sz){
		m_vram_size = need_vram_sz;
		delete [] m_vbo_positions;
		delete [] m_vbo_normals;
		delete [] m_vbo_colors;
		delete [] m_vbo_indexes;
		m_vbo_positions = new vec3f[vbo_v_count];
		m_vbo_normals = new vec3f[vbo_v_count];
		m_vbo_colors = new BYTE[vbo_v_count*4];
		m_vbo_indexes = new int[vbo_i_count];

	}





	m_vbo_vcount = 0;
	m_vbo_icount = 0;
	
	//原子(球)のセット//
	const int i_end = start + count;
	for(int i = start; i < i_end; i++){
		if(m_color_table[i] & 0xFF000000){	//完全透過は表示しない//
			SetSphere(dat->r[i], m_color_table[i],
						m_vbo_vcount,	//頂点番号のoffset
						m_vbo_positions + m_vbo_vcount,
						m_vbo_normals + m_vbo_vcount,
						m_vbo_colors + m_vbo_vcount * 4,
						m_vbo_indexes + m_vbo_icount);

			m_vbo_vcount += m_sphere_vcnt;
			m_vbo_icount += m_sphere_icnt;
		}
	}


	
}


void AtomRendererGL::CreateBond(int start, int count, ATOMS_DATA* dat, BOND_INFO* bond){

	
	//vboサイズ
	int vbo_v_count = count * m_bond_vcnt;
	int vbo_i_count = count * m_bond_icnt;
	int need_vram_sz = vbo_v_count * (6 * sizeof(float) + sizeof(unsigned int)) + vbo_i_count * sizeof(int);

	
	if(m_vram_size < need_vram_sz){
		m_vram_size = need_vram_sz;

		
		delete [] m_vbo_positions;
		delete [] m_vbo_normals;
		delete [] m_vbo_colors;
		delete [] m_vbo_indexes;
		m_vbo_positions = new vec3f[vbo_v_count];
		m_vbo_normals = new vec3f[vbo_v_count];
		m_vbo_colors = new BYTE[vbo_v_count*4];
		m_vbo_indexes = new int[vbo_i_count];

	}



	m_vbo_vcount = 0;
	m_vbo_icount = 0;
	

	//ボンドのセット
	const VECTOR_IJ* b = bond->bvector;
	const int i_end = start + count;
	for (int i = start; i < i_end; i++){
		int li = b[i].i;
		int lj = b[i].j;
				
		if((m_color_table[li] & m_color_table[lj]) & 0xFF000000){	//完全透過は表示しない//
			
			SetBond2(dat->r[li], b[i].v, 
						m_color_table[li],
						m_color_table[lj],
						m_vbo_vcount,
						m_vbo_positions + m_vbo_vcount,
						m_vbo_normals + m_vbo_vcount,
						m_vbo_colors + m_vbo_vcount*4,
						m_vbo_indexes + m_vbo_icount);
			
			m_vbo_vcount += m_bond_vcnt;
			m_vbo_icount += m_bond_icnt;
		}
	}
		
	
}


///////////////////////////////////////////////
//
//VBOにポリゴンをセットする機能
//
////////////////////////////////////////////////
	
void AtomRendererGL::InitSphere(float radius, int slices, int stacks){
	int i, k;
	double daXY = 2.0*M_PI/double(stacks);
	double daZX = M_PI/double(slices);
	double angleZX;
	double angleXY;
	float cosZX, sinZX;
	float *cosXY, *sinXY;
	

	
	cosXY = new float[stacks];
	sinXY = new float[stacks];
	
	angleXY = daXY;
	cosXY[0] = 1.f;
	sinXY[0] = 0.f;
	for (i = 1; i < stacks; i++){
		cosXY[i] = (float)cos(angleXY);
		sinXY[i] = (float)sin(angleXY);
		angleXY += daXY;
	}

	m_sphere_vcnt = (slices - 1) * stacks + 2;
	delete [] m_sphere_pos;
	delete [] m_sphere_nor;
	vec3f *pos = m_sphere_pos = new vec3f[m_sphere_vcnt];
	vec3f *nor = m_sphere_nor = new vec3f[m_sphere_vcnt];


	nor->Set(0.0f, 0.0f, 1.0f);
	nor++;
	pos->Set(0.0f,0.0f,(float)radius);
	pos++;

	angleZX = daZX;
	for (i = 1; i < slices; i++){
		cosZX = (float)cos(angleZX);
		sinZX = (float)sin(angleZX);

		for (k = 0; k < stacks; k++){
			
			nor->Set(sinZX * cosXY[k], sinZX * sinXY[k], cosZX);
			pos->x = nor->x * radius;
			pos->y = nor->y * radius;
			pos->z = nor->z * radius;

			nor++;
			pos++;
		}
		angleZX += daZX;
	}
	
	nor->Set(0.0f, 0.0f, -1.0f);
	pos->Set(0.0f,0.0f, -radius);
	
	delete [] cosXY;
	delete [] sinXY;


	//インデックス配列のセット
	m_sphere_icnt = (slices - 1)*stacks*6;
	delete [] m_sphere_idx;
	int* idx = m_sphere_idx = new int[m_sphere_icnt];
	int offset = 1;
	for(i=0; i < stacks; i++){
		*idx = 0;
		idx++;
		*idx = i + offset;
		idx++;
		*idx = ((i + 1) % stacks) + offset;
		idx++;
	}
	//offset += stacks;
	
	
	for (k = 0; k < slices - 2; k++){
		for(i=0; i < stacks; i++){
			*idx = i + offset;
			idx++;
			*(idx + 3) = *idx = i + offset + stacks;
			idx++;
			*(idx + 1) = *idx = ((i + 1) % stacks) + offset;
			idx+=3;
			*idx = ((i + 1) % stacks) + offset + stacks;
			idx++;
		}
		offset += stacks;
	}

	for(i=0; i < stacks; i++){
		*idx = i + offset;
		idx++;
		*idx = m_sphere_vcnt - 1;
		idx++;
		*idx = ((i + 1) % stacks) + offset;
		idx++;
	}
	
};

inline void AtomRendererGL::SetSphere(vec3f& orgin, unsigned int onecolor, int index_offset, vec3f* positions, vec3f* normals, BYTE* colors, int* indexes){
	int i;
	unsigned int *cols = (unsigned int*)colors;
	
	for( i = 0; i < m_sphere_vcnt; i++){
		positions[i] = m_sphere_pos[i] + orgin;
		*cols = onecolor;
		cols++;		
	}
	
	memcpy(normals, m_sphere_nor, m_sphere_vcnt * sizeof(vec3f));
	
	for( i = 0; i < m_sphere_icnt; i++){
		indexes[i] = m_sphere_idx[i] + index_offset;
	}	

}



void AtomRendererGL::InitBond(float radius, int stacks){
	int i, k;
	double daXY = 2.0*M_PI/double(stacks);
	double angleXY;
	float *cosXY, *sinXY;
	

	
	cosXY = new float[stacks];
	sinXY = new float[stacks];
	
	angleXY = daXY;
	cosXY[0] = 1.f;
	sinXY[0] = 0.f;
	for (i = 1; i < stacks; i++){
		cosXY[i] = (float)cos(angleXY);
		sinXY[i] = (float)sin(angleXY);
		angleXY += daXY;
	}

	m_bond_vcnt = stacks * 2;
	vec3f *pos = m_bond_pos = new vec3f[m_bond_vcnt];
	vec3f *nor = m_bond_nor = new vec3f[m_bond_vcnt];


	for (k = 0; k < stacks; k++){
			
		nor->Set(cosXY[k], sinXY[k], 0.f);
		pos->x = nor->x * radius;
		pos->y = nor->y * radius;
		pos->z = nor->z * radius;
		
		nor++;
		pos++;
		
	}

	memcpy(nor, m_bond_nor, sizeof(vec3f)*stacks);
	memcpy(pos, m_bond_pos, sizeof(vec3f)*stacks);

	for (k = 0; k < stacks; k++){
		pos->z += 1.f;
		pos++;
	}

	delete [] cosXY;
	delete [] sinXY;


	//インデックス配列のセット
	m_bond_icnt = stacks * 6;
	int* idx = m_bond_idx = new int[m_bond_icnt];
	
	for(i=0; i < stacks; i++){
		*idx = i;
		idx++;
		*(idx + 3) = *idx = ((i + 1) % stacks);
		idx++;
		*(idx + 1) = *idx = i + stacks;
		idx+=3;
		*idx = ((i + 1) % stacks) + stacks;
		idx++;	
	}		
	
	
	
};


void AtomRendererGL::InitCone(float radius, int stacks){
	int k;

	InitBond(radius, stacks);
	vec3f *pos = m_bond_pos;
	for (k = 0; k < stacks; k++){
			
		pos->x *= 3.f;
		pos->y *= 3.f;
		pos->z = ATOM_RADIUS;
		pos++;
		
	}

	for (k = 0; k < stacks; k++){
		pos->x = 0.f;
		pos->y = 0.f;
		pos->z -= ATOM_RADIUS;
		pos++;
	}

};

inline void AtomRendererGL::SetBond(vec3f& v1, vec3f& v2, unsigned int onecolor, unsigned int othercolor, int index_offset, vec3f* positions, vec3f* normals, BYTE* colors, int* indexes){
	int i;
	//int m[16];

	unsigned int *cols = (unsigned int*)colors;
	

	vec3f axis = v2 - v1;
	double dlength = (double)(axis.x * axis.x + axis.y * axis.y);
	float sxy = (float)sqrt(dlength);
	float flength = (float)sqrt((dlength + (double)(axis.z * axis.z)));
	//flength = 

	//	memcpy(positions, m_bond_pos, m_bond_vcnt * sizeof(vec3f));
	//memcpy(normals, m_bond_nor, m_bond_vcnt * sizeof(vec3f));

	sxy /= flength;
	if (sxy < 0.00001f){	//下向き軸への例外処理
		if(axis.z < -0.f){
			for( i = 0; i < m_bond_vcnt; i++){
				positions[i].x = -m_bond_pos[i].x  + v1.x;
				positions[i].y = m_bond_pos[i].y  + v1.y;
				positions[i].z = -m_bond_pos[i].z * flength + v1.z;
				normals[i].x = -m_bond_nor[i].x;
				normals[i].y = m_bond_nor[i].y;
				normals[i].z = -m_bond_nor[i].z;
			}
		}else{
			//memcpy(positions, m_bond_pos, m_bond_vcnt * sizeof(vec3f));
			memcpy(normals, m_bond_nor, m_bond_vcnt * sizeof(vec3f));
			for( i = 0; i < m_bond_vcnt; i++){
				positions[i].x = m_bond_pos[i].x  + v1.x;
				positions[i].y = m_bond_pos[i].y  + v1.y;
				positions[i].z = m_bond_pos[i].z * flength + v1.z;
				
			}
		}
	}else{
		vec3f m1, m2, m3;
		//axis *= 1.f/flength;
		float xdsxy = axis.x / (sxy * flength); 
		float ydsxy = axis.y / (sxy * flength);
		m1.Set(axis.z / flength * xdsxy, -ydsxy, axis.x);
		m2.Set(axis.z / flength * ydsxy, xdsxy, axis.y);
		m3.Set(-sxy, 0.f, axis.z);

		for( i = 0; i < m_bond_vcnt; i++){
			positions[i].x = m1 * m_bond_pos[i] + v1.x;
			positions[i].y = m2 * m_bond_pos[i] + v1.y;
			positions[i].z = m3 * m_bond_pos[i] + v1.z;
			normals[i].x = m1 * m_bond_nor[i];
			normals[i].y = m2 * m_bond_nor[i];
			normals[i].z = m3 * m_bond_nor[i];
		}
	}

	//colorのセット	
	for( i = 0; i < m_bond_vcnt / 2; i++){
		*cols = onecolor;
		cols++;
	}

	for( i = 0; i < m_bond_vcnt / 2; i++){
		*cols = othercolor;
		cols++;
	}
		

	//インデックスのセット	
	for( i = 0; i < m_bond_icnt; i++){
		indexes[i] = m_bond_idx[i] + index_offset;
	}
		
	
	memcpy(normals, m_bond_nor, m_bond_vcnt * sizeof(vec3f));
	
	
}


inline void AtomRendererGL::SetBond2(const vec3f& v1, const vec3f& dv, unsigned int onecolor, unsigned int othercolor, int index_offset, vec3f* positions, vec3f* normals, BYTE* colors, int* indexes){
	int i;
	//int m[16];

	unsigned int *cols = (unsigned int*)colors;
	

	vec3f ax;
	vec3f ay;
	ax.Set(dv.y, -dv.x, 0.0);
	float len = ax * ax;
	if(len < 0.00001){
		ax.Set(1.0f, 0.0f, 0.0f);
		ay.Set(0.0f, 1.0f, 0.0f);
	}else{
		ax /= sqrt(len);
		ay = Cross(dv, ax);
		len = ay*ay;
		ay /= sqrt(len);
	}
	
		
	for( i = 0; i < m_bond_vcnt; i++){
		positions[i] = (ax * m_bond_pos[i].x) + (ay * m_bond_pos[i].y) + (dv * m_bond_pos[i].z) + v1;
		normals[i] = ax * m_bond_nor[i].x + ay * m_bond_nor[i].y + dv * m_bond_nor[i].z;
	}
	

	//colorのセット	
	for( i = 0; i < m_bond_vcnt / 2; i++){
		*cols = onecolor;
		cols++;
	}

	for( i = 0; i < m_bond_vcnt / 2; i++){
		*cols = othercolor;
		cols++;
	}
		

	//インデックスのセット	
	for( i = 0; i < m_bond_icnt; i++){
		indexes[i] = m_bond_idx[i] + index_offset;
	}
		
	
	memcpy(normals, m_bond_nor, m_bond_vcnt * sizeof(vec3f));
	
	
}

//原子番号を元にカラーを計算
void AtomRendererGL::CheckColorByAtomKind( ATOMS_DATA* dat){
	
	

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
void AtomRendererGL::CheckColorByBondNum2( ATOMS_DATA* dat, BOND_INFO* bond){
	
	

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
void AtomRendererGL::CheckColorByPressure( ATOMS_DATA* dat, BOND_INFO* bond){
	
	

	delete [] m_color_table;
	m_color_table = new unsigned int[dat->pcnt];
	memset(m_color_table, 0, sizeof(unsigned int) * dat->pcnt);

	//bond数を計算
	
	//色を参照
	for(int i = 0; i < dat->pcnt; i++){
		m_color_table[i] = bond->coords[i];
	}
}

void AtomRendererGL::ClearVRAM(){
	m_prev_render_dat = NULL;

}
