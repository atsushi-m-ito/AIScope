#define _USE_MATH_DEFINES
#include <math.h>

#include "winmain.h"

#include "setting.h"

#include "AIScopeProperty.h"

//グローバル変数定義(setting.ini)にてユーザー設定できるもの//

int g_orientation_sensor_enable = 0;	//1: OrientationSensorが有効の場合は使用する, 0:使用不可

int g_gpu_DX11 = 1;		//0:GL1.5, 1:DirextX11. 起動中の変更不可//
int g_vsync_mode = 2;	//0:常に描画,1:垂直同期,2:必要時のみ. 起動中の変更不可//
size_t g_max_vram_size = 1024 * 1024 * 512;
TCHAR g_dxgi_format_str[256] = { 0 };

int g_filter_mode = 0;   //ぼかしフィルターのon/off//

int g_window_width = 640;	//起動中の変更可(ドラッグ)//
int g_window_height = 480;	//起動中の変更可(ドラッグ)//
int s_multiview_mode = 1;	//起動中の変更不可(将来的には可能)//

double g_epsAtomSize = 1.0;		//起動中の変更不可(将来的には可能)//
double g_epsBondSize = 0.5;		//起動中の変更不可(将来的には可能)//
double g_epsBoxLineSize = 0.5;	//起動中の変更不可(将来的には可能)//



int s_visual_atom = 1;	//0:非表示, 1:球ポリ表示, 2:ポイントスプライト. 起動中の変更可(メニュー)//
int s_visual_atompoly = 4;	//起動中の変更不可(将来的には可能)//
int s_visual_atomcolor = 0; //0:原子番号, 1:ボンド数. 起動中の変更可(メニュー)//
int g_visual_atom_trajectory = -1; //起動中の変更不可(将来的には可能)//
double s_visual_atomradius = 0.3;	//起動中の変更不可(将来的には可能)//
int s_visual_bond = 1;		//0:非表示, 1:表示. 起動中の変更可(メニュー)//
int s_visual_bondpoly = 3;	//起動中の変更不可(将来的には可能)//
double g_bond_cutoff = 2.0;	//初期値//
//int g_visual_traj;
double g_visual_trajectory_width = 0.5;	//起動中の変更不可(将来的には可能)//
float g_visual_trajectory_color[4];				//起動中の変更不可(将来的には可能)//
int g_visual_history_frame = 0;             //トラジェクトリーの残像を消す時間, 0なら消さない//
int g_trajectory_z_reverse = 1;

int g_stl_grid_cutoff_edge_z = 0;
double g_stl_grid_width = 1.0;
double g_stl_scale = 1.0;
int g_stl_atompoly = 4;
double g_stl_atomradius = 0.3;	//起動中の変更不可(将来的には可能)//
int g_stl_bondpoly = 3;	//起動中の変更不可(将来的には可能)//

int s_visual_large_threshold = 0x7FFFFFFF;
int s_visual_large_atom;
int s_visual_large_atompoly;
int s_visual_large_atomcolor;
double s_visual_large_atomradius;
int s_visual_large_bond;
int s_visual_large_bondpoly;
int s_visual_large_box = 1;


int s_visual_box = 1;		//0:非表示, 1:表示. 起動中の変更可(メニュー)//				
float s_visual_bg_color[4] = { 0.0, 0.0, 0.0, 0.0 };
float s_visual_large_bg_color[4] = { 0.0, 0.0, 0.0, 0.0 };
static int s_visual_projection = 0;	//0:透視射影, 1:正射影. 起動中の変更可(メニュー)//

double g_visual_split_ratio = 1.0;

double g_visual_max_pressure = 160.1;	//圧力表示の最大値(GPa)

int g_periodic = 1;
mat33f g_default_boxaxis;

int g_LargeBMP_mag = 2;		//起動中の変更不可(将来的には可能)//
int g_DefaultBMP_mag = 1;
TCHAR g_DefaultOutputPath[1024] = { 0 };

double g_view_angle_deg = 27.5589;
double VIEW_ANGLE = g_view_angle_deg * M_PI / 180.0;
double g_movie_delta_angle_deg = 1.0;

