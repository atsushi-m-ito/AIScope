#include "VRMLLoader.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

#include "mychardump.h"
#include "ReadWord.h"

static const char DELIMITOR[] = " ,()\"\t\r\n";
static const char SINGLES[] = "[]{}";

VRMLLoader::VRMLLoader(const TCHAR* filename) :
	m_vertex(NULL),
	m_vertex_count(0),
	m_indexes(NULL),
	m_index_count(0),
	m_index_size(0),
	m_dirname(NULL),
	m_materials(NULL),
	m_material_count(0),
	m_time_frame(0),
	m_material_composit_count(0),
	m_material_composit(NULL)
{


    //以後はデータ部分の処理

    mLoad(filename);


    //LoadTexture();	//テクスチャの読み込み.

}


VRMLLoader::~VRMLLoader(){

    delete[] m_indexes;

    delete[] m_vertex;
    delete[] m_dirname;
	delete[] m_materials;
	delete[] m_material_composit;

}


/*
Shapeタグを読む
*/
int VRMLLoader::mReadShape(ReadWord& reader, const char* filename, mat44d& parent_matrix){

    VRML_SHAPE shape;
    shape.diffuse_color[0] = 1.0f;
    shape.diffuse_color[1] = 1.0f;
    shape.diffuse_color[2] = 1.0f;
    shape.matrix = parent_matrix;


    int index = 0;

    while (char* p = reader.Get()){

        //終了タグ//
        if (*p == '}'){
            break;
            //さらに階層をもぐる//
        } else if (*p == '['){
            index += mReadSkipAny(reader, ']');
        } else if (*p == '{'){
            index += mReadSkipAny(reader, '}');


        } else if (strcmp(p, "appearance") == 0){
            p = reader.Get();
            if (strcmp(p, "Appearance") == 0){
                p = reader.Get();
                if (*p == '{'){
                    mReadAppearance(reader, &shape);
                }
            }

        } else if (strcmp(p, "geometry") == 0){
            p = reader.Get();
            if (strcmp(p, "IndexedFaceSet") == 0){
                p = reader.Get();
                if (*p == '{'){
                    mReadIndexedFaceSet(reader, &shape, filename);
                }
            }
        }

        index++;
    }



    m_vrml_shape.push_back(shape);

    return index;

}


int VRMLLoader::mReadAppearance(ReadWord& reader, VRML_SHAPE* shape){

    int index = 0;

    while (char* p = reader.Get()){
        index++;

        //終了タグ//
        if (*p == '}'){
            break;

            //さらに階層をもぐる//
        } else if (*p == '['){
            index += mReadSkipAny(reader, ']');
        } else if (*p == '{'){
            index += mReadSkipAny(reader, '}');
        
        } else if (strcmp(p, "material") == 0){
            p = reader.Get();
            if (strcmp(p, "Material") == 0){
                p = reader.Get();
                if (*p == '{'){

                    while (p = reader.Get()){

                        //終了タグ//
                        if (*p == '}'){
                            break;
                            //さらに階層をもぐる//
                        } else if (*p == '['){
                            index += mReadSkipAny(reader, ']');
                        } else if (*p == '{'){
                            index += mReadSkipAny(reader, '}');

                        } else if (strcmp(p, "diffuseColor") == 0){
                            shape->diffuse_color[0] = strtof(reader.Get(), NULL);
                            shape->diffuse_color[1] = strtof(reader.Get(), NULL);
                            shape->diffuse_color[2] = strtof(reader.Get(), NULL);

                        }
                    }
                }
            }
        }
    }

    return index;

}


