#include <algorithm>

#include "Field3DLoader.h"
#include "bif_reader.h"
#include "cube_reader.h"

#pragma warning(disable : 4996)




Field3DLoader::~Field3DLoader(){
	for(int i = 0; i < m_frame_count; i++){
		delete [] m_frames[i].fieldb;
	}	
}

bool Field3DLoader::Load(const TCHAR* filename, ATOMS_DATA* dat){

	size_t len = _tcslen(filename);
	if(_tcscmp(filename + len - 5, _T(".cube"))==0){ 
		CubeReader cube_r;
		CubeReader::Frame cube_frame;
		float* fieldf = cube_r.LoadCube(filename, &cube_frame, dat);
		m_frames.emplace_back();
		FIELD3D_FRAME& frame = m_frames.back();
		frame.boxaxis = cube_frame.boxaxis;
		frame.boxorg = cube_frame.boxorg;
		frame.grid_x = cube_frame.grid_x;
		frame.grid_y = cube_frame.grid_y;
		frame.grid_z = cube_frame.grid_z;
		
		//m_boxaxis.m11 = (double)m_grid_x * m_boxaxis.a.x;
		//m_boxaxis.m22 = (double)m_grid_y * m_boxaxis.b.y;
		//m_boxaxis.m33 = (double)m_grid_z * m_boxaxis.c.z;
		
		//BYTEカラーデータへ変換//
		int mesh_sz = frame.grid_x* frame.grid_y* frame.grid_z;
		frame.fieldb = fieldf;

		++m_frame_count;

		return true;
	}else if (_tcscmp(filename + len - 4, _T(".bif"))==0){ 
		
		BIF_Reader* bif = new BIF_Reader();
		bif->Open(filename);

		field_int grid_x;
		field_int grid_y;
		field_int grid_z;
		double delta_x;
		double delta_y;
		double delta_z;
		size_t sz = 0;
		double* fieldd = NULL;

		int res = bif->GetSize(&grid_x, &grid_y, &grid_z, &delta_x, &delta_y, &delta_z);
		while (res > 0){
			size_t full_sz = (size_t)grid_x * (size_t)grid_y * (size_t)grid_z;
			if( full_sz > sz){
				if( fieldd ){ delete []  fieldd;}
				fieldd = new double[full_sz];
				sz = full_sz;
			}
			bif->Read(fieldd);


			float* fieldf = new float[full_sz];
			for (size_t i = 0; i < full_sz; ++i) {
				fieldf[i] = (float)fieldd[i];
			}
			
	//		int mesh_sz = m_grid_x*m_grid_y*m_grid_z;
			m_frames.emplace_back();
			m_frames[m_frame_count].fieldb = fieldf;

			m_frames[m_frame_count].grid_x = (int)grid_x;
			m_frames[m_frame_count].grid_y = (int)grid_y;
			m_frames[m_frame_count].grid_z = (int)grid_z;
			m_frames[m_frame_count].boxaxis.Set(delta_x * (double)grid_x, 0.0, 0.0,  0.0, delta_y * (double)grid_y, 0.0,  0.0, 0.0, delta_z * (double)grid_z);
			m_frames[m_frame_count].boxorg.Clear();
			m_frame_count++;

			res = bif->GetSize(&grid_x, &grid_y, &grid_z, &delta_x, &delta_y, &delta_z);
		}

		bif->Close();
		delete bif;

		
		delete [] fieldd;

		return true;
	}
	
	return false;
	

	
}


int Field3DLoader::SetFrameNum(int timeframe){

	int newFrameNo = timeframe;

	if(newFrameNo >= m_frame_index){

		if(newFrameNo >= m_frame_count){ newFrameNo = m_frame_count-1;}

	}else{
		if(newFrameNo < 0){ newFrameNo = 0;}

	}


	m_frame_index = newFrameNo;
	return m_frame_index;

}