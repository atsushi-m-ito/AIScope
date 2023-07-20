#pragma warning(disable : 4996)

#include "krb_reader.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <float.h>


static const int SPECIES_COUNT = 118 + 1;

static const int ERROR_MISSING_PARTICLE = -3;
static const int ERROR_OVERFLOW_PARTICLE = -4;



//OpenMXで入力する元素記号//
static const char* openmx_species_list[] = {
    "E",		//empty軌道//
    "H", "He",	//第1周期
    "Li", "Be", "B", "C", "N", "O", "F", "Ne",		//第2周期
    "Na", "Mg", "Al", "Si", "P", "S", "Cl", "Ar",	//第3周期
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
    for (int i = 0; i < SPECIES_COUNT; i++){
        if (strcmp(atomicsymbol, openmx_species_list[i]) == 0){
            return i;
        }
    }
    return -1;
}




int KRB_Reader::Open(const char* filepath){
    FILE* fp = fopen(filepath, "r");
    if (fp == nullptr){
        return -1;
    }

    const int LINE_SIZE = 1024;
    char line[LINE_SIZE];
    const char DELIMITER[] = " \t\r\n";

    m_buffer_size = 1024 * 1024;
    m_buffer = new KRB_PARTICLE[m_buffer_size];
    m_count = 0;
    //m_max_num_atoms = 0;
//    m_min_space.Set(FLT_MAX, FLT_MAX, FLT_MAX);
//    m_max_space = -m_min_space;

    //boxsizeの読み込み//
    while (fgets(line, LINE_SIZE, fp)){
        char* tp = strtok(line, DELIMITER);
        if (tp == NULL){
            continue;
        }

        if (strcmp(tp, "BOX") != 0){
            printf("error");
            return -3;
        }

        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_axis.m11 = strtof(tp, nullptr);
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_axis.m12 = strtof(tp, nullptr);
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_axis.m13 = strtof(tp, nullptr);
        
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_axis.m21 = strtof(tp, nullptr);
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_axis.m22 = strtof(tp, nullptr);
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_axis.m23 = strtof(tp, nullptr);
        
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_axis.m31 = strtof(tp, nullptr);
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_axis.m32 = strtof(tp, nullptr);
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_axis.m33 = strtof(tp, nullptr);

        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_org.x = (tp) ? strtof(tp, nullptr) : 0.0;
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_org.y = (tp) ? strtof(tp, nullptr) : 0.0;
        tp = strtok(nullptr, DELIMITER);
        m_krb_info.box_org.z = (tp) ? strtof(tp, nullptr) : 0.0;

        break;

    }


    while (fgets(line, LINE_SIZE, fp)){

        char* tp = strtok(line, DELIMITER);
        if (tp == nullptr){
            continue;
        }

        //バッファの再確保//
        if (m_count + 1 == m_buffer_size){
            m_buffer_size *= 2;
            KRB_PARTICLE* re_buffer = new KRB_PARTICLE[m_buffer_size];
            memcpy(re_buffer, m_buffer, sizeof(KRB_PARTICLE) * m_count);
            delete[] m_buffer;
            m_buffer = re_buffer;
        }

        


        m_buffer[m_count].step = atoll(tp);
		

        tp = strtok(nullptr, DELIMITER);
        if (strcmp(tp, "BCA") == 0){
            m_buffer[m_count].simulation_mode = KRB_MODE_BCA;
		} else if(strcmp(tp, "DEL") == 0) {
			m_buffer[m_count].simulation_mode = KRB_MODE_DEL;
		} else{
            m_buffer[m_count].simulation_mode = KRB_MODE_KMC;
        }

        tp = strtok(nullptr, DELIMITER);
        m_buffer[m_count].atomic_element = GetAtomicNumber(tp);

        tp = strtok(nullptr, DELIMITER);
        m_buffer[m_count].id = atoll(tp);

        /*if (m_buffer[m_count].id > m_max_num_atoms){
            m_max_num_atoms = m_buffer[m_count].id;
        }*/

        tp = strtok(nullptr, DELIMITER);
        m_buffer[m_count].x = strtof(tp, nullptr);

        tp = strtok(nullptr, DELIMITER);
        m_buffer[m_count].y = strtof(tp, nullptr);

        tp = strtok(nullptr, DELIMITER);
        m_buffer[m_count].z = strtof(tp, nullptr);
        
        /*
        if (m_buffer[m_count].x < m_min_space.x){
            m_min_space.x = m_buffer[m_count].x;
        }
        if (m_buffer[m_count].y < m_min_space.y){
            m_min_space.y = m_buffer[m_count].y;
        }
        if (m_buffer[m_count].z < m_min_space.z){
            m_min_space.z = m_buffer[m_count].z;
        }
        if (m_buffer[m_count].x > m_max_space.x){
            m_max_space.x = m_buffer[m_count].x;
        }
        if (m_buffer[m_count].y > m_max_space.y){
            m_max_space.y = m_buffer[m_count].y;
        }
        if (m_buffer[m_count].z > m_max_space.z){
            m_max_space.z = m_buffer[m_count].z;
        }
        */


        m_count++;
    }

    fclose(fp);


	if (m_z_reverse) {
		for (size_t i = 0; i < m_count; ++i) {
			m_buffer[i].z = -m_buffer[i].z;
		}

		m_krb_info.box_org.z -= m_krb_info.box_axis.m33;
	}


	m_krb_info.num_step = 1;
	if (m_count > 0) {
		m_max_time_step = m_buffer[m_count-1].step;
	}
    SetFrameNum(0);

    return 0;
}

