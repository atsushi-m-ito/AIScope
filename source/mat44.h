
#ifndef __mat44_h__
#define __mat44_h__


#pragma once

#include "vec4.h"

#include <math.h>
#include <string.h>



//	class mat44f;

//double================================
	class mat44d{
	public:
		union {
			double elements[16];
			struct {		//メモリのとり方に注意(縦から)
				double m11;
				double m21;
				double m31;
				double m41;
				double m12;
				double m22;
				double m32;
				double m42;
				double m13;
				double m23;
				double m33;
				double m43;
				double m14;
				double m24;
				double m34;
				double m44;
			};
			struct {		//(縦ベクトル,列,column)
				vec4d v1;
				vec4d v2;
				vec4d v3;
				vec4d v4;
			};
		};

		inline mat44d& Set(	const double& a11, const double& a12, const double& a13, const double& a14,
							const double& a21, const double& a22, const double& a23, const double& a24,
							const double& a31, const double& a32, const double& a33, const double& a34,
							const double& a41, const double& a42, const double& a43, const double& a44){
			m11 = a11;
			m21 = a21;
			m31 = a31;
			m41 = a41;
			
			m12 = a12;
			m22 = a22;
			m32 = a32;
			m42 = a42;
			
			m13 = a13;
			m23 = a23;
			m33 = a33;
			m43 = a43;

			m14 = a14;
			m24 = a24;
			m34 = a34;
			m44 = a44;
			return *this;
		};


		inline mat44d& operator +=(const mat44d& m){
			m11 += m.m11;
			m21 += m.m21;
			m31 += m.m31;
			m41 += m.m41;
			
			m12 += m.m12;
			m22 += m.m22;
			m32 += m.m32;
			m42 += m.m42;
			
			m13 += m.m13;
			m23 += m.m23;
			m33 += m.m33;
			m43 += m.m43;

			m14 += m.m14;
			m24 += m.m24;
			m34 += m.m34;
			m44 += m.m44;
			return *this;
		}
		
		inline mat44d operator -(){
			mat44d m2;
			m2.m11 = - m11;
			m2.m21 = - m21;
			m2.m31 = - m31;
			m2.m41 = - m41;
			
			m2.m12 = - m12;
			m2.m22 = - m22;
			m2.m32 = - m32;
			m2.m42 = - m42;
			
			m2.m13 = - m13;
			m2.m23 = - m23;
			m2.m33 = - m33;
			m2.m43 = - m43;
			
			m2.m14 = - m14;
			m2.m24 = - m24;
			m2.m34 = - m34;
			m2.m44 = - m44;

			return m2;
		}

		inline mat44d& operator -=(const mat44d& m){
			m11 -= m.m11;
			m21 -= m.m21;
			m31 -= m.m31;
			m41 -= m.m41;
			
			m12 -= m.m12;
			m22 -= m.m22;
			m32 -= m.m32;
			m42 -= m.m42;
			
			m13 -= m.m13;
			m23 -= m.m23;
			m33 -= m.m33;
			m43 -= m.m43;

			m14 -= m.m14;
			m24 -= m.m24;
			m34 -= m.m34;
			m44 -= m.m44;
			return *this;
		}
		
//		inline double det(){
//			return m11 * (m22 * m33 - m23* m32) + m12 * (m23 * m31 - m21 * m33) + m13 * (m21 * m32 - m22 * m31);
//		}

//		inline mat44d inverse();
			
		inline void Clear(){
			memset(elements, 0, 16 * sizeof(double));
		}

		inline void SetIdentity(){
			m11 = 1.0;
			m21 = 0.0;
			m31 = 0.0;
			m41 = 0.0;
			
			m12 = 0.0;
			m22 = 1.0;
			m32 = 0.0;
			m42 = 0.0;
			
			m13 = 0.0;
			m23 = 0.0;
			m33 = 1.0;
			m43 = 0.0;

			m14 = 0.0;
			m24 = 0.0;
			m34 = 0.0;
			m44 = 1.0;
		}
	
		//inline operator mat44f();

	};

	inline mat44d operator +(const mat44d& m, const mat44d& m2){
		mat44d mr;
		mr.m11 = m.m11 + m2.m11;
		mr.m21 = m.m21 + m2.m21;
		mr.m31 = m.m31 + m2.m31;
		mr.m41 = m.m41 + m2.m41;
		
		mr.m12 = m.m12 + m2.m12;
		mr.m22 = m.m22 + m2.m22;
		mr.m32 = m.m32 + m2.m32;
		mr.m42 = m.m42 + m2.m42;
		
		mr.m13 = m.m13 + m2.m13;
		mr.m23 = m.m23 + m2.m23;
		mr.m33 = m.m33 + m2.m33;
		mr.m43 = m.m43 + m2.m43;

		mr.m14 = m.m14 + m2.m14;
		mr.m24 = m.m24 + m2.m24;
		mr.m34 = m.m34 + m2.m34;
		mr.m44 = m.m44 + m2.m44;
		return mr;
	}

	inline mat44d operator -(const mat44d& m, const mat44d& m2){
		mat44d mr;
		mr.m11 = m.m11 - m2.m11;
		mr.m21 = m.m21 - m2.m21;
		mr.m31 = m.m31 - m2.m31;
		mr.m41 = m.m41 - m2.m41;
		
		mr.m12 = m.m12 - m2.m12;
		mr.m22 = m.m22 - m2.m22;
		mr.m32 = m.m32 - m2.m32;
		mr.m42 = m.m42 - m2.m42;
		
		mr.m13 = m.m13 - m2.m13;
		mr.m23 = m.m23 - m2.m23;
		mr.m33 = m.m33 - m2.m33;
		mr.m43 = m.m43 - m2.m43;

		mr.m14 = m.m14 - m2.m14;
		mr.m24 = m.m24 - m2.m24;
		mr.m34 = m.m34 - m2.m34;
		mr.m44 = m.m44 - m2.m44;
		return mr;
	}
	
	inline mat44d operator *(const mat44d& m, const mat44d& m2){
		mat44d mr;
		mr.m11 = m.m11 * m2.m11 + m.m12 * m2.m21 + m.m13 * m2.m31 + m.m14 * m2.m41;
		mr.m12 = m.m11 * m2.m12 + m.m12 * m2.m22 + m.m13 * m2.m32 + m.m14 * m2.m42;
		mr.m13 = m.m11 * m2.m13 + m.m12 * m2.m23 + m.m13 * m2.m33 + m.m14 * m2.m43;
		mr.m14 = m.m11 * m2.m14 + m.m12 * m2.m24 + m.m13 * m2.m34 + m.m14 * m2.m44;
		
		mr.m21 = m.m21 * m2.m11 + m.m22 * m2.m21 + m.m23 * m2.m31 + m.m24 * m2.m41;
		mr.m22 = m.m21 * m2.m12 + m.m22 * m2.m22 + m.m23 * m2.m32 + m.m24 * m2.m42;
		mr.m23 = m.m21 * m2.m13 + m.m22 * m2.m23 + m.m23 * m2.m33 + m.m24 * m2.m43;
		mr.m24 = m.m21 * m2.m14 + m.m22 * m2.m24 + m.m23 * m2.m34 + m.m24 * m2.m44;
		
		mr.m31 = m.m31 * m2.m11 + m.m32 * m2.m21 + m.m33 * m2.m31 + m.m34 * m2.m41;
		mr.m32 = m.m31 * m2.m12 + m.m32 * m2.m22 + m.m33 * m2.m32 + m.m34 * m2.m42;
		mr.m33 = m.m31 * m2.m13 + m.m32 * m2.m23 + m.m33 * m2.m33 + m.m34 * m2.m43;
		mr.m34 = m.m31 * m2.m14 + m.m32 * m2.m24 + m.m33 * m2.m34 + m.m34 * m2.m44;

		mr.m41 = m.m41 * m2.m11 + m.m42 * m2.m21 + m.m43 * m2.m31 + m.m44 * m2.m41;
		mr.m42 = m.m41 * m2.m12 + m.m42 * m2.m22 + m.m43 * m2.m32 + m.m44 * m2.m42;
		mr.m43 = m.m41 * m2.m13 + m.m42 * m2.m23 + m.m43 * m2.m33 + m.m44 * m2.m43;
		mr.m44 = m.m41 * m2.m14 + m.m42 * m2.m24 + m.m43 * m2.m34 + m.m44 * m2.m44;

		return mr;
	}
	
	inline vec4d operator *(const mat44d& m, const vec4d& v){
		vec4d vr;
		vr = m.v1 * v.x;
		vr += m.v2 * v.y;
		vr += m.v3 * v.z;
		vr += m.v4 * v.w;
		return vr;
	}
	
	inline vec4d operator *(const vec4d& v, const mat44d& m){
		vec4d vr;
		vr.Set(v * m.v1, v * m.v2, v * m.v3, v * m.v4);
		return vr;
	}
	
	inline mat44d operator *(const mat44d& m, const double& d){
		mat44d m2;
		m2.m11 = d * m.m11;
		m2.m21 = d * m.m21;
		m2.m31 = d * m.m31;
		m2.m41 = d * m.m41;
		
		m2.m12 = d * m.m12;
		m2.m22 = d * m.m22;
		m2.m32 = d * m.m32;
		m2.m42 = d * m.m42;
		
		m2.m13 = d * m.m13;
		m2.m23 = d * m.m23;
		m2.m33 = d * m.m33;
		m2.m43 = d * m.m43;
		
		m2.m14 = d * m.m14;
		m2.m24 = d * m.m24;
		m2.m34 = d * m.m34;
		m2.m44 = d * m.m44;
		return m2;
	}

	inline mat44d operator /(const mat44d& m, const double& d){
		double di = 1.0 / d;
		mat44d m2;
		m2.m11 = di * m.m11;
		m2.m21 = di * m.m21;
		m2.m31 = di * m.m31;
		m2.m41 = di * m.m41;
		
		m2.m12 = di * m.m12;
		m2.m22 = di * m.m22;
		m2.m32 = di * m.m32;
		m2.m42 = di * m.m42;
		
		m2.m13 = di * m.m13;
		m2.m23 = di * m.m23;
		m2.m33 = di * m.m33;
		m2.m43 = di * m.m43;
		
		m2.m14 = di * m.m14;
		m2.m24 = di * m.m24;
		m2.m34 = di * m.m34;
		m2.m44 = di * m.m44;
		return m2;
	}

	
	//転置
	inline mat44d Transpose(mat44d& m){
		mat44d m2;
		m2.m11 = m.m11;
		m2.m21 = m.m12;
		m2.m31 = m.m13;
		m2.m41 = m.m14;
		m2.m12 = m.m21;
		m2.m22 = m.m22;
		m2.m32 = m.m23;
		m2.m42 = m.m24;
		m2.m13 = m.m31;
		m2.m23 = m.m32;
		m2.m33 = m.m33;
		m2.m43 = m.m34;
		m2.m14 = m.m41;
		m2.m24 = m.m42;
		m2.m34 = m.m43;
		m2.m44 = m.m44;
		return m2;
	}

