
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include "lmc_reader.h"

static const int LINE_LEN = 2048;


static const int SPECIES_COUNT = 118 + 1;

static const int STATE_INITIALIZED = 1;
static const int STATE_EOF = -1;
static const int ERROR_OVERFLOW_PARTICLE = -4;



/*
//OpenMXで入力する元素記号//
static const char* openmx_species_list[] = {
		"E",		//empty軌道//
		"H", "He",	//第1周期
		"Li", "Be",	"B", "C", "N", "O", "F", "Ne",		//第2周期
		"Na", "Mg",	"Al", "Si", "P", "S", "Cl", "Ar",	//第3周期
		"K", "Ca", "Sc", "Ti", "V", "Cr", "Mn", "Fe", "Co", "Ni", "Cu", "Zn", "Ga", "Ge", "As", "Se", "Br", "Kr",	//第4周期
		"Rb", "Sr", "Y", "Zr", "Nb", "Mo", "Tc", "Ru", "Rh", "Pd", "Ag", "Cd", "In", "Sn", "Sb", "Te", "I", "Xe",	//第5周期
		"Cs", "Ba",	//第6周期(Cs, Ba)
		"La", "Ce", "Pr", "Nd", "Pm", "Sm", "Eu", "Gd", "Tb", "Dy", "Ho", "Er", "Tm", "Yb", "Lu",	////第6周期(ランタノイド)
		"Hf", "Ta", "W", "Re", "Os", "Ir", "Pt", "Au", "Hg", "Tl", "Pb", "Bi", "Po", "At", "Rn",//第6周期(Hf-Rn)
		"Fr", "Ra",	//第7周期(Fr, Ra)
		"Ac", "Th", "Pa", "U", "Np", "Pu", "Am", "Cm", "Bk", "Cf", "Es", "Fm", "Md", "No", "Lr",	////第7周期(アクチノイド)
		//104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,//第7周期(Rf-Uuo)
};


//元素記号の文字列から原子番号を返す//
inline static int GetAtomicNumber(const char* atomicsymbol){
	for(int i = 0; i < SPECIES_COUNT; i++){
		if(strcmp(atomicsymbol, openmx_species_list[i])==0){
			return i;
		}
	}
	return -1;
}
*/

LMC_Reader::LMC_Reader() : 
		m_fp (NULL),
		m_state(0),
		m_next_step_time(0.0)
{
	
	
}

LMC_Reader::~LMC_Reader(){
	Close();
}


int LMC_Reader::Open(const char* filepath){
	
	if(m_fp){
		return -2;
	}

	m_fp = fopen(filepath, "r");
	if( ! m_fp){
		m_fp = NULL;
		return -1;
	}
	
	
	ReadHeader();

	return 0;
}

void LMC_Reader::Close(){
	if(m_fp){
		fclose(m_fp);
		m_fp = NULL;
	}
	

}

int LMC_Reader::ReadHeader(){

	char line[LINE_LEN];

	//USED ELEMENTS//
	if(fgets(line,LINE_LEN,m_fp) == NULL){
		return -7;
	}

	char* tp = strtok(line,  " \t\r\n");
	if(strcmp(tp, "ELEMENTS") != 0){
		return -8;
	}
	
	int num = 0;
	tp = strtok(NULL,  " \t\r\n");
	while(tp){
		strcpy(m_elements[num].symbol, tp);
		num++;
		tp = strtok(NULL,  " \t\r\n");
	}
	m_num_elements = num;




	//GRID NUMBER//
	if(fgets(line,LINE_LEN,m_fp) == NULL){
		return -1;
	}

	tp = strtok(line,  " \t\r\n");
	if(strcmp(tp, "GRID") != 0){
		return -2;
	}

	tp = strtok(NULL,  " \t\r\n");
	m_grid_x = atoi(tp);

	tp = strtok(NULL,  " \t\r\n");
	m_grid_y = atoi(tp);

	tp = strtok(NULL,  " \t\r\n");
	m_grid_z = atoi(tp);

	//BOXSIZE//
	if(fgets(line,LINE_LEN,m_fp) == NULL){
		return -3;
	}

	tp = strtok(line,  " \t\r\n");
	if(strcmp(tp, "BOX") != 0){
		return -4;
	}
	
	for(int i = 0; i< 9; i++){
		tp = strtok(NULL,  " \t\r\n");
		m_boxaxis[i] = strtod(tp, NULL);
	}
	
	return 0;
}


