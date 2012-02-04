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

  mRocketEffect = new RobotRocketEffect(mPos.x - (ROCKET_EFFECT_WIDTH / 2.0f), mPos.y - mBodySprite->h() / 2.0f, ROCKET_EFFECT_WIDTH, ROCKET_EFFECT_HEIGHT, 10, 500);
  mStablizeEffect = new RobotStabilizeEffect(mPos.x, mPos.y, 40.0f);

  mMass = 1.0f;
  mRocketBoost = 1.0f;
  mAngle = 0.0f;
  mWeaponAngle = 0.0f;
  mStable = false;
  for(int i = 0; i < 3; i++) {
    mWeaponTimeout[i] = 0.0f;
  }
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

ProjectileList RobotModel::control(const bool * keydown, GLvector2f mousepos, unsigned int mousestate)
{
  ProjectileList result;
  static bool shotgun = false;
  if(shotgun && mWeaponTimeout[1] <= 0.6f) {
    SoundPlayer::play(gResources->getSound("data\\shotgun-reload.wav")->sound);
    shotgun = false;
  }
  if(mWeaponTimeout[mHUD->getActive()-1] <= 0.0f && mousestate & MK_LBUTTON) {
    switch(mHUD->getActive()) {
    case 1: {
        Projectile * p = new RocketProjectile(mPos, (mousepos - mPos).normal() * 5.0f, mMap);
        p->owner = this;
        result.push_back(p);
        mMap->addProjectile(p);
        mWeaponTimeout[0] = 0.1f; // in sec
      } break;
    case 2: {
        for(int i = 0; i < 6; i++) {
          Projectile * p = new ShotgunProjectile(mPos, (mousepos - mPos).rotate(frand() * 0.6).normal() * (frand() + 3.0f) * 4.0f, mMap);
          p->owner = this;
          result.push_back(p);
          mMap->addProjectile(p);
        }
        SoundPlayer::play(gResources->getSound("data\\shotgun.wav")->sound);
        mWeaponTimeout[1] = 1.2f; // in sec
        shotgun = true;
      } break;
    case 3: {
        Projectile * p = new GrenadeProjectile(mPos, (mousepos - mPos).normal() * 8.0f, mMap);
        p->owner = this;
        result.push_back(p);
        mMap->addProjectile(p);
        mWeaponTimeout[2] = 3.0f; // in sec
      } break;
    }
  } else {
    for(int i = 0; i < 3; i++) {
      mWeaponTimeout[i] -= 0.016f;
    }
  }
  for(int i = 0; i < 3; i++) {
    if(mWeaponTimeout[i] > 0.0f) {
      switch(i) {
      case 0: mButtonOpacity[i] = 0.1f + (1.0f - (mWeaponTimeout[i] / 0.1f)); break;
      case 1: mButtonOpacity[i] = 0.1f + (1.0f - (mWeaponTimeout[i] / 1.2f)); break;
      case 2: mButtonOpacity[i] = 0.1f + (1.0f - (mWeaponTimeout[i] / 3.0f)); break;
      }
    } else {
      mButtonOpacity[i] = 1.0f;
    }
  }
  if(mousestate & MK_RBUTTON) {
    mMap->LightSources().push_back(new LightSource(mPos, GLvector3f(0.0f, 0.0f, 0.0f), 1.0f));
  }

  if(keydown['1']) mHUD->setActive(1);
  if(keydown['2']) mHUD->setActive(2);
  if(keydown['3']) mHUD->setActive(3);

  // Stabilize
  if(mStable && !keydown[VK_SPACE]) {
    mVelocity += (mousepos - mPos) * 0.15f;
  }
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
    if(keydown['W'] && mRocketBoost < 3.5f) {
      mRocketBoost += 0.4f;
    } else 
    if(keydown['S'] && mRocketBoost > 0.0f) {
      mRocketBoost -= 0.5f;
    } else if(mRocketBoost > 1.0f) {
      mRocketBoost -= 0.1f;
    } else if(mRocketBoost < 1.0f && !keydown['S']) {
      mRocketBoost += 0.1f;
    }
  }
  mRocketEffect->setHeight(ROCKET_EFFECT_HEIGHT * mRocketBoost);

  // Auto Adjust Angle
  if(mStable || (!keydown['A'] && !keydown['D'])) {
    GLfloat step = 2.0f;
    if(mStable) step = abs(mAngle / 6.0f);
    if(mAngle < 0.0f)  mAngle += step;
    else               mAngle -= step;
  }

  mWeaponArmSprite->setRotation(mPos.x, mPos.y, ((mousepos - mPos).angle() / M_PI) * 180.0f);
  return result;
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
  if(mWeaponAngle != 0.0f) {
    mWeaponArmSprite->setRotation(mPos.x, mPos.y, mWeaponAngle);
  }

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
  mHUD->draw(mButtonOpacity);
}
