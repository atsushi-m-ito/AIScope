
#define _CRT_SECURE_NO_DEPRECATE

#include "PMXLoader.h"
#include "debugprint.h"


PMXLoader::PMXLoader(const TCHAR* filename) :
        m_vertex(NULL),
        m_vertex_count(0),
        m_indexes(NULL),
        m_index_count(0),
        m_index_size(0),
        m_dirname(NULL),
        m_time_frame(0)
{


    //以後はデータ部分の処理

    mLoad(filename);
    

    //LoadTexture();	//テクスチャの読み込み.

}


PMXLoader::~PMXLoader(){

    delete[] m_indexes;

    delete[] m_vertex;
    delete[] m_vec_supplement;

}




int PMXLoader::mLoad(const TCHAR* filename){

    

    //dir名の取得
    int dirlen = 0;
    TCHAR* tp = (TCHAR*)_tcsrchr(filename, _T('\\'));
    if (tp){ dirlen = (int)(tp - filename) + 1; }
    m_dirname = new TCHAR[dirlen + 1];
    _tcsncpy(m_dirname, filename, dirlen);
    m_dirname[dirlen] = _T('\0');



    FILE* fp = _tfopen(filename, _T("rb"));

    //ヘッダーの読み込み=========================
    PMX_HEADER pmdh;
    fread(&(pmdh.magic), 4, 1, fp);
    fread(&(pmdh.version), 4, 1, fp);
    fread(&(pmdh.length), 1, 1, fp);
    fread(&(pmdh.format_size), 8, 1, fp);

    TRACE(_T("PMD_ver.=%f\n"), pmdh.version);


    

    //モデル名の読み込み=========================


    int encode_type = pmdh.format_size[0];   //0:UTF16 1:UTF8//

    pmdh.model_name = mReadText(fp, encode_type);
    pmdh.model_name_eng = mReadText(fp, encode_type);
    pmdh.comment = mReadText(fp, encode_type);
    pmdh.comment_eng = mReadText(fp, encode_type);


    //頂点データの読み込み==========================

    const int bone_index_size = (int)pmdh.format_size[5];
    const int bdef1_size = bone_index_size;
    const int bdef2_size = bone_index_size * 2 + sizeof(float);
    const int bdef4_size = bone_index_size * 4 + sizeof(float) * 4;
    const int sdef_size  = bone_index_size * 2 + sizeof(float) * 10;

    m_vertex_count = 0;
    fread(&m_vertex_count, 4, 1, fp);		//頂点数

    TRACE(_T("頂点数 = %d\n"), (int)m_vertex_count);

    //追加UV数
    int num_extra_uv = pmdh.format_size[1]; //追加UV数 	| 0～4 //
    float* extra_uv = new float[num_extra_uv * 4];

    m_vertex = new VertexData[m_vertex_count];	//頂点はposition, normal, texture_uvがセットになって一要素
    //m_vec_supplement = new PMX_VERTEX_SUPPLEMENT[m_vertex_count];	//頂点はposition, normal, texture_uvがセットになって一要素
    PMX_VERTEX_SUPPLEMENT dummy_supplement;	//頂点はposition, normal, texture_uvがセットになって一要素

    for (int i = 0; i < (int)m_vertex_count; i++){
        fread(&(m_vertex[i]), 32, 1, fp);
        if (num_extra_uv > 0){
            fread(extra_uv, num_extra_uv * 4, 1, fp);
        }

        //bone-weightの読み込み//
        dummy_supplement.weight_type = 0;
        fread(&(dummy_supplement.weight_type), 1, 1, fp);
        
        switch (dummy_supplement.weight_type){
        case 0: 
            fread(&(dummy_supplement.bdef1), bdef1_size, 1, fp);
            break;
        case 1: 
            fread(&(dummy_supplement.bdef2), bdef2_size, 1, fp);
            break;
        case 2: 
            fread(&(dummy_supplement.bdef4), bdef4_size, 1, fp);
            break;
        case 3: 
            fread(&(dummy_supplement.sdef), sdef_size, 1, fp);
            break;
        }

        fread(&(dummy_supplement.edge_flag), 4, 1, fp);



    }
    delete[] extra_uv;







    TRACE(_T("最終頂点 : %f, %f, %f\n"), m_vertex[m_vertex_count - 1].position_x, m_vertex[m_vertex_count - 1].position_y, m_vertex[m_vertex_count - 1].position_z);

    //面(インデックス)データ============================
    m_index_size = pmdh.format_size[2];   //1,2,4//

    fread(&m_index_count, 4, 1, fp);
    m_indexes = new BYTE[m_index_count*m_index_size]; // 頂点番号(3個/面)
    fread(m_indexes, m_index_count*m_index_size, 1, fp);
    TRACE(_T("size = %d\n"), m_index_count);


    //テクスチャー名=============================

    fread(&m_texture_count, 4, 1, fp);
    if (m_texture_count > 0){
        m_texture_names = new TCHAR*[m_texture_count];
        for (unsigned int i = 0; i < m_texture_count; i++){
            m_texture_names[i] = mReadText(fp, encode_type);
        }

    } else{
        m_texture_count = 0;
        m_texture_names = NULL;
    }
    
    TRACE(_T("texture_count = %d\n"), m_texture_count);



    //材質(マテリアル)データ=============================

    const int texture_index_size = pmdh.format_size[3];   //1,2,4//

    fread(&m_material_count, 4, 1, fp);

    m_pmd_material = new PMX_MATERIAL[m_material_count]; // 頂点番号(3個/面)
    for (int i = 0; i < m_material_count; i++){

        m_pmd_material[i].name = mReadText(fp, encode_type);
        m_pmd_material[i].name_eng = mReadText(fp, encode_type);

        fread(m_pmd_material[i].diffuse, 4 * 11, 1, fp);
        
        fread(&(m_pmd_material[i].bitFlag), 1, 1, fp);

        fread(m_pmd_material[i].edge_color, 16, 1, fp);
        fread(&(m_pmd_material[i].edge_size), 4, 1, fp);

        //負の場合を考慮して読み込みを場合分け//
        switch (texture_index_size){
        case 1:
        {
            char texture_index;
            fread(&texture_index, texture_index_size, 1, fp);
            m_pmd_material[i].texture_index = texture_index;
            break;
        }
        case 2:
        {
            short int texture_index;
            fread(&texture_index, texture_index_size, 1, fp);
            m_pmd_material[i].texture_index = texture_index;
            break;
        }
        case 4:
        {
            int texture_index;
            fread(&texture_index, texture_index_size, 1, fp);
            m_pmd_material[i].texture_index = texture_index;
            break;
        }
        }

        m_pmd_material[i].sphere_index = 0;
        fread(&(m_pmd_material[i].sphere_index), texture_index_size, 1, fp);

        fread(&(m_pmd_material[i].sphere_mode), 1, 1, fp);
        fread(&(m_pmd_material[i].toon_mode), 1, 1, fp);
        
        m_pmd_material[i].toon_index = 0;
        if (m_pmd_material[i].toon_mode == 0){
            fread(&(m_pmd_material[i].toon_index), texture_index_size, 1, fp);
        } else{
            fread(&(m_pmd_material[i].toon_index), 1, 1, fp);
        }


        m_pmd_material[i].comment = mReadText(fp, encode_type);
        fread(&(m_pmd_material[i].face_index_count), 4, 1, fp);
        
    }

#if 0
    //ボーンデータ=========================================
    fread(&m_bone_count, sizeof(WORD), 1, fp);

    TRACE(_T("ボーン数 = %d\n"), m_bone_count);

    m_pmd_bone = new PMD_BONE[m_bone_count]; // 頂点番号(3個/面)
    for (i = 0; i < (int)m_bone_count; i++){
        fread(m_pmd_bone + i, 20, 1, fp);
        fread(&(m_pmd_bone[i].parent_bone_index), sizeof(WORD), 1, fp);
        fread(&(m_pmd_bone[i].tail_pos_bone_index), sizeof(WORD), 1, fp);
        fread(&(m_pmd_bone[i].bone_type), 1, 1, fp);
        fread(&(m_pmd_bone[i].ik_parent_bone_index), sizeof(WORD), 1, fp);
        fread(&(m_pmd_bone[i].bone_head_pos.x), 12, 1, fp);
    }


    TRACE(_T("last_bone_x = %f\n"), m_pmd_bone[m_bone_count - 1].bone_head_pos.x);

    //IKリスト==================================================

    fread(&m_ik_data_count, sizeof(WORD), 1, fp);

    TRACE(_T("IK数 = %d\n"), m_ik_data_count);

    PMD_IK *pmd_ik = new PMD_IK[m_ik_data_count]; // 頂点番号(3個/面)
    for (int i = 0; i < (int)m_ik_data_count; i++){
        fread(&(pmd_ik[i].ik_bone_index), sizeof(WORD), 1, fp);
        fread(&(pmd_ik[i].ik_target_bone_index), sizeof(WORD), 1, fp);
        fread(&(pmd_ik[i].ik_chain_length), 1, 1, fp);
        fread(&(pmd_ik[i].iterations), sizeof(WORD), 1, fp);
        fread(&(pmd_ik[i].control_weight), sizeof(float), 1, fp);
        pmd_ik[i].ik_child_bone_index = new WORD[pmd_ik[i].ik_chain_length];
        fread(pmd_ik[i].ik_child_bone_index, sizeof(WORD) * pmd_ik[i].ik_chain_length, 1, fp);
    }

    TRACE(_T("IK値 = %d\n"), pmd_ik[m_ik_data_count - 1].ik_bone_index);


    //表情リスト====================================================


    fread(&m_skin_count, sizeof(WORD), 1, fp);	//最初の一つはベース

    TRACE(_T("表情数(うちベース1つ) = %d\n"), m_skin_count);

    PMD_SKIN *pmd_skin = new PMD_SKIN[m_skin_count]; // 頂点番号(3個/面)
    for (i = 0; i < (int)m_skin_count; i++){
        fread(pmd_skin[i].skin_name, 20, 1, fp);
        fread(&(pmd_skin[i].skin_vert_count), sizeof(DWORD), 1, fp);
        fread(&(pmd_skin[i].skin_type), 1, 1, fp);
        pmd_skin[i].skin_vert_data = new PMD_VERT_DATA[pmd_skin[i].skin_vert_count];
        fread(pmd_skin[i].skin_vert_data, 16 * pmd_skin[i].skin_vert_count, 1, fp);
    }

    TRACE(_T("表情頂点数[0] = %d\n"), pmd_skin[0].skin_vert_count);
    TRACE(_T("表情頂点数[1] = %d\n"), pmd_skin[1].skin_vert_count);

    TRACE(_T("表情頂点数[last] = %d\n"), pmd_skin[m_skin_count - 1].skin_vert_count);

    //表情枠用表示リスト============================================
    fread(&m_skin_disp_count, 1, 1, fp);

    WORD *skin_index = new WORD[m_skin_disp_count]; // 表情番号
    fread(skin_index, sizeof(WORD) * m_skin_disp_count, 1, fp);


    TRACE(_T("表情枠数 = %d\n"), m_skin_disp_count);

    //ボーン枠用枠名リスト====================================
    fread(&m_bone_disp_name_count, 1, 1, fp);

    char *disp_name = new char[m_bone_disp_name_count * 50]; // 枠名(50Bytes/枠)
    fread(disp_name, m_bone_disp_name_count * 50, 1, fp);

    TRACE(_T("ボーン枠name数 = %d\n"), m_bone_disp_name_count);


    //ボーン枠用表示リスト=======================================


    fread(&m_bone_disp_count, sizeof(DWORD), 1, fp);	//最初の一つはベース

    TRACE(_T("ボーン枠数 = %d\n"), m_bone_disp_count);
    if (m_bone_disp_count > 0){
        PND_BONE_DISP *pmd_bone_disp = new PND_BONE_DISP[m_bone_disp_count]; // 頂点番号(3個/面)
        for (i = 0; i < (int)m_bone_disp_count; i++){
            fread(&(pmd_bone_disp[i].bone_index), sizeof(WORD), 1, fp);
            fread(&(pmd_bone_disp[i].bone_disp_frame_index), 1, 1, fp);
        }

        TRACE(_T("枠用ボーン番号[last] = %d\n"), pmd_bone_disp[m_bone_disp_count - 1].bone_disp_frame_index);
    }
    //========================================================
    //以下は拡張機能==========================================
    if (feof(fp) == 0){

        //拡張機能ヘッダー
        fread(&m_english_name_compatibility, 1, 1, fp);
        fread(m_model_name_eg, 20, 1, fp);
        fread(m_comment_eg, 256, 1, fp);

        //ボーン名(英語)
        char* bone_name_eg = new char[m_bone_count * 20]; // ボーン名20byte * bone_count
        fread(bone_name_eg, m_bone_count * 20, 1, fp);

        //表情名(英語)
        char* skin_name_eg = new char[m_skin_count * 20]; // 表情名20byte * (skin_count - 1); baseは名前なし
        fread(skin_name_eg, m_skin_count * 20, 1, fp);

        //ボーン枠名(英語) // MMDでは「区分名」
        char* disp_name_eg = new char[m_bone_disp_name_count * 50]; //英名50byte * bone_disp_name_count;センターは英名が登録されない
        fread(disp_name_eg, m_bone_disp_name_count * 50, 1, fp);

        //トゥーンテクスチャファイル名 個数は10個固定 (total 1000Bytes)
        char toon_file_name[100][10];
        fread(toon_file_name, 1000, 1, fp);

        //以下物理演算
        //今後必要なら作る


    }
#endif

    fclose(fp);

    return 0;
}