int VRMLLoader::mReadIndexedFaceSet(ReadWord& reader, VRML_SHAPE* shape, const char* filename){

    int index = 0;

    while (char* p = reader.Get()){
        index++;

        //終了タグ//
        if (*p == '}'){
            break;

            //さらに階層をもぐる//
        } else if (*p == '['){
            index += mReadSkipAny(reader, ']');
        } else if (*p == '{'){
            index += mReadSkipAny(reader, '}');

        } else if (strcmp(p, "coord") == 0){
            p = reader.Get();

            shape->coord_offset = (UINT)m_vrml_coord.size() / 3;

            if (strcmp(p, "DEF") == 0){
                std::string defined_name(filename);
                defined_name += reader.Get();
                m_vrml_offset_list.insert(std::pair<std::string, UINT>(defined_name, shape->coord_offset));
                p = reader.Get();
            } else if (strcmp(p, "USE") == 0){
                std::string defined_name(filename);
                defined_name += reader.Get();
                shape->coord_offset = m_vrml_offset_list[defined_name];
                
                continue;
            }
            
            if (strcmp(p, "Coordinate") != 0){
                //error//
                break;
            } 

            p = reader.Get();
            if (*p != '{'){
                //error//
                break;
            }



            p = reader.Get();
            if (strcmp(p, "point") != 0){
                //error//
                break;
            }

            p = reader.Get();
            if (*p != '['){
                //error//
                break;
            }

            p = reader.Get();
            while (*p != ']'){
                m_vrml_coord.push_back(strtof(p, NULL));
                p = reader.Get();
            }

            index += mReadSkipAny(reader, '}');

            /////////////////////////coordの読み込みここまで//

        } else if (strcmp(p, "normal") == 0){
            p = reader.Get();

            shape->normal_offset = (UINT)m_vrml_normal.size() / 3;

            if (strcmp(p, "DEF") == 0){
                std::string defined_name(filename);
                defined_name += reader.Get();
                m_vrml_offset_list.insert(std::pair<std::string, UINT>(defined_name, shape->normal_offset));
                p = reader.Get();
            } else if (strcmp(p, "USE") == 0){
                std::string defined_name(filename);
                defined_name += reader.Get();
                shape->normal_offset = m_vrml_offset_list[defined_name];
                continue;
            }


            if (strcmp(p, "Normal") != 0){
                //error//
                break;
            }

            p = reader.Get();
            if (*p != '{'){
                //error//
                break;
            }

            p = reader.Get();
            if (strcmp(p, "vector") != 0){
                //error//
                break;
            }

            p = reader.Get();
            if (*p != '['){
                //error//
                break;
            }

            p = reader.Get();
            while (*p != ']'){
                m_vrml_normal.push_back(strtof(p, NULL));
                p = reader.Get();
            }

            index += mReadSkipAny(reader, '}');

            /////////////////////////normalの読み込みここまで//

        } else if (strcmp(p, "coordIndex") == 0){
            p = reader.Get();

            shape->coord_index_offset = (UINT)m_vrml_coord_index.size();
            
            if (*p != '['){
                //error//
                break;
            }

            p = reader.Get();
            while (*p != ']'){
                m_vrml_coord_index.push_back(atoi(p));
                p = reader.Get();
                
                m_vrml_coord_index.push_back(atoi(p));
                p = reader.Get();

                m_vrml_coord_index.push_back(atoi(p));
                p = reader.Get();
                if (strcmp(p, "-1") != 0){

                    index += mReadSkipAny(reader, ']');
                    break;
                }

                p = reader.Get();

            }

            shape->coord_index_count = (UINT)m_vrml_coord_index.size() - shape->coord_index_offset;

            /////////////////////////coordIndexの読み込みここまで//

        } else if (strcmp(p, "normalIndex") == 0){
            p = reader.Get();

            shape->normal_index_offset = (UINT)m_vrml_normal_index.size();

            if (*p != '['){
                //error//
                break;
            }

            p = reader.Get();
            while (*p != ']'){
                m_vrml_normal_index.push_back(atoi(p));
                p = reader.Get();

                m_vrml_normal_index.push_back(atoi(p));
                p = reader.Get();

                m_vrml_normal_index.push_back(atoi(p));
                p = reader.Get();

                if (strcmp(p, "-1") != 0){

                    index += mReadSkipAny(reader, ']');
                    break;
                }

                p = reader.Get();

            }


            /////////////////////////normalIndexの読み込みここまで//
        }

    }

    return index;

}

/*
タグの終了まで読み飛ばす
end_tag は '}' or ']'
*/
int VRMLLoader::mReadSkipAny(ReadWord& reader, const char end_tag){

    int index = 0;

    while (char* p = reader.Get()){
        index++;

        //終了タグ//
        if (*p == end_tag){
            break;

            //さらに階層をもぐる//
        } else if (*p == '['){
            index += mReadSkipAny(reader, ']');
        } else if (*p == '{'){
            index += mReadSkipAny(reader, '}');
        }
    }

    return index;

}

