


#include "atomarray.h"

#include <stdio.h>
#include <string.h>



AtomArray::AtomArray():
	m_buffersize(0),
	m_maxID(1),
	count(0),
	knd(NULL),
	id(NULL),
	mass(NULL),
	r(NULL),
	p(NULL),
	f(NULL),
	pot(NULL)
{

};
	
AtomArray::AtomArray(int buffersize):
	m_buffersize(buffersize),
	m_maxID(1),
	count(0)
{
	this->knd = new int[buffersize];
	this->id = new int[buffersize];
	this->mass = new double [buffersize];
	this->r = new vec3d[buffersize];
	memset(this->r, 0, buffersize * sizeof(vec3d));
	this->p = new vec3d[buffersize];
	memset(this->p, 0, buffersize * sizeof(vec3d));
	this->f = new vec3d[buffersize];
	memset(this->f, 0, buffersize * sizeof(vec3d));
	this->pot = new double[buffersize];
	memset(this->pot, 0, buffersize * sizeof(double));
	
};
	


AtomArray::~AtomArray(){
	
	this->count = 0;
	delete [] this->knd;
	this->knd = NULL;
	delete [] this->id;
	this->id = NULL;
	delete [] this->mass;
	this->mass = NULL;
	delete [] this->r;
	this->r = NULL;
	delete [] this->p;
	this->p = NULL;
	delete [] this->f;
	this->f = NULL;
	delete [] this->pot;
	this->pot = NULL;
}



void AtomArray::ForceClear(){
	/*
//#pragma omp master
	{
	memset(this->f, 0, (this->count) * sizeof(vec3d));
	memset(this->pot, 0, (this->count) * sizeof(double));
	}
//#pragma omp barrier
*/

	
	
#pragma omp parallel
	{
		{
			const int i_end = (sizeof(vec3d) / sizeof(double)) * (this->count); 
			double* d_f = (double*)(this->f);
			#pragma omp for
			for(int i = 0; i < i_end; i++){
				d_f[i] = 0.0;
			}
		}
		{
			const int i_end = this->count;
			double* d_pot = (double*)(this->pot);
			#pragma omp for
			for(int i = 0; i < i_end; i++){
				d_pot[i] = 0.0;
			}
		}
	}

}


void AtomArray::CopyAtom(int indexDst, int indexSrc){

	this->knd[indexDst] = this->knd[indexSrc];
	this->mass[indexDst] = this->mass[indexSrc];
	this->id[indexDst] = this->id[indexSrc];
	
	this->r[indexDst] = this->r[indexSrc];
	this->p[indexDst] = this->p[indexSrc];
	this->f[indexDst] = this->f[indexSrc];
	this->pot[indexDst] = this->pot[indexSrc];
	
}


void AtomArray::SwapAtom(int indexDst, int indexSrc){

	int dummy = this->knd[indexDst];
	this->knd[indexDst] = this->knd[indexSrc];
	this->knd[indexSrc] = dummy;

	double dumdbl = this->mass[indexDst];
	this->mass[indexDst] = this->mass[indexSrc];
	this->mass[indexSrc] = dumdbl;

	dummy = this->id[indexDst];
	this->id[indexDst] = this->id[indexSrc];
	this->id[indexSrc] = dummy;
	
	vec3d dumv3d = this->r[indexDst];
	this->r[indexDst] = this->r[indexSrc];
	this->r[indexSrc] = dumv3d;

	dumv3d = this->p[indexDst];
	this->p[indexDst] = this->p[indexSrc];
	this->p[indexSrc] = dumv3d;

	dumv3d = this->f[indexDst];
	this->f[indexDst] = this->f[indexSrc];
	this->f[indexSrc] = dumv3d;

	dumdbl = this->pot[indexDst];
	this->pot[indexDst] = this->pot[indexSrc];
	this->pot[indexSrc] = dumdbl;
	
}

void AtomArray::CheckByIndex(int index){
	id[index] |= 0x80000000;

}

int AtomArray::SortCheck(){
	//listに収められたindexの原子にチェックをつける.
	//チェック状態の粒子はidの最上位bitが立っている.
	//チェックされた粒子を配列の最後に移動させる,
	//未チェック状態の粒子数を返す//
	//全て非チェック状態に戻す.

	
	//チェックされた粒子を配列の最後に移動させる.
	int i = -1;
	int k = count;
	while (i < k){

		//checkされたものを先頭から探索
		for( i ++ ; i < k; i++){
			if ( id[i] & 0x80000000 ) break;
		}
		
		//checkされていないものを最後尾から探索
		for( k--; i < k; k--){
			if ( (id[k] & 0x80000000) == 0) {
				SwapAtom(i, k);
				break;
			}
		}

	}

	//全て非チェック状態に戻す.
	for( k = i; k < count; k++){
		id[k] &= 0x7FFFFFFF;
	}

	return i;
}