/*
PMXファイル内の文字列データを読み込む
*/
TCHAR* PMXLoader::mReadText(FILE* fp, int encode_type){

    int size;
    fread(&size, 4, 1, fp);
    TCHAR* buffer = new TCHAR[size + 1];
    memset(buffer, 0, sizeof(TCHAR)*(size + 1));

    if (size > 0){
        fread(buffer, size, 1, fp);
    }
    return buffer;
}

/*
//ボーンの階層構造の作成
void PMDLoader::CreateBone(){

    struct BONE_CHAIN{
        vec3f position;
        BONE_CHAIN* parent;
        BONE_CHAIN* children;
        BONE_CHAIN* next;
    };

    BONE_CHAIN* bone_chain = new BONE_CHAIN[m_bone_count];

    memset(bone_chain, 0, sizeof(BONE_CHAIN) * m_bone_count);

    int i;
    for (i = 0; i < m_bone_count; i++){
        bone_chain[i].position = m_pmd_bone[i].bone_head_pos;

        if (m_pmd_bone[i].parent_bone_index >= 0){
            //親がいる場合

            BONE_CHAIN* parent = bone_chain + m_pmd_bone[i].parent_bone_index;
            bone_chain[i].parent = parent;

            //子へ追加
            bone_chain[i].next = parent->children;
            parent->children = bone_chain + i;
        }

    }


}
*/


