#pragma once
#ifndef md3_reader2_h	//version 0001//
#define md3_reader2_h



#include <stdio.h>
#include <stdlib.h>
#include "vec3.h"
#include "atomic_number.h"

namespace msz {

	template <typename T>
	class MD3_Reader2 {
	private:
		FILE* m_fp;	//ファイル
		int m_num_atoms;	//粒子数.
		int m_frameno;

	public:


		MD3_Reader2() : m_fp(nullptr), m_frameno(0){}
		virtual ~MD3_Reader2() {
			Close();
		}

		///////////////////////////////////////////////////////
		//ファイルをオープンする.
		//	正しくオープンできたときは true を返す.
		/////////////////////////////////////////////////////////////
		bool Open(const char* filepath) {
			if (m_fp) {
				return false;
			}

			m_fp = fopen(filepath, "r");

			if (!m_fp) {
				return false;
			}

			//ヘッダ部だけ読み込み//
			constexpr int LINE_LEN = 1024;
			char line[LINE_LEN];
			if (fgets(line, LINE_LEN, m_fp) != NULL) {
				m_num_atoms = std::strtol(line, nullptr, 10);
				if (m_num_atoms > 0) { return true; }
			}

			return false;
		}


		///////////////////////////////////////////////////////
		//ファイルを閉じる.
		//	destructerからもこの関数が自動で呼ばれる.
		///////////////////////////////////////////////////////////
		void Close() {
			if (m_fp) { fclose(m_fp); }
			m_fp = nullptr;
		}

		////////////////////////////////////////////////////////////////////////////////
		//ファイルを読み込みして必要なデータを返す.
		//
		//	粒子数を返す.
		//	Readの後にこの関数を呼ぶと次のフレームに進める.
		//
		///////////////////////////////////////////////////////////
		//
		int GetParticleCount() {
			return m_num_atoms;
		}

	private:

		int* m_Z;
		//int* m_id;
		vec3<T>* m_r;
		T* m_rx;
		T* m_ry;
		T* m_rz;
		vec3<T>* m_p;
		T* m_px;
		T* m_py;
		T* m_pz;
		vec3<T>* m_f;
		T* m_fx;
		T* m_fy;
		T* m_fz;
		T* m_box_axis_origin;
		char* m_text;
		T* m_stress;

	public:
		////////////////////////////////////////////////////////////////////////////////
		//ファイルを読んで必要なデータを返す.
		//
		//  読み込みたいデータの種類は,BeginFrame()とEndFrame()の間で,ReadR/P/F等を呼ぶことで指定する
		//  呼ばなかったデータは読み込まない。
		//　Userの指定したバッファに実際にデータが格納されるのはEndFrame()をコールした時点.
		//
		///////////////////////////////////////////////////////////////////////////////

		void BeginFrame() {

			m_Z = nullptr;
			//m_id = nullptr;
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
			m_box_axis_origin = nullptr;
			m_text = nullptr;
			m_stress = nullptr;
		}

		void ReadZ( int* knd) {
			m_Z = knd;
		}

		void ReadR(vec3<T>* r) {
			m_r = r;
		}

		void ReadR_SoA(T* rx, T* ry, T* rz) {
			m_rx = rx;
			m_ry = ry;
			m_rz = rz;
		}

		void ReadP(vec3<T>* p) {
			m_p = p;
		}

		void ReadP_SoA(T* px, T* py, T* pz) {
			m_px = px;
			m_py = py;
			m_pz = pz;
		}

		void ReadF(vec3<T>* f) {
			m_f = f;
		}

		void ReadF_SoA(T* fx, T* fy, T* fz) {
			m_fx = fx;
			m_fy = fy;
			m_fz = fz;
		}

		void ReadBox(T* box_axis_origin) {
			m_box_axis_origin = box_axis_origin;
		}

		void ReadStress(T* stress) {
			m_stress = stress;
		}

