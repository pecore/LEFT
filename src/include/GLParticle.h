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
  glpLight,
  glpLightCone
} particle_t;

class GLParticle : public GLSprite {
public:
  GLParticle();
  GLParticle(int width, int height, GLfloat r = 1.0f, GLfloat g = 1.0f, GLfloat b = 1.0f, GLfloat a = 1.0f, particle_t form = glpCircle);
  ~GLParticle();

  GLvector2f velocity() { return mVelocity; }
  int getForm() { return mForm; }

  void move() { mPos += mVelocity; };
  void setVelocity2f(GLfloat x, GLfloat y) { mVelocity.x = x; mVelocity.y = y; };
  void setVelocity(GLvector2f & v) { mVelocity = v; };

  bool collide(GLvector2f n, GLfloat distance);
  void draw() { this->GLSprite::draw(); };

protected:
  GLvector2f mVelocity;  
  int mForm;

  void generate();
  unsigned char getAlpha(int x, int y);
};

#endif
