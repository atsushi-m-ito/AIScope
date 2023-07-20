
#define _CRT_SECURE_NO_DEPRECATE

#include <stdlib.h>
#include <string.h>
#include "md3_reader.h"

static const int LINE_LEN = 2048;


static const int SPECIES_COUNT = 118 + 1;

static const int ERROR_MISSING_PARTICLE = -3;
static const int ERROR_OVERFLOW_PARTICLE = -4;


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



MD3_Reader::MD3_Reader() : 
		m_fp (NULL),
		m_pcnt(-1)
{
	
	
}

MD3_Reader::~MD3_Reader(){
	Close();
}


int MD3_Reader::Open(const char* filepath){
	
	if(m_fp){
		return -2;
	}

	m_fp = fopen(filepath, "r");
	if( ! m_fp){
		m_fp = NULL;
		return -1;
	}
	
	
	//ヘッダ部だけ読み込み

	char line[LINE_LEN];
	if(fgets(line,LINE_LEN,m_fp) != NULL){
		sscanf(line,"%d", &m_pcnt);
		if(m_pcnt<=0){return -3;}
		
	}
	
	

	return 0;
}

void MD3_Reader::Close(){
	if(m_fp){
		fclose(m_fp);
		m_fp = NULL;
	}
	
	m_pcnt = 0;


}


int MD3_Reader::GetParticleCount(){
	
	return m_pcnt;
	
}




int MD3_Reader::Read(int* knd, double* r, double* v, double* f, double *boxaxis, char* text){

	char line[LINE_LEN];

	//コメント行の読み出し//
	if(fgets(line,LINE_LEN,m_fp) == NULL){
		m_pcnt = -2;
		return -1;
	}

	if (text){
		strcpy(text, line);
	}

	
	double *pr = r;
	double *pv = v;
	double *pf = f;
	int count = 0;
	int next_count = -2;

	while(fgets(line,LINE_LEN,m_fp) != NULL){
			
		//最初のカラムを読み込んで判定//
		//BOX or 元素記号 or 数値(次のフレームのヘッダ)//

		char* tp = strtok(line, " \t\r\n");
        
        if (tp == NULL){
            break;
        }
        
		if(strcmp(tp, "BOX")==0){
			for(double* pb = boxaxis; pb != boxaxis + 9; pb++){
				tp = strtok(NULL, " \t\r\n");
				*pb = strtod(tp, NULL);
			}

		} else if (('0' <= *tp) && (*tp <= '9')) {
			//次の行のヘッダを読み込んだのでストップ
			next_count = atoi(tp);
			break;
		} else if (strcmp(tp, "STRESS") == 0){
			//応力読み飛ばし//

		}else{	//read atom//

			if(count == m_pcnt){	//粒子がオーバーフローした//
				next_count = ERROR_OVERFLOW_PARTICLE;
				break;
			}


			*knd = GetAtomicNumber(tp);
			knd++;

			for(double* p_end = pr + 3; pr != p_end; pr++){
				tp = strtok(NULL, " \t\r\n");
				*pr = strtod(tp, NULL);
			}

			if(pv){ //velocityはNULLが許される//
				tp = strtok(NULL, " \t\r\n");
				*pv = strtod(tp, NULL);
				pv++;
				tp = strtok(NULL, " \t\r\n");
				*pv = strtod(tp, NULL);
				pv++;
				tp = strtok(NULL, " \t\r\n");
				*pv = strtod(tp, NULL);
				pv++;
			}

			if(pf){ //forceはNULLが許される//
				tp = strtok(NULL, " \t\r\n");
				*pf = strtod(tp, NULL);
				pf++;
				tp = strtok(NULL, " \t\r\n");
				*pf = strtod(tp, NULL);
				pf++;
				tp = strtok(NULL, " \t\r\n");
				*pf = strtod(tp, NULL);
				pf++;
			}

			count++;

		}

	
	}


		if(count < m_pcnt){	//error: 粒子数が足りない//
			m_pcnt = ERROR_MISSING_PARTICLE;
//		}else if(count > m_pcnt){
//			m_pcnt = ERROR_OVERFLOW_PARTICLE;
		}else{
			m_pcnt = next_count;
		}
	//この時点で次の行のヘッダーを読まずに抜けた場合は//
	//m_pcnt = -1になっているはず//

	return count;
}
