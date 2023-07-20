
#pragma once
#ifndef IPolyRepository_H
#define IPolyRepository_H



#include <stdio.h>
#include "targetver.h"
#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <math.h>


#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "material_info.h"

#include "IDataAtoms.h"



class IPolyRepository : public IDataContainer{




public:

    virtual ~IPolyRepository(){};


    virtual UINT GetVertexBuffer(LPVOID* ppBuffer, UINT* pStride) = 0;

    virtual UINT GetIndexBuffer(LPVOID* ppBuffer, UINT* pStride) = 0;

    virtual int GetMaterialList(MATERIAL_INFO** pmaterials) = 0;

    virtual void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg) = 0;

    virtual int  SetFrameNum(int time_frame) = 0;
    
    virtual void ExchangeYZ() = 0;

};


#endif    


