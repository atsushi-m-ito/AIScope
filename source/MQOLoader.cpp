#include "MQOLoader.h"
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <algorithm>

#include "mychardump.h"

static const char DELIMITOR[] = " ()\"\t\r\n";

MQOLoader::MQOLoader(const TCHAR* filename) :
m_vertex(NULL),
m_vertex_count(0),
m_indexes(NULL),
m_index_count(0),
m_index_size(0),
m_dirname(NULL),
m_materials(NULL),
m_material_count(0),
m_time_frame(0)
{


    //以後はデータ部分の処理

    mLoad(filename);


    //LoadTexture();	//テクスチャの読み込み.

}


MQOLoader::~MQOLoader(){

    delete[] m_indexes;

    delete[] m_vertex;
    delete[] m_dirname;

}



int MQOLoader::mReadMaterial(FILE* fp){

    const int LINE_SIZE = 1024;
    char line[LINE_SIZE];


    int index = 0;

    while (fgets(line, LINE_SIZE, fp)){
        char* p = strtok(line, DELIMITOR);
        if (p == NULL){
            continue;
        }

        //終了タグ//
        if (*p == '}'){
            break;
        }
        
        //内容の読み込み//

        //この時点のpにはmaterial nameが入っている//

        
        p = strtok(NULL, DELIMITOR); //文字列colが入るはず//
        
        for (int i = 0; i < 4; i++){
            p = strtok(NULL, DELIMITOR);
            m_materials[index].diffuse_color[i] = strtof(p, NULL);
        }


        m_materials[index].texture_name = NULL;
        m_materials[index].textureid = 0;

        //textureがある場合はpathを読み込む//
        while (p = strtok(NULL, DELIMITOR)){
            if (strcmp(p, "tex") == 0){
                p = strtok(NULL, DELIMITOR);
                WCHAR* texture_name = MyCharDump<WCHAR, char>(p);
                size_t length = wcslen(m_dirname) + wcslen(texture_name) + 1;

                m_materials[index].texture_name = new WCHAR[length];
                wcscpy(m_materials[index].texture_name, m_dirname);
                wcscat(m_materials[index].texture_name, texture_name);

                break;
            }
        }
        
        ///////////////内容の読み込み終了//

        index++;
    }

    return index;

}

/*
vertexタグを読む
*/
int MQOLoader::mReadVertex(FILE* fp){

    const int LINE_SIZE = 1024;
    char line[LINE_SIZE];


    int index = 0;

    while (fgets(line, LINE_SIZE, fp)){
        char* p = strtok(line, DELIMITOR);
        if (p == NULL){
            continue;
        }


        //終了タグ//
        if (*p == '}'){
            break;
        }

        m_vertex[index].position_y = -strtof(p, NULL);
        p = strtok(NULL, DELIMITOR);
        m_vertex[index].position_z = strtof(p, NULL);
        p = strtok(NULL, DELIMITOR);
        m_vertex[index].position_x = -strtof(p, NULL);


        index++;
    }

    return index;

}

/*
faceタグを読む
*/

