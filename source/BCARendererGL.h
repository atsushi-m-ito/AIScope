// LMCRendererGL.h: LMCRendererGL クラスのインターフェイス
//
//////////////////////////////////////////////////////////////////////

#ifndef BCARendererGL_h
#define BCARendererGL_h

#pragma once

#include <stdio.h>

#include "visual.h"

#include "sdkwgl.h"
#include "vec3.h"
#include "mat33.h"
#include "mat44.h"
#include "mod.h"


#include "visual.h"

#include "trajectory.h"

#define KIND_ALL (255)


class BCARendererGL
{
private:

	//指定されたフレームまでの軌道を描画
	void RenderOne(TRAJECTORY_POS *buf, int count, int frame, double particleRadius, float* traj_color, int periodic_flag, double* boxaxisorg);
	void AI_setSphere(GLdouble radius, GLint slices, GLint stacks, float x, float y, float z);

public:

	BCARendererGL();
	virtual ~BCARendererGL();


	void Draw(const Trajectory* first, int frameno, double m_particleRadius, float* m_traj_color, int periodic_flag, double* boxaxisorg);


//	void SetViewMatrixPointer(mat44f* pViewMatrix );
//	void SetProjectionMatrixPointer(mat44f* pProjectionMatrix );

	
};





#endif // BCARendererGL