/*
Groupを読み込む
ファイルの読み始めは、parent_group_id==-1として読み込む
*/
int VRMLLoader::mReadGroup(ReadWord& reader, const int parent_group_id, const char* filename, mat44d& parent_matrix){

    mat44d matrix = parent_matrix;

    vec3d translation;
    translation.Clear();
    vec3d rotation_axis;
    rotation_axis.Clear();
    double rotation_angle = 0.0;
    vec3d center;
    center.Clear();

    int flag_translation = 0;
    int flag_rotaion = 0;


    int index = 0;

    while (char* p = reader.Get()){
        index++;

        //終了タグ//
        if (*p == '}'){
            //error//
            break;

            //さらに階層をもぐる//
        } else if (*p == '['){
            index += mReadSkipAny(reader, ']');
        } else if (*p == '{'){
            index += mReadSkipAny(reader, '}');
        
        } else if (strcmp(p, "translation") == 0){
            flag_translation = 1;
            translation.x = strtof(reader.Get(), NULL);
            translation.y = strtof(reader.Get(), NULL);
            translation.z = strtof(reader.Get(), NULL);
    


        } else if (strcmp(p, "center") == 0){
            //回転中心//
            center.x = strtof(reader.Get(), NULL);
            center.y = strtof(reader.Get(), NULL);
            center.z = strtof(reader.Get(), NULL);

        } else if (strcmp(p, "rotation") == 0){
            
            flag_rotaion = 1;

            {

                rotation_axis.x = strtof(reader.Get(), NULL);
                rotation_axis.y = strtof(reader.Get(), NULL);
                rotation_axis.z = strtof(reader.Get(), NULL);
                
                rotation_angle = strtof(reader.Get(), NULL);
                
            }

            
        } else if (strcmp(p, "children") == 0){
            
            p = reader.Get();
            if (*p == '['){

                //マトリックスの適用//
                //同時に指定した場合拡大->回転->移動の順で行われる//
                if (flag_translation){
                    //移動//

                    mat44d t;
                    t.SetIdentity();
                    t.m14 = translation.x;
                    t.m24 = translation.y;
                    t.m34 = translation.z;

                    matrix = matrix * t;
                }

                if(flag_rotaion){
                    //回転中心のずらし//
                    {
                        mat44d t;
                        t.SetIdentity();
                        t.m14 = center.x;
                        t.m24 = center.y;
                        t.m34 = center.z;
                        matrix = matrix * t;
                    }

                    //回転//
                    {//任意の単位ベクトル(vx, vy, vz)まわりに回転する場合（ロドリゲスの公式）//

                        mat44d R;
                        R.Clear();
                        R.m32 = rotation_axis.x;
                        R.m23 = -R.m32;
                        R.m13 = rotation_axis.y;
                        R.m31 = -R.m13;
                        R.m21 = rotation_axis.z;
                        R.m12 = -R.m21;

                        const double angle = rotation_angle;
                        mat44d t;
                        t.SetIdentity();
                        t += R * sin(angle) + (R*R) * (1.0 - cos(angle));
                        matrix = matrix * t;
                    }

                    //回転中心のずらし//
                    {
                        mat44d t;
                        t.SetIdentity();
                        t.m14 = -center.x;
                        t.m24 = -center.y;
                        t.m34 = -center.z;
                        matrix = matrix * t;
                    }
                }



                index += mReadChildren(reader, parent_group_id, filename, matrix);
            }

        }

    }

    return index;

}


    /*
    Groupを読み込む
    ファイルの読み始めは、parent_group_id==-1として読み込む
    */
