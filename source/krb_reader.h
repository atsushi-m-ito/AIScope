
#ifndef krb_reader_h
#define krb_reader_h

#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <algorithm>
#include "vec3.h"
#include "mat33.h"
#include "IDataAtoms.h"

struct KRB_PARTICLE{
    int atomic_element;
    float x;
    float y;
    float z;
    int64_t id;
	int64_t step;
    int simulation_mode;
};

struct KRB_INFO{
	int64_t num_step;
    KRB_PARTICLE* particles;
    //size_t count;
    mat33f box_axis;
    vec3f box_org;
};



class KRB_Reader : public IDataContainer
{
private:


    size_t m_buffer_size;
    size_t m_count;
	int64_t m_max_time_step;

    //int m_max_num_atoms;

    KRB_PARTICLE* m_buffer;
    KRB_INFO m_krb_info;


public:


    KRB_Reader(int z_reverse) : m_z_reverse(z_reverse), m_buffer(NULL), m_buffer_size(0), m_count(0), m_max_time_step(0){};
    ~KRB_Reader(){
        if (m_buffer){
            delete[] m_buffer;
        }
    };

    ///////////////////////////////////////////////////////
    //ファイルをオープンする.
    //
    //
    //	正しくオープンできたときは 0 を返す.
    //	既に別のファイルをオープン済みの場合は -2 を返す.
    //	指定したファイルが見つからないときは -1 を返す.
    //	読み込んだデータが壊れているとき -3を返す.
    //	
    /////////////////////////////////////////////////////////////
    int Open(const char* filepath);

    int SetFrameNum(const int timeframe){

		int64_t time_frame64 = (timeframe < 0 ? 0 : (int64_t)timeframe > m_max_time_step ? m_max_time_step : (int64_t)timeframe);
		

		int64_t index = std::max<int64_t>(m_krb_info.num_step - 1, 0);
		if (m_buffer[index].step > time_frame64) {
			for (int64_t i = index - 1; i >= 0; i--) {
				if (m_buffer[i].step <= time_frame64) {
					m_krb_info.num_step = i + 1;
					return (int)time_frame64;
				}
			}
			m_krb_info.num_step = 0;
			return (int)time_frame64;
		} else {
			for (int64_t i = index; i < m_count; i++) {
				if (m_buffer[i].step > time_frame64) {
					m_krb_info.num_step = i;
					return (int)time_frame64;
				}
			}
			m_krb_info.num_step = m_count;
			return (int)time_frame64;
		}

    };

	int GetFrameNum() {
		
		return m_buffer[m_krb_info.num_step - 1].step;
	};

    KRB_INFO* GetDataPointer(){
        //m_krb_info.count = m_count;
        m_krb_info.particles = m_buffer;
        /*
        m_krb_info.box_axis.Set((double)(m_max_space.x - m_min_space.x), 0.0f, 0.0f,
            0.0f, (double)(m_max_space.y - m_min_space.y), 0.0f,
            0.0f, 0.0f, (double)(m_max_space.z - m_min_space.z));

        m_krb_info.box_org.Set((double)(m_min_space.x),
            (double)(m_min_space.y),
            (double)(m_min_space.z));
            */
        return &m_krb_info;
    };

    void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg){     

        *boxaxis = m_krb_info.box_axis;
        *boxorg = m_krb_info.box_org;

    }

private:
	int m_z_reverse;

};


const int KRB_MODE_KMC = 1;
const int KRB_MODE_BCA = 2;
const int KRB_MODE_DEL = 3;



#endif	//!krb_reader_h
