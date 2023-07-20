#include <stdlib.h>
#include <string.h>
#include "spk_reader.h"



const char SPK_Reader::DELIMITER[] = " \t\r\n";

SPK_Reader::SPK_Reader() :
	m_fp(NULL)
{


}

SPK_Reader::~SPK_Reader() {
	Close();

}


int SPK_Reader::Open(const char* file_path) {
	
	if (m_fp) {
		return -2;
	}

	m_fp = fopen(file_path, "r");
	if (!m_fp) {
		m_fp = NULL;
		return -1;
	}


	//ヘッダ部だけ読み込み

	char line[LINE_LEN];
	if (fgets(line, LINE_LEN, m_fp) != NULL) {
		sscanf(line, "%d %d", &m_num_sites, &m_num_paths);
		if (m_num_sites <= 0) { return -3; }

	}
	
	return 0;
}

void SPK_Reader::Close() {

	if (m_fp) {
		fclose(m_fp);
		m_fp = NULL;
	}

}

int SPK_Reader::GetNumSites() {
	return m_num_sites;
}

int SPK_Reader::GetNumPaths() {
	return m_num_paths;
}

void SPK_Reader::Read(KMCSite* sites, KMCPath* paths, double* box_axis_org) {

	char line[LINE_LEN];

	//コメント行の読み出し//
	if (fgets(line, LINE_LEN, m_fp) == NULL) {
		m_num_sites = -2;
		return;
	}
		
	//BOX行の読み出し//
	if (fgets(line, LINE_LEN, m_fp) == NULL) {
		m_num_sites = -3;
		return;
	}


	char* tp = strtok(line, DELIMITER);
	if (tp == NULL) {
		m_num_sites = -3;
		return;
	}

	if (strcmp(tp, "BOX") == 0) {

		for (int i = 0; i < 12; ++i) {
			tp = strtok(NULL, DELIMITER);
			if (tp) {
				box_axis_org[i] = strtod(tp, NULL);
			}
		}
	}

	//SITEの読み込み//	
	for(int i = 0; i < m_num_sites; ++i){
		if (fgets(line, LINE_LEN, m_fp) == NULL) {
			m_num_sites = i;
			return;
		}

		char* tp = strtok(line, " \t\r\n");
		if (tp == NULL) {
			m_num_sites = i;
			return;
		}


		if (strcmp(tp, "SITE") != 0) {
			m_num_sites = i;
			return;
		}


		tp = strtok(NULL, " \t\r\n");
		sites[i].position.x = strtod(tp, NULL);
		tp = strtok(NULL, " \t\r\n");
		sites[i].position.y = strtod(tp, NULL);
		tp = strtok(NULL, " \t\r\n");
		sites[i].position.z = strtod(tp, NULL);

		sites[i].site_id = i;
		
	}

	//PATHの読み込み//	
	for (int i = 0; i < m_num_paths; ++i) {
		if (fgets(line, LINE_LEN, m_fp) == NULL) {
			m_num_sites = i;
			return;
		}

		char* tp = strtok(line, " \t\r\n");
		if (tp == NULL) {
			m_num_sites = i;
			return;
		}


		if (strcmp(tp, "PATH") != 0) {
			m_num_sites = i;
			return;
		}


		tp = strtok(NULL, " \t\r\n");
		paths[i].src_site_id = atoi(tp);
		tp = strtok(NULL, " \t\r\n");
		paths[i].dst_site_id = atoi(tp);
		tp = strtok(NULL, " \t\r\n");
		paths[i].E_migration_barrier = strtod(tp, NULL);
		tp = strtok(NULL, " \t\r\n"); 
		paths[i].E_bind = strtod(tp, NULL);		
	}

	return;

}