int VRMLLoader::mReadChildren(ReadWord& reader, const int parent_group_id, const char* filename, mat44d& parent_matrix){

    int index = 0;

    while (char* p = reader.Get()){
        index++;

        //終了タグ//
        if (*p == ']'){
            //error//
            break;

            //さらに階層をもぐる//
        } else if (*p == '['){
            index += mReadSkipAny(reader, ']');
        } else if (*p == '{'){
            index += mReadSkipAny(reader, '}');

            //子グループがあったので追加//
        } else if ((strcmp(p, "Group") == 0) || (strcmp(p, "Anchor") == 0) || (strcmp(p, "Transform") == 0)){
            VRML_GROUP group;
            group.parent_id = parent_group_id;
            m_groups.push_back(group);

            p = reader.Get();
            if (*p == '{'){

                index += mReadGroup(reader, (UINT)m_groups.size() - 1, filename, parent_matrix);
            }


        } else if (strcmp(p, "Shape") == 0){
            //materialの読み込み//

            p = reader.Get();
            if (*p == '{'){
                mReadShape(reader, filename, parent_matrix);
            }

        } else if(strcmp(p, "Inline")==0){
            //外部ファイルの読み込み(実際は後で読む)//
            p = reader.Get();
            if (*p == '{'){
                
                p = reader.Get();
                if (strcmp(p, "url") == 0){

                    VRML_INLINE inlne_file;
                    inlne_file.filename = reader.Get();
                    inlne_file.matrix = parent_matrix;

                    m_vrml_inline_file.push_back(inlne_file);
                }

                mReadSkipAny(reader, '}');
            }
            
        }

    }

    return index;

}
 


