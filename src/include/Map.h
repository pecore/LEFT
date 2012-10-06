/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _MAP_H_
#define _MAP_H_

#include "GLDefines.h"
#define MAP_COLLIDABLES_MAX 2048

#include "Collidable.h"
#include "Projectile.h"
#include "GLParticle.h"
#include "GLShader.h"

struct Animation {
  int frameCount;
  GLAnimatedSprite * sprite;
};

class LightSource {
public:
  LightSource(GLvector2f _pos, GLvector3f _rgb, GLfloat _intensity, particle_t form = glpLight) : size(GL_SCREEN_IWIDTH), pos(_pos), rgb(_rgb), intensity(_intensity), angle(0.0f), particle(0), visible(true) {
    if(form != glpLight) {
      particle = new GLParticle(size, size, rgb.x, rgb.y, rgb.z, 1.0f, form);
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
  GLfloat size;
  bool visible;

  GLParticle * particle;
private:
  LightSource() {}
  LightSource(LightSource & source) {}
};

class MapObject : public GLSprite {
public:
  MapObject(const char * name) {
    char filename[256];
    sprintf(filename, GL_RESOURCE_DATAPATH "%s.ply", name);
    mCollision = ((GLPolygonResource *)gResources->get(filename))->polygons[0];
    sprintf(filename, GL_RESOURCE_DATAPATH "%s.png", name);
    GLTextureResource * res = (GLTextureResource *) gResources->get(filename);
    assert(res);
    mTexture = res->texture;
    mWidth = res->width;
    mHeight = res->height;

    mpData = 0;
    mPos.x = 0.0f;
    mPos.y = 0.0f;
    mAngle = 0.0f;
    mRotation.x = 0.0f;
    mRotation.y = 0.0f;
    mInitialized = true;
  }
  Polygon &collision() { return mCollision; };
private:
  Polygon mCollision;
};

typedef std::list<MapObject *> MapObjectList;
typedef std::list<LightSource *> LightSourceList;
typedef std::list<Collidable *> CollidableList;
typedef std::list<Projectile *> ProjectileList;
typedef std::list<Animation *> AnimationList;

typedef void (*CollisionCallback)(void *, Polygon & polygon, Collidable * c, Projectile * p);

class Map {
public:
  Map();
  ~Map();
  
  GLplaneList & shadows() { return mExtraShadows; }

  Polygons polygons() {
    Lock(mMutex);
    Polygons p = mCMap;
    Unlock(mMutex);
    return p;
  }

  void setPolygons(Polygons p) {
    Lock(mMutex);
    mCMap = p;
    Unlock(mMutex);
  }

  void lock() { Lock(mMutex); }
  void unlock() { Unlock(mMutex); }

  void collision();
  void drawShadows();

  void draw();
  void drawProjectiles();
  void drawAnimations();  
  void playAnimation(GLAnimatedSprite * sprite);

  void addCollidable(Collidable * c, bool locked = false);
  void removeCollidable(Collidable * c, bool locked = false);
  void addProjectile(Projectile * proj, bool locked = false);
  void removeProjectile(Projectile * proj, bool locked = false);
  void deleteProjectile(Projectile * proj);
  bool isProjectile(Collidable * c, bool locked = false);
  
  MapObjectList & MapObjects() { return mMapObjects; }
  LightSourceList & LightSources() { return mLightSources; }
  ProjectileList & Projectiles() { return mProjectiles; }

  GLvector2f randomSpawn(GLfloat size = 100.0f);
  void addCirclePolygon(GLvector2f pos, GLfloat size, GLfloat segments = 12);
  void addPolygon(Polygon & polygon);
  void addPolygons(Polygons & p);
  void setUpdate(bool u) { mUpdate = u; }
  void setCallback(CollisionCallback cb, void * ud) { mCallback = cb; mCallbackUserData = ud; }
  
  void drawMinimap();
  void addMinimapZoom(GLfloat z) { return; if((mMinimapZoom > 2.0f && z < 0.0f) || (mMinimapZoom < 20.0f && z > 0.0f)) mMinimapZoom += z; }

  GLfloat getOpacity(GLvector2f pos);

private:
  HANDLE mMutex;
  HANDLE mCollidableMutex;

  GLGaussShader * mGaussShader;
  GLuint mFramebufferTexture;
  GLuint mFramebuffer;
  GLuint mRenderbuffer;
  GLuint mFramebufferList;
  inline void renderTarget(bool texture) {
    if(texture) {
      glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFramebuffer);
      glClearColor(0.0, 0.0, 0.0, 0.0);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    } else glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  }

  Polygons mCMap;
  GLplaneList mExtraShadows;
  bool mUpdate;

  Polygons mMinimap;
  Polygons mMinimapMask;
  GLParticle * mMinimapParticle;
  GLfloat mMinimapZoom;
  void updateMinimap();

  CollisionCallback mCallback;
  void * mCallbackUserData;

  GLParticle * mSpot;
  GLParticle * mConeSpot;
  MapObjectList mMapObjects;
  LightSourceList mLightSources;
  CollidableList mCollidables;
  ProjectileList mProjectiles;
  AnimationList mAnimations;

  void generate();
  void collide(GLplane * p);
  void collideProjectiles();
  void genCirclePolygon(GLvector2f pos, GLfloat size, Polygon & polygon, bool random = true, GLfloat segments = 12);
};

#endif
