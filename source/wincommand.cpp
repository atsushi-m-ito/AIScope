/*=====================================================


//WM_COMMAND内の処理//


=======================================================*/

#pragma warning(disable : 4996)
#define _CRT_SECURE_NO_DEPRECATE
#define _USE_MATH_DEFINES


#include "winmain.h"

#include "MDLoader.h"
#include "filelist.h"
#include "epsrender.h"
//#include "md3_writer.h"
#include "md3_writer2.h"
//#include "fa2_writer.h"
#include "fa2_writer2.h"
#include "fad_writer.h"
#include "stl_writer.h"
#include "stl_grid_writer.h"

#include "mychardump.h"

#include "AIScope.h"
#include "AIScopeProperty.h"
#include "UICamera.h"
#include "setting.h"
#include "visual.h"

#include "ClassifyMolecule.h"
#include "OutCoordinationNumber.h"

//plugin
#include "plugTDDepth.h"
#include "plugTDCoordinate.h"
#include "plugSSCoordinate.h"

#include <shellapi.h>



extern HINSTANCE g_hInst;					// 現在のインスタンス

//extern AIScope g_aiscope;
extern UICamera g_uicamera;
extern FILETIME g_filetime;

//extern MasterFileList g_files;		//開いたファイルのリスト.
extern bool g_renderingLock;	//レンダリングをしないようにLockする。メモリ転送など.
//extern CameraGL* g_camera;
//extern float bgcolor_reset[4];
extern int g_timeframe;
extern int g_frameskip;	//step number of frame skip
extern int g_iFPS;	//seconds per frame
extern int g_movie;
extern int g_movie_mode;
extern UINT_PTR g_timerID;


extern int g_client_w;
extern int g_client_h;
//extern double g_stereo_eyelength;
extern int g_vsync_mode;

extern int g_edit_mode;





void RecheckMenu(HMENU hmenu, int wmId, const int IDcount, const UINT* IDs);
void RecheckMenuFrameSkip(HMENU hmenu, int wmId);
void RecheckMenuMovieMode(HMENU hmenu, int wmId);
void EnableMovieCheckPoint(HWND hWnd, int flag);
void RecheckMenuCrossSectionMode(HMENU hmenu, int wmId) {
	const UINT ids[4]{ ID_CROSSSECTIONMODE_1AND2AND3,ID_CROSSSECTIONMODE_1OR2OR3,ID_CROSSSECTIONMODE_1AND2OR3,ID_CROSSSECTIONMODE_1OR2AND3 };
	RecheckMenu(hmenu, wmId, 4, ids);
}
void RecheckMenuCrossSectionTarget(HMENU hmenu, int wmId) {
	const UINT ids[2]{ ID_CROSSSECT_TARGET_FIELD,ID_CROSSSECT_TARGET_FIELD_ATOM};
	RecheckMenu(hmenu, wmId, 2, ids);
}

void RecheckMenuBGColor(HMENU hmenu, int wmId);
LRESULT CALLBACK About( HWND, UINT, WPARAM, LPARAM );


void OutputBMP(int number, HWND hWnd);
void OutputLargeBMP(HWND hWnd);
void OutputSequentialBMP(HWND hWnd);
void OutputSequentialStereoBMP(HWND hWnd);
void OutputSequentialMD3(HWND hWnd);
void OutputSequentialBMP_Trace(HWND hWnd);
void OutputSequentialBMP_Rotation(HWND hWnd, int rotate_axis, double radian);
void OutputRotationalBMP(HWND hWnd, double radian);
void ResetCrossSection(const vec3d& box_center);

