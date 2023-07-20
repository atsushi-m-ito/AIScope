#define _USE_MATH_DEFINES



#include <stdlib.h>
#include <stdio.h>
#include <math.h>

#include "camera.h"
#include "setting.h"





//double VIEW_ANGLE = 20.0 * M_PI / 180.0;
static const double DEFAULT_VIEW_NEAR = 0.1;
static const double DEFAULT_VIEW_FAR = 10.0;

double VIEW_NEAR = DEFAULT_VIEW_NEAR;
double VIEW_FAR = DEFAULT_VIEW_FAR;



CameraGL::CameraGL(){
	m_target.x = 0.f;
	m_target.y = 0.f;
	m_target.z = 0.f;
	m_camera_position.x = 0.f;
	m_camera_position.y = -50.f;
	m_camera_position.z = 0.f;
	m_up.x = 0.f;
	m_up.y = 0.f;
	m_up.z = 1.f;

//チェックポイント機能	
	m_chkpoint_fst = NULL;
	m_chkpoint_current = NULL;

}


CameraGL::~CameraGL(){
}


void CameraGL::ResetFocus(double focus_distance, const vec3d& target_position){
	
	//カメラからターゲットまでの距離//
	double distance = focus_distance;
	
	//カメラから視体積のznearまでの距離//
//	VIEW_NEAR = max_side * 0.1;
	
	//カメラから視体積のznearまでの距離//
//	VIEW_FAR = distance + max_side * 10.0;
	
	//ターゲットの座標//
	m_target = target_position;
	
	//カメラ座標//
	m_camera_position = m_target;
	m_camera_position.y -= distance;

	m_up.x = 0.f;
	m_up.y = 0.f;
	m_up.z = 1.f;

}

void CameraGL::GetViewMatrix(double* view_matrix){

	/////////////////////////////////////////////////////////////////////
	//左手座標系or右手座標系はここで決まる//
	//左手座標系 : 画面右をx軸正方向,画面上をy軸正方向としたとき,画面奥がz軸正方向//
	vec3d cx;
	vec3d cy;
	vec3d cz = m_target - m_camera_position;	//画面奥をz軸正方向にとる//
	cz.Normalize();
	cy = (m_up) - cz * ((m_up) * cz);	//画面上をy軸正方向にとる//
	cy.Normalize();
	cx = Cross(cz, cy);				//画面右をx軸正方向にとる//
	////////////////////////////////////////////////////////////////////////

	//各軸との内積を意味する行列//
	view_matrix[0] = cx.x;
	view_matrix[1] = cy.x;
	view_matrix[2] = cz.x;
	view_matrix[3] = 0.0;
	
	view_matrix[4] = cx.y;
	view_matrix[5] = cy.y;
	view_matrix[6] = cz.y;
	view_matrix[7] = 0.0;

	view_matrix[8] = cx.z;
	view_matrix[9] = cy.z;
	view_matrix[10] = cz.z;
	view_matrix[11] = 0.0;
	
	view_matrix[12] = -(cx * (m_camera_position));
	view_matrix[13] = -(cy * (m_camera_position));
	view_matrix[14] = -(cz * (m_camera_position));
	view_matrix[15] = 1.0;

}

double CameraGL::GetFocusDistance(){
    vec3d v = m_camera_position - m_target;

    double rr = v * v;
    return sqrt(rr);
}


/*
void CameraGL::SetGLCamera(){

	glLoadIdentity();
	gluLookAt(m_camera_position.x, m_camera_position.y, m_camera_position.z,
		m_target.x, m_target.y, m_target.z,
		m_up.x, m_up.y, m_up.z);


}
*/
void CameraGL::SetViewFront(int section, int sprit){
	
	//カメラからターゲットまでの距離//
	
	double distance = GetFocusDistance();

	//カメラから視体積のznearまでの距離//
	VIEW_NEAR = (1.0 - (double)section / (double)sprit * 0.5);
	
	//カメラから視体積のzfarまでの距離//
	//VIEW_FAR = (VIEW_NEAR + (double)sprit * 0.5);
	
}

