//公開Level1//

#ifndef MAT33_H
#define MAT33_H


#pragma once

#ifdef _OPENMP
#include <omp.h>
#endif

#include <math.h>
#include <string.h>

#include "vec3.h"



//double================================
	template<typename T>
	struct mat33{
	public:
		union {
			//T elements[9];
			struct {		//メモリのとり方に注意(縦から)
				T m11;
				T m21;
				T m31;
				T m12;
				T m22;
				T m32;
				T m13;
				T m23;
				T m33;
			};
			struct {
				vec3<T> a;
				vec3<T> b;
				vec3<T> c;
			};
		};

		mat33(){};

		mat33(const T& val) :
			m11(val),
			m21(val),
			m31(val),
			m12(val),
			m22(val),
			m32(val),
			m13(val),
			m23(val),
			m33(val)
		{};

		mat33(const T& a11, const T& a21, const T& a31,
			const T& b12, const T& b22, const T& b32,
			const T& c13, const T& c23, const T& c33) :
			m11(a11),
			m21(a21),
			m31(a31),
			m12(b12),
			m22(b22),
			m32(b32),
			m13(c13),
			m23(c23),
			m33(c33)
		{};

		template <typename U>
		mat33(const mat33<U>& src) :
			m11(src.m11),
			m21(src.m21),
			m31(src.m31),
			m12(src.m12),
			m22(src.m22),
			m32(src.m32),
			m13(src.m13),
			m23(src.m23),
			m33(src.m33)
		{};

		inline mat33<T>& Set(	const T& a11, const T& a21, const T& a31,
							const T& b12, const T& b22, const T& b32,
							const T& c13, const T& c23, const T& c33){
			m11 = a11;
			m21 = a21;
			m31 = a31;
			m12 = b12;
			m22 = b22;
			m32 = b32;
			m13 = c13;
			m23 = c23;
			m33 = c33;
			return *this;
		};


		inline mat33<T>& Set(const T* vars) {
			m11 = vars[0];
			m21 = vars[1];
			m31 = vars[2];
			m12 = vars[3];
			m22 = vars[4];
			m32 = vars[5];
			m13 = vars[6];
			m23 = vars[7];
			m33 = vars[8];
			return *this;
		};




		inline mat33<T>& operator +=(const mat33<T>& m){
			a += m.a;
			b += m.b;
			c += m.c;
			return *this;
		}
		
		inline mat33<T> operator -(){
			mat33<T> m2;
			m2.a = - a;
			m2.b = - b;
			m2.c = - c;
			return m2;
		}

		inline mat33<T>& operator -=(const mat33<T>& m){
			a -= m.a;
			b -= m.b;
			c -= m.c;
			return *this;
		}

		inline mat33<T>& operator *=(const T& d){
			m11 *= d;
			m21 *= d;
			m31 *= d;
			m12 *= d;
			m22 *= d;
			m32 *= d;
			m13 *= d;
			m23 *= d;
			m33 *= d;
			return *this;
		};

		inline mat33<T>& operator /=(const T& d){
			T di = 1.0 / d;
			m11 *= di;
			m21 *= di;
			m31 *= di;
			m12 *= di;
			m22 *= di;
			m32 *= di;
			m13 *= di;
			m23 *= di;
			m33 *= di;
			return *this;
		};
		
		//転置
		inline void SetTranspose(){
			T temp;
			temp = m12;
			m12 = m21;
			m21 = temp;
			
			temp = m13;
			m13 = m31;
			m31 = temp;
			
			temp = m23;
			m23 = m32;
			m32 = temp;
			
		}
			
		inline void Clear(){
			//memset(elements, 0, 9 * sizeof(double));
			m11 = 0.0;
			m21 = 0.0;
			m31 = 0.0;
			m12 = 0.0;
			m22 = 0.0;
			m32 = 0.0;
			m13 = 0.0;
			m23 = 0.0;
			m33 = 0.0;
			
		}
	/*
		template<typename U>
		inline operator mat33<U>(){
			mat33<U> mu;
			mu.m11 = (U)m11;
			mu.m21 = (U)m21;
			mu.m31 = (U)m31;
			mu.m12 = (U)m12;
			mu.m22 = (U)m22;
			mu.m32 = (U)m32;
			mu.m13 = (U)m13;
			mu.m23 = (U)m23;
			mu.m33 = (U)m33;
			return mu;
		};
		*/

		inline mat33& Add_atomic(const mat33& private_source){
			//this should be shared object//

			#pragma omp atomic
			a.x += private_source.a.x;

			#pragma omp atomic
			a.y += private_source.a.y;

			#pragma omp atomic
			a.z += private_source.a.z;

			#pragma omp atomic
			b.x += private_source.b.x;

			#pragma omp atomic
			b.y += private_source.b.y;

			#pragma omp atomic
			b.z += private_source.b.z;

			#pragma omp atomic
			c.x += private_source.c.x;

			#pragma omp atomic
			c.y += private_source.c.y;

			#pragma omp atomic
			c.z += private_source.c.z;

			return *this;
		}

	};

	template<typename T>
	inline mat33<T> operator +(const mat33<T>& m, const mat33<T>& m2){
		mat33<T> mr;
		mr.a = m.a + m2.a;
		mr.b = m.b + m2.b;
		mr.c = m.c + m2.c;
		return mr;
	}

	template<typename T>
	inline mat33<T> operator -(const mat33<T>& m, const mat33<T>& m2){
		mat33<T> mr;
		mr.a = m.a - m2.a;
		mr.b = m.b - m2.b;
		mr.c = m.c - m2.c;
		return mr;
	}
	
	template<typename T>
	inline mat33<T> operator *(const mat33<T>& m, const mat33<T>& m2){
		mat33<T> mr;
		mr.m11 = m.m11 * m2.m11 + m.m12 * m2.m21 + m.m13 * m2.m31;
		mr.m12 = m.m11 * m2.m12 + m.m12 * m2.m22 + m.m13 * m2.m32;
		mr.m13 = m.m11 * m2.m13 + m.m12 * m2.m23 + m.m13 * m2.m33;
		mr.m21 = m.m21 * m2.m11 + m.m22 * m2.m21 + m.m23 * m2.m31;
		mr.m22 = m.m21 * m2.m12 + m.m22 * m2.m22 + m.m23 * m2.m32;
		mr.m23 = m.m21 * m2.m13 + m.m22 * m2.m23 + m.m23 * m2.m33;
		mr.m31 = m.m31 * m2.m11 + m.m32 * m2.m21 + m.m33 * m2.m31;
		mr.m32 = m.m31 * m2.m12 + m.m32 * m2.m22 + m.m33 * m2.m32;
		mr.m33 = m.m31 * m2.m13 + m.m32 * m2.m23 + m.m33 * m2.m33;
		return mr;
	}
	
	template<typename T>
	inline vec3<T> operator *(const mat33<T>& m, const vec3<T>& v){
		vec3<T> vr;
		vr = m.a * v.x;
		vr += m.b * v.y;
		vr += m.c * v.z;
		return vr;
	}
	
	template<typename T>
	inline mat33<T> operator *(const mat33<T>& m, const T& d){
		mat33<T> m2;
		m2.a = m.a * d;
		m2.b = m.b * d;
		m2.c = m.c * d;
		return m2;
	}

	template<typename T>
	inline mat33<T> operator /(const mat33<T>& m, const T& d){
		double di = 1.0 / d;
		mat33<T> m2;
		m2.m11 = m.m11 * di;
		m2.m21 = m.m21 * di;
		m2.m31 = m.m31 * di;
		m2.m12 = m.m12 * di;
		m2.m22 = m.m22 * di;
		m2.m32 = m.m32 * di;
		m2.m13 = m.m13 * di;
		m2.m23 = m.m23 * di;
		m2.m33 = m.m33 * di;
		return m2;
	}

	
	template<typename T>
	inline double Det(mat33<T>& m){
		return m.m11 * (m.m22 * m.m33 - m.m23* m.m32) + m.m12 * (m.m23 * m.m31 - m.m21 * m.m33) + m.m13 * (m.m21 * m.m32 - m.m22 * m.m31);
	}

	template<typename T>
	inline mat33<T> Inverse(mat33<T>& m){
		mat33<T> m2;
		m2.m11 = m.b.y*m.c.z-m.b.z*m.c.y;
		m2.m12 = m.b.z*m.c.x-m.b.x*m.c.z;
		m2.m13 = m.b.x*m.c.y-m.b.y*m.c.x;
		m2.m21 = m.c.y*m.a.z-m.c.z*m.a.y;
		m2.m22 = m.c.z*m.a.x-m.c.x*m.a.z;
		m2.m23 = m.c.x*m.a.y-m.c.y*m.a.x;
		m2.m31 = m.a.y*m.b.z-m.a.z*m.b.y;
		m2.m32 = m.a.z*m.b.x-m.a.x*m.b.z;
		m2.m33 = m.a.x*m.b.y-m.a.y*m.b.x;
		m2 = m2 / (Det(m));
		return m2;
	}	
	
	template<typename T>
	inline mat33<T> Transpose(mat33<T>& m){
		mat33<T> m2;
		m2.m11 = m.m11;
		m2.m21 = m.m12;
		m2.m31 = m.m13;
		m2.m12 = m.m21;
		m2.m22 = m.m22;
		m2.m32 = m.m23;
		m2.m13 = m.m31;
		m2.m23 = m.m32;
		m2.m33 = m.m33;
		return m2;
	}
	
	template<typename T>
	inline double Trace(mat33<T>& m){
		return m.m11 + m.m22 + m.m33;
	}

	/*
	Generate matrix m, where m satisfy 
	v_r = v_1 \times v_2, then
	v_r = m * v_2
	here m = VCrossToM(v_1) .
	*/
	template<typename T>
	inline mat33<T> VCrossToM(const vec3<T>& v1) {
		const mat33<T> m = {
			 0.0,-v1.z, v1.y,
			 v1.z, 0.0,-v1.x,
			-v1.y, v1.x, 0.0
		};
		return m;
	}

	template <typename T>
	inline void ReductionSum_omp(mat33<T>& shared_dest, const mat33<T>& private_source){

		#pragma omp single
		{
			shared_dest.Clear();
		}//barrier and flush are automaticlly performed after omp single//

		shared_dest.Add_atomic(private_source);
	}
	



typedef mat33<double> mat33d; 
typedef mat33<float> mat33f; 


#endif // MAT33_H
