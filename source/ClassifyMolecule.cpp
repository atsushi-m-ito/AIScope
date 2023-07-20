
#include "OutCoordinationNumber.h"

#include "ATOMS_DATA.h"
#include "BOND_INFO.h"

#include <tchar.h>
#include <stdio.h>

#include "IPotential.h"


#pragma warning(disable : 4996)

extern int g_periodic;

//static const double BOND_LENGTH = 1.85;


static const int SPECIES_COUNT = 118 + 1;

//OpenMXで入力する元素記号//
static const TCHAR* openmx_species_list_t[] = {
		_T("E"),		//empty軌道//
		_T("H"), _T("He"),	//第1周期
		_T("Li"), _T("Be"), _T("B"), _T("C"), _T("N"), _T("O"), _T("F"), _T("Ne"),		//第2周期
		_T("Na"), _T("Mg"), _T("Al"), _T("Si"), _T("P"), _T("S"), _T("Cl"), _T("Ar"),	//第3周期
		_T("K"), _T("Ca"), _T("Sc"), _T("Ti"), _T("V"), _T("Cr"), _T("Mn"), _T("Fe"), _T("Co"), _T("Ni"), _T("Cu"), _T("Zn"), _T("Ga"), _T("Ge"), _T("As"), _T("Se"), _T("Br"), _T("Kr"),	//第4周期
		_T("Rb"), _T("Sr"), _T("Y"), _T("Zr"), _T("Nb"), _T("Mo"), _T("Tc"), _T("Ru"), _T("Rh"), _T("Pd"), _T("Ag"), _T("Cd"), _T("In"), _T("Sn"), _T("Sb"), _T("Te"), _T("I"), _T("Xe"),	//第5周期
		_T("Cs"), _T("Ba"),	//第6周期(Cs, Ba)
		_T("La"), _T("Ce"), _T("Pr"), _T("Nd"), _T("Pm"), _T("Sm"), _T("Eu"), _T("Gd"), _T("Tb"), _T("Dy"), _T("Ho"), _T("Er"), _T("Tm"), _T("Yb"), _T("Lu"),	////第6周期(ランタノイド)
		_T("Hf"), _T("Ta"), _T("W"), _T("Re"), _T("Os"), _T("Ir"), _T("Pt"), _T("Au"), _T("Hg"), _T("Tl"), _T("Pb"), _T("Bi"), _T("Po"), _T("At"), _T("Rn"),//第6周期(Hf-Rn)
		_T("Fr"), _T("Ra"),	//第7周期(Fr, Ra)
		_T("Ac"), _T("Th"), _T("Pa"), _T("U"), _T("Np"), _T("Pu"), _T("Am"), _T("Cm"), _T("Bk"), _T("Cf"), _T("Es"), _T("Fm"), _T("Md"), _T("No"), _T("Lr"),	////第7周期(アクチノイド)
		//104,105,106,107,108,109,110,111,112,113,114,115,116,117,118,//第7周期(Rf-Uuo)
};


struct MOLECULE{
	int size;
	struct NUM_ELEMENT{
		int Z;
		int num;
	}* elements;
public: 	
	MOLECULE(int sz) : size(sz){
		elements = new NUM_ELEMENT[sz];
		memset(elements, 0, sizeof(NUM_ELEMENT) * sz);
	};

	~MOLECULE(){
		delete [] elements;	
	};
};

static int cmp(const MOLECULE& a, const MOLECULE& b){
	if(a.size == b.size){
		int flag = 1;
		for(int i = 0; i < a.size; i++){
			if(a.elements[i].Z != b.elements[i].Z){
				flag = 0;
				break;
			}else if(a.elements[i].num != b.elements[i].num){
				flag = 0;
				break;
			}
		}
		return flag;
	}
	return 0;
}

/*
//分子構造
struct MolType{
	int count;
	int Cn;
	int Hn;
	int Xn;
	int size;
	MolType* next;
};

static MolType* CreateMolType(ATOMS_DATA *dat, BOND_INFO* bond);
static void DeleteMolType(MolType* moltype_fst);
static int* AI_CreateMolChaine(ATOMS_DATA *dat, BOND_INFO* bond);
*/


