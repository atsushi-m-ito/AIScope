#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>

#include "vec3.h"
//#include "atomic_number.h"
#include "GeometrySphere.h"


class STL_Writer
{
	
private:
	FILE* m_fp{ nullptr };

public:
	virtual ~STL_Writer() {
		Close();
	}

	bool Open(const char* file_path) {
		m_fp = fopen(file_path, "wb");
		return ((!m_fp) ? false : true);		
	}
	
	void Close() {
		if(m_fp) fclose(m_fp);
	}

	template<typename T>
	int Write(const vec3<T>* r, int count, double radius, int poly) {
		GeometrySphere geometry_sphere(radius, poly, poly * 2);

		constexpr const char HEADER[80]{ "STL object file created by AIScope" };
		fwrite(HEADER, 80, 1, m_fp);

		const int num_indexes = geometry_sphere.GetNumIndexes();
		const int num_vertexes = geometry_sphere.GetNumVertexes();
		int* indexes = new int[num_indexes];
		VERTEX_F3F3* vertexes = new VERTEX_F3F3[num_vertexes];

		//number of polygon
		const uint32_t num_polygon  = (num_indexes / 3) * count;
		fwrite(&num_polygon, 4, 1, m_fp);


		for (int i = 0; i < count; ++i) {
			const vec3f rf{ r[i].x, r[i].y, r[i].z };
			geometry_sphere.SetSphere(rf, 0, vertexes, indexes);
			for (int k = 0; k < num_indexes; k += 3) {
				const vec3f normal = vertexes[indexes[k]].normal + vertexes[indexes[k+1]].normal + vertexes[indexes[k+2]].normal;
				fwrite(&normal, 12, 1, m_fp);
				fwrite(&(vertexes[indexes[k]].position), 12, 1, m_fp);
				fwrite(&(vertexes[indexes[k+1]].position), 12, 1, m_fp);
				fwrite(&(vertexes[indexes[k+2]].position), 12, 1, m_fp);
				const uint16_t dummy{ 0 };
				fwrite(&dummy, 2, 1, m_fp);
			}
		}
		delete[] indexes;
		delete[] vertexes;
		return count;
	}
};

