
#ifndef __listTemplate_h__
#define __listTemplate_h__

#pragma once

#include <stdio.h>

template<class T> 
class MasterList {
public:
	T* first;
	T* last;

	MasterList(){
		first = NULL;
		last = NULL;
	};

	virtual ~MasterList(){
		for(T* element = first; element != NULL; ){
			T* prev = element;
			element = element->next;
			delete prev;
		}
	};

	void JoinFirst(T* element){
		element->next = first;
		if(first){
			first->prev = element;
		}else{
			last = element;
		}
		first = element;
		element->prev = NULL;		
	};

	void JoinLast(T* element){
		element->prev = last;
		if(last){
			last->next = element;
		}else{
			first = element;
		}
		last = element;
		element->next = NULL;		
	};


	T* AddFirst(){
		T* element = new T();
		JoinFirst(element);
		return element;
	};

	T* AddLast(){
		T* element = new T();
		JoinLast(element);
		return element;
	};


	void Unjoin(T* element){
		if(element->prev){
			element->prev->next = element->next;
		}else{
			first = element->next;
		}
		if(element->next){
			element->next->prev= element->prev;
		}else{
			last = element->prev;
		}

		element->next = element->prev = NULL;
	};

	void Delete(T* element){
		Unjoin(element);
		delete element;
	};
};


template<class T, class U>
class UniqueList{
//ユニークなkey に対してユニークなvalueがある場合のシェア構造.
private:
	struct _tag_element{
		_tag_element* prev;
		_tag_element* next;
		T key;
		U value;
	};

	_tag_element* first;
	_tag_element* last;		//最初の要素はダミー.

	_tag_element* pos;		//探索用の現在位置.
	
public:
	UniqueList(){
		first = last = new _tag_element;
		first->prev = NULL;
		first->next = NULL;
	};
	
	virtual ~UniqueList(){
		for(_tag_element* e = first; e != NULL; ){
			_tag_element* prev = e;
			e = e->next;
			delete prev;
		}
	};


	void add(T key, U value){
		_tag_element* e = new _tag_element;
		e->prev = last;
		e->next = NULL;
		last->next = e;
		last = e;
		
		e->key = key;
		e->value = value;
	};
/*
	bool findFirst(T* pkey, U* pvalue){
		pos = first->next;
		if (pos){
			*pkey = pos->key;
			*pvalue = pos->value;
			return true;
		}else{
			return false;
		}
	}

	bool findNext(T* pkey, U* pvalue){
		pos = pos->next;
		if (pos){
			*pkey = pos->key;
			*pvalue = pos->value;
			return true;
		}else{
			return false;
		}
	}
*/
	U find(T key){
		for(_tag_element* e = first->next; e != NULL; e = e->next){
			if ( IsEqual(e->key, key)) {		//一致したらtrueを返す比較関数
				return e->value;
			}
		}
		return 0;
	};

};

#endif	//__listTemplate_h__