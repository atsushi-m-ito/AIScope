//=====================================
// MQOファイル管理クラス
//
//
// MQOを読み込み＆表示する
//
// 
//=====================================
#pragma once
#ifndef MQOLoader_H
#define MQOLoader_H



#include <stdio.h>

#include <tchar.h>
#include <string.h>
#include <math.h>


#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "material_info.h"

#include "IPolyRepository.h"





class MQOLoader : public IPolyRepository{
private:



    struct VertexData {
        float position_x;
        float position_y;
        float position_z;
        float normal_x;
        float normal_y;
        float normal_z;
        float u;
        float v;
    };
    VertexData* m_vertex;
    UINT m_vertex_count;

    BYTE* m_indexes;
    UINT m_index_count;
    int m_index_size;

    UINT m_texture_count;
    TCHAR** m_texture_names;

    WCHAR* m_dirname;

    MATERIAL_INFO* m_materials;
    int m_material_count;

    //boneの表示
    vec3f *m_bone_pos;
    //void CreateBone();

    int m_time_frame;


    struct MQO_FACE{//面情報の再構築用の構造体//
        int material_id;
        int indexes[3];

        inline bool operator<(const MQO_FACE& right) {
            return material_id < right.material_id;
        };
    };


    int mLoad(const WCHAR* filename);
    //TCHAR* mReadText(FILE* fp, int encode_type);
    int mReadMaterial(FILE* fp);
    int mReadObject(FILE* fp);
    int mReadVertex(FILE* fp);
    template<typename T> int mReadFace(FILE* fp);
    int mReadSkipAny(FILE* fp);



public:

    MQOLoader(const TCHAR* filename);
    virtual ~MQOLoader();


    UINT GetVertexBuffer(LPVOID* ppBuffer, UINT* pStride);

    UINT GetIndexBuffer(LPVOID* ppBuffer, UINT* pStride);



    int GetMaterialList(MATERIAL_INFO** pmaterials);


    void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg);

    int SetFrameNum(int time_frame){

        if (0 >= time_frame){
            m_time_frame = 0;

        } else if (time_frame >= m_material_count){
            m_time_frame = m_material_count;
        } else{
            m_time_frame = time_frame;
        }

        return m_time_frame;

    };

    void ExchangeYZ(){
        for (unsigned int i = 0; i < m_vertex_count; i++){
            {
                float temp = m_vertex[i].position_y;
                m_vertex[i].position_y = m_vertex[i].position_z;
                m_vertex[i].position_z = temp;
            }
            {
                float temp = m_vertex[i].normal_y;
                m_vertex[i].normal_y = m_vertex[i].normal_z;
                m_vertex[i].normal_z = temp;
            }
        }
    };
};


#endif


