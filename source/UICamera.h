#pragma once

#ifndef UICamera_H
#define UICamera_H


#include "camera.h"



class UICamera
{
private:


	CameraGL m_camera;


	//mouse operation//
	POINT m_mouse_toggle_ctr;

	
	
public:

	UICamera() = default;
	virtual ~UICamera()=default;

	/*
	変換行列の取得
	*/
	void GetViewMatrix(double* view_matrix) {
		m_camera.GetViewMatrix(view_matrix);

	}

	/*
	焦点距離の取得
	*/
	double GetFocusDistance() {
		return m_camera.GetFocusDistance();
	}


	void ResetFocus(const double focus_distance, const vec3d& target_position){
		m_camera.ResetFocus(focus_distance, target_position);
	}



	/*
		スクリーン上の点がカメラから見てどの方向になっているかを返す
	*/
	void GetRAY(double x, double y, double w, double h, vec3d* vray, vec3d* vorg){
		m_camera.GetRAY(x, y, w, h, vray, vorg);
	}
	

	//カメラ操作系////////////////////////////////////////////////

	int TranslateUp(double d){
		m_camera.TargetMoveToUp(d);
		return 1;
	};

	int TranslateLeft(double d){
		m_camera.TargetMoveToSide(d);
		return 1;
	};

	int RotateOnUpAxis(double rad){
		m_camera.RotatePosOnUpAxis(rad);
		return 1;
	};

	int RotateOnLeftAxis(double rad){
		m_camera.RotatePosToUp(rad);
		return 1;
	};

	/*
	int SetYawPitchRoll(double yaw, double pitch, double roll){
		m_camera.SetYawPitchRoll(yaw, pitch, roll);
		return 1;
	};
	*/

	int SetRotationMatrix(const float* matrix){
		m_camera.SetRotationMatrix(matrix);
		return 1;
	};


	////////////////////////////////////////////////カメラ操作系//

	//マウスでの操作////////

	int MouseDown(int x, int y) {

		m_mouse_toggle_ctr.x = x;
		m_mouse_toggle_ctr.y = y;
		m_camera.DragStart();

		return 0;
	}



	int MouseMoveTrans(int x, int y, int w, int h) {

		x -= m_mouse_toggle_ctr.x;
		y -= m_mouse_toggle_ctr.y;
		if (x | y) {	//移動がある場合のみ

			m_camera.DragTranslate(x, -y, w, h);

			return 1;
		}

		return 0;

	}

	int MouseMoveRot(int x, int y, int w, int h) {

		x -= m_mouse_toggle_ctr.x;
		y -= m_mouse_toggle_ctr.y;
		if (x | y) {	//移動がある場合のみ

			m_camera.DragRotate(x, -y, w, h);

			return 1;

		}

		return 0;

	}


	int ZoomIn(double d) {
		m_camera.FocusDown(-0.1 * (double)d);
		return 1;
	}



	///////



    void SetPosition(vec3d& camera, vec3d& target, vec3d& up){
        m_camera.SetPosition(camera, target, up);
    }


};

#endif