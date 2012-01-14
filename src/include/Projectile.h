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

#define PROJECTILE_TYPE_NONE     0
#define PROJECTILE_TYPE_ROCKET   1
#define PROJECTILE_TYPE_SHOTGUN  2


class Projectile : public Collidable {
public:
  //virtual ~Projectile() { };
  virtual void init() = 0;
  unsigned int type;

  GLvector2f pos() { return mPos; };
  GLfloat w() { return mWidth; };
  GLfloat h() { return mHeight; };

  virtual void move() = 0;
  virtual void draw() = 0;

  void setVelocity2f(GLfloat x, GLfloat y) { mVelocity.x = x; mVelocity.y = y; };
  void setVelocity(GLvector2f & v) { mVelocity = v; };
  void addVelocity(GLvector2f & v) { mVelocity += v; };

protected:
  bool mInitialized;

  GLvector2f mPos;
  GLvector2f mVelocity;
  GLfloat mWidth;
  GLfloat mHeight;
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
  void draw() { }
  bool collide(GLvector2f n, GLfloat distance);
private:
  Map * mMap;
  GLplane * mShadow;
  Sound * mShotgunSound;
};

#endif
