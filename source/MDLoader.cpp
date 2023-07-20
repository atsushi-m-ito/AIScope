// MDLoader.cpp: MDLoader クラスのインプリメンテーション
//
//////////////////////////////////////////////////////////////////////
#pragma warning(disable : 4996)
#include <string.h>
#include <math.h>

#include "MDLoader.h"

//#include "md3_reader.h"
#include "md3_reader2.h"
#include "mychardump.h"
#include "io64.h"
#include "renew.h"
#include "AIScope.h"
#include "ResetBond.h"




extern mat33f g_default_boxaxis;


static const int TXT_INITIAL_ATOM = 4096;
//static const int TXT_INITIAL_BOND = 8192;

//////////////////////////////////////////////////////////////////////
// 構築/消滅
//////////////////////////////////////////////////////////////////////

MDLoader::MDLoader(const TCHAR *filename, double cutoff_length) :
	m_cutoff_length(cutoff_length)	
{
	
	Load(filename);
	if(!m_framev.empty()){
		m_framev[0].keep = 1;
	}


}


MDLoader::~MDLoader(){
	
	if(m_fad){ delete [] m_fad;}
	
	if(m_fa2){ delete [] m_fa2;}
	
	clearBuffer();

}



void MDLoader::Load(const TCHAR *filename){


	size_t outlen = _tcslen(filename);
	
	if(_tcscmp(filename + outlen - 4,_T(".fad")) == 0){
	
		
		if(FirstLoadFAD(filename) < 0){
			printf( "ファイルのオープンに失敗しました : %s\n",filename);	
			readMode = -1;
			return;  // 異常終了は０以外を返す
		}
		readMode = 2;
	
	}else if( _tcscmp(filename + outlen - 4,_T(".fa2")) == 0){

		
		if(FirstLoadFA2(filename) < 0){
			printf( "ファイルのオープンに失敗しました : %s\n",filename);	
			readMode = -1;
			return;  // 異常終了は０以外を返す
		}
		readMode = 4;
	
	}else if(_tcscmp(filename + outlen - 4, _T(".pdb")) == 0){
		//テキスト形式のとき
		//64ビットでは読まない。

		FILE* fp32 = _tfopen(filename,_T("r"));
		if( fp32 == NULL ){  // 関数が失敗していないか
			printf( "ファイルのオープンに失敗しました : %s\n",filename);	
			readMode = -1;
			return;  // 異常終了は０以外を返す
		}
	
		int mcnt = FirstLoadTXT(fp32, 3);
	//	m_fpos_current = m_fpos_first;
		fclose(fp32);
		readMode = 0;
		return;
	}else if((_tcscmp(filename + outlen - 4,_T(".xyz"))==0)||(_tcscmp(filename + outlen - 3,_T(".md")) == 0)){

		FILE* fp32 = _tfopen(filename,_T("r"));
		if( fp32 == NULL ){  // 関数が失敗していないか
			printf( "ファイルのオープンに失敗しました : %s\n",filename);	
			readMode = -1;
			return;  // 異常終了は０以外を返す
		}
	
		int mcnt = FirstLoadTXT(fp32, 4);
	//	m_fpos_current = m_fpos_first;
		fclose(fp32);
		readMode = 0;
		return;
	
	}else if( _tcscmp(filename + outlen - 4,_T(".md2")) == 0){

		FILE* fp32 = _tfopen(filename,_T("r"));
		if( fp32 == NULL ){  // 関数が失敗していないか
			printf( "ファイルのオープンに失敗しました : %zs\n",filename);	
			readMode = -1;
			return;  // 異常終了は０以外を返す
		}
	
		int mcnt = FirstLoadTXT(fp32, 5);
	//	m_fpos_current = m_fpos_first;
		fclose(fp32);
		readMode = 0;
		return;
	
	}else if( _tcscmp(filename + outlen - 4,_T(".md3")) == 0){

		
		if(FirstLoadMD3(filename) < 0){
			printf( "ファイルのオープンに失敗しました : %s\n",filename);	
			readMode = -1;
			return;  // 異常終了は０以外を返す
		}
	
		readMode = 0;
		return;

	}

	
}

