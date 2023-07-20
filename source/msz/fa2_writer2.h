#pragma once
#ifndef fa2_writer2_h     	//version 0002//
#define fa2_writer2_h

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include "vec3.h"

namespace msz {

	template<typename T>
	class FA2_Writer2 {
	private:
		FILE* m_fp;
		int m_frameno;
	public:
		
		FA2_Writer2() :
			m_fp(nullptr),
			m_frameno(0)
		{

		}


		bool Open(const char* fileName) {
			if (m_fp) {
				return false;
			}

			m_fp = fopen(fileName, "wb");

			if (m_fp) {
				return true;
			} else {
				return false;
			}

		}

		bool Append(const char* fileName) {
			if (m_fp) {
				return false;
			}

			m_fp = fopen(fileName, "ab");

			if (m_fp) {
				return true;
			} else {
				return false;
			}

		}


		void Close() {
			if (m_fp) { fclose(m_fp); }
			m_fp = nullptr;
		}

	private:
		int64_t m_count;
		const int* m_Z;
		const int* m_id;
		const vec3<T>* m_r;
		const T* m_rx;
		const T* m_ry;
		const T* m_rz;
		const vec3<T>* m_p;
		const T* m_px;
		const T* m_py;
		const T* m_pz;
		const vec3<T>* m_f;
		const T* m_fx;
		const T* m_fy;
		const T* m_fz;
		float m_box_axis_origin[12];
		bool m_is_box_set;

	public:

		void BeginFrame(int count) {
			m_count = (int64_t)count;
			m_Z = nullptr;
			m_id = nullptr;
			m_r = nullptr;
			m_rx = nullptr;
			m_ry = nullptr;
			m_rz = nullptr;
			m_p = nullptr;
			m_px = nullptr;
			m_py = nullptr;
			m_pz = nullptr;
			m_f = nullptr;
			m_fx = nullptr;
			m_fy = nullptr;
			m_fz = nullptr;
			for (int i = 0; i < 12; ++i) {
				m_box_axis_origin[i] = 0.0;
			}
			m_is_box_set = false;
		}

		void WriteZ(const int* knd) {
			m_Z = knd;
		}

		void WriteID(const int* id) {
			m_id = id;
		}

		void WriteR(const vec3<T>* r) {
			m_r = r;
		}

		void WriteR_SoA(const T* rx, const T* ry, const T* rz) {
			m_rx = rx;
			m_ry = ry;
			m_rz = rz;
		}

		void WriteP(const vec3<T>* p) {
			m_p = p;
		}

		void WriteP_SoA(const T* px, const T* py, const T* pz) {
			m_px = px;
			m_py = py;
			m_pz = pz;
		}

		void WriteF(const vec3<T>* f) {
			m_f = f;
		}

		void WriteF_SoA(const T* fx, const T* fy, const T* fz) {
			m_fx = fx;
			m_fy = fy;
			m_fz = fz;
		}

		void WriteBox(const T* box_axis_origin) {
			for (int i = 0; i < 12; ++i) {
				m_box_axis_origin[i] = (float)box_axis_origin[i];
			}
			m_is_box_set = true;
		}

		int EndFrame() {

			//header////////////////////////////////////////////////////
			int8_t header[32] = { 0 };
			int64_t count64 = (int64_t)m_count;
			memcpy(header, &count64, sizeof(int64_t));

			//unit size of box//
			if (m_is_box_set) { header[8] = sizeof(float); }

			//unit size of kind//
			if (m_Z) { header[9] = sizeof(int8_t); }

			//unit size of id//
			if (m_id) { header[10] = sizeof(int); }

			//unit size of r//
			if (m_r || m_rx) { header[11] = sizeof(float); }

			//unit size of p//
			if (m_p || m_px) { header[12] = sizeof(float); }

			//unit size of f//
			if (m_f || m_fx){ header[13] = sizeof(float); }


			//memcpy(header + 16, &step, sizeof(double));

			fwrite(header, 32 * sizeof(int8_t), 1, m_fp);


			if (m_is_box_set) {
				fwrite(m_box_axis_origin, sizeof(float) * 12, 1, m_fp);
			}


			int8_t* buffer = new int8_t[m_count * sizeof(float) * 3];

			if (m_Z) {
				int8_t* pZ = buffer;
				for (int i = 0; i < m_count; i++) {
					pZ[i] = (int8_t)(m_Z[i]);
				}
				fwrite(pZ, m_count * sizeof(int8_t), 1, m_fp);
			}

			if (m_id) {
				/*
				int* pint = (int*)buffer;
				for(int i = 0; i < m_count; i++){
				pint[i] = id[i];
				}
				fwrite(pint, m_count * header[0], 1, m_fp);
				*/
				fwrite(m_id, m_count * sizeof(int), 1, m_fp);

			}


			if (m_r != nullptr) {
				float* pr = (float*)buffer;
				for (int i = 0; i < m_count; ++i) {
					pr[i * 3] = (float)(m_r[i].x);
					pr[i * 3 + 1] = (float)(m_r[i].y);
					pr[i * 3 + 2] = (float)(m_r[i].z);
				}
				fwrite(pr, m_count * sizeof(float) * 3, 1, m_fp);
			}else if (m_rx != nullptr) {
				float* pr = (float*)buffer;
				for (int i = 0; i < m_count; ++i) {
					pr[i * 3] = (float)(m_rx[i]);
					pr[i * 3 + 1] = (float)(m_ry[i]);
					pr[i * 3 + 2] = (float)(m_rz[i]);
				}
				fwrite(pr, m_count * sizeof(float) * 3, 1, m_fp);
			}

			if (m_p != nullptr) {
				float* pp = (float*)buffer;
				for (int i = 0; i < m_count; ++i) {
					pp[i * 3] = (float)(m_p[i].x);
					pp[i * 3 + 1] = (float)(m_p[i].y);
					pp[i * 3 + 2] = (float)(m_p[i].z);
				}
				fwrite(pp, m_count * sizeof(float) * 3, 1, m_fp);
			}else if (m_px != nullptr) {
				float* pp = (float*)buffer;
				for (int i = 0; i < m_count; ++i) {
					pp[i * 3] = (float)(m_px[i]);
					pp[i * 3 + 1] = (float)(m_py[i]);
					pp[i * 3 + 2] = (float)(m_pz[i]);
				}
				fwrite(pp, m_count * sizeof(float) * 3, 1, m_fp);
			}

			if (m_f != nullptr) {
				float* pp = (float*)buffer;
				for (int i = 0; i < m_count; ++i) {
					pp[i * 3] = (float)(m_f[i].x);
					pp[i * 3 + 1] = (float)(m_f[i].y);
					pp[i * 3 + 2] = (float)(m_f[i].z);
				}
				fwrite(pp, m_count * sizeof(float) * 3, 1, m_fp);
			} else if (m_fx != nullptr) {
				float* pp = (float*)buffer;
				for (int i = 0; i < m_count; ++i) {
					pp[i * 3] = (float)(m_fx[i]);
					pp[i * 3 + 1] = (float)(m_fy[i]);
					pp[i * 3 + 2] = (float)(m_fz[i]);
				}
				fwrite(pp, m_count * sizeof(float) * 3, 1, m_fp);
			}

			delete[] buffer;

			++m_frameno;
			return m_frameno;
		}



	};
}//namespace msz//
#endif	//!fa2_writer2_h//
