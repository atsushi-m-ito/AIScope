
#ifndef filelist_h
#define filelist_h
#include <tchar.h>
#include <vector>


#include "MDLoader.h"
#include "Field3DLoader.h"
#include "trajectory.h"
#include "trajectory2.h"
#include "LMC_Holder.h"
#include "IPolyRepository.h"
#include "vmec_repository.h"
#include "krb_reader.h"


enum FILETYPE {
	FILETYPE_NULL,
	FILETYPE_ATOM,
	FILETYPE_TRAJ,
	FILETYPE_FIELD,
	FILETYPE_LMC,
	FILETYPE_PMD,
	FILETYPE_VMEC,
	FILETYPE_KARABA
};

#if 1
class LawData {

public:
	FILETYPE type;
	
	TCHAR* filepath;
	size_t dirlen;		//directoryの最後の\まで含んだ長さ
	TCHAR* filename;	//filepath中の先頭位置
	TCHAR* ext;			//filepath中の先頭位置

	Trajectory2 trajectory;
	//BOND_INFO bond;
private:

	IDataContainer* m_data;

public:
	LawData(const TCHAR* filepath_, IMDContainer* md) :
		type(FILETYPE_ATOM), filepath(_tcsdup(filepath_)), m_data(md)
	{
		AnalyzePath();
	}

	LawData(const TCHAR* filepath_, KRB_Reader* md) :
		type(FILETYPE_KARABA), filepath(_tcsdup(filepath_)), m_data(md)
	{
		AnalyzePath();
	}

	LawData(const TCHAR* filepath_, BCALoader* bca) :
		type(FILETYPE_TRAJ), filepath(_tcsdup(filepath_)), m_data(bca)
	{
		AnalyzePath();
	}

	LawData(const TCHAR* filepath_, Field3DLoader* field) :
		type(FILETYPE_FIELD), filepath(_tcsdup(filepath_)), m_data(field)
	{
		AnalyzePath();
		
	}

	LawData(const TCHAR* filepath_, LMC_Holder* lmc) :
		type(FILETYPE_LMC), filepath(_tcsdup(filepath_)), m_data(lmc)
	{
		AnalyzePath();
	}

	LawData(const TCHAR* filepath_, IPolyRepository* pmd) :
		type(FILETYPE_PMD), filepath(_tcsdup(filepath_)), m_data(pmd)
	{
		AnalyzePath();
	}

	LawData(const TCHAR* filepath_, VMEC_Repository* vmec) :
		type(FILETYPE_VMEC), filepath(_tcsdup(filepath_)), m_data(vmec)
	{
		AnalyzePath();
	}


	virtual ~LawData() {
		//ここではメモリ開放しない//
	}

	bool GetBoxSize(mat33d* axis, vec3d* org) {
		
		switch (this->type) {
			
		case FILETYPE_ATOM:
			dynamic_cast<IMDContainer*>(m_data)->GetBoxaxis(axis, org);
			return true;			
		case FILETYPE_KARABA:
			dynamic_cast<KRB_Reader*>(m_data)->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_TRAJ:
			dynamic_cast<BCALoader*>(m_data)->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_FIELD:
			dynamic_cast<Field3DLoader*>(m_data)->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_LMC:
			dynamic_cast<LMC_Holder*>(m_data)->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_PMD:
			dynamic_cast<IPolyRepository*>(m_data)->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_VMEC:
			dynamic_cast<VMEC_Repository*>(m_data)->GetBoxaxis(axis, org);
			return true;
		default:
			return false;
		}
	}

	ATOMS_DATA* GetDataPointer() {

		switch (this->type) {
		case FILETYPE_ATOM:
			return dynamic_cast<IMDContainer*>(m_data)->GetDataPointer();
		}

		return nullptr;
		
	}

	BOND_INFO* GetBond() {

		switch (this->type) {
		case FILETYPE_ATOM:
			return dynamic_cast<IMDContainer*>(m_data)->GetBond();
		}
		return nullptr;
	}

	int GetNumParticles() {

		switch (this->type) {
		case FILETYPE_ATOM:
			return dynamic_cast<IMDContainer*>(m_data)->GetDataPointer()->pcnt;
		}
		return 0;
	}