double g_color_hight_center = 0.0;
double g_color_hight_range = 0.0;



static int s_field_renderer = 1;	//0:3D texture is sliced to 2D textures, 1:Ray casting//
static double s_field_range_min = 1.0e-6;
static double s_field_range_max = 1.0;
static double s_field_alpha_min = 1.0e-6;
static double s_field_alpha_max = 1.0;


extern BYTE atom_colors[];
extern BYTE bondnum_colors[];




static double GetPrivateProfileDouble(LPCTSTR lpAppName, LPCTSTR lpKeyName, double defaultVal, LPCTSTR lpFileName);
static int GetPrivateProfileFloatArray(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefaultArray, float* farray, LPCTSTR lpFileName);



	
int LoadSettingFile(const TCHAR* filepath){


	//読み出し//
	
	g_gpu_DX11 = GetPrivateProfileInt(_T("GPUDriver"), _T("DirectX11"), g_gpu_DX11, filepath);
	g_vsync_mode = GetPrivateProfileInt(_T("GPUDriver"), _T("Vsync"), g_vsync_mode, filepath);	//0:常に描画,1:垂直同期,2:必要時のみ,3:マニュアル同期モード(OrientationSensor用)
	g_max_vram_size = 1024*1024*(size_t)GetPrivateProfileInt(_T("GPUDriver"), _T("VRAM"), (int)(g_max_vram_size / (1024*1024)), filepath);	//0:常に描画,1:垂直同期,2:必要時のみ.
	GetPrivateProfileString(_T("GPUDriver"), _T("ColorFormat"), _T("R8G8B8A8_UNORM"), g_dxgi_format_str, 256, filepath);

	g_orientation_sensor_enable = GetPrivateProfileInt(_T("TEST"), _T("OrientationSensor"), g_orientation_sensor_enable, filepath);
	g_filter_mode = GetPrivateProfileInt(_T("TEST"), _T("RenderingFilter"), g_filter_mode, filepath);

	g_window_width = GetPrivateProfileInt(_T("WINDOW"), _T("Width"), g_window_width, filepath);
	g_window_height = GetPrivateProfileInt(_T("WINDOW"), _T("Height"), g_window_height, filepath);
	s_multiview_mode = GetPrivateProfileInt(_T("WINDOW"), _T("Multiview"), s_multiview_mode, filepath);
	
	g_epsAtomSize = GetPrivateProfileDouble(_T("EPS"), _T("AtomSize"), g_epsAtomSize, filepath);
	g_epsBondSize = GetPrivateProfileDouble(_T("EPS"), _T("BondSize"), g_epsBondSize, filepath);
	g_epsBoxLineSize = GetPrivateProfileDouble(_T("EPS"), _T("BoxLineSize"), g_epsBoxLineSize, filepath);
	

	
	s_visual_atom = GetPrivateProfileInt(_T("VISUAL"), _T("Atom"), s_visual_atom, filepath);
	s_visual_atompoly = GetPrivateProfileInt(_T("VISUAL"), _T("AtomPoly"), s_visual_atompoly, filepath);
	s_visual_atomcolor = GetPrivateProfileInt(_T("VISUAL"), _T("AtomColor"), s_visual_atomcolor, filepath);
	s_visual_atomradius = GetPrivateProfileDouble(_T("VISUAL"), _T("AtomRadius"), s_visual_atomradius, filepath);
	g_visual_atom_trajectory = GetPrivateProfileDouble(_T("VISUAL"), _T("AtomTrajectory"), g_visual_atom_trajectory, filepath);
	s_visual_bond = GetPrivateProfileInt(_T("VISUAL"), _T("Bond"), s_visual_bond, filepath);
	s_visual_bondpoly = GetPrivateProfileInt(_T("VISUAL"), _T("BondPoly"), s_visual_bondpoly, filepath);
	g_bond_cutoff = GetPrivateProfileDouble(_T("VISUAL"), _T("BondCutoff"), g_bond_cutoff, filepath);
    g_visual_history_frame = GetPrivateProfileInt(_T("VISUAL"), _T("HistoryFrame"), g_visual_history_frame, filepath);


	g_visual_trajectory_width = GetPrivateProfileDouble(_T("VISUAL"), _T("TrajectoryWidth"), g_visual_trajectory_width, filepath);
	GetPrivateProfileFloatArray(_T("VISUAL"), _T("TrajectoryColor"), _T("1.0,1.0,1.0"), g_visual_trajectory_color, filepath);
	
	g_trajectory_z_reverse = GetPrivateProfileInt(_T("VISUAL"), _T("TrajectoryZReverse"), g_trajectory_z_reverse, filepath);

	s_visual_box = GetPrivateProfileInt(_T("VISUAL"), _T("Box"), s_visual_box, filepath);	
	GetPrivateProfileFloatArray(_T("VISUAL"), _T("BGColor"), _T("0.0,0.0,0.0"), s_visual_bg_color, filepath);
	

	s_visual_projection = GetPrivateProfileInt(_T("VISUAL"), _T("Projection"), s_visual_projection, filepath);

	
	g_visual_split_ratio = GetPrivateProfileDouble(_T("VISUAL"), _T("FieldSplit"), g_visual_split_ratio, filepath);
	
	g_visual_max_pressure = GetPrivateProfileDouble(_T("VISUAL"), _T("MaxPressure"), g_visual_max_pressure, filepath);

	g_color_hight_center = GetPrivateProfileDouble(_T("VISUAL"), _T("ColorHeightCenter"), g_color_hight_center, filepath);
	g_color_hight_range = GetPrivateProfileDouble(_T("VISUAL"), _T("ColorHeightRange"), g_color_hight_range, filepath);

	s_field_renderer = GetPrivateProfileInt(_T("VISUAL"), _T("FieldRenderer"), s_field_renderer, filepath);
	s_field_range_min = GetPrivateProfileDouble(_T("VISUAL"), _T("FieldRangeMin"), s_field_range_min, filepath);
	s_field_range_max = GetPrivateProfileDouble(_T("VISUAL"), _T("FieldRangeMax"), s_field_range_max, filepath);
	s_field_alpha_min = GetPrivateProfileDouble(_T("VISUAL"), _T("FieldAlphaMin"), s_field_alpha_min, filepath);
	s_field_alpha_max = GetPrivateProfileDouble(_T("VISUAL"), _T("FieldAlphaMax"), s_field_alpha_max, filepath);


	s_visual_large_box = GetPrivateProfileInt(_T("LARGE"), _T("Box"), s_visual_large_box, filepath);
	s_visual_large_threshold = GetPrivateProfileInt(_T("LARGE"), _T("Threshold"), s_visual_large_threshold, filepath);
	s_visual_large_atom = GetPrivateProfileInt(_T("LARGE"), _T("Atom"), s_visual_atom, filepath);
	s_visual_large_atompoly = GetPrivateProfileInt(_T("LARGE"), _T("AtomPoly"), s_visual_atompoly, filepath);
	s_visual_large_atomcolor = GetPrivateProfileInt(_T("LARGE"), _T("AtomColor"), s_visual_atomcolor, filepath);
	s_visual_large_atomradius = GetPrivateProfileDouble(_T("LARGE"), _T("AtomRadius"), s_visual_atomradius, filepath);	
	s_visual_large_bond = GetPrivateProfileInt(_T("LARGE"), _T("Bond"), s_visual_bond, filepath);
	s_visual_large_bondpoly = GetPrivateProfileInt(_T("LARGE"), _T("BondPoly"), s_visual_bondpoly, filepath);
	//s_visual_large_cutoff = GetPrivateProfileDouble(_T("LARGE"), _T("BondCutoff"), g_bond_cutoff, filepath);
	//g_visual_history_frame = GetPrivateProfileInt(_T("LARGE"), _T("HistoryFrame"), g_visual_history_frame, filepath);
	GetPrivateProfileFloatArray(_T("LARGE"), _T("BGColor"), _T("0.0,0.0,0.0"), s_visual_large_bg_color, filepath);

	
	g_periodic = GetPrivateProfileInt(_T("BASIC"), _T("Periodic"), 3, filepath);

	g_view_angle_deg = GetPrivateProfileDouble(_T("BASIC"), _T("ViewAngle"), g_view_angle_deg, filepath);
	VIEW_ANGLE = g_view_angle_deg * M_PI / 180.0;

	TCHAR line[1024];
	GetPrivateProfileString(_T("BASIC"), _T("DefaultBox"),
				_T("10.0,0.0,0.0,0.0,10.0,0.0,0.0,0.0,10.0"), line, 1024, filepath);
	TCHAR* tp = _tcstok(line, _T(" ,\t\r\n"));
	{
		float elements[9];
		for (int i = 0; i < 9; i++) {
			elements[i] = (float)_tcstod(tp, NULL);
			tp = _tcstok(NULL, _T(" ,\t\r\n"));
		}
		g_default_boxaxis.Set(elements);
	}
	
	g_LargeBMP_mag = GetPrivateProfileInt(_T("OUTPUT"), _T("LargeBmpMagnify"), g_LargeBMP_mag, filepath);
	g_DefaultBMP_mag = GetPrivateProfileInt(_T("OUTPUT"), _T("DefaultBmpMagnify"), g_DefaultBMP_mag, filepath);
	GetPrivateProfileString(_T("OUTPUT"), _T("DefaultPath"), _T(""), g_DefaultOutputPath, 1024, filepath);

	g_movie_delta_angle_deg = GetPrivateProfileDouble(_T("OUTPUT"), _T("RotationAngleDelta"), g_movie_delta_angle_deg, filepath);
	
	//STL file
	g_stl_grid_width = GetPrivateProfileDouble(_T("OUTPUT"), _T("STLGridWidth"), g_stl_grid_width, filepath);
	g_stl_grid_cutoff_edge_z = GetPrivateProfileInt(_T("OUTPUT"), _T("STLGridCutoffEdgeZ"), g_stl_grid_cutoff_edge_z, filepath);
	g_stl_scale = GetPrivateProfileDouble(_T("OUTPUT"), _T("STLScale"), g_stl_scale, filepath);	
	g_stl_atompoly = GetPrivateProfileInt(_T("OUTPUT"), _T("STLAtomPoly"), g_stl_atompoly, filepath);
	g_stl_atomradius = GetPrivateProfileDouble(_T("OUTPUT"), _T("AtomRadius"), g_stl_atomradius, filepath);
	g_stl_bondpoly = GetPrivateProfileInt(_T("OUTPUT"), _T("BondPoly"), g_stl_bondpoly, filepath);


	//color//
	const int sz = 1024*128;
	TCHAR buf[sz];
	GetPrivateProfileSection(_T("ATOM"), buf, sz, filepath);
	for(TCHAR* p = buf; *p; p += _tcslen(p)+1){
		if(_tcsncmp(p, _T("Color="), 6) == 0){
			
			TCHAR* q = _tcschr(p+6, _T(','));
			*q = _T('\0');
			int Z = GetAtomicNumber(p+6);
			
			q++;
			double val = _tcstod(q, &q);
			atom_colors[Z * 4] = (BYTE)(255.0 * val);
			q++;
			val = _tcstod(q, &q);
			atom_colors[Z * 4+1] = (BYTE)(255.0 * val);
			q++;
			val = _tcstod(q, &q);
			atom_colors[Z * 4+2] = (BYTE)(255.0 * val);
			q++;
			val = _tcstod(q, &q);
			atom_colors[Z * 4+3] = (BYTE)(255.0 * val);
			
		}else if(_tcsncmp(p, _T("ColorNBond="), 11) == 0){
			
			TCHAR* q = _tcschr(p+11, _T(','));
			*q = _T('\0');
			int num = _ttoi(p+11);
			
			q++;
			double val = _tcstod(q, &q);
			bondnum_colors[num * 4] = (BYTE)(255.0 * val);
			q++;
			val = _tcstod(q, &q);
			bondnum_colors[num * 4+1] = (BYTE)(255.0 * val);
			q++;
			val = _tcstod(q, &q);
			bondnum_colors[num * 4+2] = (BYTE)(255.0 * val);
			q++;
			val = _tcstod(q, &q);
			bondnum_colors[num * 4+3] = (BYTE)(255.0 * val);
		}
	}

	return 0;

}


