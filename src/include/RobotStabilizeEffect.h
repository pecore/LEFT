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

#define STABILIZE_PARTICLE_COUNT 750

class RobotStabilizeEffect : public GLParticleEffect {
public:
  RobotStabilizeEffect(GLfloat x, GLfloat y);
  ~RobotStabilizeEffect();

  void moveTo(GLfloat x, GLfloat y);
  void setAngle(GLfloat a) { mAngle = a; }
  void reset();

  void integrate(GLfloat dt);
  void draw();

private:
  GLvector2f mPos;
  GLfloat mAngle;
};

#endif
