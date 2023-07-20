#pragma once

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/*

class VMEC_XYZ_Reader
{

private:
    
    FILE* m_fp;
    const int m_record_size_width;
    const int m_is_big_endian;
    size_t m_record_size;
    char* m_filepath;

    void mChangeOrder(float* dest, const float* src, size_t count);

public:
    VMEC_XYZ_Reader(int record_size_width, int is_big_endian);
    ~VMEC_XYZ_Reader();

    int Open(const char* filepath);// , int num_grid_x, int num_grid_y, int num_grid_z, int num_grids_a);

    size_t GetSize();
    int Read(float* xyz_buffer);
};



*/

class VMEC_Reader
{

private:
    FILE* m_fp;
    const int m_record_size_width;
    const int m_is_big_endian;
    size_t m_record_size;
    //char* m_filepath;

    void mOrder(float* dest, const float* src, size_t count, const int offset, const int stride);
    void mOrderTransEndian(float* dest, const float* src, size_t count, const int offset, const int stride);

public:
    VMEC_Reader(int record_size_width, int is_big_endian);
    ~VMEC_Reader();

    int Open(const char* filepath);// , int num_grid_x, int num_grid_y, int num_grid_z, int num_grids_a);

    size_t GetSize();
    int Read(float* buffer, const int offset, const int stride);
};



