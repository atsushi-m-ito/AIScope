#pragma once

#include <tchar.h>
#include <string.h>
#include "MDLoader.h"
#include "spk_reader.h"
#include "mychardump.h"

class SPK_Container : public IMDContainer{
public:
	
	SPK_Container(const TCHAR* filepath):
		m_frame_num(0)
	{
		m_atm.pcnt = 0;

		if (_tcscmp(filepath + _tcslen(filepath) - 4, _T(".spk")) != 0) {
			m_frame_num = -1;
			return;
		}

		const char* cstr = MyCharDump<char, TCHAR>(filepath);
		Load(cstr);
		delete[] cstr;
	}

	virtual ~SPK_Container() {
		for (auto it = m_frame_data.begin(), it_end = m_frame_data.end(); it != it_end; ++it) {
			it->Clear();
		}
	}

	int GetFrameNum() {
		return 0;
	}

	int SetFrameNum(int frameNo) {
		return 0;
	}
	
	ATOMS_DATA* GetDataPointer() {
		if (m_frame_data.empty()) {
			return NULL;
		}

		//初回のデータ変換//
		if (m_atm.pcnt == 0) {
			m_atm.pcnt = m_frame_data[0].num_sites;
			m_atm.r = new vec3f[m_frame_data[0].num_sites];
			m_atm.knd = new int[m_frame_data[0].num_sites];
			m_atm.p = NULL;
			m_atm.frame = 0;
			m_atm.id = NULL;


			GetBoxaxis(&(m_atm.boxaxis), &(m_atm.boxorg));

			{
				const int i_end = m_frame_data[0].num_sites;
				KMCSite* sites = m_frame_data[0].sites;
				for (int i = 0; i < i_end; ++i) {
					m_atm.r[i] = sites[i].position;
				}
				for (int i = 0; i < i_end; ++i) {
					m_atm.knd[i] = 0;
				}
			}

			m_atm.bond.count = m_frame_data[0].num_paths;
			m_atm.bond.bvector = new VECTOR_IJ[m_frame_data[0].num_paths];
			{
				const int i_end = m_frame_data[0].num_paths;
				KMCPath* paths = m_frame_data[0].paths;
				VECTOR_IJ* bvector = m_atm.bond.bvector;
				const KMCSite* sites = m_frame_data[0].sites;

				const vec3d box_size(m_frame_data[0].box_axis_org[0],
					m_frame_data[0].box_axis_org[4],
					m_frame_data[0].box_axis_org[8]);
				const vec3d box_half(box_size / 2.0);

				int num_visible_paths = 0;
				for (int i = 0; i < i_end; ++i) {
					if (paths[i].dst_site_id >= 0) {
						VECTOR_IJ& bv = bvector[num_visible_paths];
						num_visible_paths++;

						bv.i = paths[i].src_site_id;
						bv.j = paths[i].dst_site_id;
						vec3d r_ji = sites[paths[i].dst_site_id].position - sites[paths[i].src_site_id].position;

						bv.v.Set(
							(r_ji.x > box_half.x) ? r_ji.x - box_size.x :
							(r_ji.x < -box_half.x) ? r_ji.x + box_size.x :
							r_ji.x,
							(r_ji.y > box_half.y) ? r_ji.y - box_size.y :
							(r_ji.y < -box_half.y) ? r_ji.y + box_size.y :
							r_ji.y,
							(r_ji.z > box_half.z) ? r_ji.z - box_size.z :
							(r_ji.z < -box_half.z) ? r_ji.z + box_size.z :
							r_ji.z);
					}
				}
				m_atm.bond.count = num_visible_paths;
			}
		}


		return &m_atm;
	}

	void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg) {
		if (m_frame_data.empty()) {
			return ;
		}

		boxaxis->Set(m_frame_data[0].box_axis_org);
		boxorg->Set(m_frame_data[0].box_axis_org[9],
			m_frame_data[0].box_axis_org[10],
			m_frame_data[0].box_axis_org[11]);
	}

	BOND_INFO* GetBond() {
		return &(m_atm.bond);
	}

	void SetBondCutoff(double cutoff_length) {
		//何もしない//
	}

private:

	class KMCSitePath {
	public:
		int num_sites;
		int num_paths;
		KMCSite* sites;
		KMCPath* paths;
		double box_axis_org[12];

		KMCSitePath(int num_sites_, int num_paths_) :
			num_sites(num_sites_), num_paths(num_paths_),
			sites(new KMCSite[num_sites_]),
			paths(new KMCPath[num_paths_])
		{

		}

		KMCSitePath(const KMCSitePath& src) :
			num_sites(src.num_sites), num_paths(src.num_paths),
			sites(src.sites), paths(src.paths)
		{
			memcpy(box_axis_org, src.box_axis_org, sizeof(double) * 12);
		}

		~KMCSitePath() = default;
		
		void Clear() {
			delete[] sites;
			delete[] paths;
		}

	};

	std::vector<KMCSitePath> m_frame_data;
	int m_frame_num;
	ATOMS_DATA m_atm;//dummy

	void Load(const char* filepath) {
		SPK_Reader reader;
		reader.Open(filepath);

		KMCSitePath kmc(reader.GetNumSites(), reader.GetNumPaths());
		
		reader.Read(kmc.sites, kmc.paths, kmc.box_axis_org);

		m_frame_data.push_back(kmc);
	}



};
