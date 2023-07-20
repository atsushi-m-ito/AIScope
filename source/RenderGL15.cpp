
#include <stdio.h>
#include <string.h>

#include "targetver.h"
#include <windows.h>
#include <tchar.h>
#include <GL/gl.h>		//OpenGL利用に必要<br />
#include <GL/glu.h>		//gluPerspectiveを使うためにインクルード

#pragma comment( lib , "opengl32.lib" )	//OpenGL用ライブラリをリンク
#pragma comment( lib , "glu32.lib" )	//glu関数用ライブラリをリンク

#include "RenderGL15.h"

//#include "filelist.h"
//#include "camera.h"
#include "RenderingProperty.h"



#include "AtomRendererGL.h"
#include "PointRendererGL.h"
#include "LMCRendererGL.h"
#include "FieldRendererGL.h"
#include "BCARendererGL.h"


//#include "camera.h"
#include "filelist.h"

//#include "ai_graphic.h"
#include "windib2.h"
#include "setting.h"
#include "debugprint.h"



#define _USE_MATH_DEFINES
#include <math.h>




//extern CAMERA_INFO cam_info;
//extern MDLoader* atm;
//extern AtomRendererGL* renderer;
//extern Field3DLoader* g_field;
//extern int visibleMode;
//extern float bgcolor_reset[4];

//extern int num_files;
extern int AutoBMP;

//extern CameraGL* g_camera;

extern int g_timeframe;



static float* g_geoVertex = NULL;
static int g_geoVertexCount = 0;
static int* g_geoIndex = NULL;
static int g_geoIndexCount = 0;

static float* vbo_vertex = NULL;
static int vbo_vertex_count = 0;
static int* vbo_index = NULL;
static int vbo_index_count = 0;

static ATOMS_DATA* m_prev_render_dat;


extern double VIEW_ANGLE;
extern double VIEW_NEAR;
extern double VIEW_FAR;


static AtomRendererGL* m_atomRenderer = NULL;
static PointRendererGL* m_pointRenderer = NULL;
static FieldRendererGL* m_fieldRenderer = NULL;
static LMCRendererGL* m_lmcRenderer = NULL;
static BCARendererGL* m_bcaRenderer = NULL;

	

void CopyInstanceVertex(float* geoVertex, int geoVertexCount, int geoVertexStride, float* positions, int count, float **ppVertex, int *pVertexCount, int **ppIndex, int *pIndexCount);


void RenderingOneGL15(LawData& data, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property);

void InitGL15(void){
	
	
	glShadeModel(GL_SMOOTH);/*シェーディングモード,GL_SMOOTH or GL_FLAT */
	glEnable(GL_DEPTH_TEST);/* デプスバッファ(zバッファ)を使用する */
	glClearDepth( 1.0 );/* デプスバッファ(zバッファ)クリア値*/

	glEnable(GL_LIGHTING);/*光源を使用する*/
	glEnable(GL_LIGHT0);/*光源を使用する*/
	glEnable(GL_NORMALIZE);/* 法線計算を正規化 */

	glEnable(GL_CULL_FACE);/*カリングを行う*/
	glCullFace(GL_BACK);/*背面を表示しない*/


	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); /* モデリング行列の初期化 */
	
	//ライト設定はglMatrixMode(GL_MODELVIEW)の後に書かなければ有効にならない//
	const GLfloat lva[] = {0.4f, 0.4f, 0.4f,1.0f};
	//const GLfloat lvp[] = {0.0f, 0.0f, 1.0f,0.0f};
	//ライトの設定
	glLightfv(GL_LIGHT0, GL_AMBIENT, lva);
	//glLightfv(GL_LIGHT0, GL_POSITION, lvp);
}


void TerminateGL15(void){
	if(m_atomRenderer) delete [] m_atomRenderer; m_atomRenderer = NULL;
	if(m_fieldRenderer) delete [] m_fieldRenderer; m_fieldRenderer = NULL;
	if(m_lmcRenderer) delete [] m_lmcRenderer; m_lmcRenderer = NULL;
	if(m_bcaRenderer) delete [] m_bcaRenderer; m_bcaRenderer = NULL;
}