void CameraGL::ResetViewFront(){
	
	VIEW_NEAR = DEFAULT_VIEW_NEAR;
	VIEW_FAR = DEFAULT_VIEW_FAR;
}

void CameraGL::DragStart(){

	m_drag_org = m_target;
	m_drag_up = m_up;
	m_drag_front = m_target - m_camera_position;
	m_drag_distance = sqrt(m_drag_front*m_drag_front);

	OuterProd(&m_drag_front, &m_up, &m_drag_right);
	m_drag_right = Unit(m_drag_right);

}

void CameraGL::DragTranslate(double dx, double dy, double w, double h){

	
	double top = m_drag_distance * tan(VIEW_ANGLE);
	double right = top * (w/h);

	double dvx = right * (2.0*dx/w);
	double dvy = top * (2.0*dy/h);

	m_target = m_drag_org - m_drag_right * dvx - m_drag_up * dvy;
	m_camera_position = m_target - m_drag_front;
	m_up = m_drag_up;

}

void CameraGL::DragRotate(double dx, double dy, double w, double h){

	double rad = sqrt(dx*dx + dy*dy) / (max(w,h));
	rad *= M_PI;

	vec3d axis =  m_drag_right * (dy/h) - m_drag_up * (dx/h);
	axis = Unit(axis);

	m_target = m_drag_org;
	m_camera_position = -m_drag_front;
	m_up = m_drag_up;
	RotateOnAxis(&axis, &m_camera_position, &m_up, rad);
	
	m_camera_position += m_target;

}



void CameraGL::GetRAY(double x, double y, double w, double h, vec3d* vray, vec3d* vorg){

	
	vec3d vfront = m_target - m_camera_position;
	double distance = sqrt(vfront*vfront);

	vec3d vup = m_up;

	vec3d vright;
	OuterProd(&vfront, &vup, &vright);
	vright = Unit(vright);

	double top = distance * tan(VIEW_ANGLE);
	vup *= top;
	vright *= top * (w/h);

	double dvx = (2.0*x/w-1.0);
	double dvy = -(2.0*y/h-1.0);

	*vray = vfront + vright * dvx + vup * dvy;
	*vorg = m_camera_position;
	return;
}




//軸aを中心に、ベクトルvを回転する
void CameraGL::RotateOnAxis(vec3d* a, vec3d* v, double rad){
	
	
	double cos_rad = cos(rad);
	double sin_rad = sin(rad);
	double one_cos = 1.0 - cos_rad;
	double sin_ax = sin_rad * a->x;
	double sin_ay = sin_rad * a->y;
	double sin_az = sin_rad * a->z;

//回転行列生成
	double m[9];
	m[0] = cos_rad + one_cos * a->x * a->x;
	m[3] = m[1] = one_cos * a->x * a->y;
	m[6] = m[2] = one_cos * a->z * a->x;
	m[7] = m[5] = one_cos * a->y * a->z;
	m[4] = cos_rad + one_cos * a->y * a->y;
	m[8] = cos_rad + one_cos * a->z * a->z;

	m[1] -= sin_az;
	m[2] += sin_ay;
	m[3] += sin_az;
	m[5] -= sin_ax;
	m[6] -= sin_ay;
	m[7] += sin_ax;
	
	vec3d v_org = *v;
	v->x = m[0] * v_org.x + m[1] * v_org.y + m[2] * v_org.z;
	v->y = m[3] * v_org.x + m[4] * v_org.y + m[5] * v_org.z;
	v->z = m[6] * v_org.x + m[7] * v_org.y + m[8] * v_org.z;
	
			
}


