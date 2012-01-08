/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _ROBOTMODEL_H_
#define _ROBOTMODEL_H_

#include "GLSprite.h"
#include "RobotRocketEffect.h"
#include "RobotStabilizeEffect.h"
#include "RigidBody.h"

class RobotModel : public RigidBody {
public:
  RobotModel();
  ~RobotModel();

  GLfloat w() { return mBodySprite->w(); };
  GLfloat h() { return mBodySprite->h(); };
  bool collide(GLvector2f n, GLfloat distance);

  void integrate(GLfloat dt);
  void moveTo(GLfloat x, GLfloat y);
  
  void setVelocity2f(GLfloat x, GLfloat y) { mVelocity.x = x; mVelocity.y = y; };
  void setVelocity(GLvector2f & v) { mVelocity = v; };
  void addVelocity(GLvector2f & v) { mVelocity += v; };
  void setBoost(GLfloat b) { mRocketBoost = b; };
  void setAngle(GLfloat a) { mAngle = a; };

  void control(const bool * keydown, GLvector2f mousepos);
  void draw();

private:
  GLSprite * mBodySprite;
  RobotRocketEffect * mRocketEffect;
  RobotStabilizeEffect * mStablizeEffect;
  GLSprite * mWeaponArmSprite;

  GLfloat mRocketBoost;
  GLfloat mAngle;
  bool mStable;
  bool mReset;
};

#endif