int WinCommand(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam, AIScope& g_aiscope, AIScopeProperty* aiproperty){
	TCHAR spath[1024];
	int wmId    = LOWORD(wParam); 
	int wmEvent = HIWORD(wParam); 



	// メニュー選択の解析:
	switch( wmId ) {

	case IDM_ABOUT:
		DialogBox(g_hInst, (LPCTSTR)IDD_ABOUTBOX, hWnd, (DLGPROC)About);
		break;
	case IDM_EXIT:
		DestroyWindow( hWnd );
		break;

	case IDM_FILE_OPEN:
		{
			g_renderingLock = true;

			OPENFILENAME ofn;
			TCHAR szFile[ MAX_PATH ] = _T("");
			ZeroMemory( &ofn, sizeof( ofn ) );
			ofn.lStructSize = sizeof( OPENFILENAME );
            ofn.lpstrFilter = TEXT("対応データ形式(fa2,md3,fad,cube,traj,pmd,pmx,mqo,vmec)\0*.fa2;*.md3;*.fad;*.cube;*.traj;*.pmd;*.pmx;*.mqo;*.vmec\0")
				TEXT("全ての形式(*.*)\0*.*\0\0");

/*
			ofn.lpstrFilter = TEXT("Fortran形式原子座標データ(*.fad)\0*.fad\0")
								TEXT("バイナリ原子座標データ(*.dat)\0*.dat\0")
								TEXT("XYZ原子座標データ(*.xyz)\0*.xyz\0")
								TEXT("PDB原子座標データ(*.pdb)\0*.pdb\0")
								TEXT("トラジェクトリーデータ(*.txt)\0*.txt\0")
								TEXT("cubeデータ(*.cube)\0*.cube\0")
								TEXT("pmdデータ(*.pmd)\0*.pmd\0")
								TEXT("全ての形式(*.*)\0*.*\0\0");
*/

			ofn.lpstrFile = szFile;
			ofn.nMaxFile = MAX_PATH;
			ofn.Flags = OFN_FILEMUSTEXIST;
			ofn.lpstrInitialDir = _T("");
			ofn.lpstrTitle = _T("可視化したいデータファイルを選択してください");

			if (GetOpenFileName ( &ofn ) ){

				g_aiscope.OpenFile(szFile, hWnd);
				if (g_aiscope.ExistData()){
					double focus_distance;
					vec3d target_position;
					g_aiscope.GetFocusInfomation(&focus_distance, &target_position);
					g_uicamera.ResetFocus(focus_distance, target_position);
					ResetCrossSection(target_position);
					ApplySetting(aiproperty, g_aiscope.GetNumAtoms());
				}
				ResetCurrentFileMenu(hWnd, g_aiscope.GetFileType());
				ResetWindowText(hWnd);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			g_renderingLock = false;
		}
		break;

	case IDM_EDIT_START:
		if(g_aiscope.ExistData()){
			TCHAR filepath[1024];
			g_aiscope.GetFilePath(filepath);
			if(_tcscmp(filepath + _tcslen(filepath) - 4, _T(".md3")) == 0){
				g_edit_mode = 1;
				MyGetFileTime(filepath, &g_filetime);
				ResetWindowText(hWnd);
			}else{
				MessageBox(	NULL , _T("You can edit md3 only during viewing."), _T("Error") , MB_OK);
			}
		}
		break;
//全体共通の設定////////////////////////////////////
	//射影法の変更.
		
	case IDM_PROJECTION_PERSE:
		aiproperty->ChangeProperty(AISCUPE_KEY_PROJECTION, 0);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case IDM_PROJECTION_ORTHO:
		aiproperty->ChangeProperty(AISCUPE_KEY_PROJECTION, 1);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	
	//ボックスの描画.
	case IDM_BOXFRAME_ON:
		aiproperty->ChangeProperty(AISCUPE_KEY_BOX_DRAW, 1);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case IDM_BOXFRAME_OFF:
		aiproperty->ChangeProperty(AISCUPE_KEY_BOX_DRAW, 0);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
		
	case IDM_MULTIVIEW_OVERLAP:
		aiproperty->ChangeProperty(AISCUPE_KEY_MULTIVIEW, 0);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case IDM_MULTIVIEW_SIDEBYSIDE:
		aiproperty->ChangeProperty(AISCUPE_KEY_MULTIVIEW, 1);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	
	
//原子分子用の機能////////////////////////////

//表示形状の設定
	case IDM_SHAPE_SPHERE:
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_DRAW, 1);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case IDM_SHAPE_POINT:
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_DRAW, 2);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case IDM_SHAPE_ATOM_NONE:
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_DRAW, 0);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case IDM_SHAPE_PIPE:
		aiproperty->ChangeProperty(AISCUPE_KEY_BOND_DRAW, 1);
		//g_aiscope.ResetBond();
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	/*
	case IDM_SHAPE_ENERGYJ:
		aiproperty->ChangeProperty(AISCUPE_KEY_BOND_DRAW, 2);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	*/
	case IDM_SHAPE_BOND_NONE:
		aiproperty->ChangeProperty(AISCUPE_KEY_BOND_DRAW, 0);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	
	case IDM_COLOR_ATOMIC:
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_COLOR, 0);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case IDM_COLOR_BONDNUM:
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_COLOR, 1);

		//g_aiscope.ResetBond();
		
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case IDM_COLOR_PRESSURE:
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_COLOR, 2);

		g_aiscope.ResetColorByPressure();

		InvalidateRect(hWnd, NULL, FALSE);
		break;
	case IDM_COLOR_HEIGHT:
		aiproperty->ChangeProperty(AISCUPE_KEY_ATOM_COLOR, 3);

		g_aiscope.ResetColorByPressure();

		InvalidateRect(hWnd, NULL, FALSE);
		break;
