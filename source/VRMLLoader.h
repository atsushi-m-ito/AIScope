//=====================================
// MQOファイル管理クラス
//
//
// MQOを読み込み＆表示する
//
// 
//=====================================
#pragma once
#ifndef VRMLLoader_H
#define VRMLLoader_H



#include <stdio.h>

#include <tchar.h>
#include <string.h>
#include <math.h>
#include <vector>
#include <map>
#include <string>


#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "material_info.h"

#include "IPolyRepository.h"

class ReadWord;



class VRMLLoader : public IPolyRepository{
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


    struct VRML_GROUP{
        int parent_id;  //親がいない場合は-1//

    };

    std::vector<VRML_GROUP> m_groups;


    struct VRML_SHAPE{
        float diffuse_color[3];
        UINT coord_offset;
        UINT normal_offset;
        UINT coord_index_offset;
        UINT normal_index_offset;
        UINT coord_index_count;

        mat44d matrix;
    };
    std::vector<VRML_SHAPE> m_vrml_shape;

    std::map<std::string, UINT> m_vrml_offset_list;   //coordおよびnormal配列のオフセットをDEFで毎に定義//
    std::vector<float> m_vrml_coord;
    std::vector<float> m_vrml_normal;
    std::vector<UINT> m_vrml_coord_index;
    std::vector<UINT> m_vrml_normal_index;


    struct VRML_INLINE{
        std::string filename;
        mat44d matrix;
    };
    std::vector<VRML_INLINE> m_vrml_inline_file;


	MATERIAL_COMPOSIT* m_material_composit;
	int m_material_composit_count;


    int mLoad(const WCHAR* filename);
    int mReadGroup(ReadWord& reader, const int parent_group_id, const char* filename, mat44d& parent_matrix);
    int mReadChildren(ReadWord& reader, const int parent_group_id, const char* filename, mat44d& parent_matrix);
    int mReadShape(ReadWord& reader, const char* filename, mat44d& parent_matrix);
    int mReadSkipAny(ReadWord& reader, const char end_tag);

    int mReadAppearance(ReadWord& reader, VRML_SHAPE* shape);
    int mReadIndexedFaceSet(ReadWord& reader, VRML_SHAPE* shape, const char* filename);


    //TCHAR* mReadText(FILE* fp, int encode_type);
    int mReadMaterial(FILE* fp);
    int mReadVertex(FILE* fp);
    template<typename T> int mReadFace(FILE* fp);
    


public:

    VRMLLoader(const TCHAR* filename);
    virtual ~VRMLLoader();


    UINT GetVertexBuffer(LPVOID* ppBuffer, UINT* pStride);

    UINT GetIndexBuffer(LPVOID* ppBuffer, UINT* pStride);



    int GetMaterialList(MATERIAL_INFO** pmaterials);
	int GetMaterialComposit(const MATERIAL_COMPOSIT** pmaterial_composit);


    void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg);

#if 1
    int SetFrameNum(int time_frame){


        if ( time_frame <= 0){
            m_time_frame = 0;

        } else if (time_frame >= m_material_composit_count) {
            m_time_frame = m_material_composit_count;
        } else{
            m_time_frame = time_frame;
        }

        return m_time_frame;

    };
#else

	int SetFrameNum(int time_frame) {

		if (0 >= time_frame) {
			m_time_frame = 0;

		} else if (time_frame >= m_material_count) {
			m_time_frame = m_material_count;
		} else {
			m_time_frame = time_frame;
		}

		return m_time_frame;

	};
#endif

    //頂点データのy,z転置//
    void ExchangeYZ(){
		
		for (unsigned int i = 0; i < m_vertex_count; i++) {
			{
				float temp = m_vertex[i].position_y;
				m_vertex[i].position_y = -m_vertex[i].position_z;
				m_vertex[i].position_z = temp;
			}
			{
				float temp = m_vertex[i].normal_y;
				m_vertex[i].normal_y = -m_vertex[i].normal_z;
				m_vertex[i].normal_z = temp;
			}
		}
    };
};


#endif