	KRB_INFO* GetDataPointerKRB() {

		switch (this->type) {
		case FILETYPE_KARABA:
			return dynamic_cast<KRB_Reader*>(m_data)->GetDataPointer();

		default:
			return nullptr;
		}
	}

	const TrajectoryData* GetTrajectory() {

		switch (this->type) {
		case FILETYPE_TRAJ:
			return dynamic_cast<BCALoader*>(m_data)->GetData();

		default:
			return nullptr;
		}
	}

	const FIELD3D_FRAME* GetField() {

		switch (this->type) {
		case FILETYPE_FIELD:
			return dynamic_cast<Field3DLoader*>(m_data)->GetFieldData();

		default:
			return nullptr;
		}
	}


	IPolyRepository* GetPolyRepository() {
		switch (this->type) {
		case FILETYPE_PMD:
			return dynamic_cast<IPolyRepository*>(m_data);
		default:
			return nullptr;
		}
	}


	VMEC_Repository* GetVmec() {
		switch (this->type) {
		case FILETYPE_VMEC:
			return dynamic_cast<VMEC_Repository*>(m_data);
		default:
			return nullptr;
		}
	}

	int SetFrameNum(int frame_number) {

		
		switch (this->type) {
		case FILETYPE_ATOM:
			return dynamic_cast<IMDContainer*>(m_data)->SetFrameNum(frame_number);
		case FILETYPE_KARABA:
			return dynamic_cast<KRB_Reader*>(m_data)->SetFrameNum(frame_number);
		case FILETYPE_TRAJ:
			return dynamic_cast<BCALoader*>(m_data)->SetFrameNum(frame_number);
		case FILETYPE_FIELD:
			return dynamic_cast<Field3DLoader*>(m_data)->SetFrameNum(frame_number);
		case FILETYPE_LMC:
			return dynamic_cast<LMC_Holder*>(m_data)->SetFrameNum(frame_number);
		case FILETYPE_PMD:
			return dynamic_cast<IPolyRepository*>(m_data)->SetFrameNum(frame_number);
		case FILETYPE_VMEC:
			return dynamic_cast<VMEC_Repository*>(m_data)->SetFrameNum(frame_number);
		default:
			return 0;
		}
	}

	int GetFrameNum() {

		
		switch (this->type) {
		case FILETYPE_ATOM:
			return dynamic_cast<IMDContainer*>(m_data)->GetFrameNum();
		case FILETYPE_KARABA:
			return dynamic_cast<KRB_Reader*>(m_data)->GetFrameNum();
		case FILETYPE_TRAJ:
			return dynamic_cast<BCALoader*>(m_data)->GetFrameNum();
		case FILETYPE_FIELD:
			return 0;// m_field_list[file_id]->GetFrameNum();
		case FILETYPE_LMC:
			return 0;// m_lmc_list[file_id]->GetFrameNum();
		case FILETYPE_PMD:
			return 0;// m_pmd_list[file_id]->GetFrameNum();
			break;
		case FILETYPE_VMEC:
			return 0; //m_vmec_list[file_id]->GetFrameNum();
			break;
		default:
			return 0;
		}
	}

private:

	

	void AnalyzePath() {
		//ファイル名,拡張子等の分解.
		TCHAR* p = _tcsrchr(filepath, _T('.'));
		ext = p + 1;
		p = _tcsrchr(filepath, _T('\\'));
		if (p) {
			dirlen = (int)(p - filepath) + 1;
			filename = p + 1;
		} else {
			dirlen = 0;
			filename = filepath;
		}
	};

	template<typename T>
	static void DeleteList(std::vector<T*> list) {
		for (auto it = list.begin(), it_end = list.end(); it != it_end; ++it) {
			delete (*it);
		}
	}
	/*
	template<typename T>
	void mReplaceToHead(vector<T*> list, int file_id) {
		delete (list[0]);
		list[0] = list[file_id];
	}
	*/
};



#else
class LawData {

public:
	FILETYPE type;
	int file_id;

