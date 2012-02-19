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
  void setSize(GLfloat w, GLfloat h) { 
    if(w != mWidth || h != mHeight) {
      mWidth = w;
      mHeight = h;
      buildList();
    }
  }
  bool collide(GLvector2f n, GLfloat distance) { return true; };

  void moveTo(GLfloat x, GLfloat y) { mPos.x = x; mPos.y = y; };
  void setRotation(GLfloat x, GLfloat y, GLfloat angle) { mRotation.x = x; mRotation.y = y; mAngle = angle; };
  virtual void draw();

  void setColor4f(GLfloat r, GLfloat g, GLfloat b, GLfloat a) { mr = r; mg = g; mb = b; ma = a; };
  void setColor(GLvector3f & c, GLfloat a) { mr = c.x; mg = c.y; mb = c.z; ma = a; };
  GLvector3f getColor() { return GLvector3f(mr, mg, mb); }
  void setAlpha(GLfloat alpha) { ma = alpha; }
  

protected:
  GLSprite();
  GLuint mTexture;
  GLuint mDisplayList;

  GLvector2f mPos;
  GLvector2f mRotation;
  GLfloat mAngle;

  GLfloat mr;
  GLfloat mg;
  GLfloat mb;
  GLfloat ma;

  const char * mFilename;
  GLfloat mWidth;
  GLfloat mHeight;
  bool mInitialized;

  unsigned char * mpData;
  int mSizeX;
  int mSizeY;

  void buildList();
  bool prepare();
  virtual void colorMask();
};

class GLAnimatedSprite {
public:
  GLAnimatedSprite(const char * filename, GLvector2f pos, GLfloat width, GLfloat height);
  ~GLAnimatedSprite();

  void moveTo(GLfloat x, GLfloat y) { mPos.x = x; mPos.y = y; };
  bool draw(int index);

private:
  bool mInitialized;
  GLSprite * mSprite;
  GLuint mDisplayLists;
  unsigned int mCount;

  GLfloat mWidth;
  GLfloat mHeight;

  GLvector2f mPos;
  GLvector2f mRotation;
  GLfloat mAngle;

  bool prepare();
};

#endif
