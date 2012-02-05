/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _GLSPRITE_H
#define _GLSPRITE_H

#include "GLDefines.h"
#include "Collidable.h"

class GLSprite : public Collidable {
public:
  GLSprite(const char * filename, int width = -1, int height = -1);
  ~GLSprite();

  GLuint texture() { return mTexture; };

  GLvector2f pos() { return mPos; };
  GLfloat w() { return mWidth; };
  GLfloat h() { return mHeight; };
  void setSize(GLfloat w, GLfloat h) { mWidth = w; mHeight = h; }
  bool collide(GLvector2f n, GLfloat distance) { return true; };

  void moveTo(GLfloat x, GLfloat y) { mPos.x = x; mPos.y = y; };
  void setRotation(GLfloat x, GLfloat y, GLfloat angle) { mRotation.x = x; mRotation.y = y; mAngle = angle; };
  void setScale(GLfloat scale) { mScale = scale; }
  void setAlpha(GLfloat alpha) { ma = alpha; }
  virtual void draw();

protected:
  GLSprite();
  GLuint mTexture;
  GLvector2f mPos;
  GLvector2f mRotation;
  GLfloat mAngle;
  GLfloat mScale;
  GLfloat ma;

  const char * mFilename;
  GLfloat mWidth;
  GLfloat mHeight;
  bool mInitialized;

  unsigned char * mpData;
  int mSizeX;
  int mSizeY;

  bool prepare();
  virtual void colorMask();
};

class GLAnimatedSprite {
public:
  GLAnimatedSprite(const char * filename, GLvector2f pos, GLfloat width, GLfloat height);
  ~GLAnimatedSprite();

  bool draw(int index);

private:
  GLSprite * mSprite;
  GLfloat mWidth;
  GLfloat mHeight;

  GLvector2f mPos;
  GLvector2f mRotation;
  GLfloat mAngle;
};

#endif
