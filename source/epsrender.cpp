
#include "epsrender.h"
#include <float.h>
#include <math.h>
#include "vec3.h"
#include "setting.h"

#pragma warning(disable : 4996)

#define DRAWEPS_WIDTH	200



void QuickSort(double *array, int lngStart, int lngEnd, int *attendant);
void QuickSort(float *array, int lngStart, int lngEnd, int *attendant);
double calcRadius(double dx, double dy, double dz){
	return sqrt(dx*dx + dy*dy + dz*dz);
}

int** new_int2D(int x, int y){
	int **a;
	int i;

	a = new int*[x];
	for (i = 0; i < x; i++){
		a[i] = new int[y];
	}
	
	return a;
}

void delete_int2D(int **a, int x){
	int i;
	
	for (i = 0; i < x; i++){
		delete [] a[i];
	}
	delete [] a;
	
}


void DrawEpsCircle(FILE* fp,double x, double y, double r, double red, double green, double blue){
	
	outEPS_gsave(fp);
	outEPS_pathCircle(fp,x,y,r);
	outEPS_setrgbcolor(fp,red,green,blue);
	outEPS_fill(fp);

	outEPS_pathCircle(fp,x,y,r);
	outEPS_setrgbcolor(fp, 0.0, 0.0, 0.0);
	outEPS_stroke(fp);
	outEPS_grestore(fp);

}

void DrawEpsLine(FILE* fp,double x1, double y1, double x2, double y2, double red, double green, double blue){
	
	outEPS_gsave(fp);
	outEPS_moveto(fp, x1, y1);
	outEPS_lineto(fp, x2, y2);
	outEPS_setrgbcolor(fp,red,green,blue);
	outEPS_stroke(fp);
	outEPS_grestore(fp);

}

FILE* OpenAtomEPS(const TCHAR* filename, int w, int h){


	FILE* fp = _tfopen(filename, _T("w"));
	if(! fp){
		return NULL;
	}

	outEPS_header(fp, 0, 0, DRAWEPS_WIDTH, (DRAWEPS_WIDTH*h)/w);
	

	return fp;
}

void CloseAtomEPS(FILE* fp){


	outEPS_footer(fp);

	fprintf(fp, "test\n");
	fclose(fp);
}

