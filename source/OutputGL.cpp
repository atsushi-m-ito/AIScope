

#include "RenderingProperty.h"
#include "RenderGL15.h"

//#include "camera.h"
#include "filelist.h"
#include "setting.h"

#include "windib2.h"
#include "FieldRGBA.h"



extern double VIEW_ANGLE;
extern double VIEW_NEAR;
extern double VIEW_FAR;



//GLで描画した内容をビットマップ画像として保存//
void OutputBMfromGLMag(std::vector<LawData>& data_list, const TCHAR *fName, int width, int height, int magnify, const double* view_matrix, const double focus_distance, const RenderingProperty& rendering_property){
	
	FILE* fp = _tfopen(fName,_T("wb"));
	if (!fp){
		printf("bmpファイルの作成に失敗しました\n");
		return;
	}
	

	
	glPixelStorei( GL_PACK_ALIGNMENT, 4);
	//データ転送を4byteずつにセット。32bitごとのが早い。//
	//余分な部分は0で埋められる。DIBも同じ仕様なのでちょうど良い//
	
	//If using double buffer, select source to read pixels.
	//By default, mode is GL_FRONT in single-buffered configurations,
	//and GL_BACK in double-buffered configurations.
	glReadBuffer(GL_BACK);

	
	int size_Color = 3;	
	int line_size = width * size_Color;
	if (line_size % 4){	line_size += 4 - (line_size % 4);}
	BYTE *bmBuf = new BYTE[line_size * height];

	int all_line_size = width * size_Color * magnify;
	if (all_line_size % 4){	all_line_size += 4 - (all_line_size % 4);}
	BYTE *allBuf = new BYTE[all_line_size * height * magnify];
	
	glLineWidth((float)magnify);

	
	int offset_y = 0;

	for (int y = 0; y < magnify; y++){
		int offset_x = 0;
		for (int x = 0; x < magnify; x++){


			RenderingGL15_2(data_list, width * magnify, height * magnify, width * x, height * y, width, height, view_matrix, focus_distance, rendering_property);

			glViewport(0, 0, width, height);

			glReadPixels(0, 0, width, height,  // 読み取りサイズ
				GL_BGR,             // 読み取り形式設定(verが低いと動かないかも)
				GL_UNSIGNED_BYTE,                  // 保存先のポインタの型 
				bmBuf                 // 保存先の配列のポインタ
			);

			for(int k = 0; k < height; k++){
				memcpy(allBuf + offset_y + offset_x + k * all_line_size,
						bmBuf + k * line_size, 	width * size_Color);
			}
					
			offset_x += width * size_Color;

		}
		offset_y += all_line_size * height;

	}

	delete [] bmBuf;
	
	writeDIB24(fp, width * magnify, height * magnify, allBuf);
	
	delete [] allBuf;
	fclose(fp);

	glLineWidth(1.0f);

}


/*
点rからベクトルv方向へ伸びる直線と
原点を通り法線nを持つ平面との交点(r+t*v)を計算する
戻り値にtを返す
*/
double GetCrossPoint(const vec3d& r, const vec3d& v, const vec3d& n){
	return -(r*n) / (v*n);

}



