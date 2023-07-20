#pragma once

#pragma warning(disable : 4244)
#pragma warning(disable : 4996)

#include <stdlib.h>
#include <stdio.h>
#include <tchar.h>
#include <float.h>
#include "mychardump.h"
#include "vmec_reader.h"
#include "vec3.h"
#include "JobParam.h"
#include "IDataAtoms.h"

class VMEC_Repository : public IDataContainer
{
private:

private:
    int m_num_grids_x;
    int m_num_grids_y;
    int m_num_grids_z;
    int m_num_grids_a;
    char* m_binary_file_path;
    const TCHAR* DELIMITER;

    //vec3f* m_coordinates;
    float* m_data;
    size_t m_num_data;
    vec3f m_max_space;
    vec3f m_min_space;

    float m_max_value[3];
    float m_min_value[3];
    float m_alpha;

    int m_start_frame_number;
    int m_frame_index;
    int m_frame_count;

    char* m_file_path;
    JobParam m_param;

    int m_num_species;

public:
    VMEC_Repository(const TCHAR* filepath) :
        DELIMITER(_T(" \t\r\n")),
        m_binary_file_path(NULL),
        m_data(NULL),
        m_start_frame_number(0),
        m_file_path(MyCharDump<char, TCHAR>(filepath)),
        m_param(m_file_path, "*.Begin", "*.End"),
        m_num_species(1),
        m_alpha(0.01f)
    {



        //グリッド数など前提情報の読み込み//
        {
            int iarray[4];
            m_param.GetIntArray("grids", iarray, 4);
            m_num_grids_x = iarray[0];
            m_num_grids_y = iarray[1];
            m_num_grids_z = iarray[2];
            m_num_grids_a = iarray[3];
        }
        

        //ディレクトリ名の取得//
        {
            m_binary_file_path = strdup(m_file_path);
            char* p = strrchr(m_binary_file_path, '\\');
            if (p){
                *(p + 1) = '\0';
            } else{
                *m_binary_file_path = '\0';
            }
        }

        //データ数の取得//
        m_num_species = m_param.GetInt("DATA.Species");
        if (m_num_species < 1 || m_num_species > 3){
            m_num_species = 1;
        }

        //座標ファイルの読み込み//
        {

            char open_path[1024];

            {
                strcpy(open_path, m_binary_file_path);
                strcat(open_path, m_param.GetString("Coordinate.X"));

                VMEC_Reader vmec_xyz(4, 1);
                int res = vmec_xyz.Open(open_path);
                if (res != 0){
                    return;
                }

                size_t buffer_size = vmec_xyz.GetSize();

                if (buffer_size != m_num_grids_x * m_num_grids_y * m_num_grids_z * m_num_grids_a){
                    printf("error: file size is not valid.\n");
                    return;
                }


                m_num_data = buffer_size;

                m_data = new float[(m_num_species + 3) * m_num_data];
                vmec_xyz.Read(m_data, 0, m_num_species + 3);
            }

            //Y座標の読み込み//
            {
                strcpy(open_path, m_binary_file_path);
                strcat(open_path, m_param.GetString("Coordinate.Y"));

                VMEC_Reader vmec_xyz(4, 1);
                int res = vmec_xyz.Open(open_path);
                if (res != 0){
                    return;
                }
                vmec_xyz.Read(m_data, 1, m_num_species + 3);
            }

            //Z座標の読み込み//
            {
                strcpy(open_path, m_binary_file_path);
                strcat(open_path, m_param.GetString("Coordinate.Z"));

                VMEC_Reader vmec_xyz(4, 1);
                int res = vmec_xyz.Open(open_path);
                if (res != 0){
                    return;
                }
                vmec_xyz.Read(m_data, 2, m_num_species + 3);
            }
        }

        //空間範囲の取得//
        mGetMaxSpace(m_data, m_num_data, & m_max_space, &m_min_space);

        //最大フレーム数の取得//
        m_frame_count = m_param.GetBlockNumLines("DATA.1");
        if (m_frame_count == 0){
            m_frame_count = m_param.GetBlockNumLines("DATA.2");
        }
        if (m_frame_count == 0){
            m_frame_count = m_param.GetBlockNumLines("DATA.3");
        }

        //場の値の取得//
        int res = mReadFieldFile(m_start_frame_number);
        if (res == 0){

            double range[2];

            //場の値のレンジの取得//
            if (m_param.Find("Color.1.Range")){
                m_param.GetDoubleArray("Color.1.Range", range, 2);
                m_max_value[0] = range[1];
                m_min_value[0] = range[0];
            } else{
                //場の値のレンジの自動計算//
                mGetRange(m_data, m_num_data, &(m_max_value[0]), &(m_min_value[0]));
            }

            if (m_num_species > 1){

                //場の値のレンジの取得//
                if (m_param.Find("Color.2.Range")){
                    m_param.GetDoubleArray("Color.2.Range", range, 2);
                    m_max_value[1] = range[1];
                    m_min_value[1] = range[0];
                } else{
                    //場の値のレンジの自動計算//
                    mGetRange(m_data, m_num_data, &(m_max_value[1]), &(m_min_value[1]));
                }
            }

            if (m_num_species > 2){

                //場の値のレンジの取得//
                if (m_param.Find("Color.3.Range")){
                    m_param.GetDoubleArray("Color.3.Range", range, 2);
                    m_max_value[2] = range[1];
                    m_min_value[2] = range[0];
                } else{
                    //場の値のレンジの自動計算//
                    mGetRange(m_data, m_num_data, &(m_max_value[2]), &(m_min_value[2]));
                }
            }

            if (m_param.Find("Color.Alpha")){
                m_alpha = m_param.GetDouble("Color.Alpha");
            }

            //フレーム数のセット(暫定で1フレームのみ)//
            m_frame_index = 0;
            
        } else{
            m_frame_index = 0; 
            m_frame_count = 0;
        }
    };