double GetPrivateProfileDouble(LPCTSTR lpAppName, LPCTSTR lpKeyName, double defaultVal, LPCTSTR lpFileName){

	const int bufsz = 1024;
	TCHAR buffer[bufsz];

	GetPrivateProfileString(lpAppName, lpKeyName, _T("S"), buffer, bufsz, lpFileName);
	if (*buffer == _T('S')){
		return defaultVal;
	}else{
		return _tcstod(buffer, NULL);
	}

}

int GetPrivateProfileFloatArray(LPCTSTR lpAppName, LPCTSTR lpKeyName, LPCTSTR lpDefaultArray, float* farray, LPCTSTR lpFileName){

	const int bufsz = 1024;
	TCHAR buffer[bufsz];

	GetPrivateProfileString(lpAppName, lpKeyName, lpDefaultArray, buffer, bufsz, lpFileName);
	float* p = farray;
	for(TCHAR* tp = _tcstok(buffer,_T(" ,\t")); tp != NULL; tp = _tcstok(NULL,_T(" ,\t")) ){
		*p = (float)_tcstod(tp, NULL);
		p++;
	}
	return (int)(p - farray);
}


void ApplySetting(AIScopeProperty* aiproperty, int num_atoms){

	aiproperty->ChangeProperty(AISCUPE_KEY_PROJECTION, s_visual_projection);
	aiproperty->ChangeProperty(AISCUPE_KEY_MULTIVIEW, s_multiview_mode);

	if (num_atoms >= s_visual_large_threshold) {
		aiproperty->ChangeProperty(AISCUPE_KEY_BOX_DRAW, s_visual_large_box);
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_DRAW, s_visual_large_atom);
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_COLOR, s_visual_large_atomcolor);
		aiproperty->ChangeProperty(AISCUPE_KEY_BOND_DRAW, s_visual_large_bond);
		aiproperty->ChangeProperty(AISCUPE_KEY_BOND_POLY, s_visual_large_bondpoly);
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_POLY, s_visual_large_atompoly);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_ATOM_RADIUS, s_visual_large_atomradius);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_BG_COLOR_R, s_visual_large_bg_color[0]);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_BG_COLOR_G, s_visual_large_bg_color[1]);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_BG_COLOR_B, s_visual_large_bg_color[2]);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_BG_COLOR_A, s_visual_large_bg_color[3]);
	}
	else {
		aiproperty->ChangeProperty(AISCUPE_KEY_BOX_DRAW, s_visual_box);
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_DRAW, s_visual_atom);
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_COLOR, s_visual_atomcolor);
		aiproperty->ChangeProperty(AISCUPE_KEY_BOND_DRAW, s_visual_bond);
		aiproperty->ChangeProperty(AISCUPE_KEY_BOND_POLY, s_visual_bondpoly);
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_POLY, s_visual_atompoly);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_ATOM_RADIUS, s_visual_atomradius);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_BG_COLOR_R, s_visual_bg_color[0]);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_BG_COLOR_G, s_visual_bg_color[1]);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_BG_COLOR_B, s_visual_bg_color[2]);
		aiproperty->ChangePropertyDouble(AISCUPE_KEY_BG_COLOR_A, s_visual_bg_color[3]);
	}

	aiproperty->ChangeProperty(AISCUPE_KEY_FIELD_RENDERMODE, s_field_renderer);
	aiproperty->ChangePropertyDouble(AISCUPE_KEY_FIELD_RANGE_MIN, s_field_range_min);
	aiproperty->ChangePropertyDouble(AISCUPE_KEY_FIELD_RANGE_MAX, s_field_range_max);
	aiproperty->ChangePropertyDouble(AISCUPE_KEY_FIELD_ALPHA_MIN, s_field_alpha_min);
	aiproperty->ChangePropertyDouble(AISCUPE_KEY_FIELD_ALPHA_MAX, s_field_alpha_max);
}