#ifndef _GLPARTICLEEFFECT_H_
#define _GLPARTICLEEFFECT_H_

#include "GLParticle.h"

#define GL_EFFECT_MAX_PARTICLES 4096

class GLParticleDummy : public GLParticle {
public:
  GLParticleDummy(GLParticle * parent) : mParent(parent) { };
  ~GLParticleDummy() { };

  void draw() {
    mParent->setColor4f(mr, mg, mb, ma);
    mParent->moveTo(mPos.x, mPos.y);
    mParent->draw();
  }

private:
  GLParticle * mParent;
};

class GLParticleEffect {
public:
  GLParticleEffect(int width, int height);
  ~GLParticleEffect();

protected:
  GLParticleDummy * addParticle();
  GLParticleDummy * getParticle(int id);

private:
  GLParticle * mParticle;
  GLParticleDummy * mDummies[GL_EFFECT_MAX_PARTICLES];

  unsigned int mCount;
};

#endif
