#include "vmec_reader.h"
#include "trans_endian.h"

static const char DELIMITER[] = " \t\r\n";

/*

VMEC_XYZ_Reader::VMEC_XYZ_Reader(int record_size_width, int is_big_endian) :
    m_fp(NULL),
    m_filepath(NULL),
    m_record_size_width(record_size_width),
    m_is_big_endian(is_big_endian)
{

}

VMEC_XYZ_Reader::~VMEC_XYZ_Reader()
{
    if (m_fp){
        fclose(m_fp);
    }
    if (m_filepath){
        delete[] m_filepath;
    }
}

int VMEC_XYZ_Reader::Open(const char* filepath)//, int num_grids_x, int num_grids_y, int num_grids_z, int num_grids_a)
{
    m_filepath = new char[strlen(filepath)+5];
    //ファイル名の合成//
    strcpy(m_filepath, filepath);
    strcat(m_filepath, ".xcd");

    
    m_fp = fopen(m_filepath, "rb");
    if (m_fp == NULL){
        return -1;
    }

    //レコード長の読み込み//


    if (m_record_size_width == 8){
        long long record_size = 0;
        fread(&record_size, m_record_size_width, 1, m_fp);
        if (m_is_big_endian){
            record_size = TransEndian64(record_size);
        }
        m_record_size = record_size / 4;

    } else if (m_record_size_width == 4){
        int record_size = 0;
        fread(&record_size, m_record_size_width, 1, m_fp);
        if (m_is_big_endian){
            record_size = TransEndian32(record_size);
        }
        m_record_size = (long long)record_size / 4;
    }

    return 0;
}

size_t VMEC_XYZ_Reader::GetSize(){
    return m_record_size;
}


int VMEC_XYZ_Reader::Read(float* xyz_buffer, const int stride){
    
    if (m_fp == NULL){
        return -1;
    }

    //バッファ確保//
    float* sub_buffer = new float[m_record_size];
    
    //x-coordinateの読み込み//
    fread(sub_buffer, sizeof(float)*m_record_size, 1, m_fp);
    fclose(m_fp);
    mChangeOrder(xyz_buffer, sub_buffer, m_record_size, stride);
    
    //y-coordinateの読み込み//
    strcpy(m_filepath + strlen(m_filepath) - 4, ".ycd");
    m_fp = fopen(m_filepath, "rb");
    long long record_size = 0;
    fread(&record_size, m_record_size_width, 1, m_fp);
    fread(sub_buffer, sizeof(float)*m_record_size, 1, m_fp);
    fclose(m_fp);
    mChangeOrder(xyz_buffer + 1, sub_buffer, m_record_size);

    //z-coordinateの読み込み//
    strcpy(m_filepath + strlen(m_filepath) - 4, ".zcd");
    m_fp = fopen(m_filepath, "rb");
    record_size = 0;
    fread(&record_size, m_record_size_width, 1, m_fp);
    fread(sub_buffer, sizeof(float)*m_record_size, 1, m_fp);
    fclose(m_fp);
    mChangeOrder(xyz_buffer + 2, sub_buffer, m_record_size);

    m_fp = NULL;
    delete[] sub_buffer;
    return 0;
}

void VMEC_XYZ_Reader::mChangeOrder(float* dest, const float* src, size_t count, const int offset, const int stride){
    if (m_is_big_endian){
        for (size_t i = 0; i < count; i++){
            dest[i * stride] = TransEndian32(src[i]);
        }
    } else{
        for (size_t i = 0; i < count; i++){
            dest[i * 3] = src[i];
        }
    }
}



*/


VMEC_Reader::VMEC_Reader(int record_size_width, int is_big_endian) :
m_fp(NULL),
m_record_size_width(record_size_width),
m_is_big_endian(is_big_endian)
{

}

VMEC_Reader::~VMEC_Reader()
{
    if (m_fp){
        fclose(m_fp);
    }

}

int VMEC_Reader::Open(const char* filepath)//, int num_grids_x, int num_grids_y, int num_grids_z, int num_grids_a)
{

    m_fp = fopen(filepath, "rb");
    if (m_fp == NULL){
        return -1;
    }

    //レコード長の読み込み//


    if (m_record_size_width == 8){
        long long record_size = 0;
        fread(&record_size, m_record_size_width, 1, m_fp);
        if (m_is_big_endian){
            record_size = TransEndian64(record_size);
        }
        m_record_size = record_size / 4;

    } else if (m_record_size_width == 4){
        int record_size = 0;
        fread(&record_size, m_record_size_width, 1, m_fp);
        if (m_is_big_endian){
            record_size = TransEndian32(record_size);
        }
        m_record_size = (long long)record_size / 4;
    }

    return 0;
}

size_t VMEC_Reader::GetSize(){
    return m_record_size;
}


int VMEC_Reader::Read(float* buffer, const int offset, const int stride){

    if (m_fp == NULL){
        return -1;
    }


    float* sub_buffer = new float[m_record_size];

    fread(sub_buffer, sizeof(float)*m_record_size, 1, m_fp);
    fclose(m_fp);
    m_fp = NULL;

    if (m_is_big_endian){
        mOrderTransEndian(buffer, sub_buffer, m_record_size, offset, stride);
    } else{
        mOrder(buffer, sub_buffer, m_record_size, offset, stride);
    }

    delete[] sub_buffer;

    return 0;
}

void VMEC_Reader::mOrderTransEndian(float* dest, const float* src, size_t count, const int offset, const int stride){
    for (size_t i = 0; i < count; i++){
        dest[i*stride + offset] = TransEndian32(src[i]);
    }
}

void VMEC_Reader::mOrder(float* dest, const float* src, size_t count, const int offset, const int stride){
    for (size_t i = 0; i < count; i++){
        dest[i*stride + offset] = src[i];
    }
}