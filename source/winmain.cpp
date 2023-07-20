// AIScope3.cpp : アプリケーション用のエントリ ポイントの定義
//

#pragma warning(disable : 4996)
#define UNIVERSAL_1
#define _USE_MATH_DEFINES

#include "targetver.h"
#include <windows.h>
#include <windowsx.h>

// C ランタイム ヘッダー ファイル
#include <stdlib.h>
#include <math.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

#include <shlwapi.h>
#pragma comment(lib, "shlwapi.lib")


#include <locale.h>

#include <shellapi.h>

#include <process.h>

#include <mmsystem.h>
#pragma comment( lib, "winmm.lib" )

#include "resource.h"
#include "quotationtok.h"

#include "winmain.h"

#define AISCOPE_GLOBAL
#include "atomic_color.h"
#include "atomic_element.h"

#ifdef UNIVERSAL_1
#include "IGraphicDevice.h"
#include "GraphicDeviceDX11.h"
#include "GraphicDeviceDX11withD2D.h"
#include "GraphicDeviceGL15.h"

#endif

#include "AIScope.h"
#include "AIScopeProperty.h"
#include "UICamera.h"
#include "UIPanel.h"
#include "MyPanelID.h"
#include "NotifyDistributer.h"
#include "setting.h"
#include "UserCrossSection.h"
#include "OrientationSensor.h"

#include "drawpanel.h"


#include "OrientationSensor.h"



#define MAX_LOADSTRING 100

HINSTANCE g_hInst;					// 現在のインスタンス
TCHAR szTitle[] = _T("AIScope 3.8.5");
TCHAR szWindowClass[] = _T("AIScopeWindow");			// タイトル バー テキスト

double g_FPS = 0.0;


// このコード モジュールに含まれる関数の前宣言:
HWND InitWindow( HINSTANCE, int );
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

bool g_windowmode = true;	//windowmode or fullscreen.
bool g_renderingLock = false;	//レンダリングをしないようにLockする。メモリ転送など.
bool g_updateFlag = true;	//レンダリングをしないようにLockする。メモリ転送など.


//////////////////////////////////////////////////////////////////
int g_lastcheck = 0;	//1なら最後のフレームまでチェックポイント再生する

FILETIME g_filetime;


int AutoBMP = 0;//0:none, 1:auto saving bitmap

/*
* g_mouse_toggle : state of mouse operation
* 0: no operation
* 10: toggle for translation of camera
* 20: toggle for rotaion of camera
* 110: toggle for translation of user cross section
* 120: toggle for rotaion of user cross section
* 210: toggle for translation of user cross section
* 220: toggle for rotaion of user cross section
* 310: toggle for translation of user cross section
* 320: toggle for rotaion of user cross section
*/
int g_mouse_toggle = 0;

int g_client_w;
int g_client_h;


int g_timeframe = 0;
int g_frameskip = 1;	//step number of frame skip
const int g_iFPS = 20;	//seconds per frame
int g_movie=0;
int g_movie_mode = 0;
UINT_PTR g_timerID;



const int MS_PAR_FRAME = 32;

//編集モード//
int g_edit_mode = 0;


//関数========================
//void DeleteTrajectory();

extern double g_epsAtomSize;
extern double g_epsBondSize;
extern double g_epsBoxLineSize;


static NotifyDistributer notify_distributer;
static AIScopeProperty aiproperty;
AIScope g_aiscope(&aiproperty);
UICamera g_uicamera;
UIPanel g_uipanel;

namespace {

	UserCrossSection s_cross_sections[3]{
		{vec3d{0.0,0.0,0.0}, vec3d{0.0,0.0,1.0}},
		{vec3d{0.0,0.0,0.0}, vec3d{1.0,0.0,0.0}},
		{vec3d{0.0,0.0,0.0}, vec3d{0.0,-1.0,0.0}},
	};
	bool s_cross_section_effective[3]{ false,false,false };
	int s_cross_section_and_or[3]{ 0,0,0 };

	bool SetCrossSection(int id) {
		return aiproperty.SetCrossSection(id, s_cross_section_effective[id], s_cross_sections[id].position, s_cross_sections[id].normal, s_cross_section_and_or[id]);
	}

	void ResetCrossSection(const vec3d& box_center) {
		s_cross_sections[0].position = box_center;
		s_cross_sections[1].position = box_center;
		s_cross_sections[2].position = box_center;
		SetCrossSection(0);
		SetCrossSection(1);
		SetCrossSection(2);
	}