	TCHAR* filepath;
	size_t dirlen;		//directoryの最後の\まで含んだ長さ
	TCHAR* filename;	//filepath中の先頭位置
	TCHAR* ext;			//filepath中の先頭位置
	
	Trajectory2 trajectory;
	//BOND_INFO bond;

public:
	LawData(const TCHAR* filepath_, IMDContainer* md) :
		type(FILETYPE_ATOM), filepath(_tcsdup(filepath_))
	{
		AnalyzePath();
		m_md_loader_list.push_back(md);
		file_id = m_md_loader_list.size() - 1;
	}
	
	LawData(const TCHAR* filepath_, KRB_Reader* md) :
		type(FILETYPE_KARABA), filepath(_tcsdup(filepath_))
	{
		AnalyzePath();
		m_karaba_list.push_back(md);
		file_id = m_karaba_list.size() - 1;
	}

	LawData(const TCHAR* filepath_, BCALoader* bca) :
		type(FILETYPE_TRAJ), filepath(_tcsdup(filepath_))
	{
		AnalyzePath();
		m_bca_list.push_back(bca);
		file_id = (int)m_bca_list.size() - 1;
	}

	LawData(const TCHAR* filepath_, Field3DLoader* field) :
		type(FILETYPE_FIELD), filepath(_tcsdup(filepath_))
	{
		AnalyzePath();
		m_field_list.push_back(field);
		file_id = (int)m_field_list.size() - 1;
	}

	LawData(const TCHAR* filepath_, LMC_Holder* lmc) :
		type(FILETYPE_LMC), filepath(_tcsdup(filepath_))
	{
		AnalyzePath();
		m_lmc_list.push_back(lmc);
		file_id = m_lmc_list.size() - 1;
	}

	LawData(const TCHAR* filepath_, IPolyRepository* pmd) :
		type(FILETYPE_PMD), filepath(_tcsdup(filepath_))
	{
		AnalyzePath();
		m_pmd_list.push_back(pmd);
		file_id = m_pmd_list.size() - 1;
	}

	LawData(const TCHAR* filepath_, VMEC_Repository* vmec) :
		type(FILETYPE_VMEC), filepath(_tcsdup(filepath_))
	{
		AnalyzePath();
		m_vmec_list.push_back(vmec);
		file_id = m_vmec_list.size() - 1;
	}


	virtual ~LawData() {
		//ここではメモリ開放しない//
	}

	bool GetBoxSize(mat33d* axis, vec3d* org) {

		switch (this->type) {
		case FILETYPE_ATOM:
			m_md_loader_list[file_id]->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_KARABA:
			m_karaba_list[file_id]->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_TRAJ:
			m_bca_list[file_id]->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_FIELD:
			m_field_list[file_id]->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_LMC:
			m_lmc_list[file_id]->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_PMD:
			m_pmd_list[file_id]->GetBoxaxis(axis, org);
			return true;
		case FILETYPE_VMEC:
			m_vmec_list[file_id]->GetBoxaxis(axis, org);
			return true;
		default:
			return false;
		}
	}

	ATOMS_DATA* GetDataPointer() {

		switch (this->type) {
		case FILETYPE_ATOM:
			return m_md_loader_list[file_id]->GetDataPointer();
		default:
			return NULL;
		}
	}

	BOND_INFO* GetBond() {

		switch (this->type) {
		case FILETYPE_ATOM:
			return m_md_loader_list[file_id]->GetBond();
		default:
			return NULL;
		}
	}


	KRB_INFO* GetDataPointerKRB() {

		switch (this->type) {
		case FILETYPE_KARABA:
			return m_karaba_list[file_id]->GetDataPointer();

		default:
			return NULL;
		}
	}

	const TrajectoryData* GetTrajectory() {

		switch (this->type) {
		case FILETYPE_TRAJ:
			return m_bca_list[file_id]->GetData();

		default:
			return NULL;
		}
	}

	IPolyRepository* GetPolyRepository() {
		switch (this->type) {
		case FILETYPE_PMD:
			return m_pmd_list[file_id];
		default:
			return NULL;
		}
	}


	VMEC_Repository* GetVmec() {
		switch (this->type) {
		case FILETYPE_VMEC:
			return m_vmec_list[file_id];
		default:
			return NULL;
		}
	}