//float================================
	
//double================================
	class mat44f{
	public:
		union {
			float elements[16];
			struct {		//メモリのとり方に注意(縦から)
				float m11;
				float m21;
				float m31;
				float m41;
				float m12;
				float m22;
				float m32;
				float m42;
				float m13;
				float m23;
				float m33;
				float m43;
				float m14;
				float m24;
				float m34;
				float m44;
			};
			struct {		//(縦ベクトル,列,column)
				vec4f v1;
				vec4f v2;
				vec4f v3;
				vec4f v4;
			};
		};

		inline mat44f& Set(	const float& a11, const float& a12, const float& a13, const float& a14,
							const float& a21, const float& a22, const float& a23, const float& a24,
							const float& a31, const float& a32, const float& a33, const float& a34,
							const float& a41, const float& a42, const float& a43, const float& a44){
			m11 = a11;
			m21 = a21;
			m31 = a31;
			m41 = a41;
			
			m12 = a12;
			m22 = a22;
			m32 = a32;
			m42 = a42;
			
			m13 = a13;
			m23 = a23;
			m33 = a33;
			m43 = a43;

			m14 = a14;
			m24 = a24;
			m34 = a34;
			m44 = a44;
			return *this;
		};


		inline mat44f& operator +=(const mat44f& m){
			m11 += m.m11;
			m21 += m.m21;
			m31 += m.m31;
			m41 += m.m41;
			
			m12 += m.m12;
			m22 += m.m22;
			m32 += m.m32;
			m42 += m.m42;
			
			m13 += m.m13;
			m23 += m.m23;
			m33 += m.m33;
			m43 += m.m43;

			m14 += m.m14;
			m24 += m.m24;
			m34 += m.m34;
			m44 += m.m44;
			return *this;
		}
		
		inline mat44f operator -(){
			mat44f m2;
			m2.m11 = - m11;
			m2.m21 = - m21;
			m2.m31 = - m31;
			m2.m41 = - m41;
			
			m2.m12 = - m12;
			m2.m22 = - m22;
			m2.m32 = - m32;
			m2.m42 = - m42;
			
			m2.m13 = - m13;
			m2.m23 = - m23;
			m2.m33 = - m33;
			m2.m43 = - m43;
			
			m2.m14 = - m14;
			m2.m24 = - m24;
			m2.m34 = - m34;
			m2.m44 = - m44;

			return m2;
		}

		inline mat44f& operator -=(const mat44f& m){
			m11 -= m.m11;
			m21 -= m.m21;
			m31 -= m.m31;
			m41 -= m.m41;
			
			m12 -= m.m12;
			m22 -= m.m22;
			m32 -= m.m32;
			m42 -= m.m42;
			
			m13 -= m.m13;
			m23 -= m.m23;
			m33 -= m.m33;
			m43 -= m.m43;

			m14 -= m.m14;
			m24 -= m.m24;
			m34 -= m.m34;
			m44 -= m.m44;
			return *this;
		}
		
//		inline float det(){
//			return m11 * (m22 * m33 - m23* m32) + m12 * (m23 * m31 - m21 * m33) + m13 * (m21 * m32 - m22 * m31);
//		}

//		inline mat44f inverse();

			
		inline void Clear(){
			memset(elements, 0, 16 * sizeof(float));
		}
		
		inline void SetIdentity(){
			m11 = 1.0f;
			m21 = 0.0f;
			m31 = 0.0f;
			m41 = 0.0f;
			
			m12 = 0.0f;
			m22 = 1.0f;
			m32 = 0.0f;
			m42 = 0.0f;
			
			m13 = 0.0f;
			m23 = 0.0f;
			m33 = 1.0f;
			m43 = 0.0f;

			m14 = 0.0f;
			m24 = 0.0f;
			m34 = 0.0f;
			m44 = 1.0f;
		}
	
		//inline operator mat44f();

	};

	inline mat44f operator +(const mat44f& m, const mat44f& m2){
		mat44f mr;
		mr.m11 = m.m11 + m2.m11;
		mr.m21 = m.m21 + m2.m21;
		mr.m31 = m.m31 + m2.m31;
		mr.m41 = m.m41 + m2.m41;
		
		mr.m12 = m.m12 + m2.m12;
		mr.m22 = m.m22 + m2.m22;
		mr.m32 = m.m32 + m2.m32;
		mr.m42 = m.m42 + m2.m42;
		
		mr.m13 = m.m13 + m2.m13;
		mr.m23 = m.m23 + m2.m23;
		mr.m33 = m.m33 + m2.m33;
		mr.m43 = m.m43 + m2.m43;

		mr.m14 = m.m14 + m2.m14;
		mr.m24 = m.m24 + m2.m24;
		mr.m34 = m.m34 + m2.m34;
		mr.m44 = m.m44 + m2.m44;
		return mr;
	}

	inline mat44f operator -(const mat44f& m, const mat44f& m2){
		mat44f mr;
		mr.m11 = m.m11 - m2.m11;
		mr.m21 = m.m21 - m2.m21;
		mr.m31 = m.m31 - m2.m31;
		mr.m41 = m.m41 - m2.m41;
		
		mr.m12 = m.m12 - m2.m12;
		mr.m22 = m.m22 - m2.m22;
		mr.m32 = m.m32 - m2.m32;
		mr.m42 = m.m42 - m2.m42;
		
		mr.m13 = m.m13 - m2.m13;
		mr.m23 = m.m23 - m2.m23;
		mr.m33 = m.m33 - m2.m33;
		mr.m43 = m.m43 - m2.m43;

		mr.m14 = m.m14 - m2.m14;
		mr.m24 = m.m24 - m2.m24;
		mr.m34 = m.m34 - m2.m34;
		mr.m44 = m.m44 - m2.m44;
		return mr;
	}
	
	inline mat44f operator *(const mat44f& m, const mat44f& m2){
		mat44f mr;
		mr.m11 = m.m11 * m2.m11 + m.m12 * m2.m21 + m.m13 * m2.m31 + m.m14 * m2.m41;
		mr.m12 = m.m11 * m2.m12 + m.m12 * m2.m22 + m.m13 * m2.m32 + m.m14 * m2.m42;
		mr.m13 = m.m11 * m2.m13 + m.m12 * m2.m23 + m.m13 * m2.m33 + m.m14 * m2.m43;
		mr.m14 = m.m11 * m2.m14 + m.m12 * m2.m24 + m.m13 * m2.m34 + m.m14 * m2.m44;
		
		mr.m21 = m.m21 * m2.m11 + m.m22 * m2.m21 + m.m23 * m2.m31 + m.m24 * m2.m41;
		mr.m22 = m.m21 * m2.m12 + m.m22 * m2.m22 + m.m23 * m2.m32 + m.m24 * m2.m42;
		mr.m23 = m.m21 * m2.m13 + m.m22 * m2.m23 + m.m23 * m2.m33 + m.m24 * m2.m43;
		mr.m24 = m.m21 * m2.m14 + m.m22 * m2.m24 + m.m23 * m2.m34 + m.m24 * m2.m44;
		
		mr.m31 = m.m31 * m2.m11 + m.m32 * m2.m21 + m.m33 * m2.m31 + m.m34 * m2.m41;
		mr.m32 = m.m31 * m2.m12 + m.m32 * m2.m22 + m.m33 * m2.m32 + m.m34 * m2.m42;
		mr.m33 = m.m31 * m2.m13 + m.m32 * m2.m23 + m.m33 * m2.m33 + m.m34 * m2.m43;
		mr.m34 = m.m31 * m2.m14 + m.m32 * m2.m24 + m.m33 * m2.m34 + m.m34 * m2.m44;

		mr.m41 = m.m41 * m2.m11 + m.m42 * m2.m21 + m.m43 * m2.m31 + m.m44 * m2.m41;
		mr.m42 = m.m41 * m2.m12 + m.m42 * m2.m22 + m.m43 * m2.m32 + m.m44 * m2.m42;
		mr.m43 = m.m41 * m2.m13 + m.m42 * m2.m23 + m.m43 * m2.m33 + m.m44 * m2.m43;
		mr.m44 = m.m41 * m2.m14 + m.m42 * m2.m24 + m.m43 * m2.m34 + m.m44 * m2.m44;

		return mr;
	}
	
	inline vec4f operator *(const mat44f& m, const vec4f& v){
		vec4f vr;
		vr = m.v1 * v.x;
		vr += m.v2 * v.y;
		vr += m.v3 * v.z;
		vr += m.v4 * v.w;
		return vr;
	}
	
	inline vec4f operator *(const vec4f& v, const mat44f& m){
		vec4f vr;
		vr.Set(v * m.v1, v * m.v2, v * m.v3, v * m.v4);
		return vr;
	}
	
	inline mat44f operator *(const mat44f& m, const float& d){
		mat44f m2;
		m2.m11 = d * m.m11;
		m2.m21 = d * m.m21;
		m2.m31 = d * m.m31;
		m2.m41 = d * m.m41;
		
		m2.m12 = d * m.m12;
		m2.m22 = d * m.m22;
		m2.m32 = d * m.m32;
		m2.m42 = d * m.m42;
		
		m2.m13 = d * m.m13;
		m2.m23 = d * m.m23;
		m2.m33 = d * m.m33;
		m2.m43 = d * m.m43;
		
		m2.m14 = d * m.m14;
		m2.m24 = d * m.m24;
		m2.m34 = d * m.m34;
		m2.m44 = d * m.m44;
		return m2;
	}

	inline mat44f operator /(const mat44f& m, const float& d){
		float di = 1.0f / d;
		mat44f m2;
		m2.m11 = di * m.m11;
		m2.m21 = di * m.m21;
		m2.m31 = di * m.m31;
		m2.m41 = di * m.m41;
		
		m2.m12 = di * m.m12;
		m2.m22 = di * m.m22;
		m2.m32 = di * m.m32;
		m2.m42 = di * m.m42;
		
		m2.m13 = di * m.m13;
		m2.m23 = di * m.m23;
		m2.m33 = di * m.m33;
		m2.m43 = di * m.m43;
		
		m2.m14 = di * m.m14;
		m2.m24 = di * m.m24;
		m2.m34 = di * m.m34;
		m2.m44 = di * m.m44;
		return m2;
	}


	//転置
	inline mat44f Transpose(mat44f& m){
		mat44f m2;
		m2.m11 = m.m11;
		m2.m21 = m.m12;
		m2.m31 = m.m13;
		m2.m41 = m.m14;
		m2.m12 = m.m21;
		m2.m22 = m.m22;
		m2.m32 = m.m23;
		m2.m42 = m.m24;
		m2.m13 = m.m31;
		m2.m23 = m.m32;
		m2.m33 = m.m33;
		m2.m43 = m.m34;
		m2.m14 = m.m41;
		m2.m24 = m.m42;
		m2.m34 = m.m43;
		m2.m44 = m.m44;
		return m2;
	}


#endif // __mat44_h__