	int CameraOrCross() {
		if (GetKeyState('1') < 0) { //ユーザー定義断面の操作//
			return 1;
		}
		else if (GetKeyState('2') < 0) { //ユーザー定義断面の操作//
			return 2;
		}
		else if (GetKeyState('3') < 0) { //ユーザー定義断面の操作//
			return 3;
		}
		return 0; //0 indicates camera//
	}

	/*
	NG: 3key同時押しはハードウェア制限で検出できないPCが多い.
	//ユーザー定義断面の操作なら1以上を返す//
	//n番目の断面トグル中はn番目のbitが立つ//
	int CameraOrCross2() {
		return ((GetKeyState('1') < 0) ? 1 : 0)
			+ ((GetKeyState('2') < 0) ? 2 : 0)
			+ ((GetKeyState('3') < 0) ? 4 : 0);
	}
	*/
}

void SetCrossSectionMode(int and_or1, int and_or2, int and_or3) {
	s_cross_section_and_or[0] = and_or1;
	s_cross_section_and_or[1] = and_or2;
	s_cross_section_and_or[2] = and_or3;
	SetCrossSection(0);
	SetCrossSection(1);
	SetCrossSection(2);
}

void SetCrossSectionTarget(int target) {
	aiproperty.SetCrossSectionTarget(target);
}

GraphicDeviceDX11withD2D* d3d_device_with_d2d = nullptr;
IGraphicDevice* graphic_device = nullptr;
PanelD2D* g_panel = nullptr;




//メイン関数//////////////////////////////////

int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
{
	
	
	CoInitializeEx(NULL, COINIT_MULTITHREADED);


	//設定ファイルの読み込み//
	TCHAR settingpath[1024];
	SeekFilePath(_T("setting.ini"), settingpath, 1024);
	LoadSettingFile(settingpath);
	
	if(g_orientation_sensor_enable == 1){
		//デバイスを持っていない場合は0が入る
		g_orientation_sensor_enable = OrientationSensor_Initialize();
		g_vsync_mode = 3;//マニュアルフレームレートモード//
	}


	
	

//Windowアプリケーションの初期化/////////////////////////


	HWND hWnd = InitWindow( hInstance, nCmdShow );//WM_CREATEが同時に実行される//
	if( ! hWnd) 
	{
		return FALSE;
	}


	HACCEL hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_AISCOPE2);

/////////////////////////////////Windowアプリケーションの初期化//


//描画ライブラリの初期化//////////////////////////////////
//WM_CREATEはすでに呼ばれた後/////////////////////////


	//描画ライブラリの初期化//////////////////////////////////
	//WM_CREATEはすでに呼ばれた後/////////////////////////

	

	
	//DX11_CORE dx11_core;

	if (g_gpu_DX11 == 1){

		//DirectX11の初期化//////////////////////////
#if 1
		d3d_device_with_d2d = new GraphicDeviceDX11withD2D(hWnd, _tcscmp(g_dxgi_format_str, _T("R16G16B16A16_FLOAT")) == 0 ? DXGI_FORMAT_R16G16B16A16_FLOAT :DXGI_FORMAT_R8G8B8A8_UNORM);
		graphic_device = d3d_device_with_d2d;
#else
		graphic_device = new GraphicDeviceDX11(hWnd, _tcscmp(g_dxgi_format_str, _T("R16G16B16A16_FLOAT")) == 0 ? DXGI_FORMAT_R16G16B16A16_FLOAT : DXGI_FORMAT_R8G8B8A8_UNORM);
#endif

		RECT rc;
		GetClientRect(hWnd, &rc);
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;

		if (graphic_device->InitializeWindow(width, height, g_windowmode, g_vsync_mode)){
			delete graphic_device;
			return 0;
		}

		if (d3d_device_with_d2d) {
			ID2D1DeviceContext* d2ddc = d3d_device_with_d2d->GetD2DDeviceContext();
			g_panel = new PanelD2D(d2ddc);
			d2ddc->Release();
		}


	} else{

		//OpenGLの初期化/////////////////////////////////////////////
		graphic_device = new GraphicDeviceGL15(hWnd);


		RECT rc;
		GetClientRect(hWnd, &rc);
		UINT width = rc.right - rc.left;
		UINT height = rc.bottom - rc.top;

		int result = graphic_device->InitializeWindow(width, height, g_windowmode, g_vsync_mode);
		if (result == 3){
			g_vsync_mode = 3;
		}else if(result){
			delete graphic_device;
			return 0;
		}



	}




////////////////////////////////////描画ライブラリの初期化//

