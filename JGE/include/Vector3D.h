#ifndef __VECTOR3D_H_
#define __VECTOR3D_H_

#include <math.h>


/*************************** Macros and constants ***************************/
// returns a number ranging from -1.0 to 1.0
#define FRAND   (((float)rand()-(float)rand())/RAND_MAX)
#define Clamp(x, min, max)  x = (x<min  ? min : x<max ? x : max);

#define SQUARE(x)  (x)*(x)


struct Vector3D
{
  Vector3D(float x, float y, float z) : x(x), y(y), z(z) {}
  Vector3D(const Vector3D &v) : x(v.x), y(v.y), z(v.z) {}
  Vector3D() : x(0.0f), y(0.0f), z(0.0f) {}

  Vector3D& operator=(const Vector3D &rhs)
  {
    x = rhs.x;
    y = rhs.y;
    z = rhs.z;
    return *this;
  }

  // vector add
  Vector3D operator+(const Vector3D &rhs) const
  {
    return Vector3D(x + rhs.x, y + rhs.y, z + rhs.z);
  }

  // vector subtract
  Vector3D operator-(const Vector3D &rhs) const
  {
    return Vector3D(x - rhs.x, y - rhs.y, z - rhs.z);
  }

  // scalar multiplication
  Vector3D operator*(const float scalar) const
  {
    return Vector3D(x * scalar, y * scalar, z * scalar);
  }

  // dot product
  float operator*(const Vector3D &rhs) const
  {
    return x * rhs.x + y * rhs.y + z * rhs.z;
  }

  // cross product
  Vector3D operator^(const Vector3D &rhs) const
  {
    return Vector3D(y * rhs.z - rhs.y * z, rhs.x * z - x * rhs.z, x * rhs.y - rhs.x * y);
  }

  float& operator[](int index)
  {
    return v[index];
  }

  float Length()
  {
    float length = (float)sqrt(SQUARE(x) + SQUARE(y) + SQUARE(z));
    return (length != 0.0f) ? length : 1.0f;
  }

/*****************************************************************************
 Normalize()

 Helper function to normalize vectors
*****************************************************************************/
  Vector3D Normalize()
  {
    *this = *this * (1.0f/Length());
    return *this;
  }

  union
  {
    struct
    {
      float x;
      float y;
      float z;
    };
    float v[3];
  };
};


#endif 

