/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "Projectile.h"

#include "Map.h"
#include "GLSprite.h"
#include "RobotRocketEffect.h"
#include "SoundPlayer.h"
#include "BFGEffect.h"
#include "Debug.h"

ShotgunProjectile::ShotgunProjectile(GLvector2f pos, GLvector2f velocity, Map * map) 
{ 
  type = PROJECTILE_TYPE_SHOTGUN;
  owner = 0;
  mInitialized = false;
  mMap = map;
  mPos = pos;
  mVelocity = velocity;
  mWidth = 20.0f;
  mHeight = 20.0f;
  mMaxDistance = 500.0f;
  mStart = pos;
  init();
}

ShotgunProjectile::~ShotgunProjectile() 
{
  mMap->shadows().remove(mShadow);
  if(mShadow) {
    delete mShadow;
  }
  if(mParticle) {
    delete mParticle;
  }
}

void ShotgunProjectile::init() 
{
  if(mInitialized) return;
  mInitialized = true;
  mShadow = new GLplane(mPos, GLplane(mPos, mVelocity).n().normal() * mHeight);
  mMap->shadows().push_back(mShadow);
  mParticle = new GLParticle(8, 6, 0.6f, 0.6f, 0.0f, 0.8f, glpSolid);
}

void ShotgunProjectile::draw() 
{
  if(!mInitialized) return;
  mParticle->draw();
}

void ShotgunProjectile::move()
{
  if(!mInitialized) return;
  mShadow->base += mVelocity;
  mShadow->dest += mVelocity;
  mParticle->moveTo(mPos.x, mPos.y);
  mParticle->setRotation(mPos.x, mPos.y, frand() * 2.0f * M_PI * 360.0f / (2.0f * M_PI));
  mPos += mVelocity;
}

bool ShotgunProjectile::collide(GLvector2f n, GLfloat distance) 
{ 
  if(!mInitialized) return true;
  mMap->addCirclePolygon(mPos, mWidth, 8);
  return false; 
}



RocketProjectile::RocketProjectile(GLvector2f pos, GLvector2f velocity, Map * map)
{
  type = PROJECTILE_TYPE_ROCKET;
  owner = 0;
  mInitialized = false;
  mMap = map;
  mPos = pos;
  mVelocity = velocity;
  mMaxDistance = 1000.0f;
  mStart = pos;
  init();
}

RocketProjectile::~RocketProjectile()
{
  if(mSprite) {
    delete mSprite;
  }
  if(mRocketEffect) {
    delete mRocketEffect;
  }
  if(mLight) {
    mMap->LightSources().remove(mLight);
    delete mLight;
  }
}

void RocketProjectile::init()
{
  if(mInitialized) return;
  mSprite = new GLSprite("data\\rocketprojectile.png");
  mRocketEffect = new RobotRocketEffect(mPos.x, mPos.y, 14.0f, 18.0f, 4, 20);
  mLight = new LightSource(mPos, GLvector3f(1.0f, 1.0f, 0.0f), 0.1, glpLight);
  mMap->LightSources().push_back(mLight);
  mWidth = mSprite->w();
  mHeight = mSprite->h();
  mExplosionSound = ((GLSoundResource *) gResources->get("data\\bomb.wav"))->sound;
  mInitialized = true; 
}

bool RocketProjectile::collide(GLvector2f n, GLfloat distance)
{
  if(!mInitialized) return true;
  mMap->addCirclePolygon(mPos, 100.0f);
  mMap->playAnimation(new GLAnimatedSprite("data\\explode.png", mPos, 64, 64));
  SoundPlayer::play(mExplosionSound);
  return false;
}

void RocketProjectile::draw()
{
  if(!mInitialized) return;
  GLfloat angle = mVelocity.angle() * 360.0f / (2.0f * M_PI);
  
  mLight->pos = mPos;
  mSprite->moveTo(mPos.x, mPos.y);
  mRocketEffect->moveTo(mPos.x - 7.0f, mPos.y - 3.0f);
  mSprite->setRotation(mSprite->pos().x, mSprite->pos().y, angle);
  mRocketEffect->setRotation(mSprite->pos().x, mSprite->pos().y, angle - 90.0);
  mSprite->draw();
  mRocketEffect->draw();
  mVelocity *= 1.02f;
}



BFGProjectile::BFGProjectile(GLvector2f pos, GLvector2f velocity, Map * map)
{
  type = PROJECTILE_TYPE_BFG;
  owner = 0;
  mInitialized = false;
  mMap = map;
  mPos = pos;
  mVelocity = velocity;
  mMaxDistance = 1200.0f;
  mStart = pos;
  init();
}

BFGProjectile::~BFGProjectile()
{
  if(mBFGEffect) {
    delete mBFGEffect;
  }
  if(mLight) {
    mMap->LightSources().remove(mLight);
    delete mLight;
  }
}

