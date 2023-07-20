
#include "FieldRendererGL.h"

#pragma warning(disable : 4996)

extern double g_visual_split_ratio;

FieldRendererGL::FieldRendererGL() :
	m_textureName(0),
	m_bfield_prev(NULL)
{
	

	
}


FieldRendererGL::~FieldRendererGL(){
	
	
}

void FieldRendererGL::Create3DTexture(GLubyte* bfield, int grid_x, int grid_y, int grid_z){
	

	//3Dテクスチャの生成=====================================================

	//GLのテクスチャ初期化
	glEnable(GL_TEXTURE_3D);
	if(m_textureName == 0){
		glGenTextures(1 , &m_textureName);		//テクスチャ生成
	}
	glBindTexture(GL_TEXTURE_3D , m_textureName);		//テクスチャ接続

	//転送を4バイトごとに、32bitでは高速
	glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
  	
	PFNGLTEXIMAGE3DPROC glTexImage3D;
	//APIを取得して実行
	glTexImage3D = (PFNGLTEXIMAGE3DPROC)wglGetProcAddress("glTexImage3D");
	glTexImage3D(GL_TEXTURE_3D, 0, GL_RGBA, grid_x, grid_y, grid_z, 0, GL_RGBA, GL_UNSIGNED_BYTE, bfield);

	// テクスチャを拡大・縮小する方法の指定
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);	//GL_NEAREST
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	// テクスチャの繰り返しの指定
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_S, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_T, GL_CLAMP);
	glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_WRAP_R, GL_CLAMP);

	glDisable(GL_TEXTURE_3D);

}


void FieldRendererGL::CreateSideVector(mat33d* boxaxis, vec3d* boxorg){

	//立方体の生成
	
	
	m_box_vtx = new vec3d[8];
	/*
	m_box_vtx[0].x = -m_boxaxis.m11 *0.5;
	m_box_vtx[0].y = -m_boxaxis.m22 *0.5;
	m_box_vtx[1].x = m_boxaxis.m11 *0.5;
	m_box_vtx[1].y = -m_boxaxis.m22 *0.5;
	m_box_vtx[2].x = m_boxaxis.m11 *0.5;
	m_box_vtx[2].y = m_boxaxis.m22 *0.5;
	m_box_vtx[3].x = -m_boxaxis.m11 *0.5;
	m_box_vtx[3].y = m_boxaxis.m22 *0.5;
	
	for(int i = 0; i< 4; i++){
		m_box_vtx[i+4] = m_box_vtx[i];
		m_box_vtx[i].z = -m_boxaxis.m33 *0.5;
		m_box_vtx[i+4].z = m_boxaxis.m33 *0.5;
	}*/

	m_box_vtx[0] = *boxorg;
	m_box_vtx[1] = *boxorg + boxaxis->a;
	m_box_vtx[2] = *boxorg + boxaxis->a + boxaxis->b;
	m_box_vtx[3] = *boxorg + boxaxis->b;
	for(int i = 0; i< 4; i++){
		m_box_vtx[i+4] = m_box_vtx[i] + boxaxis->c;
	}

#define TEXMARGIN		(1.0E-2)
	m_box_crd = new vec3d[8];
	m_box_crd[0].x = TEXMARGIN;
	m_box_crd[0].y = TEXMARGIN;
	m_box_crd[1].x = 1.0 - TEXMARGIN;
	m_box_crd[1].y = TEXMARGIN;
	m_box_crd[2].x = 1.0 - TEXMARGIN;
	m_box_crd[2].y = 1.0 - TEXMARGIN;
	m_box_crd[3].x = TEXMARGIN;
	m_box_crd[3].y = 1.0 - TEXMARGIN;
	int i;
	for(i = 0; i< 4; i++){
		m_box_crd[i+4] = m_box_crd[i];
		m_box_crd[i].z = TEXMARGIN;
		m_box_crd[i+4].z = 1.0 - TEXMARGIN;
	}
}


