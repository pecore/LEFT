/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "GLParticleEffect.h"

GLParticleEffect::GLParticleEffect(int width, int height)
{
  mParticle = new GLParticle(width, height, 1.0f, 1.0f, 1.0f);
  for(int i = 0; i < GL_EFFECT_MAX_PARTICLES; i++) {
    mDummies[i] = 0;
  }
  mCount = 0;
}

GLParticleEffect::~GLParticleEffect()
{
  if(mParticle) delete mParticle;
  mParticle = 0;

  for(int i = 0; i < GL_EFFECT_MAX_PARTICLES; i++) {
    if(mDummies[i]) delete mDummies[i];
    mDummies[i] = 0;
  }
  mCount = 0;
}

GLParticleDummy * GLParticleEffect::addParticle()
{
  if(mCount >= GL_EFFECT_MAX_PARTICLES) {
    return 0;
  }

  for(int i = 0; i < GL_EFFECT_MAX_PARTICLES; i++) {
    if(!mDummies[i]) {
      mDummies[i] = new GLParticleDummy(mParticle);
      if(mDummies[i]) {
        mCount++;
        return mDummies[i];
      } else {
        mDummies[i] = 0;
      }
    }
  }

  return 0;
}

GLParticleDummy * GLParticleEffect::getParticle(int id)
{
  int index = 0;
  for(int i = 0; i < GL_EFFECT_MAX_PARTICLES; i++) {
    if(mDummies[i]) {
      if(index++ == id) {
        return mDummies[i];
      }
    }
  }
  return 0;
}