#pragma once

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>

#include "vec3.h"
#include "VertexTypes.h"

class GeometryCylinder {
public:
	GeometryCylinder(double radius, int stacks) {
		InitBond(radius, stacks);
	}

	virtual ~GeometryCylinder() {

		delete[] m_bond_pos;
		delete[] m_bond_nor;
		delete[] m_bond_idx;
	}

	int GetNumVertexes() {
		return m_num_vertexes;
	}

	int GetNumIndexes() {
		return m_num_indexes;
	}


	void SetBond(const vec3f& v1, const vec3f& dv, unsigned int onecolor, unsigned int othercolor, int index_offset, VERTEX_F3F3UI1* vtx, int* indexes) {



		vec3f ax;
		vec3f ay;
		ax.Set(dv.y, -dv.x, 0.0);
		float len = ax * ax;
		if (len < 0.00001) {
			ax.Set(1.0f, 0.0f, 0.0f);
			ay.Set(0.0f, -1.0f, 0.0f);
		} else {
			ax /= sqrt(len);
			ay = Cross(dv, ax);
			len = ay*ay;
			ay /= sqrt(len);
		}


		for (int i = 0; i < m_num_vertexes; i++) {
			vtx[i].position = (ax * m_bond_pos[i].x) + (ay * m_bond_pos[i].y) + (dv * m_bond_pos[i].z) + v1;
			vtx[i].normal = ax * m_bond_nor[i].x + ay * m_bond_nor[i].y + dv * m_bond_nor[i].z;
		}


		//colorのセット	
		for (int i = 0; i < m_num_vertexes / 2; i++) {
			vtx[i].color = onecolor;

		}

		for (int i = m_num_vertexes / 2; i < m_num_vertexes; i++) {
			vtx[i].color = othercolor;

		}


		//インデックスのセット	
		for (int i = 0; i < m_num_indexes; i++) {
			indexes[i] = m_bond_idx[i] + index_offset;
		}


	}


#if 0
	void PushBond(const vec3f& v1, const vec3f& dv, unsigned int onecolor, unsigned int othercolor, vector<VERTEX_F3F3UI1>& vtx, vector<int>& indexes) {



		vec3f ax(dv.y, -dv.x, 0.0);
		vec3f ay;
		float len = ax * ax;
		if (len < 0.00001) {
			ax.Set(1.0f, 0.0f, 0.0f);
			ay.Set(0.0f, 1.0f, 0.0f);
		} else {
			ax /= sqrt(len);
			ay = Cross(dv, ax);
			len = ay*ay;
			ay /= sqrt(len);
		}
		vec3f ev(dv);
		ev.Normalize();


		const int index_offset = vtx.size();

		for (int i = 0; i < m_num_vertexes / 2; i++) {
			vtx.push_back(VERTEX_F3F3UI1((ax * m_bond_pos[i].x) + (ay * m_bond_pos[i].y) + (dv * m_bond_pos[i].z) - (ev * m_radius) + v1,
				ax * m_bond_nor[i].x + ay * m_bond_nor[i].y + dv * m_bond_nor[i].z,
				onecolor));
		}

		for (int i = m_num_vertexes / 2; i < m_num_vertexes; i++) {
			vtx.push_back(VERTEX_F3F3UI1((ax * m_bond_pos[i].x) + (ay * m_bond_pos[i].y) + (dv * m_bond_pos[i].z) + (ev * m_radius) + v1,
				ax * m_bond_nor[i].x + ay * m_bond_nor[i].y + dv * m_bond_nor[i].z,
				othercolor));
				
		}


		//インデックスのセット	
		for (int i = 0; i < m_num_indexes; i++) {
			indexes.push_back(m_bond_idx[i] + index_offset);
		}


	}
#endif

