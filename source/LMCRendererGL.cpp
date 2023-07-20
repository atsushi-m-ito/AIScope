
#define _USE_MATH_DEFINES

#include <string.h>
#include <math.h>
#include "LMCRendererGL.h"


static const BYTE atom_colors[] = { 51, 205, 205, 255,	//C_custom
						255, 255, 255, 255,	//H
						255, 240, 30, 255, //He
						255, 20, 151, 255, //Li
						255, 20, 151, 255, //Be
						255, 20, 151, 255, //B
						51, 205, 205, 255, //C
						143, 143, 255, 255, //N
						255, 0, 0, 255, //O
						255, 20, 151, 255, //F
						255, 240, 30, 255, //Ne
};


LMCRendererGL::LMCRendererGL() :
	m_vbo_positions( NULL ),
	m_vbo_normals( NULL ),
	m_vbo_colors( NULL ),
	m_vbo_indexes( NULL ),
	m_sphere_pos( NULL ),
	m_sphere_nor( NULL ),
	m_sphere_idx( NULL )
{
	
	memset(&m_prev_vis, 0, sizeof(VISUAL_SETTING));


}


LMCRendererGL::~LMCRendererGL(){

	delete [] m_vbo_positions;
	delete [] m_vbo_normals;
	delete [] m_vbo_colors;
	delete [] m_vbo_indexes;

	delete [] m_sphere_pos;
	delete [] m_sphere_nor;
	delete [] m_sphere_idx;

}



//VBOの設定
void LMCRendererGL::Draw(LMC_INT* dat, int grid_x, int grid_y, int grid_z, int num_elements, float* boxaxis, VISUAL_SETTING* vis, int step){	//int j_mode, float Jmax
	

	
	if (dat == NULL) return;
	
/*
	//ポイント描画.
	if (vis->atom == VISUAL_ATOM_POINT){

		//vboセット(カラーのみ)
		if(dat != m_prev_render_dat){
			m_prev_render_dat = dat;
			delete [] m_vbo_colors;
			m_vbo_colors = new BYTE[dat->pcnt * 4];

			UINT* v = (UINT*)m_vbo_colors;
			for(int i = 0; i < dat->pcnt; i++){
				v[i] = 0x4DB3B3B3;				
			}
			//memset(m_vbo_colors, 255, dat->pcnt * 4);
		}


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
		
					//glBindBuffer(GL_ARRAY_BUFFER, 0);
					//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		glVertexPointer(3, GL_FLOAT, 0, (float*)(dat->r));
		//glNormalPointer(GL_FLOAT, 0, (float*)m_vbo_normals);
		glColorPointer(4, GL_UNSIGNED_BYTE, 0, (BYTE*)m_vbo_colors);	//初期値は必ず4である
		
		glDrawArrays(GL_POINTS, 0, dat->pcnt);
		GLenum err = glGetError();
		if(err){
			char buf[1024];
			sprintf(buf, "%s", gluErrorString(err));
			printf("%s\n", buf);
		}

		glFlush();
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




	//ポリゴン描画.
	}else{
	*/
	if(vis->atom == VISUAL_ATOM_SPHERE){
		//vboのセット
		int redraw_flag = 0;
		if((vis->atom_poly != m_prev_vis.atom_poly) || (vis->atom_radius != m_prev_vis.atom_radius)){
			//ポリゴンモデルの初期化
			InitSphere( vis->atom_radius, vis->atom_poly, vis->atom_poly*2);
			redraw_flag = 1;
		}

		if((step != m_prev_step) || (redraw_flag)){
			CreateSphereArray(dat, grid_x, grid_y, grid_z, num_elements, boxaxis);
			m_prev_step = step;
		}
		
		glDisable(GL_BLEND);
				
			glColor4f( 1.0f, 1.0f, 1.0f, 1.0f );
		glDisable(GL_CULL_FACE);
		
		glEnableClientState(GL_VERTEX_ARRAY);
		glEnableClientState(GL_NORMAL_ARRAY);
		glEnableClientState(GL_COLOR_ARRAY);
		glEnable(GL_COLOR_MATERIAL);//頂点カラーを反映
		glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
		
					//glBindBuffer(GL_ARRAY_BUFFER, 0);
					//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

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
		glDisableClientState(GL_VERTEX_ARRAY);
		glDisableClientState(GL_NORMAL_ARRAY);
		glDisableClientState(GL_COLOR_ARRAY);
		glDisable(GL_COLOR_MATERIAL);//頂点カラーを反映
		glEnable(GL_CULL_FACE);

		m_prev_vis = *vis;
	}
}


void LMCRendererGL::CreateSphereArray(LMC_INT* dat, int grid_x, int grid_y, int grid_z, int num_elements, float* boxaxis ){


	//vboサイズ
	int num_grid = grid_x * grid_y * grid_z;	
	int num_ptcl = CountValidGridData(dat, num_grid, num_elements);

	int vbo_v_sz = num_ptcl * m_sphere_vcnt;
	int vbo_i_sz = num_ptcl * m_sphere_icnt;

	delete [] m_vbo_positions;
	m_vbo_positions = new vec3f[vbo_v_sz];
	delete [] m_vbo_normals;
	m_vbo_normals = new vec3f[vbo_v_sz];
	delete [] m_vbo_colors;
	m_vbo_colors = new BYTE[vbo_v_sz*4];
	delete [] m_vbo_indexes;
	m_vbo_indexes = new int[vbo_i_sz];

	m_vbo_vcount = 0;
	m_vbo_icount = 0;

	

	vec3f latticesize;
	latticesize.Set( boxaxis[0] / (float)grid_x, boxaxis[4] / (float)grid_y, boxaxis[8] / (float)grid_z );
	
	//原子(球)のセット
	vec3f r0;
	for(int iz = 0; iz < grid_z; iz++){
		r0.z = latticesize.z * (float)iz;
		for(int iy = 0; iy < grid_y; iy++){
			r0.y = latticesize.y * (float)iy;
			int idx = grid_x * (iy + grid_y * iz);
			for(int ix = 0; ix < grid_x; ix++){
				
				for(int k = 0; k < num_elements; k++){
					if(dat[(idx+ix)*num_elements + k]){

						r0.x = latticesize.x * (float)ix;

						SetSphere(r0,
							*(((unsigned int*)atom_colors) + k),	//color//
							m_vbo_vcount,	//頂点番号のoffset
							m_vbo_positions + m_vbo_vcount,
							m_vbo_normals + m_vbo_vcount,
							m_vbo_colors + m_vbo_vcount * 4,
							m_vbo_indexes + m_vbo_icount);
			
						m_vbo_vcount += m_sphere_vcnt;
						m_vbo_icount += m_sphere_icnt;
						break;
					}
				}
			}
		}
	}

	
}


void LMCRendererGL::InitSphere(double radius, int slices, int stacks){
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


//原子番号を元にカラーを計算
//表示する原子数を返す//
int LMCRendererGL::CountValidGridData( LMC_INT* dat, int num_grid, int num_elements){
	
	

	//delete [] m_color_table;
	//m_color_table = new unsigned int[num_grid];
	
	//色を参照
	int num = 0;
	
	for(int i = 0; i < num_grid; i++){
		for(int k = 0; k < num_elements; k++){
			if(dat[i*num_elements + k]){
				//m_color_table[i] = *(((unsigned int*)atom_colors) + k);
				num++;
				break;
			}
		}
	}

	return num;
}


inline void LMCRendererGL::SetSphere(vec3f& orgin, unsigned int onecolor, int index_offset, vec3f* positions, vec3f* normals, BYTE* colors, int* indexes){
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