int MDLoader::FirstLoadFA2(const TCHAR* filepath){

	
	FA2_Reader* fa2 = new FA2_Reader();
	{

		char* mbfpath = MbsDup(filepath);
		int ierr = fa2->Open(mbfpath);
		delete [] mbfpath;
		if( ierr != 0){
			printf("error: file cannot be opened\n");
			delete fa2;
			return -2;
		}

	}

	m_fa2 = fa2;
	m_unknown_pos = 0LL;

	int frame = 0;
	FRAME_POS framepos;
	int count = ReadFrameFA2(&framepos);
	
	while(count){
		framepos.filepos = m_unknown_pos;
		framepos.atmdat->frame = frame;
		framepos.atmdat->istep = frame;
		framepos.atmdat->no = frame;
		m_framev.push_back(framepos);
	
		m_unknown_pos = m_fa2->GetFilePointer();
			
		count = ReadFrameFA2(&framepos);
		frame++;

		if(m_unknown_pos > READLIMIT) break;

	}	

	if(count == 0){
		m_unknown_pos = 0LL;
	}

	return 0;
}
	
int MDLoader::ReadFrameFA2(FRAME_POS* framepos){

	framepos->atmdat = NULL;
	framepos->keep =0;

	if(m_fa2 == NULL){
		return 0;
	}
		
	long long fpos = m_fa2->GetFilePointer();
	int count = m_fa2->GetParticleCount();
	if(count == 0){
		return 0;
	}
	

	//バッファ確保//
	ATOMS_DATA *a = new ATOMS_DATA;
	a->pcnt = count;

    FA2_DATASIZE data_size;
    m_fa2->GetInfo(&data_size);

    if (data_size.kind_size > 0){
        a->knd = new int[count];
    } else{
        a->knd = NULL;
    }

    if (data_size.id_size > 0){
        a->id = new int[count];
    } else{
        a->id = NULL;
    }

    if (data_size.r_size > 0){
        a->r = new vec3f[count];
    } else{
        a->r = NULL;
    }

    if (data_size.p_size > 0){
        a->p = new vec3f[count];
    } else{
        a->p = NULL;
    }
		

	//ファイル読み込み//////////////////////////////////
	float boxaxis_f[12];
	m_fa2->ReadF(a->knd, a->id, (float*)(a->r), (float*)(a->p), boxaxis_f);
	a->boxaxis.a.Set(boxaxis_f[0], boxaxis_f[1], boxaxis_f[2]);
	a->boxaxis.b.Set(boxaxis_f[3], boxaxis_f[4], boxaxis_f[5]);
	a->boxaxis.c.Set(boxaxis_f[6], boxaxis_f[7], boxaxis_f[8]);
	a->boxorg.Set(boxaxis_f[9], boxaxis_f[10], boxaxis_f[11]);
		
	framepos->filepos = fpos;
	framepos->atmdat = a;
	
	return count;
	
}




int MDLoader::FirstLoadFAD(const TCHAR* filepath){

	
	FAD_Reader* fad = new FAD_Reader();
	{
		char* mbfpath = MbsDup(filepath);
		int ierr = fad->Open(mbfpath);
		delete [] mbfpath;
		if( ierr != 0){
			printf("error: file cannot be opened\n");
			delete fad;
			return -2;
		}

	}

	m_fad = fad;
	m_unknown_pos = 0LL;

	int frame = 0;
	FRAME_POS framepos;
	int count = ReadFrameFAD(&framepos);
	
	while(count){
		framepos.filepos = m_unknown_pos;
		framepos.atmdat->frame = frame;
		framepos.atmdat->istep = frame;
		framepos.atmdat->no = frame;
		m_framev.push_back(framepos);
			
		m_unknown_pos = m_fad->GetFilePointer();

		count = ReadFrameFAD(&framepos);
		frame++;

		if(m_unknown_pos > READLIMIT) break;
	}

	if(count == 0){
		m_unknown_pos = 0LL;
	}
	
	return 0;
}
	
