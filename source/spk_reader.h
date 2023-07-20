#pragma once
#include <stdio.h>
#include "kmc_path.h"
/* UTF8 BOMなし ヘッダーファイル */


class SPK_Reader {
public:
	SPK_Reader();
	virtual ~SPK_Reader();

	int Open(const char* file_path);
	void Close();

	int GetNumSites();
	int GetNumPaths();
	
	void Read(KMCSite* sites, KMCPath* paths, double* box_axis_org);
	
private:
	FILE* m_fp;
	static const int LINE_LEN = 1024;
	static const char DELIMITER[];
	int m_num_sites;
	int m_num_paths;

};