	void PushFirst(const vec3f& v1, const vec3f& v2, unsigned int onecolor, std::vector<VERTEX_F3F3UI1>& vtx, std::vector<int>& indexes) {

		vec3f dv(v2 - v1);
		vec3f ev(dv);
		ev.Normalize();

		if (ev.z > 0.f) {
			m_up.Set(0.0f, 1.0f, 0.0f);
			m_right.Set(-1.0f, 0.0f, 0.0f);
			m_front.Set(0.0f, 0.0f, 1.0f);
		} else {
			m_up.Set(0.0f, -1.0f, 0.0f);
			m_right.Set(-1.0f, 0.0f, 0.0f);
			m_front.Set(0.0f, 0.0f, -1.0f);
		}
		vec3f axis(Cross(m_front, ev));
		float cos_theta = m_front * ev;

		//Stripの折れ曲がりに対応した回転ベクトルM//
		mat33f M = RotaionMatrixOnAxis(axis, cos_theta);

		vec3f next_up = M * m_up;
		vec3f next_right = M * m_right;
		
		for (int i = 0; i < m_num_vertexes / 2; i++) {
			vtx.push_back(VERTEX_F3F3UI1((next_up * m_bond_pos[i].x) + (next_right * m_bond_pos[i].y) + v1,
				next_up * m_bond_nor[i].x + next_right * m_bond_nor[i].y,
				onecolor));
		}
		
		m_up = next_up;
		m_right = next_right;
		m_front = ev;


	}


	void PushSecond(const vec3f& v1, const vec3f& v2, unsigned int onecolor, std::vector<VERTEX_F3F3UI1>& vtx, std::vector<int>& indexes) {
		vec3f dv(v2 - v1);
		vec3f ev(dv);
		ev.Normalize();
		float cos_theta = m_front * ev;
		vec3f axis(Cross(m_front, ev));

		//Stripの折れ曲がりに対応した回転ベクトルM//
		mat33f M = RotaionMatrixOnAxis(axis, cos_theta);

		vec3f next_up = M * m_up;
		vec3f next_right = M * m_right;


		if (cos_theta > -0.5f) 
		{


			//折れ曲がった二つのCylinderの側面の交点を計算//

			const float ratio = (cos_theta > -0.5f) ? sqrt((1.0f - cos_theta) / (1.0f + cos_theta)) :
				sqrt((1.0f + cos_theta) / (1.0f - cos_theta));

			//const float ratio = sqrt((1.0f - cos_theta) / (1.0f + cos_theta));
			//axis.Normalize();
			for (int i = 0; i < m_num_vertexes / 2; i++) {
				vec3f p = m_up * m_bond_pos[i].x + m_right * m_bond_pos[i].y;
				vtx.push_back(VERTEX_F3F3UI1(Cross(axis, p) * ratio + p + v1,
					m_up * m_bond_nor[i].x + m_right * m_bond_nor[i].y,
					onecolor));
			}

			const int index_offset = vtx.size() - m_num_vertexes;
			//インデックスのセット(過去の頂点と合わせて)
			for (int i = 0; i < m_num_indexes; i++) {
				indexes.push_back(m_bond_idx[i] + index_offset);
			}

		} else {


			for (int i = 0; i < m_num_vertexes / 2; i++) {
				vec3f p = m_up * m_bond_pos[i].x + m_right * m_bond_pos[i].y;
				vtx.push_back(VERTEX_F3F3UI1(p + v1,
					m_up * m_bond_nor[i].x + m_right * m_bond_nor[i].y,
					onecolor));
			}
			{
				const int index_offset = vtx.size() - m_num_vertexes;
				//インデックスのセット(過去の頂点と合わせて)
				for (int i = 0; i < m_num_indexes; i++) {
					indexes.push_back(m_bond_idx[i] + index_offset);
				}
			}


			for (int i = 0; i < m_num_vertexes / 2; i++) {
				vec3f p = next_up * m_bond_pos[i].x + next_right * m_bond_pos[i].y;
				vtx.push_back(VERTEX_F3F3UI1(p + v1,
					next_up * m_bond_nor[i].x + next_right * m_bond_nor[i].y,
					onecolor));
			}
			{
				const int index_offset = vtx.size() - m_num_vertexes;
				//インデックスのセット(過去の頂点と合わせて)
				for (int i = 0; i < m_num_indexes; i++) {
					indexes.push_back(m_bond_idx[i] + index_offset);
				}
			}


		}


		m_up = next_up;
		m_right = next_right;
		m_front = ev;


	}