int VRMLLoader::mLoad(const WCHAR* filepath){


    //dir名の取得
    int dirlen = 0;
    TCHAR* tp = (TCHAR*)_tcsrchr(filepath, _T('\\'));
    if (tp){ dirlen = (int)(tp - filepath) + 1; }
    m_dirname = new TCHAR[dirlen + 1];
    _tcsncpy(m_dirname, filepath, dirlen);
    m_dirname[dirlen] = _T('\0');

    //filename
    char* filename = MyCharDump<char, WCHAR>(tp + 1);





    FILE* fp = _wfopen(filepath, L"r");

    const int LINE_SIZE = 1024;
    char line[LINE_SIZE];


    //最初の一行の読み込み//
    if (fgets(line, LINE_SIZE, fp) == NULL){
        return 0;
    } else if (strncmp(line, "#VRML V2.0 utf8", 15) != 0){
        return 0;

    }
    
    {
        ReadWord reader(fp, LINE_SIZE, DELIMITOR, SINGLES);
        mat44d matrix;
        matrix.SetIdentity();
        mReadChildren(reader, -1, filename, matrix);
    }

    fclose(fp);
    delete[] filename;

    int num_groups = m_groups.size();

    int num_coord = m_vrml_coord.size();
    int num_normal = m_vrml_normal.size();
    int num_coord_index = m_vrml_coord_index.size();
    int num_normal_index = m_vrml_normal_index.size();


    /*
    VRML to general mesh
    */
    {
        int num_vertex = 0;
        std::vector<VertexData> v_vertex;

        std::vector<UINT> v_face_indexes;

        std::vector<MATERIAL_INFO> v_materials;
		std::vector<MATERIAL_COMPOSIT> v_material_composit;
        
        struct INDEX_COORD_NORMAL{
            UINT coord_index;
            UINT normal_index;
        };

        
        for (int s = 0; s < m_vrml_shape.size(); s++){
            const int i_begin = m_vrml_shape[s].coord_index_offset;
            const int i_end = i_begin + m_vrml_shape[s].coord_index_count;

            //異なるshape間で頂点は共有しないようにindexを振りなおすためここでmapを定義//
            std::map<long long, UINT> vertex_index_list;   //coordおよびnormal配列のオフセットをDEFで毎に定義//


            for (int i = i_begin; i < i_end; i++){
                INDEX_COORD_NORMAL key;
                key.coord_index = m_vrml_coord_index[i];
                key.normal_index = m_vrml_normal_index[i];
                long long key64;
                memcpy(&key64, &key, 8);

                std::map < long long, UINT>::iterator it = vertex_index_list.find(key64);
                if (it == vertex_index_list.end()){
                    //同じ頂点が存在しない場合//
                    vertex_index_list.insert(std::pair < long long, UINT>(key64, num_vertex));
                    v_face_indexes.push_back(num_vertex);

                    VertexData vertex;
                    UINT c_index = key.coord_index * 3;

                    vec4d v;
                    v.Set(m_vrml_coord[c_index], m_vrml_coord[c_index + 1], m_vrml_coord[c_index + 2], 1.0f);
                    v = m_vrml_shape[s].matrix * v;

                    vertex.position_x = v.x;
                    vertex.position_y = v.y;
                    vertex.position_z = v.z;

                    UINT n_index = key.normal_index * 3;
                    v.Set(m_vrml_normal[n_index], m_vrml_normal[n_index + 1], m_vrml_normal[n_index + 2], 0.0f);
                    v = m_vrml_shape[s].matrix * v;

                    vertex.normal_x = v.x;
                    vertex.normal_y = v.y;
                    vertex.normal_z = v.z;

                    vertex.u = 0;
                    vertex.v = 0;
                    v_vertex.push_back(vertex);

                    num_vertex++;
                } else{
                    //同じ頂点が存在する場合//
                    v_face_indexes.push_back (it->second);
                }


            }//loop of shape//


            MATERIAL_INFO material;
            material.indexOffset = m_vrml_shape[s].coord_index_offset;
            material.indexCount = m_vrml_shape[s].coord_index_count;
            material.diffuse_color[0] = m_vrml_shape[s].diffuse_color[0];
            material.diffuse_color[1] = m_vrml_shape[s].diffuse_color[1];
            material.diffuse_color[2] = m_vrml_shape[s].diffuse_color[2];
            material.diffuse_color[3] = 1.0f;

            material.textureid = -1;
            material.texture_name = NULL;
			

			MATERIAL_COMPOSIT mcomp;
			mcomp.begin_index = v_materials.size();
			mcomp.end_index = mcomp.begin_index + 1;
			mcomp.rank = 0;
			v_material_composit.push_back(mcomp);

            v_materials.push_back(material);

        }


        m_vrml_coord.clear();
        m_vrml_normal.clear();
        m_vrml_coord_index.clear();
        m_vrml_normal_index.clear();

        {
            int num_vertex = v_vertex.size();
            int num_index = v_face_indexes.size();
            int num_material = v_materials.size();


        }


        //Inline行による外部ファイルの読み込み//////////////////////////////////////////

		//std::vector<MATERIAL_COMPOSIT> v_material_composit;

        for (int s = 0; s < m_vrml_inline_file.size(); s++){

            UINT offset_vertex = v_vertex.size();;
            UINT offset_face_index = v_face_indexes.size();

            WCHAR* wfilename = MyCharDump<WCHAR, char>(m_vrml_inline_file[s].filename.c_str());
            WCHAR* wpath = new WCHAR[wcslen(m_dirname) + wcslen(wfilename) + 1];
            wcscpy(wpath, m_dirname);
            wcscat(wpath, wfilename);

            //外部ファイル読み込み
            VRMLLoader inline_vrml(wpath);

            //頂点の引継ぎ
            {
                VertexData* inline_vertex;
                UINT stride;
                UINT inline_vertex_count = inline_vrml.GetVertexBuffer((LPVOID*)&inline_vertex, &stride);

                for (int i = 0; i < inline_vertex_count; i++){
                    VertexData vertex = inline_vertex[i];


                    vec4d v;
                    v.Set(vertex.position_x, vertex.position_y, vertex.position_z, 1.0f);
                    v = m_vrml_inline_file[s].matrix * v;

                    //y,z転置//
                    vertex.position_x = v.x;
                    vertex.position_y = v.y;
                    vertex.position_z = v.z;

                    v.Set(vertex.normal_x, vertex.normal_y, vertex.normal_z, 0.0f);
                    v = m_vrml_inline_file[s].matrix * v;

                    //y,z転置//
                    vertex.normal_x = v.x;
                    vertex.normal_y = v.y;
                    vertex.normal_z = v.z;

                    v_vertex.push_back(vertex);


                }
                num_vertex += inline_vertex_count;
            }

            //面indexの引継ぎ
            {
                UINT* inline_face_index;
                UINT stride;
                UINT inline_face_index_count = inline_vrml.GetIndexBuffer((LPVOID*)&inline_face_index, &stride);

                for (int i = 0; i < inline_face_index_count; i++){
                    v_face_indexes.push_back(inline_face_index[i] + offset_vertex);
                }
                
            }

            //materialの引継ぎ
            {
                MATERIAL_INFO* inline_materials;
                UINT inline_material_count = inline_vrml.GetMaterialList(&inline_materials);

				MATERIAL_COMPOSIT mcomp;
				mcomp.begin_index = v_materials.size();
				mcomp.end_index = mcomp.begin_index + inline_material_count;
				mcomp.rank = 0;
				v_material_composit.push_back(mcomp);


                for (int i = 0; i < inline_material_count; i++){
                    MATERIAL_INFO material = inline_materials[i];
                    material.indexOffset += offset_face_index;
                    v_materials.push_back(material);
                }

				
				const int offset_mcomp = mcomp.begin_index;
				const MATERIAL_COMPOSIT* inline_material_composit;
				int num_inline_mcomp = inline_vrml.GetMaterialComposit(&inline_material_composit);
				for (int i = 0; i < num_inline_mcomp; i++) {
					MATERIAL_COMPOSIT mcomp2 = inline_material_composit[i];
					mcomp2.begin_index += offset_mcomp;
					mcomp2.end_index += offset_mcomp;
					mcomp2.rank++;
					v_material_composit.push_back(mcomp2);
				}
				
            }

            {
                int num_vertex = v_vertex.size();
                int num_index = v_face_indexes.size();
                int num_material = v_materials.size();


            }


        }

        //vectorを通常配列にコピー///////////////////////////////////////////////////////

        m_vertex_count = v_vertex.size();  //1776//
        m_vertex = new VertexData[m_vertex_count];
        for (int i = 0; i < m_vertex_count; i++){
            m_vertex[i] = v_vertex[i];
        }


        m_index_count = v_face_indexes.size();
        m_index_size = 4;
        UINT* face_indexes = new UINT[m_index_count];
        m_indexes = (BYTE*)face_indexes;
        for (int i = 0; i < m_index_count; i++){
            face_indexes[i] = v_face_indexes[i];
        }

        m_material_count = v_materials.size();
        m_materials = new MATERIAL_INFO[m_material_count];
        for (int i = 0; i < m_material_count; i++){
            m_materials[i] = v_materials[i];
        }



		std::stable_sort(v_material_composit.begin(), v_material_composit.end());

		m_material_composit_count = v_material_composit.size();
		m_material_composit = new MATERIAL_COMPOSIT[m_material_composit_count];
		for (int i = 0; i < m_material_composit_count; i++) {
			m_material_composit[i] = v_material_composit[i];
		}

    }





    return 0;
}