//一つのデータの可視化に関して、描画範囲をPrjection範囲として設定//
void SetProjectionRangeGL(int projection_mode, int image_w, int image_h, int range_x, int range_y, int range_w, int range_h, double distance){
	
	//射影変換の設定===================================//
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	if(projection_mode == 1){  //正射影//
		double top = distance * tan(VIEW_ANGLE);
		double right = top * (double)image_w / (double)image_h;
		
		double dh = top * 2.0 * ((double)range_h / (double)image_h);
		double dw = right * 2.0 * ((double)range_w / (double)image_w);
		
		double btm = top * (-1.0 + 2.0 * ((double)range_y / (double)image_h) );
		double left = right * (-1.0 + 2.0 * ((double)range_x / (double)image_w) );
		
		glOrtho(left, left + dw, btm, btm + dh, VIEW_NEAR*distance, VIEW_FAR * distance);

	}else{
		double top = VIEW_NEAR * distance * tan(VIEW_ANGLE);
		double right = top * (double)image_w / (double)image_h;
		
		double dh = top * 2.0 * ((double)range_h / (double)image_h);
		double dw = right * 2.0 * ((double)range_w / (double)image_w);
		
		double btm = top * (-1.0 + 2.0 * ((double)range_y / (double)image_h) );
		double left = right * (-1.0 + 2.0 * ((double)range_x / (double)image_w) );

		glFrustum(left, left + dw, btm, btm + dh, VIEW_NEAR*distance, VIEW_FAR * distance);
	}
	//===================================射影変換//
}

static void GLFromDX(mat44d& view_matrix){
	view_matrix.m31 = -view_matrix.m31;
	view_matrix.m32 = -view_matrix.m32;
	view_matrix.m33 = -view_matrix.m33;
	view_matrix.m34 = -view_matrix.m34;

}


/*
GLでの描画を行う関数
screen_w, screen_h: 対象となるスクリーン全体の幅
range_x, range_y, range_w, range_h: スクリーンのうち描画するエリア
全エリアを描画する場合は(range_x, range_y, range_w, range_h) = (0,0,screen_w,screen_h)を指定
拡大画像出力などで複数回に分けて描画する際はこれ以外の値を指定
*/
void RenderingGL15_2(std::vector<LawData>& data_list, int screen_w, int screen_h, int range_x, int range_y, int range_w, int range_h, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property){
			//初期描画と再描画時に呼ばれる。
			//resize時も再描画されるので呼ばれる。resize -> displayの順
	glClearColor(rendering_property.bg_color[0], rendering_property.bg_color[1], rendering_property.bg_color[2], rendering_property.bg_color[3]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);	//画面を背景色でクリア

	//SetLightGL();
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	
	const GLfloat lvp[] = {0.0f, 1.0f, 0.0f, 0.0f};
	glLightfv(GL_LIGHT0, GL_POSITION, lvp);


	//視点起点のMODELVIEW行列を設定//
	mat44d view_matrix_gl(*((mat44d*)view_matrix));
	//cam->GetViewMatrix((double*)&view_matrix);
	GLFromDX(view_matrix_gl);
	glLoadMatrixd((double*)&view_matrix_gl);
	


	const int num_files = data_list.size();
	if((rendering_property.multiview_mode == 0) || (num_files == 1)){

		//描画先スクリーンと画像が一枚のデータ描画サイズが同じ//
		const int image_w = screen_w;
		const int image_h = screen_h;

		glViewport(0, 0, range_w, range_h);

		SetProjectionRangeGL(rendering_property.projection_mode, image_w, image_h, range_x, range_y, range_w, range_h, focus_distance);
		//SetProjectionGL(range_w, range_h, cam->GetForcusDistance());
		
		
		for (auto it = data_list.begin(), it_end = data_list.end(); it != it_end; ++it) {
			RenderingOneGL15(*it, view_matrix, focus_distance, rendering_property);
		}
	

	}else{
		
		//スクリーンサイズ比からイメージを並べる数を計算//
		double dnx = sqrt((double)num_files * (double)screen_w / (double)screen_h);
		int multiview_nx = (int)(dnx)+1;
		if(multiview_nx > num_files ){ multiview_nx = num_files;}
		int multiview_ny = num_files / multiview_nx;
		if (num_files > multiview_nx * multiview_ny){ multiview_ny++;}


		int nx = 0;
		int ny = multiview_ny - 1;
		
		for(auto it = data_list.begin(), it_end = data_list.end(); it != it_end; ++it){
			
			
		//一枚当たりのイメージサイズ//
			const int image_x = (screen_w * nx) / multiview_nx;
			const int image_w = (screen_w * (nx+1)) / multiview_nx - image_x;
			const int image_y = (screen_h * ny) / multiview_ny;
			const int image_h = (screen_h * (ny+1)) / multiview_ny - image_y;
		
			//要求描画範囲とオーバーラップがあるか確認//
			if(range_x + range_w > image_x){
			if(image_x + image_w > range_x){
			if(range_y + range_h > image_y){
			if(image_y + image_h > range_y){
			
				//オーバーラップ部分の範囲(ただしimage_x,yからの相対位置)
				const int real_range_x = max(range_x - image_x , 0);
				const int real_range_w = min(range_x + range_w - image_x , image_w) - real_range_x;
				const int real_range_y = max(range_y - image_y , 0);
				const int real_range_h = min(range_y + range_h - image_y, image_h) - real_range_y;
			
				//オーバーラップ部分の範囲(ただしrange_x,yからの相対位置)
				const int view_x = max(0, image_x - range_x);
				const int view_w = min(range_w, image_x + image_w - range_x) - view_x;
				const int view_y = max(0, image_y - range_y);
				const int view_h = min(range_h, image_y + image_h - range_y) - view_y;
			
				glViewport( view_x, view_y, view_w, view_h);

				SetProjectionRangeGL(rendering_property.projection_mode, image_w, image_h, real_range_x, real_range_y, real_range_w, real_range_h, focus_distance);
				//SetProjectionGL( image_w, image_h, cam->GetForcusDistance() );
				RenderingOneGL15(*it, view_matrix, focus_distance, rendering_property);
			}}}}
			nx++;
			if(nx == multiview_nx){
				nx = 0;
				ny--;
			}
			
		
		}

	}


}


