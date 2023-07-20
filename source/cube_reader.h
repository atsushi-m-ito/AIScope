#pragma once
#include <tchar.h>
#include <stdio.h>
#include <vec3.h>
#include <mat33.h>
#include <charconv>
#include "ATOMS_DATA.h"

class CubeReader {
public:
	CubeReader() = default;
	~CubeReader() = default;

	struct Frame {
		int grid_x;//グリッド数
		int grid_y;
		int grid_z;
		mat33d boxaxis;
		vec3d boxorg;
	};

#if 1
	float* LoadCube(const TCHAR* filename, Frame* frame, ATOMS_DATA* dat) {

		FILE* fp = _tfopen(filename, _T("r"));

		const size_t SIZE = 1024;
		char line[SIZE];

		if (fgets(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//タイトル
		if (fgets(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//コメント

		if (fgets(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//原子数とvolumeデータの原点座標
		int pcnt;
		sscanf(line, "%d %lf %lf %lf", &pcnt, &(frame->boxorg.x), &(frame->boxorg.y), &(frame->boxorg.z));

		if (fgets(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//x方向のメッシュ数とx軸
		sscanf(line, "%d %lf %lf %lf", &(frame->grid_x), &(frame->boxaxis.a.x), &(frame->boxaxis.a.y), &(frame->boxaxis.a.z));

		if (fgets(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//y方向のメッシュ数とx軸
		sscanf(line, "%d %lf %lf %lf", &(frame->grid_y), &(frame->boxaxis.b.x), &(frame->boxaxis.b.y), &(frame->boxaxis.b.z));

		if (fgets(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//z方向のメッシュ数とx軸
		sscanf(line, "%d %lf %lf %lf", &(frame->grid_z), &(frame->boxaxis.c.x), &(frame->boxaxis.c.y), &(frame->boxaxis.c.z));

		frame->boxorg -= (frame->boxaxis.a + frame->boxaxis.b + frame->boxaxis.c) / 2.0;
		frame->boxaxis.a *= (double)frame->grid_x;
		frame->boxaxis.b *= (double)frame->grid_y;
		frame->boxaxis.c *= (double)frame->grid_z;

		if (dat) {
			dat->boxaxis = frame->boxaxis;
			dat->boxorg = frame->boxorg;
			dat->pcnt = pcnt;
			dat->knd = new int[pcnt];
			dat->r = new vec3f[pcnt];
			for (int i = 0; i < pcnt; i++) {
				float dummy;
				if (fgets(line, 1024, fp) == NULL) { fclose(fp); return nullptr; }	//原子位置読み込み
				sscanf(line, "%d %f %f %f %f", &(dat->knd[i]), &dummy, &(dat->r[i].x), &(dat->r[i].y), &(dat->r[i].z));
				/*dat->r[i].x += dat->boxorg.x;
				dat->r[i].y += dat->boxorg.y;
				dat->r[i].z += dat->boxorg.z;*/
			}
		} else {
			for (int i = 0; i < pcnt; i++) {
				if (fgets(line, 1024, fp) == NULL) { fclose(fp); return nullptr; }	//原子位置読み込み				
			}
		}

		//ボリュームデータの読み込み
		size_t mesh_sz = (frame->grid_x) * (frame->grid_y) * (frame->grid_z);
		float* values = new float[mesh_sz];
		char* tp;
		float maxvalue = 0.0;
		float minvalue = 1E10;
		size_t ix = 0;
		size_t iy = 0;
		size_t iz = 0;
		const size_t iz_end = (frame->grid_z);
		const char* const endptr = line + SIZE - 1;
		while (fgets(line, SIZE, fp) != NULL) {
			
			const char* tp = strtok(line, " \t\r\n");
			while (tp){
				float vp{};
				auto res = std::from_chars(tp, endptr, vp);
				if (res.ec != std::errc{}) break;
				tp = res.ptr;

				size_t index = ix + (frame->grid_x) * (iy + (frame->grid_y) * iz);
				values[index] = vp;


				//最大値の取得
				if (maxvalue < (vp)) { maxvalue = vp; }
				if (minvalue > (vp) && (vp > 0.0)) { minvalue = vp; }
				//tp = nptr;
				tp = strtok(nullptr, " \t\r\n");
				//vp++;

				++iz;
				if (iz == iz_end) {
					iz = 0;
					++iy;
					break;
				}

				
				//res = std::from_chars(tp, endptr, vp);
			}

			if (iy == (frame->grid_y)) {
				iy = 0;
				++ix;
				if (ix == (frame->grid_x)) {
					break;
				}

			}
		}

		fclose(fp);


		return values;

	}
#elif 1
	float* LoadCube(const TCHAR* filename, Frame* frame, ATOMS_DATA* dat){
	
		FILE* fp = _tfopen(filename, _T("r"));
	
		
		char line[1024];
	
		if(fgets(line, 1024, fp) == NULL){ fclose(fp);return NULL;}	//タイトル
		if(fgets(line, 1024, fp) == NULL){ fclose(fp);return NULL;}	//コメント

		if(fgets(line, 1024, fp) == NULL){ fclose(fp);return NULL;}	//原子数とvolumeデータの原点座標
		int pcnt;
		sscanf(line, "%d %lf %lf %lf", &pcnt, &(frame->boxorg.x), &(frame->boxorg.y), &(frame->boxorg.z));

		if(fgets(line, 1024, fp) == NULL){ fclose(fp);return NULL;}	//x方向のメッシュ数とx軸
		sscanf(line, "%d %lf %lf %lf", &(frame->grid_x), &(frame->boxaxis.a.x), &(frame->boxaxis.a.y), &(frame->boxaxis.a.z));

		if(fgets(line, 1024, fp) == NULL){ fclose(fp);return NULL;}	//y方向のメッシュ数とx軸
		sscanf(line, "%d %lf %lf %lf", &(frame->grid_y), &(frame->boxaxis.b.x), &(frame->boxaxis.b.y), &(frame->boxaxis.b.z));

		if(fgets(line, 1024, fp) == NULL){ fclose(fp);return NULL;}	//z方向のメッシュ数とx軸
		sscanf(line, "%d %lf %lf %lf", &(frame->grid_z), &(frame->boxaxis.c.x), &(frame->boxaxis.c.y), &(frame->boxaxis.c.z));

		frame->boxorg -= (frame->boxaxis.a + frame->boxaxis.b + frame->boxaxis.c) / 2.0;
		frame->boxaxis.a *= (double)frame->grid_x;
		frame->boxaxis.b *= (double)frame->grid_y;
		frame->boxaxis.c *= (double)frame->grid_z;

		if (dat) {
			dat->boxaxis = frame->boxaxis;
			dat->boxorg = frame->boxorg;
			dat->pcnt = pcnt;
			dat->knd = new int[pcnt];
			dat->r = new vec3f[pcnt];
			for (int i = 0; i < pcnt; i++) {
				float dummy;
				if (fgets(line, 1024, fp) == NULL) { fclose(fp); return nullptr; }	//原子位置読み込み
				sscanf(line, "%d %f %f %f %f", &(dat->knd[i]), &dummy, &(dat->r[i].x), &(dat->r[i].y), &(dat->r[i].z));
				/*dat->r[i].x += dat->boxorg.x;
				dat->r[i].y += dat->boxorg.y;
				dat->r[i].z += dat->boxorg.z;*/
			}
		}else{
			for (int i = 0; i < pcnt; i++) {
				if (fgets(line, 1024, fp) == NULL) { fclose(fp); return nullptr; }	//原子位置読み込み				
			}
		}

		//ボリュームデータの読み込み
		size_t mesh_sz = (frame->grid_x) * (frame->grid_y) * (frame->grid_z);
		float* values = new float[mesh_sz];
		char* tp;
		float maxvalue = 0.0;
		float minvalue = 1E10;
		size_t ix = 0;
		size_t iy = 0;
		size_t iz = 0;
		const size_t iz_end = (frame->grid_z);
		while(fgets(line, 1024, fp) != NULL){
			tp = strtok(line, " \t\r\n");
			while (tp) {
				char* nptr;
				float vp = strtof(tp, &nptr);
			
				size_t index = ix + (frame->grid_x) * (iy + (frame->grid_y) * iz);
				values[index] = vp;
			

				//最大値の取得
				if ( maxvalue < (vp)){ maxvalue = vp;}
				if ( minvalue > (vp) && (vp > 0.0)){ minvalue = vp;}
				//tp = nptr;
				tp = strtok(nullptr, " \t\r\n");
				//vp++;

				++iz;
				if (iz == iz_end) {
					iz = 0;
					++iy;
					break;
				}
			}
		
			if (iy == (frame->grid_y)) {
				iy = 0;
				++ix;
				if (ix == (frame->grid_x)) {
					break;
				}
			
			}
		}
	
		fclose(fp);
	
	
		return values;

	}
#else
	float* LoadCube(const TCHAR* filename, Frame* frame, ATOMS_DATA* dat) {

		FILE* fp = _tfopen(filename, _T("r"));


		TCHAR line[1024];

		if (_fgetts(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//タイトル
		if (_fgetts(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//コメント

		if (_fgetts(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//原子数とvolumeデータの原点座標
		int pcnt;
		_stscanf(line, _T("%d %lf %lf %lf"), &pcnt, &(frame->boxorg.x), &(frame->boxorg.y), &(frame->boxorg.z));

		if (_fgetts(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//x方向のメッシュ数とx軸
		_stscanf(line, _T("%d %lf %lf %lf"), &(frame->grid_x), &(frame->boxaxis.a.x), &(frame->boxaxis.a.y), &(frame->boxaxis.a.z));

		if (_fgetts(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//y方向のメッシュ数とx軸
		_stscanf(line, _T("%d %lf %lf %lf"), &(frame->grid_y), &(frame->boxaxis.b.x), &(frame->boxaxis.b.y), &(frame->boxaxis.b.z));

		if (_fgetts(line, 1024, fp) == NULL) { fclose(fp); return NULL; }	//z方向のメッシュ数とx軸
		_stscanf(line, _T("%d %lf %lf %lf"), &(frame->grid_z), &(frame->boxaxis.c.x), &(frame->boxaxis.c.y), &(frame->boxaxis.c.z));

		frame->boxorg -= (frame->boxaxis.a + frame->boxaxis.b + frame->boxaxis.c) / 2.0;
		frame->boxaxis.a *= (double)frame->grid_x;
		frame->boxaxis.b *= (double)frame->grid_y;
		frame->boxaxis.c *= (double)frame->grid_z;

		if (dat) {
			dat->boxaxis = frame->boxaxis;
			dat->boxorg = frame->boxorg;
			dat->pcnt = pcnt;
			dat->knd = new int[pcnt];
			dat->r = new vec3f[pcnt];
			for (int i = 0; i < pcnt; i++) {
				float dummy;
				if (_fgetts(line, 1024, fp) == NULL) { fclose(fp); return nullptr; }	//原子位置読み込み
				_stscanf(line, _T("%d %f %f %f %f"), &(dat->knd[i]), &dummy, &(dat->r[i].x), &(dat->r[i].y), &(dat->r[i].z));
				/*dat->r[i].x += dat->boxorg.x;
				dat->r[i].y += dat->boxorg.y;
				dat->r[i].z += dat->boxorg.z;*/
			}
		} else {
			for (int i = 0; i < pcnt; i++) {
				if (_fgetts(line, 1024, fp) == NULL) { fclose(fp); return nullptr; }	//原子位置読み込み				
			}
		}

		//ボリュームデータの読み込み
		size_t mesh_sz = (frame->grid_x) * (frame->grid_y) * (frame->grid_z);
		float* values = new float[mesh_sz];
		TCHAR* tp;
		float maxvalue = 0.0;
		float minvalue = 1E10;
		size_t ix = 0;
		size_t iy = 0;
		size_t iz = 0;
		const size_t iz_end = (frame->grid_z);
		while (_fgetts(line, 1024, fp) != NULL) {
			tp = _tcstok(line, _T(" \t\r\n"));
			while (tp) {
				TCHAR* nptr;
				float vp = _tcstof(tp, &nptr);

				size_t index = ix + (frame->grid_x) * (iy + (frame->grid_y) * iz);
				values[index] = vp;


				//最大値の取得
				if (maxvalue < (vp)) { maxvalue = vp; }
				if (minvalue > (vp) && (vp > 0.0)) { minvalue = vp; }
				//tp = nptr;
				tp = _tcstok(nullptr, _T(" \t\r\n"));
				//vp++;

				++iz;
				if (iz == iz_end) {
					iz = 0;
					++iy;
					break;
				}
			}

			if (iy == (frame->grid_y)) {
				iy = 0;
				++ix;
				if (ix == (frame->grid_x)) {
					break;
				}

			}
		}

		fclose(fp);


		return values;

}
#endif

};