int AtomArray::EjectCheckedElements(){
	//listに収められたindexの原子にチェックをつける.
	//チェック状態の粒子はidの最上位bitが立っている.
	//チェックされた粒子を配列の最後に移動させた後,
	//粒子数countを非チェック粒子の数(k)にし,
	//全て非チェック状態に戻す.

	int org_count = count;
	count = SortCheck();
	
	return org_count - count;
}

int AtomArray::GetIndex(int ID){
	for(int i = 0; i < count; i++){
		if(id[i] == ID){ return  i;}
	}
	return -1;
}

int AtomArray::GetIndex(int ID, int count_org){
	for(int i = 0; i < count_org; i++){
		if(id[i] == ID){ return  i;}
	}
	return -1;
}

int AtomArray::GetRealID(int index){
	return id[index] & 0x7FFFFFFF;
}


void AtomArray::Add(int knd_i, double mass_d, vec3d* r0, vec3d* p0){
		 
	m_maxID++;

	knd[count] = knd_i;
	id[count] = m_maxID;
	mass[count] = mass_d;
	r[count] = *r0;
	if(p0){
		p[count] = *p0;
	}else{
		p[count].Clear();
	}
	f[count].Clear();;			
	count ++;

}

void AtomArray::Add(int knd_i, double mass_d, const vec3d& r0, const vec3d& p0, const vec3d& f0){
		 
	m_maxID++;

	knd[count] = knd_i;
	id[count] = m_maxID;
	mass[count] = mass_d;
	r[count] = r0;
	p[count] = p0;
	f[count] = f0;			
	count ++;
}

void AtomArray::Add(int knd_i, double mass_d, const vec3d& r0, const vec3d& p0){
		 
	m_maxID++;

	knd[count] = knd_i;
	id[count] = m_maxID;
	mass[count] = mass_d;
	r[count] = r0;
	p[count] = p0;
	f[count].Clear();;			
	count ++;
}

void AtomArray::Add(int knd_i, double mass_d, const vec3d& r0){
		 
	m_maxID++;

	knd[count] = knd_i;
	id[count] = m_maxID;
	mass[count] = mass_d;
	r[count] = r0;
	p[count].Clear();
	f[count].Clear();			
	count ++;
}

double AtomArray::GetTotalKineticEnergy(){
		
		double kin = 0.0;
		const vec3d* lp = this->p;
		const double* lm = this->mass;
		
		const int l_count = this->count;

		#pragma omp parallel for reduction(+:kin)
		for(int i = 0; i < l_count; i++){
			kin += (lp[i] * lp[i]) / lm[i];
		}

		return kin * 0.5;
		
}

double AtomArray::GetTotalKineticEnergy(vec3d* K_v){
		
		double Kx = 0.0;
		double Ky = 0.0;
		double Kz = 0.0;
		const vec3d* lp = this->p;
		const double* lm = this->mass;
		
		const int l_count = this->count;

		#pragma omp parallel for reduction(+:Kx,Ky,Kz)
		for(int i = 0; i < l_count; i++){
			Kx += (lp[i].x * lp[i].x) / lm[i];
			Ky += (lp[i].y * lp[i].y) / lm[i];
			Kz += (lp[i].z * lp[i].z) / lm[i];
		}

		K_v->Set(Kx / 2.0, Ky / 2.0, Kz / 2.0);
		return (Kx + Ky + Kz) / 2.0;
		
}

double AtomArray::GetTotalPotentialEnergy(){
		
	
		double sum = 0.0;
		const double* l_pot = this->pot;
		const int l_count = this->count;
		
		#pragma omp parallel for reduction(+:sum)
		for(int i = 0; i < l_count; i++){
			sum += l_pot[i];
		}
		return sum * 0.5;

}

vec3d AtomArray::GetTotalForce(){
		
	vec3d frc = {0.0, 0.0, 0.0};
	for( int i = 0; i < count; i++){
		frc += f[i];
	}
	return frc;
};


int AtomArray::GetMaxID(){
	return m_maxID;
}

void AtomArray::IncrementMaxID(){
	m_maxID++;
}
