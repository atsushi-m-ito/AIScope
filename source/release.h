#pragma once

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p) if(p){(p)->Release(); (p)=NULL;}
#endif