int MDLoader::ReadFrameFAD(FRAME_POS* framepos){

	framepos->atmdat = NULL;
	framepos->keep =0;

	if(m_fad == NULL){
		return 0;
	}
		

	int count = m_fad->GetParticleCount();
	if(count == 0){
		return 0;
	}
		
	//バッファ確保//
	ATOMS_DATA *a = new ATOMS_DATA;
	a->pcnt = count;
	a->knd = new int[count];
	a->id = new int[count];
	a->r = new vec3f[count];
	a->p = NULL;
		

	//ファイル読み込み//////////////////////////////////
	float boxaxis_f[12];
	m_fad->ReadF(a->knd, a->id, (float*)(a->r), NULL, boxaxis_f);
	a->boxaxis.a.Set(boxaxis_f[0], boxaxis_f[1], boxaxis_f[2]);
	a->boxaxis.b.Set(boxaxis_f[3], boxaxis_f[4], boxaxis_f[5]);
	a->boxaxis.c.Set(boxaxis_f[6], boxaxis_f[7], boxaxis_f[8]);
	a->boxorg.Set(boxaxis_f[9], boxaxis_f[10], boxaxis_f[11]);

	framepos->atmdat = a;
	
	
	
	return count;
	
}




int MDLoader::FirstLoadTXT(FILE *fp, int mode){

	ATOMS_DATA *a;
	ATOMS_DATA *readBuf= new ATOMS_DATA;

	int mcnt;
		
	mcnt = 0;

	readMode = 0;

	
	readBuf->pcnt = TXT_INITIAL_ATOM;



	readBuf->knd = new int[TXT_INITIAL_ATOM];
	readBuf->r = new vec3f[TXT_INITIAL_ATOM];

	readBuf->no = 1;
	readBuf->info = NULL;
	
	switch(mode){
	case 3:
		a = readFramePDB(fp,readBuf);
		break;
	case 4:
		a = readFrameXYZ(fp,readBuf);
		break;
	case 5:
		a = readFrameMD2(fp,readBuf);
		break;
	default:
		return 0;
	}

	while(a){
		

		if(a->no == -1){ a->no = mcnt;}
		FRAME_POS framepos;
		framepos.filepos = 0;
		framepos.atmdat = a;
		framepos.keep = 0;
		m_framev.push_back(framepos);


		mcnt ++;
		readBuf->no++;


		switch(mode){
		case 3:
			a = readFramePDB(fp,readBuf);
			break;
		case 4:
			a = readFrameXYZ(fp,readBuf);
			break;
		case 5:
			a = readFrameMD2(fp,readBuf);
			break;
		}

	}

	if (m_unknown_pos) {
		//途中中断
		printf("buffer fulls halfway\n");
	}

	


	printf("%d frame loaded\n",mcnt);
	
	return mcnt;
	
}

bool MDLoader::AddFrameData(ATOMS_DATA* a) {
	if (a == nullptr)return false;

	readMode = 0;
	const int num_frame = (int)(m_framev.size());
	
	FRAME_POS framepos;
	framepos.filepos = 0;
	framepos.atmdat = new ATOMS_DATA;
	*(framepos.atmdat) = *a;
	a->id = nullptr;
	a->r = nullptr;
	a->p = nullptr;
	a->knd = nullptr;
	a->info= nullptr;
	framepos.atmdat->frame = num_frame;
	framepos.atmdat->istep = num_frame;
	framepos.atmdat->no = num_frame;
	framepos.keep = 0;
	m_framev.push_back(framepos);
	return true;
}


