#ifndef _GLDEFINES_H_
#define _GLDEFINES_H_

#include "GLWindow.h"
extern GLWindow * gWindow;

#define _USE_MATH_DEFINES
#include <math.h>
#include <list>

#define Lock(mutex) WaitForSingleObject(mutex, INFINITE)
#define Unlock(mutex) ReleaseMutex(mutex)

#define frand() ((GLfloat) rand() / (GLfloat)RAND_MAX)
#define rfrand(from, to) ((((GLfloat) rand() / (GLfloat)RAND_MAX) + from) * (to - from))

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
  GLvector2f normal();
  GLfloat dot(const GLvector2f & right);
  static void crossing(const GLvector2f & baseA, const GLvector2f & dirA, const GLvector2f & baseB, const GLvector2f & dirB, GLfloat & coeffA, GLfloat & coeffB);
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

  GLplane();
  GLplane(GLvector2f & _base, GLvector2f & _dir);
  ~GLplane();
};

#define foreach(T, iter, list)  T::iterator iter ## _ref = list.begin(); \
                                if(iter ## _ref != list.end()) iter = *iter ## _ref; \
                                for(; iter && iter ## _ref != list.end(); iter ## _ref++, iter = (iter ## _ref != list.end()) ? *iter ## _ref : 0)

typedef std::list<GLplane *> GLplaneList;
typedef std::list<GLtriangle *> GLtriangleList;
typedef std::list<GLvector2f *> GLpolygon;

static GLvector3f red(1.0f, 0.0f, 0.0f);
static GLvector3f yellow(1.0f, 0.7f, 0.0f);
static GLvector3f blue(0.3f, 0.6f, 0.9f);
static GLvector3f black(0.0f, 0.0f, 0.0f);
static GLvector2f gravity(0.0f, -9.81f);

#endif