//軸aを中心に、ベクトルv1,v2を回転する
void CameraGL::RotateOnAxis(vec3d* a, vec3d* v1, vec3d* v2, double rad){
	
	
	double cos_rad = cos(rad);
	double sin_rad = sin(rad);
	double one_cos = 1.0 - cos_rad;
	double sin_ax = sin_rad * a->x;
	double sin_ay = sin_rad * a->y;
	double sin_az = sin_rad * a->z;

//回転行列生成
	double m[9];
	m[0] = cos_rad + one_cos * a->x * a->x;
	m[3] = m[1] = one_cos * a->x * a->y;
	m[6] = m[2] = one_cos * a->z * a->x;
	m[7] = m[5] = one_cos * a->y * a->z;
	m[4] = cos_rad + one_cos * a->y * a->y;
	m[8] = cos_rad + one_cos * a->z * a->z;

	m[1] -= sin_az;
	m[2] += sin_ay;
	m[3] += sin_az;
	m[5] -= sin_ax;
	m[6] -= sin_ay;
	m[7] += sin_ax;
	
	vec3d v_org = *v1;
	v1->x = m[0] * v_org.x + m[1] * v_org.y + m[2] * v_org.z;
	v1->y = m[3] * v_org.x + m[4] * v_org.y + m[5] * v_org.z;
	v1->z = m[6] * v_org.x + m[7] * v_org.y + m[8] * v_org.z;
	
	v_org = *v2;
	v2->x = m[0] * v_org.x + m[1] * v_org.y + m[2] * v_org.z;
	v2->y = m[3] * v_org.x + m[4] * v_org.y + m[5] * v_org.z;
	v2->z = m[6] * v_org.x + m[7] * v_org.y + m[8] * v_org.z;
		
}


void CameraGL::OuterProd(vec3d* u, vec3d* v, vec3d* a){
	a->x = u->y * v->z - u->z * v->y;
	a->y = u->z * v->x - u->x * v->z;
	a->z = u->x * v->y - u->y * v->x;
}

//カメラの上方向軸と平行なターゲット原点の軸を中心に回転
void CameraGL::RotatePosOnUpAxis(double rad){
	vec3d v;
	v.x = m_camera_position.x - m_target.x;
	v.y = m_camera_position.y - m_target.y;
	v.z = m_camera_position.z - m_target.z;
	
	RotateOnAxis(&m_up, &v, rad);
	
	m_camera_position.x = v.x + m_target.x;
	m_camera_position.y = v.y + m_target.y;
	m_camera_position.z = v.z + m_target.z;
	
}

//ワールド座標のz軸中心の回転
void CameraGL::RotatePosOnZAxis(double rad){
	vec3d v;
	v.x = m_camera_position.x - m_target.x;
	v.y = m_camera_position.y - m_target.y;
	v.z = m_camera_position.z - m_target.z;
	
	vec3d a;
	a.x = 0.0;
	a.y = 0.0;
	a.z = 1.0;
	
	RotateOnAxis(&a, &v, rad);
	RotateOnAxis(&a, &m_up, rad);
	
	m_camera_position.x = v.x + m_target.x;
	m_camera_position.y = v.y + m_target.y;
	m_camera_position.z = v.z + m_target.z;
	
}

//カメラの上方向への回転
void CameraGL::RotatePosToUp(double rad){
	vec3d v;
	v.x = m_camera_position.x - m_target.x;
	v.y = m_camera_position.y - m_target.y;
	v.z = m_camera_position.z - m_target.z;

	vec3d a;
	OuterProd(&m_up, &v, &a);
	//a.x = m_up.y * v.z - m_up.z * v.y;
	//a.y = m_up.z * v.x - m_up.x * v.z;
	//a.z = m_up.x * v.y - m_up.y * v.x;
	double rr = a.x*a.x + a.y*a.y + a.z*a.z;
	rr = 1.0 / sqrt(rr);
	a.x *= rr;
	a.y *= rr;
	a.z *= rr;

	RotateOnAxis(&a, &v, -rad);
	RotateOnAxis(&a, &m_up, -rad);
	
	m_camera_position.x = v.x + m_target.x;
	m_camera_position.y = v.y + m_target.y;
	m_camera_position.z = v.z + m_target.z;
	
}

