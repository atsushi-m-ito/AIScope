
#ifndef __vec4_h__
#define __vec4_h__


#pragma once

#include <math.h>
#include <string.h>
#include "vec3.h"


//double================================
	template <typename T>
	class vec4{
	public:
		T x;
		T y;
		T z;
		T w;

		vec4() = default;

		vec4(const T& d1, const T& d2, const T& d3, const T& d4) :
			x(d1),y(d2),z(d3),w(d4)
		{		}


		vec4(const vec3<T>& v3, const T& d4) :
			x(v3.x), y(v3.y), z(v3.z), w(d4)
		{		};

		vec4(const T& d) :
			x(d), y(d), z(d), w(d)
		{		};



		inline vec4& Set(const T& d1, const T& d2, const T& d3, const T& d4){
			x = d1;
			y = d2;
			z = d3;
			w = d4;
			return *this;
		};

		inline vec4& operator +=(const vec4& v){
			x += v.x;
			y += v.y;
			z += v.z;
			w += v.w;
			return *this;
		}
		
		inline vec4 operator -(){
			vec4 v2;
			v2.x = - x;
			v2.y = - y;
			v2.z = - z;
			v2.w = - w;
			return v2;
		}

		inline vec4& operator -=(const vec4& v){
			x -= v.x;
			y -= v.y;
			z -= v.z;
			w -= v.w;
			return *this;
		}
		
		inline vec4& operator *=(const T& d){
			x *= d;
			y *= d;
			z *= d;
			w *= d;
			return *this;
		}
		
		inline vec4& operator /=(const T& d){
			T di = 1.0 / d;
			x *= di;
			y *= di;
			z *= di;
			w *= di;
			return *this;
		}
		
		inline void Clear(){
			memset(&x, 0, 4 * sizeof(T));
		}
		
		template <typename U>
		inline operator vec4<U>() {
			vec4<U> vf;
			vf.x = (U)x;
			vf.y = (U)y;
			vf.z = (U)z;
			vf.w = (U)w;
			return vf;
		};

	};

	template <typename T>
	inline T operator *(const vec4<T>& v, const vec4<T>& v2){
		return (v.x * v2.x + v.y * v2.y + v.z * v2.z + v.w * v2.w);
	}
	
	template <typename T>
	inline vec4<T> operator +(const vec4<T>& v, const vec4<T>& v2){
		vec4<T> vr;
		vr.x = v.x + v2.x;
		vr.y = v.y + v2.y;
		vr.z = v.z + v2.z;
		vr.w = v.w + v2.w;
		return vr;
	}

	template <typename T>
	inline vec4<T> operator -(const vec4<T>& v, const vec4<T>& v2){
		vec4<T> vr;
		vr.x = v.x - v2.x;
		vr.y = v.y - v2.y;
		vr.z = v.z - v2.z;
		vr.w = v.w - v2.w;
		return vr;
	}

	template <typename T>
	inline vec4<T> operator *(const vec4<T>& v, const T& d){
		vec4<T> v2;
		v2.x = v.x * d;
		v2.y = v.y * d;
		v2.z = v.z * d;
		v2.w = v.w * d;
		return v2;
	}
	
	template <typename T>
	inline vec4<T> operator /(const vec4<T>& v, const T& d){
		T di = 1.0 / d;
		vec4<T> v2;
		v2.x = v.x * di;
		v2.y = v.y * di;
		v2.z = v.z * di;
		v2.w = v.w * di;
		return v2;
	}			



	typedef vec4<double> vec4d;
	typedef vec4<float> vec4f;

	

#endif // __vec4_h__