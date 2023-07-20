
#pragma warning ( disable : 4996 )
#include "fa2_reader.h"
#include <stdint.h>
#include <string.h>


static const long long HEADER_SIZE = 32;

FA2_Reader::FA2_Reader() : 
	m_state(0),
	m_readcount(0),
	m_current_frame_pos(0),
	m_fp(NULL)
{
		
}

FA2_Reader::~FA2_Reader(){
	this->Close();
}


int FA2_Reader::Open(const char* fileName){
	
	if(m_fp){
		return -2;
	}

	m_fp = fopen(fileName, "rb");
	
	if(m_fp){
		return 0;
	}else{
		return -1;
	}

}


void FA2_Reader::Close(){
	if(m_fp){
		fclose(m_fp);
		m_fp = NULL;
	}
}

long long FA2_Reader::GetFilePointer(){
	return _ftelli64(m_fp);
}

void FA2_Reader::SetFilePointer(long long offset){
	_fseeki64(m_fp, offset, SEEK_SET);
}

int FA2_Reader::GetParticleCount(){

	//header////////////////////////////////////////////////////
	if(m_state == 0){
		m_current_frame_pos = GetFilePointer();
		m_readcount = 0;

		size_t res = fread(m_header, 32, 1, m_fp);
		if(res==0){
			m_state = 0;
			return 0;
		}
		m_state = 1;	//header had been read//
	}

	int64_t count64 = *((int64_t*)m_header);
	return (int)count64;

}


int FA2_Reader::GetInfo(FA2_DATASIZE* data_size){

    //header////////////////////////////////////////////////////
    if (m_state == 1){
        
        data_size->box_size = m_header[8];
        data_size->kind_size = m_header[9];
        data_size->id_size = m_header[10];
        data_size->r_size = m_header[11];
        data_size->p_size = m_header[12];

        return 1;
    }

    return 0;

}


void FA2_Reader::Read(int* knd, int* id, double* r, double* p, double* boxaxisorg){
	
	if(m_state != 1){return;}

	int64_t count64 = *((int64_t*)m_header);
	int count = (int)count64;
	
	
	if(boxaxisorg){
		if (m_header[8] == sizeof(double)){
			fread(boxaxisorg, m_header[8]*12, 1, m_fp);
		}else{
			float* boxf = new float[12];
			
			fread(boxf, m_header[8]*12, 1, m_fp);
			for(int i = 0; i < 12; i++){
				boxaxisorg[i] = (double)(boxf[i]);
			}
			delete [] boxf;
		}
	}else{
		_fseeki64(m_fp, 12 * m_header[8], SEEK_CUR);
	}


	if(knd){
		if (m_header[9] == sizeof(int)){
			fread(knd, count64 * m_header[9], 1, m_fp);
		}else if (m_header[9] > 0){
			int8_t* pknd = new int8_t[count64];
			fread(pknd, count64 * m_header[9], 1, m_fp);
			for(int i = 0; i < count; i++){
				knd[i] = (int)(pknd[i]);
			}
			delete [] pknd;
		}
	}else{
		_fseeki64(m_fp, count64 * m_header[9], SEEK_CUR);
	}


	if(id){
		if (m_header[10] == sizeof(int)){
			fread(id, count64 * m_header[10], 1, m_fp);
		}else if (m_header[10] > 0){
			int64_t* pid = new int64_t[count64];
			fread(pid, count64 * m_header[10], 1, m_fp);
			for(int i = 0; i < count; i++){
				knd[i] = (int)(pid[i]);
			}
			delete [] pid;
		}
	}else{
		_fseeki64(m_fp, count64 * m_header[10], SEEK_CUR);
	}

	float* fbuf = NULL;
	if(r){
		if (m_header[11] == sizeof(double)){
			fread(r, count64 * m_header[11] * 3, 1, m_fp);
		}else if (m_header[11] > 0){
			fbuf = new float[count64 * 3];
			fread(fbuf, count64 * m_header[11] * 3, 1, m_fp);
			for(int i = 0; i < count * 3; i++){
				r[i] = (double)(fbuf[i]);
			}
			//delete [] dbuf;
		}
	}else{
		_fseeki64(m_fp, count64 * m_header[11] * 3, SEEK_CUR);
	}

	if(p){
		if (m_header[12] == sizeof(double)){
			fread(p, count64 * m_header[12] * 3, 1, m_fp);
		}else if (m_header[12] > 0){
			if(fbuf == NULL) fbuf = new float[count64 * 3];
			fread(fbuf, count64 * m_header[12] * 3, 1, m_fp);
			for(int i = 0; i < count * 3; i++){
				p[i] = (double)(fbuf[i]);
			}
			//delete [] dbuf;
		}
	}else{
		_fseeki64(m_fp, count64 * m_header[12] * 3, SEEK_CUR);
	}

	
	if(fbuf) delete [] fbuf;

	m_state = 0;	//finish to read the frame//
		
}



