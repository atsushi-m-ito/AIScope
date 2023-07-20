/* Debug.h */

///////////////////////////////////////////
//
//      ﾃﾞﾊﾞｯｸﾞ用ﾏｸﾛ定義
//

#ifndef _H_DEBUG_
#define _H_DEBUG_
#ifdef _DEBUG

#include "targetver.h"
#include <windows.h>
#include <stdio.h>
#include <tchar.h>
#define TRACE( str, ... ) { \
        TCHAR c[1024]; \
        _stprintf_s( c, str, __VA_ARGS__ ); \
        OutputDebugString( c ); \
      }
#else
#    define TRACE( str, ... ) // 空実装
#endif


#endif // _H_DEBUG_