//ボリュームレンダリング//
void FieldRendererGL::Draw(GLubyte* bfield, int grid_x, int grid_y, int grid_z, mat33d* boxaxis, vec3d* boxorg, vec3d* target, vec3d* camera){
	if(	m_bfield_prev != bfield ){
		Create3DTexture(bfield, grid_x, grid_y, grid_z);
		m_bfield_prev = bfield;
		CreateSideVector(boxaxis, boxorg);
	}

	vec3d org = *target;	//断面の中心
	int depth_resolution = (int)(g_visual_split_ratio * sqrt( ((double)grid_x)*((double)grid_x) + ((double)grid_y)*((double)grid_y) + ((double)grid_z)*((double)grid_z) ) );
	
	

	double len = (boxaxis->m11 + boxaxis->m22 + boxaxis->m33)/3.0;
	vec3d camdir;
	camdir = (*target ) - (*camera);
	camdir.Normalize();

	org.x += camdir.x * len;
	org.y += camdir.y * len;
	org.z += camdir.z * len;
	double ratio = len * 2.0 /(double)depth_resolution;
/*
		org.x -= camdir->x * ratio*35.0;
		org.y -= camdir->y * ratio*35.0;
		org.z -= camdir->z * ratio*35.0;

		SetBinarySection(camdir, org);
*/	
	
	
	glDisable(GL_CULL_FACE);

	
	for (int i = 0; i < depth_resolution; i++){
		
		org.x -= camdir.x * ratio;
		org.y -= camdir.y * ratio;
		org.z -= camdir.z * ratio;
		SetBinarySection(&camdir, &org, boxaxis);
	}

	
}


void FieldRendererGL::SetBinarySection(vec3d* camdir, vec3d* org, mat33d* boxaxis){
//断面と6つの面が交差するかをチェック.
//交差するなら断面の中心と、二つ辺の交差点でできる三角形の面を描画面として追加する.


	vec3d org_crd;	//断面の中心のテクスチャ座標
	org_crd.x = (org->x - m_box_vtx[0].x) / (boxaxis->m11);
	org_crd.y = (org->y - m_box_vtx[0].y) / (boxaxis->m22);
	org_crd.z = (org->z - m_box_vtx[0].z) / (boxaxis->m33);
	


	//断面と12本の辺の交点が、辺の内部(0<t<1)に収まっているかチェック
	
	static int vpair[12*2] = { 0, 1,  1, 2,  2, 3,  3, 0,  
					 4, 5,  5, 6,  6, 7,  7, 4,
					 0, 4,  1, 5,  2, 6,  3, 7};

	double t[12];
	vec3d p[12];
	vec3d crd[12];
	double tt;

	int i;
	for (i = 0; i < 12; i++){
		int a = vpair[i*2];
		int b = vpair[i*2+1];
			
		tt = t[i] = CrossPoint(org, camdir, &m_box_vtx[a], &m_box_vtx[b]);

		p[i].x = tt * (m_box_vtx[a].x - m_box_vtx[b].x) + m_box_vtx[b].x;
		p[i].y = tt * (m_box_vtx[a].y - m_box_vtx[b].y) + m_box_vtx[b].y;
		p[i].z = tt * (m_box_vtx[a].z - m_box_vtx[b].z) + m_box_vtx[b].z;
		crd[i].x = tt * (m_box_crd[a].x - m_box_crd[b].x) + m_box_crd[b].x;
		crd[i].y = tt * (m_box_crd[a].y - m_box_crd[b].y) + m_box_crd[b].y;
		crd[i].z = tt * (m_box_crd[a].z - m_box_crd[b].z) + m_box_crd[b].z;
					
	}

	//6つの面を断面が交わるかチェック
	//面と交わるなら、面を構成する4つの辺のうち二つと交差するはず
	int facelist[6*4] = { 0, 1, 2, 3,
						  4, 5, 6, 7,
						  0, 9, 4, 8,
						  1, 10, 5, 9,
						  2, 11, 6, 10, 
						  3, 8, 7, 11};

	float clr[] = {0.0, 1.0, 1.0,1.0};
	//glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,clr);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE);
		
	glEnable(GL_TEXTURE_3D);
	glBindTexture(GL_TEXTURE_3D , m_textureName);

	int polynum = 0;
	glBegin(GL_TRIANGLES);
	int k;
	int num;
	int vfirst = -1;		//最初の交点は断面を構成する三角形の共通とする
	int vlist[6*2];
	int vnum = 0;
	
	for (i = 0; i < 6; i++){
		num = 0;	//四つの辺のうち交わった個数
		
		for (k = 0; k < 4; k++){
			int face = facelist[i * 4 + k];
			
			
			if (0.0 <= t[face]){
				if(t[face]<=1.0){
					
					if(vfirst == -1){//最初の交点は断面を構成する三角形の共通とする
						vfirst = face;
						break;
					}else if(vfirst == face){//最初の交点と同じ点では三角形を作れない
						break;
					}else{					////最初の交点と共に三角形を作る残り二つの点
						vlist[vnum + num] = face;
						num++;
						if(num == 2){
							vnum += 2;
							break;
						}
					}
				}
			}
		}		
	}
	
	
	//交点が二点見つかったのでポリゴン追加
	for (i = 0; i < vnum; i+=2){
					
		glTexCoord3dv((double*)(crd + vlist[i]));
		glNormal3dv((double*)camdir);
		glVertex3dv((double*)(p + vlist[i]));
		//TRACE(_T("tri-1: %lf, %lf, %lf\n"), p[0].x, p[0].y, p[0].z);

		glTexCoord3dv((double*)(crd + vlist[i+1]));
		glNormal3dv((double*)camdir);
		glVertex3dv((double*)(p + vlist[i+1]));
		//TRACE(_T("tri-2: %lf, %lf, %lf\n"), p[1].x, p[1].y, p[1].z);
					
		glTexCoord3dv((double*)(crd + vfirst));
		glNormal3dv((double*)camdir);
		glVertex3dv((double*)(p + vfirst));
				//TRACE(_T("tri-3: %lf, %lf, %lf\n"), org.x, org.y, org.z);
	}	

	glEnd();

	glDisable(GL_TEXTURE_3D);
	glDisable(GL_BLEND);

	//TRACE(_T("polynum:%d, %lf, %lf, %lf\n"), polynum, org.x, org.y, org.z);
	
	
}

