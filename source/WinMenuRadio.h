#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include "INotifyProperty.h"


class WinMenuRadio : public INotifyProperty
{
private:

	const HMENU hMenu_;
	const int count_;
	const UINT* MENU_IDs_;	// = { IDM_PROJECTION_PERSE, IDM_PROJECTION_ORTHO };
	const int* values_;	// = { IDM_PROJECTION_PERSE, IDM_PROJECTION_ORTHO };

public:
	WinMenuRadio(HMENU hMenu, const int count, const UINT* MENU_IDs, const int* values) :
		hMenu_(hMenu),
		count_(count),
		MENU_IDs_(mDumpArray(count, MENU_IDs)),
		values_(mDumpArray(count, values))
	{
	};

	~WinMenuRadio(){
		delete[] MENU_IDs_;
		delete[] values_;
	};


	template <typename S>
	const S* mDumpArray(const int count, const S* values){
		S* buffer = new S[count];
		for (int i = 0; i < count; ++i){
			buffer[i] = values[i];
		}
		return buffer;
	};

//プロパティー変更脳内を受けた場合////////////////////////////////////////////////////////
	
	void Notify(int key, int value){
		RecheckMenu(value);
	};

	void RecheckMenu(int value){

		MENUITEMINFO menuInfo;
		ZeroMemory(&menuInfo, sizeof(MENUITEMINFO));
		menuInfo.cbSize = sizeof(MENUITEMINFO);
		menuInfo.fMask = MIIM_STATE;


		for (int i = 0; i < count_; i++){
			if(values_[i] == value){
				menuInfo.fState = MFS_CHECKED;
			}
			else{
				menuInfo.fState = MFS_UNCHECKED;
			}
			SetMenuItemInfo(hMenu_, MENU_IDs_[i], FALSE, &menuInfo);
		}

	}
};
