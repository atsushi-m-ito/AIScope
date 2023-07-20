#include <float.h>

#include "trajectory.h"
//#include "sdkwgl.h"
#include "mychardump.h"


//#include "ai_funcs.h"

#pragma warning(disable : 4996)
/*
#ifdef _UNICODE
#define _ttof(x) strtod((x),NULL)
#else
#define _ttof atof
#endif
*/

Trajectory::Trajectory() :
	m_buffer(NULL),
	m_bufsz(0),
	next(NULL)
{

}


Trajectory::~Trajectory(){
	delete [] m_buffer;
}

void Trajectory::Set(const TRAJECTORY_POS* buf, int count){
	m_buffer = new TRAJECTORY_POS[count];
	m_bufsz = count;
	
	for(int i = 0; i < count; i++){
		m_buffer[i] = buf[i];
	}
}



void Trajectory::GetMaxsize(float *max_x_f, float *max_y_f, float *max_z_f){
	double max_x = (double)*max_x_f;
	double max_y = (double)*max_y_f;
	double max_z = (double)*max_z_f;


	TRAJECTORY_POS* buf;
	for (buf = m_buffer; buf != m_buffer + m_bufsz; buf++){
	
		if(fabs(buf->x) > max_x ){ max_x = fabs(buf->x);}
		if(fabs(buf->y) > max_y ){ max_y = fabs(buf->y);}
		if(fabs(buf->z) > max_z ){ max_z = fabs(buf->z);}
	}

	*max_x_f = (float)max_x;
	*max_y_f = (float)max_y;
	*max_z_f = (float)max_z;
	
}


void Trajectory::GetMaxtime(int *frame){


	TRAJECTORY_POS* buf;
	for (buf = m_buffer; buf != m_buffer + m_bufsz; buf++){
	
		if(buf->tm > *frame ){ *frame = buf->tm;}
	}

}


////////////////////////////////////////////////////////
//
//	BCALoaderクラス
//
////////////////////////////////////////////////////////
BCALoader::BCALoader(const TCHAR *filename) :
	m_trajectories(NULL),
	m_flag_remake(true)
{


	//ファイル読み込み
	
	const char* fileppath = MyCharDump<char, TCHAR>(filename);
	FILE* fp = fopen(fileppath, "r");
	delete[] fileppath;
	if(fp == NULL){
		MessageBox(NULL, _T("error:trajectory"), _T("error"), MB_OK);
	}
		
	int flag_box = 0;

	m_frameNo = 0;
	m_frame_max = 0;


	

	const int line_size = 1024;
	char line[line_size];
	int frameCnt = 0;
	//TCHAR* tp;
	const char DELIMITER[] = " \t\r\n";

	int i = 0;
	int strip_length = 0;

	
	while(fgets(line, line_size, fp)){

		char* tp = strtok(line, DELIMITER);
		if(tp == NULL){
			//空の行があるので次の軌跡として新しいオブジェクトを生成
			if (strip_length > 0) {
				m_trajectory_lengths.push_back(strip_length);
			}
			strip_length = 0;

		//boxsizeの読み込み//
		}else if(strcmp(tp, "BOX") == 0){
			tp = strtok( NULL, DELIMITER );
			m_box_axis.a.x = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_axis.a.y = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_axis.a.z = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_axis.b.x = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_axis.b.y = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_axis.b.z = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_axis.c.x = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_axis.c.y = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_axis.c.z = strtod(tp, NULL);
		
			
			tp = strtok( NULL, DELIMITER );
			m_box_org.x = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_org.y = strtod(tp, NULL);
		
			tp = strtok( NULL, DELIMITER );
			m_box_org.z = strtod(tp, NULL);
			
			flag_box = 1;

		//トラジェクトリーの読み込み//
		}else{
			char* tpy = strtok(NULL, DELIMITER);
			char* tpz = strtok(NULL, DELIMITER);

			m_position_list.push_back(vec3f((float)strtof(tp, NULL),
				(float)strtof(tpy, NULL),
				(float)strtof(tpz, NULL)));

			tp = strtok(NULL, DELIMITER);
			if(tp){
				m_time_list.push_back(atoi(tp));
			}else{
				m_time_list.push_back(0);
			}
			strip_length++;
		}
		
	}

	//最後のトラジェクトリーを追加//
	if(i > 0){	
		if (strip_length > 0) {
			m_trajectory_lengths.push_back(strip_length);
		}
	}

	m_frame_max = 0;
	for (auto it = m_time_list.begin(), it_end = m_time_list.end(); it != it_end; ++it) {
		if (m_frame_max < *it)m_frame_max = *it;
	}

	//box情報が無い場合は昔の取り決めに従う//
	if(flag_box == 0){
	
		vec3f boxmax(0.0f, 0.0f, 0.0f);
		for (auto it = m_position_list.begin(), it_end = m_position_list.end(); it != it_end; ++it) {
			if (boxmax.x < abs(it->x)) boxmax.x = abs(it->x);
			if (boxmax.y < abs(it->y)) boxmax.y = abs(it->y);
			if (boxmax.z < abs(it->z)) boxmax.z = abs(it->z);
		}

		
		m_box_org.Set( - boxmax.x, -boxmax.y, - boxmax.z);
		m_box_axis.Set( boxmax.x * 2.0, 0.0, 0.0, 0.0, boxmax.y * 2.0, 0.0,  0.0, 0.0, boxmax.z * 2.0);
	}

};


BCALoader::~BCALoader(){

}

/*
void BCALoader::GetBoxsize(vec3d* boxsize, vec3d* boxorg){
	*boxsize = m_box_size;
	*boxorg = m_box_org;

}
*/

void BCALoader::GetBoxaxis(mat33d* axis, vec3d* org){
	*axis = m_box_axis;
	org->x = (m_box_org.x);
	org->y = (m_box_org.y);
	org->z = (m_box_org.z);

}

int BCALoader::GetFrameNum(){
	return m_frameNo;
}

int BCALoader::SetFrameNum(int timeframe){
	int prev_frame = m_frameNo;
	
	m_frameNo = max(0, min(timeframe, m_frame_max - 1));

	if (prev_frame != m_frameNo) {
		m_flag_remake = true;
	}

	return m_frameNo;

}


const TrajectoryData* BCALoader::GetData() {
	if (m_position_list.empty()) {
		return NULL;
	}

	if (m_flag_remake) {
		m_flag_remake = false;

		m_visible_data.positions = &(m_position_list[0]);

		m_visible_indexs.clear();
		m_visible_length.clear();

		int k = 0;
		for (int i = 0; i < m_trajectory_lengths.size(); ++i) {
			int visible_length = 0;
			const int k_end = k + m_trajectory_lengths[i];
			for (; k < k_end; ++k) {
				if (m_time_list[k] > m_frameNo) {
					break;
				}

				visible_length++;
				m_visible_indexs.push_back(k);
			}

			if (visible_length > 0) {
				m_visible_length.push_back(visible_length);
			}
		}
		
		m_visible_data.strip_indexes = m_visible_indexs.empty() ? NULL : &(m_visible_indexs[0]);
		m_visible_data.strip_lengths = m_visible_length.empty() ? NULL : &(m_visible_length[0]);
		m_visible_data.num_strips = m_visible_length.size();

	}

	return &m_visible_data;
}


/*
void BCALoader::SetRadius(double particleRadius){
	m_particleRadius = particleRadius;
}
*/