int MDLoader::FirstLoadMD3(const TCHAR* filepath) {


	readMode = 0;




	msz::MD3_Reader2<float> md3;
	{

		char* mbfpath = MbsDup(filepath);
		if (! md3.Open(mbfpath)) {
			printf("error: file cannot be opened\n");
			delete[] mbfpath;
			return -2;
		}
		delete[] mbfpath;

	}

	int count = md3.GetParticleCount();
	int frame = 0;
	float boxaxisorg[12];

	while (count > 0) {

		//バッファ確保//
		ATOMS_DATA *a = new ATOMS_DATA;
		a->pcnt = count;
		a->knd = new int[count];
		a->r = new vec3f[count];


		//ファイル読み込み//////////////////////////////////
		md3.BeginFrame();
		md3.ReadZ(a->knd);
		md3.ReadR(a->r);
		md3.ReadBox(boxaxisorg);
		int res = md3.EndFrame();
		if (res < 0) {
			printf("%d frame error\n", frame+1);
			delete a;
			break;
		}

		/////////////////////////////////ファイル読み込み//

		a->boxaxis.a.Set((double)boxaxisorg[0], (double)boxaxisorg[1], (double)boxaxisorg[2]);
		a->boxaxis.b.Set((double)boxaxisorg[3], (double)boxaxisorg[4], (double)boxaxisorg[5]);
		a->boxaxis.c.Set((double)boxaxisorg[6], (double)boxaxisorg[7], (double)boxaxisorg[8]);
		a->boxorg.Clear();
		

		a->id = nullptr;
		/*
		for (int i = 0; i < count; i++) {
		a->id[i] = i;
		}
		*/


		a->frame = frame;
		a->istep = frame;
		a->no = frame;

		a->p = nullptr;

		FRAME_POS framepos;
		framepos.filepos = 0;
		framepos.atmdat = a;
		framepos.keep = 0;
		m_framev.push_back(framepos);

		frame++;

		count = md3.GetParticleCount();

	}

	md3.Close();
	
	printf("%d frame loaded\n", frame);

	return frame;

}


ATOMS_DATA* MDLoader::readFramePDB(FILE* fp, ATOMS_DATA* readBuf){
	
	char chbuf[256];
	char kndch[256];
	char line[256];

//	int dmyi;
	int pcnt,bcnt;
//	float rr,Jmax;
//	vec3f dr;
	const float bondmax[] = {2.0f*2.0f, 1.8f*1.8f, 1.7f*1.7f};//{1.85*1.85, 1.55*1.55, 1.4*1.4};
	
	int istep;
	short int* pkindint; 

	
	int* knd = readBuf->knd;

	vec3f* r = readBuf->r;



	int flag_J;	//0: energy current有, 1: 無し
	
	ATOMS_DATA *a;



//	Jmax = -1.0;
	if(fgets(line,255,fp) == NULL){ return NULL;}
	if (strncmp(line,"MODEL",5) != 0){ return NULL;}

		sscanf(line,"%s %d",chbuf, &istep);
			flag_J = 0x8;

		pcnt = 0;
		bcnt=0;
		

		while (fgets(line,255,fp) != NULL){
			if(strncmp(line,"ENDMDL",6) == 0){break;}
			if(strncmp(line,"ATOM",4) == 0){
				sscanf(line,"%s%s%s%s %f %f %f",chbuf,chbuf,kndch,chbuf,
					&(r[pcnt].x), &(r[pcnt].y), &(r[pcnt].z));

				pkindint = (short int*)kndch;
				if (strcmp(kndch,"Cc") == 0){	//C_custom
					knd[pcnt] = 0;
				//}else if (strcmp(kndch,"H")==0){
				}else if (*pkindint == 72){
					knd[pcnt] = 1;
				}else if (strcmp(kndch,"He")==0){
					knd[pcnt] = 2;
				}else if (strcmp(kndch,"Li")==0){
					knd[pcnt] = 3;
				}else if (strcmp(kndch,"Be")==0){
					knd[pcnt] = 4;
				}else if (strcmp(kndch,"B")==0){
					knd[pcnt] = 5;
				}else if (strcmp(kndch,"C")==0){
					knd[pcnt] = 6;
				}else if (strcmp(kndch,"N")==0){
					knd[pcnt] = 7;
				}else if (strcmp(kndch,"O")==0){
					knd[pcnt] = 8;
				}else if (strcmp(kndch,"F")==0){
					knd[pcnt] = 9;
				}else if (strcmp(kndch,"N")==0){
					knd[pcnt] = 10;
				}else if (strcmp(kndch,"Na")==0){
					knd[pcnt] = 11;
				}else if (strcmp(kndch,"Mg")==0){
					knd[pcnt] = 12;
				}else if (strcmp(kndch,"Al")==0){
					knd[pcnt] = 13;
				}else if (strcmp(kndch,"Si")==0){
					knd[pcnt] = 14;
				}else if (strcmp(kndch,"P")==0){
					knd[pcnt] = 15;
				}else if (strcmp(kndch,"S")==0){
					knd[pcnt] = 16;
				}else if (strcmp(kndch,"Cl")==0){
					knd[pcnt] = 17;
				}else if (strcmp(kndch,"Ar")==0){
					knd[pcnt] = 18;
				}else if (strcmp(kndch,"K")==0){
					knd[pcnt] = 19;
				}else if (strcmp(kndch,"Ca")==0){
					knd[pcnt] = 20;
				}else{
					knd[pcnt] = 21;
				}

				pcnt ++ ;
				//バッファオーバーフロー
				if (pcnt == readBuf->pcnt){
					
					readBuf->pcnt *= 4;
					
					renew_preserve<int>(&knd, pcnt, readBuf->pcnt);
					renew_preserve<vec3f>(&r, pcnt, readBuf->pcnt);
					
					readBuf->knd = knd;
					readBuf->r = r;
				}

			}
			
		}
		


		//AI_atomdatにコピー
		a = new ATOMS_DATA;
		
		a->pcnt=pcnt;
//		a->bcnt=bcnt;
		a->boxaxis.Clear();
		a->boxorg.Clear();
		a->tempave = 0.0f;
		a->tempvar = 0.0f;
		a->no = readBuf->no;
		a->istep = istep;
		a->realtm = 0.0;
		
		a->knd = new int[pcnt];
		memcpy(a->knd, knd, sizeof(char) * pcnt);
		a->r = new vec3f[pcnt];
		memcpy(a->r, r, sizeof(vec3f) * pcnt);

		a->info = NULL;


	return a;


}



