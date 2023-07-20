#pragma warning(disable : 4996)
#define UNIVERSAL_1
#define _USE_MATH_DEFINES

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。
#include "targetver.h"
#include <windows.h>
#include "winmain.h"
#include "AIScope.h"
#ifdef UNIVERSAL_1
#include "IGraphicDevice.h"
#endif
//#include "AIScopeProperty.h"
#include "UICamera.h"
#include "md3_writer2.h"
#include "setting.h"

extern AIScope g_aiscope;
extern UICamera g_uicamera;
extern FILETIME g_filetime;
extern int g_timeframe;

extern IGraphicDevice* graphic_device;



void OutputBMP(int number, HWND hWnd) {

	int default_mag = g_DefaultBMP_mag;

	double view_matrix[16];
	g_uicamera.GetViewMatrix(view_matrix);
	double focus_distance = g_uicamera.GetFocusDistance();

	TCHAR spath[2048];
	if (_tcslen(g_DefaultOutputPath) == 0) {
		g_aiscope.GetFilePath(spath);

	} else {
		_tcscpy(spath, g_DefaultOutputPath);
		if (*(spath + _tcslen(spath) - 1) != _T('\\')) {
			_tcscat(spath, _T("\\"));
		}

		g_aiscope.GetFileName(spath + _tcslen(spath));

	}

	TCHAR* p = _tcsrchr(spath, _T('.'));
	if (p) { *p = _T('\0'); }

	size_t length = _tcslen(spath);
	if (number < 10) {
		_stprintf(spath + length, _T("000%d.bmp"), number);
	} else if (number < 100) {
		_stprintf(spath + length, _T("00%d.bmp"), number);
	} else if (number < 1000) {
		_stprintf(spath + length, _T("0%d.bmp"), number);
	} else {
		_stprintf(spath + length, _T("%d.bmp"), number);
	}


	graphic_device->BeginRendering();

	if (g_gpu_DX11 == 0) {
		HDC hDC = GetDC(hWnd);
		HGLRC hRC = GetSharedRC(hWnd);
		wglMakeCurrent(hDC, hRC);
		g_aiscope.OutputBMPMagGL(spath, default_mag, view_matrix, focus_distance);
		//AI_getBMfromGL(spath, GL_FRONT);
		wglMakeCurrent(NULL, NULL);
		ReleaseDC(hWnd, hDC);
	} else {
		//if (g_aiscope.GetFileType() == FILETYPE_FIELD) {
        //	g_aiscope.OutputFieldRaytracing(spath, default_mag, view_matrix, focus_distance);
		//	} else {
		g_aiscope.OutputBMPMagDX11(spath, default_mag, view_matrix, focus_distance);
		//}
	}
	graphic_device->EndRendering();
}

void OutputLargeBMP(HWND hWnd) {

	double view_matrix[16];
	g_uicamera.GetViewMatrix(view_matrix);
	double focus_distance = g_uicamera.GetFocusDistance();

	const int mag = g_LargeBMP_mag;


	TCHAR spath[2048];
	if (_tcslen(g_DefaultOutputPath) == 0) {
		g_aiscope.GetFilePath(spath);

	} else {
		_tcscpy(spath, g_DefaultOutputPath);
		if (*(spath + _tcslen(spath) - 1) != _T('\\')) {
			_tcscat(spath, _T("\\"));
		}

		g_aiscope.GetFileName(spath + _tcslen(spath));

	}


	TCHAR* p = _tcsrchr(spath, _T('.'));
	if (p) { *p = _T('\0'); }
	//length = (int)(current->ext - current->filepath - 1);
	//_tcsncpy(spath, current->filepath, length);
	size_t length = _tcslen(spath);
	_stprintf(spath + length, _T("_x%d_%d.bmp"), mag, g_timeframe);

	graphic_device->BeginRendering();
	if (g_gpu_DX11 == 0) {
		HDC hDC = GetDC(hWnd);
		HGLRC hRC = GetSharedRC(hWnd);
		wglMakeCurrent(hDC, hRC);
		g_aiscope.OutputBMPMagGL(spath, mag, view_matrix, focus_distance);
		//AI_getBMfromGLMag(dat, current->atomRenderer, spath, 10);
		wglMakeCurrent(NULL, NULL);
		ReleaseDC(hWnd, hDC);
	} else {
		g_aiscope.OutputBMPMagDX11(spath, mag, view_matrix, focus_distance);
	}
	graphic_device->EndRendering();

}

void OutputSequentialStereoBMP(HWND hWnd) {

	/*
	int res = 0;
	while(res==0){
	//1フレーム進める
	OutputStereoBMP(hWnd);
	Sleep(0);

	res = PlayMovie();
	}
	*/

}