template<typename T>
int MQOLoader::mReadFace(FILE* fp){

    const int LINE_SIZE = 1024;
    char line[LINE_SIZE];


    int* used_count = new int[m_vertex_count];
    for (int i = 0; i < (int)m_vertex_count; i++){
        used_count[i] = 0;
    }

    MQO_FACE* faces = new MQO_FACE[m_index_count / 3];

    //UVが異なる場合の追加頂点//
    std::vector<VertexData> additioval_vertex;


    int index = 0;

    while (fgets(line, LINE_SIZE, fp)){
        char* p = strtok(line, DELIMITOR);
        if (p == NULL){
            continue;
        }


        //終了タグ//
        if (*p == '}'){
            break;
        }

        if (*p != '3'){
            continue;   //取りあえず三角ポリゴン以外は非対応ということで無視//
        }

        p = strtok(NULL, DELIMITOR);    //Vと入るはず//

        
        faces[index].indexes[0] = atoi(strtok(NULL, DELIMITOR));
        faces[index].indexes[1] = atoi(strtok(NULL, DELIMITOR));
        faces[index].indexes[2] = atoi(strtok(NULL, DELIMITOR));

        p = strtok(NULL, DELIMITOR);    //Mと入るはず//

        faces[index].material_id = atoi(strtok(NULL, DELIMITOR));//material_id//

        //法線ベクトルの計算//
        vec3f a;
        a.x = m_vertex[faces[index].indexes[1]].position_x - m_vertex[faces[index].indexes[0]].position_x;
        a.y = m_vertex[faces[index].indexes[1]].position_y - m_vertex[faces[index].indexes[0]].position_y;
        a.z = m_vertex[faces[index].indexes[1]].position_z - m_vertex[faces[index].indexes[0]].position_z;

        vec3f b;
        b.x = m_vertex[faces[index].indexes[2]].position_x - m_vertex[faces[index].indexes[0]].position_x;
        b.y = m_vertex[faces[index].indexes[2]].position_y - m_vertex[faces[index].indexes[0]].position_y;
        b.z = m_vertex[faces[index].indexes[2]].position_z - m_vertex[faces[index].indexes[0]].position_z;

        vec3f normal = Cross(b,a);
        normal.Normalize();


        p = strtok(NULL, DELIMITOR);    //UVと入るはず//


        //UVを読み取って末に使われている頂点と同じものか判定//
        //異なるUV値なら頂点を複製//
        for (int i = 0; i < 3; i++){
            float u = strtof(strtok(NULL, DELIMITOR), NULL);
            float v = strtof(strtok(NULL, DELIMITOR), NULL);

            int id = faces[index].indexes[i];

            if (used_count[id] == 0){
                used_count[id] = 1;
                m_vertex[id].normal_x = normal.x;
                m_vertex[id].normal_y = normal.y;
                m_vertex[id].normal_z = normal.z;
                m_vertex[id].u = u;
                m_vertex[id].v = v;
#if 1
                //case 0

            } else if ((m_vertex[id].u != u) || (m_vertex[id].v != v) || (m_vertex[id].normal_x != normal.x) || (m_vertex[id].normal_y != normal.y) || (m_vertex[id].normal_z != normal.z)){
                VertexData vtx;
                vtx.position_x = m_vertex[id].position_x;
                vtx.position_y = m_vertex[id].position_y;
                vtx.position_z = m_vertex[id].position_z;
                vtx.normal_x = normal.x;
                vtx.normal_y = normal.y;
                vtx.normal_z = normal.z;
                vtx.u = u;
                vtx.v = v;
                additioval_vertex.push_back(vtx);
                faces[index].indexes[i] = (int)additioval_vertex.size() - 1 + m_vertex_count;

            } else{
                used_count[id] ++;
            }
#else
            } else if ((m_vertex[id].u == u) && (m_vertex[id].v == v)){
                used_count[id] ++;
                m_vertex[id].normal_x += normal.x;
                m_vertex[id].normal_y += normal.y;
                m_vertex[id].normal_z += normal.z;

            } else{
                VertexData vtx;
                vtx.position_x = m_vertex[id].position_x;
                vtx.position_y = m_vertex[id].position_y;
                vtx.position_z = m_vertex[id].position_z;
                vtx.normal_x = normal.x;
                vtx.normal_y = normal.y;
                vtx.normal_z = normal.z;
                vtx.u = u;
                vtx.v = v;
                additioval_vertex.push_back(vtx);
                faces[index].indexes[i] = (int)additioval_vertex.size() - 1 + m_vertex_count;
            }
#endif
        }



        index++;
    }

    //追加頂点がある場合はバッファを更新//
    if (additioval_vertex.size() > 0){
        UINT re_vertex_count = m_vertex_count + (UINT)additioval_vertex.size();
        VertexData* re_vertex = new VertexData[re_vertex_count];
        memcpy(re_vertex, m_vertex, sizeof(VertexData) * m_vertex_count);
        for (UINT i = m_vertex_count; i < re_vertex_count; i++){
            re_vertex[i] = additioval_vertex[i - m_vertex_count];
        }
        delete[] m_vertex;
        m_vertex = re_vertex;
        m_vertex_count = re_vertex_count;
    }

    //法線のnormalize//
    for (UINT i = 0; i < m_vertex_count; i++){
        ((vec3f*)(&(m_vertex[i].normal_x)))->Normalize();
    }


    //faceをmaterial_idでソート//
    std::sort(faces, faces + index);

    int material_id = -1;
    int offset = 0;

    T* indexes = (T*)m_indexes;

    for (int i = 0; i < index; i++){
        indexes[i * 3] = faces[i].indexes[0];
        indexes[i * 3 + 1] = faces[i].indexes[1];
        indexes[i * 3 + 2] = faces[i].indexes[2];

        if (material_id != faces[i].material_id){
            if (material_id >= 0){
                m_materials[material_id].indexCount = (i - offset)*3;
            }

            material_id = faces[i].material_id;
            m_materials[material_id].indexOffset = i * 3;
            offset = i;
        }
    }
    if (material_id >= 0){
        m_materials[material_id].indexCount = (index - offset) * 3;
    }

    
    return index;

}

