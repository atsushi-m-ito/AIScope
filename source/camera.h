
#pragma once
#ifndef camera_h
#define camera_h




#include "targetver.h"
#include <windows.h>
#include <tchar.h>


#include "vec3.h"

class CameraGL  
{
private:
	vec3d m_target;
	vec3d m_camera_position;
	vec3d m_up;

	//ドラッグ用操作用	
	double m_drag_distance;
	vec3d m_drag_org;
	vec3d m_drag_up;
	vec3d m_drag_front;
	vec3d m_drag_right;
	
	//push and pop用
	vec3d m_target_mem;
	vec3d m_camera_mem;
	vec3d m_up_mem;

	void RotateOnAxis(vec3d* a, vec3d* v, double rad);
	void RotateOnAxis(vec3d* a, vec3d* v1, vec3d* v2, double rad);
	void OuterProd(vec3d* u, vec3d* v, vec3d* a);

	
	struct CHKPOINT{
		CHKPOINT* next;
		int frame;
		int overblow;
		//CAMERA_INFO caminfo;
		vec3d target;
		vec3d camera;
		vec3d up;
	};
	
	
	CHKPOINT* m_chkpoint_fst;
	CHKPOINT* m_chkpoint_current;
	CHKPOINT* m_chkpoint_last;
	int m_playtm;	//チェックポイント間のフレーム数(再生時はチェックポイント通過の度に0になる)

	void DeleteAllCheckPoint();
	
public:
	
	CameraGL();
	virtual ~CameraGL();
	
	void ResetFocus(double focus_distance, const vec3d& target_position);
	void GetViewMatrix(double* view_matrix);


	void DragStart();
	void DragTranslate(double dx, double dy, double w, double h);
	void DragRotate(double dx, double dy, double w, double h);
	
	void GetRAY(double x, double y, double w, double h, vec3d* vray, vec3d* vorg);

	//void SetGLCamera();

	void RotatePosOnUpAxis(double rad);
	void RotatePosToUp(double rad);
	void FocusDown(double r);
	void RotatePosOnZAxis(double rad);
	void TargetMoveToZ(double z);
	void TargetMoveToUp(double d);
	void TargetMoveToSide(double d);
	void TargetMoveToFront(double d);
	void SetYawPitchRoll(double yaw, double pitch, double roll);
	void SetRotationMatrix(const float* matrix);

	double GetFocusDistance();
	

	void SetStereoMode(double ratio);
	void PushState();
	void PullState();

	
	void InsertCheckPoint(int frame, int overblow);
	int PlayCheckPoint(int* pframe, int frameskip);
	void RewindCheckPoint();

	void SaveCheckPoint(const TCHAR* filename);
	void LoadCheckPoint(const TCHAR* filename);

	void GetDirection(vec3d *target, vec3d *camera){
		*target = m_target;
		*camera = m_camera_position;
	};


    void SetPosition(vec3d& camera, vec3d& target, vec3d& up){
        m_target = target;
        m_camera_position = camera;
        m_up = up;
    };

	template<class T>
	void GetInfo(T *target, T *camera, T *up){
		*target = (T)m_target;
		*camera = (T)m_camera_position;
		*up = (T)m_up;
	};

	/*
	void GetFloatInfo(vec3f *target, vec3f *camera, vec3f *up){
		*target = (vec3f)m_target;
		*camera = (vec3f)m_camera;
		*up= (vec3f)m_up;
	};
	*/
	
	void SetViewFront(int section, int sprit);
	void ResetViewFront();

};

#endif // camera_h