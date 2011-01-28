//-------------------------------------------------------------------------------------
//
// JGE++ is a hardware accelerated 2D game SDK for PSP/Windows.
//
// Licensed under the BSD license, see LICENSE in JGE root for details.
// 
// Copyright (c) 2007 James Hui (a.k.a. Dr.Watson) <jhkhui@gmail.com>
// 
//-------------------------------------------------------------------------------------


#include "../include/Vector2D.h"


float Vector2D::Length(void) const 
{
	return sqrtf(x*x + y*y); 
}

float Vector2D::Normalize(void) 
{	
	float fLength = Length();	

	if (fLength == 0.0f) 
		return 0.0f; 

	(*this) *= (1.0f / fLength); 

	return fLength;	
}


Vector2D Vector2D::Direction(void) const
{
	Vector2D temp(*this);

	temp.Normalize();

	return temp;
}

float Vector2D::Angle(const Vector2D& xE)
{
	float dot = (*this) * xE;
	float cross = (*this) ^ xE;

	// angle between segments
	float angle = atan2f(cross, dot);

	return angle;
}


Vector2D& Vector2D::Rotate(float angle)
{
	float tx = x;
	x =  x * cosf(angle) - y * sinf(angle);
	y = tx * sinf(angle) + y * cosf(angle);

	return *this;
}



Vector2D& Vector2D::Rotate(const Vector2D& xCentre, float fAngle)
{
	Vector2D D = *this - xCentre;
	D.Rotate(fAngle);

	*this = xCentre + D;

	return *this;
}


void Vector2D::Clamp(const Vector2D& min, const Vector2D& max)
{
	x = (x < min.x)? min.x : (x > max.x)? max.x : x;
	x = (y < min.y)? min.y : (y > max.y)? max.y : y;
}

