/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "GLDefines.h"

GLvector2f::GLvector2f() : x(0.0f), y(0.0f)
{
}

GLvector2f::GLvector2f(GLfloat _x, GLfloat _y) : x(_x), y(_y) 
{
}

GLvector2f::GLvector2f(const GLvector2f & v) : x(v.x), y(v.y) 
{
}

bool GLvector2f::operator==(const GLvector2f & right)
{
  return (x == right.x) && (y == right.y);
}

bool GLvector2f::operator!=(const GLvector2f & right)
{
  return (x != right.x) || (y != right.y);
}

GLvector2f GLvector2f::operator+(const GLvector2f & right) 
{
  return GLvector2f(x + right.x, y + right.y);
}

GLvector2f GLvector2f::operator-(const GLvector2f & right) 
{
  return GLvector2f(x - right.x, y - right.y);
}

GLvector2f GLvector2f::operator*(const GLfloat & right) 
{
  return GLvector2f(x * right, y * right);
}

GLvector2f GLvector2f::operator/(const GLfloat & right) 
{
  if(right == 0.0f) return GLvector2f(x, y);
  return GLvector2f(x / right, y / right);
}

void GLvector2f::operator+=(const GLvector2f & right) 
{
  x += right.x;
  y += right.y;
}

void GLvector2f::operator-=(const GLvector2f & right) 
{
  x -= right.x;
  y -= right.y;
}

void GLvector2f::operator*=(const GLfloat & right) 
{
  x *= right;
  y *= right;
}

void GLvector2f::operator/=(const GLfloat & right) 
{
  if(right == 0.0f) return;
  x /= right;
  y /= right;
}

void GLvector2f::operator<<=(const GLfloat & angle) 
{
  GLfloat tx = x, ty = y;
  x = tx * cos(angle) - ty * sin(angle);
  y = tx * sin(angle) + ty * cos(angle);
}

GLvector2f GLvector2f::rotate(const GLfloat & angle) 
{
  return GLvector2f(x * cos(angle) - y * sin(angle), x * sin(angle) + y * cos(angle));
}

GLfloat GLvector2f::dot(const GLvector2f & right)
{
  return (x * right.x + y * right.y);
}

GLfloat GLvector2f::angle()
{
  return atan2(y, x);
}

GLfloat GLvector2f::len() 
{
  return sqrt(x * x + y * y);
}

GLfloat GLvector2f::fastlen() 
{
    union {
      int tmp;
      float f;
    } u;
 
    u.f = x * x + y * y; 
    u.tmp -= 1 << 23; 
    u.tmp >>= 1;      
    u.tmp += 1 << 29;
    return u.f;
}

GLvector2f GLvector2f::normal() 
{
  GLfloat length = len(); 
  if(length == 0.0f) {
    return GLvector2f(x, y);
  }
  return GLvector2f(x / length, y / length);
}

void GLvector2f::crossing(const GLvector2f & baseA, const GLvector2f & dirA, const GLvector2f & baseB, const GLvector2f & dirB, GLfloat & coeffA, GLfloat & coeffB) 
{
  if(dirB.x != 0.0f) {
    GLfloat f = dirB.y / dirB.x;
    coeffA = ( (baseB.y - baseA.y) - (f * (baseB.x - baseA.x)) ) / ( dirA.y - (f * dirA.x) );
  } else {
    coeffA = (baseB.x - baseA.x) / dirA.x; 
  }
  if(dirA.x != 0.0f) {
    GLfloat f = dirA.y / dirA.x;
    coeffB = ( (baseB.y - baseA.y) - (f * (baseB.x - baseA.x)) ) / ( dirB.y - (f * dirB.x) );
  } else {
    coeffB = (baseB.x - baseA.x) / dirB.x;
  }
}

GLvector3f::GLvector3f() : x(0.0f), y(0.0f), z(0.0f)
{
}

GLvector3f::GLvector3f(GLfloat _x, GLfloat _y, GLfloat _z) : x(_x), y(_y), z(_z)
{
}

GLvector3f::GLvector3f(const GLvector3f &v) : x(v.x), y(v.y), z(v.z)
{
}

GLvector3f GLvector3f::operator+(const GLvector3f & right) 
{
  return GLvector3f(x + right.x, y + right.y, z + right.z);
}

GLvector3f GLvector3f::operator-(const GLvector3f & right) 
{
  return GLvector3f(x - right.x, y - right.y, z - right.z);
}

GLvector3f GLvector3f::operator*(const GLfloat & right) 
{
  return GLvector3f(x * right, y * right, z * right);
}

GLvector3f GLvector3f::operator/(const GLfloat & right) 
{
  return GLvector3f(x / right, y / right, z / right);
}  

GLtriangle::GLtriangle(GLvector2f & v1, GLvector2f & v2, GLvector2f & v3) : A(v1), B(v2), C(v3)
{
}

GLtriangle::~GLtriangle() 
{
}

GLplane::GLplane()
{
}

GLplane::GLplane(GLvector2f & _base, GLvector2f & _dir) : base(_base), dir(_dir), dest(base + dir)
{
}

GLplane::~GLplane() 
{
}