void DrawAtomEPS(FILE* fp, ATOMS_DATA *dat, int w, int h){
	
	
	
	
	const float atomc[] = { 0.2f, 0.8f, 0.8f, 1.0f,	//C_custom
						1.0f, 1.0f, 1.0f, 1.0f,	//H
						1.0f, 0.08f, 0.59f, 1.0f, //He
						1.0f, 0.08f, 0.59f, 1.0f, //Li
						1.0f, 0.08f, 0.59f, 1.0f, //Be
						1.0f, 0.08f, 0.59f, 1.0f, //B
						0.47f, 0.47f, 0.47f, 1.0f, //C
						0.56f, 0.56f, 1.0f, 1.0f, //N
						1.0f, 0.0f, 0.0f, 1.0f, //O
						1.0f, 0.08f, 0.59f, 1.0f, //F
						1.0f, 0.08f, 0.59f, 1.0f, //Ne
						0.0f, 0.0f, 1.0f, 1.0f, //Na
						1.0f, 0.08f, 0.59f, 1.0f, //Mg
						1.0f, 0.08f, 0.59f, 1.0f, //Al
						1.0f, 0.08f, 0.59f, 1.0f, //Si
						1.0f, 0.5f, 0.0f, 1.0f, //P
						1.0f, 1.0f, 0.2f, 1.0f, //S
						0.0f, 1.0f, 0.0f, 1.0f, //Cl
						1.0f, 0.08f, 0.59f, 1.0f, //Ar
						1.0f, 0.08f, 0.59f, 1.0f, //K
						0.2f, 0.3f, 0.3f, 1.0f, //Ca
						1.0f, 0.08f, 0.59f, 1.0f, //the others						
						};
	if (dat == NULL) return;

	int* knd = dat->knd;
	vec3f* r = dat->r;
	

	double hw = (double)DRAWEPS_WIDTH / 2.0;
	double hh = hw * (double)h/(double)w;
	

	
//座標変換行列の取得
	float model[16], proj[16],mpmat[16];
	
	glGetFloatv(GL_MODELVIEW_MATRIX, model);
	glGetFloatv(GL_PROJECTION_MATRIX, proj);
	
	int i, k, x;
	for (i=0;i<4;i++){
		for (k=0;k<4;k++){
			mpmat[i+k*4] = 0.0;
			for (x=0;x<4;x++){
				mpmat[i+k*4] += proj[i+x*4] * model[x + k*4];
			}
		}
	}


//sim.ボックス頂点の計算.
	vec3f* boxv = new vec3f[8];
	boxv[0] = dat->boxorg;
	boxv[1] = dat->boxorg + dat->boxaxis.a;
	boxv[2] = dat->boxorg + dat->boxaxis.b;
	boxv[3] = dat->boxorg + dat->boxaxis.a + dat->boxaxis.b;
	boxv[4] = dat->boxorg + dat->boxaxis.c;
	boxv[5] = dat->boxorg + dat->boxaxis.a + dat->boxaxis.c;
	boxv[6] = dat->boxorg + dat->boxaxis.b + dat->boxaxis.c;
	boxv[7] = dat->boxorg + dat->boxaxis.a + dat->boxaxis.b + dat->boxaxis.c;

	//二次元座標へ変換.
	i = 0;
	int min_i;
	float depth = -FLT_MAX;
	float depth_w0;
	for(vec3f* v=boxv; v != boxv + 8; v++){
		
		vec3f xyz0;
		xyz0.x = mpmat[0] * (v->x);
		xyz0.x += mpmat[4] * (v->y);
		xyz0.x += mpmat[8] * (v->z);
		xyz0.x += mpmat[12]; 
		
		xyz0.y = mpmat[1] * (v->x);
		xyz0.y += mpmat[5] * (v->y);
		xyz0.y += mpmat[9] * (v->z);
		xyz0.y += mpmat[13];
		
		xyz0.z = mpmat[2] * (v->x);
		xyz0.z += mpmat[6] * (v->y);
		xyz0.z += mpmat[10] * (v->z);
		xyz0.z += mpmat[14];

		float w0 = mpmat[3] * (v->x);
		w0 += mpmat[7] * (v->y);
		w0 += mpmat[11] * (v->z);
		w0 += mpmat[15];
		
		*v = xyz0 / w0;
		
		if( xyz0.z > depth){
			depth = xyz0.z;
			depth_w0 = w0;
			min_i = i;
		}
		i++;

	}
	//boxの最も奥の頂点を含む辺を描画.
	outEPS_setlinewidth(fp, g_epsBoxLineSize /depth_w0 * hw);

	DrawEpsLine(fp,	(boxv[min_i].x + 1.0) * hw, (boxv[min_i].y + 1.0) * hh,
					(boxv[min_i ^ 0x1].x + 1.) * hw, (boxv[min_i ^ 0x1].y + 1.) * hh,
					0.0, 0.0, 0.0);

	DrawEpsLine(fp,	(boxv[min_i].x + 1.0) * hw, (boxv[min_i].y + 1.0) * hh,
					(boxv[min_i ^ 0x2].x + 1.) * hw, (boxv[min_i ^ 0x2].y + 1.) * hh,
					0.0, 0.0, 0.0);

	DrawEpsLine(fp,	(boxv[min_i].x + 1.0) * hw, (boxv[min_i].y + 1.0) * hh,
					(boxv[min_i ^ 0x4].x + 1.) * hw, (boxv[min_i ^ 0x4].y + 1.) * hh,
					0.0, 0.0, 0.0);


	//粒子の描画.
	float x0,y0,z0,w0;
	
	float *epsx = new float[dat->pcnt];
	float *epsy = new float[dat->pcnt];
	float *epsz = new float[dat->pcnt];
	float *epsw = new float[dat->pcnt];
	int *indexes = new int[dat->pcnt];
	
	//座標変換
	for(i=0;i<dat->pcnt;i++){
		
		
		x0 = mpmat[0] * r[i].x;
		x0 += mpmat[4] * r[i].y;
		x0 += mpmat[8] * r[i].z;
		x0 += mpmat[12]; 
		
		y0 = mpmat[1] * r[i].x;
		y0 += mpmat[5] * r[i].y;
		y0 += mpmat[9] * r[i].z;
		y0 += mpmat[13];
		
		z0 = mpmat[2] * r[i].x;
		z0 += mpmat[6] * r[i].y;
		z0 += mpmat[10] * r[i].z;
		z0 += mpmat[14];

		w0 = mpmat[3] * r[i].x;
		w0 += mpmat[7] * r[i].y;
		w0 += mpmat[11] * r[i].z;
		w0 += mpmat[15];
		
		epsx[i] = x0/w0;
		epsy[i] = y0/w0;
		epsz[i] = z0/w0;
		epsw[i] = w0;
		indexes[i] = i;

	}

	//ボンドを先に全部書いてしまう方法は
	//粒子から手前に伸びるものも粒子の後ろに隠れてしまうので却下

	//以下ではボンドペアの粒子うちの奥に位置する粒子を判別し
	//その粒子とを描画した直後にボンドも描画する
	

	int **NbList = new_int2D(dat->pcnt, 12);	//最大の配位数
	int *NbCnt = new int[dat->pcnt];
	int ip;	//index of back particle
	int ifront;	//index of front particle

	memset(NbCnt, 0, sizeof(int) * dat->pcnt);

/*	
	for (SIMPLE_IJ* b = dat->bonds; b != dat->bonds + dat->bondInsideCnt;b++){
	
		if(epsz[b->i] > epsz[b->j]){		//liが奥
			ip = b->i;
			NbList[ip][NbCnt[ip]] = b->j;
			NbCnt[ip] ++;
		}else{
			ip = b->j;
			NbList[ip][NbCnt[ip]] = b->i;
			NbCnt[ip] ++;
		}
		
	}
*/

//粒子とボンドのレンダリング---------------------------
	//z orderでクイックソート
	float *sortz = new float[dat->pcnt];
	memcpy(sortz, epsz, sizeof(float)*dat->pcnt);
	QuickSort(sortz, 0, dat->pcnt-1, indexes);
	delete [] sortz;


	

	//粒子の描画
	ip = 0;
	for(i=dat->pcnt-1;i>=0;i--){
		//double epsr= 1.0/epsw[ip];	//粒子半径、さじ加減で
		double epsr= g_epsAtomSize / epsw[ip];
		ip = indexes[i];

		//描画領域に入っているか判定
		if(fabs(epsx[ip]) > 1.0f){ continue;}
		if(fabs(epsy[ip]) > 1.0f){ continue;}

		
		//outEPS_setlinewidth(fp, 10.0/epsw[ip]);	//さじ加減で
		outEPS_setlinewidth(fp, 0.2/epsw[ip] * hw);
		DrawEpsCircle(fp, (epsx[ip] + 1.) * hw, (epsy[ip] + 1.) * hh,
				epsr*hw,
				atomc[knd[ip]*4], atomc[knd[ip]*4+1], atomc[knd[ip]*4+2]);

		//粒子から手前方向に伸びるボンドの描画
		outEPS_setlinewidth(fp, g_epsBondSize /epsw[ip] * hw);
		//outEPS_setlinewidth(fp, 30.0/epsw[ip]);
		for(k=0; k< NbCnt[ip];k++){
			ifront = NbList[ip][k];
			double dx = epsx[ifront] - epsx[ip];
			double dy = epsy[ifront] - epsy[ip];
			double dr3d = calcRadius(dx, dy, epsz[ifront]-epsz[ip]);
			//double dr3d = calcRadius(rz[ifront] - rx[ip], ry[ifront] - ry[ip], rz[ifront]-rz[ip]);
			//double dr2d = dx*dx + dy*dy;
			//dr2d = sqrt(dr2d);
			dx *= 0.5f * epsr / dr3d;
			dy *= 0.5f * epsr / dr3d;

			DrawEpsLine(fp, //epsx[ip] * EPS_CENTERX + EPS_CENTERX, epsy[ip] * EPS_CENTERY + EPS_CENTERY,
					(epsx[ip] + dx + 1.0) * hw, (epsy[ip] + dy + 1.0) * hh,
					(epsx[ifront] + 1.) * hw, (epsy[ifront] + 1.) * hh,
					0.2, 0.6, 0.6);
					//atomc[knd[ip]*4], atomc[knd[ip]*4+1], atomc[knd[ip]*4+2]);
		}


	}

	delete_int2D(NbList, dat->pcnt);
	delete [] NbCnt;
	delete [] epsx;
	delete [] epsy;
	delete [] epsz;
	delete [] epsw;
	delete [] indexes;

	
	//boxの最も奥の頂点以外で作られる辺を描画.
	outEPS_setlinewidth(fp, g_epsBoxLineSize /depth_w0 * hw);
	
	for (i = 0; i < 3; i++){
		//最奥以外の三つの頂点から三本づつ辺をとればいい.
		switch( i){
		case 0:	k = min_i ^ 0x3; break;
		case 1:	k = min_i ^ 0x5; break;
		case 2:	k = min_i ^ 0x6; break;
		}

		DrawEpsLine(fp,	(boxv[k].x + 1.0) * hw, (boxv[k].y + 1.0) * hh,
					(boxv[k ^ 0x1].x + 1.) * hw, (boxv[k ^ 0x1].y + 1.) * hh,
					0.0, 0.0, 0.0);

		DrawEpsLine(fp,	(boxv[k].x + 1.0) * hw, (boxv[k].y + 1.0) * hh,
					(boxv[k ^ 0x2].x + 1.) * hw, (boxv[k ^ 0x2].y + 1.) * hh,
					0.0, 0.0, 0.0);

		DrawEpsLine(fp,	(boxv[k].x + 1.0) * hw, (boxv[k].y + 1.0) * hh,
					(boxv[k ^ 0x4].x + 1.) * hw, (boxv[k ^ 0x4].y + 1.) * hh,
					0.0, 0.0, 0.0);
	}
	delete [] boxv;
}



