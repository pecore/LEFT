/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _MAP_H_
#define _MAP_H_

#include "GLDefines.h"
#include "clipper.hpp"
using namespace ClipperLib;
#define Polygon ClipperLib::Polygon
#define CLIPPER_PRECISION 100000.0

#include "Collidable.h"
#define MAP_COLLIDABLES_MAX 2048

#include "GLParticle.h"

class LightSource {
public:
  LightSource(GLvector2f _pos, GLvector3f _rgb) : pos(_pos), rgb(_rgb) { }
  GLvector2f pos;
  GLvector3f rgb;
};
typedef std::list<LightSource *> LightSourceList;
typedef std::list<Collidable *> CollidableList;

class Map {
public:
  Map();
  ~Map();

  GLplaneList collision() { 
    Lock(mMutex);
    GLplaneList c = mCollision;
    Unlock(mMutex);
    return c;
  }

  void lock() { Lock(mMutex); }
  void unlock() { Unlock(mMutex); }

  void draw();
  void drawShadows(GLvector2f window);
  void collide();
  
  void addCollidable(Collidable * c) { 
    mCollidables.push_back(c);
    mCollidableCount++;
  }
  
  void removeCollidable(Collidable * c) { 
    if(mCollidableCount > 0) {
      mCollidables.remove(c);
      mCollidableCount--;
    }
  }

  LightSourceList & LightSources() { return mLightSources; }

  void updateCollision();
  void addCirclePolygon(GLvector2f pos, GLfloat size);

private:
  HANDLE mMutex;

  GLParticle * mSpot;
  GLuint mFramebufferTexture;
  GLuint mFramebuffer;
  void renderTarget(bool texture) {
    if(texture) {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFramebuffer);
      glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mFramebufferTexture, 0);
      glClear(GL_COLOR_BUFFER_BIT);
    } else glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  }

  Polygons mCMap;
  GLtriangleList mMap;
  GLplaneList mCollision;

  LightSourceList mLightSources;
  CollidableList mCollidables;
  int mCollidableCount;

  void generate();
  void genCirclePolygon(GLvector2f pos, GLfloat size, GLtriangleList & triangles, Polygon & polygon, bool random = true, GLfloat _segments_per_100 = 12);
};

#endif