ATOMS_DATA* MDLoader::readFrameXYZ(FILE* fp, ATOMS_DATA* readBuf){
	
	char kndch[256];
	char line[1024];

	int pcnt,bcnt;
	float Jmax;
	const float BOND_LENGTH_SQUARE = 2.0f*2.0f;
	

	
	int* knd = NULL;
	vec3f* r = NULL;
//	float* Jij = readBuf->Jij;
	short int* pkindint; 

	
	ATOMS_DATA *a;



	Jmax = -1.0;
	if(fgets(line,1024,fp) == NULL){ return NULL;}
		sscanf(line,"%d", &pcnt);
		if(pcnt<=0){return NULL;}
		bcnt=0;

		
		if(fgets(line,1024,fp) == NULL){ return NULL;}
		//各pictureごとのtitleの読み取り

		a = new ATOMS_DATA;
		a->knd = knd = new int[pcnt];
		a->r = r = new vec3f[pcnt];
		a->pcnt = pcnt;
		pcnt = 0;
		
		a->info = _strdup(line);

		while (fgets(line,1024,fp) != NULL){
			sscanf(line,"%s %f %f %f",kndch, &(r[pcnt].x), &(r[pcnt].y), &(r[pcnt].z));
			
			pkindint = (short int*)kndch;
			switch (*pkindint){
			case 0x6343:
			case 0x4343:
				knd[pcnt] = 0;
				break;
			case 'H':
			case 'h':
				knd[pcnt] = 1;
				break;
			case 0x6548:
			case 0x6568:
				knd[pcnt] = 2;
				break;
			case 0x694C:
			case 0x696C:
				knd[pcnt] = 3;
				break;
			case 0x6542:
			case 0x6562:
				knd[pcnt] = 4;
				break;
			case 'B':
			case 'b':
				knd[pcnt] = 5;
				break;
			case 'C':
			case 'c':
				knd[pcnt] = 6;
				break;
			case 'N':
			case 'n':
				knd[pcnt] = 7;
				break;
			case 'O':
			case 'o':
				knd[pcnt] = 8;
				break;
			case 'F':
			case 'f':
				knd[pcnt] = 9;
				break;
			case 0x654E:
			case 0x656E:
				knd[pcnt] = 10;
				break;
			case 0x614E:
			case 0x616E:
				knd[pcnt] = 11;
				break;
			case 0x674D:
			case 0x676D:
				knd[pcnt] = 12;
				break;
			case 0x6c41:
			case 0x6c61:
				knd[pcnt] = 13;
				break;
			case 0x6953:
			case 0x6973:
				knd[pcnt] = 14;
				break;
			case 'P':
			case 'p':
				knd[pcnt] = 15;
				break;
			case 'S':
			case 's':
				knd[pcnt] = 16;
				break;
			case 0x6C43:
			case 0x6C63:
				knd[pcnt] = 17;
				break;
			case 0x7241:
			case 0x7261:
				knd[pcnt] = 18;
				break;
			case 'k':
			case 'K':
				knd[pcnt] = 19;
				break;
			case 0x6145:
			case 0x6165:
				knd[pcnt] = 17;
				break;
			default:
				knd[pcnt] = 0;
			}
		
				pcnt ++ ;
				//バッファオーバーフロー
				if (pcnt == a->pcnt){break;}
		}


		//AI_atomdatにコピー		
//		a->bcnt=bcnt;
		a->boxaxis = g_default_boxaxis;
		a->boxorg.Clear();
		a->tempave = 0.0f;
		a->tempvar = 0.0f;
		a->no = readBuf->no;
		a->istep = 0;
		a->realtm = 0.0;
		


	return a;


}


