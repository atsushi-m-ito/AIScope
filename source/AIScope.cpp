
//#define STEREO_BMP

#define  _CRT_NON_CONFORMING_SWPRINTFS
#include <tchar.h>
#include <windows.h>
#include <commctrl.h>
#include <chrono>
#include "AIScope.h"
#include "setting.h"
#include "winmain.h"
#include "visual.h"

#include "filelist.h"

#include "ResetBond.h"
#include "ResetColorByPressure.h"

#include "vmec_repository.h"
#include "mychardump.h"

#include "krb_reader.h"
#include "spk_container.h"

#include "PMDLoader.h"
#include "PMXLoader.h"
#include "MQOLoader.h"
#include "VRMLLoader.h"

extern bool g_updateFlag;
extern int g_visual_atom_trajectory;
static const double STEREO_EYELENGTH = 0.6;

//reference: http://marupeke296.com/DBG_No3_OutputDebugWindow.html
//#ifdef _DEBUG
#define MyOutputDebugString( str, ... ) \
      { \
        TCHAR c[256]; \
        _stprintf( c, str, __VA_ARGS__ ); \
        OutputDebugString( c ); \
      }
//#else
//#define MyOutputDebugString( str, ... ) // 空実装
//#endif

//vector<IMDContainer*> LawData::m_md_loader_list;
//vector<KRB_Reader*> LawData::m_karaba_list;
//vector<BCALoader*> LawData::m_bca_list;
//vector<Field3DLoader*> LawData::m_field_list;
//vector<LMC_Holder*> LawData::m_lmc_list;
//vector<IPolyRepository*> LawData::m_pmd_list;
//vector<VMEC_Repository*> LawData::m_vmec_list;


AIScope::AIScope(AIScopeProperty* aiproperty) :
	m_aiproperty(aiproperty),
	m_outNumBMP(0),
	m_selected_id(-1)
{


}

AIScope::~AIScope(){

}



int AIScope::Resize(int w, int h){
	m_client_w = w;
	m_client_h = h;

	return 1;
}

void AIScope::GetFocusInfomation(double* focus_distance, vec3d* target_position){
	if (!m_data_list.empty()) {
		mat33d axis;
		vec3d org;


		switch (m_data_list[0].type) {
		case FILETYPE_ATOM:
		case FILETYPE_KARABA:		
		case FILETYPE_TRAJ:
		case FILETYPE_FIELD:
		case FILETYPE_LMC:
		case FILETYPE_PMD:
		case FILETYPE_VMEC:
			
			m_data_list[0].GetBoxSize(&axis, &org);
			
			break;
			
		default:
						
			axis.Clear();
			axis.m11 = axis.m22 = axis.m33 = 10.0;
			org.Clear();
			break;
			
		}

		*focus_distance = 2.0 * (double)max(max(axis.m11, axis.m22), axis.m33);

		*target_position = ((axis.a + axis.b + axis.c) * 0.5 + org);
		return;
	}



}

	
int AIScope::OpenFile(const TCHAR* filepath, HWND hWnd){
	
	
	
	if (OpenDataFile(filepath, hWnd)) {

		switch (m_data_list.back().type) {
		case FILETYPE_ATOM:
			{
				//表示用のプロパティーの取得//
				RenderingProperty rendering_property;
				m_aiproperty->GetRenderingProperty(&rendering_property);
#if 0
				if ((rendering_property.atom_color == VISUAL_ATOMCOLOR_BONDNUM) || rendering_property.bond_draw) {
					glips_ResetBond(m_data_list.back().GetDataPointer(), m_data_list.back().GetBond(), g_bond_cutoff);
				}
#endif
				if ((rendering_property.atom_color == VISUAL_ATOMCOLOR_PRESSURE)) {
					//glips_ResetColorByPressure(finfo->atm->GetDataPointer(), &(finfo->bond));
					glips_ResetColorByPx(m_data_list.back().GetDataPointer(), m_data_list.back().GetBond());
				}
			}
			break;
		}


		

		return 1;
	} 

		
	return 0;
}


