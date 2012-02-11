/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _PROJECTILE_H_
#define _PROJECTILE_H_

#include "GLDefines.h"
#include "Collidable.h"

class Map;
class GLSprite;
class RobotRocketEffect;
class LightSource;
struct Sound;
class GLParticle;
class BFGEffect;

#define PROJECTILE_TYPE_NONE     0
#define PROJECTILE_TYPE_ROCKET   1
#define PROJECTILE_TYPE_SHOTGUN  2
#define PROJECTILE_TYPE_BFG      3
#define PROJECTILE_TYPE_GRENADE  4
#define PROJECTILE_TYPE_NAIL     5

class Projectile : public Collidable {
public:
  //virtual ~Projectile() { };
  virtual void init() = 0;
  unsigned int type;
  void * owner;

  void moveTo(GLfloat x, GLfloat y) { mPos.x = x; mPos.y = y; };
  GLvector2f pos() { return mPos; };
  GLfloat w() { return mWidth; };
  GLfloat h() { return mHeight; };
  
  virtual void move() = 0;
  virtual void draw() = 0;

  GLvector2f velocity() { return mVelocity; }
  void setVelocity2f(GLfloat x, GLfloat y) { mVelocity.x = x; mVelocity.y = y; };
  void setVelocity(GLvector2f & v) { mVelocity = v; };
  void addVelocity(GLvector2f & v) { mVelocity += v; };

  GLvector2f start() { return mStart; }
  GLfloat maxdistance() { return mMaxDistance; }

protected:
  bool mInitialized;

  GLvector2f mPos;
  GLvector2f mVelocity;
  GLfloat mWidth;
  GLfloat mHeight;
  GLvector2f mStart;
  GLfloat mMaxDistance;
};

class RocketProjectile : public Projectile {
public:
  RocketProjectile(GLvector2f pos, GLvector2f velocity, Map * map);
  ~RocketProjectile();
  void init();
  void move() { mPos += mVelocity; };
  void draw();
  bool collide(GLvector2f n, GLfloat distance);
private:
  Map * mMap;
  GLSprite * mSprite;
  RobotRocketEffect * mRocketEffect;
  LightSource * mLight;
  Sound * mExplosionSound;
};

class ShotgunProjectile : public Projectile {
public:
  ShotgunProjectile(GLvector2f pos, GLvector2f velocity, Map * map);
  ~ShotgunProjectile();
  void init();
  void move();
  void draw();
  bool collide(GLvector2f n, GLfloat distance);
private:
  Map * mMap;
  GLplane * mShadow;
  Sound * mShotgunSound;
  GLParticle * mParticle;
};

class BFGProjectile : public Projectile {
public:
  BFGProjectile(GLvector2f pos, GLvector2f velocity, Map * map);
  ~BFGProjectile();
  void init();
  void move() { mPos += mVelocity; };
  void draw();
  bool collide(GLvector2f n, GLfloat distance);
private:
  Map * mMap;
  BFGEffect * mBFGEffect;
  LightSource * mLight;
  Sound * mBFGSound;
};

class GrenadeProjectile : public Projectile {
public:
  GrenadeProjectile(GLvector2f pos, GLvector2f velocity, Map * map);
  ~GrenadeProjectile();
  void init();
  void move() { 
    mPos += mVelocity;
    mVelocity += gravity * 0.01f;
  };
  void draw();
  bool collide(GLvector2f n, GLfloat distance);
private:
  Map * mMap;
  GLSprite * mSprite;
  Sound * mExplosionSound;
};


class NailProjectile : public Projectile {
public:
  NailProjectile(GLvector2f pos, GLvector2f velocity, Map * map);
  ~NailProjectile();
  void init();
  void move();
  void draw();
  bool collide(GLvector2f n, GLfloat distance);
private:
  Map * mMap;
  GLParticle * mParticle;
  GLplane * mShadow;
};
#endif