//1フレームの解析//////////////////////////////////
	//結合の再判定
	case IDM_RESET_BOND:
		if(g_movie){
			MessageBox(hWnd, _T("Prease stop movie.\n"), NULL, MB_OK);
			break;
		}
		
		if(g_aiscope.ResetBond()){
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
	//分子に分類
	case IDM_CLASSIFYMOL:
		//if(atm==NULL){ break;}
		if(g_movie){
			printf("Prease stop movie.\n");

		}else{
		
			if(g_aiscope.GetFileType() == FILETYPE_ATOM){
				g_aiscope.GetFilePath(spath);
				TCHAR* p = _tcsrchr(spath, _T('.'));
				if(p){ *p = _T('\0');}
				_tcscat(spath,_T("_mol.txt"));
				ATOMS_DATA* dat = g_aiscope.GetAtomData();
				BOND_INFO* bond = g_aiscope.GetBondData();
				
				AI_WriteClassifyMolecule2(dat, bond, spath);
		
			}
		}
	//原子ごとの配位数の出力
	case IDM_COORDINATE:
		//if(atm==NULL){ break;}
		if(g_movie){
			printf("Prease stop movie.\n");

		}else{
		
			if(g_aiscope.GetFileType() == FILETYPE_ATOM){
					g_aiscope.GetFilePath(spath);
					TCHAR* p = _tcsrchr(spath, _T('.'));
					if(p){ *p = _T('\0');}
					size_t length = _tcslen(spath);
					_stprintf(spath + length,_T("_coord_%d.txt"), g_timeframe);
					ATOMS_DATA* dat = g_aiscope.GetAtomData();
					if (dat) {
						BOND_INFO* bond = g_aiscope.GetBondData();
						if (bond->count > 0) {
							OutCoordinationNumber(dat, bond, spath);
						}
					}
				
			}
		}
		break;
	//Field///////////////////////////////////////
	case IDM_FIELD_RENDERINGMODE_RAYCASTING:
	{
		aiproperty->ChangeProperty(AISCUPE_KEY_FIELD_RENDERMODE, 1);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case IDM_FIELD_RENDERINGMODE_SLICING:
	{
		aiproperty->ChangeProperty(AISCUPE_KEY_FIELD_RENDERMODE, 0);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case ID_CROSSSECTIONMODE_1AND2AND3:
	{
		SetCrossSectionMode(0, 0, 0);
		RecheckMenuCrossSectionMode(GetMenu(hWnd), wmId);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case ID_CROSSSECTIONMODE_1OR2OR3:
	{
		SetCrossSectionMode(1, 1, 1);
		RecheckMenuCrossSectionMode(GetMenu(hWnd), wmId);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case ID_CROSSSECTIONMODE_1AND2OR3:
	{
		SetCrossSectionMode(0, 0, 1);
		RecheckMenuCrossSectionMode(GetMenu(hWnd), wmId);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case ID_CROSSSECTIONMODE_1OR2AND3:
	{
		SetCrossSectionMode(1, 1, 0);
		RecheckMenuCrossSectionMode(GetMenu(hWnd), wmId);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case ID_CROSSSECT_TARGET_FIELD:
	{
		SetCrossSectionTarget(0b1);
		RecheckMenuCrossSectionTarget(GetMenu(hWnd), wmId);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
	case ID_CROSSSECT_TARGET_FIELD_ATOM:
	{
		SetCrossSectionTarget(0b11);
		RecheckMenuCrossSectionTarget(GetMenu(hWnd), wmId);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	break;
/*
	//動径分布関数g(r)
	case IDM_RADIALDISTFUNC:
		//if(atm==NULL){ break;}

		dat = current->atm->GetDataPointer();
		//test------
		AI_eraceBulk(dat, dat->boxaxis.m33 * 0.5f - 75.0f);
		//----test

		length = (int)(current->ext - current->filepath - 1);
		_tcsncpy(spath, current->filepath, length);
		_stprintf(spath + length,_T("_gr%d.txt"), dat->no);
		
		WriteRadialDistFunc(0.02, dat, spath);
		
		break;

	//密度分布ρ(r)
	case IDM_DENCITY:
		//if(atm==NULL){ break;}
		if(g_movie){
			printf("Prease stop movie.\n");
			break;
		}

		dat = current->atm->GetDataPointer();
		
	
		length = (int)(current->ext - current->filepath - 1);
		_tcsncpy(spath, current->filepath, length);
		_stprintf(spath + length,_T("_dencity%d.txt"), dat->no);
		
		WriteDencity( dat, spath,dat->boxaxis.m33 * 0.5f , -dat->boxaxis.m33 * 0.5f);

		break;

	//ボンド数出力
	case IDM_COORDINATE:
		//if(atm==NULL){ break;}
		if(g_movie){
			printf("Prease stop movie.\n");
			break;
		}

		dat = current->atm->GetDataPointer();
		
		plugFrameSS_Coordinate(current->filepath, dat);

		break;
	
	//ボンドの異方性解析.
	case IDM_BONDDIR:
		//if(atm==NULL){ break;}
		if(g_movie){
			printf("Prease stop movie.\n");
			break;
		}

		dat = current->atm->GetDataPointer();
		
		
		length = (int)(current->ext - current->filepath - 1);
		_tcsncpy(spath, current->filepath, length);
		_stprintf(spath + length,_T("_bonddir%d.txt"), dat->no);
		
		WriteBondDirection( dat, spath);

		break;

	//sp2の法線出力
	case IDM_SP2NORMAL:
		//if(atm==NULL){ break;}
		if(g_movie){
			printf("Prease stop movie.\n");
			break;
		}

		dat = current->atm->GetDataPointer();
		
		
		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_stprintf(spath + length,_T("_sp2nor%d.txt"), dat->no);
		
		WriteSp2normal( dat, spath);

		break;



	//Cと結合したHの抽出.
	case IDM_HBOUNDC:
		//if(atm==NULL){ break;}

		if(g_movie){
			printf("Prease stop movie.\n");
			break;
		}

		dat = current->atm->GetDataPointer();
		
		
		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_stprintf(spath + length,_T("_absorbedH%d.txt"), dat->no);
		
		WriteBoundAtom(dat, g_frameskip, spath, KIND_H, KIND_C);
		
		InvalidateRect(hWnd, NULL, FALSE);
		break;
*/
//時系列解析//////////////////////////////////
	case IDM_TD_COORDINATE:
		if (g_movie) {
			printf("Prease stop movie.\n");

		} else {

			if (g_aiscope.GetFileType() == FILETYPE_ATOM) {
				g_aiscope.GetFilePath(spath);
				TCHAR* p = _tcsrchr(spath, _T('.'));
				if (p) { *p = _T('\0'); }
				size_t length = _tcslen(spath);
				_stprintf(spath + length, _T("_coord.txt"));
				OutputSequentialCoordinateNumber(spath, g_aiscope);				
				
			}
		}
		break;

/*
	case IDM_TD_DEPTH:
		//if(atm==NULL){ break;}

		plugReadyTD_Depth(current->filepath, current->dirlen);
		
		dat = current->atm->GetDataPointer();
		while(dat){

			plugFrameTD_Depth(dat);

			if(g_frameskip > 1){
				if (current->atm->goNext(g_frameskip) == 0){
					break;
				}
			}else{		
				if (current->atm->goNext() == 0){
					break;
				}
			}
			dat = current->atm->GetDataPointer();
		}


		plugFinishTD_Depth(dat);

		ResetWindowText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	//水素吸着数(TIME-DEPENDENT)
	case IDM_ADSORBED_H_TD:
		//if(atm==NULL){ break;}

		
		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T("_adsH.txt"));

		AI_WriteAdsorbedH( current->atm, g_frameskip,spath);
		
		ResetWindowText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	case IDM_RADIALDISTFUNC_PEAK_TD:
		//if(atm==NULL){ break;}

		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T("_grtime.txt"));

		WriteRadialDistFuncMax(0.02, current->atm, g_frameskip,spath);
		
		ResetWindowText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);

		break;
	
	case IDM_YIELD_TD:
		//if(atm==NULL){ break;}

		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T("_yield.txt"));

		WriteErosionYield(24.0, current->atm, g_frameskip,spath);
		
		ResetWindowText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);

		break;

//			case IDM_BONDSTATE_TD:
//				//if(atm==NULL){ break;}
//
//				length = current->ext - current->filepath - 1;
//				_tcsncpy(spath, current->filepath, length);
//				_tcscpy(spath + length,_T("_coord2.txt"));
//
//				WriteCoordinateC2(current->atm, g_frameskip, spath);
//
//				ResetWindowText(hWnd);				
//				InvalidateRect(hWnd, NULL, FALSE);
//
//				break;
	
	case IDM_HCRATIO_TD:
		//if(atm==NULL){ break;}

		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T("_hc.txt"));

		WriteHCRatio(current->atm, g_frameskip, spath);
		
		
		ResetWindowText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);

		break;

	case IDM_MOLECULE_TD:
		//if(atm==NULL){ break;}

		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T("_mol.txt"));

		WriteClassifiedMolecule2(current->atm, g_frameskip, spath);
		
		
		ResetWindowText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);

		break;

	
	case IDM_RETENTION_TD:
		//if(atm==NULL){ break;}

		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T("_ret.txt"));

		WriteRetention(current->atm, g_frameskip, spath, 3.35);
		
		
		ResetWindowText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);

		break;

	case IDM_DEPOSITION_TD:
		//if(atm==NULL){ break;}
		
		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T("_depo.txt"));

		WriteDeposition(current->atm, g_frameskip, spath);
		
		
		ResetWindowText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);
		break;

	//ボンド生存量
	case IDM_BONDSURVIVE_TD:
		//if(atm==NULL){ break;}

		if(g_movie){
			printf("Prease stop movie.\n");
			break;
		}

		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T("_bondlist000000.txt"));

		WriteBondListBase(current->atm, g_frameskip, spath);
		
		ResetWindowText(hWnd);
		InvalidateRect(hWnd, NULL, FALSE);
		break;
	*/
//動画関連===========================
				
//FrameSkipの設定
	case IDM_FRAMESKIP_OFF:
		g_frameskip = 1;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_2:
		g_frameskip = 2;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_4:
		g_frameskip = 4;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_8:
		g_frameskip = 8;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_10:
		g_frameskip = 10;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_16:
		g_frameskip = 16;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_20:
		g_frameskip = 20;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_32:
		g_frameskip = 32;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_64:
		g_frameskip = 64;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_100:
		g_frameskip = 100;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_1000:
		g_frameskip = 1000;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;

	case IDM_FRAMESKIP_10000:
		g_frameskip = 10000;
		RecheckMenuFrameSkip(GetMenu(hWnd), wmId);
		break;
	case IDM_MOVIEMODE_SIMPLE:
		g_movie_mode= 0;
		RecheckMenuMovieMode(GetMenu(hWnd), wmId);
		EnableMovieCheckPoint(hWnd, 0);
		break;
/*
	case IDM_MOVIEMODE_CHKPOINT:
		g_movie_mode= 1;
		RecheckMenuMovieMode(GetMenu(hWnd), wmId);
		EnableMovieCheckPoint(hWnd, 1);
		break;

	case IDM_CHKPOINT_INSERT:
		//if(g_checkpoint){
			if(g_files.current){
				g_camera->InsertCheckPoint(g_timeframe, 0);
			}
		//}

		break;

	case IDM_CHKPOINT_SAVE:
		//if(g_checkpoint){
		
		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T(".chp"));

		g_camera->SaveCheckPoint(spath);
		
		break;

	case IDM_CHKPOINT_LOAD:
		length = current->ext - current->filepath - 1;
		_tcsncpy(spath, current->filepath, length);
		_tcscpy(spath + length,_T(".chp"));
		g_camera->LoadCheckPoint(spath);
		break;

	case IDM_CHKPOINT_PLAYALL:
		//if(g_checkpoint){
			//g_checkpoint->Rewind();
			g_timeframe = 0;
			g_camera->RewindCheckPoint();
			
			g_movie = 2; //1: once movie
			g_timerID = SetTimer( hWnd, 1, g_iFPS, NULL);
		//}
		break;
*/
	case IDM_CHKPOINT_PLAYWITHBMP:
		{

            OutputSequentialBMP(hWnd);
			ResetWindowText(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			
        }
		break;
    case IDM_ROTATIONAL_BMP:
    {

        OutputRotationalBMP(hWnd, g_movie_delta_angle_deg *M_PI / 180.0);
        ResetWindowText(hWnd);
        InvalidateRect(hWnd, NULL, FALSE);

    }
    break;
	case IDM_CHKPOINT_PLAYWITHBMP_ROTATE_RIGHT:
		{

			OutputSequentialBMP_Rotation(hWnd, 1, -g_movie_delta_angle_deg *M_PI/180.0);
			ResetWindowText(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);

		}
		break;
	case IDM_CHKPOINT_PLAYWITHBMP_ROTATE_DOWN:
		{

			OutputSequentialBMP_Rotation(hWnd, 2, g_movie_delta_angle_deg *M_PI / 180.0);
			ResetWindowText(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);

		}
		break;
	case IDM_CHKPOINT_PLAYWITHBMP_ROTATE_UP:
		{

			OutputSequentialBMP_Rotation(hWnd, 2, -g_movie_delta_angle_deg *M_PI / 180.0);
			ResetWindowText(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);

		}
		break;
    case IDM_CHKPOINT_PLAYWITHBMP_TRACE:
        {

            OutputSequentialBMP_Trace(hWnd);
            ResetWindowText(hWnd);
            InvalidateRect(hWnd, NULL, FALSE);

        }
        break;
	case IDM_CHKPOINT_PLAYWITHMD3:
		{
			if (g_aiscope.GetFileType() == FILETYPE_ATOM) {
				OutputSequentialMD3(hWnd);
				ResetWindowText(hWnd);
				InvalidateRect(hWnd, NULL, FALSE);
			}
		}
    /*
	case IDM_CHKPOINT_PLAYWITHSTEREOBMP:
			g_timeframe = 0;
			g_camera->RewindCheckPoint();
								
			OutputSequentialStereoBMP(hWnd);
			
			ResetWindowText(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
		
		break;
		
	case IDM_CHKPOINT_PLAYBEFORE:
		break;
	case IDM_CHKPOINT_PLAYAFTER:

		break;
*/
//スナップショット出力.==========================
	//bmp出力
	case IDM_SNAPSHOT_BMP:
		if(g_aiscope.ExistData()){
            OutputBMP(g_timeframe, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
        }
		break;
		
	//ラージサイズbmp出力
	case IDM_SNAPSHOT_LARGE_BMP:
		if(g_aiscope.ExistData()){
			OutputLargeBMP(hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
		}
		break;
		//立体視機能
	case IDM_STEREO_BMP:

		if(g_aiscope.ExistData()){

			double view_matrix[16];
			g_uicamera.GetViewMatrix(view_matrix);
			double focus_distance = g_uicamera.GetFocusDistance();

			HDC hDC = GetDC(hWnd);
			HGLRC hRC = GetSharedRC(hWnd);
			wglMakeCurrent(hDC,hRC);

			g_aiscope.OutputStereoBMPGL(view_matrix, focus_distance);
			
			wglMakeCurrent(NULL, NULL);
			ReleaseDC(hWnd, hDC);
		}
		break;
	//eps出力
	case IDM_SNAPSHOT_EPS:
		if(g_aiscope.ExistData()){
			if(g_aiscope.GetFileType() == FILETYPE_ATOM){ 
			
				
				g_aiscope.GetFilePath(spath);
				TCHAR* p = _tcsrchr(spath, _T('.'));
				if(p){ *p = _T('\0');}
				//length = (int)(current->ext - current->filepath - 1);
				//_tcsncpy(spath, current->filepath, length);
				size_t length = _tcslen(spath);
				_stprintf(spath + length,_T("%d.eps"), g_timeframe);

				ATOMS_DATA* dat = g_aiscope.GetAtomData();
			
				HDC hDC = GetDC(hWnd);
				HGLRC hRC = GetSharedRC(hWnd);
				wglMakeCurrent(hDC,hRC);
				
				FILE* fp = OpenAtomEPS(spath, g_client_w,g_client_h);
				DrawAtomEPS(fp, dat, g_client_w,g_client_h);
				
				CloseAtomEPS(fp);					

				wglMakeCurrent(NULL, NULL);
				ReleaseDC(hWnd, hDC);
			}
		}
		break;
	//md3形式で出力//
	case IDM_SNAPSHOT_MD3:
		if(g_aiscope.ExistData()){
			if(g_aiscope.GetFileType() == FILETYPE_ATOM){ 
		
			
				g_aiscope.GetFilePath(spath);
				TCHAR* p = _tcsrchr(spath, _T('.'));
				if(p){ *p = _T('\0');}
				//length = (int)(current->ext - current->filepath - 1);
				//_tcsncpy(spath, current->filepath, length);
				size_t length = _tcslen(spath);
				_stprintf(spath + length,_T("_%d.md3"), g_timeframe);

				ATOMS_DATA* dat = g_aiscope.GetAtomData();
				
				float box_f[12];				
				*((mat33f*)box_f) = dat->boxaxis;
				*((vec3f*)(box_f + 9)) = dat->boxorg;
				
#if 1
				char* mbfpath = MbsDup(spath);
				msz::MD3_Writer2<float> md3;
				if (md3.Open(mbfpath)) {
					md3.BeginFrame(dat->pcnt);
					md3.WriteBox(box_f);
					md3.WriteZ(dat->knd);
					md3.WriteR(dat->r);
					md3.EndFrame();
					md3.Close();
				}
				delete[] mbfpath;

#else
				char* mbfpath = MbsDup(spath);
				MD3_Writer* md3 = new MD3_Writer;
				int ierr = md3->Open(mbfpath);
				delete[] mbfpath;

				md3->Write_f(dat->pcnt, dat->knd, (float*)dat->r, NULL, box_f,NULL);
				md3->Close();
				delete md3;
#endif
			}

		}
		break;

		
	//fa2形式で出力//
	case IDM_SNAPSHOT_FA2:
		if(g_aiscope.ExistData()){
			if(g_aiscope.GetFileType() == FILETYPE_ATOM){ 
		
			
				g_aiscope.GetFilePath(spath);
				TCHAR* p = _tcsrchr(spath, _T('.'));
				if(p){ *p = _T('\0');}
				//length = (int)(current->ext - current->filepath - 1);
				//_tcsncpy(spath, current->filepath, length);
				size_t length = _tcslen(spath);
				_stprintf(spath + length,_T("_%d.fa2"), g_timeframe);

				ATOMS_DATA* dat = g_aiscope.GetAtomData();
			
				float box_f[12];
				*((mat33f*)box_f) = dat->boxaxis;
				*((vec3f*)(box_f + 9)) = dat->boxorg;

#if 1
				char* mbfpath = MbsDup(spath);
				msz::FA2_Writer2<float> fa2;
				if (fa2.Open(mbfpath)) {
					fa2.BeginFrame(dat->pcnt);
					fa2.WriteBox(box_f);
					fa2.WriteZ(dat->knd);
					fa2.WriteR(dat->r);
					fa2.EndFrame();
					fa2.Close();
				}
				delete[] mbfpath;

#else
				char* mbfpath = MbsDup(spath);
				FA2_Writer* fa2 = new FA2_Writer;
				fa2->Open(mbfpath);

				fa2->Write(dat->pcnt, dat->knd, NULL, (float*)(dat->r), NULL, box_f);
				fa2->Close();
				delete fa2;
				delete[]mbfpath;
#endif
			}

		}
		break;
		
	//fad形式で出力//
	case IDM_SNAPSHOT_FAD:
		if(g_aiscope.ExistData()){
			if(g_aiscope.GetFileType() == FILETYPE_ATOM){ 
		
			
				g_aiscope.GetFilePath(spath);
				TCHAR* p = _tcsrchr(spath, _T('.'));
				if(p){ *p = _T('\0');}
				//length = (int)(current->ext - current->filepath - 1);
				//_tcsncpy(spath, current->filepath, length);
				size_t length = _tcslen(spath);
				_stprintf(spath + length,_T("_%d.fad"), g_timeframe);

				ATOMS_DATA* dat = g_aiscope.GetAtomData();

				float box_f[12];
				*((mat33f*)box_f) = dat->boxaxis;
				*((vec3f*)(box_f + 9)) = dat->boxorg;

				char* mbfpath = MbsDup(spath);
				FAD_Writer* fad = new FAD_Writer;
				fad->Open(mbfpath);

				fad->Write(dat->pcnt, dat->knd, NULL, (float*)(dat->r), NULL, box_f);
				fad->Close();
				delete fad;
			}

		}
		break;

		//fa2形式で出力//
	case IDM_SNAPSHOT_STL:
		if (g_aiscope.ExistData()) {
			if (g_aiscope.GetFileType() == FILETYPE_ATOM) {


				g_aiscope.GetFilePath(spath);
				TCHAR* p = _tcsrchr(spath, _T('.'));
				if (p) { *p = _T('\0'); }
				//length = (int)(current->ext - current->filepath - 1);
				//_tcsncpy(spath, current->filepath, length);
				size_t length = _tcslen(spath);
				_stprintf(spath + length, _T("_%d.stl"), g_timeframe);

				ATOMS_DATA* dat = g_aiscope.GetAtomData();

				float box_f[12];
				*((mat33f*)box_f) = dat->boxaxis;
				*((vec3f*)(box_f + 9)) = dat->boxorg;

				char* mbfpath = MbsDup(spath);

				STL_Writer stl;
				if (stl.Open(mbfpath)) {
					stl.Write<float>(dat->r, dat->pcnt, g_stl_atomradius, g_stl_atompoly);					
					stl.Close();
				}

				delete[] mbfpath;

			}

		}
		break;
		//fa2形式で出力//
	case IDM_SNAPSHOT_STL_GRID:
		if (g_aiscope.ExistData()) {
			if (g_aiscope.GetFileType() == FILETYPE_ATOM) {


				g_aiscope.GetFilePath(spath);
				TCHAR* p = _tcsrchr(spath, _T('.'));
				if (p) { *p = _T('\0'); }
				//length = (int)(current->ext - current->filepath - 1);
				//_tcsncpy(spath, current->filepath, length);
				size_t length = _tcslen(spath);
				_stprintf(spath + length, _T("_%d.stl"), g_timeframe);

				ATOMS_DATA* dat = g_aiscope.GetAtomData();

				float box_f[12];
				*((mat33f*)box_f) = dat->boxaxis;
				*((vec3f*)(box_f + 9)) = dat->boxorg;

				char* mbfpath = MbsDup(spath);

				STL_Grid_Writer stl;
				if (stl.Open(mbfpath)) {

					stl.Write(dat->r, dat->pcnt, g_stl_atomradius, dat->boxaxis, g_stl_grid_width, g_stl_grid_cutoff_edge_z, g_stl_scale);
					stl.Close();
				}


				delete[] mbfpath;

			}

		}
		break;
//設定関係.
	//setting.iniをユーザー既定のエディタで開く.
	case IDM_OPEN_SETTING:
		{
			TCHAR settingpath[1024];
			if(SeekFilePath(_T("setting.ini"), settingpath, 1024) > 0){
				ShellExecute(NULL, _T("open"), settingpath, NULL, NULL, SW_SHOWNORMAL);
			}
		}
		break;
	//setting.iniの再読み込み
	case IDM_RELOAD_SETTING:
		{
		TCHAR settingpath[1024];
		SeekFilePath(_T("setting.ini"), settingpath, 1024);
		LoadSettingFile(settingpath);
		//ApplySetting();
	
		}
		break;
//	default:
//	   return DefWindowProc( hWnd, message, wParam, lParam );
	}


	return 0;
}


void RecheckMenu(HMENU hmenu, int wmId, const int IDcount, const UINT* IDs){
	
	MENUITEMINFO menuInfo;
	ZeroMemory(&menuInfo, sizeof(MENUITEMINFO));
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_STATE;
					

	for(int i = 0; i < IDcount; i++){
		if (IDs[i] == wmId){
			menuInfo.fState = MFS_CHECKED;
		}else{
			menuInfo.fState = MFS_UNCHECKED;
		}
		SetMenuItemInfo(hmenu , IDs[i], FALSE , &menuInfo);
	}

}

void RecheckMenuFrameSkip(HMENU hmenu, int wmId){
	
	const int IDcount = 6;
	const UINT IDs[] = {IDM_FRAMESKIP_OFF, IDM_FRAMESKIP_2, IDM_FRAMESKIP_10, IDM_FRAMESKIP_20, IDM_FRAMESKIP_100, IDM_FRAMESKIP_1000};

	MENUITEMINFO menuInfo;
	ZeroMemory(&menuInfo, sizeof(MENUITEMINFO));
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_STATE;
					

	for(int i = 0; i < IDcount; i++){
		if (IDs[i] == wmId){
			menuInfo.fState = MFS_CHECKED;
		}else{
			menuInfo.fState = MFS_UNCHECKED;
		}
		SetMenuItemInfo(hmenu , IDs[i], FALSE , &menuInfo);
	}

}

void RecheckMenuMovieMode(HMENU hmenu, int wmId){
		
	const int IDcount = 2;
	const UINT IDs[] = {IDM_MOVIEMODE_SIMPLE, IDM_MOVIEMODE_CHKPOINT};

	MENUITEMINFO menuInfo;
	ZeroMemory(&menuInfo, sizeof(MENUITEMINFO));
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_STATE;
					

	for(int i = 0; i < IDcount; i++){
		if (IDs[i] == wmId){
			menuInfo.fState = MFS_CHECKED;
		}else{
			menuInfo.fState = MFS_UNCHECKED;
		}
		SetMenuItemInfo(hmenu , IDs[i], FALSE , &menuInfo);
	}

	
}


void EnableMovieCheckPoint(HWND hWnd, int flag){
	//Movie Check Point機能のOn/Offに合わせて使えるメニューを設定.
	//flagが1ならENABLE, 0 ならDISABLEにする.

	
	const int IDcount = 3;
	const UINT IDs[] = {IDM_CHKPOINT_INSERT, IDM_CHKPOINT_SAVE, IDM_CHKPOINT_LOAD};

	HMENU hmenu = GetMenu(hWnd);

	UINT uEnable;
	if (flag == 0){
		uEnable = MF_DISABLED;	// | MF_GRAYED
	}else{
		uEnable = MF_ENABLED;
	}

	
	for(int i = 0; i < IDcount; i++){
		EnableMenuItem(hmenu, IDs[i], uEnable);
	}
	DrawMenuBar(hWnd);

	return;


}






void RecheckMenuBGColor(HMENU hmenu, int wmId){
	
	MENUITEMINFO menuInfo;
	ZeroMemory(&menuInfo, sizeof(MENUITEMINFO));
	menuInfo.cbSize = sizeof(MENUITEMINFO);
	menuInfo.fMask = MIIM_STATE;
					
	menuInfo.fState = MFS_UNCHECKED;
	SetMenuItemInfo(hmenu , IDM_BGCOLOR_BLACK, FALSE , &menuInfo);
	SetMenuItemInfo(hmenu , IDM_BGCOLOR_WHITE, FALSE , &menuInfo);
		
	menuInfo.fState = MFS_CHECKED;
	SetMenuItemInfo(hmenu , wmId , FALSE , &menuInfo);
}



// バージョン情報ボックス用メッセージ ハンドラ
LRESULT CALLBACK About( HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch( message )
	{
		case WM_INITDIALOG:
				return TRUE;

		case WM_COMMAND:
			if( LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL ) 
			{
				EndDialog(hDlg, LOWORD(wParam));
				return TRUE;
			}
			break;
	}
    return FALSE;
}

[[deprecated]]
void ResetCurrentFileMenu(HWND hWnd, FILETYPE filetype){
	//カレントファイルに合わせて使えるメニューを設定.
	return; //nothing to do//

	switch(filetype){
	case FILETYPE_ATOM:		//原子分子の時
		EnableMenuItem(GetMenu(hWnd), 0, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), 4, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), 5, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 6, MF_BYPOSITION | MF_ENABLED);
			
//		RecheckMenuShapeAtom(GetMenu(hWnd));
//		RecheckMenuShapeBond(GetMenu(hWnd));
//		RecheckMenuAtomColor(GetMenu(hWnd));
		DrawMenuBar(hWnd);
		return;
	case FILETYPE_TRAJ:		//原子分子の時
		EnableMenuItem(GetMenu(hWnd), 0, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 4, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), 5, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 6, MF_BYPOSITION | MF_ENABLED);
		DrawMenuBar(hWnd);
		return;
	case FILETYPE_FIELD:		//原子分子の時
		EnableMenuItem(GetMenu(hWnd), 0, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), 4, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 5, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 6, MF_BYPOSITION | MF_ENABLED);
		DrawMenuBar(hWnd);
		return;
	case FILETYPE_LMC:		//原子分子の時
		EnableMenuItem(GetMenu(hWnd), 0, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), 4, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
		EnableMenuItem(GetMenu(hWnd), 5, MF_BYPOSITION | MF_ENABLED);
		EnableMenuItem(GetMenu(hWnd), 6, MF_BYPOSITION | MF_ENABLED);
			
//		RecheckMenuShapeAtom(GetMenu(hWnd));
//		RecheckMenuShapeBond(GetMenu(hWnd));
//		RecheckMenuAtomColor(GetMenu(hWnd));
		DrawMenuBar(hWnd);
		return;

    case FILETYPE_VMEC:		//原子分子の時
        EnableMenuItem(GetMenu(hWnd), 0, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), 4, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), 5, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 6, MF_BYPOSITION | MF_ENABLED);

        //		RecheckMenuShapeAtom(GetMenu(hWnd));
        //		RecheckMenuShapeBond(GetMenu(hWnd));
        //		RecheckMenuAtomColor(GetMenu(hWnd));
        DrawMenuBar(hWnd);
        return;

    case FILETYPE_PMD:		//原子分子の時
        EnableMenuItem(GetMenu(hWnd), 0, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), 4, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), 5, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 6, MF_BYPOSITION | MF_ENABLED);

        //		RecheckMenuShapeAtom(GetMenu(hWnd));
        //		RecheckMenuShapeBond(GetMenu(hWnd));
        //		RecheckMenuAtomColor(GetMenu(hWnd));
        DrawMenuBar(hWnd);

    case FILETYPE_KARABA:		//原子分子の時
        EnableMenuItem(GetMenu(hWnd), 0, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 1, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), 4, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
        EnableMenuItem(GetMenu(hWnd), 5, MF_BYPOSITION | MF_ENABLED);
        EnableMenuItem(GetMenu(hWnd), 6, MF_BYPOSITION | MF_ENABLED);

        return;
	}
	

	//ファイルが開いてないときや、未対応フォーマットの時.
	
	EnableMenuItem(GetMenu(hWnd), 2, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), 3, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), 4, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), 5, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	EnableMenuItem(GetMenu(hWnd), 6, MF_BYPOSITION | MF_DISABLED | MF_GRAYED);
	DrawMenuBar(hWnd);

	return;


}


		