void BFGProjectile::init()
{
  if(mInitialized) return;
  mBFGEffect = new BFGEffect(mPos.x, mPos.y);
  mLight = new LightSource(mPos, GLvector3f(1.0f, 1.0f, 1.0f), 0.8, glpLight);
  mMap->LightSources().push_back(mLight);
  mWidth = mHeight = 20.0f;
  mBFGSound = ((GLSoundResource *) gResources->get("data\\bfg.wav"))->sound;
  SoundPlayer::play(mBFGSound);
  mInitialized = true;
}

bool BFGProjectile::collide(GLvector2f n, GLfloat distance)
{
  if(!mInitialized) return true;
  mMap->addCirclePolygon(mPos, 120.0f);
  return true;
}

void BFGProjectile::draw()
{
  if(!mInitialized) return;
  GLfloat angle = mVelocity.angle() * 360.0f / (2.0f * M_PI);
  
  mLight->pos = mPos;
  mLight->angle = angle;
  mBFGEffect->moveTo(mPos.x - 7.0f, mPos.y - 3.0f);
  mBFGEffect->draw();
}



GrenadeProjectile::GrenadeProjectile(GLvector2f pos, GLvector2f velocity, Map * map)
{
  type = PROJECTILE_TYPE_GRENADE;
  owner = 0;
  mInitialized = false;
  mMap = map;
  mPos = pos;
  mVelocity = velocity;
  mMaxDistance = 0.0f;
  mStart = pos;
  init();
}

GrenadeProjectile::~GrenadeProjectile()
{
  if(mSprite) {
    delete mSprite;
  }
}

void GrenadeProjectile::init()
{
  if(mInitialized) return;
  mSprite = new GLSprite("data\\grenade.png");
  mWidth = mSprite->w();
  mHeight = mSprite->h();
  mExplosionSound = ((GLSoundResource *) gResources->get("data\\bomb.wav"))->sound;
  mInitialized = true;
}

bool GrenadeProjectile::collide(GLvector2f n, GLfloat distance)
{
  if(!mInitialized) return true; 
  mMap->addCirclePolygon(mPos, 300.0f);
  mMap->playAnimation(new GLAnimatedSprite("data\\explode.png", mPos, 64, 64));
  SoundPlayer::play(mExplosionSound);
  return false;
}

void GrenadeProjectile::draw()
{
  if(!mInitialized) return;
  GLfloat angle = mVelocity.angle() * 360.0f / (2.0f * M_PI);
  mSprite->moveTo(mPos.x, mPos.y);
  mSprite->setRotation(mSprite->pos().x, mSprite->pos().y, angle);
  mSprite->setAlpha(mMap->getOpacity(mPos));
  mSprite->draw();
}

NailProjectile::NailProjectile(GLvector2f pos, GLvector2f velocity, Map * map) 
{ 
  type = PROJECTILE_TYPE_NAIL;
  owner = 0;
  mInitialized = false;
  mMap = map;
  mPos = pos;
  mVelocity = velocity;
  mWidth = 20.0f;
  mHeight = 2.0f;
  mMaxDistance = 620.0f;
  mStart = pos;
  init();
}

NailProjectile::~NailProjectile() 
{
  mMap->shadows().remove(mShadow);
  if(mShadow) {
    delete mShadow;
  }
  if(mParticle) {
    delete mParticle;
  }
}

void NailProjectile::init() 
{
  if(mInitialized) return;
  mInitialized = true;
  char sound_filename[32];
  sprintf(sound_filename, "data\\ric%d.wav", (int)(frand() * 100.0f) % 3);
  GLSoundResource * res = gResources->getSound(sound_filename);
  if(res) SoundPlayer::play(res->sound);


  mShadow = new GLplane(mPos, mVelocity.normal() * mWidth);
  mMap->shadows().push_back(mShadow);
  mParticle = new GLParticle(mWidth, mHeight, 0.6f, 0.6f, 0.6f, 1.0f, glpSolid);
}

void NailProjectile::draw() 
{
  if(!mInitialized) return;
  mParticle->draw();
}

void NailProjectile::move()
{
  if(!mInitialized) return;
  GLfloat angle = mVelocity.angle() * 360.0f / (2.0f * M_PI);
  
  mShadow->base += mVelocity;
  mShadow->dest += mVelocity;
  mParticle->moveTo(mPos.x, mPos.y);
  mParticle->setRotation(mPos.x, mPos.y, angle);
  mPos += mVelocity;
}

bool NailProjectile::collide(GLvector2f n, GLfloat distance) 
{ 
  if(!mInitialized) return true;
  mMap->addCirclePolygon(mPos, mWidth, 6);
  return false; 
}