ATOMS_DATA* MDLoader::readFrameMD2(FILE* fp, ATOMS_DATA* readBuf){
	
	char kndch[256];
	char line[1024];

	int pcnt,bcnt;
	float Jmax;
	const float BOND_LENGTH_SQUARE = 2.0f*2.0f;
	

	
	int* knd = NULL;
	vec3f* r = NULL;
//	float* Jij = readBuf->Jij;
	short int* pkindint; 

	
	ATOMS_DATA *a;



	Jmax = -1.0;
	if(fgets(line,1024,fp) == NULL){ return NULL;}
		sscanf(line,"%d", &pcnt);
		if(pcnt<=0){return NULL;}
		bcnt=0;

		
		if(fgets(line,1024,fp) == NULL){ return NULL;}
		//各pictureごとのtitleの読み取り

		a = new ATOMS_DATA;
		a->knd = knd = new int[pcnt];
		a->r = r = new vec3f[pcnt];
		a->pcnt = pcnt;
		pcnt = 0;
		
		a->info = _strdup(line);

		while (fgets(line,1024,fp) != NULL){
			int id = 0;
			sscanf(line,"%d %s %f %f %f", &id, kndch, &(r[pcnt].x), &(r[pcnt].y), &(r[pcnt].z));
			
			pkindint = (short int*)kndch;
			switch (*pkindint){
			case 0x6343:
			case 0x4343:
				knd[pcnt] = 0;
				break;
			case 'H':
			case 'h':
				knd[pcnt] = 1;
				break;
			case 0x6548:
			case 0x6568:
				knd[pcnt] = 2;
				break;
			case 0x694C:
			case 0x696C:
				knd[pcnt] = 3;
				break;
			case 0x6542:
			case 0x6562:
				knd[pcnt] = 4;
				break;
			case 'B':
			case 'b':
				knd[pcnt] = 5;
				break;
			case 'C':
			case 'c':
				knd[pcnt] = 6;
				break;
			case 'N':
			case 'n':
				knd[pcnt] = 7;
				break;
			case 'O':
			case 'o':
				knd[pcnt] = 8;
				break;
			case 'F':
			case 'f':
				knd[pcnt] = 9;
				break;
			case 0x654E:
			case 0x656E:
				knd[pcnt] = 10;
				break;
			case 0x614E:
			case 0x616E:
				knd[pcnt] = 11;
				break;
			case 0x674D:
			case 0x676D:
				knd[pcnt] = 12;
				break;
			case 0x6c41:
			case 0x6c61:
				knd[pcnt] = 13;
				break;
			case 0x6953:
			case 0x6973:
				knd[pcnt] = 14;
				break;
			case 'P':
			case 'p':
				knd[pcnt] = 15;
				break;
			case 'S':
			case 's':
				knd[pcnt] = 16;
				break;
			case 0x6C43:
			case 0x6C63:
				knd[pcnt] = 17;
				break;
			case 0x7241:
			case 0x7261:
				knd[pcnt] = 18;
				break;
			case 'k':
			case 'K':
				knd[pcnt] = 19;
				break;
			case 0x6145:
			case 0x6165:
				knd[pcnt] = 17;
				break;
/*			case 'w':
			case 'W':
				knd[pcnt] = 74;
				break;*/
			default:
				knd[pcnt] = 0;
			}
		
				pcnt ++ ;
				//バッファオーバーフロー
				if (pcnt == a->pcnt){break;}
		}


		//AI_atomdatにコピー		
//		a->bcnt=bcnt;
		a->boxaxis = g_default_boxaxis;
		a->boxorg.Clear();
		a->tempave = 0.0f;
		a->tempvar = 0.0f;
		a->no = readBuf->no;
		a->istep = 0;
		a->realtm = 0.0;
		


	return a;


}



