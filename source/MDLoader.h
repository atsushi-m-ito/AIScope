
#ifndef MDLoader_h
#define MDLoader_h

#pragma once

#include <cstdio>
#include <cstdint>
#include <vector>


#include "BOND_INFO.h"

#include "io64.h"
#include "mod.h"

#include "fa2_reader.h"
#include "fad_reader.h"


#include "ATOMS_DATA.h"

#include "IDataAtoms.h"


const long64 READLIMIT = 16*1024*1024;	// 64M


#define KIND_ALL (255)
#define MDL_FLAG_READBOND	 (1)



class IMDContainer : public IDataContainer {

public:

	virtual ~IMDContainer() = default;

	virtual int GetFrameNum() = 0;
	virtual int SetFrameNum(int frameNo) = 0;
	virtual ATOMS_DATA* GetDataPointer() = 0;
	virtual void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg) = 0;
	virtual BOND_INFO* GetBond() = 0;
	virtual void SetBondCutoff(double cutoff_length) = 0;
};

class MDLoader : public IMDContainer{

public:
	MDLoader() = default;

	MDLoader(const TCHAR *filename, double cutoff_length);
	virtual ~MDLoader();	
	
	int GetFrameNum();
	int SetFrameNum(int frameNo);

	ATOMS_DATA* GetDataPointer();

	void GetBoxaxis(mat33d* boxaxis, vec3d* boxorg);

	BOND_INFO* GetBond();
	void SetBondCutoff(double cutoff_length);

	bool AddFrameData(ATOMS_DATA* a);

private:

	struct FRAME_POS{
		long64 filepos;
		ATOMS_DATA* atmdat;
		int keep;
	};

typedef std::vector<FRAME_POS>	V_FRAME_POS;
typedef std::vector<FRAME_POS>::iterator	IT_FRAME_POS;

	V_FRAME_POS m_framev;	//キャッシュ//

	FA2_Reader* m_fa2 = nullptr;
	FAD_Reader* m_fad = nullptr;


	long64 m_unknown_pos;		//まだ読み込みを試していないファイルpos. 終端が見つかっているならLONG64_ZERO.
	//FILEPOS_LIST->next==NULL のとき
	//	unknown_pos == LONG64_ZERO なら, ファイル末尾
	//	unknown_pos != LONG64_ZERO なら, 読み込みを試す
	//		読み込み失敗なら, ファイル末尾としてunknown_posにLONG64_ZEROをセット
	//		読み込み成功なら, ファイル末尾ではないので, リストを付け足す, かつ, unknown_posに新しいファイルポインタをセット.


	int readMode=0;//0: バッファ処理できないファイル形式, 2: fad形式, 4: fa2形式
	int m_frameNo=0;			//現在の動画のフレームナンバー

	
	double m_cutoff_length=0.0;
	


	int FirstLoadTXT(FILE *fp, int mode);
	ATOMS_DATA* readFramePDB(FILE* fp, ATOMS_DATA* readBuf);
	ATOMS_DATA* readFrameXYZ(FILE* fp, ATOMS_DATA* readBuf);
	ATOMS_DATA* readFrameMD2(FILE* fp, ATOMS_DATA* readBuf);

	int FirstLoadMD3(const TCHAR* filepath);
	

	int FirstLoadFAD(const TCHAR* filepath);
	int ReadFrameFAD(FRAME_POS* framepos);
	
	int FirstLoadFA2(const TCHAR* filepath);
	int ReadFrameFA2(FRAME_POS* framepos);

	void clearBuffer();
	void DeleteFrame(ATOMS_DATA *dat);
	
	void Load(const TCHAR *filename);
	void UnloadStock();


	int goNext();
	int goNext(int num);
	int goPrev();
	int goPrev(int num);

};





#endif // !MDLoader_h

