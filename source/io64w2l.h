#ifndef IO64W2L_H
#define IO64W2L_H

#pragma once

#if defined(_WIN32) || defined(_WIN64)
#include <stdio.h>

#else //linux
//#define _LARGE_FILES
//#define _LARGEFILE_SOURCE
//#define _FILE_OFFSET_BITS 64
//#define __USE_LARGEFILE64
#include <stdio.h>
#include <stdlib.h>

#define _fseeki64  fseek
#define _ftelli64  ftell
#endif


#endif    // IO64W2L_H
