
#pragma once

class INotifyProperty{
private:

public:

	virtual ~INotifyProperty(){};

	virtual void Notify(int key_property, int value) = 0;
	
/*
	virtual void ChangedPropertyInt(int key_property, int value) = 0;
	virtual void ChangedPropertyDouble(int key_property, double value) = 0;
	
*/
};