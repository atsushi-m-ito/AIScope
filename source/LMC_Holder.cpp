
#include "LMC_Holder.h"
#include "renew.h"
#include "mychardump.h"

static const int MAX_EVENT_PAR_STEP = 10;

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

LMC_Holder::LMC_Holder() : 
		m_lmc_r ( NULL ),
		m_frameNo( 0 )
{
	
}


LMC_Holder::~LMC_Holder(){
	
	if(m_lmc_r){ 
		m_lmc_r->Close();
		delete m_lmc_r;
	}
	
}

int LMC_Holder::Load(const TCHAR* filepath){

	if(m_lmc_r) {return -1;}


	m_lmc_r = new LMC_Reader();
	
//	int length = _tcslen(filepath);
//	char* mbfpath = new char[length*2];
//	TCharToMultiByte(mbfpath, filepath, length);
	char* mbfpath = MbsDup(filepath);
	m_lmc_r->Open(mbfpath);
	delete [] mbfpath;
	


	m_lmc_r->GetNumGrid(&m_grid_x, &m_grid_y, &m_grid_z);

	m_lmc_r->GetBoxSize(m_boxaxis);

	m_num_elements = m_lmc_r->GetNumElements();

	int grid_sz = m_grid_x * m_grid_y * m_grid_z * m_num_elements;

	m_grid = new LMC_INT[grid_sz];

	m_lmc_r->ReadInitialGrid(m_grid);


	m_event_buf_sz = 1000000;
	m_events = new LMC_EVENT[m_event_buf_sz];
	m_num_events = 0;

	m_step_buf_sz = 1000000;
	m_steps = new LMC_STEP[m_step_buf_sz];
	m_num_steps = 0;

	double time;
	int event_per_step = m_lmc_r->ReadStep(&time, &(m_events[m_num_events]));
	while( event_per_step > 0){

		//バッファ拡張//
		m_steps[m_num_steps].num_event = event_per_step;
		m_steps[m_num_steps].time = time;

		m_num_steps++;
		if(m_num_steps == m_step_buf_sz){
			renew_preserve<LMC_STEP>(&m_steps, m_step_buf_sz, m_step_buf_sz * 2);
			m_step_buf_sz *= 2;
		}
		
		//バッファ拡張//
		m_num_events += event_per_step;
		if(m_num_events >= m_event_buf_sz - MAX_EVENT_PAR_STEP){
			renew_preserve<LMC_EVENT>(&m_events, m_event_buf_sz, m_event_buf_sz * 2);
			m_event_buf_sz *= 2;
		}
		
		event_per_step = m_lmc_r->ReadStep(&time, &(m_events[m_num_events]));
	}

	m_current_enent = m_events;



	return 0;
}




void LMC_Holder::GetBoxaxis(mat33d* boxaxis, vec3d* boxorg){

	boxaxis->Set(m_boxaxis);
	boxorg->Set(0.0, 0.0, 0.0);

}

int LMC_Holder::GetNumElements(){
	return m_lmc_r->GetNumElements();
}

void LMC_Holder::GetNumGrid(int* grid_x, int* grid_y, int* grid_z){
	m_lmc_r->GetNumGrid( grid_x,  grid_y,  grid_z);
}


LMC_INT* LMC_Holder::GetDataPointer(){
	return m_grid;
}


int LMC_Holder::SetFrameNum(int timeframe){

	int newFrameNo = timeframe;

	if(newFrameNo >= m_frameNo){

		if(newFrameNo > m_num_steps){ newFrameNo = m_num_steps;}

		LMC_EVENT* pe = m_current_enent;
		for(int i = m_frameNo; i < newFrameNo; i++){
			
			for(int k = 0; k < m_steps[i].num_event; k++){
				int ig = pe->grid_index;
				m_grid[ig] += pe->delta;
				pe++;
			}
		}
		m_current_enent = pe;
		

	}else{
		if(newFrameNo < 0){ newFrameNo = 0;}

		LMC_EVENT* pe = m_current_enent;
		for(int i = m_frameNo - 1; i >= newFrameNo; i--){
			for(int k = 0; k < m_steps[i].num_event; k++){
				pe--;
				int ig = pe->grid_index;
				m_grid[ig] -= pe->delta;
			}
		}
		m_current_enent = pe;
		
	}


	m_frameNo = newFrameNo;
	return m_frameNo;

}