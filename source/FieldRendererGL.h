
#ifndef FIELDRENDERERGL_H
#define FIELDRENDERERGL_H



#pragma once


#include <stdio.h>
#include <tchar.h>
#include <string.h>

#include "sdkwgl.h"

#include "vec3.h"
#include "mat33.h"

#include "debugprint.h"


class FieldRendererGL{
private:
	vec3d* m_box_vtx;
	vec3d* m_box_crd;

	//int m_grid_x;//グリッド数
	//int m_grid_y;
	//int m_grid_z;

	//int m_depth_resolution;

	//mat33d m_boxaxis;
	//vec3d m_boxorg;

	GLubyte* m_bfield_prev;

	GLuint m_textureName;
	//GLubyte* m_texture;
	double CrossPoint(vec3d* org, vec3d* v, vec3d* a, vec3d* b);

	void SpectrumColor(double value, BYTE *r, BYTE* g, BYTE * b);

public:
	
	FieldRendererGL();
	virtual ~FieldRendererGL();
	
	//void TestData();
	//int Load(const TCHAR* filename);
	void Create3DTexture(GLubyte* bfield, int grid_x, int grid_y, int grid_z);
	void CreateSideVector(mat33d* boxaxis, vec3d* boxorg);
	void Draw(GLubyte* bfield, int grid_x, int grid_y, int grid_z, mat33d* boxaxis, vec3d* boxorg, vec3d* target, vec3d* camera);
	void SetBoundary();
	void SetBinarySection(vec3d* camdir, vec3d* org, mat33d* boxaxis);

//	void GetBoxaxis(mat33f* boxaxis, vec3f* boxorg);

	
};





#endif //FIELDRENDERERGL_H