
#ifndef AIScopeEvent_H
#define AIScopeEvent_H


#pragma once
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <stdio.h>

#include "targetver.h"
#include <windows.h>
#include <tchar.h>

#include <vector>


#include "vec3.h"

#include "filelist.h"
#include "AIScopeProperty.h"
#include "RenderGL15.h"
#include "RenderDX11.h"
#include "RenderingProperty.h"



/*

UIと連携させる最上位ラッパー

*/
class AIScope
{
public:
	
	AIScope(AIScopeProperty* aiproperty);
	virtual ~AIScope();


	//クラスユーザーの利用できるイベント/////////////////////////////////////
	
    /*
    ウィンドウサイズの変更時に呼び出す
    */
    int Resize(int w, int h);

	
    /*
    ファイルを開くときに呼び出す
    */
	int OpenFile(const TCHAR* filepath);

    /*
    ファイルを再読み込みしたいときに呼び出す
    */
    int ReloadFile();


    /*
    movie frame operation
    */
    int FrameJump(int timeframe);


    /*
    ファイル情報の取得
    */
	void GetFocusInfomation(double* focus_distance, vec3d* target_position);


	//get infomation//
	int GetFileName(TCHAR* buffer){
		if(!m_data_list.empty()){
			_tcscpy(buffer, m_data_list[0].filename);
		}
		return 0;
	};
	
	int GetFilePath(TCHAR* buffer){
		if(!m_data_list.empty()){
			_tcscpy(buffer, m_data_list[0].filepath);
		}
		return 0;
	};

	FILETYPE GetFileType(){
		if(!m_data_list.empty()){
			return m_data_list[0].type;
		}
		return FILETYPE_NULL;
	};

#if 0
	FileInfo* GetFileInfo(){
		if(! m_files.empty()){
			return m_files[0];
		}
		return NULL;
	};
#endif

	int GetNumFiles(){
		return (int)(m_data_list.size());		
	};

    bool ExistData(){
		return (GetNumFiles() > 0);
    }

	ATOMS_DATA* GetAtomData(){
		if (!m_data_list.empty()) {
			return m_data_list[0].GetDataPointer();
		}

		return NULL;
	};

	int GetNumAtoms() {
		if (!m_data_list.empty()) {
			return m_data_list[0].GetNumParticles();
		}

		return 0;
	};

	BOND_INFO* GetBondData() {
		if (!m_data_list.empty()) {
			return m_data_list[0].GetBond();
		}

		return nullptr;
	};

	bool GetBoxSize(mat33d* axis, vec3d* org) {
		if (!m_data_list.empty()) {
			return m_data_list[0].GetBoxSize(axis, org);
		}
		return false;
	}

    /*
    仕様の要変更
    */
	int SelectAtom(const vec3d& ray, const vec3d& org);

	int GetSelectedAtomID(){ return m_selected_id;};
		
	int GetSelectedAtomInfo(int *a_knd, vec3d* a_r){
		ATOMS_DATA* dat = m_data_list[0].GetDataPointer();
		m_selected_id;
		if(a_knd && (dat->knd)){
			*a_knd = dat->knd[m_selected_id];
		}
		if(a_r && (dat->r)){
			a_r->Set(dat->r[m_selected_id].x, dat->r[m_selected_id].y, dat->r[m_selected_id].z);
		}
		return m_selected_id;
	};


//レンダリング関連//////////////////////////////////////////////////////////////////////////////////////

	//view by OpenMX//
	void DisplayGL15(const double* view_matrix, const double focus_distance);

	//view by DisplayDX11//
	void DisplayDX11(const double* view_matrix, const double focus_distance);


	//output data//
	void OutputBMPMagGL(const TCHAR* path, int magnify, const double* view_matrix, const double focus_distance){

		//表示用のプロパティーの取得//
		RenderingProperty rendering_property;
		m_aiproperty->GetRenderingProperty(&rendering_property);

		//レンダリング//
		OutputBMfromGLMag(m_data_list,  path, m_client_w, m_client_h, magnify, view_matrix, focus_distance, rendering_property);
	}
	
	void OutputFieldRaytracing(const TCHAR* path, int magnify, const double* view_matrix, const double focus_distance){
		OutputRaytracing(m_data_list[0], path, m_client_w, m_client_h, magnify, view_matrix, focus_distance);
	}
	
	void OutputStereoBMPGL(const double* view_matrix, const double focus_distance);
	void OutputSequentialBMPGL(const double* view_matrix, const double focus_distance);

	void OutputBMPMagDX11(const TCHAR* path, int magnify, const double* view_matrix, const double focus_distance){

		//表示用のプロパティーの取得//
		RenderingProperty rendering_property;
		m_aiproperty->GetRenderingProperty(&rendering_property);

		//レンダリング//
		OutputDX11(m_data_list, path, m_client_w, m_client_h, magnify, view_matrix, focus_distance, rendering_property);
	}


	//analyze for view//
	int ResetBond();
	int ResetColorByPressure();


private:

						//CameraGL* m_camera;
	std::vector<LawData> m_data_list;

	int m_client_w;
	int m_client_h;


	int m_selected_id;

	int m_outNumBMP;

	//file load//
	int OpenDataFile(const TCHAR* filepath);
	

	AIScopeProperty* m_aiproperty;

};

#endif // AIScopeEvent_H