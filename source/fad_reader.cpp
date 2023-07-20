
#define _CRT_SECURE_NO_DEPRECATE

#include "fad_reader.h"
//#include <stdio.h>



#define FAD_FLAG_J		(0x1)		//エネルギー流データがある.
#define FAD_FLAG_SIMBOX	(0x2)	//simboxの指定がある.
#define FAD_FLAG_NONORTHOGONALBOX	(0x20)	//simboxが非直交軸である.
#define FAD_FLAG_TEMPERATURE		(0x4)	//温度情報がある.
#define FAD_FLAG_STEP		(0x8)		//int4型のstep数情報がある.
#define FAD_FLAG_ATOMEXT		(0x10)		//粒子のidの配列がある.

const long HEADER_SIZE = sizeof(int)*5;
const long DUMMY_SIZE = sizeof(int);


FAD_Reader::FAD_Reader() : 
	m_state(0),
	m_fp(NULL),
	m_pcnt(0),
	m_flag(0)	
{
	
}

FAD_Reader::~FAD_Reader(){
	this->Close();
}


int FAD_Reader::Open(const char* filepath){
	
	if(m_fp){
		return -2;
	}

	m_fp = fopen(filepath, "rb");

	if(m_fp){
		return 0;
	}else{
		return -1;
	}
}

void FAD_Reader::Close(){
	if(m_fp){
		fclose(m_fp);
		m_fp = NULL;
	}
}


long long FAD_Reader::GetFilePointer(){
	return _ftelli64(m_fp);
}

void FAD_Reader::SetFilePointer(long long offset){
	_fseeki64(m_fp, offset, SEEK_SET);
}

int FAD_Reader::GetParticleCount(){
	
	
	//header////////////////////////////////////////////////////
	if(m_state == 0){
		if(fread(m_header, sizeof(int) * 5, 1, m_fp) == 0){
			m_state = 0;
			return 0;
		}
		m_state = 1;	//header had been read//
	}

	
	return m_pcnt;

}