    ~VMEC_Repository(){

        if (m_binary_file_path){
            delete[] m_binary_file_path;
        }
        if (m_data){
            delete[] m_data;
        }
        delete[] m_file_path;
        
    };

    size_t GetNumParticles(){
        return m_num_grids_x * m_num_grids_y *m_num_grids_z * m_num_grids_a;
    };


    float* GetFieldPointer(){
        return m_data;
    };

    int GetStride(){
        return (m_num_species + 3) * sizeof(float);
    };

    /*
        この範囲内であれば描画対象となる
    */
    void GetEffectiveRange(float* range_max, float* range_min, float* alpha){
        
        for (int i = 0; i < m_num_species; i++){
            range_max[i] = m_max_value[i];
            range_min[i] = m_min_value[i];
        }

        *alpha = m_alpha;
        /*
        *range_max = 86.0f;
        *range_min = 86.0f - (86.0f - (-80.0f)) * 0.6f;
        */
    }


    void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg){

        boxaxis->Set((double)(m_max_space.x - m_min_space.x), 0.0, 0.0,
            0.0, (double)(m_max_space.y - m_min_space.y), 0.0,
            0.0, 0.0, (double)(m_max_space.z - m_min_space.z));
    
        
        boxorg->Set((double)(m_min_space.x), 
                    (double)(m_min_space.y), 
                    (double)(m_min_space.z));


    }


    int SetFrameNum(int timeframe){

        if (timeframe < 0){ timeframe = 0; }
        if (m_frame_count <= timeframe){
            timeframe = m_frame_count - 1;
        }

        if (timeframe != m_frame_index){
            m_frame_index = timeframe;
         
            //場の値の取得//
            int res = mReadFieldFile(timeframe);
            

        }
        
        return m_frame_index;

    }

private:
    void mGetMaxSpace(const float* coordinates, const size_t count, vec3f* p_max_space, vec3f* p_min_space ){

        const size_t stride = m_num_species + 3;

        vec3f max_space = { -FLT_MAX, -FLT_MAX, -FLT_MAX };
        vec3f min_space = { FLT_MAX, FLT_MAX, FLT_MAX };
        
        for (size_t i = 0; i < count; i++){
            const double x = coordinates[i*stride];
            if (max_space.x < x){
                max_space.x = x;
            }

            if (min_space.x > x){
                min_space.x = x;
            }

            const double y = coordinates[i*stride + 1];
            if (max_space.y < y){
                max_space.y = y;
            }

            if (min_space.y > y){
                min_space.y = y;
            }

            const double z = coordinates[i*stride + 2];
            if (max_space.z < z){
                max_space.z = z;
            }

            if (min_space.z > z){
                min_space.z = z;
            }
        }
        
        *p_max_space = max_space;
        *p_min_space = min_space;
        

    };

    void mGetRange(const float* values, const size_t count, float* p_max_range, float* p_min_range){

        const size_t stride = m_num_species + 3;

        float max_range = { -FLT_MAX};
        float min_range = { FLT_MAX };

        for (size_t i = 0; i < count; i++){

            for (int k = 3; k < m_num_species + 3; k++){
                double val = values[i* stride + k];
                if (max_range < val){
                    max_range = val;
                }

                if (min_range > val){
                    min_range = val;
                }
            }
        }

        *p_max_range = max_range;
        *p_min_range = min_range;
        

    };

    int mReadFieldFile(int number){


        char filepath[1024];

        //第一系列のデータ読み込み//
        {
            strcpy(filepath, m_binary_file_path);
            strcat(filepath, m_param.GetBlockString("DATA.1", number));

            VMEC_Reader vmec(4, 1);
            int res = vmec.Open(filepath);
            if (res != 0){
                return res;
            }
            size_t buffer_size = vmec.GetSize();

            if (buffer_size != m_num_data){
                printf("error: file size is not valid.\n");
                return -2;
            }

            vmec.Read(m_data, 3, m_num_species + 3);

        }

        //第2系列のデータ読み込み//
        if (m_num_species >= 2){
            strcpy(filepath, m_binary_file_path);
            strcat(filepath, m_param.GetBlockString("DATA.2", number));

            VMEC_Reader vmec(4, 1);
            int res = vmec.Open(filepath);
            if (res != 0){
                return res;
            }
            size_t buffer_size = vmec.GetSize();

            if (buffer_size != m_num_data){
                printf("error: file size is not valid.\n");
                return -2;
            }

            vmec.Read(m_data, 4, m_num_species + 3);

        }


        //第2系列のデータ読み込み//
        if (m_num_species >= 3){
            strcpy(filepath, m_binary_file_path);
            strcat(filepath, m_param.GetBlockString("DATA.3", number));

            VMEC_Reader vmec(4, 1);
            int res = vmec.Open(filepath);
            if (res != 0){
                return res;
            }
            size_t buffer_size = vmec.GetSize();

            if (buffer_size != m_num_data){
                printf("error: file size is not valid.\n");
                return -2;
            }

            vmec.Read(m_data, 5, m_num_species + 3);

        }


        

        return 0;
    };
};