int AIScope::OpenDataFile(const TCHAR* filepath, HWND hWnd) {

	TCHAR* long_path = nullptr;
	

	//ショートネームからロングネームの取得
	unsigned int filenamelen = GetLongPathName(filepath, nullptr, 0);
	if (_tcslen(filepath) < filenamelen) {
		long_path = new TCHAR[filenamelen + 1];
		GetLongPathName(filepath, long_path, filenamelen + 1);
	} else {
		long_path = _tcsdup(filepath);
	}

	//ファイル名,拡張子等の分解.
	TCHAR* p = _tcsrchr(long_path, _T('.'));
	const TCHAR* file_ext = p + 1;
	

	//原子位置ファイルのとき.
	if ((_tcscmp(file_ext, _T("fad")) == 0)
		|| (_tcscmp(file_ext, _T("fa2")) == 0)
		|| (_tcscmp(file_ext, _T("xyz")) == 0)
		|| (_tcscmp(file_ext, _T("md2")) == 0)
		|| (_tcscmp(file_ext, _T("md3")) == 0)
		|| (_tcscmp(file_ext, _T("md")) == 0)
		|| (_tcscmp(file_ext, _T("pdb")) == 0)) {
		
		//finfo->atm = new MDLoader(finfo->filepath);
		MDLoader* md = new MDLoader(long_path, g_bond_cutoff);
		if (md->GetDataPointer() != NULL){
			m_data_list.push_back(LawData(long_path, md));			
		}

		//KMCSitePathのとき
	} else if (_tcscmp(file_ext, _T("spk")) == 0) {

		m_data_list.push_back(LawData(long_path, new SPK_Container(filepath)));
		


		//g_visual_traj = VISUAL_TRAJ_ON;

		//KMCトラジェクトリーファイルのとき
	} else if (_tcscmp(file_ext, _T("krb")) == 0) {

		char* filepath = MyCharDump<char, TCHAR>(long_path);
		KRB_Reader* krb = new KRB_Reader(g_trajectory_z_reverse);
		if (krb->Open(filepath) == 0) {
			m_data_list.push_back(LawData(long_path, krb));
		} else {
			delete krb;
		}
		delete[] filepath;


		//g_visual_traj = VISUAL_TRAJ_ON;

		//トラジェクトリーファイルのとき
	} else if ((_tcscmp(file_ext, _T("txt")) == 0)
		|| (_tcscmp(file_ext, _T("traj")) == 0)) {


		BCALoader* bca = new BCALoader(long_path);
		
		if (bca->GetData()) {
			m_data_list.push_back(LawData(long_path, bca));
		}else{
			MessageBox(NULL, _T("error:trajectory"), _T("error"), MB_OK);
			delete bca;
		}

		//g_visual_traj = VISUAL_TRAJ_ON;



		//PMDファイルのとき.
	} else if (_tcscmp(file_ext, _T("pmd")) == 0) {

		IPolyRepository* pmd = new PMDLoader(long_path);
		pmd->ExchangeYZ();
		m_data_list.push_back(LawData(long_path, pmd));

		//PMXファイルのとき.
	} else if (_tcscmp(file_ext, _T("pmx")) == 0) {

		IPolyRepository* pmd = new PMXLoader(long_path);
		pmd->ExchangeYZ();
		m_data_list.push_back(LawData(long_path, pmd));

		//MQOファイルのとき.
	} else if (_tcscmp(file_ext, _T("mqo")) == 0) {

		IPolyRepository* pmd = new MQOLoader(long_path);
		m_data_list.push_back(LawData(long_path, pmd));

		//VRMLファイルのとき.
	} else if (_tcscmp(file_ext, _T("wrl")) == 0) {

		IPolyRepository* pmd = new VRMLLoader(long_path);
		pmd->ExchangeYZ();
		m_data_list.push_back(LawData(long_path, pmd));

		//VMECファイルのとき.
	} else if (_tcscmp(file_ext, _T("vmec")) == 0) {
			

		VMEC_Repository* vmec = new VMEC_Repository(long_path);

		if (vmec->GetNumParticles() > 0) {
			m_data_list.push_back(LawData(long_path, vmec));
		} else {			
			delete vmec;
		}

		//cubeファイルのとき.
	} else if (_tcscmp(file_ext, _T("cube")) == 0){

		const auto start_tm = std::chrono::high_resolution_clock::now();

		Field3DLoader* field = new Field3DLoader();
		ATOMS_DATA dat;
		if (field->Load(long_path, &dat)) {
			
			m_data_list.push_back(LawData(long_path, field));
			MDLoader* md = new MDLoader();
			md->AddFrameData(&dat);
			md->SetBondCutoff(g_bond_cutoff);
			m_data_list.push_back(LawData(long_path, md));



			//便利関数///////////////////////////////
			auto isNumberChar = [](TCHAR c) {
				return (c >= _T('0')) && (c <= _T('9'));
			};
			auto isOnlyNumber = [&isNumberChar](const TCHAR* text, int i_begin, int i_end) {
				for (int i = i_begin; i < i_end; ++i) {
					if (!isNumberChar(text[i])) return false;
				}
				return true;
				};

			//連番ファイルを読み込む///////////////////////////////
			//ファイル名の数字をワイルドカードに変更//
			TCHAR* wild_path = _tcsdup(long_path);
			int pos_wild = -1;
			for (int i = _tcslen(wild_path) - 6; i >= 0; --i) {
				if (isNumberChar(wild_path[i])) {
					//wild_path[i] = _T('*');//数字1文字のワイルドカードに変更//
					pos_wild = i;
				} else {
					break;
				}
			}
			if (pos_wild >= 0) {
				//ワイルドカードで連番ファイルを探索//
				_tcscpy(wild_path + pos_wild, _T("*.cube"));
			
				//get directory and it is used as fullpath of found file
				TCHAR* tmp_path = new TCHAR[_tcslen(long_path) * 2];
				_tcscpy(tmp_path, long_path);
				const TCHAR* pend_dir = _tcsrchr(tmp_path, _T('\\'));
				const int ihead_filename = (pend_dir) ? (pend_dir - tmp_path) + 1 : 0;
				pos_wild = pos_wild - ihead_filename;

				WIN32_FIND_DATA info;
				auto hFind = FindFirstFile(wild_path, &info);
				int is_found = (hFind != INVALID_HANDLE_VALUE);
				
				HWND hPrgressBar = 0;
				if (is_found) {//プログレスバーコントロールの作成//
					RECT rect;
					GetWindowRect(hWnd, &rect);
					const LONG ww = rect.right - rect.left;
					const LONG wh = rect.bottom - rect.top;

					hPrgressBar = CreateWindowEx(0, PROGRESS_CLASS, TEXT("reading cube files"),
						WS_CHILD | WS_VISIBLE | WS_BORDER | PBS_SMOOTH,
						//WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CCS_NODIVIDER,
						ww / 10, (wh * 7)/20, (ww * 8) /10, (wh) / 20,
						hWnd, 0,
						(HINSTANCE)GetWindowLong(hWnd, GWLP_HINSTANCE), NULL);
					//ShowWindow(hPrgressBar, SW_SHOW);

					if (hPrgressBar) {
						//ウィンドウ生成成功//
						//ファイル数を取得
						int num_target_files = 0;

						bool is_found_me = false;
						while (is_found) {
							//found//
							if (_tcscmp(info.cFileName, long_path + ihead_filename) == 0) {
								is_found_me = true;
							} else {
								if (is_found_me) { //not の場合はまだ最初に開いたファイルよりも若い番号なので無視する//

									if (!isOnlyNumber(info.cFileName, pos_wild, _tcslen(info.cFileName) - 5)) {	//5 is length of ".cube"//
										break;//読み込み終了
									}

									//連番ファイルとして認めた//
									num_target_files++;

								}
							}							
							is_found = FindNextFile(hFind, &info);
						}


						SendMessage(hPrgressBar, PBM_SETRANGE32, 0, num_target_files);

						SendMessage(hPrgressBar, PBM_SETSTEP, (WPARAM)1, 0);

						//reset 
						hFind = FindFirstFile(wild_path, &info);
						is_found = (hFind != INVALID_HANDLE_VALUE);
					}
				}

				bool is_found_me = false;
				while (is_found) {
					//found//
					if (_tcscmp(info.cFileName, long_path + ihead_filename)==0){
						is_found_me = true;
					} else {
						if (is_found_me) { //not の場合はまだ最初に開いたファイルよりも若い番号なので無視する//
							
							if (!isOnlyNumber(info.cFileName, pos_wild, _tcslen(info.cFileName) - 5)) {	//5 is length of ".cube"//
								break;//読み込み終了
							}

							//連番ファイルとして認めた//
							_tcscpy(tmp_path + ihead_filename, info.cFileName);

							//ファイル読みこみ

							if (field->Load(tmp_path, &dat)) {
								md->AddFrameData(&dat);								
							}


							SendMessage(hPrgressBar, PBM_STEPIT, 0, 0);
						}
					}
					is_found = FindNextFile(hFind, &info);
				}

				if (hPrgressBar) {
					DestroyWindow(hPrgressBar);
				}

				delete[] tmp_path;
			}
			delete[] wild_path;

		}else{
			delete field;
		}
		
		const auto end_tm = std::chrono::high_resolution_clock::now();
		double elapse_time = (double)(std::chrono::duration_cast<std::chrono::duration<double>>(end_tm - start_tm).count());
		MyOutputDebugString(_T("file load time = %f[s]"), elapse_time);



		//fieldファイルのとき.
	} else if (_tcscmp(file_ext, _T("bif")) == 0) {

		Field3DLoader* field = new Field3DLoader();
		if (field->Load(long_path)) {
			m_data_list.push_back(LawData(long_path, field));
		} else {
			delete field;
		}



		//lattice monte-calro.
	} else if (_tcscmp(file_ext, _T("lmc")) == 0) {

		LMC_Holder* lmc = new LMC_Holder();

		if (lmc->Load(long_path)) {
			delete lmc;
		} else {
			m_data_list.push_back(LawData(long_path, lmc));
		}

	}

	delete[] long_path;

	return 1;

}