void OutputRaytracing(LawData& data, const TCHAR *fName, int width, int height, int magnify, const double* view_matrix, const double focus_distance){

#if 1
//void OutputRaytracing(FileInfo* fi, const TCHAR *fName, int width, int height, int magnify, CameraGL* cam)
	const int screen_w = width * magnify;
	const int screen_h = height * magnify;


	auto field = data.GetField();
	const int nx = field->grid_x;
	const int ny = field->grid_y;
	const int nz = field->grid_z;
	mat33d axis = field->boxaxis;
	vec3d org = field->boxorg;

	FieldRGBA<float> field_rgba;
	const uint8_t* buf = field_rgba.Convert(field->fieldb, field->grid_x, field->grid_y, field->grid_z);

	vec3d v0{ view_matrix[2], view_matrix[6],view_matrix[10] };
	vec3d up{ view_matrix[1], view_matrix[5],view_matrix[9] };
	//vec3d r, tgt, up;
	//cam->GetInfo(&tgt, &r, &up);
	//vec3d v0 = tgt - r;
	//double distance = std::sqrt(v0*v0);
	double distance = focus_distance;
	//vec3d vr = Cross(v0, up);
	vec3d vr{ view_matrix[0], view_matrix[4],view_matrix[8] };
	vr = Unit(vr);
	v0 *= focus_distance;
	vec3d r = -v0 ;

	double top = distance * tan(VIEW_ANGLE);
	double right = top * (double)screen_w / (double)screen_h;
				
	up *= top;
	vr *= right;

	v0 -= up + vr;

	up *= 1.0 / (double)screen_h * 2.0;
	vr *= 1.0 / (double)screen_w * 2.0;


	vec3d eea = axis.a / (axis.a * axis.a);
	vec3d eeb = axis.b / (axis.b * axis.b);
	vec3d eec = axis.c / (axis.c * axis.c);

				
	int size_Color = 3;	
	int line_size = screen_w * size_Color;
	if (line_size % 4){	line_size += 4 - (line_size % 4);}
	BYTE *bmBuf = new BYTE[line_size * screen_h];


	for(int sy = 0; sy < screen_h; sy++){

		BYTE* wBuf = bmBuf + line_size * sy;

		for(int sx = 0; sx < screen_w; sx++){

			//スクリーン画面上のピクセル(sx,sy)に関する計算//


			wBuf[0] = 0;
			wBuf[1] = 0;
			wBuf[2] = 0;
			//wBuf[3] = 0;

			BYTE maxalpha = 7;

			
			//スクリーン画面上のピクセル(sx,sy)を通過する3次元空間中のベクトル//
			vec3d v = v0 + up * ((double)sy) + vr * ((double)sx);

			
			//ベクトルvとシミュレーションボックスの6つの面との交点を求める。//
			//ボックスを貫くなら交わる面は2つあるはず//

			double cross_point[6];
			int num_cross = 0;


			//x-y面との交差//
			{
				double t = GetCrossPoint(r, v, axis.c);
				vec3d p = r + v * t;
			
				double dx = p * eea;
				double dy = p * eeb;
			
				if(dx >= 0.0){
				if(dx < 1.0){
				if(dy >= 0.0){
				if(dy < 1.0){
					cross_point[num_cross] = t;
					num_cross++;
				}}}}
			}

			//x-y面(上)との交差//
			{
				double t = GetCrossPoint(r - axis.c, v, axis.c);
				vec3d p = (r-axis.c) + v * t;
			
				double dx = p * eea;
				double dy = p * eeb;
			
				if(dx >= 0.0){
				if(dx < 1.0){
				if(dy >= 0.0){
				if(dy < 1.0){
					cross_point[num_cross] = t;
					num_cross++;
				}}}}
			}
			
			
			//y-z面との交差//
			{
				double t = GetCrossPoint(r, v, axis.a);
				vec3d p = r + v * t;
			
				double dx = p * eeb;
				double dy = p * eec;
			
				if(dx >= 0.0){
				if(dx < 1.0){
				if(dy >= 0.0){
				if(dy < 1.0){
					cross_point[num_cross] = t;
					num_cross++;
				}}}}
			}

			//y-z面(上)との交差//
			{
				double t = GetCrossPoint(r - axis.a, v, axis.a);
				vec3d p = (r-axis.a) + v * t;
			
				double dx = p * eeb;
				double dy = p * eec;
			
				if(dx >= 0.0){
				if(dx < 1.0){
				if(dy >= 0.0){
				if(dy < 1.0){
					cross_point[num_cross] = t;
					num_cross++;
				}}}}
			}

			
			//z-x面との交差//
			{
				double t = GetCrossPoint(r, v, axis.b);
				vec3d p = r + v * t;
			
				double dx = p * eec;
				double dy = p * eea;
			
				if(dx >= 0.0){
				if(dx < 1.0){
				if(dy >= 0.0){
				if(dy < 1.0){
					cross_point[num_cross] = t;
					num_cross++;
				}}}}
			}

			//z-x面(上)との交差//
			{
				double t = GetCrossPoint(r - axis.b, v, axis.b);
				vec3d p = (r-axis.b) + v * t;
			
				double dx = p * eec;
				double dy = p * eea;
			
				if(dx >= 0.0){
				if(dx < 1.0){
				if(dy >= 0.0){
				if(dy < 1.0){
					cross_point[num_cross] = t;
					num_cross++;
				}}}}
			}


			//場のデータの読み込み//
			if (num_cross == 2){

				double t1, t2;
				if(cross_point[0] < cross_point[1]){
					t1 = cross_point[0];
					t2 = cross_point[1];
				}else{
					t1 = cross_point[1];
					t2 = cross_point[0];
				}

				if(t1 < VIEW_NEAR){ t1 = VIEW_NEAR;}
				if(t2 > VIEW_FAR){ t2 = VIEW_FAR;}
				
				const double dt = distance / 64.0;
				
				//奥行き方向へ探索//
				for(double t = t1; t < t2; t+=dt){

					vec3d p = r + v * t;

					double dx = p * eea;
					double dy = p * eeb;
					double dz = p * eec;

					
					int ix = (int)(dx * (double)nx);
					int iy = (int)(dy * (double)ny);
					int iz = (int)(dz * (double)nz);

					
					int index = ix + nx * (iy + ny * iz);
					index *= 4;

					if(buf[index + 3] > maxalpha){
									
						wBuf[0] = buf[index + 2];
						wBuf[1] = buf[index + 1];
						wBuf[2] = buf[index + 0];
						//wBuf[3] = buf[index + 3];
						
						maxalpha = buf[index + 3];
						
					}
				}

			}


			wBuf += 3;

		}
	}



	FILE* fp = _tfopen(fName, _T("wb"));
	writeDIB24(fp, screen_w, screen_h, bmBuf);
	fclose(fp);
	delete [] bmBuf;
	

#endif
}

