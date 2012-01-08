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

class Projectile : public Collidable {
public:
  virtual void init() = 0;

  GLvector2f pos() { return mPos; };
  GLfloat w() { return mWidth; };
  GLfloat h() { return mHeight; };

  void move() { mPos += mVelocity; };
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

  bool collide(GLvector2f n, GLfloat distance);
  void draw();

private:
  Map * mMap;
  GLSprite * mSprite;
  RobotRocketEffect * mRocketEffect;
  LightSource * mLight;
};

#endif
