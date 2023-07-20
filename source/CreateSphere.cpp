#if 0
#include "CreateSphere.h"


#define _USE_MATH_DEFINES
#include <math.h>


struct VERTEX_3V2 {
	vec3f position;
	vec3f normal;
};


void CreatePolygonSphere(float radius, int slices, int stacks, vec3f** ppVertex, int* pVertexCount, WORD** ppIndex, int* pIndexCount){

	int i, k;
	double daXY = 2.0*M_PI / double(stacks);
	double daZX = M_PI / double(slices);
	double angleZX;
	double angleXY;
	float cosZX, sinZX;
	float *cosXY, *sinXY;


	//頂点データのセット.
	cosXY = new float[stacks];
	sinXY = new float[stacks];

	angleXY = daXY;
	cosXY[0] = 1.f;
	sinXY[0] = 0.f;
	for (i = 1; i < stacks; i++){
		cosXY[i] = (float)cos(angleXY);
		sinXY[i] = (float)sin(angleXY);
		angleXY += daXY;
	}

	int vertex_count = (slices - 1) * stacks + 2;
	VERTEX_3V2* ver = new VERTEX_3V2[vertex_count];
	*ppVertex = (vec3f*)ver;


	ver->position.Set(0.0f, 0.0f, (float)radius);
	ver->normal.Set(0.0f, 0.0f, 1.0f);
	ver++;

	angleZX = daZX;
	for (i = 1; i < slices; i++){
		cosZX = (float)cos(angleZX);
		sinZX = (float)sin(angleZX);

		for (k = 0; k < stacks; k++){

			ver->normal.Set(sinZX * cosXY[k], sinZX * sinXY[k], cosZX);
			ver->position = ver->normal * radius;

			ver++;
		}
		angleZX += daZX;
	}

	ver->position.Set(.0f, 0.0f, -radius);
	ver->normal.Set(0.0f, 0.0f, -1.0f);

	delete[] cosXY;
	delete[] sinXY;


	//インデックス配列のセット.
	int index_count = (slices - 1)*stacks * 6;
	WORD* idx = new WORD[index_count];
	*ppIndex = idx;
	int offset = 1;
	for (i = 0; i < stacks; i++){
		*idx = 0;
		idx++;
		*idx = i + offset;
		idx++;
		*idx = ((i + 1) % stacks) + offset;
		idx++;
	}
	//offset += stacks;


	for (k = 0; k < slices - 2; k++){
		for (i = 0; i < stacks; i++){
			*idx = i + offset;
			idx++;
			*(idx + 3) = *idx = i + offset + stacks;
			idx++;
			*(idx + 1) = *idx = ((i + 1) % stacks) + offset;
			idx += 3;
			*idx = ((i + 1) % stacks) + offset + stacks;
			idx++;
		}
		offset += stacks;
	}

	for (i = 0; i < stacks; i++){
		*idx = i + offset;
		idx++;
		*idx = vertex_count - 1;
		idx++;
		*idx = ((i + 1) % stacks) + offset;
		idx++;
	}

	//	*ppVertex = (vec3f*)ver;
	*pVertexCount = vertex_count;
	//	*ppIndex = idx;
	*pIndexCount = index_count;
};

#endif