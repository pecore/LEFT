/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _GLDEFINES_H_
#define _GLDEFINES_H_

#define LEFT_USE_FAST_SQRT 1

#include "GLResources.h"
extern GLResources * gResources;

#define frand() ((GLfloat) rand() / (GLfloat)RAND_MAX)
#define rfrand(from, to) ((((GLfloat) rand() / (GLfloat)RAND_MAX) + from) * (to - from))
#define GL_ALPHA_CUTOFF 0.08f

class GLvector2f {
public:
  GLfloat x;
  GLfloat y;

  GLvector2f();
  GLvector2f(GLfloat _x, GLfloat _y);
  GLvector2f(const GLvector2f & v);

  GLvector2f operator+(const GLvector2f & right);
  GLvector2f operator-(const GLvector2f & right);
  GLvector2f operator*(const GLfloat & right);
  GLvector2f operator/(const GLfloat & right);
  GLvector2f rotate(const GLfloat & angle);
  void operator+=(const GLvector2f & right);
  void operator-=(const GLvector2f & right);
  void operator*=(const GLfloat & right);
  void operator/=(const GLfloat & right);
  void operator<<=(const GLfloat & angle);
  
  bool operator==(const GLvector2f & right);
  bool operator!=(const GLvector2f & right);

  GLfloat angle();
  GLfloat len();
  GLfloat fastlen();
  GLvector2f normal();
  GLfloat dot(const GLvector2f & right);
  static void intersect(const GLvector2f & baseA, const GLvector2f & dirA, const GLvector2f & baseB, const GLvector2f & dirB, GLfloat & coeffA, GLfloat & coeffB);
};

class GLvector3f {
public:
  GLfloat x;
  GLfloat y;
  GLfloat z;

  GLvector3f();
  GLvector3f(GLfloat _x, GLfloat _y, GLfloat _z);
  GLvector3f(const GLvector3f &v);

  GLvector3f operator+(const GLvector3f & right);
  GLvector3f operator-(const GLvector3f & right);
  GLvector3f operator*(const GLfloat & right);
  GLvector3f operator/(const GLfloat & right);
};

class GLtriangle {
public:
  GLvector2f A;
  GLvector2f B;
  GLvector2f C;

  GLtriangle(GLvector2f & v1, GLvector2f & v2, GLvector2f & v3);
  ~GLtriangle();
};

class GLplane {
public:
  GLvector2f base;
  GLvector2f dir;
  GLvector2f dest;
  GLvector2f n() { return GLvector2f((dest.y - base.y), -(dest.x - base.x)); }

  bool bordered;

  GLplane();
  GLplane(GLvector2f & _base, GLvector2f & _dir);
  ~GLplane();
};

typedef std::list<GLplane *> GLplaneList;
typedef std::list<GLvector2f *> GLpolygon;

static GLvector3f red(1.0f, 0.0f, 0.0f);
static GLvector3f yellow(1.0f, 0.7f, 0.0f);
static GLvector3f blue(0.3f, 0.6f, 0.9f);
static GLvector3f black(0.0f, 0.0f, 0.0f);
static GLvector2f gravity(0.0f, -9.81f);

extern GLvector2f gScreen;
extern GLvector2f gScreenSize;
#define GL_SCREEN_BOTTOMLEFT gScreen
#define GL_SCREEN_CENTER     (gScreen + GLvector2f(GL_SCREEN_FWIDTH / 2.0f, GL_SCREEN_FHEIGHT / 2.0f))
#define GL_SCREEN_SIZE       gScreenSize
#define GL_SCREEN_IWIDTH     (int)gScreenSize.x
#define GL_SCREEN_FWIDTH     gScreenSize.x
#define GL_SCREEN_IHEIGHT    (int)gScreenSize.y
#define GL_SCREEN_FHEIGHT    gScreenSize.y
#ifndef _DEBUG
#define GL_ASSERT() assert(GL_NO_ERROR == glGetError());
#else
#define GL_ASSERT() {GLenum err; if(GL_NO_ERROR != (err = glGetError())) Debug::Log("glError: %d 0x%08x [%s %d]", err, err, __FILE__, __LINE__);}
#endif
#endif