void FA2_Reader::ReadF(int* knd, int* id, float* r, float* p, float* boxaxisorg){
	
	if(m_state != 1){return;}

	int64_t count64 = *((int64_t*)m_header);
	int count = (int)count64;
	

	if(boxaxisorg){
		if (m_header[8] == sizeof(float)){
			fread(boxaxisorg, m_header[8]*12, 1, m_fp);
		}else{
			double* boxd = new double[12];
			
			fread(boxd, m_header[8]*12, 1, m_fp);
			for(int i = 0; i < 12; i++){
				boxaxisorg[i] = (float)(boxd[i]);
			}
			delete [] boxd;
		}
	}else{
		_fseeki64(m_fp, 12 * m_header[8], SEEK_CUR);
	}

	if(knd){
		if (m_header[9] == sizeof(int)){
			fread(knd, count64 * m_header[9], 1, m_fp);
		}else if (m_header[9] > 0){
			int8_t* pknd = new int8_t[count64];
			fread(pknd, count64 * m_header[9], 1, m_fp);
			for(int i = 0; i < count; i++){
				knd[i] = (int)(pknd[i]);
			}
			delete [] pknd;
		}
	}else{
		_fseeki64(m_fp, count64 * m_header[9], SEEK_CUR);
	}

	if(id){
		if (m_header[10] == sizeof(int)){
			fread(id, count64 * m_header[10], 1, m_fp);
		}else if (m_header[10] > 0){
			int64_t* pid = new int64_t[count64];
			fread(pid, count64 * m_header[10], 1, m_fp);
			for(int i = 0; i < count; i++){
				knd[i] = (int)(pid[i]);
			}
			delete [] pid;
		}
	}else{
		_fseeki64(m_fp, count64 * m_header[10], SEEK_CUR);
	}

	double* dbuf = NULL;
	if(r){
		if (m_header[11] == sizeof(float)){
			fread(r, count64 * m_header[11] * 3, 1, m_fp);
		}else if (m_header[11] > 0){
			dbuf = new double[count64 * 3];
			fread(dbuf, count64 * m_header[11] * 3, 1, m_fp);
			for(int i = 0; i < count * 3; i++){
				r[i] = (float)(dbuf[i]);
			}
			//delete [] dbuf;
		}
	}else{
		_fseeki64(m_fp, count64 * m_header[11] * 3, SEEK_CUR);
	}

	if(p){
		if (m_header[12] == sizeof(float)){
			fread(p, count64 * m_header[12] * 3, 1, m_fp);
		}else if (m_header[12] > 0){
			if(dbuf == NULL) dbuf = new double[count64 * 3];
			fread(dbuf, count64 * m_header[12] * 3, 1, m_fp);
			for(int i = 0; i < count * 3; i++){
				p[i] = (float)(dbuf[i]);
			}
			//delete [] dbuf;
		}
	}else{
		_fseeki64(m_fp, count64 * m_header[12] * 3, SEEK_CUR);
	}

	
	if(dbuf) delete [] dbuf;

	m_state = 0;	//finish to read the frame//
}

void FA2_Reader::SkipFrame(){
	if(m_state != 1){ return;}
	
	int64_t count64 = *((int64_t*)m_header);
	int64_t skip_sz = 0;

	skip_sz += m_header[8]*12;
	skip_sz += count64 * (int64_t)(m_header[9]);
	skip_sz += count64 * (int64_t)(m_header[10]);
	skip_sz += count64 * (int64_t)(m_header[11]) * 3;
	skip_sz += count64 * (int64_t)(m_header[12]) * 3;
	//skip_sz += count64 * (int64_t)(m_header[13]) * 3;
	
	_fseeki64(m_fp, skip_sz, SEEK_CUR);

	m_state = 0;	//finish to read the frame//
}