//ヨー・ピッチ・ロールによる///////////////////////////////////////
void CameraGL::SetYawPitchRoll(double yaw, double pitch, double roll){
	
	vec3d v = m_camera_position - m_target;
	double distance = sqrt(v*v);

	vec3d up = { -sin(-yaw), cos(-yaw), 0.0};
	vec3d cam = { 0.0, 0.0, distance};

	vec3d axis_for_pitch = Cross(cam, up);
	axis_for_pitch.Normalize();
	RotateOnAxis(&axis_for_pitch, &cam, &up, pitch);
	
	vec3d axis_for_roll = up;
	RotateOnAxis(&axis_for_roll, &cam, roll);
	
	m_up = up;
	m_camera_position = m_target + cam;
}


//回転行列による///////////////////////////////////////
void CameraGL::SetRotationMatrix(const float* matrix){
	
	vec3d v = m_camera_position - m_target;
	double distance = sqrt(v*v);

	vec3d up = { 0.0, 1.0, 0.0};
	vec3d eye= { 0.0, 0.0, 1.0};

//	vec3d cam;// = { 0.0, 0.0, distance};

	m_up.x = matrix[1] * up.y;
	m_up.y = matrix[4] * up.y;
	m_up.z = matrix[7] * up.y;

	m_up.Normalize();

	m_camera_position.x = matrix[2] * eye.z;
	m_camera_position.y = matrix[5] * eye.z;
	m_camera_position.z = matrix[8] * eye.z;
	m_camera_position.Normalize();
	m_camera_position*= distance;
	


	m_camera_position += m_target;
}
//カメラの接近1
void CameraGL::FocusDown(double ratio){
	vec3d v;
	v = m_camera_position - m_target;

	if(ratio > 0.0){
		m_camera_position = m_target + v * (1.0 + ratio);
	}else{
		m_camera_position = m_target + v / (1.0 - ratio);
	}

}


void CameraGL::TargetMoveToZ(double z){
	m_camera_position.z += z;
	m_target.z += z;
}

//視線方向からみて真横に移動
void CameraGL::TargetMoveToUp(double d){
	
	m_target.x += m_up.x*d;
	m_target.y += m_up.y*d;
	m_target.z += m_up.z*d;
	m_camera_position.x += m_up.x*d;
	m_camera_position.y += m_up.y*d;
	m_camera_position.z += m_up.z*d;

}


//視線方向からみて真横に移動
void CameraGL::TargetMoveToSide(double d){
	vec3d v;
	v.x = m_camera_position.x - m_target.x;
	v.y = m_camera_position.y - m_target.y;
	v.z = m_camera_position.z - m_target.z;
	
	vec3d a;
	OuterProd(&m_up, &v, &a);
	double rr = a.x*a.x + a.y*a.y + a.z*a.z;
	rr = d / sqrt(rr);
	a.x *= rr;
	a.y *= rr;
	a.z *= rr;
	
	m_target += a;
	m_camera_position += a;

}

//視線方向からみて前方に移動
void CameraGL::TargetMoveToFront(double d){
	vec3d v;
	v = m_target - m_camera_position;
	
	double rr = v.x*v.x + v.y*v.y + v.z*v.z;
	rr = d / sqrt(rr);
	v *= rr;
	
	m_target += v;
	m_camera_position += v;

}

void CameraGL::SetStereoMode(double ratio){
	if ( ratio <= 1.0){ return;}

	vec3d v;
	v.x = m_camera_position.x - m_target.x;
	v.y = m_camera_position.y - m_target.y;
	v.z = m_camera_position.z - m_target.z;
	
	vec3d a;
	OuterProd(&m_up, &v, &a);
	
	m_target.x = m_camera_position.x - v.x * ratio;
	m_target.y = m_camera_position.y - v.y * ratio;
	m_target.z = m_camera_position.z - v.z * ratio;
	

	v.x = m_camera_position.x - m_target.x;
	v.y = m_camera_position.y - m_target.y;
	v.z = m_camera_position.z - m_target.z;
	OuterProd(&v, &a, &m_up);
	
	double rr = m_up.x*m_up.x + m_up.y*m_up.y + m_up.z*m_up.z;
	rr = 1.0 / sqrt(rr);
	m_up.x *= rr;
	m_up.y *= rr;
	m_up.z *= rr;
	
}


