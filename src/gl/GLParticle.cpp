/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "GLParticle.h"
#include <math.h>

GLParticle::GLParticle()
{
  mFilename = "";

  mpData = 0;
  mAngle = 0.0f;
  mPos.x = 0.0f;
  mPos.y = 0.0f;
  
  mr = 1.0f;
  mg = 1.0f;
  mb = 1.0f; 
  ma = 1.0f;

  mInitialized = false;
}

GLParticle::GLParticle(int width, int height, GLfloat r, GLfloat g, GLfloat b, GLfloat a, particle_t form)
{
  mFilename = "";
  mSizeX = mWidth = width;
  mSizeY =  mHeight = height;
  mForm = form;

  mr = r;
  mg = g;
  mb = b; 
  ma = a;

  generate();
  mInitialized = prepare();
}

GLParticle::~GLParticle()
{
  if(mpData) delete mpData;
  mpData = 0;
}

void GLParticle::generate()
{
  unsigned int width = (unsigned int)mWidth;
  unsigned int height = (unsigned int)mHeight;

  if(mpData) delete mpData;
  mpData = 0;

  int size = width * height * 4;
  mpData = new unsigned char[size];

  for(int i = 0; i < size; i += 4) {
    int x = (i / 4) % width;
    int y = (i / 4) / width;
    mpData[i] = 0xFF;
    mpData[i + 1] = 0xFF;
    mpData[i + 2] = 0xFF;
    mpData[i + 3] = getAlpha(x, y);
  }
}

void GLParticle::colorMask()
{
   glColor4f(mr, mg, mb, ma);
}


void GLParticle::collide(GLvector2f n, GLfloat distance)
{
  mPos -= n * ((mWidth / 2.0f) - distance);
  mVelocity -= n * 2.0f * mVelocity.dot(n);
}

char GLParticle::getAlpha(int x, int y)
{
  double cx = x - (mWidth / 2);
  double cy = mHeight - (y + (mHeight / 2));
  double alpha = 0.0f;
  double max_distance = mWidth < mHeight ? mWidth / 2 : mHeight / 2;
  double distance = sqrt( cx*cx + cy*cy );
  bool col = false;
  bool row = false;

  switch(mForm) {
  case glpCircle:
    if(distance <= max_distance) {
      alpha = 1.0f - (distance / max_distance);
    } else {
      alpha = 0.0f;
    }
    break;
  case glpCross:
    col = (y > (mHeight / 2) - 3.0f && y < (mHeight / 2) + 3.0f);
    row = (x > (mWidth  / 2) - 3.0f && x < (mWidth  / 2) + 3.0f);
    
    alpha = 0.0f;
    if(col ^ row) {
      if(distance <= max_distance) {
        alpha = 1.0f - (distance / max_distance);
      }
    }
    break;
  case glpSolid:
    alpha = ma;
    break;
  case glpLight:
    if(distance <= max_distance) {
      alpha = 5000.0f / (distance * distance);
    } else {
      alpha = 0.0f;
    }
    if(alpha > 1.0f) alpha = 1.0f; 
    break;
  }

  return (char)(alpha * 255.0f); 
}