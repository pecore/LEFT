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

  mPos.x = 0.0f;
  mPos.y = 0.0f;
  mAngle = 0.0f;
  mRotation.x = 0.0f;
  mRotation.y = 0.0f;
  ma = 1.0f;

  mpData = 0;
  mTexture = 0;
  mDisplayList = 0;
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

    mTexture = 0;
    mInitialized = false;
  } else {
    mTexture = res->texture;
    mWidth = res->width;
    mHeight = res->height;

    mpData = 0;
    mInitialized = true;
  }  
  mDisplayList = 0;

  mPos.x = 0.0f;
  mPos.y = 0.0f;
  mAngle = 0.0f;
  mRotation.x = 0.0f;
  mRotation.y = 0.0f;
  ma = 1.0f;
}

GLSprite::~GLSprite()
{
  glDeleteLists(mDisplayList, 1);
  if(mpData) { 
    delete mpData;
    mpData = 0;
  }
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

  if(result) {
    buildList();
  }

  return result;
}

void GLSprite::buildList()
{
  mDisplayList = glGenLists(1);
  glNewList(mDisplayList, GL_COMPILE);
  glBindTexture(GL_TEXTURE_2D, mTexture);
  GL_ASSERT();
  glBegin(GL_QUADS);
	  glTexCoord2f(0.0f, 0.0f); 
    glVertex3f( -(mWidth / 2.0f),  -(mHeight / 2.0f), 0.0f);
		
    glTexCoord2f(1.0f, 0.0f);
    glVertex3f( +(mWidth / 2.0f),  -(mHeight / 2.0f), 0.0f);

    glTexCoord2f(1.0f, 1.0f); 
    glVertex3f( +(mWidth / 2.0f),  +(mHeight / 2.0f), 0.0f);
		
    glTexCoord2f(0.0f, 1.0f); 
    glVertex3f( -(mWidth / 2.0f),  +(mHeight / 2.0f), 0.0f);
  glEnd();
  glBindTexture(GL_TEXTURE_2D, 0);
  glEndList();
  GL_ASSERT();
}

void GLSprite::draw()
{
  if(!mInitialized) {
    mInitialized = prepare();
  } else if(!mDisplayList) {
    buildList();
  } assert(mInitialized);

  GLvector2f pos = mPos - GL_SCREEN_BOTTOMLEFT;
  GLvector2f rot = mRotation - GL_SCREEN_BOTTOMLEFT;

  glPushMatrix();
  glTranslatef(rot.x, rot.y, 0.0f);
  glRotatef(mAngle, 0.0f, 0.0f, 1.0f);
  glTranslatef(-rot.x, -rot.y, 0.0f);

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  colorMask();
  glTranslatef(pos.x, pos.y, 0.0f);
  glCallList(mDisplayList);
  glTranslatef(-pos.x, -pos.y, 0.0f);
  glDisable(GL_BLEND);
  
  glPopMatrix();
}

GLAnimatedSprite::GLAnimatedSprite(const char * filename, GLvector2f pos, GLfloat width, GLfloat height)
{
  mSprite = new GLSprite(filename);
  mWidth = width;
  mHeight = height;
  mPos = pos;

  int colcount = (int)(mSprite->w() / mWidth);
  int rowcount = (int)(mSprite->h() / mHeight);
  mCount = colcount * rowcount;
  mDisplayLists = glGenLists(mCount);

  for(int index = 0; index < colcount * rowcount; index++) {
    int xindex = index % colcount;
    int yindex = (rowcount-1) - (index / colcount);

    GLvector2f tsize(mWidth / mSprite->w(), mHeight / mSprite->h());
    GLvector2f tindex(xindex * tsize.x, yindex * tsize.y);

    glNewList(mDisplayLists + index, GL_COMPILE);
    glBindTexture(GL_TEXTURE_2D, mSprite->texture());
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    glBegin(GL_QUADS);
		  glTexCoord2f(tindex.x, tindex.y); 
      glVertex3f(-(width / 2), -(height / 2),  0.0f);
  		
      glTexCoord2f(tindex.x + tsize.x, tindex.y);
      glVertex3f(+(width / 2), -(height / 2),  0.0f);

      glTexCoord2f(tindex.x + tsize.x, tindex.y + tsize.y); 
      glVertex3f(+(width / 2), +(height / 2),  0.0f);
  		
      glTexCoord2f(tindex.x, tindex.y + tsize.y); 
      glVertex3f(-(width / 2), +(height / 2),  0.0f);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glEndList();
  }
}

GLAnimatedSprite::~GLAnimatedSprite()
{
  glDeleteLists(mDisplayLists, mCount);
  delete mSprite;
}

bool GLAnimatedSprite::draw(int index)
{
  if(index >= mCount) return false;
  GLvector2f pos = mPos - GL_SCREEN_BOTTOMLEFT;
  GLvector2f rot = mRotation - GL_SCREEN_BOTTOMLEFT;

  glPushMatrix();
  glTranslatef(rot.x, rot.y, 0.0f);
  glRotatef(mAngle, 0.0f, 0.0f, 1.0f);
  glTranslatef(-rot.x, -rot.y, 0.0f);
  glPopMatrix();

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  glTranslatef(pos.x, pos.y, 0.0f);
  glCallList(mDisplayLists + index);
  glTranslatef(-pos.x, -pos.y, 0.0f);
  glDisable(GL_BLEND);

  return true;
}