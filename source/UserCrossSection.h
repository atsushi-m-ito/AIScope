#pragma once
/*
* ユーザー定義断面のマウス・キー操作による回転移動を制御するクラス
* 断面に必要な情報は、
* 断面が通る中心点position
* 断面に垂直な法線ベクトルnormal
* の二つ。
* 操作自体はカメラの回転移動に似ている
*/
#include "vec3.h"
#include "mat33.h"

class UserCrossSection {
public:
	vec3d position;
	vec3d normal;
	int and_or = 0;
private:
	//for mouse operation;
	POINT m_mouse_toggle_ctr{ 0,0 };
	mat44d m_drag_view_matrix{ 0.0 };
	vec3d m_drag_position;
	vec3d m_drag_normal;

public:
	UserCrossSection(const vec3d& default_position, const vec3d& default_normal):
		position(default_position), normal(default_normal)
	{		}

	~UserCrossSection() = default;

	int MouseDown(int x, int y, const double* view_matrix) {
		
		m_mouse_toggle_ctr.x = x;
		m_mouse_toggle_ctr.y = y;

		//drag開始時のviewmatrixを保存//
		//a軸: cameraからみて右方向//
		//b軸: cameraからみて上方向//
		//c軸: cameraの視線方向//
		for (int i = 0; i < 16; ++i) {
			m_drag_view_matrix.elements[i] = view_matrix[i];
		}

		m_drag_position = position;
		m_drag_normal = normal;
				
		return 0;
	}


	int MouseMoveTrans(int x, int y, int w, int h) {
		
		x -= m_mouse_toggle_ctr.x;
		y -= m_mouse_toggle_ctr.y;
		if (x | y) {	//移動がある場合のみ

			const double dx = -(double)x;
			const double dy = (double)y;
			/*
			double top = m_drag_distance * tan(VIEW_ANGLE);
			double right = top * (w / h);

			double dvx = right * (2.0 * dx / w);
			double dvy = top * (2.0 * dy / h);

			m_target = m_drag_org - m_drag_right * dvx - m_drag_up * dvy;
			m_camera_position = m_target - m_drag_front;
			m_up = m_drag_up;
			*/
			return 1;
		}
		
		return 0;

	}

	int MouseMoveRot(int x, int y, int w, int h) {
		
		x -= m_mouse_toggle_ctr.x;
		y -= m_mouse_toggle_ctr.y;
		if (x | y) {	//移動がある場合のみ
			const double dx = -(double)x;
			const double dy = (double)y;

			const double rad = M_PI * sqrt(dx * dx + dy * dy) / ((double)max(w, h));
			
			const vec3d right{ m_drag_view_matrix.elements[0], m_drag_view_matrix.elements[4], m_drag_view_matrix.elements[8] };
			const vec3d up{ m_drag_view_matrix.elements[1], m_drag_view_matrix.elements[5], m_drag_view_matrix.elements[9] };

			vec3d axis = right * (dy / (double)h) - up * (dx / (double)h);
			axis = Unit(axis);

			//ドラッグ開始時からの回転(現在の状態からではない)//
			normal = mRotateOnAxis(axis, m_drag_normal, rad);

			return 1;

		}
		
		return 0;

	}

	void RotateOnUpAxis(double rad, const double* view_matrix) {
		const vec3d axis{ view_matrix[1],view_matrix[5],view_matrix[9] };
		normal = mRotateOnAxis(axis, normal, rad);
	}

	void RotateOnRightAxis(double rad, const double* view_matrix) {
		const vec3d axis{ view_matrix[0],view_matrix[4],view_matrix[8] };
		normal = mRotateOnAxis(axis, normal, rad);
	}


	//軸aを中心に、ベクトルv1を回転する
	static vec3d mRotateOnAxis(const vec3d& a, const vec3d& v1, double rad) {


		const double cos_rad = cos(rad);
		const double sin_rad = sin(rad);
		const double one_cos = 1.0 - cos_rad;
		const double sin_ax = sin_rad * a.x;
		const double sin_ay = sin_rad * a.y;
		const double sin_az = sin_rad * a.z;

		//回転行列生成
		double m[9];
		m[0] = cos_rad + one_cos * a.x * a.x;
		m[3] = m[1] = one_cos * a.x * a.y;
		m[6] = m[2] = one_cos * a.z * a.x;
		m[7] = m[5] = one_cos * a.y * a.z;
		m[4] = cos_rad + one_cos * a.y * a.y;
		m[8] = cos_rad + one_cos * a.z * a.z;

		m[1] -= sin_az;
		m[2] += sin_ay;
		m[3] += sin_az;
		m[5] -= sin_ax;
		m[6] -= sin_ay;
		m[7] += sin_ax;

		return vec3d{
			m[0] * v1.x + m[1] * v1.y + m[2] * v1.z,
			m[3] * v1.x + m[4] * v1.y + m[5] * v1.z,
			m[6] * v1.x + m[7] * v1.y + m[8] * v1.z
		};

	}