void QuickSort(double *array, int lngStart, int lngEnd, int *attendant){

	int lngBaseNumber;								//中央の要素番号を格納する変数
	double base;									//基準値を格納する変数
	int lngCounter;                                 //格納位置カウンタ
	double lngBuffer;                               //値をスワップするための作業域
	int i;                                          //ループカウンタ
	int res, baseattend;

    if (lngStart >= lngEnd) {return;}				//終了番号が開始番号以下の場合、プロシージャを抜ける
    lngBaseNumber = (lngStart + lngEnd) / 2;        //中央の要素番号を求める
    base = array[lngBaseNumber];					//中央の値を基準値とする
		baseattend = attendant[lngBaseNumber];
    array[lngBaseNumber] = array[lngStart];         //中央の要素に開始番号の値を格納
		attendant[lngBaseNumber] = attendant[lngStart];
    lngCounter = lngStart;                          //格納位置カウンタを開始番号と同じにする
    for( i = lngStart + 1; i <= lngEnd; i++){		//開始番号の次の要素から終了番号までループ
        if (array[i] < base){						//値が基準値より小さい場合
            lngCounter = lngCounter + 1;            //格納位置カウンタをインクリメント
            
			lngBuffer = array[lngCounter];          //array(i) と array(lngCounter) の値をスワップ
            array[lngCounter] = array[i];
            array[i] = lngBuffer;

			res = attendant[lngCounter];
			attendant[lngCounter] = attendant[i];
			attendant[i] = res;

        }
    }

    array[lngStart] = array[lngCounter];             //array(lngCounter) を開始番号の値にする
		attendant[lngStart] = attendant[lngCounter];
    array[lngCounter] = base;						//基準値を array(lngCounter) に格納
		attendant[lngCounter] = baseattend;
    QuickSort(array, lngStart, lngCounter - 1, attendant);		//分割された配列をクイックソート(再帰)
    QuickSort(array, lngCounter + 1, lngEnd, attendant);		//分割された配列をクイックソート(再帰)

}


