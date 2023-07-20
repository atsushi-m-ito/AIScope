// AtomRendererGL.h: AtomRendererGL クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#ifndef __LMC_Holder_h__
#define __LMC_Holder_h__

#pragma once

#include <stdio.h>
#include <tchar.h>
#include <vector>


#include "vec3.h"
#include "mat33.h"
#include "io64.h"
#include "IDataAtoms.h"

#include "lmc_reader.h"



class LMC_Holder : public IDataContainer{

public:

	LMC_Holder();
	virtual ~LMC_Holder();

	int Load(const TCHAR* filepath);

	void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg);
	void GetNumGrid(int* grid_x, int* grid_y, int* grid_z);
	int GetNumElements();


	LMC_INT* GetDataPointer();
	int SetFrameNum(int timeframe);

private:

	LMC_Reader* m_lmc_r;

	struct LMC_STEP {
		int num_event;
		double time;
	};

	int m_grid_x;
	int m_grid_y;
	int m_grid_z;
	double m_boxaxis[12];
	int m_num_elements;
	int m_bufsz;

	LMC_INT* m_grid;

	int m_event_buf_sz;
	LMC_EVENT* m_events;
	int m_num_events;
	LMC_EVENT* m_current_enent;

	int m_step_buf_sz;
	LMC_STEP* m_steps;
	int m_num_steps;

	int m_frameNo;
};

#endif // __LMC_Holder_h__