void MDLoader::clearBuffer(){
	
	
	for( IT_FRAME_POS it = m_framev.begin(); it != m_framev.end(); it++){
		if (it->atmdat){
			DeleteFrame(it->atmdat);
			it->atmdat = NULL;
		}
	}
	//delete m_framev;
}


int MDLoader::goNext(){
	//成功したら1を返す


	if (readMode == -1){ return 0;}

	

	if(m_frameNo + 1 < (int)m_framev.size() ){
		//次が存在し、読み込み済みである
		m_frameNo++;
		return 1;
	}
	
	if(m_unknown_pos == LONG64_ZERO){
		//ファイル終端
		return 0;
	}

	//未読部分の読み込み//
	int count = 0;
	FRAME_POS framepos;
			
	const long long current_pos = m_unknown_pos;
	switch (readMode){
	case 2:
	{
		m_fad->SetFilePointer(m_unknown_pos);
		count = ReadFrameFAD(&framepos);
		if (count > 0) {
			m_unknown_pos = m_fad->GetFilePointer();
		} else {
			m_unknown_pos = 0;
			return 0;
		}
		break;
	}
	case 4:
	{
		m_fa2->SetFilePointer(m_unknown_pos);
		count = ReadFrameFA2(&framepos);		
		if (count > 0) {
			m_unknown_pos = m_fa2->GetFilePointer();
		} else {
			m_unknown_pos = 0;
			return 0;
		}
		break;
	}
	case 0:
		return 0;
	}
	
	{
		
		m_frameNo++;
		framepos.filepos = current_pos;
		framepos.atmdat->frame = m_frameNo;
		framepos.atmdat->istep = m_frameNo;
		framepos.atmdat->no = m_frameNo;
		m_framev.push_back(framepos);
	}
	
	//bufferの移行
	UnloadStock();

	//m_frameNo++;
	return 1;
	
}


int MDLoader::goNext(int num){
	

	if(num<=0){return -1;}
	if (readMode == -1){ return 0;}

	for(int i=0;i<num - 1;i++){
		
		if(m_frameNo + 1 < (int)m_framev.size() ){
			//次が存在する
			m_frameNo++;
			
		}else{
			
			if(m_unknown_pos == LONG64_ZERO){
				//ファイル終端
				return 0;
			}

			const long long current_pos = m_unknown_pos;
			switch (readMode){
			case 2:
				{
					m_fad->SetFilePointer(current_pos);
					int count = m_fad->GetParticleCount();
					if(count > 0){
						m_fad->SkipFrame();
						m_unknown_pos = m_fad->GetFilePointer();
					}else{
						m_unknown_pos = LONG64_ZERO;
						return 0;
					}
				}
				break;
			case 4:
				{
					m_fa2->SetFilePointer(current_pos);
					int count = m_fa2->GetParticleCount();
					if(count > 0){
						m_fa2->SkipFrame();
						m_unknown_pos = m_fa2->GetFilePointer();
					}else{
						m_unknown_pos = LONG64_ZERO;
						return 0;
					}
				}
				break;
			case 0:
				return 0;
			}
				
			
			//if (count > 0)//
			{
		
				FRAME_POS framepos;
				framepos.keep = 0;
				framepos.atmdat = NULL;
				framepos.filepos = current_pos;
				m_framev.push_back(framepos);
			}
					
			m_frameNo++;
		}
	}
	
	
	return goNext();
}

