#pragma once

// 以前の Windows プラットフォーム用にアプリケーションをビルドする場合は、WinSDKVer.h をインクルードし、
// SDKDDKVer.h をインクルードする前に、サポート対象とするプラットフォームを示すように _WIN32_WINNT マクロを設定します。
/*
#ifdef WIN7
#define _WIN32_WINNT   0x0601
#define WINVER         0x0601
#endif
*/
#include <SDKDDKVer.h>

#define WIN32_LEAN_AND_MEAN             // Windows ヘッダーから使用されていない部分を除外します。