/*
指定した数だけ読み込み
読み込めた数が返す.
boxaxisorgは初回のみ読み込める.
*/
int FA2_Reader::ReadPart(int count, int* knd, int* id, double* r, double* p, double* boxaxisorg){
	
	if(m_state != 1){return 0;}

	int64_t count64 = *((int64_t*)m_header);
	int total = (int)count64;
	//long long range = count;

	if(m_readcount >= total ){//既に全部読んでいる.//
		
		m_state = 0;
		return 0;
	}else{
		
		if(count > total - m_readcount){
			count = total - m_readcount;
		}

		long long data_start = m_current_frame_pos + HEADER_SIZE;
		if(m_readcount > 0){
			data_start += 12 * m_header[8];
		}

		SetFilePointer(data_start);

	}

	long long jump_count = count64 - m_readcount - count;


	
	if(m_readcount == 0){
		if(boxaxisorg ){
			if (m_header[8] == sizeof(double)){
				fread(boxaxisorg, m_header[8]*12, 1, m_fp);
			}else{
				float* boxf = new float[12];
			
				fread(boxf, m_header[8]*12, 1, m_fp);
				for(int i = 0; i < 12; i++){
					boxaxisorg[i] = (double)(boxf[i]);
				}
				delete [] boxf;
			}
		}else{
			_fseeki64(m_fp, 12 * m_header[8], SEEK_CUR);
		}
	}

	if(knd){
		_fseeki64(m_fp, m_readcount * m_header[9], SEEK_CUR);
		if (m_header[9] == sizeof(int)){
			fread(knd, count * m_header[9], 1, m_fp);
		}else if (m_header[9] > 0){
			int8_t* pknd = new int8_t[count];
			fread(pknd, count * m_header[9], 1, m_fp);
			for(int i = 0; i < count; i++){
				knd[i] = (int)(pknd[i]);
			}
			delete [] pknd;
		}
		_fseeki64(m_fp, jump_count * m_header[9], SEEK_CUR);
	}else{
		_fseeki64(m_fp, count64 * m_header[9], SEEK_CUR);
	}
	


	if(id){
		_fseeki64(m_fp, m_readcount * m_header[10], SEEK_CUR);
		if (m_header[10] == sizeof(int)){
			fread(id, count * m_header[10], 1, m_fp);
		}else if (m_header[10] > 0){
			int64_t* pid = new int64_t[count];
			fread(pid, count * m_header[10], 1, m_fp);
			for(int i = 0; i < count; i++){
				knd[i] = (int)(pid[i]);
			}
			delete [] pid;
		}
		_fseeki64(m_fp, jump_count * m_header[10], SEEK_CUR);
	}else{
		_fseeki64(m_fp, count64 * m_header[10], SEEK_CUR);
	}

	float* fbuf = NULL;
	if(r){
		_fseeki64(m_fp, m_readcount * m_header[11] * 3, SEEK_CUR);
		if (m_header[11] == sizeof(double)){
			fread(r, count * m_header[11] * 3, 1, m_fp);
		}else if (m_header[11] > 0){
			fbuf = new float[count * 3];
			fread(fbuf, count * m_header[11] * 3, 1, m_fp);
			for(int i = 0; i < count * 3; i++){
				r[i] = (double)(fbuf[i]);
			}
			//delete [] dbuf;
		}
		_fseeki64(m_fp, jump_count * m_header[11] * 3, SEEK_CUR);
	}else{
		_fseeki64(m_fp, count64 * m_header[11] * 3, SEEK_CUR);
	}

	if(p){
		_fseeki64(m_fp, m_readcount * m_header[12] * 3, SEEK_CUR);
		if (m_header[12] == sizeof(double)){
			fread(p, count * m_header[12] * 3, 1, m_fp);
		}else if (m_header[12] > 0){
			if(fbuf == NULL) fbuf = new float[count * 3];
			fread(fbuf, count * m_header[12] * 3, 1, m_fp);
			for(int i = 0; i < count * 3; i++){
				p[i] = (double)(fbuf[i]);
			}
			//delete [] dbuf;
		}
		_fseeki64(m_fp, jump_count * m_header[12] * 3, SEEK_CUR);
	}else{
		_fseeki64(m_fp, count64 * m_header[12] * 3, SEEK_CUR);
	}

	
	if(fbuf) delete [] fbuf;

	m_readcount += count;
	if(m_readcount == total){
		m_state = 0;
	}
	return count;
	
		
}