void FAD_Reader::Read(int* knd, int* id, double* r, double* p, double* boxaxis){
	
	if(m_state != 1){return;}

	int  count = m_pcnt;

	//initial dummy skip//

	//kndの読み込み.
	if(knd){
		_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
		fread(knd, sizeof(int) * count, 1, m_fp);
		_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
	}else{
		_fseeki64(m_fp, sizeof(int) * count + DUMMY_SIZE*2, SEEK_CUR);
	}
	
	if(m_flag & FAD_FLAG_ATOMEXT){
		if(id){
			_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
			fread(id, sizeof(int) * count, 1, m_fp);
			_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
		}else{
			_fseeki64(m_fp, sizeof(int) * count + DUMMY_SIZE*2, SEEK_CUR);
		}
	}

	float* buf = new float[count];
	if(r){
		_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
		fread(buf, sizeof(float) * count, 1, m_fp);
		_fseeki64(m_fp, DUMMY_SIZE*2, SEEK_CUR);
		for(int i = 0; i < count; i++){
			r[i*3] = (double)(buf[i]);
		}
		fread(buf, sizeof(float) * count, 1, m_fp);
		_fseeki64(m_fp, DUMMY_SIZE*2, SEEK_CUR);
		for(int i = 0; i < count; i++){
			r[i*3+1] = (double)(buf[i]);
		}
		fread(buf, sizeof(float) * count, 1, m_fp);
		_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
		for(int i = 0; i < count; i++){
			r[i*3+2] = (double)(buf[i]);
		}
	}else{
		_fseeki64(m_fp, (sizeof(float) * count + DUMMY_SIZE*2)*3, SEEK_CUR);
	}

	if(m_flag & FAD_FLAG_ATOMEXT){
		if(p){
			_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
			fread(buf, sizeof(float) * count, 1, m_fp);
			_fseeki64(m_fp, DUMMY_SIZE*2, SEEK_CUR);
			for(int i = 0; i < count; i++){
				p[i*3] = (double)(buf[i]);
			}
			fread(buf, sizeof(float) * count, 1, m_fp);
			_fseeki64(m_fp, DUMMY_SIZE*2, SEEK_CUR);
			for(int i = 0; i < count; i++){
				p[i*3+1] = (double)(buf[i]);
			}
			fread(buf, sizeof(float) * count, 1, m_fp);
			_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
			for(int i = 0; i < count; i++){
				p[i*3+2] = (double)(buf[i]);
			}
		}else{
			_fseeki64(m_fp, (sizeof(float) * count + DUMMY_SIZE*2)*3, SEEK_CUR);
		}
	}


	//ボンドデータがあるなら読み飛ばす.
	if(m_bcnt){
		if(m_flag & FAD_FLAG_J){
			_fseeki64(m_fp, (sizeof(int) * m_bcnt + DUMMY_SIZE * 2) * 3, SEEK_CUR);
		}else{
			_fseeki64(m_fp, (sizeof(int) * m_bcnt + DUMMY_SIZE * 2) * 2, SEEK_CUR);
		}
	}


	//ボックスサイズ.
	if (m_flag & FAD_FLAG_SIMBOX) { //simboxの指定がある
		if (m_flag & FAD_FLAG_NONORTHOGONALBOX){//非直交軸によるsimbox指定.
			if(boxaxis){			
				
				float boxf[12];
				_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
				fread(boxf,	sizeof(float) * 12, 1, m_fp);
				_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
				for(int i = 0; i < 12; i++){
					boxaxis[i] = (double)boxf[i];
				}
			}else{
				_fseeki64(m_fp, sizeof(float) * 12 + DUMMY_SIZE*2, SEEK_CUR);
			}
			
		}else{//直交軸によるsimbox指定.

				
			if(boxaxis){	
				float boxf[3];
				_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
				fread(boxf,	sizeof(float) * 3, 1, m_fp);
				_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
			
				for(int i = 1; i < 8; i++){
					boxaxis[i] = 0.0;
				}
				boxaxis[0] = (double)boxf[0];
				boxaxis[4] = (double)boxf[1];
				boxaxis[8] = (double)boxf[2];

				boxaxis[9] = - 0.5 * boxaxis[0];
				boxaxis[10] = - 0.5 * boxaxis[4];
				boxaxis[11] = - 0.5 * boxaxis[8];

				//位置座標がboxsizeの半分ずれている点を修正.
				double dx = 0.5 * (double)boxf[0];
				double dy = 0.5 * (double)boxf[1];
				double dz = 0.5 * (double)boxf[2];
				if(r){
					for(int i = 0; i < count; i++){
						r[i*3] += dx;
						r[i*3+1] += dy;
						r[i*3+2] += dz;
					}
				}
			}else{
				_fseeki64(m_fp, sizeof(float) * 3 + DUMMY_SIZE*2, SEEK_CUR);
			
			}
		}
	}else{
		if(boxaxis){
			for(int i = 0; i < 12; i++){
					boxaxis[i] = 0.0;
			}
		}
	}

	//温度があるなら読み飛ばす
	if(m_flag & FAD_FLAG_TEMPERATURE){
		_fseeki64(m_fp, sizeof(float) * 2 + DUMMY_SIZE*2, SEEK_CUR);
			

	}

	//ステップ数の情報があるなら読み飛ばす
	if(m_flag & FAD_FLAG_STEP){
		_fseeki64(m_fp, sizeof(float) * 3 + DUMMY_SIZE*2, SEEK_CUR);
			
	}
	
		
	if(buf) delete [] buf;

	m_state = 0;	//finish to read the frame//

}