	void PushLast(const vec3f& v1, unsigned int onecolor, std::vector<VERTEX_F3F3UI1>& vtx, std::vector<int>& indexes) {
		
		float a = abs(m_up * m_right);
		float b = abs(m_front * m_right);
		float c = abs(m_front * m_up);

		for (int i = 0; i < m_num_vertexes / 2; i++) {
			vec3f p = m_up * m_bond_pos[i].x + m_right * m_bond_pos[i].y;

			vtx.push_back(VERTEX_F3F3UI1(p + v1,
				m_up * m_bond_nor[i].x + m_right * m_bond_nor[i].y,
				onecolor));
		}

		const int index_offset = vtx.size() - m_num_vertexes;
		//インデックスのセット	
		for (int i = 0; i < m_num_indexes; i++) {
			indexes.push_back(m_bond_idx[i] + index_offset);
		}
		
	}


protected:
	template <typename T>
	mat33<T> RotaionMatrixOnAxis(const vec3<T>& axis_sin_thta, T& cos_theta) {
		mat33<T> R(static_cast<T>(0.0), -axis_sin_thta.z, axis_sin_thta.y,
			axis_sin_thta.z, static_cast<T>(0.0), -axis_sin_thta.x,
			-axis_sin_thta.y, axis_sin_thta.x, static_cast<T>(0.0));
		mat33<T> M(static_cast<T>(1.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(1.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(0.0), static_cast<T>(1.0));
		M += R + ((R * R) * (static_cast<T>(1.0) / (static_cast<T>(1.0) + cos_theta)));
		return M;
	}

	void InitBond(double radius, int stacks) {
		int i, k;
		double daXY = 2.0*M_PI / double(stacks);
		double angleXY;
		float *cosXY, *sinXY;



		cosXY = new float[stacks];
		sinXY = new float[stacks];

		angleXY = daXY;
		cosXY[0] = 1.f;
		sinXY[0] = 0.f;
		for (i = 1; i < stacks; i++) {
			cosXY[i] = (float)cos(angleXY);
			sinXY[i] = (float)sin(angleXY);
			angleXY += daXY;
		}

		m_num_vertexes = stacks * 2;
		vec3f *pos = m_bond_pos = new vec3f[m_num_vertexes];
		vec3f *nor = m_bond_nor = new vec3f[m_num_vertexes];

		m_radius = (float)radius;
		for (k = 0; k < stacks; k++) {

			nor->Set(cosXY[k], sinXY[k], 0.f);
			*pos = (*nor) * m_radius;

			nor++;
			pos++;

		}

		memcpy(nor, m_bond_nor, sizeof(vec3f)*stacks);
		memcpy(pos, m_bond_pos, sizeof(vec3f)*stacks);

		for (k = 0; k < stacks; k++) {
			pos->z += 1.f;
			pos++;
		}

		delete[] cosXY;
		delete[] sinXY;


		//インデックス配列のセット
		m_num_indexes = stacks * 6;
		int* idx = m_bond_idx = new int[m_num_indexes];

		for (i = 0; i < stacks; i++) {
			*idx = i;
			idx++;
			*(idx + 3) = *idx = ((i + 1) % stacks);
			idx++;
			*(idx + 1) = *idx = i + stacks;
			idx += 3;
			*idx = ((i + 1) % stacks) + stacks;
			idx++;
		}



	};




	vec3f* m_bond_pos;
	vec3f* m_bond_nor;
	int* m_bond_idx;
	float m_radius;

	int m_num_vertexes;
	int m_num_indexes;
	
	vec3f m_up;
	vec3f m_right;
	vec3f m_front;
	vec3f m_v0;

};



class GeometryArrow : public GeometryCylinder {
public:
	GeometryArrow(double radius, int stacks) : 
		GeometryCylinder(radius, stacks){
		InitCone(radius, stacks);
	}
private:

	
	void InitCone(double radius, int stacks) {


		//InitBond(radius, stacks);
		vec3f *pos = m_bond_pos;
		for (int k = 0; k < stacks; k++) {

			pos->x *= 3.f;
			pos->y *= 3.f;
			pos->z = radius * 2.f;
			pos++;

		}

		for (int k = 0; k < stacks; k++) {
			pos->x = 0.f;
			pos->y = 0.f;
			pos->z -= radius * 2.f;
			pos++;
		}

	}
	
};