void RenderingOneGL15(LawData& data, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property){
	

	//visual setting//
	VISUAL_SETTING vis;
	vis.atom = rendering_property.atom_draw;
	vis.atom_poly = rendering_property.atom_poly;
	vis.atom_radius = rendering_property.atom_radius;
	vis.atom_color = rendering_property.atom_color;
	vis.bond = rendering_property.bond_draw;
	vis.bond_poly = rendering_property.bond_poly;
	vis.trajectory_width = g_visual_trajectory_width;


	switch(data.type){
	case FILETYPE_ATOM:
		{			   

			ATOMS_DATA* dat = data.GetDataPointer();
			
			if(vis.atom == VISUAL_ATOM_SPHERE){
				if(m_atomRenderer == NULL) m_atomRenderer = new AtomRendererGL();

				m_atomRenderer->Draw(dat, data.GetBond(), vis);
				
			}else if(vis.atom == VISUAL_ATOM_POINT){
				if(m_pointRenderer == NULL) m_pointRenderer = new PointRendererGL();

				m_pointRenderer->Draw(dat, NULL, vis);
				
			}			

			if (rendering_property.box_draw){
				
				if((_tcscmp(data.ext,_T("xyz")) != 0) 
					&& (_tcscmp(data.ext,_T("md2")) != 0)
					&& (_tcscmp(data.ext,_T("md")) != 0)
					&& (_tcscmp(data.ext,_T("pdb")) != 0)) {

					mat33f boxaxis_f(dat->boxaxis);
					vec3f boxorg_f(dat->boxorg);
					DrawCubeGL15(&(boxaxis_f), &(boxorg_f), rendering_property.bg_color);
					glFlush();
				}

			}



		}
		break;

#if 0
	case FILETYPE_TRAJ:
		{
			glLineWidth(2.0f);


			//fi->bca->Rendering();
			Trajectory* traj = data.GetTrajectory();
			int frameno = data.GetFrameNum();
			
			double boxaxisorg[12];
			data.GetBoxSize((mat33d*)boxaxisorg, (vec3d*)(boxaxisorg+9));
				
			if(m_bcaRenderer == NULL) m_bcaRenderer = new BCARendererGL();
			
			m_bcaRenderer->Draw(traj, frameno, g_visual_atomradius, g_visual_trajectory_color, g_periodic, boxaxisorg);
			

			if (rendering_property.box_draw){
				
				mat33f baf; 
				baf.Set(boxaxisorg[0], boxaxisorg[1], boxaxisorg[2], boxaxisorg[3], boxaxisorg[4], boxaxisorg[5], boxaxisorg[6], boxaxisorg[7], boxaxisorg[8]);
				vec3f bof;
				bof.Set(boxaxisorg[9], boxaxisorg[10], boxaxisorg[11]);
				DrawCubeGL15(&baf, &bof, rendering_property.bg_color);
                glFlush();
			}
			
			glLineWidth(1.0f);
			break;
		}
#endif
	}		


	return;
	
}


void CopyInstanceVertex(float* geoVertex, int geoVertexCount, int geoVertexStride, float* positions, int count, float **ppVertex, int *pVertexCount, int **ppIndex, int *pIndexCount){

	int snum = 6;
	int stride = snum * sizeof(float);
	float *buffer = *ppVertex;

	for(int k = 0; k < count * 3; k+=3){
		
		for(int i = 0; i < geoVertexCount * snum; i += snum){ 
			memcpy(buffer + i, geoVertex, stride);
			buffer[i] += positions[k];
			buffer[i+1] += positions[k+1];
			buffer[i+2] += positions[k+2];
		}
		buffer += geoVertexCount * snum;
	}
	
}



