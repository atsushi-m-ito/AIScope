//=====================================
// PMXファイル管理クラス
//
//
// PMXを読み込み＆表示する
//
// 
//=====================================
#pragma once
#ifndef PMXLoader_h__
#define PMXLoader_h__


//#define RENDER_DX11


#include <stdio.h>

#include <tchar.h>
#include <string.h>
#include <math.h>


#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "material_info.h"

#include "IPolyRepository.h"





class PMXLoader : public IPolyRepository{
private:




    struct PMX_HEADER{
        char magic[4]; // "Pmd"
        float version; // 00 00 80 3F == 1.00
        char length;
        char format_size[8];
        TCHAR* model_name;
        TCHAR* model_name_eng;
        TCHAR* comment;
        TCHAR* comment_eng;

    public:
        ~PMX_HEADER(){
            delete[] model_name;
            delete[] model_name_eng;
            delete[] comment;
            delete[] comment_eng;

        }
    };



    struct PMX_VERTEX_SUPPLEMENT {
        int weight_type;
        union{
            struct BDEF1{               //4 byte//
                int bone_index[1];
            } bdef1;
            struct BDEF2{               //12 byte//
                int bone_index[2];
                float bone_weight[1];
            } bdef2;
            struct BDEF4{               //32 byte//
                int bone_index[4];
                float bone_weight[4];
            } bdef4;
            struct SDEF{               //48 byte//
                int bone_index[2];
                float bone_weight[1];
                float C[3];
                float R0[3];
                float R1[3];
            } sdef;
        };
        float edge_flag; // 0:通常、1:エッジ無効 // エッジ(輪郭)が有効の場合
    };
    PMX_VERTEX_SUPPLEMENT* m_vec_supplement;



    struct PMX_MATERIAL{	// 材質(マテリアル)構造体(70byte)
        TCHAR* name;        // 材質名//
        TCHAR* name_eng;    //  材質名_eng//

        float diffuse[4]; // dr, dg, db da// 減衰色
        float specular[3]; // sr, sg, sb // 光沢色
        float specular_coef;
        float ambient[3]; // mr, mg, mb // 環境色(ambient)
        BYTE bitFlag;
        float edge_color[4];
        float edge_size;
        int texture_index; //   テクスチャインデックス
        int sphere_index; //   スフィアテクスチャインデックス
        BYTE sphere_mode;
        BYTE toon_mode;
        int toon_index;

        TCHAR* comment;
        int face_index_count;

    };

    PMX_MATERIAL* m_pmd_material;

    int m_material_count; // 材質(マテリアル)数


    WORD m_bone_count; // ボーン数(2byte)

    struct PMX_BONE{		// ボーン構造体(39byte)
        char bone_name[20]; // ボーン名
        WORD parent_bone_index; // 親ボーン番号(ない場合は0xFFFF)
        WORD tail_pos_bone_index; // tail位置のボーン番号(チェーン末端の場合は0xFFFF) // 親：子は1：多なので、主に位置決め用
        BYTE bone_type; // ボーンの種類
        WORD ik_parent_bone_index; // IKボーン番号(影響IKボーン。ない場合は0)
        vec3f bone_head_pos; // x, y, z // ボーンのヘッドの位置
    };
    PMX_BONE *m_pmd_bone;
    /*・ボーンの種類
    0:回転 1:回転と移動 2:IK 3:不明 4:IK影響下 5:回転影響下 6:IK接続先 7:非表示

    ・ボーンの種類 (MMD 4.0～)
    8:捻り 9:回転運動

    補足：
    1モデルあたりのボーン数の実質的な限界が変わったようです。(MMD6.x～)
    実験結果：
    MMD6.02、6.08
    約500本x1(テストに使ったデータは513本)→落ちない
    約2000本x1(テストに使ったデータは2599本)→落ちる
    約500本x10→落ちない
    MMD5.22
    約500本x1→落ちない
    約2000本x1→落ちない
    */


    WORD m_ik_data_count; // IKデータ数

    struct PMX_IK{		// IKデータ構造体(13byte)
        WORD ik_bone_index; // IKボーン番号
        WORD ik_target_bone_index; // IKターゲットボーン番号 // IKボーンが最初に接続するボーン
        BYTE ik_chain_length; // IKチェーンの長さ(子の数)
        WORD iterations; // 再帰演算回数 // IK値1
        float control_weight; // IKの影響度 // IK値2
        //WORD ik_child_bone_index[ik_chain_length]; // IK影響下のボーン番号
        WORD* ik_child_bone_index;
    };


    WORD m_skin_count; // 表情数

    struct PMX_VERT_DATA{	//
        DWORD skin_vert_index;
        vec3f skin_vert_pos; // x, y, z 
    };
    //baseの場合(最初のPMD_SKINの頂点は全てbase)
    // skin_vert_index: 表情用の頂点の番号(頂点リストにある番号)
    // skin_vert_pos: 表情用の頂点の座標(頂点自体の座標)
    //base以外
    // skin_vert_index: 表情用の頂点の番号(baseの番号。skin_vert_index)
    // skin_vert_pos: 表情用の頂点の座標オフセット値(baseに対するオフセット)

    struct PMX_SKIN{	// 表情構造体
        char skin_name[20]; //　表情名
        DWORD skin_vert_count; // 表情用の頂点数
        BYTE skin_type; // 表情の種類 // 0：base、1：まゆ、2：目、3：リップ、4：その他
        PMX_VERT_DATA* skin_vert_data; // 表情用の頂点のデータ(16Bytes/vert)
    };

    BYTE m_skin_disp_count; // 表情枠に表示する表情数
    BYTE m_bone_disp_name_count; // ボーン枠用の枠名数
    /*
    補足：
    PMDeditorを使う場合は、枠名を0x0A00で終わらせる必要があります。(0x00のみだと表示されません)→0.0.4.2cで確認。
    MMDでは文字列終端は0x00のみでも表示可能です。→6.08で確認。(付属モデルは0x0A00で終端しているようです)
    */

    DWORD m_bone_disp_count; // ボーン枠に表示するボーン数 (枠0(センター)を除く、すべてのボーン枠の合計)

    struct PMX_BONE_DISP{
        WORD bone_index; // 枠用ボーン番号
        BYTE bone_disp_frame_index; // 表示枠番号
    };
    /*
    補足：
    1つのボーン枠に表示できるボーン数は254個です。
    1つのボーン枠に登録したボーン数と表示される内容は、こんな感じ。
    254：254本表示される
    255、256：枠内の表示が消える
    257：枠内の最初のボーン1本だけ表示
    258～：256を引いた数ずつ表示される
    */

    //拡張機能===========================
    //英語名対応
    BYTE m_english_name_compatibility; // 英名対応(01:英名対応あり)
    char m_model_name_eg[20]; // モデル名(英語)
    char m_comment_eg[256]; // コメント(英語)




    //GLのVBOに乗りやすいデータ構造=====================


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


    int m_render_mode;
    TCHAR* m_dirname;


    int mLoad(const TCHAR* filename);
    TCHAR* mReadText(FILE* fp, int encode_type);

    


    //boneの表示
    vec3f *m_bone_pos;
    //void CreateBone();

    int m_time_frame;

public:

    PMXLoader(const TCHAR* filename);
    virtual ~PMXLoader();


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


#endif    // PMXLoader_h__