int* AI_CreateMolChaine(ATOMS_DATA *dat, BOND_INFO* bond){
	//MolListの仕様
	//同一分子が粒子番号チェーンとなっている。
	//i番目の要素には粒子iが繋がる次の粒子の番号が入る。
	//ただし、チェーンの繋ぎ目として、最後粒子の場合は
	// - (頭の粒子番号 + 1) < 0
	//を要素とする。
	//チェーンの頭を取得するには、負の値を持つ要素を探し、
	//その値に対し、上の逆変換
	// - (負の値) - 1
	//が頭の粒子番号となる。
	
	

	int *pNext = new int[dat->pcnt];
	for (int i = 0; i < dat->pcnt; i++){
		pNext[i] = -(i+1);
	}	
	
	//
	//チェーン構造の作成
	//
	
	
	for (VECTOR_IJ* b = bond->bvector; b != bond->bvector + bond->count;b++){
		int li = b->i;
		int lj = b->j;
				
		
		//printf("check %d: (%d -- %d)", i, li, lj);
		
		
		//liと繋がってる最後の原子を取得
		while ( pNext[li] >= 0){
			li = pNext[li];
		}
		
		//printf("--");
		
		//ljと繋がってる最後の原子を取得
		while ( pNext[lj] >= 0){
			lj = pNext[lj];
		}


		if (li == lj){
		//	printf("--> same molecule\n");
			continue;
		}
		
		int head = -pNext[lj] -1;
		pNext[lj] = pNext[li];
		pNext[li] = head;
		
		//printf("--> (%d -- %d)\n", pNext[lj], lj);
		
	}

	//printf("finish to classify\n");

	return pNext;
	
}


int AI_WriteClassifyMolecule2(ATOMS_DATA *dat, BOND_INFO* bond, const TCHAR* filename){
	//MolListを元に、分子名を出力
	//
	//結果の表示
	//
	
	
	//結合している原子のチェーン構造データを作成
	int *pNext = AI_CreateMolChaine(dat, bond);
	if (pNext==NULL){
		return 0;
	}
	


	//分子数のカウント//
	
	int molCnt = 0;
	for (int i = 0; i < dat->pcnt; i++){
		//末尾ならheadの番号で符号を負にした値が入る
		if (pNext[i] < 0){ 
			molCnt ++;
		}
	}


	int* num_atom = new int[SPECIES_COUNT];//元素ごとの粒子数//
	int* knd = dat->knd;
	
	struct MOL_LIST{
		int count;
		MOLECULE* mol;
	};
	MOL_LIST* mol_list = new MOL_LIST[molCnt];
	
	int num_diff_mol = 0;
	for (int i = 0; i < dat->pcnt; i++){
		//末尾ならheadの番号で符号を負にした値が入る
		if (pNext[i] >=0){ continue;}
		
		memset(num_atom, 0, sizeof(int) * SPECIES_COUNT);
		int cntX = 0;

		int num_elements = 0;

		//一分子分の粒子チェーンでループ//
		int head = -pNext[i] -1;
		for (int li = head; li >= 0; li = pNext[li]){
			
			if( knd[li] < SPECIES_COUNT){
				if(num_atom[knd[li]] == 0){
					num_elements++;
				}
				num_atom[knd[li]] ++;
			}else{
				if(cntX == 0){
					num_elements++;
				}
				cntX ++;
			}
		}
	
		//MOLECULE構造体の作成//
		MOLECULE* mol = new MOLECULE(num_elements);
		int k = 0;
		for(int i = 0; i< SPECIES_COUNT; i++){
			if(num_atom[i]){
				mol->elements[k].Z = i;
				mol->elements[k].num = num_atom[i];
				//_ftprintf(fp, _T("%s%d"), openmx_species_list[i], num_element[i]);
				k++;
			}
		}
		if(cntX){
			mol->elements[k].Z = SPECIES_COUNT;
			mol->elements[k].num = cntX;
			k++;
		}

		{
			int i;
			for(i = 0; i < num_diff_mol; i++){
				if( cmp( *(mol_list[i].mol), *mol)){//既存の分子だった//
					mol_list[i].count++;
					delete mol;
					break;
				}
			}
			if(i == num_diff_mol){
		
				//新しい分子だった//
				mol_list[num_diff_mol].count = 1;
				mol_list[num_diff_mol].mol = mol;
				num_diff_mol++;
			
			}
		}
	}
	

	

	FILE *fp = _tfopen(filename, _T("w"));
	if (fp == NULL){
		printf("error; open file\n");
		return 0;
	}

	//分子の出力
	for(int i = 0; i < num_diff_mol; i++){
		const MOLECULE* mol = mol_list[i].mol;
		_ftprintf(fp, _T("%d\t"), mol_list[i].count);
		for(int k = 0; k < mol->size; k++){
			_ftprintf(fp, _T("%s%d"), openmx_species_list_t[mol->elements[k].Z], mol->elements[k].num);
		}
		_ftprintf(fp, _T("\n"));
	}


	fclose(fp);

	//delete//
	for(int i = 0; i < num_diff_mol; i++){
		delete mol_list[i].mol;
	}
	delete [] mol_list;
	
	return 1;
}

