/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _COLLIDABLE_H_
#define _COLLIDABLE_H_

#include "GLDefines.h"

class Collidable abstract {
public:
  virtual void moveTo(GLfloat x, GLfloat y) = 0;
  virtual GLvector2f pos() = 0;
  virtual GLfloat w() = 0;
  virtual GLfloat h() = 0;

  virtual bool collide(GLvector2f n, GLfloat distance) = 0;
};

#endif
