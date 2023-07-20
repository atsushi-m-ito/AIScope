#pragma once
#include "vec3.h"


struct VERTEX_F3F3UI1 {
	vec3f position;
	vec3f normal;
	unsigned int color;
	VERTEX_F3F3UI1() {};
	VERTEX_F3F3UI1(const vec3f& position_, const vec3f& normal_, unsigned int color_) :
		position(position_), normal(normal_), color(color_)
	{	};

};

struct VERTEX_F3F3 {
	vec3f position;
	vec3f normal;	
	VERTEX_F3F3() {};
	VERTEX_F3F3(const vec3f& position_, const vec3f& normal_) :
		position(position_), normal(normal_)
	{	};

};

#ifndef VERTEX_F3UI1_DEF
#define VERTEX_F3UI1_DEF
struct VERTEX_F3UI1 {
	vec3f position;
	unsigned int color;
	VERTEX_F3UI1() {}
	VERTEX_F3UI1(vec3f position_, unsigned int color_) :
		position(position_), color(color_)
	{}
};

struct VERTEX_F3F1 {
	vec3f position;
	float color;
	VERTEX_F3F1() {}
	VERTEX_F3F1(const vec3f& position_, float color_) :
		position(position_), color(color_)
	{}
};

struct VERTEX_F4UI1 {
	float x;
	float y;
	float z;
	float w;
	unsigned int color;
	VERTEX_F4UI1() {}
	VERTEX_F4UI1(float x_, float y_, float z_, float w_, unsigned int color_) :
		x(x_), y(y_), z(z_), w(w_), color(color_)
	{}
};

struct VERTEX_F4 {
	float x;
	float y;
	float z;
	float w;
	VERTEX_F4() {}
	VERTEX_F4(float x_, float y_, float z_, float w_) :
		x(x_), y(y_), z(z_), w(w_)
	{}

	VERTEX_F4(const vec3f& xyz_, float w_) :
		x(xyz_.x), y(xyz_.y), z(xyz_.z), w(w_) 
	{
	}
};
#endif

struct VERTEX_F3F3F2 {
	vec3f position;
	vec3f normal;
	float coord_u;
	float coord_v;
	VERTEX_F3F3F2() = default;
	VERTEX_F3F3F2(const vec3f& position_, const vec3f& normal_, const float u_, const float v_) :
		position(position_), normal(normal_), coord_u(u_), coord_v(v_)
	{	};

};

struct VERTEX_F3F3F3 {
	vec3f position;
	vec3f normal;
	float coord_u;
	float coord_v;
	float coord_w;
	VERTEX_F3F3F3() = default;
	VERTEX_F3F3F3(const vec3f& position_, const vec3f& normal_, const float u_, const float v_, const float w_) :
		position(position_), normal(normal_), coord_u(u_), coord_v(v_), coord_w(w_)
	{	};

};


struct VERTEX_F3F2 {
	vec3f position;
	float coord_u;
	float coord_v;
	VERTEX_F3F2() {};
	VERTEX_F3F2(const vec3f& position_, const float u_, const float v_) :
		position(position_), coord_u(u_), coord_v(v_)
	{	};

};


