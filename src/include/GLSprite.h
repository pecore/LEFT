#ifndef _GLSPRITE_H
#define _GLSPRITE_H

#include "GLDefines.h"
#include "Collidable.h"

class GLSprite : public Collidable {
public:
  GLSprite(const char * filename, int width = -1, int height = -1);
  ~GLSprite();
  bool isInitialized() { return mInitialized; };

  unsigned char * data() { return mpData; };
  GLuint texture() { return mpTextures[1]; };

  GLvector2f pos() { return mPos; };
  GLfloat w() { return mWidth; };
  GLfloat h() { return mHeight; };
  void collide(GLvector2f n, GLfloat distance) { };

  void moveTo(GLfloat x, GLfloat y) { mPos.x = x; mPos.y = y; };
  void setRotation(GLfloat x, GLfloat y, GLfloat angle) { mRotation.x = x; mRotation.y = y; mAngle = angle; };
  void setScale(GLfloat width) {
    mHeight = width * mHeight / mWidth;
    mWidth = width;
  }

  virtual void draw();

protected:
  GLSprite();
  GLvector2f mPos;
  GLvector2f mRotation;
  GLfloat mAngle;

  const char * mFilename;
  GLfloat mWidth;
  GLfloat mHeight;
  bool mInitialized;

  unsigned char * mpData;
  int mSizeX;
  int mSizeY;

  GLuint mpTextures[1];

  bool load();
  bool prepare();
  virtual void colorMask();
};

#endif