	///////////////////////////////////////////////////////////////////////////////
	//  断面の垂直方向平行移動//
	//  断面が移動した時は、断面の位置positionを、有効部分(box内部に位置する領域)の重心にする//
	//////////////////////////////////////////////////////////////////////////////
	void TranslateNormal(double delta, const mat33d& boxaxis, const vec3d& boxorg) {
		const vec3d pos_org = position;
		const vec3d dir = normal * delta;
		//移動//
		position += dir;

		//断面とboxの交差判定から重心を出す//
		std::vector<vec3d> cross_point;
		const size_t num_cross = CrossPointWithBox(boxaxis, boxorg, cross_point);
		if (num_cross > 0) {//断面とboxの交点があった場合, 重心を新たな断面中心にする//
			
			vec3d cm(0.0, 0.0, 0.0);
			for (const auto& a : cross_point) {
				cm += a;
			}
			position = cm / (double)num_cross;
		
		}else{//ボックスから外に出た場合//
			//元の断面から進行方向に位置するbox頂点を洗い出し、
			//その重心を断面の中心(point)にする//
			vec3d cm(0.0, 0.0, 0.0);
			auto SumIfFront = [](const vec3d& v, const vec3d& position, const vec3d& dir, vec3d& sum){
				if ((v - position) * dir > 0.0) {
					sum += v;
					return 1;
				}
				return 0;
			};
			int res = 0;
			res += SumIfFront(boxorg, pos_org, dir, cm);
			res += SumIfFront(boxorg + boxaxis.a, pos_org, dir, cm);
			res += SumIfFront(boxorg +boxaxis.a + boxaxis.b, pos_org, dir, cm);
			res += SumIfFront(boxorg + boxaxis.b, pos_org, dir, cm);

			res += SumIfFront(boxorg + boxaxis.c, pos_org, dir, cm);
			res += SumIfFront(boxorg + boxaxis.c + boxaxis.a, pos_org, dir, cm);
			res += SumIfFront(boxorg + boxaxis.c + boxaxis.a + boxaxis.b, pos_org, dir, cm);
			res += SumIfFront(boxorg + boxaxis.c + boxaxis.b, pos_org, dir, cm);
			if (res > 0) {
				position = cm / (double)res;
			}
			else {
				position = pos_org;
			}
		}
	}

	/*
	Estimate the cross point between a line and a surface.
	o: origin of line (line_org)
	d: direction of line (line_dir)
	s: origin of surface (surface_org)
	n: normal of surface (surface_normal)
	p: cross point.

	First p is defined by 
		p = t d + o,  ... (1)
	where t is parameter. 
	If the direction d indicate line segment that the tips are the point o and (o + d),
	t is in the range [0,1]. 
	When t is out of the range, the surface and line segment do not have cross point.

	Since p is on the surface also, the vector (p - s) is orthogonal to the normal n. 
		(p - s) * n = 0.  ... (2)
	Then, we obtain
	    t = ((s - o) * n) / (d * n).

	*/
	static double CrossPointLineSurface(const vec3d& line_org, const vec3d& line_dir, const vec3d& surface_org, const vec3d& surface_normal) {
		const double d_n = line_dir * surface_normal;
		const double ps_n = (surface_org - line_org) * surface_normal;
		if (d_n == 0.0) return DBL_MAX;
		return ps_n / d_n;
	}

private:
	int mAddCrossPoint(const vec3d& line_org, const vec3d& line_dir, std::vector<vec3d>& cross_points) {
		const double t = CrossPointLineSurface(line_org, line_dir, position, normal);
		if (t < 0.0) return 0;
		if (t > 1.0) return 0;
		
		const vec3d p(line_dir * t + line_org);

		//重複を除く//
		bool is_same = false;
		for (const auto& a : cross_points) {
			const double dd = (a - p) * (a - p);
			if (dd < 1.0e-5) {
				is_same = false;
				break;
			}
		}
		if (is_same) {
			return 0;
		}

		cross_points.push_back(p);
		return 1;
	}

public:
	/*
	* このクラスの持つ断面と、box 構成する12本の線分との交差判定をして、
	* 交点をリストアップする.
	*/
	size_t CrossPointWithBox(const mat33d& boxaxis, const vec3d boxorg, std::vector<vec3d>& cross_points) {
		cross_points.clear();
		mAddCrossPoint(boxorg, boxaxis.a, cross_points);
		mAddCrossPoint(boxorg + boxaxis.b, boxaxis.a, cross_points);
		mAddCrossPoint(boxorg + boxaxis.b + boxaxis.c, boxaxis.a, cross_points);
		mAddCrossPoint(boxorg + boxaxis.c, boxaxis.a, cross_points);

		mAddCrossPoint(boxorg, boxaxis.b, cross_points);
		mAddCrossPoint(boxorg + boxaxis.c, boxaxis.b, cross_points);
		mAddCrossPoint(boxorg + boxaxis.c + boxaxis.a, boxaxis.b, cross_points);
		mAddCrossPoint(boxorg + boxaxis.a, boxaxis.b, cross_points);

		mAddCrossPoint(boxorg, boxaxis.c, cross_points);
		mAddCrossPoint(boxorg + boxaxis.a, boxaxis.c, cross_points);
		mAddCrossPoint(boxorg + boxaxis.a + boxaxis.b, boxaxis.c, cross_points);
		mAddCrossPoint(boxorg + boxaxis.b, boxaxis.c, cross_points);

		return cross_points.size();
	}

};

 