void InitSphereGeometry(float radius, int slices, int stacks, float **ppVertex, int *pVertexCount, int **ppIndex, int *pIndexCount){
	
	const int snum = 6;

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

	int vertexCount = ((slices - 1) * stacks + 2);
	*ppVertex = new float[vertexCount * snum];
	//vec3f *nor = m_sphere_nor = new vec3f[m_sphere_vcnt];


	vec3f *pos = (vec3f*)(*ppVertex);
	vec3f *nor = (vec3f*)(vertexCount + 3);
	pos->Set(0.0f,0.0f,(float)radius);
	nor->Set(0.0f, 0.0f, 1.0f);

	angleZX = daZX;
	for (i = 1; i < slices; i++){
		cosZX = (float)cos(angleZX);
		sinZX = (float)sin(angleZX);

		for (k = 0; k < stacks; k++){
	
			pos = (vec3f*)(*ppVertex + k + i * stacks);
			nor = (vec3f*)(*ppVertex + k + i * stacks + 3);
	
			nor->Set(sinZX * cosXY[k], sinZX * sinXY[k], cosZX);
			pos->x = nor->x * radius;
			pos->y = nor->y * radius;
			pos->z = nor->z * radius;

		}
		angleZX += daZX;
	}
	
	pos = (vec3f*)(*ppVertex + k + i * stacks);
	nor = (vec3f*)(*ppVertex + k + i * stacks + 3);

	nor->Set(0.0f, 0.0f, -1.0f);
	pos->Set(0.0f,0.0f, -radius);
	
	delete [] cosXY;
	delete [] sinXY;


	//インデックス配列のセット
	int indexCount = (slices - 1)*stacks*6;
	int* idx = *ppIndex = new int[indexCount];
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
		*idx = vertexCount - 1;
		idx++;
		*idx = ((i + 1) % stacks) + offset;
		idx++;
	}

	*pVertexCount = vertexCount;
	*pIndexCount = indexCount;

};




void DrawCubeGL15(mat33f* axis, vec3f* org, const float* bg_color){
	float clr[4];
	clr[0] = 1.f - bg_color[0];
	clr[1] = 1.f - bg_color[1];
	clr[2] = 1.f - bg_color[2];
	//clr[3] = 1.0f;
	
	const float boundary_width = 2.0f;
	float original_width = 0.0f;
	glGetFloatv(GL_LINE_WIDTH, &original_width);
	glLineWidth(boundary_width);
	
	//glMaterialfv(GL_FRONT_AND_BACK,GL_AMBIENT_AND_DIFFUSE,clr);

	
	glDisable(GL_LIGHTING);
	glColor3fv(clr);
	vec3f pos;

	glBegin(GL_LINE_LOOP);
		pos = *org + axis->b; 
		glVertex3fv((float*)&pos);//(x1, y2, z1);
		pos = *org; 
		glVertex3fv((float*)&pos);
		pos = *org + axis->c; 
		glVertex3fv((float*)&pos);
		pos = *org + axis->b + axis->c; 
		glVertex3fv((float*)&pos);
	glEnd();

	glBegin(GL_LINE_LOOP);
		pos = *org; 
		glVertex3fv((float*)&pos);//(x1, y2, z1);
		pos = *org + axis->a; 
		glVertex3fv((float*)&pos);
		pos = *org + axis->a + axis->c; 
		glVertex3fv((float*)&pos);
		pos = *org + axis->c; 
		glVertex3fv((float*)&pos);
	glEnd();
	
	
	glBegin(GL_LINE_LOOP);
		pos = *org + axis->a; 
		glVertex3fv((float*)&pos);//(x1, y2, z1);
		pos = *org + axis->a + axis->b; 
		glVertex3fv((float*)&pos);
		pos = *org + axis->a + axis->b + axis->c; 
		glVertex3fv((float*)&pos);
		pos = *org + axis->a + axis->c; 
		glVertex3fv((float*)&pos);
	glEnd();
	
	
	glBegin(GL_LINE_LOOP);
		pos = *org + axis->a + axis->b; 
		glVertex3fv((float*)&pos);//(x1, y2, z1);
		pos = *org + axis->b; 
		glVertex3fv((float*)&pos);
		pos = *org + axis->b + axis->c; 
		glVertex3fv((float*)&pos);
		pos = *org + axis->a + axis->b + axis->c;
		glVertex3fv((float*)&pos);
	glEnd();
	
	glLineWidth(original_width);
	
	glEnable(GL_LIGHTING);
	
	const float white[4] = {1.0f, 1.0f, 1.0f, 1.0f};
	glColor3fv(white);
	
	
}
