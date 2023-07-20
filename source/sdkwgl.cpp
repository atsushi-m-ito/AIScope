///////////////////////////////////////////////
//
//		ver. 20100413
//
///////////////////////////////////////////////


#include "sdkwgl.h"
#include "debugprint.h"


////VBOの準備----拡張関数

PFNGLGENBUFFERSPROC glGenBuffers;
PFNGLISBUFFERPROC glIsBuffer;
PFNGLBINDBUFFERPROC glBindBuffer;
PFNGLBUFFERDATAPROC glBufferData;
PFNGLDELETEBUFFERSPROC glDeleteBuffers;

PFNGLACTIVETEXTUREPROC glActiveTexture;
PFNGLCLIENTACTIVETEXTUREPROC glClientActiveTexture;

PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB;

int g_swapInterval = 0;
int g_refleshRate = 60;
int g_fpscounter = 0;
int g_zeroTm = 0;

//==============================
//WGL関数の初期化
//
//(1) hWndからhDCを取得
//(2) hDCのピクセルフォーマットをOpenGL用に変更
//(3) hDCとcompatibleなOpenGLのコンテキスト(hRC)を発行
//(4) hDCとにhRCを関連付け
//(5) hDCの開放しない！！(CS_OWNDC)
//
// WM_CREATEメッセージハンドラで呼び出す
// hWndに指定する
//==============================
HDC GetWGLDC(HWND hWnd){
	return GetWGLDCbyPixelFormat(hWnd, 0);
}

HDC GetWGLDCbyPixelFormat(HWND hWnd, int pixelFormat){
	
	int res;
	HDC hDC;

	PIXELFORMATDESCRIPTOR pfd = { 
		sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd 
		1,                     // version number 
		PFD_DRAW_TO_WINDOW |   // support window 
		//PFD_DRAW_TO_BITMAP |   // support bitmap 
		PFD_SUPPORT_OPENGL |   // support OpenGL 
		PFD_DOUBLEBUFFER,	   // double buffered 
		PFD_TYPE_RGBA,         // RGBA type 
		32,                    // 24-bit color depth 
		0, 0, 0, 0, 0, 0,      // color bits ignored 
		8,                     // no alpha buffer 
		0,                     // shift bit ignored 
		0,                     // no accumulation buffer 
		0, 0, 0, 0,            // accum bits ignored 
		16,                    // 32-bit z-buffer 
		0,                     // no stencil buffer 
		0,                     // no auxiliary buffer 
		PFD_MAIN_PLANE,        // main layer 
		0,                     // reserved 
		0, 0, 0                // layer masks ignored 
	}; 

	hDC = GetDC(hWnd);

	//一つのhDCに二度以上SetPixelしてはいけない
	if(pixelFormat <= 0){
		pixelFormat=ChoosePixelFormat(hDC,&pfd);
	}
	res = SetPixelFormat(hDC,pixelFormat,&pfd);
	
    return hDC;

}



HGLRC CreateRC(HDC hDC){
	HGLRC hRC=wglCreateContext(hDC);
	wglMakeCurrent(hDC,hRC);
	return hRC;
}

void DeleteRC(HGLRC hRC){

	wglMakeCurrent(0, 0);
	wglDeleteContext(hRC);
}

void Set2D(int scrW, int scrH, int scaling){
	glViewport(0, 0, scrW * scaling, scrH * scaling);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // 透視変換行列の初期化
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // 透視変換行列の初期化
	//2DなのでglOrthoで描画範囲指定
	glOrtho(0.0f, (float)scrW, (float)scrH, 0.0f, 1.0f,-1.0f);
}