/*
int AI_WriteClassifyMolecule(ATOMS_DATA *dat, BOND_INFO* bond, const TCHAR* filename){
	//MolListを元に、分子名を出力
	//
	//結果の表示
	//
	FILE *fp;
	

	MolType* fstMT = CreateMolType(dat, bond);
	if (fstMT==NULL){
		return 0;
	}
	


	fp = _tfopen(filename, _T("w"));
	if (fp == NULL){
		printf("error; open file\n");
		return 0;
	}

		
	

	MolType* a = fstMT;
	while (a){

		printf("%d ", a->count);
		_ftprintf(fp, _T("%d "), a->count);
		
		if (a->Cn){
			printf("C%d", a->Cn);
			_ftprintf(fp, _T("C%d"), a->Cn);
		}
		if(a->Hn){
			printf("H%d", a->Hn);
			_ftprintf(fp, _T("H%d"), a->Hn);
		}
		if(a->Xn){
			printf("X%d", a->Xn);
			_ftprintf(fp, _T("X%d"), a->Xn);
		}
		printf("\n");
		_ftprintf(fp, _T("\n"));
		a = a->next;
	}

	fclose(fp);

	DeleteMolType(fstMT);

	return 1;
}




MolType* CreateMolType(ATOMS_DATA *dat, BOND_INFO* bond){
	//MolListを元に、分子構造(MolType)のリストを作る
	
	//int i, li, head;
	//
	//結果の表示
	//
	int cntC = 0;
	int cntH = 0;
	int cntX = 0;
	int molCnt = 0;

	
	
	MolType* fstMT = NULL;
	MolType* a = NULL;
	

	int *pNext = AI_CreateMolChaine(dat, bond);

	for (int i = 0; i < dat->pcnt; i++){
		//末尾ならheadの番号で符号を負にした値が入る
		if (pNext[i] >=0){ continue;}
		
		molCnt ++;
		cntC = 0;
		cntH = 0;
		cntX = 0;

		//繋がっている原子を表示
		int head = -pNext[i] -1;
		for (int li = head; li >= 0; li = pNext[li]){
			//printf(" %d,", li);
			switch( dat->knd[li] ){
			case KIND_H:
				cntH ++;
				break;
			case KIND_C:
				cntC ++;
				break;
			default:
				cntX ++;
				break;
			}
		}

		
		if(fstMT==NULL){
			fstMT = a = new MolType;
			a->next = NULL;
			a->Cn = cntC;
			a->Hn = cntH;
			a->Xn = cntX;
			a->count = 1;
		}else{
			a = fstMT;
			while (a){
				if( (a->Cn == cntC) && (a->Hn == cntH) && (a->Xn == cntX) ){
					a->count++;
					break;
				}
				
				if (a->next==NULL){
					a->next = new MolType;
					a = a->next;
					a->next = NULL;
					a->Cn = cntC;
					a->Hn = cntH;
					a->Xn = cntX;
					a->count = 1;
					break;
				}
				a = a->next;
			}
		}

	}

	delete [] pNext;

	return fstMT;


}


void DeleteMolType(MolType* moltype_fst){
	MolType* moltype = moltype_fst;
	while (moltype){
		moltype_fst = moltype;
		moltype = moltype->next;

		delete moltype_fst;
	}
}

*/

/*
int AI_nearestAtom(ATOMS_DATA *dat, float x, float y, float z, float limit_r){
	//粒子の一から半径limit_r以内をクリックした場合には粒子番号を返す
	//適合粒子がいないときには-1を返す
	
	int i;
	float dr2Min = limit_r*limit_r;
	float dr2;
	int idxMin=-1;
	vec3f* r;
	

	if (dat == NULL) {return -1;}

	r=dat->r;
	

	
	//glMaterialfv(GL_FRONT,GL_AMBIENT_AND_DIFFUSE,atomc[g_knd[i]]);
	for(i=0;i<dat->pcnt;i++){
		dr2 = (r[i].x-x)*(r[i].x-x) + (r[i].y-y)*(r[i].y-y) + (r[i].z-z)*(r[i].z-z);
		
		if (dr2 <= dr2Min){
			dr2Min = dr2;
			idxMin = i;
		}
	}
	
	return idxMin;

}
*/

