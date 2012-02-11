/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "RobotStabilizeEffect.h"
#define _USE_MATH_DEFINES
#include <math.h>

RobotStabilizeEffect::RobotStabilizeEffect(GLfloat x, GLfloat y, GLfloat radius) : GLParticleEffect(4, 4)
{
  mPos.x = x;
  mPos.y = y;

  mWidth = mHeight = radius;

  for(int i = 0; i < STABILIZE_PARTICLE_COUNT; i++) {
    GLParticleDummy * p = addParticle();
    GLfloat angle = frand() * 2.0f * (GLfloat)M_PI;
    p->moveTo(mPos.x + mWidth * cos(angle), mPos.y + mWidth * sin(angle));
    mDirection[i] = frand() < 0.5 ? -1 : 1;
  }
}

RobotStabilizeEffect::~RobotStabilizeEffect()
{
  int i = 0;
}

void RobotStabilizeEffect::spawn(GLParticleDummy * p, int i)
{
  GLvector2f frompos = p->pos() - mPos;
  GLvector2f dest;

  if(frand() < 0.1) {
    mDirection[i] = 2;
  }

  switch(mDirection[i]) {
  case -1:
  case 1:
    p->setColor4f(0.4f, 0.6f, 0.8f, 1.0f);
    dest = mPos + frompos.rotate(2.0f * (GLfloat)M_PI * (5.0f * mDirection[i] / 360.0f));
    p->moveTo(dest.x, dest.y);  
    p->setVelocity2f(0.0f, 0.0f);
    break;
  case 2:
    GLvector2f dir = (p->pos() - mPos);
    p->setColor4f(0.8f, 0.9f, 1.0f, 0.6f);
    p->setVelocity(dir.normal() * -1.0f * frand() * 5.0f);
    if(dir.len() < 5.0f) {
      reset(i);
      mDirection[i] = frand() < 0.5f ? -1 : 1;
    }
    break;
  }
}

void RobotStabilizeEffect::moveTo(GLfloat x, GLfloat y) 
{ 
  mPos.x = x; 
  mPos.y = y;
}

void RobotStabilizeEffect::reset() 
{
  for(int i = 0; i < STABILIZE_PARTICLE_COUNT; i++) {
    reset(i);
  }
}

void RobotStabilizeEffect::reset(int i) 
{
  GLParticleDummy * p = getParticle(i);
  GLfloat angle = frand() * 2.0f * (GLfloat)M_PI;
  GLvector2f circle(mWidth * cos(angle), mWidth * sin(angle));
  GLvector2f dir = ((mPos + circle) - p->pos());
  
  circle *= 1.0f;
  GLvector2f dest = mPos + circle;

  p->moveTo(dest.x, dest.y);
  p->setVelocity(dir.normal() * frand() * 5.0f);
}

void RobotStabilizeEffect::draw()
{
  for(int i = 0; i < STABILIZE_PARTICLE_COUNT; i++) {
    GLParticleDummy * p = getParticle(i);
    spawn(p, i);
    p->move();
    p->draw();
  }
}
