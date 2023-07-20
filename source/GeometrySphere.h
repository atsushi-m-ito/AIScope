#pragma once

#define _USE_MATH_DEFINES
#include <math.h>

#include "vec3.h"
#include "VertexTypes.h"

class GeometrySphere {
public:
	GeometrySphere(double radius, int slices, int stacks) {
		InitSphere(radius, slices, stacks);
	}

	~GeometrySphere() {
		delete[] m_sphere_pos;
		delete[] m_sphere_nor;
		delete[] m_sphere_idx;
	}

	int GetNumVertexes() {
		return m_num_vertexes;
	}

	int GetNumIndexes() {
		return m_num_indexes;
	}

	void SetSphere(const vec3f& orgin, unsigned int onecolor, int index_offset, VERTEX_F3F3UI1* vtx, int* indexes) {

		for (int i = 0; i < m_num_vertexes; i++) {
			vtx[i].position = m_sphere_pos[i] + orgin;
			vtx[i].normal = m_sphere_nor[i];
			vtx[i].color = onecolor;
		}


		for (int i = 0; i < m_num_indexes; i++) {
			indexes[i] = m_sphere_idx[i] + index_offset;
		}

	}

	void SetSphere(const vec3f& orgin, int index_offset, VERTEX_F3F3* vtx, int* indexes) {

		for (int i = 0; i < m_num_vertexes; i++) {
			vtx[i].position = m_sphere_pos[i] + orgin;
			vtx[i].normal = m_sphere_nor[i];			
		}


		for (int i = 0; i < m_num_indexes; i++) {
			indexes[i] = m_sphere_idx[i] + index_offset;
		}

	}

	void SetSphere(const vec3f& orgin, unsigned int onecolor, int index_offset, VERTEX_F3F3UI1* vtx, int* num_vertex, int* indexes, int* num_index ) {

		for (int i = 0; i < m_num_vertexes; i++) {
			vtx[i].position = m_sphere_pos[i] + orgin;
			vtx[i].normal = m_sphere_nor[i];
			vtx[i].color = onecolor;
		}


		for (int i = 0; i < m_num_indexes; i++) {
			indexes[i] = m_sphere_idx[i] + index_offset;
		}
		
		*num_vertex += m_num_vertexes;
		*num_index += m_num_indexes;
	}

private:
	void InitSphere(double radius, int slices, int stacks) {
		
		double daXY = 2.0*M_PI / double(stacks);
		double daZX = M_PI / double(slices);
		double angleZX;
		double angleXY;
		float cosZX, sinZX;
		float *cosXY, *sinXY;



		cosXY = new float[stacks];
		sinXY = new float[stacks];

		angleXY = daXY;
		cosXY[0] = 1.f;
		sinXY[0] = 0.f;
		for (int i = 1; i < stacks; i++) {
			cosXY[i] = (float)cos(angleXY);
			sinXY[i] = (float)sin(angleXY);
			angleXY += daXY;
		}

		m_num_vertexes = (slices - 1) * stacks + 2;

		vec3f *pos = m_sphere_pos = new vec3f[m_num_vertexes];
		vec3f *nor = m_sphere_nor = new vec3f[m_num_vertexes];

		const float f_radius = (float)radius;

		nor->Set(0.0f, 0.0f, 1.0f);
		nor++;
		pos->Set(0.0f, 0.0f, f_radius);
		pos++;

		angleZX = daZX;
		for (int i = 1; i < slices; i++) {
			cosZX = (float)cos(angleZX);
			sinZX = (float)sin(angleZX);

			for (int k = 0; k < stacks; k++) {

				nor->Set(sinZX * cosXY[k], sinZX * sinXY[k], cosZX);
				*pos = (*nor) * f_radius;

				nor++;
				pos++;
			}
			angleZX += daZX;
		}

		nor->Set(0.0f, 0.0f, -1.0f);
		pos->Set(0.0f, 0.0f, -f_radius);

		delete[] cosXY;
		delete[] sinXY;


		//インデックス配列のセット
		m_num_indexes = (slices - 1)*stacks * 6;
		
		int* idx = m_sphere_idx = new int[m_num_indexes];
		int offset = 1;
		for (int i = 0; i < stacks; i++) {
			*idx = 0;
			idx++;
			*idx = i + offset;
			idx++;
			*idx = ((i + 1) % stacks) + offset;
			idx++;
		}
		//offset += stacks;


		for (int k = 0; k < slices - 2; k++) {
			for (int i = 0; i < stacks; i++) {
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

		for (int i = 0; i < stacks; i++) {
			*idx = i + offset;
			idx++;
			*idx = m_num_vertexes - 1;
			idx++;
			*idx = ((i + 1) % stacks) + offset;
			idx++;
		}

	};

	

	vec3f* m_sphere_pos;
	vec3f* m_sphere_nor;
	int* m_sphere_idx;
	int m_num_vertexes;
	int m_num_indexes;



};

template <typename T>
int CalcNumDrawOnce(T& geometry, const size_t max_vram_size) {
	return max_vram_size / ((geometry->GetNumVertexes()) * sizeof(VERTEX_F3F3UI1) + (geometry->GetNumIndexes()) * sizeof(int));
}