/*
void WriteAdsH(MDLoader* a_atm, int frameskip, const char* spath){
	
	ATOMS_DATA *dat = a_atm->GetDataPointer();
	ATOMS_DATA *bfr_dat;
	int i,k;
	int* ml;
	int molCnt;
	int over2Cnt;	//2原子分子以上の数
	int CmolCnt;	//C原子を1つ以上含む分子の数
	int cntC,cntH,cntX;
	int adsH,lrgC;
	int head;

	int bfr_idx;
	int* vns_head;
	int* vns_no;
	int* vns_C;
	int* vns_H;
	int vns_cnt;
	int vns_CmolCnt=0;
	int vns_over2Cnt=0;
	int vanishflag;
	
	FILE *fp;

	
	if(dat->id == NULL){
		printf("data does not has index array\n");
		return;
	}

	

	fp = fopen(spath,"w");
	if( fp == NULL ){  // 関数が失敗していないか
		printf( "ファイルのオープンに失敗しました\n");
		return;  // 異常終了は０以外を返す
	}

	//系から出て行った分子の記憶用
	vns_head = new int[dat->pcnt];
	vns_no = new int[dat->pcnt];
	vns_C = new int[dat->pcnt];
	vns_H = new int[dat->pcnt];
	vns_cnt =0;



	while(1){
		
		ml = AI_CreateMolChaine(dat);
		adsH = 0;
		molCnt = 0;
		lrgC = 0;
		over2Cnt=0;
		CmolCnt=0;
		
		for (i = 0; i < dat->pcnt; i++){
			//分子リストの末端なら負の値
			if (ml[i] >=0){ continue;}
			molCnt ++;
			//printf("Molecule %d: ", molCnt);
			cntC = 0;
			cntH = 0;
			cntX = 0;

			//繋がっている原子をカウント
			head = -ml[i] -1;
			for (k = head; k >= 0; k = ml[k]){
				
				switch( dat->knd[k] ){
				case KIND_H:
					cntH ++;
					break;
				case KIND_C:
					cntC ++;
					break;
				default:
					cntX ++;
					break;
				}
			}

			if (cntC >= 100) {
				lrgC++;
				adsH += cntH;
			}

			if(cntC + cntH + cntX >=2){
				over2Cnt++;
			}

			if(cntC >0){
				CmolCnt++;
			}


		}

		//-------------------------
		//消えた分子を出力データに加える
		//-------------------------



		//-------------------------
		//データ出力
		//-------------------------

		printf("time = %d,\t N=%d, adsorbedH=%d\n",dat->istep,dat->pcnt,adsH);
		
		_ftprintf(fp, _T(" %d\t%d\t%d\t%d\t%d\t%d\t%d\t%f8.3\n"),
				dat->istep,		//timestep
				dat->pcnt,		//粒子数
				molCnt,			//全分子数(単一原子も含む)
				adsH,			//吸着水素原子数
				lrgC,			//C100以上の分子の数
				over2Cnt + vns_over2Cnt,		//2原子分子以上の数
				CmolCnt + vns_CmolCnt,		//炭素原子を一つ以上含む分子の数
				dat->tempave);	//温度
		

		if(frameskip > 1){
			if (a_atm->goNext(frameskip) == 0){
				
				delete [] ml;
				break;
			}
		}else{		
			if (a_atm->goNext() == 0){
				
				delete [] ml;
				break;
			}
		}
		bfr_dat = dat;
		dat = a_atm->GetDataPointer();


		
		//-----------------------------
		//消えた粒子のリストアップ
		//-----------------------------
		for(i=0;i<bfr_dat->pcnt;i++){
			bfr_idx = bfr_dat->id[i];
			vanishflag = 1;
			for(k=0;k<dat->pcnt;k++){
				if(bfr_idx == dat->id[k]){
					vanishflag = 0;
				}
			}

			//出て行った粒子の記憶
			if(vanishflag){

				///分子のheadの捜索
				head = i;
				while(ml[head]>=0){
					head = ml[head];
				}
				head = ml[head];

				//同じ分子が記憶されていないかチェック
				vanishflag = 1;
				for (k=vns_cnt-1;k!=0;k--){
					if (vns_no[k] != dat->no){
						break;
					}
					if(head == vns_head[k]){
						vanishflag=0;
					}
				}
				//分子の記憶
				if(vanishflag){
					vns_head[vns_cnt] = head;
					vns_no[vns_cnt] = dat->no;

					cntC = 0;
					cntH = 0;
					cntX = 0;
					head = -head -1;
					if(head==1267){
						ml[head] = ml[head];
					}
					for (k = head; k >= 0; k = ml[k]){
						
						switch( dat->knd[k] ){
						case KIND_H:
							cntH ++;
							break;
						case KIND_C:
							cntC ++;
							break;
						default:
							cntX ++;
							break;
						}
					}

					
					if(cntC + cntH + cntX >=2){
						vns_over2Cnt++;
					}

					if(cntC >0){
						vns_CmolCnt++;
					}


					vns_C[vns_cnt] = cntC;
					vns_H[vns_cnt] = cntH;
					vns_cnt++;

				}

			}
		}
			
				


		
		delete [] ml;
	}
	
	fclose(fp);
}
*/

	/*
int AI_WriteAdsorbedH(MDLoader* a_atm, int frameskip, const TCHAR* filename){
	//MolListを元に、分子名を出力
	//
	//結果の表示
	//
	FILE *fp;
	
#define MATERIAL_DEFINITION		(20)

	if(a_atm==NULL) { return 0;}

	


	fp = _tfopen(filename, _T("w"));
	if (fp == NULL){
		printf("error; open file\n");
		return 0;
	}
	_ftprintf(fp, _T("# step, adsorbed_H, T \n"));

	ATOMS_DATA *dat = a_atm->GetDataPointer();
	while(dat){
		
		int adsHcount = 0;


		MolType* fstMT = CreateMolType(dat);
		MolType* a = fstMT;
		while (a){

			if (a->Cn >= MATERIAL_DEFINITION){
				adsHcount += (a->Hn * a->count);
			}
			a = a->next;
		}
		DeleteMolType(fstMT);

		printf("adsorbed H: %d step, ads-H %d atoms, T = %7.3lf K\n", dat->istep,  adsHcount, dat->tempave);
		_ftprintf(fp,_T("%d\t%d\t%7.3lf\n"), dat->istep,  adsHcount, dat->tempave);


		if(frameskip > 1){
			if (a_atm->goNext(frameskip) == 0){
				break;
			}
		}else{		
			if (a_atm->goNext() == 0){
				break;
			}
		}
		dat = a_atm->GetDataPointer();
	}

	fclose(fp);


	return 1;
}
*/