/*
objectタグを読む
*/
int MQOLoader::mReadObject(FILE* fp){

    const int LINE_SIZE = 1024;
    char line[LINE_SIZE];


    int index = 0;

    while (fgets(line, LINE_SIZE, fp)){
        char* p = strtok(line, DELIMITOR);
        if (p == NULL){
            continue;
        }


        //終了タグ//
        if (*p == '}'){
            break;
        }

        if (strcmp(p, "vertex") == 0){
            p = strtok(NULL, DELIMITOR);
            m_vertex_count = atoi(p);


            p = strtok(NULL, DELIMITOR);
            if (*p != '{'){
                //error//
                m_vertex_count = 0;

            } else{


                m_vertex = new VertexData[m_vertex_count];

                int count = mReadVertex(fp);
                if (count != m_vertex_count){
                    //error//
                    m_vertex_count = count;
                }
            }


        } else if (strcmp(p, "face") == 0){
            //Objectの読み込み//

            p = strtok(NULL, DELIMITOR);
            m_index_count = atoi(p) * 3;

            p = strtok(NULL, DELIMITOR);
            if (*p != '{'){
                //error//
                m_index_count = 0;
            } else{

                int count;
                if (m_index_count >= 0x10000){
                    m_index_size = 4;
                    m_indexes = new BYTE[m_index_count * m_index_size];
                    count = mReadFace<unsigned int>(fp);

                } else{
                    m_index_size = 2;
                    m_indexes = new BYTE[m_index_count * m_index_size];
                    count = mReadFace<unsigned short>(fp);
                }
                if (count * 3 != m_index_count){
                    //error//
                    m_index_count = count * 3;
                }
            }
        }

        index++;
    }

    return index;

}

/*
タグの終了まで読み飛ばす
*/
int MQOLoader::mReadSkipAny(FILE* fp){

    const int LINE_SIZE = 1024;
    char line[LINE_SIZE];


    int index = 0;

    while (fgets(line, LINE_SIZE, fp)){
        index++;

        char* p = strtok(line, DELIMITOR);
        if (p == NULL){
            continue;
        }

        //終了タグ//
        if (*p == '}'){
            break;
        }

    }

    return index;

}

int MQOLoader::mLoad(const WCHAR* filename){


    //dir名の取得
    int dirlen = 0;
    TCHAR* tp = (TCHAR*)_tcsrchr(filename, _T('\\'));
    if (tp){ dirlen = (int)(tp - filename) + 1; }
    m_dirname = new TCHAR[dirlen + 1];
    _tcsncpy(m_dirname, filename, dirlen);
    m_dirname[dirlen] = _T('\0');



    FILE* fp = _wfopen(filename, L"r");

    const int LINE_SIZE = 1024;
    char line[LINE_SIZE];



    while (fgets(line, LINE_SIZE, fp)){
        char* p = strtok(line, DELIMITOR);
        if (p == NULL){
            continue;
        }


        if (strcmp(p, "Material") == 0){
            //materialの読み込み//

            p = strtok(NULL, DELIMITOR);
            m_material_count = atoi(p);

            p = strtok(NULL, DELIMITOR);
            if (*p != '{'){
                //error//
                m_material_count = 0;
                break;
            }

            m_materials = new MATERIAL_INFO[m_material_count];
            int count = mReadMaterial(fp);
            if (count != m_material_count){
                //error//
                m_material_count = count;
            }


        } else if (strcmp(p, "Object") == 0){
            //Objectの読み込み//

            p = strtok(NULL, DELIMITOR);
            //skip, p is object name//

            p = strtok(NULL, DELIMITOR);
            if (*p != '{'){
                //error//
                //m_material_count = 0;
                break;
            }

            mReadObject(fp);


        } else{

            while (p = strtok(NULL, DELIMITOR)){
                if (*p == '{'){
                    mReadSkipAny(fp);
                    break;
                }
            }





        }

    }

    fclose(fp);

    return 0;
}



int MQOLoader::GetMaterialList(MATERIAL_INFO** pmaterials){


    *pmaterials = m_materials;

    //暫定仕様//
    if ((0 < m_time_frame) && (m_time_frame < m_material_count)){
        return m_time_frame;
    } else{
        return m_material_count;
    }

}


UINT MQOLoader::GetVertexBuffer(LPVOID* ppBuffer, UINT* pStride){
    *ppBuffer = (LPVOID)m_vertex;
    *pStride = sizeof(VertexData);	//1データのサイズ
    return m_vertex_count;
};

UINT MQOLoader::GetIndexBuffer(LPVOID* ppBuffer, UINT* pStride){
    *ppBuffer = (LPVOID)m_indexes;
    *pStride = m_index_size;	//1データのサイズ

    return m_index_count;
};


void MQOLoader::GetBoxaxis(mat33d* boxaxis, vec3d* boxorg){


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

