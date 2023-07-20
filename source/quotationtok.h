#ifndef quotationtok_h
#define quotationtok_h

#include <tchar.h>

TCHAR* quotationtok(TCHAR* argv, const TCHAR *control){
	//ダブルクォーテーションで囲まれた場合も加味してfilepathを取得.
	//将来は_tcstokのクォーテーション対策をしたものに作り替え

	static TCHAR* pos = NULL;
	if(argv){
		pos = argv;
	}
	
	//単語の開始を探す.
	while(1){
		if(!*pos){
			return NULL;
		}
		
		//(*pos)がspace文字か調べる.
		const TCHAR *ctl;
		for (ctl = control; *ctl && *ctl != *pos; ctl++);
		if (!*ctl){
			break;	//space文字以外==先頭が見つかった
		}
		pos++;
	}
	//ここにたどり着いたらposは先頭文字の位置になっている.

	TCHAR* token = pos;
	if(*pos == _T('\"')){	//ダブルクォーテーション文字列.
		token = pos + 1;
		if(pos = _tcschr(pos + 1, _T('\"'))){
			*pos = _T('\0');
			pos++;			
		}
		
	}else{
		pos++;
		while(*pos){
			
			//(*s_pos)がspace文字か調べる.
			for (const TCHAR* ctl = control; *ctl; ctl++){
				if(*ctl == *pos){
					//space文字だった.
					*pos = _T('\0');
					pos++;
					return token;
				}
			}
			pos++;
		}
	}

	return token;

	
}

#endif	//!quotationtok_h