//UIPanelの初期化/////////////////////////////////////////////////////////

	InitializeUI(g_uipanel);



/////////////////////////////////////////////////////////UIPanelの初期化//

//FPS計測初期化/////////////////////////////////////////////////////
	timeBeginPeriod(1);
	DWORD prev_time = timeGetTime();
	DWORD prev_time_manual_swap = 0;
	int frame = 0;

//メインループ(ゲームループ)///////////////////////////////////
	bool is_rendering = true;
	MSG msg = {0};
	while( WM_QUIT != msg.message )
    {

		//レンダリング
		if (is_rendering) {

			if (g_orientation_sensor_enable == 1) {

				float matrix[9];
				int res = OrientationSensor_GetRotationMatrix(matrix);
				if (res == 1) {
					g_uicamera.SetRotationMatrix(matrix);
				}
			}

			//レンダリング関数の呼び出し//////////////

			if (g_aiscope.ExistData()) {//データがあるときだけ描画//
				double view_matrix[16];
				g_uicamera.GetViewMatrix(view_matrix);
				double focus_distance = g_uicamera.GetFocusDistance();

				if (g_gpu_DX11) {

					//DirectXでの描画//
					graphic_device->BeginRendering();

					g_aiscope.DisplayDX11(view_matrix, focus_distance);

					if (d3d_device_with_d2d) {
						ID2D1DeviceContext* d2dDeviceContext = d3d_device_with_d2d->GetD2DDeviceContext();
						d2dDeviceContext->BeginDraw();
						g_panel->Draw();
						d2dDeviceContext->EndDraw();
						d2dDeviceContext->Release();
					}
					//バックバッファのフリップ.
					graphic_device->EndRendering();

				} else {

					//OpenGLでの描画//
					graphic_device->BeginRendering();


					g_aiscope.DisplayGL15(view_matrix, focus_distance);

					glFlush();

					DrawUIPanelGL15(g_client_w, g_client_h, g_uipanel);

					glFlush();


					graphic_device->EndRendering();

				}

				frame++;
			}

			////////////////////レンダリング終了//


			//FPS表示
			if (frame == 100) {
				DWORD time = timeGetTime();
				g_FPS = ((double)frame) * 1000.0 / (double)(time - prev_time);
				ResetWindowText(hWnd);

				frame = 0;
				prev_time = time;

			}

			Sleep(0);


		}
		
		is_rendering = true;

		//Windowメッセージ処理//////////////////////////
		if(g_vsync_mode != 2){
			if( PeekMessage( &msg, NULL, 0, 0, PM_REMOVE ) ) {	
				
				if (WM_PAINT != msg.message) {
					is_rendering = false;
				}

				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
				
			}else if(g_vsync_mode == 3){
				DWORD tm = timeGetTime();
				if(tm - prev_time_manual_swap > MS_PAR_FRAME){
					prev_time_manual_swap = tm;
				}else{
					is_rendering = false;
				}
			}
		}else{
			if( GetMessage(&msg, NULL, 0, 0)){
				if (WM_PAINT != msg.message) {
					is_rendering = false;
				}

				if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) {
					TranslateMessage(&msg);
					DispatchMessage(&msg);					
				}
				
			}else{
				//GetMessageの戻り値が0なのでWM_QUITを取得しているはず.
				break;//ループを抜ける.
			}
		}

    }
//////////////////////////////////メインループ(ゲームループ)//

	timeEndPeriod(1);
	
	if(g_orientation_sensor_enable==1){
		OrientationSensor_Terminate();
	}

//描画ライブラリの解放////////////////////////////////////////////////////

	delete g_panel;
	delete graphic_device;
	

	return (int)(msg.wParam);
}


HWND InitWindow( HINSTANCE hInstance, int nCmdShow ){
	
	WNDCLASSEX wcex;
	wcex.cbSize = sizeof(WNDCLASSEX); 
	wcex.style		= CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon		= LoadIcon(hInstance, (LPCTSTR)IDI_AISCOPE);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCTSTR)IDC_AISCOPE2;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

	if( !RegisterClassEx( &wcex ) )
        return NULL;

	
	g_hInst = hInstance; // グローバル変数にインスタンス ハンドルを保存

	
	HWND hWnd;
	RECT rc = { 0, 0, g_window_width, g_window_height};

	//windowmode
	if(g_windowmode){
		//windowアプリケーション	
		DWORD dwStyle = WS_OVERLAPPEDWINDOW;
		AdjustWindowRect(&rc, dwStyle, TRUE); // 第三引数はメニューの有無
		hWnd = CreateWindow(szWindowClass, szTitle, dwStyle,
				  CW_USEDEFAULT, 0, rc.right-rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL);

	}else{   
		//fullscreen
		hWnd = CreateWindow( szWindowClass, szTitle, WS_VISIBLE | WS_POPUP,
                           0, 0, rc.right - rc.left, rc.bottom - rc.top, NULL, NULL, hInstance, NULL );
	}


	if( hWnd ){
	    ShowWindow( hWnd, nCmdShow );//win10,1809よりWM_PAINTが最初のメッセージループ前に実行されてしまうようになった.
	//	UpdateWindow( hWnd );先にこれを呼ぶと、WM_PAINTが最初のメッセージループ前に実行されてしまう.
	}
	
	return hWnd;
}