//カメラの現在位置を記憶
void CameraGL::PushState(){
	
	m_target_mem = m_target;
	m_camera_mem = m_camera_position;
	m_up_mem = m_up;
}

void CameraGL::PullState(){
	
	m_target = m_target_mem;
	m_camera_position = m_camera_mem;
	m_up = m_up_mem;
}



//////////////////////////////////////////////////////
//
//    チェックポイント機能
//
//////////////////////////////////////////////////////
void CameraGL::DeleteAllCheckPoint(){
	
	CHKPOINT* chkpoint;
	while(m_chkpoint_fst){
		chkpoint = m_chkpoint_fst->next;
		delete m_chkpoint_fst;
		m_chkpoint_fst = chkpoint;
	}	

	//終了後はfstはNULLになっているはず
}


//リストにチェックポイントを挿入する
//overblowには次フレームとのフレーム数の差に余分に付け加えて表示させるフレーム数を記述。
//今のところoverblowは次のチェックポイントを同じフレームでカメラの向きが異なる場合に使用する
//overblowに0をsetしたときは次のフレームとの差がそのままoverblowになる
void CameraGL::InsertCheckPoint(int frame, int overblow){
	
	m_chkpoint_current = new CHKPOINT;
	
	m_chkpoint_current->frame = frame;
	m_chkpoint_current->overblow = overblow;
	m_chkpoint_current->target = m_target;	//ポインタではなく値コピー
	m_chkpoint_current->camera = m_camera_position;	//ポインタではなく値コピー
	m_chkpoint_current->up = m_up;	//ポインタではなく値コピー


	if(m_chkpoint_fst){
		
		//フレーム番号で昇順になるように挿入
		if(m_chkpoint_fst->frame > frame){
			m_chkpoint_current->next = m_chkpoint_fst;
			m_chkpoint_fst = m_chkpoint_current;
		
		}else{
			CHKPOINT* chkpoint = m_chkpoint_fst;
			while(chkpoint->next){
				if(chkpoint->next->frame > frame){					
					m_chkpoint_current->next = chkpoint->next;
					chkpoint->next = m_chkpoint_current;
					break;
				}
				chkpoint = chkpoint->next;
			}
			if(chkpoint->next == NULL){
				chkpoint->next = m_chkpoint_current;
				m_chkpoint_current->next = NULL;
			}

			if(chkpoint->frame == frame){
				if(chkpoint->overblow == 0){
					chkpoint->overblow = 1;
				}
			}
		}

	}else{
		m_chkpoint_fst = m_chkpoint_current;
		m_chkpoint_current->next = NULL;
	}

	m_playtm = 0;

}