void OutputSequentialBMP(HWND hWnd) {

	static int number = 0;

	int res = 0;
	while (res == 0) {


		//画像作り
		OutputBMP(number, hWnd);

		Sleep(0);
		//1フレーム進める
		res = PlayMovie();
		number++;
	}

}



void OutputSequentialMD3(HWND hWnd) {
	TCHAR spath[1024 * 24];
	g_aiscope.GetFilePath(spath);
	TCHAR* p = _tcsrchr(spath, _T('.'));
	if (p) { *p = _T('\0'); }
	//length = (int)(current->ext - current->filepath - 1);
	//_tcsncpy(spath, current->filepath, length);
	size_t length = _tcslen(spath);
	_stprintf(spath + length, _T("_all.md3"), g_timeframe);


	char* mbfpath = MbsDup(spath);
#if 1
	msz::MD3_Writer2<float> md3;
	if (!md3.Open(mbfpath)) {
		delete[] mbfpath;
		return;
	}
	delete[] mbfpath;


	int res = 0;
	while (res == 0) {

		ATOMS_DATA* dat = g_aiscope.GetAtomData();

		float box_f[12];
		*((mat33f*)box_f) = dat->boxaxis;
		*((vec3f*)(box_f + 9)) = dat->boxorg;

		md3.BeginFrame(dat->pcnt);
		md3.WriteBox(box_f);
		md3.WriteZ(dat->knd);
		md3.WriteR(dat->r);
		md3.WriteP(dat->p);
		md3.WriteComment("converted by AIScope");
		md3.EndFrame();

		//Sleep(0);
		//1フレーム進める
		res = PlayMovie();
	}
	md3.Close();

#else
	MD3_Writer md3;
	int ierr = md3.Open(mbfpath);
	delete[] mbfpath;

	int res = 0;
	while (res == 0) {

		ATOMS_DATA* dat = g_aiscope.GetAtomData();

		float box_f[12];
		*((mat33f*)box_f) = dat->boxaxis;
		*((vec3f*)(box_f + 9)) = dat->boxorg;

		md3.Write_f(dat->pcnt, dat->knd, (float*)dat->r, (float*)dat->p, box_f, "converted by AIScope");

		Sleep(0);
		//1フレーム進める
		res = PlayMovie();
	}
	md3.Close();
#endif

}

void OutputSequentialBMP_Trace(HWND hWnd) {


	if (g_aiscope.GetFileType() != FILETYPE_ATOM) {
		return;
	}

	g_aiscope.GetAtomData()->r[1];

	static int number = 0;

	int res = 0;
	while (res == 0) {


		//カメラの位置の取得//
		vec3d cam_pos = g_aiscope.GetAtomData()->r[0];
		cam_pos.z += 3.0;
		cam_pos.y += 1.0;
		vec3d target = cam_pos;
		target.z -= 10.0;
		vec3d up;
		up.Set(0.0, 1.0, 0.0);

		g_uicamera.SetPosition(cam_pos, target, up);

		//画像作り
		OutputBMP(number, hWnd);

		Sleep(0);
		//1フレーム進める
		res = PlayMovie();
		number++;
	}

}

void OutputRotationalBMP(HWND hWnd, double radian) {



	const int split = (int)(2.0 * M_PI / radian);

	for (int i = 0; i <= split; i++) {


		//画像作り
		OutputBMP(i, hWnd);
		Sleep(0);

		//右に回転する

		g_uicamera.RotateOnUpAxis(radian);

	}

}



void OutputSequentialBMP_Rotation(HWND hWnd, int rotate_axis, double radian) {

	static int number = 0;

	double total_radian = 0.0;

	int res = 0;
	while (res == 0) {


		//画像作り
		OutputBMP(number, hWnd);

		Sleep(0);
		//1フレーム進める
		res = PlayMovie();
		number++;


		if (rotate_axis != 0) {
			total_radian += radian;
			if (total_radian > 2.0 * M_PI) {
				total_radian -= 2.0 * M_PI;
			} else if (total_radian < -2.0 * M_PI) {
				total_radian += 2.0 * M_PI;
			} else {
				res = 0; //ちょうど一回転する時まで抜けないように
			}
		}

		if (res == 0) {
			if (rotate_axis == 1) {
				//右に回転する
				g_uicamera.RotateOnUpAxis(radian);
			} else if (rotate_axis == 2) {
				//右に回転する
				g_uicamera.RotateOnLeftAxis(radian);
			}
		}
	}

}