int AIScope::ReloadFile(HWND hWnd) {



	if (OpenDataFile(m_data_list[0].filepath, hWnd)) {
		/*
		もし動かないとするとこの処理を省いたための可能性がある
		m_data_list.back().ReplaceToHead();
		m_data_list.pop_back();
		*/


		ATOMS_DATA* dat = m_data_list[0].GetDataPointer();
		BOND_INFO* bond = m_data_list[0].GetBond();

		//表示用のプロパティーの取得//
		RenderingProperty rendering_property;
		m_aiproperty->GetRenderingProperty(&rendering_property);
#if 0
		if ((rendering_property.atom_color == VISUAL_ATOMCOLOR_BONDNUM) || rendering_property.bond_draw) {
			glips_ResetBond(dat, bond, g_bond_cutoff);
		}
#endif
		if ((rendering_property.atom_color == VISUAL_ATOMCOLOR_PRESSURE)) {
			glips_ResetColorByPx(dat, bond);
		}

		return 1;
	}
	//関数終了時にrefinfoと共にatmも破棄されるのでまずい//
	return 0;

}

void AIScope::DisplayGL15(const double* view_matrix, const double focus_distance){
	
	//表示用のプロパティーの取得//
	RenderingProperty rendering_property;
	m_aiproperty->GetRenderingProperty(&rendering_property);
	
	//レンダリング//
	RenderingGL15_2(m_data_list, m_client_w, m_client_h, 0, 0, m_client_w, m_client_h, view_matrix, focus_distance, rendering_property);
}

