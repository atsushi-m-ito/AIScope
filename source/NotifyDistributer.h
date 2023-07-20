
#pragma once

#include <map>
#include "INotifyProperty.h"

class NotifyDistributer : public INotifyProperty{
private:
	std::map<int, INotifyProperty*> m_recievers;

public:

	//NotifyDistributer(){};
	//~NotifyDistributer(){};

	
	void Notify(int key, int value){
		auto itr = m_recievers.find(key);
		if (itr != m_recievers.end()){
			//upper_bound: 指定されたキーよりも大きいキーを持つ最初のイテレータ//
			// 存在しない場合はend()と同じ//
			const auto itr_end = m_recievers.upper_bound(key); 

			for (; itr != itr_end; ++itr){
				itr->second->Notify(key, value);
			}
		}
	}

	/*
	void ChangePropertyInt(int key, int value){
		ChangedProperty<int>(key, value);
	}

	void ChangePropertyDouble(int key, double value){
		ChangedProperty<double>(key, value);
	}
	*/

	//分配先となるレシーバーの登録//
	void AddReciever(int key, INotifyProperty* reciever){
		if (reciever){
			m_recievers.insert(std::pair<int, INotifyProperty*>(key, reciever));
		}
	}

};