int CameraGL::PlayCheckPoint(int* pframe, int frameskip){

	if(m_chkpoint_current == NULL){ return 0;}
	if(m_chkpoint_current->next == NULL){
		*pframe = m_chkpoint_current->frame;
		m_target = m_chkpoint_current->target;
		m_camera_position = m_chkpoint_current->camera;
		m_up = m_chkpoint_current->up;
		
		if(m_playtm == 0){
			m_playtm = 1;
			return 1;	//最終チェックポイントの最後の一回だけは成功を返す
		}else{
			return 0;	//終了時は0を返す
		}
	}

	CHKPOINT* next = m_chkpoint_current->next;
	int overblow = m_chkpoint_current->overblow;
	if(overblow == 0){
		overblow = next->frame - m_chkpoint_current->frame;
	}

	double rate = ((double)m_playtm)/(double)(overblow);
	m_target.x = m_chkpoint_current->target.x
			+ (next->target.x - m_chkpoint_current->target.x) * rate;
	m_target.y = m_chkpoint_current->target.y
			+ (next->target.y - m_chkpoint_current->target.y) * rate;
	m_target.z = m_chkpoint_current->target.z
			+ (next->target.z - m_chkpoint_current->target.z) * rate;

	m_camera_position.x = m_chkpoint_current->camera.x
			+ (next->camera.x - m_chkpoint_current->camera.x) * rate;
	m_camera_position.y = m_chkpoint_current->camera.y
			+ (next->camera.y - m_chkpoint_current->camera.y) * rate;
	m_camera_position.z = m_chkpoint_current->camera.z
			+ (next->camera.z - m_chkpoint_current->camera.z) * rate;

	m_up.x = m_chkpoint_current->up.x
			+ (next->up.x - m_chkpoint_current->up.x) * rate;
	m_up.y = m_chkpoint_current->up.y
			+ (next->up.y - m_chkpoint_current->up.y) * rate;
	m_up.z = m_chkpoint_current->up.z
			+ (next->up.z - m_chkpoint_current->up.z) * rate;

	*pframe = m_chkpoint_current->frame
			+ ((next->frame - m_chkpoint_current->frame) * m_playtm)/overblow;

	m_playtm += frameskip;
	while(m_playtm >= overblow){
		m_playtm -= overblow;
		m_chkpoint_current = m_chkpoint_current->next;
		if(m_chkpoint_current->next==NULL){
			m_playtm = 0;
			break;
		}
		overblow = m_chkpoint_current->overblow;
		if(overblow == 0){
			overblow = m_chkpoint_current->next->frame - m_chkpoint_current->frame;
		}
	}

	return 1;
}

//最初まで撒き戻し
void CameraGL::RewindCheckPoint(){

	m_chkpoint_current = m_chkpoint_fst;
	m_playtm = 0;
}


void CameraGL::SaveCheckPoint(const TCHAR* filename){

//ファイル出力
	FILE* fp = _tfopen(filename,_T("w"));
	if( fp == NULL ){  // 関数が失敗していないか
		printf( "ファイルのオープンに失敗しました\n");
		return;  // 異常終了は０以外を返す
	}

	_ftprintf(fp, _T("#checkpoint for movie ver1.1\n"));
	_ftprintf(fp, _T("#frame, overblew, target_x, _y, _z, camera_x, _y, _z, ")
				_T("up_x, _y, _z\n"));
	
	CHKPOINT* chkpoint = m_chkpoint_fst;
	while(chkpoint){
		_ftprintf(fp, _T("%d\t%d\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n"),
				chkpoint->frame,chkpoint->overblow,
				chkpoint->target.x, chkpoint->target.y, chkpoint->target.z,
				chkpoint->camera.x, chkpoint->camera.y, chkpoint->camera.z,
				chkpoint->up.x, chkpoint->up.y, chkpoint->up.z);

		chkpoint = chkpoint->next;
	}	


	fclose(fp);

}


void CameraGL::LoadCheckPoint(const TCHAR* filename){

//ファイル出力
	FILE* fp = _tfopen(filename,_T("r"));
	if( fp == NULL ){  // 関数が失敗していないか
		printf( "ファイルのオープンに失敗しました\n");
		return;  // 異常終了は０以外を返す
	}

	TCHAR chbuf[1024];

	
	
	DeleteAllCheckPoint();
	CHKPOINT* chkpoint;
	
	while(_fgetts(chbuf,1024, fp)){
		
		if(*chbuf == _T('#')){ continue;}

		if(m_chkpoint_fst){
			chkpoint->next = new CHKPOINT;
			chkpoint = chkpoint->next;
		}else{
			chkpoint = m_chkpoint_fst = new CHKPOINT;
		}
		chkpoint->next = NULL;


		_stscanf(chbuf,_T("%d\t%d\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\t%lf\n"),
				&(chkpoint->frame),&(chkpoint->overblow),
				&(chkpoint->target.x), &(chkpoint->target.y), &(chkpoint->target.z),
				&(chkpoint->camera.x), &(chkpoint->camera.y), &(chkpoint->camera.z),
				&(chkpoint->up.x), &(chkpoint->up.y), &(chkpoint->up.z));

	}


	fclose(fp);

}