int MDLoader::goPrev(){
	if (readMode == -1){ return 0;}

	if(m_frameNo > 0){
		m_frameNo--;
		return 1;
	}
	return 0;
}


int MDLoader::goPrev(int num){

	if(num<=0){return -1;}
	if (readMode == -1){ return 0;}

	if(m_frameNo == 0){return 0;}
	m_frameNo-=num;
	if(m_frameNo < 0){
		m_frameNo = 0;
	}
	return 1;
}


ATOMS_DATA* MDLoader::GetDataPointer(){
	if (readMode == -1){ return NULL;}

	if(m_framev[m_frameNo].atmdat == NULL){
		if(readMode>0){
			switch (readMode){
			case 2:
	
				m_fad->SetFilePointer(m_framev[m_frameNo].filepos);
				ReadFrameFAD(&(m_framev[m_frameNo]));
				break;
			case 4:
				
				m_fa2->SetFilePointer(m_framev[m_frameNo].filepos);
				ReadFrameFA2(&(m_framev[m_frameNo]));
				break;
			}
			


			if(m_framev[m_frameNo].atmdat){
				//読めたとき(基本的に必ず読めることが保障されているはず)
				UnloadStock();
			}
		}
	}


	return m_framev[m_frameNo].atmdat;
	
}

void MDLoader::GetBoxaxis(mat33d* boxaxis, vec3d* boxorg) {
	if (m_framev[m_frameNo].atmdat != NULL) {
		*boxaxis = m_framev[m_frameNo].atmdat->boxaxis;
		*boxorg = m_framev[m_frameNo].atmdat->boxorg;
	}
}


BOND_INFO* MDLoader::GetBond() {
	if (m_framev[m_frameNo].atmdat == nullptr) {
		return nullptr;
	}

	if (m_framev[m_frameNo].atmdat->bond.count == 0) {
		glips_ResetBond(m_framev[m_frameNo].atmdat, &(m_framev[m_frameNo].atmdat->bond), m_cutoff_length);
	}
	return &(m_framev[m_frameNo].atmdat->bond);
}

void MDLoader::SetBondCutoff(double cutoff_length) {
	m_cutoff_length = cutoff_length;

};


int MDLoader::GetFrameNum(){
	return m_frameNo;
}

int MDLoader::SetFrameNum(int frameNo){
//指定したフレームまでジャンプする.
//ただし、そのフレームのデータが存在しない場合には.
//その存在する最終フレームまでジャンプして止まる.
//戻り値には実際に移動したフレームのNoが帰るので,
//それによってファイルの最終フレームに達したかが判断できる.

	int delta = frameNo - m_frameNo;
	if(delta > 0){
		goNext(delta);
	}else if(delta < 0){
		goPrev(-delta);
	}

	
	return m_frameNo;

}

void MDLoader::DeleteFrame(ATOMS_DATA *dat){
	
	if (dat){
		delete dat;
	}
}


void MDLoader::UnloadStock(){
	int i = 0;
	for (IT_FRAME_POS it = m_framev.begin(); it != m_framev.end(); it++){
		if( i != m_frameNo){
			if(it->atmdat){
				if(it->keep == 0){
					DeleteFrame(it->atmdat);
					it->atmdat = NULL;
					break;
				}
				
			}
		}
		i++;
	}
}