int LMC_Reader::GetNumElements(){
	return m_num_elements;
}

void LMC_Reader::GetNumGrid(int* grid_x, int* grid_y, int* grid_z){
	*grid_x = m_grid_x;
	*grid_y = m_grid_y;
	*grid_z = m_grid_z;

}

void LMC_Reader::GetBoxSize(double* boxaxis){
	for(int i = 0; i < 9; i++){
		boxaxis[i] = m_boxaxis[i];
	}
}

int LMC_Reader::ReadInitialGrid(LMC_INT* buffer){ 

	char line[LINE_LEN];

	int grid_size = m_grid_x * m_grid_y * m_grid_z * m_num_elements;
	memset(buffer, 0, sizeof(LMC_INT) * grid_size);
	
	for(int i = 0; i < grid_size; i+=m_num_elements){
		
		if(fgets(line,LINE_LEN,m_fp) == NULL){
			return -5;
		}

		char* tp = strtok(line,  " \t\r\n");
		if(strcmp(tp, "INIT") != 0){
			return -6;
		}

		tp = strtok(NULL,  " \t\r\n");
		while(tp){
			int index = GetDataIndex(tp, m_elements, m_num_elements);
			if(index == -1){
				break;
			}

			tp = strtok(NULL,  " \t\r\n");
			
			buffer[i+index] = atoi(tp);

			tp = strtok(NULL,  " \t\r\n");
		}

	}


	//最初のステップのヘッダー行の読み出し//
	if(fgets(line,LINE_LEN,m_fp) == NULL){
		m_state = STATE_EOF;
	}else{

		m_state = STATE_INITIALIZED;

		char* tp = strtok(line,  " \t\r\n");
		if(strcmp(tp, "STEP") != 0){
			m_state = STATE_EOF;
			return 0;
		}

		tp = strtok(NULL,  " \t\r\n");
		m_next_step_time = strtod(tp,NULL);

	}
	
	return 0;

}


//元素記号の文字列から原子番号を返す//
inline int LMC_Reader::GetDataIndex(const char* atomicsymbol, const ATOMIC_ELEMENT* elements, int num_elements){
	for(int i = 0; i < num_elements; i++){
		if(strcmp(atomicsymbol, elements[i].symbol)==0){
			return i;
		}
	}
	return -1;
}



int LMC_Reader::ReadStep(double* time, LMC_EVENT* events){

	if(	m_state == STATE_EOF ){
		return -1;
	}

	
	*time = m_next_step_time;
	int num_event = 0;


	//////////////////////////////////////////
	//イベント一行の読み込み
	char line[LINE_LEN];
	LMC_EVENT* ep = events;
	while(fgets(line,LINE_LEN,m_fp)){
		
		//STEP指定子なら次のステップなので読み込み中断//
		char* tp = strtok(line,  " \t\r\n");
		if(strcmp(tp, "STEP") == 0){
			tp = strtok(NULL,  " \t\r\n");
			m_next_step_time = strtod(tp, NULL);//次ステップの時間保存//
			return num_event;
		}

		//エラー//
		if(strcmp(tp, "EVENT") != 0){
			//error//
			return -2;
		}

		//イベント情報の読み込みと記録//
		
		tp = strtok(NULL,  " \t\r\n");
		int ix = atoi(tp);
		
		tp = strtok(NULL,  " \t\r\n");
		int iy = atoi(tp);
		
		tp = strtok(NULL,  " \t\r\n");
		int iz = atoi(tp);

		
		tp = strtok(NULL,  " \t\r\n");
		int index = GetDataIndex(tp, m_elements, m_num_elements);
		if(index == -1){
			break;
		}
		
		ep->grid_index= index + (ix + m_grid_x * (iy + m_grid_y * iz)) * m_num_elements;
		
		tp = strtok(NULL,  " \t\r\n");
		ep->delta = atoi(tp);

		//一行読み込み終了//
		ep++;
		num_event++;

	}

	m_state = STATE_EOF;
	return num_event;		//ここから抜けるときはファイル末尾のみ//
}
