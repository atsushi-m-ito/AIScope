#pragma once
#ifndef ATOM_SORT_H

#include <algorithm>


struct SORT_ID{
    size_t value;
    size_t index;
};

inline bool operator<(const SORT_ID& left, const SORT_ID& right) {
    return left.value < right.value;
}

inline void AtomSort(SORT_ID* elements, const size_t count){
    std::sort(elements, elements + count);
}

template<typename T>
inline void AtomOrder(T* values, T* work, SORT_ID* elements, const size_t count){
    {
        for (size_t i = 0; i < count; i++){
            work[i] = values[i];
        }
        for (size_t i = 0; i < count; i++){
            const size_t index = elements[i].index;
            values[i] = work[index];
        }
    }

}

#endif 
