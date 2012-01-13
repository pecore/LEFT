/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "GLSprite.h"
#include "GLResources.h"
#include "Debug.h"

GLSprite::GLSprite()
{
  mFilename = "";
  mScale = 1.0f;

  mpData = 0;
  mAngle = 0.0f;
  mPos.x = 0.0f;
  mPos.y = 0.0f;

  mInitialized = false;
}

GLSprite::GLSprite(const char * filename, int width, int height)
{
  GLTextureResource * res = (GLTextureResource *) gResources->get(filename);
  if(!res) {
    unsigned int size;
    int width, height;
    gResources->load(filename, &mpData, width, height, size);
    mWidth = (GLfloat) (mSizeX = width);
    mHeight = (GLfloat) (mSizeY = height);
    prepare();
  } else {
    mTexture = res->texture;
    mWidth = res->width;
    mHeight = res->height;
  }
  mScale = 1.0f;
  ma = 1.0f;

  mpData = 0;
  mPos.x = 0.0f;
  mPos.y = 0.0f;
  mAngle = 0.0f;
  mRotation.x = 0.0f;
  mRotation.y = 0.0f;
  mInitialized = true;
}

GLSprite::~GLSprite()
{
  if(mpData) { 
    delete mpData;
  }
  mpData = 0;
}

void GLSprite::colorMask()
{
  glColor4f(1.0f, 1.0f, 1.0f, ma);
}

bool GLSprite::prepare()
{
  bool result = true;

  if(!mpData || (mWidth == 0)) { 
    result = false;
  }

  if(result) {
    glGenTextures(1, &mTexture);
    glBindTexture(GL_TEXTURE_2D, mTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, mSizeX, mSizeY, 0, GL_RGBA, GL_UNSIGNED_BYTE, mpData);
    GL_ASSERT();
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glBindTexture(GL_TEXTURE_2D, 0);

    delete mpData;
    mpData = 0;
  }

  return result;
}

void GLSprite::draw()
{
  if(!mInitialized) return;
  GLfloat width = mWidth * mScale;
  GLfloat height = mHeight * mScale;

  GLvector2f pos = mPos - GL_SCREEN_BOTTOMLEFT;
  GLvector2f rot = mRotation - GL_SCREEN_BOTTOMLEFT;

  glPushMatrix();
  glTranslatef(rot.x, rot.y, 0.0f);
  glRotatef(mAngle, 0.0f, 0.0f, 1.0f);
  glTranslatef(-rot.x, -rot.y, 0.0f);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  colorMask();
  glBindTexture(GL_TEXTURE_2D, mTexture);
  GL_ASSERT();
  glBegin(GL_QUADS);
		glTexCoord2f(0.0f, 0.0f); 
    glVertex3f(pos.x - (width / 2), pos.y - (height / 2),  0.0f);
		
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f(pos.x + (width / 2), pos.y - (height / 2),  0.0f);

    glTexCoord2f(1.0f, 1.0f); 
    glVertex3f(pos.x + (width / 2), pos.y + (height / 2),  0.0f);
		
    glTexCoord2f(0.0f, 1.0f); 
    glVertex3f(pos.x - (width / 2), pos.y + (height / 2),  0.0f);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);
  
  glPopMatrix();
}

GLAnimatedSprite::GLAnimatedSprite(const char * filename, GLvector2f pos, GLfloat width, GLfloat height)
{
  mSprite = new GLSprite(filename);
  mWidth = width;
  mHeight = height;
  mPos = pos;
}

GLAnimatedSprite::~GLAnimatedSprite()
{
  delete mSprite;
}

bool GLAnimatedSprite::draw(int index)
{
  GLfloat width = mWidth;
  GLfloat height = mHeight;
  int colcount = (int)(mSprite->w() / mWidth);
  int rowcount = (int)(mSprite->h() / mHeight);
  if(index >= colcount * rowcount) {
    return false;
  }
  int xindex = index % colcount;
  int yindex = (rowcount-1) - (index / colcount);

  GLvector2f pos = mPos - GL_SCREEN_BOTTOMLEFT;
  GLvector2f rot = mRotation - GL_SCREEN_BOTTOMLEFT;

  glPushMatrix();
  glTranslatef(rot.x, rot.y, 0.0f);
  glRotatef(mAngle, 0.0f, 0.0f, 1.0f);
  glTranslatef(-rot.x, -rot.y, 0.0f);
  glPopMatrix();

  GLvector2f tsize(mWidth / mSprite->w(), mHeight / mSprite->h());
  GLvector2f tindex(xindex * tsize.x, yindex * tsize.y);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glBindTexture(GL_TEXTURE_2D, mSprite->texture());
  glBegin(GL_QUADS);
		glTexCoord2f(tindex.x, tindex.y); 
    glVertex3f(pos.x - (width / 2), pos.y - (height / 2),  0.0f);
		
    glTexCoord2f(tindex.x + tsize.x, tindex.y);
    glVertex3f(pos.x + (width / 2), pos.y - (height / 2),  0.0f);

    glTexCoord2f(tindex.x + tsize.x, tindex.y + tsize.y); 
    glVertex3f(pos.x + (width / 2), pos.y + (height / 2),  0.0f);
		
    glTexCoord2f(tindex.x, tindex.y + tsize.y); 
    glVertex3f(pos.x - (width / 2), pos.y + (height / 2),  0.0f);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glDisable(GL_BLEND);

  return true;
}