void FAD_Reader::ReadF(int* knd, int* id, float* r, float* p, float* boxaxis){
	
	if(m_state != 1){return;}

	int  count = m_pcnt;

	//kndの読み込み.
	if(knd){
		
		_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
		fread(knd, sizeof(int) * count, 1, m_fp);
		_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
	}else{
		_fseeki64(m_fp, sizeof(int) * count + DUMMY_SIZE*2, SEEK_CUR);
	}
	
	if(m_flag & FAD_FLAG_ATOMEXT){
		if(id){
			_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
			fread(id, sizeof(int) * count, 1, m_fp);
			_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
		}else{
			_fseeki64(m_fp, sizeof(int) * count + DUMMY_SIZE*2, SEEK_CUR);
		}
	}

	float* buf = new float[count];
	if(r){
		_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
		fread(buf, sizeof(float) * count, 1, m_fp);
		_fseeki64(m_fp, DUMMY_SIZE*2, SEEK_CUR);
		for(int i = 0; i < count; i++){
			r[i*3] = (float)(buf[i]);
		}
		fread(buf, sizeof(float) * count, 1, m_fp);
		_fseeki64(m_fp, DUMMY_SIZE*2, SEEK_CUR);
		for(int i = 0; i < count; i++){
			r[i*3+1] = (float)(buf[i]);
		}
		fread(buf, sizeof(float) * count, 1, m_fp);
		_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
		for(int i = 0; i < count; i++){
			r[i*3+2] = (float)(buf[i]);
		}
	}else{
		_fseeki64(m_fp, (sizeof(float) * count + DUMMY_SIZE*2)*3, SEEK_CUR);
	}

	if(m_flag & FAD_FLAG_ATOMEXT){
		if(p){
			_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
			fread(buf, sizeof(float) * count, 1, m_fp);
			_fseeki64(m_fp, DUMMY_SIZE*2, SEEK_CUR);
			for(int i = 0; i < count; i++){
				p[i*3] = (float)(buf[i]);
			}
			fread(buf, sizeof(float) * count, 1, m_fp);
			_fseeki64(m_fp, DUMMY_SIZE*2, SEEK_CUR);
			for(int i = 0; i < count; i++){
				p[i*3+1] = (float)(buf[i]);
			}
			fread(buf, sizeof(float) * count, 1, m_fp);
			_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
			for(int i = 0; i < count; i++){
				p[i*3+2] = (float)(buf[i]);
			}
		}else{
			_fseeki64(m_fp, (sizeof(float) * count + DUMMY_SIZE*2)*3, SEEK_CUR);
		}
	}


	//ボンドデータがあるなら読み飛ばす.
	if(m_bcnt){
		if(m_flag & FAD_FLAG_J){
			_fseeki64(m_fp, (sizeof(int) * m_bcnt + DUMMY_SIZE * 2) * 3, SEEK_CUR);
		}else{
			_fseeki64(m_fp, (sizeof(int) * m_bcnt + DUMMY_SIZE * 2) * 2, SEEK_CUR);
		}
	}


	//ボックスサイズ.
	if (m_flag & FAD_FLAG_SIMBOX) { //simboxの指定がある
		if (m_flag & FAD_FLAG_NONORTHOGONALBOX){//非直交軸によるsimbox指定.
			if(boxaxis){			
				
				_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
				fread(boxaxis,	sizeof(float) * 12, 1, m_fp);
				_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);

			}else{
				_fseeki64(m_fp, sizeof(float) * 12 + DUMMY_SIZE*2, SEEK_CUR);
			}
			
		}else{//直交軸によるsimbox指定.

				
			if(boxaxis){	
				float boxf[3];
				
				_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
				fread(boxf,	sizeof(float) * 3, 1, m_fp);
				_fseeki64(m_fp, DUMMY_SIZE, SEEK_CUR);
			
				for(int i = 1; i < 8; i++){
					boxaxis[i] = 0.0;
				}
				boxaxis[0] = boxf[0];
				boxaxis[4] = boxf[1];
				boxaxis[8] = boxf[2];

				boxaxis[9] = - 0.5f * boxaxis[0];
				boxaxis[10] = - 0.5f * boxaxis[4];
				boxaxis[11] = - 0.5f * boxaxis[8];

				//位置座標がboxsizeの半分ずれている点を修正.
				float dx = 0.5f * boxf[0];
				float dy = 0.5f * boxf[1];
				float dz = 0.5f * boxf[2];
				if(r){
					for(int i = 0; i < count; i++){
						r[i*3] += dx;
						r[i*3+1] += dy;
						r[i*3+2] += dz;
					}
				}
			}else{
				_fseeki64(m_fp, sizeof(float) * 3 + DUMMY_SIZE*2, SEEK_CUR);
			
			}
		}
	}else{
		if(boxaxis){
			for(int i = 0; i < 12; i++){
					boxaxis[i] = 0.0f;
			}
		}
	}

	//温度があるなら読み飛ばす
	if(m_flag & FAD_FLAG_TEMPERATURE){
		_fseeki64(m_fp, sizeof(float) * 2 + DUMMY_SIZE*2, SEEK_CUR);
			

	}

	//ステップ数の情報があるなら読み飛ばす
	if(m_flag & FAD_FLAG_STEP){
		int length = 0;
		fread(&length, sizeof(int) , 1, m_fp);//step数のバイト数はfadのversionによって異なる//
		_fseeki64(m_fp, length + DUMMY_SIZE, SEEK_CUR);			
	}
	
		
	if(buf) delete [] buf;

	m_state = 0;	//finish to read the frame//

}


