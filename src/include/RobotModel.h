/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _ROBOTMODEL_H_
#define _ROBOTMODEL_H_

#include "HUD.h"
#include "RobotRocketEffect.h"
#include "RobotStabilizeEffect.h"
#include "RigidBody.h"
#include "Map.h"

class RobotModel : public RigidBody {
public:
  RobotModel(Map * map, const char * model = GL_RESOURCE_DATAPATH "robotv3.png", const char * arm = GL_RESOURCE_DATAPATH "arm.png", bool external = false);
  ~RobotModel();

  GLfloat angle() { return mAngle; }
  GLfloat w() { return mBodySprite->w(); }
  GLfloat h() { return mBodySprite->h(); }
  bool collide(GLvector2f n, GLfloat distance);

  void integrate(GLfloat dt);
  void moveTo(GLfloat x, GLfloat y);
  
  void setVelocity2f(GLfloat x, GLfloat y) { mVelocity.x = x; mVelocity.y = y; }
  void setVelocity(GLvector2f & v) { mVelocity = v; }
  void addVelocity(GLvector2f & v) { mVelocity += v; }
  void setBoost(GLfloat b) { mRocketBoost = b; }
  void setAngle(GLfloat a) { mAngle = a; }
  void setWeaponAngle(GLfloat a) { mWeaponAngle = a; }

  ProjectileList control(const bool * keydown, GLvector2f mousepos, unsigned int mousestate);
  void setAlpha(GLfloat o) { mAlpha = o; }
  void draw();

  HUD * getHUD() { return mHUD; }

private:
  Map * mMap;
  HUD * mHUD;
  GLfloat mButtonOpacity[4];
  GLfloat mWeaponTimeout[4];

  GLSprite * mBodySprite;
  RobotRocketEffect * mRocketEffect;
  RobotStabilizeEffect * mStabilizeEffect;
  GLSprite * mWeaponArmSprite;

  GLfloat mAlpha;
  GLfloat mRocketBoost;
  GLfloat mAngle;
  GLfloat mWeaponAngle;
  bool mStable;
  GLfloat mTurbo;
  bool mTurboReady;
  bool mKeyCooldown[256];
};

#endif
