//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
//
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
//
//-------------------------------------------------------------------------------------

#ifndef _VECTOR2D_H
#define _VECTOR2D_H

#ifdef PSP
#include <fastmath.h>
#else
#include <math.h>
#endif


struct Vector2D
{

	float x, y;
	static const Vector2D& Blank() { static const Vector2D V(0, 0); return V; }

	inline Vector2D(void)	{}

	inline Vector2D(float _x,float _y) : x(_x), y(_y)	{}

	inline Vector2D &operator /=(const float scalar)	{ x /= scalar; y /= scalar;		return *this; }

	inline Vector2D &operator *=(const float scalar)	{ x *= scalar; y *= scalar;		return *this; }

	inline Vector2D &operator +=(const Vector2D &v) { x += v.x;	y += v.y;	return *this; }

	inline Vector2D &operator -=(const Vector2D &v) { x -= v.x;	y -= v.y;	return *this;	}

	inline bool operator ==(const Vector2D &v) const { return x == v.x && y == v.y; }
	inline bool operator !=(const Vector2D &v) const { return x != v.x || y != v.y; }


	// cross product
	inline float operator ^ (const Vector2D &v)	const	{ return (x * v.y) - (y * v.x); }

	// dot product
	inline float operator * (const Vector2D &v)	const	{ return (x*v.x) + (y*v.y); }

	inline float Dot(const Vector2D &v)	const	{ return (x * v.x) + (y * v.y); }
	inline float Cross(const Vector2D &v) const	{ return (x * v.y) - (y * v.x); }


	inline Vector2D operator * (float s)			const	{	return Vector2D(x*s, y*s); }
	inline Vector2D operator / (float s)			const	{	return Vector2D(x/s, y/s); }
	inline Vector2D operator + (const Vector2D &v)	const	{	return Vector2D(x+v.x, y+v.y); }
	inline Vector2D operator - (const Vector2D &v)	const	{	return Vector2D(x-v.x, y-v.y); }
	friend Vector2D operator * (float k, const Vector2D& v) {	return Vector2D(v.x*k, v.y*k); }
	inline Vector2D operator -(void) const { return Vector2D(-x, -y); }

	inline float Length(void) const;
	float Normalize(void) ;
	Vector2D Direction(void) const;
	float Angle(const Vector2D& xE);
	Vector2D& Rotate(float angle);
	Vector2D& Rotate(const Vector2D& xCentre, float fAngle);
	void Clamp(const Vector2D& min, const Vector2D& max);

};


#endif
