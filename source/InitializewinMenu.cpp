/**************************************************

アブリケーション固有のメニューGUIの設定を記述

WM_CREATEにて呼ばれる

***************************************************/

#include "resource.h"

#include <vector>
#include "WinMenuRadio.h"
#include "NotifyDistributer.h"
#include "PropertyList.h"

//暫定的にここで確保//
static std::vector<INotifyProperty*> s_menus;

void InitializewinMenu(HMENU hMenu, NotifyDistributer* distributer)
{

	{
		const int key = AISCUPE_KEY_PROJECTION;	//property key//
		const int count = 2;							//num of state of property//
		const int values[] = { 0, 1 };										//list of values of property//
		const UINT IDs[] = { IDM_PROJECTION_PERSE, IDM_PROJECTION_ORTHO };	//list of IDs in window menu//
		WinMenuRadio* win_menu = new WinMenuRadio(hMenu, count, IDs, values);//通知を受けるクラスの生成//
		s_menus.push_back(win_menu);//クラスの保存//
		distributer->AddReciever(key, win_menu);
	}

	{
		const int key = AISCUPE_KEY_BOX_DRAW;	//property key//
		const int count = 2;							//num of state of property//
		const int values[] = { 0, 1 };										//list of values of property//
		const UINT IDs[] = { IDM_BOXFRAME_OFF, IDM_BOXFRAME_ON };	//list of IDs in window menu//
		WinMenuRadio* win_menu = new WinMenuRadio(hMenu, count, IDs, values);//通知を受けるクラスの生成//
		s_menus.push_back(win_menu);//クラスの保存//
		distributer->AddReciever(key, win_menu);
	}

	{
		const int key = AISCUPE_KEY_MULTIVIEW;	//property key//
		const int count = 2;							//num of state of property//
		const int values[] = { 0, 1 };										//list of values of property//
		const UINT IDs[] = { IDM_MULTIVIEW_OVERLAP, IDM_MULTIVIEW_SIDEBYSIDE };	//list of IDs in window menu//
		WinMenuRadio* win_menu = new WinMenuRadio(hMenu, count, IDs, values);//通知を受けるクラスの生成//
		s_menus.push_back(win_menu);//クラスの保存//
		distributer->AddReciever(key, win_menu);
	}

	{
		const int key = AISCUPE_KEY_ATOM_DRAW;	//property key//
		const int count = 3;							//num of state of property//
		const int values[] = { 0, 1, 2};										//list of values of property//
		const UINT IDs[] = { IDM_SHAPE_ATOM_NONE, IDM_SHAPE_SPHERE, IDM_SHAPE_POINT };	//list of IDs in window menu//
		WinMenuRadio* win_menu = new WinMenuRadio(hMenu, count, IDs, values);//通知を受けるクラスの生成//
		s_menus.push_back(win_menu);//クラスの保存//
		distributer->AddReciever(key, win_menu);
	}


	{
		const int key = AISCUPE_KEY_BOND_DRAW;	//property key//
		const int count = 2;	//3						//num of state of property//
		const int values[] = { 0, 1, 2 };										//list of values of property//
		const UINT IDs[] = { IDM_SHAPE_BOND_NONE, IDM_SHAPE_PIPE , IDM_SHAPE_ENERGYJ };	//list of IDs in window menu//
		WinMenuRadio* win_menu = new WinMenuRadio(hMenu, count, IDs, values);//通知を受けるクラスの生成//
		s_menus.push_back(win_menu);//クラスの保存//
		distributer->AddReciever(key, win_menu);
	}

	{
		const int key = AISCUPE_KEY_ATOM_COLOR;	//property key//
		const int count = 3;						//num of state of property//
		const int values[] = { 0, 1, 2 };										//list of values of property//
		const UINT IDs[] = { IDM_COLOR_ATOMIC, IDM_COLOR_BONDNUM, IDM_COLOR_PRESSURE };	//list of IDs in window menu//
		WinMenuRadio* win_menu = new WinMenuRadio(hMenu, count, IDs, values);//通知を受けるクラスの生成//
		s_menus.push_back(win_menu);//クラスの保存//
		distributer->AddReciever(key, win_menu);
	}
	
	
}

void TerminateMenu(){
	for (auto itr = s_menus.begin(); itr != s_menus.end(); ++itr){
		delete (*itr);
	}
}