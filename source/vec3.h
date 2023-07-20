//公開Level1//

#ifndef VEC3_H
#define VEC3_H


#pragma once

#include <cmath>
//#include <string.h>

#if 1


template<typename T>
struct vec3{
	public:
		T x;
		T y;
		T z;

		//mat33 で unionの要素にするためにデフォルトコンストラクタ以外のコンストラクタを作ってはいけない//
		//unionにしなければいいみたい//
		//vec3() : x(), y(), z(){};
		vec3(){};
		vec3(const T& x0, const T& y0, const T& z0) : x(x0),y(y0),z(z0){};

		inline vec3& Set(const T& d1, const T& d2, const T& d3){
			x = d1;
			y = d2;
			z = d3;
			return *this;
		}

		
		
		inline void Clear(){
			x = 0.0;
			y = 0.0;
			z = 0.0;

			//memset(&x, 0, 3 * sizeof(T));
		}
		
//		inline operator vec3();
		
		template<typename U>
		inline operator vec3<U>(){
			vec3<U> vf;
			vf.x = static_cast<U>(x);
			vf.y = static_cast<U>(y);
			vf.z = static_cast<U>(z);
			return vf;
		}
		
		inline void Normalize(){
			T ni = 1.0/ std::sqrt(x*x + y*y + z*z);
			*this *= ni;
		}
	
		//for openmp ////////////////////////////////////////////
		inline vec3& Add_atomic(const vec3& v2){
			#pragma omp atomic
			x += v2.x;
			#pragma omp atomic
			y += v2.y;
			#pragma omp atomic
			z += v2.z;
			return *this;
		}

		inline vec3& Sub_atomic(const vec3& v2){
			#pragma omp atomic
			x -= v2.x;
			#pragma omp atomic
			y -= v2.y;
			#pragma omp atomic
			z -= v2.z;
			return *this;
		}

		//inline vec3 GetUnit();
	
	};

	template<typename T>
	inline vec3<T>& operator +(vec3<T>& v){
		return v;
	}

	template<typename T>
	inline vec3<T>& operator +=(vec3<T>& v1, const vec3<T>& v2){
		v1.x += v2.x;
		v1.y += v2.y;
		v1.z += v2.z;
		return v1;
	}
		
	template<typename T>
	inline vec3<T> operator -(const vec3<T>& v){
		//vec3<T> v2(- (v.x), - (v.y), - (v.z));

		vec3<T> v2 = {- (v.x), - (v.y), - (v.z)};
		return v2;
	}

	template<typename T>
	inline vec3<T>& operator -=(vec3<T>& v1, const vec3<T>& v2){
		v1.x -= v2.x;
		v1.y -= v2.y;
		v1.z -= v2.z;
		return v1;
	}
		
	template<typename T>
	inline vec3<T>& operator *=(vec3<T>& v, const T& d){
		v.x *= d;
		v.y *= d;
		v.z *= d;
		return v;
	}

	template<typename T>
	inline vec3<T>& operator /=(vec3<T>& v, const T& d){
			T di = 1.0 / d;
			v.x *= di;
			v.y *= di;
			v.z *= di;
			return v;
	}

	template<typename T>
	inline T operator *(const vec3<T>& v1, const vec3<T>& v2){
		return (v1.x * v2.x + v1.y * v2.y + v1.z * v2.z);
	}
	
	template<typename T>
	inline vec3<T> operator +(const vec3<T>& v1, const vec3<T>& v2){
		vec3<T> vr = {v1.x + v2.x, v1.y + v2.y, v1.z + v2.z};
		return vr;

		/*
		vec3<T> vr;
		vr.x = v.x + v2.x;
		vr.y = v.y + v2.y;
		vr.z = v.z + v2.z;
		return vr;
		*/

	}

	template<typename T>
	inline vec3<T> operator -(const vec3<T>& v1, const vec3<T>& v2){
		vec3<T> vr = { v1.x - v2.x, v1.y - v2.y, v1.z - v2.z};
		return vr;
		

		/*		
		vec3<T> vr;
		vr.x = v.x - v2.x;
		vr.y = v.y - v2.y;
		vr.z = v.z - v2.z;
		return vr;
		*/
	}
	
	template<typename T>
	inline vec3<T> operator *(const vec3<T>& v, const T& d){
		vec3<T> vr = {v.x * d, v.y * d, v.z * d};
		return vr;
		/*
		vec3<T> v2;
		v2.x = v.x * d;
		v2.y = v.y * d;
		v2.z = v.z * d;
		return v2;
		*/
	}
	
	template<typename T>
	inline vec3<T> operator *(const T& d, const vec3<T>& v){
		vec3<T> vr = {v.x * d, v.y * d, v.z * d};
		return vr;
		/*
		vec3<T> v2;
		v2.x = v.x * d;
		v2.y = v.y * d;
		v2.z = v.z * d;
		return v2;
		*/
	}
	
	template<typename T>
	inline vec3<T> operator /(const vec3<T>& v, const T& d){
		T di = 1.0 / d;
		vec3<T> vr = {v.x * di, v.y * di, v.z * di};
		return vr;
		/*
		T di = 1.0 / d;
		vec3<T> v2;
		v2.x = v.x * di;
		v2.y = v.y * di;
		v2.z = v.z * di;
		return v2;
		*/
	}			

	
	template<typename T>
	inline vec3<T> Unit(const vec3<T>& v){
		T ni = 1.0/ std::sqrt(v*v);
		vec3<T> vr(v * ni);
		return vr;			
	}

	template<typename T>
	inline vec3<T> Cross(const vec3<T>& v1, const vec3<T>& v2){
		vec3<T> vr = {
			v1.y * v2.z - v1.z * v2.y,
			v1.z * v2.x - v1.x * v2.z,
			v1.x * v2.y - v1.y * v2.x
		};
		return vr;
		/*
		vec3<T> v3;
		v3.x = v.y * v2.z - v.z * v2.y;
		v3.y = v.z * v2.x - v.x * v2.z;
		v3.z = v.x * v2.y - v.y * v2.x;
		return v3;
		*/
	}

	template<class T>
	inline T Abs(const vec3<T>& v){
		return sqrt(v*v);
	}


	template <typename T>
	inline void ReductionSum_omp(vec3<T>& shared_dest, const vec3<T>& private_source){

#pragma omp single
		{
			shared_dest.Clear();
		}//barrier and flush are automaticlly performed after omp single//

		shared_dest.Add_atomic(private_source);
	}


