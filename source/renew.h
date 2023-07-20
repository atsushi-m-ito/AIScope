#pragma once


template <typename T>
void renew_preserve(T** v, size_t count, size_t next_size) {
	if (*v == NULL) {
		*v = new T[next_size];
		 
	} else {

		T* nv = new T[next_size];
		size_t w = sizeof(T) * (count < next_size ? count : next_size);
		memcpy(nv, *v, w);
		delete[] (*v);
		*v = nv;
	}
}



template <typename T>
void renew(T** v, size_t next_size) {

	if(*v) delete[] (*v);
	*v = new T[next_size];

}

