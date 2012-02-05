/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "RobotRocketEffect.h"
#define _USE_MATH_DEFINES
#include <math.h>

RobotRocketEffect::RobotRocketEffect(GLfloat x, GLfloat y, GLfloat width, GLfloat height, int strength, int count) : GLParticleEffect(strength, strength)
{
  mCount = count;
  mPos.x = x;
  mPos.y = y;
  mRotation.x = 0.0f;
  mRotation.y = 0.0f;
  mAngle = 0.0f;
  mAlpha = 1.0f;

  mWidth = width;
  mHeight = height;
  mVelocity.x = 0.0f;
  mVelocity.y = 0.0f;

  recalculateVectors();
  mMaxDistance = (msrcMid - mdestMid).len();

  for(int i = 0; i < mCount; i++) {
    GLParticleDummy * p = addParticle();
    p->moveTo(0.0f, 0.0f);
    mLifeTime[i] = 0;
  }
  mColor[0] = yellow - red;
  mColor[1] = blue - yellow; 
  mColor[2] = GLvector3f(0.0f, 0.0f, 0.0f); 
}

RobotRocketEffect::~RobotRocketEffect()
{
}

void RobotRocketEffect::recalculateVectors()
{
  msrcA.x = mPos.x;
  msrcA.y = mPos.y;
  msrcB.x = mPos.x + mWidth;
  msrcB.y = mPos.y;
  mdestA.x = mPos.x + (mWidth / 3);
  mdestA.y = mPos.y - mHeight;
  mdestB.x = mPos.x + (mWidth * 2 / 3);
  mdestB.y = mPos.y - mHeight;
  if(mAngle != 0.0f) {
    msrcA -= mRotation;
    msrcB -= mRotation;
    mdestA -= mRotation;
    mdestB -= mRotation;

    GLfloat bow = 2.0f * (GLfloat)M_PI * (mAngle / 360.0f);
    msrcA <<= bow;
    msrcB <<= bow;
    mdestA <<= bow;
    mdestB <<= bow;

    msrcA += mRotation;
    msrcB += mRotation;
    mdestA += mRotation;
    mdestB += mRotation;
  }

  msrcAB = msrcB - msrcA; 
  mdestAB = mdestB - mdestA; 
  msrcMid = msrcA + (msrcAB / 2.0f);
  mdestMid = mdestA + (mdestAB / 2.0f);

  mMaxDistance = (msrcMid - mdestMid).len();
}

void RobotRocketEffect::spawn(GLParticleDummy * p, int i)
{
  GLfloat distance = (msrcMid - p->pos()).len();

  mLifeTime[i]--;
  if(mLifeTime[i] <= 0) {
    GLvector2f src = msrcA + (msrcAB * frand());
    GLvector2f dest = mdestA + (mdestAB * frand());
    
    p->moveTo(src.x, src.y);
  
    GLvector2f target = dest - src;
    GLfloat k = (1.0f + frand()) * 5.0f;

    p->setVelocity(target.normal() * k);
    mLifeTime[i] = mMaxDistance / k;
    distance = 0.0f;
  }
  
  // 10 % RED
  if(distance == 0.0f) {
    GLfloat k = distance / (mMaxDistance * 0.1f);
    p->setColor(red + (mColor[0] * k), 1.0f);
    if(distance > 0.0) 
      k = k;
  } else 

  // 40 % YELLOW
  if(distance < (mMaxDistance * 0.5f)) {
    GLfloat k = (distance - (mMaxDistance * 0.1f)) / (mMaxDistance * 0.4f);
    p->setColor(yellow + (mColor[1] * k), 1.0f);
  } else 

  // 50 % BLUE
  if(distance > (mMaxDistance * 0.5f)) {
    GLfloat k = (distance - (mMaxDistance * 0.5f)) / (mMaxDistance * 0.5f);
    p->setColor(blue + (mColor[2] * k), 1.0f - k);
  }
}

void RobotRocketEffect::moveTo(GLfloat x, GLfloat y)
{
  GLvector2f diff = mPos - GLvector2f(x, y);
  mPos.x = x; 
  mPos.y = y;

  recalculateVectors();
  for(int i = 0; i < mCount; i++) {
    GLParticleDummy * p = getParticle(i);
    GLvector2f pos = p->pos();
    GLvector2f dest = mdestA + (mdestAB * frand());

    p->moveTo(pos.x - diff.x, pos.y - diff.y);
    p->setVelocity((dest - pos).normal() * frand() * 6.0f);  
  }
}

void RobotRocketEffect::setRotation(GLfloat x, GLfloat y, GLfloat angle)
{ 
  mRotation.x = x;
  mRotation.y = y;
  mAngle = angle;
  recalculateVectors();
}

void RobotRocketEffect::draw()
{
  if(mHeight <= 0.0f) return;
  for(int i = 0; i < mCount; i++) {
    GLParticleDummy * p = getParticle(i);
    spawn(p, i);
    p->move();
    p->setAlpha(mAlpha);
    p->draw();
  }
}