typedef vec3<double> vec3d;
typedef vec3<float> vec3f;
	

#else

template<typename T>
struct vec3{
	public:
		T x;
		T y;
		T z;

		//vec3d(){};
		//vec3d(double d) : x(d),y(d),z(d){};

		inline vec3& Set(const T& d1, const T& d2, const T& d3){
			x = d1;
			y = d2;
			z = d3;
			return *this;
		};

		inline vec3& operator +=(const vec3& v){
			x += v.x;
			y += v.y;
			z += v.z;
			return *this;
		}
		
		inline vec3 operator -(){
			vec3 v2;
			v2.x = - x;
			v2.y = - y;
			v2.z = - z;
			return v2;
		}

		inline vec3& operator -=(const vec3& v){
			x -= v.x;
			y -= v.y;
			z -= v.z;
			return *this;
		}
		
		inline vec3& operator *=(const T& d){
			x *= d;
			y *= d;
			z *= d;
			return *this;
		}

		inline vec3& operator /=(const T& d){
			T di = 1.0 / d;
			x *= di;
			y *= di;
			z *= di;
			return *this;
		}
		
		inline void Clear(){
			x = 0.0;
			y = 0.0;
			z = 0.0;

			//memset(&x, 0, 3 * sizeof(T));
		}
		
//		inline operator vec3();
		
		template<typename U>
		inline operator vec3<U>(){
			vec3<U> vf;
			vf.x = static_cast<U>(x);
			vf.y = static_cast<U>(y);
			vf.z = static_cast<U>(z);
			return vf;
		};
		
		inline void Normalize(){
			T ni = 1.0/ std::sqrt<T>(x*x + y*y + z*z);
			*this *= ni;
		}
	
		inline vec3 GetUnit();
	
	};

	template<typename T>
	inline double operator *(const vec3<T>& v, const vec3<T>& v2){
		return (v.x * v2.x + v.y * v2.y + v.z * v2.z);
	}
	
	template<typename T>
	inline vec3<T> operator +(const vec3<T>& v, const vec3<T>& v2){
		vec3<T> vr;
		vr.x = v.x + v2.x;
		vr.y = v.y + v2.y;
		vr.z = v.z + v2.z;
		return vr;
	}

	template<typename T>
	inline vec3<T> operator -(const vec3<T>& v, const vec3<T>& v2){
	//inline vec3d operator -(const vec3d v, const vec3d v2){
		vec3<T> vr;
		vr.x = v.x - v2.x;
		vr.y = v.y - v2.y;
		vr.z = v.z - v2.z;
		return vr;
	}

	template<typename T>
	inline vec3<T> operator *(const vec3<T>& v, const T& d){
		vec3<T> v2;
		v2.x = v.x * d;
		v2.y = v.y * d;
		v2.z = v.z * d;
		return v2;
	}
	
	template<typename T>
	inline vec3<T> operator /(const vec3<T>& v, const T& d){
		T di = 1.0 / d;
		vec3<T> v2;
		v2.x = v.x * di;
		v2.y = v.y * di;
		v2.z = v.z * di;
		return v2;
	}			

	
	template<typename T>
	inline vec3<T> Unit(const vec3<T>& v){
		T ni = 1.0/ std::sqrt<T>(v*v);
		vec3d vr = v * ni;
		return vr;			
	}

	template<typename T>
	inline vec3<T> Cross(const vec3<T>& v, const vec3<T>& v2){
		vec3<T> v3;
		v3.x = v.y * v2.z - v.z * v2.y;
		v3.y = v.z * v2.x - v.x * v2.z;
		v3.z = v.x * v2.y - v.y * v2.x;
		return v3;
	}

	template<class T>
	inline T Abs(const vec3<T>& v){
		return sqrt(v*v);
	}




typedef vec3<double> vec3d;
typedef vec3<float> vec3f;
	
#endif
	
#endif // VEC3_H
