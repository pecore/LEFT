#ifndef _COLLIDABLE_H_
#define _COLLIDABLE_H_

#include "GLDefines.h"

class Collidable abstract {
public:
  virtual GLvector2f pos() = 0;
  virtual GLfloat w() = 0;
  virtual GLfloat h() = 0;

  virtual void collide(GLvector2f n, GLfloat distance) = 0;
};

#endif