void FAD_Reader::SkipFrame(){
	if(m_state != 1){ return;}
	
	int count = m_pcnt;
	long long skipsz = 0;

	skipsz += sizeof(int)*count + DUMMY_SIZE * 2;	//knd
		
	if(m_flag & FAD_FLAG_ATOMEXT){
		skipsz += sizeof(int)*count + DUMMY_SIZE * 2;	//id
	}
		
	skipsz += (sizeof(float)*count + DUMMY_SIZE * 2) * 3;	//rx,ry,rz

	if(m_flag & FAD_FLAG_ATOMEXT){
		skipsz += (sizeof(float)*count + DUMMY_SIZE * 2) * 3;	//px,py,pz
	}

	if(m_bcnt>0){
		if(m_flag & FAD_FLAG_J){
			skipsz += (sizeof(int)*m_bcnt + DUMMY_SIZE * 2) * 3;	//li,lj,Jij
		}else{
			skipsz += (sizeof(int)*m_bcnt + DUMMY_SIZE * 2) * 2;	//li,lj
		}
	}

	if(m_flag & FAD_FLAG_SIMBOX){
		if (m_flag & FAD_FLAG_NONORTHOGONALBOX){//直交軸によるsimbox指定.
			skipsz += sizeof(float) * 12 + DUMMY_SIZE * 2;	//boxaxis,boxorg
		}else{
			skipsz += sizeof(float) * 3 + DUMMY_SIZE * 2;	//boxx,boxy,boxz
		}
	}
		
	if(m_flag & FAD_FLAG_TEMPERATURE){
		skipsz += sizeof(float) * 2 + DUMMY_SIZE * 2;	//tempave,tempvar
	}
		
	if(m_flag & FAD_FLAG_STEP){
		skipsz += sizeof(int) * 3 + DUMMY_SIZE * 2;	//no,istep
		//skipsz += sizeof(int) * 2 + DUMMY_SIZE * 2;	//no,istep	//旧ファイル//
	}


	_fseeki64(m_fp, skipsz, SEEK_CUR);
	m_state = 0;	//finish to read the frame//
}