/*
void AI_eraceBulk(ATOMS_DATA *dat, float limit_z){
//指定のz座標以下の粒子を消す
	

	int i;    //昇順に調べる
	int k;    //降順に調べる
	//float dz;
		
	i = 0;
	k = dat->pcnt-1;
		
	while(1){
	
		//昇順にボックス外の粒子を探す
		while (i <= k){
				
			if (dat->r[i].z < limit_z){
				break;
			}

			i = i + 1;
		}
			
		//降順にボックス内の粒子を探す
		while (k > i){
			if (dat->r[k].z >= limit_z){
				break;
			}
			k = k - 1;
		}	
		
		if (i >= k) {
			break;
		}
			
		//粒子を入れ替え
		dat->r[i] = dat->r[k];
		dat->p[i] = dat->p[k];
		dat->knd[i] = dat->knd[k];
			
		i = i + 1;
		k = k - 1;
	}
		
	dat->pcnt = i;
	
}
	


void AI_eraceUpper(ATOMS_DATA *dat, float limit_z){
//指定のz座標以下の粒子を消す
	

	int i;    //昇順に調べる
	int k;    //降順に調べる
	//float dz;
		
	i = 0;
	k = dat->pcnt-1;
		
	while(1){
	
		//昇順にボックス外の粒子を探す
		while (i <= k){
				
			if (dat->r[i].z >= limit_z){
				break;
			}

			i = i + 1;
		}
			
		//降順にボックス内の粒子を探す
		while (k > i){
			if (dat->r[k].z < limit_z){
				break;
			}
			k = k - 1;
		}	
		
		if (i >= k) {
			break;
		}
			
		//粒子を入れ替え
		dat->r[i] = dat->r[k];
		dat->p[i] = dat->p[k];
		dat->knd[i] = dat->knd[k];
			
		i = i + 1;
		k = k - 1;
	}
		
	dat->pcnt = i;
	
}
	


double fperiodic(double a, double size){
	double res;
	
	res = fmod(a,size);
	if (res*2.0>size){
		res -= size;
	}else if(res * -2.0 > size){
		res += size;
	}
	return res;

}

float fperiodic(float a, float size){
	float res;
	
	res = (float)fmod(a,size);
	if (res*2.0>size){
		res -= size;
	}else if(res * -2.0 > size){
		res += size;
	}
	return res;

}

*/