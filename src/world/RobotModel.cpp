/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "RobotModel.h"
#define _USE_MATH_DEFINES
#include <math.h>

#include "SoundPlayer.h"
#include "Map.h"
#include "Debug.h"

#define ROCKET_EFFECT_WIDTH 18.0f
#define ROCKET_EFFECT_HEIGHT 32.0f

RobotModel::RobotModel(Map * map) : mMap(map)
{
  mHUD = new HUD();
  mBodySprite = new GLSprite("data\\robotv3.png");
  mWeaponArmSprite = new GLSprite("data\\arm.png");

  mPos.x = GL_SCREEN_FWIDTH / 2.0f - mBodySprite->w() / 2.0f;
  mPos.y = GL_SCREEN_FHEIGHT / 2.0f - mBodySprite->h() / 2.0f;
  mBodySprite->moveTo(mPos.x, mPos.y);
  mWeaponArmSprite->moveTo(mPos.x, mPos.y);

  mRocketEffect = new RobotRocketEffect(mPos.x - (ROCKET_EFFECT_WIDTH / 2.0f), mPos.y - mBodySprite->h() / 2.0f, ROCKET_EFFECT_WIDTH, ROCKET_EFFECT_HEIGHT);
  mStablizeEffect = new RobotStabilizeEffect(mPos.x, mPos.y, 40.0f);

  mMass = 1.0f;
  mRocketBoost = 1.0f;
  mAngle = 0.0f;
  mStable = false;
}

RobotModel::~RobotModel()
{
  if(mBodySprite) {
    delete mBodySprite;
  }
  if(mRocketEffect) {
    delete mRocketEffect;
  }
  if(mStablizeEffect) {
    delete mStablizeEffect;
  }
  if(mWeaponArmSprite) {
    delete mWeaponArmSprite;
  }
  if(mHUD) {
    delete mHUD;
  }
}

void RobotModel::control(const bool * keydown, GLvector2f mousepos, unsigned int mousestate)
{
  if(mousestate & MK_LBUTTON) {
    switch(mHUD->getActive()) {
    case 1:
      mMap->addProjectile(new RocketProjectile(mPos, (mousepos - mPos).normal() * 5.0f, mMap));
      break;
    case 2:
      for(int i = 0; i < 6; i++) {
        mMap->addProjectile(new ShotgunProjectile(mPos, (mousepos - mPos).rotate(frand() * 0.6).normal() * (frand() + 3.0f) * 4.0f, mMap));
      }
      break;
    case 3:
      break;
    }
  }
  if(mousestate & MK_RBUTTON) {
    mMap->LightSources().push_back(new LightSource(mPos, GLvector3f(0.1f, 0.1f, 0.1f), 1.0f));
  }

  if(keydown['1']) mHUD->setActive(1);
  if(keydown['2']) mHUD->setActive(2);
  if(keydown['3']) mHUD->setActive(3);

  // Stabilize
  mStable = keydown[VK_SPACE];
  mStablizeEffect->moveTo(mPos.x, mPos.y);
  if(mStable) {
    mRocketBoost = 1.0f + (0.81f/9.0f);
    if(mVelocity.len() > 0.1f) {
      mVelocity *= 0.90f;
    } else {
      mVelocity.x = 0;
      mVelocity.y = 0;
    } 
    if(!mReset) {
      mStablizeEffect->reset();
      mReset = true;
    }
  } else {
    mReset = false;
  }

  if(!mStable) {
    // Turn Robot
    if(keydown['A']) {
      mAngle += 4.5f;
      if(mAngle > 60.0f) mAngle = 60.0f;
    }
    if(keydown['D']) {
      mAngle -= 4.5f;
      if(mAngle < -60.0f) mAngle = -60.0f;
    }

    // Control Boost
    if(keydown['W']) {
      mRocketBoost = 2.0f;
    } else 
    if(keydown['S']) {
      mRocketBoost = 0.5f;
    } else {
      mRocketBoost = 1.0f;
    }
  }

  // Auto Adjust Angle
  if(mStable || (!keydown['A'] && !keydown['D'])) {
    GLfloat step = 2.0f;
    if(mStable) step = abs(mAngle / 6.0f);
    if(mAngle < 0.0f)  mAngle += step;
    else               mAngle -= step;
  }

  mWeaponArmSprite->setRotation(mPos.x, mPos.y, ((mousepos - mPos).angle() / M_PI) * 180.0f);
}

bool RobotModel::collide(GLvector2f n, GLfloat distance)
{
  mPos -= n * ((mBodySprite->h() / 2.0f) - distance);
  mVelocity -= n * 2.0f * mVelocity.dot(n);
  mVelocity *= 0.5f;
  return true;
}

void RobotModel::integrate(GLfloat dt)
{ 
  mBodySprite->moveTo(mPos.x, mPos.y);
  mWeaponArmSprite->moveTo(mPos.x, mPos.y);
  mRocketEffect->moveTo(mPos.x - (ROCKET_EFFECT_WIDTH / 2.0f), mPos.y - mBodySprite->h() / 2.0f + 3.0f);
  mBodySprite->setRotation(mBodySprite->pos().x, mBodySprite->pos().y, mAngle);
  mRocketEffect->setRotation(mBodySprite->pos().x, mBodySprite->pos().y, mAngle);

  mVelocity += mRocketEffect->direction().normal() * -9.0f * mRocketBoost * dt;

  const GLfloat maxspeed = 150.0f;
  if(mVelocity.len() > maxspeed) mVelocity = mVelocity.normal() * maxspeed;

  update(dt);
  mPos += mVelocity * dt;
}

void RobotModel::moveTo(GLfloat x, GLfloat y)
{ 
  mPos.x = x; 
  mPos.y = y; 
}

void RobotModel::draw()
{
  mBodySprite->draw();
  mWeaponArmSprite->draw();
  if(mStable) {
    mStablizeEffect->draw();
  } else {
    mRocketEffect->draw();
  }
  mHUD->draw();
}