void AIScope::DisplayDX11(const double* view_matrix, const double focus_distance){
	//表示用のプロパティーの取得//
	RenderingProperty rendering_property;
	m_aiproperty->GetRenderingProperty(&rendering_property);

	//レンダリング//
	RenderingDX11(m_data_list, m_client_w, m_client_h, 0, 0, m_client_w, m_client_h, view_matrix, focus_distance, rendering_property);
	
}


/*
スクリーン座標(x,y)を元にVR空間中の原子を選択
*/
int AIScope::SelectAtom(const vec3d& ray, const vec3d& org){
			
	int sid = -1;


	
	if(! m_data_list.empty()){

		//vec3d vray, vorg;
		//m_camera->GetRAY(x, y, m_client_w, m_client_h, &vray, &vorg);
		
		RenderingProperty rendering_property;
		m_aiproperty->GetRenderingProperty(&rendering_property);
		const double cutoff = rendering_property.atom_radius;

		ATOMS_DATA *dat = m_data_list[0].GetDataPointer();
		
//		dat->r[0].x = (float)(vray.x + vorg.x);
//		dat->r[0].y = (float)(vray.y + vorg.y);
//		dat->r[0].z = (float)(vray.z + vorg.z);
		
		vec3d uray = Unit(ray);
		const vec3f* r = dat->r;
		int count = dat->pcnt;
		double min_dr2 = DBL_MAX;
		for(int i = 0; i <count; i++){
			vec3d ro;
			ro.x = (double)(r[i].x) - org.x;
			ro.y = (double)(r[i].y) - org.y;
			ro.z = (double)(r[i].z) - org.z;
			double len1 = ro * uray;
			double dr2 = (ro*ro) - len1*len1;
			if( dr2 < min_dr2 ) {
				min_dr2 = dr2;
				sid = i;
			}
		}
		
		if(min_dr2 > cutoff * cutoff){
			sid = -1;
		}else{
			
			m_selected_id = sid;
		}
		

	}
		
	return sid;
}



