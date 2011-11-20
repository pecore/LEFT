/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _ROBOTSTABILIZEEFFECT_H_
#define _ROBOTSTABILIZEEFFECT_H_

#include "GLDefines.h"
#include "GLParticleEffect.h"

#define STABILIZE_PARTICLE_COUNT 200

class RobotStabilizeEffect : public GLParticleEffect {
public:
  RobotStabilizeEffect(GLfloat x, GLfloat y, GLfloat radius);
  ~RobotStabilizeEffect();

  void moveTo(GLfloat x, GLfloat y);
  void reset();

  void draw();

private:
  GLvector2f mPos;

  void spawn(GLParticleDummy * p, int i);
  int mDirection[STABILIZE_PARTICLE_COUNT];

  GLfloat mWidth;
  GLfloat mHeight;

  void reset(int i);
};

#endif
