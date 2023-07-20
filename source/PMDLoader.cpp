
#define _CRT_SECURE_NO_DEPRECATE

#include "PMDLoader.h"
#include "mychardump.h"
#include "debugprint.h"


PMDLoader::PMDLoader(const TCHAR* filename) : 
		m_vertex(NULL),
		m_vertex_count(0),
		m_index4b(NULL),
		m_index2b(NULL),
		m_index_count(0),
		m_index_size(2),
		m_dirname(NULL),
        m_time_frame(0)
{


	//以後はデータ部分の処理

	Load(filename);
	
	//LoadTexture();	//テクスチャの読み込み.
	
}


PMDLoader::~PMDLoader(){
	
	delete [] m_index4b;
	delete [] m_index2b;

	delete [] m_vertex;
	delete[] m_vec_supplement;

}



int PMDLoader::Load(const TCHAR* filename){


	//dir名の取得
	int dirlen = 0;
	TCHAR* tp = (TCHAR*)_tcsrchr(filename, _T('\\'));
	if(tp){ dirlen = (int)(tp - filename) + 1;}
	m_dirname = new TCHAR[dirlen + 1];
	_tcsncpy(m_dirname, filename, dirlen);
	m_dirname[dirlen] = _T('\0');

	

	FILE* fp = _tfopen(filename, _T("rb"));
	
//ヘッダーの読み込み=========================
	PMD_HEADER pmdh;
	fread(&(pmdh.magic), 3, 1, fp);
	fread(&(pmdh.version), 4, 1, fp);
	fread(&(pmdh.model_name), 20, 1, fp);
	fread(&(pmdh.comment), 256, 1, fp);

	TRACE(_T("PMD_ver.=%f\n"),pmdh.version);

//頂点データの読み込み==========================
	m_vertex_count = 0;
	fread(&m_vertex_count, 4, 1, fp);		//頂点数

	TRACE(_T("頂点数 = %d\n"), (int)m_vertex_count);

	m_vertex = new VertexData[m_vertex_count];	//頂点はposition, normal, texture_uvがセットになって一要素
	m_vec_supplement = new PMD_VERTEX_SUPPLEMENT[m_vertex_count];	//頂点はposition, normal, texture_uvがセットになって一要素
	
    for (int i = 0; i < (int)m_vertex_count; i++){
		fread(m_vertex + i, 32, 1, fp);
		fread(m_vec_supplement + i, 6, 1, fp);

	}
	
	TRACE(_T("最終頂点 : %f, %f, %f\n"), m_vertex[m_vertex_count - 1].position_x, m_vertex[m_vertex_count - 1].position_y, m_vertex[m_vertex_count - 1].position_z);

//面(インデックス)データ============================
	fread(&m_index_count, 4, 1, fp);
	
	if(m_vertex_count > 0x10000){
		m_index_size = 4;
		m_index4b = new UINT[m_index_count]; // 頂点番号(3個/面)
		fread(m_index4b, m_index_size * m_index_count, 1, fp);

	}else{
		m_index_size = 2;
		m_index2b = new USHORT[m_index_count]; // 頂点番号(3個/面)
		fread(m_index2b, m_index_size * m_index_count, 1, fp);
	
	}


	TRACE(_T("size = %d\n"), m_index_count);


//材質(マテリアル)データ=============================
	fread(&m_material_count, 4, 1, fp);
	
	m_pmd_material = new PMD_MATERIAL[m_material_count]; // 頂点番号(3個/面)
    for (int i = 0; i < m_material_count; i++){
		fread(m_pmd_material + i, 44, 1, fp);
		fread(&(m_pmd_material[i].toon_index), 1, 1, fp);
		fread(&(m_pmd_material[i].edge_flag), 1, 1, fp);
		fread(&(m_pmd_material[i].face_index_count), 4, 1, fp);
		fread(m_pmd_material[i].texture_file_name, 20, 1, fp);
		m_pmd_material[i].texture_file_name[20] = '\0';

		//スフィアテクスチャのチェック
		char* cp = strchr(m_pmd_material[i].texture_file_name, '*');
		if(cp){
			//テクスチャもスフィア両方ある
			strcpy(m_pmd_material[i].sphere_file_name,cp + 1);
			*cp = '\0';
		
		}else if (strcmp(m_pmd_material[i].texture_file_name + strlen(m_pmd_material[i].texture_file_name) - 4, ".sph")==0) {
			//スフィアのみ存在する
			strcpy(m_pmd_material[i].sphere_file_name, m_pmd_material[i].texture_file_name);
			m_pmd_material[i].texture_file_name[0] = '\0';
		
		}else if (strcmp(m_pmd_material[i].texture_file_name + strlen(m_pmd_material[i].texture_file_name) - 4, ".spa")==0) {
				//またスフィアマップのファイル名拡張子"sph"を、"spa"にすることにより,
				//スフィアマップの展開が乗算でなく、加算で行われる仕様が追加されています(ver5.12より）
				//ref. VPVP wiki.
			//スフィアのみ存在する
			strcpy(m_pmd_material[i].sphere_file_name, m_pmd_material[i].texture_file_name);
			m_pmd_material[i].texture_file_name[0] = '\0';

		}else{
			//テクスチャのみ存在する
			m_pmd_material[i].sphere_file_name[0] = '\0';
		}

	}
	


	TRACE(_T("size = %d\n"), sizeof(PMD_MATERIAL));
	TRACE(_T("face = %d\n"), m_pmd_material[m_material_count-1].face_index_count);
	TRACE(_T("shininess = %f\n"), m_pmd_material[m_material_count-1].specularity);
	
//ボーンデータ=========================================
	fread(&m_bone_count, sizeof(WORD), 1, fp);

	TRACE(_T("ボーン数 = %d\n"), m_bone_count);
	
	m_pmd_bone = new PMD_BONE[m_bone_count]; // 頂点番号(3個/面)
    for (int i = 0; i < (int)m_bone_count; i++){
		fread(m_pmd_bone + i, 20, 1, fp);
		fread(&(m_pmd_bone[i].parent_bone_index), sizeof(WORD), 1, fp);
		fread(&(m_pmd_bone[i].tail_pos_bone_index), sizeof(WORD), 1, fp);
		fread(&(m_pmd_bone[i].bone_type), 1, 1, fp);
		fread(&(m_pmd_bone[i].ik_parent_bone_index), sizeof(WORD), 1, fp);
		fread(&(m_pmd_bone[i].bone_head_pos.x), 12, 1, fp);
	}
	
	
	TRACE(_T("last_bone_x = %f\n"), m_pmd_bone[m_bone_count-1].bone_head_pos.x);

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
	
	PMD_SKIN *pmd_skin = new PMD_SKIN[ m_skin_count ]; // 頂点番号(3個/面)
    for (int i = 0; i < (int)m_skin_count; i++){
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
        for (int i = 0; i < (int)m_bone_disp_count; i++){
			fread(&(pmd_bone_disp[i].bone_index), sizeof(WORD), 1, fp);
			fread(&(pmd_bone_disp[i].bone_disp_frame_index), 1, 1, fp);
		}

		TRACE(_T("枠用ボーン番号[last] = %d\n"), pmd_bone_disp[m_bone_disp_count - 1].bone_disp_frame_index);
	}
//========================================================
//以下は拡張機能==========================================
	if(feof(fp) == 0){

	//拡張機能ヘッダー
	fread(&m_english_name_compatibility,1, 1, fp);
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

	fclose(fp);

	return 0;
}
	


/*
//GLのVBOに乗りやすいデータ構造に変更
void PMDLoader::Build(){



	if( m_vertex ) delete [] m_vertex;
	m_vertex = new VertexData[m_vertex_count];
	
	for(int  i = 0; i < (int)m_vertex_count; i++){
		m_vertex[i].position = m_pmd_vec[i].pos;
		m_vertex[i].normal = m_pmd_vec[i].normal;
		m_vertex[i].u = m_pmd_vec[i].u;
		m_vertex[i].v = m_pmd_vec[i].v;
	}


}
*/

	
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

	for (int  i = 0 ; i < m_bone_count; i++ ){
		bone_chain[i].position = m_pmd_bone[i].bone_head_pos;

		if(m_pmd_bone[i].parent_bone_index >= 0){
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

int PMDLoader::GetMaterialList(MATERIAL_INFO** pmaterials){
	
	//int count = pmd->GetPMDMaterialList(&pmd_material);
	MATERIAL_INFO* materials = new MATERIAL_INFO[m_material_count];
	UINT start = 0;
	TCHAR texpath[MAX_PATH];

	_tcscpy(texpath, _T("resource\\"));


	for (int i = 0; i < m_material_count; i++){
		materials[i].indexCount = m_pmd_material[i].face_index_count;
		materials[i].indexOffset = start;
		start += m_pmd_material[i].face_index_count;
		materials[i].diffuse_color[0] = m_pmd_material[i].diffuse_color[0];
		materials[i].diffuse_color[1] = m_pmd_material[i].diffuse_color[1];
		materials[i].diffuse_color[2] = m_pmd_material[i].diffuse_color[2];
		materials[i].diffuse_color[3] = m_pmd_material[i].alpha;

		if (m_pmd_material[i].texture_file_name[0] != '\0'){
			
            WCHAR* texture_name = MyCharDump<WCHAR, char>(m_pmd_material[i].texture_file_name);
            size_t length = wcslen(m_dirname) +wcslen(texture_name) + 1;

            materials[i].texture_name = new WCHAR[length];
            wcscpy(materials[i].texture_name, m_dirname);
            wcscat(materials[i].texture_name, texture_name);
            delete[] texture_name;

		}
		else{
            materials[i].texture_name = NULL;
		}
		materials[i].textureid = 0;
	}

	*pmaterials = materials;

    //暫定仕様//
    if ((0 < m_time_frame) && (m_time_frame < m_material_count)){
        return m_time_frame;
    }else{
        return m_material_count;
    }
}


UINT PMDLoader::GetVertexBuffer(LPVOID* ppBuffer, UINT* pStride){
	*ppBuffer = (LPVOID)m_vertex;
	*pStride = sizeof(VertexData);	//1データのサイズ
	return m_vertex_count;
};

UINT PMDLoader::GetIndexBuffer(LPVOID* ppBuffer, UINT* pStride){
	if (m_index_size == 4){
		*ppBuffer = (LPVOID)m_index4b;
		*pStride = sizeof(UINT);	//1データのサイズ
	}
	else{
		*ppBuffer = (LPVOID)m_index2b;
		*pStride = sizeof(USHORT);	//1データのサイズ
	}
	return m_index_count;
};


void PMDLoader::GetBoxaxis(mat33d* boxaxis, vec3d* boxorg){


    float max_x = 0.0;
    float max_y = 0.0;
    float max_z = 0.0;

    for (int i = 0; i < m_vertex_count; i++){
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