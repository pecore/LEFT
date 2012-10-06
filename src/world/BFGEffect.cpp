/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "BFGEffect.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <list>

BFGEffect::BFGEffect(GLfloat x, GLfloat y) : GLParticleEffect(12, 12)
{
  mPos.x = x;
  mPos.y = y;

  mWidth = mHeight = 20;
  mCount = 32;
  
  GLfloat angle = 0.0f;
  unsigned int g = 0;
  while(g++ < mCount) {
    ParticleGroup * group = new ParticleGroup;
    group->pos = group->center = mPos + GLvector2f(0.0f, -5.0f) * (GLfloat) g;
    angle += (M_PI / 16.0f);
    group->a = frand() * 2.0f * M_PI;
    group->r = frand() * 24.0f;
    for(int i = 0; i < PARTICLE_GROUP_COUNT; i++) {
      group->particles[i] = addParticle(); 
      group->particles[i]->moveTo(group->pos.x, group->pos.y);
      group->angle[i] = frand() * 2.0f * M_PI;
      group->radius[i] = 5.0f;
    }
    mGroups.push_back(group);
  }
}

BFGEffect::~BFGEffect()
{
  ParticleGroup * group = 0;
  foreach(ParticleGroupList, group, mGroups) {
    delete group;
  }
}

void BFGEffect::moveTo(GLfloat x, GLfloat y) 
{ 
  mPos.x = x; 
  mPos.y = y;
}

void BFGEffect::draw()
{
  GLfloat coeff = 1.0f, gcoeff = 1.0f;
  GLfloat delta = (M_PI / 16.0f);
  ParticleGroup * group = 0, * lastgroup = 0;
  foreach(ParticleGroupList, group, mGroups) {
    group->a += gcoeff * delta;
    if(lastgroup) group->center = lastgroup->pos; else group->center = mPos;
    group->pos = GLvector2f(group->center.x + group->r * cos(group->a), group->center.y + group->r * sin(group->a));
    for(int i = 0; i < PARTICLE_GROUP_COUNT; i++) {
      group->angle[i] += coeff * delta; 
      group->particles[i]->moveTo(group->pos.x + group->radius[i] * cos(group->angle[i]), group->pos.y + group->radius[i] * sin(group->angle[i]));
      
      GLfloat r = frand(); 
      GLvector3f color = GLvector3f(1.0f, 1.0f, 1.0f) - GLvector3f(r * 0.7f, r * 0.4f, r * 0.1f);
      group->particles[i]->setColor(color, 0.7f);
      group->particles[i]->draw();
      coeff *= -1.0f;
    }
    gcoeff *= -1.0f;
    lastgroup = group;
  }
}