/*
int PMDLoader::GetPMDMaterialList(PMD_MATERIAL** material){
*material = m_pmd_material;
return m_material_count;
}
*/

int PMXLoader::GetMaterialList(MATERIAL_INFO** pmaterials){

    MATERIAL_INFO* materials = new MATERIAL_INFO[m_material_count];
    UINT start = 0;
    TCHAR texpath[MAX_PATH];

    _tcscpy(texpath, _T("resource\\"));


    for (int i = 0; i < m_material_count; i++){
        materials[i].indexCount = m_pmd_material[i].face_index_count;
        materials[i].indexOffset = start;
        start += m_pmd_material[i].face_index_count;
        materials[i].diffuse_color[0] = m_pmd_material[i].diffuse[0];
        materials[i].diffuse_color[1] = m_pmd_material[i].diffuse[1];
        materials[i].diffuse_color[2] = m_pmd_material[i].diffuse[2];
        materials[i].diffuse_color[3] = m_pmd_material[i].diffuse[3];

        /*
        if (m_pmd_material[i].texture_file_name[0] != '\0'){

            materials[i].texture_name = _tcsdup(m_pmd_material[i].texture_file_name);
            //MultiByteToTChar(texpath + _tcslen(texpath), pmd_material[i].texture_file_name, strlen(pmd_material[i].texture_file_name) + 1);
            //materials[i].textureid = comTexture->LoadTexture(texpath, NULL, NULL);
        } else{
            materials[i].texture_name = NULL;
        }
        */

        if (m_pmd_material[i].texture_index > 0){
            size_t length = wcslen(m_dirname) + wcslen(m_texture_names[m_pmd_material[i].texture_index]) + 1;

            materials[i].texture_name = new WCHAR[length];
            wcscpy(materials[i].texture_name, m_dirname);
            wcscat(materials[i].texture_name, m_texture_names[m_pmd_material[i].texture_index]);
        } else{
            materials[i].texture_name = NULL;
        }
        materials[i].textureid = 0;
    }

    *pmaterials = materials;
    
    //暫定仕様//
    if ((0 < m_time_frame) && (m_time_frame < m_material_count)){
        return m_time_frame;
    } else{
        return m_material_count;
    }
    
}