//指定したフレームまで全てのファイルのタイムフレームをジャンプさせる.
//実際に移動したフレームのmaximumを返す.
//minimumは0のはずなので
//戻り値(maxframe) < 引数(timeframe)なら最後のフレームに到達した証拠.
int AIScope::FrameJump(int timeframe){

	int maxframe = 0;
	


	for (auto it = m_data_list.begin(), it_end = m_data_list.end(); it != it_end; ++it) {
		int res;

		switch (it->type) {
		case FILETYPE_ATOM:
			{

				int prevno = it->GetDataPointer()->frame;
				res = it->SetFrameNum(timeframe);
				if (prevno != res) {

					//表示用のプロパティーの取得//
					RenderingProperty rendering_property;
					m_aiproperty->GetRenderingProperty(&rendering_property);


#if 0
					if ((rendering_property.atom_color == VISUAL_ATOMCOLOR_BONDNUM) || rendering_property.bond_draw) {
						glips_ResetBond(dat, &(dat->bond), g_bond_cutoff);
						//	fi->atomRenderer->ClearVRAM();
					}
#endif
					if ((rendering_property.atom_color == VISUAL_ATOMCOLOR_PRESSURE)) {

						ATOMS_DATA* dat = it->GetDataPointer();
						glips_ResetColorByPx(dat, &(dat->bond));
					}

					if (g_visual_atom_trajectory >= 0) {
						ATOMS_DATA* dat = it->GetDataPointer();
						if (g_visual_atom_trajectory < dat->pcnt) {
							it->trajectory.Add(dat->frame, dat->r[g_visual_atom_trajectory]);
						}
					}
				}
			}
			break;
		case FILETYPE_KARABA:
		case FILETYPE_TRAJ:
		case FILETYPE_LMC:
		case FILETYPE_FIELD:
		case FILETYPE_PMD:
		case FILETYPE_VMEC:
			res = it->SetFrameNum(timeframe);
			break;
		default:
			res = 0;

			break;
		}

		if (maxframe < res) maxframe = res;
	}


	g_updateFlag = true;
				

	return maxframe;
}


