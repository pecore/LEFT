/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include "GLDefines.h"

class Projectile {
public:

  void collide(GLvector2f base, GLvector2f dir);
  void moveTo(GLfloat x, GLfloat y);
  
  void setVelocity2f(GLfloat x, GLfloat y) { mVelocity.x = x; mVelocity.y = y; };
  void setVelocity(GLvector2f & v) { mVelocity = v; };
  void addVelocity(GLvector2f & v) { mVelocity += v; };

protected:
  GLvector2f mPos;
  GLvector2f mVelocity;

};

#endif
