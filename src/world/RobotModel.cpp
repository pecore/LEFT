/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "RobotModel.h"
#define _USE_MATH_DEFINES
#include <math.h>

//#include "SoundPlayer.h"
#include "Map.h"
#include "Debug.h"

#define ROCKET_EFFECT_WIDTH 18.0f
#define ROCKET_EFFECT_HEIGHT 48.0f

RobotModel::RobotModel(Map * map, const char * model, const char * arm, bool external) : mMap(map)
{
  if(!external) mHUD = new HUD(map);
  else mHUD = 0;
  mBodySprite = new GLSprite(model);
  mWeaponArmSprite = new GLSprite(arm);

  mPos.x = GL_SCREEN_FWIDTH / 2.0f - mBodySprite->w() / 2.0f;
  mPos.y = GL_SCREEN_FHEIGHT / 2.0f - mBodySprite->h() / 2.0f;
  mBodySprite->moveTo(mPos.x, mPos.y);
  mWeaponArmSprite->moveTo(mPos.x, mPos.y);

  mRocketEffect = new RobotRocketEffect(mPos.x - (ROCKET_EFFECT_WIDTH / 2.0f), mPos.y - mBodySprite->h() / 2.0f, ROCKET_EFFECT_WIDTH, ROCKET_EFFECT_HEIGHT, 8, 500);
  mStabilizeEffect = new RobotStabilizeEffect(mPos.x, mPos.y);
  mStabilizeEffect->reset();

  mAlpha = 1.0f;
  mMass = 1.0f;
  mRocketBoost = 1.0f;
  mTurbo = 1.0f;
  mTurboReady = true;
  mAngle = 0.0f;
  mWeaponAngle = 0.0f;
  mStable = false;
  for(int i = 0; i < 3; i++) {
    mWeaponTimeout[i] = 0.0f;
  }
  memset(mKeyCooldown, 0, 256 * sizeof(bool));
}

