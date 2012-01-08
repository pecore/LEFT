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
#include "Projectile.h"
extern unsigned int gProjectileCount;

#include "GLParticle.h"

class LightSource {
public:
  LightSource(GLvector2f _pos, GLvector3f _rgb, GLfloat _intensity, particle_t form = glpLight) : pos(_pos), rgb(_rgb), intensity(_intensity) {
    if(form != glpLight) {
      particle = new GLParticle(GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT, rgb.x, rgb.y, rgb.z, 1.0f, form);
    } else {
      particle = 0;
    }
  }
  ~LightSource() {
    if(particle) {
      delete particle;
    }
  }
  
  GLvector2f pos;
  GLvector3f rgb;
  GLfloat intensity;
  GLfloat angle;

  GLParticle * particle;
};
typedef std::list<LightSource *> LightSourceList;
typedef std::list<Collidable *> CollidableList;
typedef std::list<Projectile *> ProjectileList;

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

  void addProjectile(Projectile * proj) {
    mProjectiles.push_back(proj);
    addCollidable((Collidable *) proj);
  }

  void removeProjectile(Projectile * proj) {
    mProjectiles.remove(proj);
    delete proj;
  }

  bool isProjectile(Collidable * c) {
    Projectile * proj = 0;
    foreach(ProjectileList, proj, mProjectiles) {
      if(proj == c) {
        return true;
      }
    }
    return false;
  }

  void drawProjectiles() {
    Projectile * proj = 0;
    foreach(ProjectileList, proj, mProjectiles) {
      proj->draw();
    }
  }

  LightSourceList & LightSources() { return mLightSources; }
  ProjectileList & Projectiles() { return mProjectiles; }

  void updateCollision();
  void addCirclePolygon(GLvector2f pos, GLfloat size);

private:
  HANDLE mMutex;

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

  GLParticle * mSpot;
  LightSourceList mLightSources;
  CollidableList mCollidables;
  int mCollidableCount;
  ProjectileList mProjectiles;

  void generate();
  void genCirclePolygon(GLvector2f pos, GLfloat size, GLtriangleList & triangles, Polygon & polygon, bool random = true, GLfloat _segments_per_100 = 12);
};

#endif