void QuickSort(float *array, int lngStart, int lngEnd, int *attendant){

	int lngBaseNumber;								//中央の要素番号を格納する変数
	float base;									//基準値を格納する変数
	int lngCounter;                                 //格納位置カウンタ
	float lngBuffer;                               //値をスワップするための作業域
	int i;                                          //ループカウンタ
	int res, baseattend;

    if (lngStart >= lngEnd) {return;}				//終了番号が開始番号以下の場合、プロシージャを抜ける
    lngBaseNumber = (lngStart + lngEnd) / 2;        //中央の要素番号を求める
    base = array[lngBaseNumber];					//中央の値を基準値とする
		baseattend = attendant[lngBaseNumber];
    array[lngBaseNumber] = array[lngStart];         //中央の要素に開始番号の値を格納
		attendant[lngBaseNumber] = attendant[lngStart];
    lngCounter = lngStart;                          //格納位置カウンタを開始番号と同じにする
    for( i = lngStart + 1; i <= lngEnd; i++){		//開始番号の次の要素から終了番号までループ
        if (array[i] < base){						//値が基準値より小さい場合
            lngCounter = lngCounter + 1;            //格納位置カウンタをインクリメント
            
			lngBuffer = array[lngCounter];          //array(i) と array(lngCounter) の値をスワップ
            array[lngCounter] = array[i];
            array[i] = lngBuffer;

			res = attendant[lngCounter];
			attendant[lngCounter] = attendant[i];
			attendant[i] = res;

        }
    }

    array[lngStart] = array[lngCounter];             //array(lngCounter) を開始番号の値にする
		attendant[lngStart] = attendant[lngCounter];
    array[lngCounter] = base;						//基準値を array(lngCounter) に格納
		attendant[lngCounter] = baseattend;
    QuickSort(array, lngStart, lngCounter - 1, attendant);		//分割された配列をクイックソート(再帰)
    QuickSort(array, lngCounter + 1, lngEnd, attendant);		//分割された配列をクイックソート(再帰)

}