/*
int FAD_Reader::_Read(int count, int offset, int* knd, int* id, double* r, double* rx, double* ry, double* rz,
					  double* p, double* px, double* py, double* pz, double* boxaxis){

	if(!m_fp){ return 0;}
	if(count <= 0){ return 0;}

	int read_count;
	if( count < m_pcnt - offset){
		read_count = count;
	}else{
		read_count = m_pcnt - offset;
	}

	long filepos = m_current_header_pos + HEADER_SIZE + DUMMY_SIZE;



	//kndの読み込み.
	if(knd){

		//位置へ移動.
		fseek(m_fp, filepos + offset * sizeof(int), SEEK_SET);

		fread(knd, sizeof(int) * read_count, 1, m_fp);
	}
	filepos += sizeof(int) * (m_pcnt) + DUMMY_SIZE * 2;


	if(m_flag & FAD_FLAG_ATOMEXT){
		if(id){

			fseek(m_fp, filepos + offset * sizeof(int), SEEK_SET);
	
			fread(id,	 sizeof(int) * read_count, 1, m_fp);
		}			
		filepos += sizeof(int) * (m_pcnt) + DUMMY_SIZE * 2;
	}

	if(r){	//ここがあれば三次元ベクトルとして読み込む
		ReadFloat3AsVector3D(filepos, read_count, offset, r);

		filepos += (sizeof(float) * (m_pcnt) + DUMMY_SIZE * 2) * 3;
		
	}else{	//無ければxxx,yyy,zzzとして独立に読み込む.
		
		if(rx){
			ReadFloatAsDouble(filepos, read_count, offset, rx);
		}
		filepos += sizeof(float) * (m_pcnt) + DUMMY_SIZE * 2;

		if(ry){
			ReadFloatAsDouble(filepos, read_count, offset, ry);
		}
		filepos += sizeof(float) * (m_pcnt) + DUMMY_SIZE * 2;

		if(rz){
			ReadFloatAsDouble(filepos, read_count, offset, rz);
		}
		filepos += sizeof(float) * (m_pcnt) + DUMMY_SIZE * 2;

	}
	


	if(m_flag & FAD_FLAG_ATOMEXT){
		if(p){	//ここがあれば三次元ベクトルとして読み込む
			ReadFloat3AsVector3D(filepos, read_count, offset, p);

			filepos += (sizeof(float) * (m_pcnt) + DUMMY_SIZE * 2) * 3;
			
		}else{	//無ければxxx,yyy,zzzとして独立に読み込む.
			
			if(px){
				ReadFloatAsDouble(filepos, read_count, offset, px);
			}
			filepos += sizeof(float) * (m_pcnt) + DUMMY_SIZE * 2;

			if(py){
				ReadFloatAsDouble(filepos, read_count, offset, py);
			}
			filepos += sizeof(float) * (m_pcnt) + DUMMY_SIZE * 2;

			if(pz){
				ReadFloatAsDouble(filepos, read_count, offset, pz);
			}
			filepos += sizeof(float) * (m_pcnt) + DUMMY_SIZE * 2;

		}
	}

	//ボンドデータがあるなら読み飛ばす.
	if(m_bcnt){
		
		filepos += (sizeof(int) * m_bcnt + DUMMY_SIZE * 2) * 2;
		
		if(m_flag & FAD_FLAG_J){
		
			filepos += sizeof(float) * m_bcnt + DUMMY_SIZE * 2;
		}

	}


	//ボックスサイズ.
	if (m_flag & FAD_FLAG_SIMBOX) { //simboxの指定がある
		if (m_flag & FAD_FLAG_NONORTHOGONALBOX){//非直交軸によるsimbox指定.
					
			
			if(boxaxis){			
				fseek(m_fp, filepos, SEEK_SET);
				
				float boxf[12];
				fread(boxf,	sizeof(float) * 12, 1, m_fp);
				for(int i = 0; i < 12; i++){
					boxaxis[i] = (double)boxf[i];
				}
			}
			
			filepos += sizeof(float) * 12 + DUMMY_SIZE * 2;

		}else{//直交軸によるsimbox指定.

			
			fseek(m_fp, filepos, SEEK_SET);

			filepos += sizeof(float) * 3 + DUMMY_SIZE * 2;
			
			float boxf[3];
			fread(boxf,	sizeof(float) * 3, 1, m_fp);
				
			if(boxaxis){	
				
				for(int i = 1; i < 8; i++){
					boxaxis[i] = 0.0;
				}
				boxaxis[0] = (double)boxf[0];
				boxaxis[4] = (double)boxf[1];
				boxaxis[8] = (double)boxf[2];

				boxaxis[9] = - 0.5f * boxaxis[0];
				boxaxis[10] = - 0.5f * boxaxis[4];
				boxaxis[11] = - 0.5f * boxaxis[8];
			}

			//位置座標がboxsizeの半分ずれている点を修正.
			double dx = 0.5f * (double)boxf[0];
			double dy = 0.5f * (double)boxf[1];
			double dz = 0.5f * (double)boxf[2];
			if(r){
				for(int i = 0; i < read_count; i++){
					r[i*3] += dx;
					r[i*3+1] += dy;
					r[i*3+2] += dz;
				}
			}else{
				if(rx){
					for(int i = 0; i < read_count; i++){
						rx[i] += dx;
					}
				}
				if(ry){
					for(int i = 0; i < read_count; i++){
						ry[i] += dy;
					}
				}
				if(rz){
					for(int i = 0; i < read_count; i++){
						rz[i] += dz;
					}
				}
			}
		}
	}else{
		if(boxaxis){
			for(int i = 0; i < 12; i++){
					boxaxis[i] = 0.0;
			}
		}
	}

	//温度があるなら読み飛ばす
	if(m_flag & FAD_FLAG_TEMPERATURE){
		
		filepos += sizeof(float) * 2 + DUMMY_SIZE * 2;
			
	}

	//ステップ数の情報があるなら読み飛ばす
	if(m_flag & FAD_FLAG_STEP){

		filepos += sizeof(float) * 3 + DUMMY_SIZE * 2;
	}
	
	//次のフレームへファイルポインタを移動.
	filepos -= DUMMY_SIZE;
	fseek(m_fp, filepos, SEEK_SET);

	return read_count;

}

//xxx, yyy, zzzのデータを読んで, xyz, xyz...の配列に入れる.
void FAD_Reader::ReadFloat3AsVector3D(long filepos, int read_count, int offset, double* v3d){
	
	if(m_buffer4_sz < read_count){
		delete [] m_buffer4;
		m_buffer4 = new float[read_count];
		m_buffer4_sz = read_count;
	}

	long pos = filepos + offset * sizeof(float);

	fseek(m_fp, pos, SEEK_SET);
	fread(m_buffer4, sizeof(float) * read_count, 1, m_fp);

	int k = 0;
	for(int i = 0; i < read_count; i++){
		v3d[k] = (double)m_buffer4[i];
		k += 3;
	}

	pos += m_pcnt * sizeof(float) + DUMMY_SIZE * 2;
	fseek(m_fp, pos, SEEK_SET);
	fread(m_buffer4, sizeof(float) * read_count, 1, m_fp);

	k = 1;
	for(int i = 0; i < read_count; i++){
		v3d[k] = (double)m_buffer4[i];
		k += 3;
	}

	pos += m_pcnt * sizeof(float) + DUMMY_SIZE * 2;
	fseek(m_fp, pos, SEEK_SET);
	fread(m_buffer4, sizeof(float) * read_count, 1, m_fp);

	k = 2;
	for(int i = 0; i < read_count; i++){
		v3d[k] = (double)m_buffer4[i];
		k += 3;
	}

}


//xxxデータを読んで, xxxの配列に入れる.
void FAD_Reader::ReadFloatAsDouble(long filepos, int read_count, int offset, double* xxx){
	
	if(m_buffer4_sz < read_count){
		delete [] m_buffer4;
		m_buffer4 = new float[read_count];
		m_buffer4_sz = read_count;
	}

	fseek(m_fp, filepos + offset * sizeof(float), SEEK_SET);
	fread(m_buffer4, sizeof(float) * read_count, 1, m_fp);

	
	for(int i = 0; i < read_count; i++){
		xxx[i] = (double)m_buffer4[i];
	}

}



int FAD_Reader::Read(int count, int offset, int* knd, int* id, double* r, double* p, double *boxaxis){

	return _Read(count, offset, knd, id, r, NULL, NULL, NULL, p, NULL, NULL, NULL, boxaxis);

}

int FAD_Reader::Read(int count, int offset, int* knd, int* id, double* rx, double* ry, double* rz, double* px, double* py, double* pz, double *boxaxis){

	return _Read(count, offset, knd, id, NULL, rx, ry, rz, NULL, px, py, pz, boxaxis);

}
*/