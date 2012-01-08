/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _GLPARTICLE_H_
#define _GLPARTICLE_H_

#include "GLSprite.h"

typedef enum {
  glpCircle = 0,
  glpCross,
  glpSolid,
  glpLight
} particle_t;

class GLParticle : public GLSprite {
public:
  GLParticle();
  GLParticle(int width, int height, GLfloat r = 1.0f, GLfloat g = 1.0f, GLfloat b = 1.0f, GLfloat a = 1.0f, particle_t form = glpCircle);
  ~GLParticle();

  GLvector2f velocity() { return mVelocity; };

  void setColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { mr = r; mg = g; mb = b; ma = a; };
  void setColor(GLvector3f & c, GLfloat a) { mr = c.x; mg = c.y; mb = c.z; ma = a; };
  GLvector3f getColor() { return GLvector3f(mr, mg, mb); }

  void move() { mPos += mVelocity; };
  void setVelocity2f(GLfloat x, GLfloat y) { mVelocity.x = x; mVelocity.y = y; };
  void setVelocity(GLvector2f & v) { mVelocity = v; };

  void collide(GLvector2f n, GLfloat distance);
  void draw() { this->GLSprite::draw(); };

protected:
  GLvector2f mVelocity;

  GLfloat mr;
  GLfloat mg;
  GLfloat mb;
  GLfloat ma;
  
  int mForm;

  void generate();
  char getAlpha(int x, int y);
  void colorMask();
};

#endif