UINT PMXLoader::GetVertexBuffer(LPVOID* ppBuffer, UINT* pStride){
    *ppBuffer = (LPVOID)m_vertex;
    *pStride = sizeof(VertexData);	//1データのサイズ
    return m_vertex_count;
};

UINT PMXLoader::GetIndexBuffer(LPVOID* ppBuffer, UINT* pStride){
    *ppBuffer = (LPVOID)m_indexes;
    *pStride = m_index_size;	//1データのサイズ
    
    return m_index_count;
};


void PMXLoader::GetBoxaxis(mat33d* boxaxis, vec3d* boxorg){


    float max_x = 0.0;
    float max_y = 0.0;
    float max_z = 0.0;

    for (unsigned int i = 0; i < m_vertex_count; i++){
        if (fabs(m_vertex[i].position_x) > max_x){
            max_x = fabs(m_vertex[i].position_x);
        }
        if (fabs(m_vertex[i].position_y) > max_y){
            max_y = fabs(m_vertex[i].position_y);
        }
        if (fabs(m_vertex[i].position_z) > max_z){
            max_z = fabs(m_vertex[i].position_z);
        }
    }

    boxaxis->a.Set(max_x*2.0, 0.0, 0.0);
    boxaxis->b.Set(0.0, max_y*2.0, 0.0);
    boxaxis->c.Set(0.0, 0.0, max_z*2.0);

    boxorg->Set(-max_x, -max_y, -max_z);

}
