#pragma once
#include "vec3.h"

/* UTF8 BOMなし ヘッダーファイル */

class IDataContainer {
public:
	virtual ~IDataContainer() = default;
};


class IDataSimulationBox {
public:
	virtual ~IDataSimulationBox() = default;

	virtual bool GetBoxSize(mat33d* axis, vec3d* org) = 0;

};

class IDataAtoms {
public:
	virtual ~IDataAtoms() = default;

	virtual int GetSpecies() = 0;
	virtual int GetPositions() = 0;
	virtual int GetBonds() = 0;
};