void SetScreen2D(int screen_x, int screen_y, int screen_w, int screen_h, float camera_w, float camera_h){
	glViewport(screen_x, screen_y, screen_w, screen_h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // 透視変換行列の初期化
	//2DなのでglOrthoで描画範囲指定
	glOrtho(0.0f, camera_w, camera_h, 0.0f, 1.0f,-1.0f);
}


BOOL SetSwapInterval(int interval){
	//拡張関数を受け取る関数ポインタ
	
	//拡張関数のWGL_EXT_swap_controlをサポートしているか調べる
	if( strstr( (char*)glGetString( GL_EXTENSIONS ), "WGL_EXT_swap_control") == 0)
	{
		//WGL_EXT_swap_controlをサポートしていない
		return 0;
	}
	
	//APIを取得して実行
	BOOL (WINAPI *wglSwapIntervalEXT)(int) = NULL;
	wglSwapIntervalEXT = (BOOL (WINAPI*)(int))wglGetProcAddress("wglSwapIntervalEXT");
	if( wglSwapIntervalEXT ){
		return wglSwapIntervalEXT(interval);
		//TRACE(L"(setVSYNC: %d\n", bres);
	}
	

	return 0;
}


int GetSwapInterval(void){
	//拡張関数を受け取る関数ポインタ
	int (WINAPI *wglGetSwapIntervalEXT)() = NULL;

	//拡張関数のWGL_EXT_swap_controlをサポートしているか調べる
	//注意:glGetStringはhDCがReleaseDCされているとNULLを返す
	if( strstr( (char*)glGetString( GL_EXTENSIONS ), "WGL_EXT_swap_control") == 0)
	{
		//WGL_EXT_swap_controlをサポートしていない
		return 0;
	}

	wglGetSwapIntervalEXT = (int (WINAPI*)())wglGetProcAddress("wglGetSwapIntervalEXT");
	if(wglGetSwapIntervalEXT){
		return wglGetSwapIntervalEXT();
	}

	return 0;
}



int InitWaitVsync(){
	//垂直同期を使う場合に呼び出す
	//SwapInterval設定後のGetSwapIntervalの戻り値をそのまま返す
	//つまり、戻り値が1のときだけ垂直同期成功
	

	HDC hDC = wglGetCurrentDC();
	if (hDC==NULL){
		return 0;
	}

	
	int def_vreflesh = GetDeviceCaps(hDC, VREFRESH);
	TRACE(TEXT("Vreflesh = %d Hz\n"), def_vreflesh);
	g_refleshRate = def_vreflesh;
	
	
	g_swapInterval = GetSwapInterval();
	TRACE(TEXT("Default Swap Interval = %d\n"), g_swapInterval);

	//垂直同期セット
	if (g_swapInterval != 1){
		SetSwapInterval(1);	//省電力モードでは戻り値がtrueでも設定できていない場合がある
		g_swapInterval = GetSwapInterval();
		TRACE(TEXT("Swap Interval = %d\n"), g_swapInterval);
	}
	
	return g_swapInterval;
}


int InitWait(int refleshRate){
	//垂直同期を使わない場合に呼び出す
	//SwapInterval設定後のGetSwapIntervalの戻り値をそのまま返す
	//つまり、戻り値が0のときだけ垂直同期の解除に成功
	

	g_refleshRate = refleshRate;
	
	g_swapInterval = GetSwapInterval();
	TRACE(TEXT("Default Swap Interval = %d\n"), g_swapInterval);

	//垂直同期解除
	if (g_swapInterval != 0){
		SetSwapInterval(0);	//省電力モードでは戻り値がtrueでも設定できていない場合がある
		g_swapInterval = GetSwapInterval();
		TRACE(TEXT("Swap Interval = %d\n"), g_swapInterval);
	}

	return g_swapInterval;
}


//必要時間だけスリープする
//戻り値はFPS
double WaitSleep( int *r_sleepTm){
	//60フレームの区切りの時だけ測定
	//それ以外のときは0を返す
	static int s_sleepTm = 0;		//60フレーム(1000ms中にスリープした時間)
	int tm;


////初回=========================================
	if (g_zeroTm == 0){
		Sleep(0);
		g_zeroTm = timeGetTime();
		g_fpscounter = 0;
		s_sleepTm = 0;

//垂直同期:ON================================
	}else if (g_swapInterval){
		Sleep(0);		//垂直同期時もSleep(0)すべし
		tm = timeGetTime();
		s_sleepTm = 0;
		g_fpscounter ++;
		
//垂直同期:OFF================================
	}else{
		
		g_fpscounter ++;
		//Frame間の時間の計算
		double dtm = 1000.0 * ((double)g_fpscounter) / ((double)g_refleshRate);
		
		tm = timeGetTime();
		
		int sleepTm = g_zeroTm + (int)dtm - tm;
		
		if ( sleepTm > 1){
			//TRACE(TEXT("waitTm %d\n"), sleepTm);
			Sleep(sleepTm);
			tm = timeGetTime();
			s_sleepTm += sleepTm;
		}else{
			Sleep(0);
		}

	}


//fps計測(counterはここまでにインクリメントされている)
	
	if (g_fpscounter >= g_refleshRate){
		double fps_dbl = (double)(g_fpscounter * 1000) / (double)(tm - g_zeroTm);
		g_zeroTm = tm;
		g_fpscounter = 0;
		*r_sleepTm = s_sleepTm;
		s_sleepTm = 0;
		return fps_dbl;

	}

	return 0.0;
	
}

//fps測定をリセットする
void ResetWait(){
	g_zeroTm = 0;
	g_fpscounter = 0;
}


void LoadGLEXT(){
	
	//VBO
	glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
	glIsBuffer = (PFNGLISBUFFERPROC)wglGetProcAddress("glIsBuffer");
	glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
	glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
	glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");

	//MultiTexture
	glActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glActiveTexture");
	glClientActiveTexture = (PFNGLCLIENTACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");

	//multisample
};

//マルチサンプルなどOpenGL拡張をサポートするPixelFormatを取得する
//しかし取得にはOpenGL使用可能な状態(wglMakeCurrent済み)で無ければならない。
//SetPixelFormatによる変更はウィンドウに対して一度きりしか許されない為、
//ダミーのウィンドウを作ってとりあえずのPixelFormatでOpenGLを使用可能にし、
//そこでwglChoosePixelFormatARBを使用して拡張対応のPixelFormatを探す。
//最後にダミーのウィンドウを削除して関数を抜ける
int GetPixelFormatByDummy(HINSTANCE hInstance){

#ifdef UNICODE
#define SDKWGL_DUMMYCLASSNAME	L"sdkwgldummywnd"
#else
#define SDKWGL_DUMMYCLASSNAME	"sdkwgldummywnd" 
#endif
	
	//ダミーウィンドウの作成
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= DefWindowProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= NULL;
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= NULL;
	wcex.lpszClassName	= SDKWGL_DUMMYCLASSNAME;
	wcex.hIconSm		= NULL;

	RegisterClassEx(&wcex);

	
	RECT rc = { 0, 0, 320, 320};
	AdjustWindowRect(&rc, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, TRUE); // ウィンドウスタイル	|WS_CLIPCHILDREN
	
	HWND hWnd = CreateWindow(SDKWGL_DUMMYCLASSNAME, NULL,
				WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX | WS_CLIPCHILDREN, 
				CW_USEDEFAULT, 0, rc.right - rc.left, rc.bottom - rc.top,
				NULL, NULL, hInstance, NULL);

	//とりあえずのPixelFormatを選んでダミーウィンドウ上でOpenGLを使用可能にする
	HDC hDC = GetWGLDC(hWnd);
	HGLRC hRC = CreateRC(hDC);

	//wglChoosePixelFormatARBをDLLからロード
	wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)wglGetProcAddress("wglChoosePixelFormatARB");
	if( ! wglChoosePixelFormatARB ){
		DeleteRC(hRC);
		ReleaseDC(hWnd, hDC);
		DestroyWindow(hWnd);

		return 0;
	}
	
	//GL拡張機能に対応したPixelFormatを探す
	int		pixelFormat;
	int		valid;
	UINT	numFormats;
	float	fAttributes[] = {0,0};

	// These Attributes Are The Bits We Want To Test For In Our Sample
	// Everything Is Pretty Standard, The Only One We Want To 
	// Really Focus On Is The SAMPLE BUFFERS ARB And WGL SAMPLES
	// These Two Are Going To Do The Main Testing For Whether Or Not
	// We Support Multisampling On This Hardware.
	int iAttributes[] =
	{
		WGL_DRAW_TO_WINDOW_ARB,GL_TRUE,
		WGL_SUPPORT_OPENGL_ARB,GL_TRUE,
		WGL_ACCELERATION_ARB,WGL_FULL_ACCELERATION_ARB,
		WGL_COLOR_BITS_ARB,24,
		WGL_ALPHA_BITS_ARB,8,
		WGL_DEPTH_BITS_ARB,16,
		WGL_STENCIL_BITS_ARB,0,
		WGL_DOUBLE_BUFFER_ARB,GL_TRUE,
		WGL_SAMPLE_BUFFERS_ARB,GL_TRUE,
		WGL_SAMPLES_ARB,4,
		0,0
	};

	// First We Check To See If We Can Get A Pixel Format For 4 Samples
	valid = wglChoosePixelFormatARB(hDC,iAttributes,fAttributes,1,&pixelFormat,&numFormats);
	
	// If We Returned True, And Our Format Count Is Greater Than 1
	if (valid && numFormats >= 1)
	{
		return pixelFormat;
	}


	//finalize
	DeleteRC(hRC);
	ReleaseDC(hWnd, hDC);
	DestroyWindow(hWnd);
	return 0;

}




///////////////////////////////////////////////////////////////////////////////
//
//レンダリングコンテキスト(RC)とShareLists関連の管理
//
//仕様:
//OpenGLではウィンドウごとにRCを作る。
//テクスチャ共有するウィンドウは、必ず一度hWndを本ライブラリに登録する。
//(一つのRCの使いまわしはしない。vista用intelドライバで上手く動かないため)
//最初に登録したhWndのRCをマスターとして、
//ShareListsで二番目以降のRCを関連付ける
//全てのテクスチャはマスターのRC上に生成する
//
///////////////////////////////////////////////////////////////////////////////
struct SDLWGL_RC_LIST{
	HWND hWnd;
	HGLRC hRC;
};

SDLWGL_RC_LIST *g_rc_list = NULL;
int g_rc_count = 0;
int g_rc_size = 0;

//ウィンドウ登録
void RegisterSharedRC(HWND hWnd, HGLRC hRC){
	SDLWGL_RC_LIST* rclist;

	if(g_rc_list){
		if( g_rc_size == g_rc_count){//バッファ拡張
			g_rc_size *= 2;
			rclist = g_rc_list;
			g_rc_list = new SDLWGL_RC_LIST[g_rc_size];
			CopyMemory(g_rc_list, rclist, sizeof(SDLWGL_RC_LIST) * g_rc_count);
			delete [] rclist;
		}
	}else{
		g_rc_size = 4;
		g_rc_count = 0;
		g_rc_list = new SDLWGL_RC_LIST[g_rc_size];
	}
	rclist = g_rc_list + g_rc_count;
	g_rc_count++;

	//Wndの登録
	rclist->hWnd = hWnd;
	rclist->hRC = hRC;
		
								
	if(rclist != g_rc_list){
		
		//wglShareListsの前にカレントをセットしておかないと
		//4つ以上のRC使用時に表示がバグる(intel)
		HDC hDC = GetDC(hWnd);
		wglMakeCurrent(hDC,rclist->hRC);			
		wglShareLists(g_rc_list->hRC, rclist->hRC);	//テクスチャ共有
		ReleaseDC(hWnd, hDC);
	}
	return;
}


void DeleteSharedRC(){
	
	wglMakeCurrent(NULL,NULL);
	int i;
	SDLWGL_RC_LIST* rclist = g_rc_list;
	for (i = 0; i < g_rc_count; i++){
		wglDeleteContext(rclist->hRC);
		rclist++;
	}
	delete [] g_rc_list;
	g_rc_list = NULL;
	g_rc_count = 0;
	g_rc_size = 0;

}

//hWndに関連づいたRCを返す
HGLRC GetSharedRC(HWND hWnd){
	
	int i;
	SDLWGL_RC_LIST* rclist = g_rc_list;
	for (i = 0; i < g_rc_count; i++){
		if(hWnd == rclist->hWnd){
			return rclist->hRC;
		}
		rclist++;
	}
	return NULL;
}