double FieldRendererGL::CrossPoint(vec3d* org, vec3d* v, vec3d* a, vec3d* b){

	vec3d ao, bo;
	
	ao.x = a->x - org->x;
	ao.y = a->y - org->y;
	ao.z = a->z - org->z;

	bo.x = b->x - org->x;
	bo.y = b->y - org->y;
	bo.z = b->z - org->z;

	double va = v->x * ao.x + v->y * ao.y + v->z * ao.z;
	double vb = v->x * bo.x + v->y * bo.y + v->z * bo.z;
	
	if(fabs(va-vb) > 1.0e-10){
		return -vb/(va-vb);
	}else{
		return 1.0e10;
	}
	
}

/*
void FieldRendererGL::SetBoundary(){
	float clr[] = {1.0, 1.0, 1.0,1.0};
	
	glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,clr);


	glBegin(GL_LINE_LOOP);
		glVertex3dv((double*)m_box_vtx);
		glVertex3dv((double*)(m_box_vtx+1));
		glVertex3dv((double*)(m_box_vtx+2));
		glVertex3dv((double*)(m_box_vtx+3));
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3dv((double*)(m_box_vtx+4));
		glVertex3dv((double*)(m_box_vtx+5));
		glVertex3dv((double*)(m_box_vtx+6));
		glVertex3dv((double*)(m_box_vtx+7));
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3dv((double*)(m_box_vtx));
		glVertex3dv((double*)(m_box_vtx+4));
		glVertex3dv((double*)(m_box_vtx+5));
		glVertex3dv((double*)(m_box_vtx+1));
	glEnd();

	glBegin(GL_LINE_LOOP);
		glVertex3dv((double*)(m_box_vtx+2));
		glVertex3dv((double*)(m_box_vtx+6));
		glVertex3dv((double*)(m_box_vtx+7));
		glVertex3dv((double*)(m_box_vtx+3));
	glEnd();

	
}
*/
/*

//スペクトル色を返す
void FieldRendererGL::SpectrumColor(double value, BYTE *r, BYTE* g, BYTE * b){
//	static BYTE tr[11] = {0, 12, 90,160,200,225,240,250,250,250,250};
//	static BYTE tg[11] = {0,  0,  0,  0,  0, 40, 90,140,190,240,250};
//	static BYTE tb[11] = {0,120,150,150,120, 10,  0,  0, 10,120,250};
	static BYTE tr[11] = {0, 38, 76,  0,  0,  0,  0, 60,120,180,255};
	static BYTE tg[11] = {0,  0,  0,  0,140,180,250,180,120, 60,  0};
	static BYTE tb[11] = {0,106,211,230,120, 60,  0,  0,  0,  0,  0};

	if(value < 0.0){
		*r = *g = *b = 0;
	}

	double pos;
	double tt = modf(value * 10.0, &pos);
	int ipos = (int)(pos);
	if(ipos >= 10){
		*r = tr[10];
		*g = tg[10];
		*b = tb[10];
	}else{
		*r = tr[ipos] + (BYTE)(tt * (tr[ipos+1]-tr[ipos]));
		*g = tg[ipos] + (BYTE)(tt * (tg[ipos+1]-tg[ipos]));
		*b = tb[ipos] + (BYTE)(tt * (tb[ipos+1]-tb[ipos]));
	}

}

void FieldRendererGL::GetBoxaxis(mat33f* boxaxis, vec3f* boxorg){

	*boxaxis = m_boxaxis;
	*boxorg = m_boxorg;


}
*/