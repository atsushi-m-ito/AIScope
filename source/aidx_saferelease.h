#pragma once

namespace AIDX11 {
	template<class T>
	void SafeRelease(T* p) {
		if (p) { p->Release(); p = NULL; }
	}

}