	int SetFrameNum(int frame_number) {
		
		switch (this->type) {
		case FILETYPE_ATOM:
			return m_md_loader_list[file_id]->SetFrameNum(frame_number);
		case FILETYPE_KARABA:
			return m_karaba_list[file_id]->SetFrameNum(frame_number);
		case FILETYPE_TRAJ:
			return m_bca_list[file_id]->SetFrameNum(frame_number);
		case FILETYPE_FIELD:
			return m_field_list[file_id]->SetFrameNum(frame_number);
		case FILETYPE_LMC:
			return m_lmc_list[file_id]->SetFrameNum(frame_number);
		case FILETYPE_PMD:
			return m_pmd_list[file_id]->SetFrameNum(frame_number);
		case FILETYPE_VMEC:
			return m_vmec_list[file_id]->SetFrameNum(frame_number);
		default:
			return 0;
		}
	}

	int GetFrameNum() {
		switch (this->type) {
		case FILETYPE_ATOM:
			return m_md_loader_list[file_id]->GetFrameNum();
		case FILETYPE_KARABA:
			return m_karaba_list[file_id]->GetFrameNum();
		case FILETYPE_TRAJ:
			return m_bca_list[file_id]->GetFrameNum();
		case FILETYPE_FIELD:
			return 0;// m_field_list[file_id]->GetFrameNum();
		case FILETYPE_LMC:
			return 0;// m_lmc_list[file_id]->GetFrameNum();
		case FILETYPE_PMD:
			return 0;// m_pmd_list[file_id]->GetFrameNum();
			break;
		case FILETYPE_VMEC:
			return 0; //m_vmec_list[file_id]->GetFrameNum();
			break;
		default:
			return 0;
		}
	}


	void ReplaceToHead() {
		if (file_id == 0) {
			return;
		}

		switch (this->type) {
		case FILETYPE_ATOM:
			mReplaceToHead(m_md_loader_list, file_id);
			break;
		case FILETYPE_KARABA:
			mReplaceToHead(m_karaba_list, file_id);
			break;
		case FILETYPE_TRAJ:
			mReplaceToHead(m_bca_list, file_id);
			break;
		case FILETYPE_FIELD:
			mReplaceToHead(m_field_list, file_id);
			break;
		case FILETYPE_LMC:
			mReplaceToHead(m_lmc_list, file_id);
			break;
		case FILETYPE_PMD:
			mReplaceToHead(m_pmd_list, file_id);
			break;
		case FILETYPE_VMEC:
			mReplaceToHead(m_vmec_list, file_id);
			break;
		default:
			return;
		}
	}

	static void ClearAll() {
		DeleteList(m_md_loader_list);
		DeleteList(m_karaba_list);
		DeleteList(m_bca_list);
		DeleteList(m_field_list);
		DeleteList(m_lmc_list);
		DeleteList(m_pmd_list);
		DeleteList(m_vmec_list);
	}

private:

	static vector<IMDContainer*> m_md_loader_list;//for FILETYPE_ATOM//
	static vector<KRB_Reader*> m_karaba_list;
	static vector<BCALoader*> m_bca_list;
	
	static vector<Field3DLoader*> m_field_list;
	static vector<LMC_Holder*> m_lmc_list;
	static vector<IPolyRepository*> m_pmd_list;
	static vector<VMEC_Repository*> m_vmec_list;


	void AnalyzePath() {
		//ファイル名,拡張子等の分解.
		TCHAR* p = _tcsrchr(filepath, _T('.'));
		ext = p + 1;
		p = _tcsrchr(filepath, _T('\\'));
		if (p) {
			dirlen = (int)(p - filepath) + 1;
			filename = p + 1;
		} else {
			dirlen = 0;
			filename = filepath;
		}
	};
	
	template<typename T>
	static void DeleteList(vector<T*> list) {
		for (auto it = list.begin(), it_end = list.end(); it != it_end; ++it) {
			delete (*it);
		}
	}

	template<typename T>
	void mReplaceToHead(vector<T*> list, int file_id) {
		delete (list[0]);
		list[0] = list[file_id];
	}
	
};


#endif

#endif

