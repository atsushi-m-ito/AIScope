
#ifndef __trajectory_h__
#define __trajectory_h__


#pragma once

#include <stdlib.h>
#include <stdio.h>

#include <tchar.h>
#include <string.h>
#include <math.h>
#include <vector>


#include "vec3.h"
#include "mat33.h"
#include "IDataAtoms.h"


struct TRAJECTORY_POS{
	float x;
	float y;
	float z;
	int tm;
};
	
class Trajectory  
{
private:

	

public:
	TRAJECTORY_POS *m_buffer;
	int m_bufsz;

	Trajectory* next;
	
	Trajectory();
	virtual ~Trajectory();

	//int ReadTextFile(FILE* fp);
	//void Render(int frame, double particleRadius, float* traj_color);
	
	void GetMaxsize(float *max_x, float *max_y, float *max_z);
	void GetMaxtime(int *frame);
	
	void Set(const TRAJECTORY_POS* buf, int count);

};



struct TrajectoryData {
	const vec3f* positions;
	const int* strip_indexes;
	const int* strip_lengths;
	int num_strips;
};

class BCALoader : public IDataContainer{
public:
	BCALoader(const TCHAR *filename);
	virtual ~BCALoader();

	//void GetBoxsize(vec3d* boxsize, vec3d* boxorg);
	void GetBoxaxis(mat33d* axis, vec3d* org);

		

	const TrajectoryData* GetData();

	int GetFrameNum();
	int SetFrameNum(int timeframe);
	//void Rendering();
	/*
	void SetRadius(double particleRadius);
	void SetColor(float* color){
		memcpy(m_traj_color, color, sizeof(float)*3);
	};
	*/


private:

	std::vector<vec3f> m_position_list;
	std::vector<int> m_time_list;
	std::vector<int> m_trajectory_lengths;

	bool m_flag_remake;
	std::vector<int> m_visible_indexs;
	std::vector<int> m_visible_length;
	TrajectoryData m_visible_data;
	Trajectory* m_trajectories;

	int m_frameNo;		//現在のタイムフレーム.
	int m_frame_max;	//最終フレーム数.
	union {
		double m_box_axisorg[12];
		struct {
			mat33d m_box_axis;	//ボックスサイズ.
			vec3d m_box_org;	//ボックス左下座標
		};
	};
	double m_particleRadius;

	float m_traj_color[4];

};



#endif // __trajectory_h__