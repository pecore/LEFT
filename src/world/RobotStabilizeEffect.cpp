/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "RobotStabilizeEffect.h"
#define _USE_MATH_DEFINES
#include <math.h>

RobotStabilizeEffect::RobotStabilizeEffect(GLfloat x, GLfloat y) : GLParticleEffect(4, 4)
{
  mPos.x = x;
  mPos.y = y;

  for(int i = 0; i < STABILIZE_PARTICLE_COUNT; i++) {
    GLParticleDummy * p = addParticle();
    GLfloat r = 0.6f + frand();
    GLfloat y = frand() * 0.6f;
    p->setColor4f(y/2, y, r, 1.0f);
    p->setVelocity2f(0.0f, 0.0f);
  }
}

RobotStabilizeEffect::~RobotStabilizeEffect()
{
}

void RobotStabilizeEffect::moveTo(GLfloat x, GLfloat y) 
{ 
  GLvector2f dir = GLvector2f(x, y) - mPos;
  mPos = GLvector2f(x, y);
  for(int i = 0; i < STABILIZE_PARTICLE_COUNT; i++) {
    GLParticleDummy * p = getParticle(i);
    GLvector2f dest = p->pos() + dir;
    p->moveTo(dest.x, dest.y);
  }
}

void RobotStabilizeEffect::reset() 
{
  for(int i = 0; i < STABILIZE_PARTICLE_COUNT; i++) {
    GLParticleDummy * p = getParticle(i);
    GLfloat angle = 2.0f * M_PI * frand();
    GLfloat radius = 30.0f + frand() * 5.0f;
    p->moveTo(mPos.x + radius * cos(angle), mPos.y + radius * sin(angle));
    p->setVelocity2f(2.0f * frand(), 2.0f * frand());
  }
  GLvector2f backup = mPos;
  for(int i = 0; i < STABILIZE_PARTICLE_COUNT; i++) {
    //mPos += GLvector2f(0.05f, 0.05f);
    integrate(0.1f);
  }
  mPos = backup;
}

void RobotStabilizeEffect::integrate(GLfloat dt)
{
  for(int i = 0; i < STABILIZE_PARTICLE_COUNT; i++) {
    GLParticleDummy * p = getParticle(i);
    GLvector2f velocity = p->velocity();

    GLvector2f direction = mPos - p->pos();
    velocity += direction.normal() * 9.0f * dt;
    p->setVelocity(velocity);

    p->move();
  }
}

void RobotStabilizeEffect::draw()
{
  for(int i = 0; i < STABILIZE_PARTICLE_COUNT; i++) {
    GLParticleDummy * p = getParticle(i);
    p->draw();
  }
}
