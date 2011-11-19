#ifndef _Debug_H_
#define _Debug_H_

#include <stdio.h>
#include "GLParticle.h"
#include "GLDefines.h"

class Debug {
public:
  static void Log(const char * fmt, ...) {
#ifdef _DEBUG
    char msg[256];
    va_list args;
    va_start(args, fmt);
    vsprintf(msg, fmt, args);
    MessageBox(NULL, msg, "Debug", 0);
    va_end(args);
#endif
  }

  static void drawVector(GLvector2f base, GLvector2f dir) {
#ifdef _DEBUG
    GLParticle * p = new GLParticle(2, 2, 1.0f, 1.0f, 1.0f, PARTICLE_FORM_SOLID);
    int repeat = 1;
    bool showdir = false;

    for(int i = 0; i < repeat; i++) {
      GLvector2f dest = base + dir / (showdir ? ((i+1 == repeat) ? 1.5f : 2.0f) : 1.0f) * (GLfloat)i / (GLfloat)(repeat - 1);
      p->moveTo(dest.x, dest.y);
      p->draw();
    }

    delete p;
#endif
  }

};

#endif
