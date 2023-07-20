//=====================================
// スクリプト用の3dxml_loader管理クラス
//
//
// 3dxmlを読み込み＆表示する
//
// 
//=====================================

#ifndef __field3dloader_h__
#define __field3dloader_h__



#pragma once

#include "targetver.h"
#include <windows.h>

#include <stdio.h>
#include <tchar.h>
#include <string.h>
#include <vector>

#include "sdkwgl.h"

#include "vec3.h"
#include "mat33.h"
#include "IDataAtoms.h"
#include "ATOMS_DATA.h"

#include "debugprint.h"



struct FIELD3D_FRAME {
	//long64 filepos;
	float* fieldb;
	int grid_x;//グリッド数
	int grid_y;
	int grid_z;
	mat33d boxaxis;
	vec3d boxorg;
};

class Field3DLoader : public IDataContainer{
	
private:

	std::vector<FIELD3D_FRAME> m_frames;
	int m_frame_count=0;
	int m_frame_index=0;
	
	
	//bool m_created_tex;

	//GLuint m_textureName;
	
	

	//float* LoadCube(const TCHAR* filename, FIELD3D_FRAME* frame);
	
public:
	
	Field3DLoader() = default;
	virtual ~Field3DLoader();
	
	bool Load(const TCHAR* filename) {
		return Load(filename, nullptr);
	}

	//load by append mode//
	bool Load(const TCHAR* filename, ATOMS_DATA* dat);
	

	void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg){
		*boxaxis = m_frames[m_frame_index].boxaxis;
		*boxorg = m_frames[m_frame_index].boxorg;
		
	};

	void GetGridSize(int* pgridx, int* pgridy, int* pgridz ){
		*pgridx = m_frames[m_frame_index].grid_x;
		*pgridy = m_frames[m_frame_index].grid_y;
		*pgridz = m_frames[m_frame_index].grid_z;
	};
	
	FIELD3D_FRAME* GetFieldData(){
		return &(m_frames[m_frame_index]);
	};
	
	int SetFrameNum(int timeframe);
	
};


#endif    // __field3dloader_h__


