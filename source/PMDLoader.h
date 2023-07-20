//=====================================
// PMDファイル管理クラス
//
//
// PMDを読み込み＆表示する
//
// 
//=====================================
#pragma once
#ifndef PMDLoader_h__
#define PMDLoader_h__


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




	

class PMDLoader : public IPolyRepository{
private:

	
	

	struct PMD_HEADER{
		char magic[3]; // "Pmd"
		float version; // 00 00 80 3F == 1.00
		char model_name[20];
		char comment[256];
	};



	struct PMD_VERTEX_SUPPLEMENT {
		WORD bone_num[2]; // ボーン番号1、番号2 // モデル変形(頂点移動)時に影響
		BYTE bone_weight; // ボーン1に与える影響度 // min:0 max:100 // ボーン2への影響度は、(100 - bone_weight)
		BYTE edge_flag; // 0:通常、1:エッジ無効 // エッジ(輪郭)が有効の場合
	};
	PMD_VERTEX_SUPPLEMENT* m_vec_supplement;



	struct PMD_MATERIAL{	// 材質(マテリアル)構造体(70byte)
		float diffuse_color[3]; // dr, dg, db // 減衰色
		float alpha;
		float specularity;
		float specular_color[3]; // sr, sg, sb // 光沢色
		float mirror_color[3]; // mr, mg, mb // 環境色(ambient)
		BYTE toon_index; // toon??.bmp // 0.bmp:0xFF, 1(01).bmp:0x00 ・・・ 10.bmp:0x09
		BYTE edge_flag; // 輪郭、影
		DWORD face_index_count; // 面頂点数 // インデックスに変換する場合は、材質0から順に加算
		char texture_file_name[21]; // テクスチャファイル名 // 20バイトぎりぎりまで使える(終端の0x00は無くても動く)
		char sphere_file_name[21]; // テクスチャファイル名 // 20バイトぎりぎりまで使える(終端の0x00は無くても動く)
	};			//psh拡張子は中身はスフィアマッピングするのか？

	PMD_MATERIAL* m_pmd_material;

	int m_material_count; // 材質(マテリアル)数


	WORD m_bone_count; // ボーン数(2byte)
	
	struct PMD_BONE{		// ボーン構造体(39byte)
		char bone_name[20]; // ボーン名
		WORD parent_bone_index; // 親ボーン番号(ない場合は0xFFFF)
		WORD tail_pos_bone_index; // tail位置のボーン番号(チェーン末端の場合は0xFFFF) // 親：子は1：多なので、主に位置決め用
		BYTE bone_type; // ボーンの種類
		WORD ik_parent_bone_index; // IKボーン番号(影響IKボーン。ない場合は0)
		vec3f bone_head_pos; // x, y, z // ボーンのヘッドの位置
	};
	PMD_BONE *m_pmd_bone;
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

	struct PMD_IK{		// IKデータ構造体(13byte)
		WORD ik_bone_index; // IKボーン番号
		WORD ik_target_bone_index; // IKターゲットボーン番号 // IKボーンが最初に接続するボーン
		BYTE ik_chain_length; // IKチェーンの長さ(子の数)
		WORD iterations; // 再帰演算回数 // IK値1
		float control_weight; // IKの影響度 // IK値2
		//WORD ik_child_bone_index[ik_chain_length]; // IK影響下のボーン番号
		WORD* ik_child_bone_index;
	};

	
	WORD m_skin_count; // 表情数

	struct PMD_VERT_DATA{	//
		DWORD skin_vert_index; 
		vec3f skin_vert_pos; // x, y, z 
	};
	//baseの場合(最初のPMD_SKINの頂点は全てbase)
	// skin_vert_index: 表情用の頂点の番号(頂点リストにある番号)
	// skin_vert_pos: 表情用の頂点の座標(頂点自体の座標)
	//base以外
	// skin_vert_index: 表情用の頂点の番号(baseの番号。skin_vert_index)
	// skin_vert_pos: 表情用の頂点の座標オフセット値(baseに対するオフセット)

	struct PMD_SKIN{	// 表情構造体
		char skin_name[20]; //　表情名
		DWORD skin_vert_count; // 表情用の頂点数
		BYTE skin_type; // 表情の種類 // 0：base、1：まゆ、2：目、3：リップ、4：その他
		PMD_VERT_DATA* skin_vert_data; // 表情用の頂点のデータ(16Bytes/vert)
	};

	BYTE m_skin_disp_count; // 表情枠に表示する表情数
	BYTE m_bone_disp_name_count; // ボーン枠用の枠名数
		/*
		補足：
		PMDeditorを使う場合は、枠名を0x0A00で終わらせる必要があります。(0x00のみだと表示されません)→0.0.4.2cで確認。
		MMDでは文字列終端は0x00のみでも表示可能です。→6.08で確認。(付属モデルは0x0A00で終端しているようです)
		*/

	DWORD m_bone_disp_count; // ボーン枠に表示するボーン数 (枠0(センター)を除く、すべてのボーン枠の合計)

	struct PND_BONE_DISP{
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

	UINT* m_index4b;
	USHORT* m_index2b;
	UINT m_index_count;
	int m_index_size;


	int m_render_mode;
	TCHAR* m_dirname;


	int Load(const TCHAR* filename);
    	

	//boneの表示
	vec3f *m_bone_pos; 
	void CreateBone();


	UINT m_indexCount;
    int m_time_frame;

public:
	
	PMDLoader(const TCHAR* filename);
	virtual ~PMDLoader();
	

	UINT GetVertexBuffer(LPVOID* ppBuffer, UINT* pStride);

	UINT GetIndexBuffer(LPVOID* ppBuffer, UINT* pStride);

	//int GetPMDMaterialList(PMD_MATERIAL** material);

	int GetMaterialList(MATERIAL_INFO** pmaterials);


    void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg);
    
    //暫定仕様//
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


#endif    // __PMDLoader_h__


