/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _BFGEFFECT_H_
#define _BFGEFFECT_H_

#include "GLDefines.h"
#include "GLParticleEffect.h"
#include <list>

#define PARTICLE_GROUP_COUNT 7
struct ParticleGroup {
  GLvector2f center;
  GLvector2f pos;
  GLfloat angle[PARTICLE_GROUP_COUNT];
  GLfloat radius[PARTICLE_GROUP_COUNT];
  GLfloat a, r;
  GLParticleDummy * particles[PARTICLE_GROUP_COUNT];
};

typedef std::list<ParticleGroup *> ParticleGroupList;

class BFGEffect : public GLParticleEffect {
public:
  BFGEffect(GLfloat x, GLfloat y);
  ~BFGEffect();

  void moveTo(GLfloat x, GLfloat y);
  void draw();

private:
  GLvector2f mPos;
  unsigned int mCount;
  ParticleGroupList mGroups;

  GLfloat mWidth;
  GLfloat mHeight;
};

#endif