//
//  関数: WndProc(HWND, unsigned, WORD, LONG)
//
//  用途: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND	- アプリケーション メニューの処理
//  WM_PAINT	- メイン ウィンドウの描画
//  WM_DESTROY	- 終了メッセージの通知とリターン
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	
	//メニュー関連
	//static HMENU hMenu;	
	double view_matrix[16];

	switch( message ) {
	case WM_CREATE:
		{	
			///////////////////////////////////////
			//		メニューGUIの初期化
			///////////////////////////////////////
			InitializewinMenu(GetMenu(hWnd), &notify_distributer);
			aiproperty.RegisterNotifyReciever(&notify_distributer);
			ApplySetting(&aiproperty);//setting.iniの値を反映させる(タイミングとしてここで行うのは暫定)//

			//ファイルオープン
			TCHAR* buf = _tcsdup(GetCommandLine());
			quotationtok(buf, _T(" \t\r\n;"));
			TCHAR* filename = quotationtok(NULL, _T(" \t\n\r"));	//ファイル名の取得""で囲まれた場合の対策.
			if(filename){
				g_renderingLock = true;

				g_aiscope.OpenFile(filename);

				if (g_aiscope.ExistData()){
					double focus_distance;
					vec3d target_position;
					g_aiscope.GetFocusInfomation(&focus_distance, &target_position);
					g_uicamera.ResetFocus(focus_distance, target_position);
					ResetCrossSection(target_position);

					ApplySetting(&aiproperty, g_aiscope.GetNumAtoms());
				}

				ResetCurrentFileMenu(hWnd, g_aiscope.GetFileType());
				ResetWindowText(hWnd);

				g_renderingLock = false;

			}
			delete [] buf;

			//ファイルドロップの許可.
			DragAcceptFiles(hWnd, TRUE);		
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	
	case WM_CLOSE:
		TerminateMenu();
		DestroyWindow(hWnd);
		break;
	case WM_DESTROY:
		
		
		PostQuitMessage( 0 );
		break;

	case WM_SIZE:	//初回の描画の前に必ず飛んでくる
		g_client_w = (int)LOWORD(lParam);
		g_client_h = (int)HIWORD(lParam);

		if (graphic_device){
			graphic_device->Resize(g_client_w, g_client_h);
		}


		g_aiscope.Resize(g_client_w, g_client_h);

		break;
		
	////ムービー用タイマー/////////////////////////////////////////////
	case WM_TIMER:
		
		switch(g_movie){
		case 1:		//フレーム自動送り
			{
				g_timeframe += g_frameskip;
				int actualframe = g_aiscope.FrameJump( g_timeframe);
				if(actualframe < g_timeframe){	//フレームが最後に到達した.
					g_timeframe = actualframe;	//実際のtimeframeをセット
					g_movie=0;
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;

		case 3:		//フレーム自動逆送り
			{
				g_timeframe -= g_frameskip;
				int actualframe = g_aiscope.FrameJump( g_timeframe);
				if(actualframe > g_timeframe){	//フレームが最後に到達した.
					g_timeframe = actualframe;	//実際のtimeframeをセット
					g_movie=0;
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
			
			break;
		
		
		case 2:	//ムービー再生
			{
				//if(g_checkpoint->Play(&frametgt, &cam_info, g_frameskip)){
				if(PlayMovie()){
					g_movie = 0;	//ムービーストップ
				}
				InvalidateRect(hWnd, NULL, FALSE);
			}
			break;
		default:
			AutoBMP = 0;
			KillTimer(hWnd, g_timerID);
			break;
		}
		ResetWindowText(hWnd);

		
		break;

	/////////キー入力処理////////////////////////////////////
	case WM_KEYDOWN:
		if(! (g_aiscope.ExistData())) {
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		switch(wParam){

		//カメラ回転////////////////////////////
		case VK_PRIOR:
			g_uicamera.ZoomIn(-0.1);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_NEXT:
			g_uicamera.ZoomIn(0.1);
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case VK_LEFT:
		{
			int id = CameraOrCross();
			if (id == 0) {//to operate camera
				if (GetKeyState(VK_CONTROL) < 0) {
					//平行移動(カメラの位置とセンターを平行移動)
					g_uicamera.TranslateLeft(-0.2);
				}
				else {
					g_uicamera.RotateOnUpAxis(-M_PI / 90.0);
				}
			}
			else {
				g_uicamera.GetViewMatrix(view_matrix);
				s_cross_sections[id - 1].RotateOnUpAxis(-M_PI / 90.0, view_matrix);
				SetCrossSection(id - 1);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}			
		case VK_RIGHT:
		{
			int id = CameraOrCross();
			if (id == 0) {//to operate camera
				if (GetKeyState(VK_CONTROL) < 0) {
					//平行移動(カメラの位置とセンターを平行移動)
					g_uicamera.TranslateLeft(0.2);
				}
				else {
					g_uicamera.RotateOnUpAxis(M_PI / 180.0);
				}
			}
			else {
				g_uicamera.GetViewMatrix(view_matrix);
				s_cross_sections[id - 1].RotateOnUpAxis(M_PI / 180.0, view_matrix);
				SetCrossSection(id - 1);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}			
		case VK_UP:
		{
			int id = CameraOrCross();
			if (id == 0) {//to operate camera
				if (GetKeyState(VK_CONTROL) < 0) {
					//平行移動(カメラの位置とセンターを平行移動)
					g_uicamera.TranslateUp(-0.2);
				}
				else {
					g_uicamera.RotateOnLeftAxis(M_PI / 90.0);
				}
			}
			else {
				g_uicamera.GetViewMatrix(view_matrix);
				s_cross_sections[id - 1].RotateOnRightAxis(M_PI / 180.0, view_matrix);
				SetCrossSection(id - 1);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		case VK_DOWN:
		{
			int id = CameraOrCross();
			if (id == 0) {//to operate camera
				if (GetKeyState(VK_CONTROL) < 0) {
					//平行移動(カメラの位置とセンターを平行移動)
					g_uicamera.TranslateUp(0.2);
				}
				else {
					g_uicamera.RotateOnLeftAxis(-M_PI / 90.0);
				}
			}
			else {
				g_uicamera.GetViewMatrix(view_matrix);
				s_cross_sections[id - 1].RotateOnRightAxis(-M_PI / 180.0, view_matrix);
				SetCrossSection(id - 1);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		}
		///////////////////////////カメラ回転///
		
		//ユーザー定義断面/////////////////////////	
		case '1':
		case '2':
		case '3':
			{
				//ユーザー定義断面の有効無効切り替え//
				const int id = wParam - '1';
				s_cross_section_effective[id] = !(GetKeyState(VK_SHIFT) < 0);
				if (SetCrossSection(id)) {
					InvalidateRect(hWnd, NULL, FALSE);
				}
			}
			break;

		///ムービー前進後退/////////////////////
		case 'X':
			if(g_movie){
				g_movie = 0;
				
			}else if(g_aiscope.ExistData()){

				if(GetKeyState( VK_SHIFT ) < 0){	//ムービー開始.
					
					g_movie=1; //1: once movie
					g_timerID = SetTimer( hWnd, 1, g_iFPS, NULL);
					
				}else{	//1フレームだけ進める

					g_timeframe = g_aiscope.FrameJump( g_timeframe + g_frameskip);
			
					ResetWindowText(hWnd);		

				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		case 'Z':
			if(g_movie){
				g_movie = 0;
			}else if(g_aiscope.ExistData()){

				if(GetKeyState( VK_SHIFT ) < 0){	//ムービー開始.
					g_movie=3; //1: once movie
					g_timerID = SetTimer( hWnd, 1, g_iFPS, NULL);
				
				}else{	//1フレームだけ進める

					g_timeframe = g_aiscope.FrameJump( g_timeframe - g_frameskip);
			
					ResetWindowText(hWnd);		
				}
			}
			InvalidateRect(hWnd, NULL, FALSE);
			break;
		
		/////////////////////ムービー前進後退///

		
		////カレントファイル切り替え////////////////////
		/*case VK_TAB:
			if(GetKeyState( VK_CONTROL ) < 0){
				if(GetKeyState( VK_SHIFT ) < 0){	//逆順.
					

				}else{		//正順
					if(g_files.current->next){
						g_files.current = g_files.current->next;
					}else{
						g_files.current = g_files.first;
					}
				}
				
					ResetCurrentFileMenu(hWnd, g_files.current);
					ResetWindowText(hWnd);


			}
			break;
			*/
		/////////////////////ムービー前進後退///
		default:
			return DefWindowProc( hWnd, message, wParam, lParam );
		}
		break;
	/////////////////////////////////////////////キー入力処理///
	//マウス処理/////////////////////////////////////////////////
	case WM_LBUTTONDOWN:
	{
		const int x = GET_X_LPARAM(lParam);
		const int y = GET_Y_LPARAM(lParam);

		if (g_panel != nullptr) {
			if (g_aiscope.ExistData()) {
				const int button_id = g_panel->Push(x, y);
				switch (button_id) {
				case 5: //stop movie
					g_movie = 0;
					break;
				case 2: //movie next
					g_movie = 1; //1: once movie
					g_timerID = SetTimer(hWnd, 1, g_iFPS, NULL);
					break;
				case 1: //movie next one frame
					g_movie = 0;
					g_timeframe = g_aiscope.FrameJump(g_timeframe + g_frameskip);
					ResetWindowText(hWnd);
					break;
				case 4: //movie back to prev				
					g_movie = 3;
					g_timerID = SetTimer(hWnd, 1, g_iFPS, NULL);
					break;
				case 3: //movie back to prev one frame//
					g_movie = 0;
					g_timeframe = g_aiscope.FrameJump(g_timeframe - g_frameskip);
					ResetWindowText(hWnd);
					break;
				}

				if (button_id) {
					InvalidateRect(hWnd, NULL, FALSE);
					break;
				}
			}
		}
		{	//UIPanel以外の領域

			if ((g_edit_mode == 1) && (wParam & MK_CONTROL)){


				vec3d ray, org;
				g_uicamera.GetRAY(x, y, g_client_w, g_client_h, &ray, &org);

				int sid = g_aiscope.SelectAtom(ray, org);
				ResetWindowText(hWnd);
				InvalidateRect(hWnd, NULL, FALSE);

			}
			else{
				if (g_mouse_toggle == 0) {
					int cx = g_client_w / 2;
					int cy = g_client_h / 2;

					int idbit = CameraOrCross();
					if(idbit ==0){//to operate camera//

						if ((abs(x - cx) < 100) && (abs(y - cy) < 100)) {
							//カメラ操作
							g_mouse_toggle = 10;
							g_uicamera.MouseDown(x, y);
						}
						else {
							g_mouse_toggle = 20;
							g_uicamera.MouseDown(x, y);
						}
					}
					else {//to operate user cross section//
						g_uicamera.GetViewMatrix(view_matrix);
						int id = idbit - 1;
						g_mouse_toggle = 20 + idbit * 100;
						s_cross_sections[id].MouseDown(x, y, view_matrix);
						
						/*
						* 3key同時押しはハードウェア制限でNG
						g_uicamera.GetViewMatrix(view_matrix); 
						g_mouse_toggle = 20 + idbit*100;
						for(int id = 0, b = 1; id < 3;++id, b<<=1) {
							if (idbit & b) {
								s_cross_sections[id].MouseDown(x, y, view_matrix);
							}
						}
						*/
					}
				}


				//ウィンドウ領域外でのマウスイベントを受け取るようにする//
				SetCapture(hWnd);

			}
		}
		break;
	}
		
	case WM_LBUTTONUP:
		ReleaseCapture();//領域外でのマウスイベントの受け取りを止める//
		if (g_mouse_toggle){
			g_mouse_toggle = 0;			
		}
		else{
			const int x = GET_X_LPARAM(lParam);
			const int y = GET_Y_LPARAM(lParam);

			const int panel_id = g_uipanel.HitPanel(x, y, g_client_w, g_client_h);
			switch (panel_id){
			case PANEL_ID_G:

				break;
			}

			if (panel_id >= 0){
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		break;
	
	case WM_MOUSEMOVE:
		{			
			int x = GET_X_LPARAM(lParam);
			int y = GET_Y_LPARAM(lParam);
			int res = 0;
			if (g_mouse_toggle == 10) {
				//マウス平行移動
				res = g_uicamera.MouseMoveTrans(x, y, g_client_w, g_client_h);
			}
			else if (g_mouse_toggle == 20) {
				//マウス回転
				res = g_uicamera.MouseMoveRot(x, y, g_client_w, g_client_h);
			}
			/*
			else if (g_mouse_toggle % 100 == 10) {
				//ユーザー定義断面平行移動
				const int idbit = g_mouse_toggle / 100;
				for (int id = 0, b = 1; id < 3; ++id, b <<= 1) {
					if (idbit & b) {
						if (s_cross_sections[id].MouseMoveTrans(x, y, g_client_w, g_client_h)) {
							SetCrossSection(id);
							res = 1;
						}
					}
				}
			}
			*/
			else if (g_mouse_toggle % 100 == 20) {
				//ユーザー定義断面回転
				const int id = g_mouse_toggle / 100 - 1;
				res = s_cross_sections[id].MouseMoveRot(x, y, g_client_w, g_client_h);
				if (res) {
					SetCrossSection(id);
				}
				/*
				const int idbit = g_mouse_toggle / 100;
				for (int id = 0, b = 1; id < 3; ++id, b <<= 1) {
					if (idbit & b) {
						if (s_cross_sections[id].MouseMoveRot(x, y, g_client_w, g_client_h)) {
							SetCrossSection(id);
							res = 1;
						}
					}
				}
				*/
			}

			if(res){
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		break;
	case WM_MOUSEWHEEL://拡大縮小//
		{

			int d = GET_WHEEL_DELTA_WPARAM(wParam) / WHEEL_DELTA;

			int idbit = CameraOrCross();
			if (idbit == 0) {
				if (g_uicamera.ZoomIn(d)) {
					InvalidateRect(hWnd, NULL, FALSE);
				}
			}
			else if(g_aiscope.ExistData()) {
				mat33d boxaxis;
				vec3d boxorg;
				g_aiscope.GetBoxSize(&boxaxis, &boxorg);
				int id = idbit - 1;
				s_cross_sections[id].TranslateNormal(0.2 * (double)d, boxaxis, boxorg);
				SetCrossSection(id);

				/*
				for (int id = 0, b = 1; id < 3; ++id, b <<= 1) {
					if (idbit & b) {
						s_cross_sections[id].TranslateNormal(0.2 * (double)d, boxaxis, boxorg);
						SetCrossSection(id);
					}
				}
				*/
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
		break;
	///メニュー処理////////////////////////////////////////////////
	case WM_COMMAND:
		
		return WinCommand(hWnd, message, wParam, lParam, g_aiscope, &aiproperty);


		break;
	case WM_DROPFILES:
		{
			HDROP hDrop = (HDROP) wParam;  // 内部のDROP構造体へのハンドル.
			TCHAR szFilePath[1024];

			g_renderingLock = true;
			//ドロップされたファイル数の取得.
			int fileCnt = (int)DragQueryFile(hDrop, 0xFFFFFFFF, NULL, 0);

			int res = 0;

            for(int i = 0; i < (int)fileCnt; i++) {
				//ファイル名の取得.
                DragQueryFile(hDrop, i, szFilePath, sizeof(szFilePath)/2);


				res |= g_aiscope.OpenFile(szFilePath);

				if (g_aiscope.ExistData()){
					double focus_distance;
					vec3d target_position;
					g_aiscope.GetFocusInfomation(&focus_distance, &target_position);
					g_uicamera.ResetFocus(focus_distance, target_position);
					ResetCrossSection(target_position);
					ApplySetting(&aiproperty, g_aiscope.GetNumAtoms());
				}

				ResetCurrentFileMenu(hWnd, g_aiscope.GetFileType());
				ResetWindowText(hWnd);
			
            }

			DragFinish(hDrop);		//ドロップしたファイル名(メモリ)の解放.

			g_renderingLock = false;
			if(res){ InvalidateRect(hWnd, NULL, FALSE);}


		}
		break;
	//case WM_ACTIVATEAPP:
	case WM_ACTIVATEAPP:
		if(LOWORD(wParam)){//アクティブor非アクティブ//
			if(g_edit_mode){
				TCHAR filepath[1024];
				g_aiscope.GetFilePath(filepath);
				FILETIME tm;
				MyGetFileTime(filepath, &tm);

				if ((tm.dwHighDateTime > g_filetime.dwHighDateTime) || 
					((tm.dwHighDateTime == g_filetime.dwHighDateTime) && 
					(tm.dwLowDateTime > g_filetime.dwLowDateTime)) ){
					//ファイルが更新されているので再読み込み//
					g_filetime = tm;
				
					g_renderingLock = true;
					int res = g_aiscope.ReloadFile();
					g_renderingLock = false;
					if(res){
						InvalidateRect(hWnd, NULL, FALSE);
					}

				}
			}
		}
		break;
	default:
		return DefWindowProc( hWnd, message, wParam, lParam );
   }
   return 0;
}

int MyGetFileTime(const TCHAR* filepath, FILETIME* tm){
	HANDLE hFile = CreateFile(
				filepath , GENERIC_READ , FILE_SHARE_READ|FILE_SHARE_WRITE|FILE_SHARE_DELETE, NULL ,
				OPEN_EXISTING , FILE_ATTRIBUTE_NORMAL , NULL);
			
	if (hFile == INVALID_HANDLE_VALUE) {
		MessageBox(	NULL , _T("The file does not exist."), _T("Error") , MB_OK);
		return 0;
	}

	GetFileTime(hFile, NULL, NULL, tm);
	CloseHandle(hFile);
	return 1;
}

/////////////////////////////////////////////////////
//
//Movie 再生
//
/////////////////////////////////////////////////////


int PlayMovie(){
////単純再生//////////////////////////////////////////////
	if(g_movie_mode == 0){
		int frametgt = g_timeframe + g_frameskip;
		g_timeframe = g_aiscope.FrameJump(frametgt);
		if(g_timeframe < frametgt){	//最終フレームに到達.
			return 1;
		}
		

////Movie Chack Point 機能.//////////////////////////
		/*
	}else{
		int frametgt;
		if(g_camera->PlayCheckPoint(&frametgt, g_frameskip)){
			g_timeframe = g_aiscope.FrameJump(frametgt);
		}else{
			return 1;	//終了
		}
		return 0;
		*/
	}
	return 0;
}





void ResetWindowText(HWND hWnd){
	//ウィンドウタイトルの文字の切り替え.
	//カレントファイル切り替え時と、タイムフレーム変更時にcallする.
	if(g_aiscope.ExistData()){
		TCHAR buf[1024];
		
		TCHAR filename[1024];
		g_aiscope.GetFileName(filename);

		if(g_edit_mode == 0){

			TCHAR* vmstr[] = {_T("full"), _T("vsync"), _T("eco"), _T("manual")};
			//_stprintf(buf, _T("%s - %s: %d, %.3ffps(%s), cam=(%.3lf,%.3lf)"), szTitle, g_files.current->filename, g_timeframe, g_FPS, vmstr[g_vsync_mode], theta,phi);
			_stprintf(buf, _T("%s - %s: %d, %.3ffps(%s)"), szTitle, filename, g_timeframe, g_FPS, vmstr[g_vsync_mode]);
		}else{
			int sid = g_aiscope.GetSelectedAtomID();
			if (sid == -1){
				_stprintf(buf, _T("%s - %s: [edit] no-selected"), szTitle, filename);
			}else{
				int knd;
				vec3d pos;
				g_aiscope.GetSelectedAtomInfo(&knd, &pos);
				_stprintf(buf, _T("%s - %s: [edit] %d-%s:%.6lf,%.6lf,%.6lf"), szTitle, filename, sid, GetAtomicSymbol(knd), pos.x, pos.y, pos.z);
			}
		}

		SetWindowText(hWnd, buf);
	}
}





int SetModuleDirPath(const TCHAR* filename, TCHAR* returnpath, DWORD nbuflength){
	
	//設定ファイルのフルパスの取得.
	int length = GetModuleFileName(NULL, returnpath, nbuflength);
	if(length <= 0){ return -1;}

	TCHAR* p = _tcsrchr(returnpath, _T('\\'));
	if(! p ) { return -1;}

	_tcscpy(p + 1, filename);

	return length;
}

int SetCurrentDirPath(const TCHAR* filename, TCHAR* returnpath, DWORD nbuflength){
	
	//設定ファイルのフルパスの取得.
	int length = GetCurrentDirectory(nbuflength, returnpath);
	if(length <= 0){ return -1;}

	TCHAR* p = returnpath + length - 1;
	if( *p != _T('\\')){
		p++;
		*p = _T('\\');
	}

	_tcscpy(p + 1, filename);

	return length;
}

int SeekFilePath(const TCHAR* filename, TCHAR* filepath, DWORD nbuflength){

	
	//設定ファイルのフルパスの取得.
	//最初に実行場所のiniファイルの読み込みを試し、失敗したらexeと同じディレクトリのiniファイルを読み込む
	int length = SetCurrentDirPath(filename, filepath, nbuflength);
	if(length > 0){
		if(!PathFileExists(filepath)){
			length = -2;
		}
	}

	if(length < 0){
		length = SetModuleDirPath(filename, filepath, nbuflength);
	}
	
	return length;
}