		//////////////////////////////////////////////
		//
		//  実際の読み込みを行う
		//
		//  戻り値としてこれまでに出力したフレーム数を返す(初回は1)
		//  エラーの場合は負の値を返す
		//////////////////////////////////////////////
		int EndFrame() {
			constexpr int LINE_LEN = 1024;
			char line[LINE_LEN];

			//コメント行の読み出し//
			if (fgets(line, LINE_LEN, m_fp) == NULL) {
				m_num_atoms = 0;
				return 0;
			}

			if (m_text) {
				strcpy(m_text, line);
			}


			int count = 0;
			int next_count = -2;

			while (fgets(line, LINE_LEN, m_fp) != NULL) {

				//最初のカラムを読み込んで判定//
				//BOX or 元素記号 or 数値(次のフレームのヘッダ)//
				char* tp = strtok(line, " \t\r\n");
				if (tp == NULL) {
					break;
				}

				if (*tp == '#') {

				}else if (strcmp(tp, "BOX") == 0) {
					if (m_box_axis_origin) {
						for (int i = 0; i < 9; ++i) {
							tp = strtok(NULL, " \t\r\n");
							m_box_axis_origin[i] = strtod(tp, NULL);
						}
					}
				} else if (('0' <= *tp) && (*tp <= '9')) {
					//次の行のヘッダを読み込んだのでストップ
					next_count = std::strtol(tp, nullptr, 10);
					break;
				} else if (strcmp(tp, "STRESS") == 0) {
					//応力読み飛ばし//
					if (m_stress) {
						for (int i = 0; i < 9; ++i) {
							tp = strtok(NULL, " \t\r\n");
							m_stress[i] = strtod(tp, NULL);
						}
					}
				} else {	//read atom//

					if (count == m_num_atoms) {	//粒子がオーバーフローした//
						next_count = ERROR_OVERFLOW_PARTICLE;
						break;
					}

					if (m_Z) {
						m_Z[count] = msz::GetAtomicNumber(tp);
					}


					if (m_r) {
						tp = strtok(NULL, " \t\r\n");
						m_r[count].x = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_r[count].y = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_r[count].z = strtod(tp, NULL);
					} else if (m_rx) {
						tp = strtok(NULL, " \t\r\n");
						m_rx[count] = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_ry[count] = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_rz[count] = strtod(tp, NULL);
					}
				
					if (m_p) {
						tp = strtok(NULL, " \t\r\n");
						m_p[count].x = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_p[count].y = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_p[count].z = strtod(tp, NULL);
					} else if (m_px) {
						tp = strtok(NULL, " \t\r\n");
						m_px[count] = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_py[count] = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_pz[count] = strtod(tp, NULL);
					}else if(m_f || m_fx){
						tp = strtok(NULL, " \t\r\n");
						tp = strtok(NULL, " \t\r\n");
						tp = strtok(NULL, " \t\r\n");
					}

					if (m_f) {
						tp = strtok(NULL, " \t\r\n");
						m_f[count].x = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_f[count].y = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_f[count].z = strtod(tp, NULL);
					} else if (m_fx) {
						tp = strtok(NULL, " \t\r\n");
						m_fx[count] = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_fy[count] = strtod(tp, NULL);
						tp = strtok(NULL, " \t\r\n");
						m_fz[count] = strtod(tp, NULL);
					}

					++count;

				}


			}


			if (count < m_num_atoms) {	//error: 粒子数が足りない//
				m_num_atoms = ERROR_MISSING_PARTICLE;
				//		}else if(count > m_num_atoms){
				//			m_num_atoms = ERROR_OVERFLOW_PARTICLE;
			} else {
				m_num_atoms = next_count;
			}
			//この時点で次の行のヘッダーを読まずに抜けた場合は//
			//m_num_atoms = -1になっているはず//
			++m_frameno;
			return m_frameno;
		}



		static constexpr const int ERROR_MISSING_PARTICLE = -3;
		static constexpr const int ERROR_OVERFLOW_PARTICLE = -4;
	};



}//namespace msz//
#endif	//!md3_reader2_h//