RobotModel::~RobotModel()
{
  if(mBodySprite) {
    delete mBodySprite;
  }
  if(mRocketEffect) {
    delete mRocketEffect;
  }
  if(mStabilizeEffect) {
    delete mStabilizeEffect;
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
  if(!mHUD) return result;
  static bool shotgun = false;
  if(shotgun && mWeaponTimeout[1] <= 0.6f) {
//    SoundPlayer::play(gResources->getSound("data\\shotgun-reload.wav")->sound);
    shotgun = false;
  }
  if(mWeaponTimeout[mHUD->getActive()-1] <= 0.0f && mousestate) {
    switch(mHUD->getActive()) {
    case 1: {
        Projectile * p = new RocketProjectile(mPos, (mousepos - mPos).normal() * 5.0f, mMap);
        p->owner = this;
        result.push_back(p);
        mMap->addProjectile(p);
        mWeaponTimeout[0] = 0.15f; // in sec
      } break;
    case 2: {
        for(int i = 0; i < 6; i++) {
          Projectile * p = new ShotgunProjectile(mPos, (mousepos - mPos).rotate(frand() * 0.6).normal() * (frand() + 3.0f) * 4.0f, mMap);
          p->owner = this;
          result.push_back(p);
          mMap->addProjectile(p);
        }
//        SoundPlayer::play(gResources->getSound("data\\shotgun.wav")->sound);
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
    case 4: {
        Projectile * p = new NailProjectile(mPos, (mousepos - mPos).normal() * 8.0f, mMap);
        p->owner = this;
        result.push_back(p);
        mMap->addProjectile(p);
        mWeaponTimeout[3] = 0.08f; // in sec
      } break;
    }
  } else {
    for(int i = 0; i < mHUD->count(); i++) {
      mWeaponTimeout[i] -= 0.016f;
    }
  }
  for(int i = 0; i < mHUD->count(); i++) {
    if(mWeaponTimeout[i] > 0.0f) {
      switch(i) {
      case 0: mButtonOpacity[i] = 0.1f + (1.0f - (mWeaponTimeout[i] / 0.15f)); break;
      case 1: mButtonOpacity[i] = 0.1f + (1.0f - (mWeaponTimeout[i] / 1.2f)); break;
      case 2: mButtonOpacity[i] = 0.1f + (1.0f - (mWeaponTimeout[i] / 3.0f)); break;
      case 3: mButtonOpacity[i] = 0.1f + (1.0f - (mWeaponTimeout[i] / 0.08f)); break;
      }
    } else {
      mButtonOpacity[i] = 1.0f;
    }
  }
  //if(mousestate & MK_RBUTTON) {
  //  mMap->LightSources().push_back(new LightSource(mPos, GLvector3f(0.0f, 0.0f, 0.0f), 1.0f));
  //}

  if(keydown['1']) mHUD->setActive(1);
  if(keydown['2']) mHUD->setActive(2);
  if(keydown['3']) mHUD->setActive(3);
  if(keydown['4']) mHUD->setActive(4);
  if(keydown['Q'] && !mKeyCooldown['Q']) {
    mHUD->nextActive();
    mKeyCooldown['Q'] = true;
  }
  if(!keydown['Q'] && mKeyCooldown['Q']) mKeyCooldown['Q'] = false;

  if(keydown[VK_SHIFT] && mTurboReady) {
    mVelocity += (mousepos - mPos) * 0.2f;
    mTurbo -= 0.015f;
    if(mTurbo <= 0.0f) {
      mVelocity *= 0.5f;
      mTurboReady = false;
    }
  } else {
    if(mTurbo >= 1.0f) {
      mTurbo = 1.0f;
      mTurboReady = true;
    } else {
      if(mTurboReady) {
        mTurbo += 0.02f;
      } else {
        mTurbo += 0.005f;
      }
    }
  }

  mHUD->setTurboLoading(!mTurboReady);
  mHUD->setTurboCharge(mTurbo);

  bool mStableButton = keydown['S'] || keydown[VK_SPACE];
  if(mStableButton) {
    if(mVelocity.len() > 5.0f) {
      GLfloat delta = mVelocity.len() - 5.0f;
      GLfloat color = 0.4f + (delta / 150.0f);
      mBodySprite->setColor4f(color/2, color, 1.0f, 1.0f);
      mWeaponArmSprite->setColor4f(color, color, 1.0f, 1.0f);
      mVelocity *= 0.9f;
      mVelocity += gravity * -0.1f;
    } else {
      mStable = true;
      mVelocity = GLvector2f(0.0f, 0.0f);
    }
    mRocketBoost = 0.0f;
  } else {
    mBodySprite->setColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    mWeaponArmSprite->setColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    mStable = false;
  }
  if(!mStable) { 
    // Turn Robot
    if(keydown['A']) mAngle += 6.0f;
    if(keydown['D']) mAngle -= 6.0f;
    if(!(mousestate & MK_RBUTTON)) {
      mAngle = ((mousepos - mPos).angle() / M_PI) * 180.0f - 90.0f;
    }

    // Control Boost
    if(keydown['w'] && mRocketBoost < 4.0f) {
      mRocketBoost += 0.8f;
    } else 
    if(mRocketBoost > 0.0f) {
      mRocketBoost -= 0.5f;
    }
  }

  GLfloat rocketeffectsize = mRocketBoost / 4.0f;
  if(keydown[VK_SHIFT] && mTurboReady && mTurbo < 1.0f) {
    rocketeffectsize = 1.2f;
  }
  mRocketEffect->setHeight(ROCKET_EFFECT_HEIGHT * (rocketeffectsize));
  mWeaponAngle = ((mousepos - mPos).angle() / M_PI) * 180.0f;
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
  if(mStable) {
    mStabilizeEffect->moveTo(mPos.x, mPos.y);
    mStabilizeEffect->setAngle(mAngle);
    mStabilizeEffect->integrate(dt);
    return;
  }
  update(dt);
  mVelocity += mRocketEffect->direction().normal() * -9.0f * mRocketBoost * dt;
  if(mVelocity.len() > 150.0f)  {
    mVelocity = mVelocity.normal() * 150.0f;
  }
  mPos += mVelocity * dt;

  mBodySprite->moveTo(mPos.x, mPos.y);
  mWeaponArmSprite->moveTo(mPos.x, mPos.y);
  mRocketEffect->moveTo(mPos.x - (ROCKET_EFFECT_WIDTH / 2.0f), mPos.y - mBodySprite->h() / 2.0f + 3.0f);
  
  mBodySprite->setRotation(mBodySprite->pos().x, mBodySprite->pos().y, mAngle);
  mRocketEffect->setRotation(mBodySprite->pos().x, mBodySprite->pos().y, mAngle);
  if(mWeaponAngle != 0.0f) {
    mWeaponArmSprite->setRotation(mBodySprite->pos().x, mBodySprite->pos().y, mWeaponAngle);
  }
}

void RobotModel::moveTo(GLfloat x, GLfloat y)
{ 
  mPos.x = x; 
  mPos.y = y;
}

void RobotModel::draw()
{
  mBodySprite->setAlpha(mAlpha);
  mWeaponArmSprite->setAlpha(mAlpha);
  mRocketEffect->setAlpha(mAlpha);

  mBodySprite->draw();
  mWeaponArmSprite->draw();
  if(mStable) {
    mStabilizeEffect->draw();
  } else {
    mRocketEffect->draw();
  }
  //if(mHUD) mHUD->draw(mButtonOpacity);
}