int VRMLLoader::GetMaterialList(MATERIAL_INFO** pmaterials) {

#if 1

	if (m_time_frame <= 0) {

		*pmaterials = m_materials;
		return m_material_count;
	}

	if (m_time_frame > m_material_composit_count) {
		m_time_frame = m_material_composit_count;
	}

	*pmaterials = m_materials + m_material_composit[m_time_frame - 1].begin_index;
	return m_material_composit[m_time_frame - 1].end_index - m_material_composit[m_time_frame - 1].begin_index;

#else

    *pmaterials = m_materials;

    //暫定仕様//
    if ((0 < m_time_frame) && (m_time_frame < m_material_count)){
        return m_time_frame;
    } else{
        return m_material_count;
    }
#endif
}

int VRMLLoader::GetMaterialComposit(const MATERIAL_COMPOSIT** pmaterial_composit){
	*pmaterial_composit = m_material_composit;
	return m_material_composit_count;
}

UINT VRMLLoader::GetVertexBuffer(LPVOID* ppBuffer, UINT* pStride){
    *ppBuffer = (LPVOID)m_vertex;
    *pStride = sizeof(VertexData);	//1データのサイズ
    return m_vertex_count;
};

UINT VRMLLoader::GetIndexBuffer(LPVOID* ppBuffer, UINT* pStride){
    *ppBuffer = (LPVOID)m_indexes;
    *pStride = m_index_size;	//1データのサイズ

    return m_index_count;
};


void VRMLLoader::GetBoxaxis(mat33d* boxaxis, vec3d* boxorg){


    float max_x = 0.0;
    float max_y = 0.0;
    float max_z = 0.0;

    for (UINT i = 0; i < m_vertex_count; i++){
        if (abs(m_vertex[i].position_x) > max_x){
            max_x = abs(m_vertex[i].position_x);
        }
        if (abs(m_vertex[i].position_y) > max_y){
            max_y = abs(m_vertex[i].position_y);
        }
        if (abs(m_vertex[i].position_z) > max_z){
            max_z = abs(m_vertex[i].position_z);
        }
    }

    boxaxis->a.Set(max_x*2.0, 0.0, 0.0);
    boxaxis->b.Set(0.0, max_y*2.0, 0.0);
    boxaxis->c.Set(0.0, 0.0, max_z*2.0);

    boxorg->Set(-max_x, -max_y, -max_z);

}

