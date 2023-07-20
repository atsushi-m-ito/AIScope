
#pragma once

//endianの変換//
template <typename T>
T TransEndian32(const T& src){
    T dest;
    const unsigned char* src_b = (const unsigned char*)&src;
    unsigned char* dest_b = (unsigned char*)&dest;
    
    dest_b[0] = src_b[3];
    dest_b[1] = src_b[2];
    dest_b[2] = src_b[1];
    dest_b[3] = src_b[0];
    
    return dest;
};


//endianの変換//
template <typename T>
T TransEndian64(const T& src){
    T dest;
    const unsigned char* src_b = (const unsigned char*)&src;
    unsigned char* dest_b = (unsigned char*)&dest;

    dest_b[0] = src_b[7];
    dest_b[1] = src_b[6];
    dest_b[2] = src_b[5];
    dest_b[3] = src_b[4];
    dest_b[4] = src_b[3];
    dest_b[5] = src_b[2];
    dest_b[6] = src_b[1];
    dest_b[7] = src_b[0];

    return dest;
};

