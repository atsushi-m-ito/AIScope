#pragma once
#include "vec3.h"
/* UTF8 BOMなし ヘッダーファイル */


struct KMCPath {
	int src_site_id;
	int dst_site_id;
	double E_migration_barrier;	//iからjに移動する際の移動障壁エネルギー(ただし周辺粒子が全くいない場合)//
	double E_bind;	//サイトi,jにHがいる場合の結合エネルギー//
	
};

struct KMCSite {
	int site_id;
	vec3d position;
};