void AIScope::OutputStereoBMPGL(const double* view_matrix, const double focus_distance){

#if STEREO_BMP
	TCHAR spath[MAX_PATH];
	

	//右目用画像
	FileInfo* current = m_files[0];
	
	int length = (int)(current->ext - current->filepath - 1);
	_tcsncpy(spath, current->filepath, length);
	_stprintf(spath + length,_T("%d_right.bmp"), m_outNumBMP);
	//m_outNumBMP++;
	printf("%s\n",spath);

	m_camera->PushState();
	m_camera->SetStereoMode(2.0);
	double radius = m_camera->GetForcusDistance();
	double delta_aglxy = STEREO_EYELENGTH / radius;
	m_camera->RotatePosOnUpAxis(delta_aglxy);

	OutputBMfromGLMag(m_files, spath, m_client_w, m_client_h, 1, view_matrix, focus_distance);

	//左目用画像
	_stprintf(spath + length,_T("%d_left.bmp"), m_outNumBMP);
	m_outNumBMP++;
	printf("%s\n",spath);

	m_camera->RotatePosOnUpAxis(-2.0*delta_aglxy);
	
//				cam_info.focus_y -= width2 * cos(cam_info.aglxy);
//				cam_info.focus_x += width2 * sin(cam_info.aglxy);

	OutputBMfromGLMag(m_files, spath, m_client_w, m_client_h, 1, view_matrix, focus_distance);

	//g_camera->RotatePosOnUpAxis(delta_aglxy);
	m_camera->PullState();

#endif
}

void AIScope::OutputSequentialBMPGL(const double* view_matrix, const double focus_distance){

	//表示用のプロパティーの取得//
	RenderingProperty rendering_property;
	m_aiproperty->GetRenderingProperty(&rendering_property);

	//レンダリング//

	TCHAR spath[MAX_PATH];

	LawData& current = m_data_list[0];
	size_t length = current.ext - current.filepath - 1;
	_tcsncpy(spath, current.filepath, length);
		

		
	int res = 0;
	while(res==0){

		//ファイル名//
		if(m_outNumBMP < 10){
			_stprintf(spath + length,_T("000%d.bmp"), m_outNumBMP);
		}else if(m_outNumBMP < 100){
			_stprintf(spath + length,_T("00%d.bmp"), m_outNumBMP);
		}else if(m_outNumBMP < 1000){
			_stprintf(spath + length,_T("0%d.bmp"), m_outNumBMP);
		}else{
			_stprintf(spath + length,_T("%d.bmp"), m_outNumBMP);
		}
		m_outNumBMP++;

		//画像作り
		OutputBMfromGLMag(m_data_list,  spath, m_client_w, m_client_h, 1, view_matrix, focus_distance, rendering_property);

		Sleep(0);
		//1フレーム進める
		res = PlayMovie();
	}




}

//analyze for view//
int AIScope::ResetBond(){

	for (auto it = m_data_list.begin(), it_end = m_data_list.end(); it != it_end; ++it) {

		if(it->type == FILETYPE_ATOM){
			ATOMS_DATA* dat = it->GetDataPointer();
			glips_ResetBond(dat, &(dat->bond), g_bond_cutoff);
		}
	}
	
	return 1;
}

int AIScope::ResetColorByPressure(){
	
	for (auto it = m_data_list.begin(), it_end = m_data_list.end(); it != it_end; ++it) {
		
		if(it->type == FILETYPE_ATOM){
			ATOMS_DATA* dat = it->GetDataPointer();
			//glips_ResetColorByPressure(fi->atm->GetDataPointer(), &(fi->bond));
			glips_ResetColorByPx(dat, &(dat->bond));
		}
	}
	
	return 1;
}

