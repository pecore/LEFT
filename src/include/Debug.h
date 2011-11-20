/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

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

  static bool DebugActive;
  static GLplaneList DebugVectors;
  static void clear() {
    GLplane * p = 0;
    foreach(GLplaneList, p, DebugVectors) {
      delete p;
    }
    DebugVectors.clear();
  }
  static void drawVectors() {
    if(!DebugActive) return;
    GLplane * p = 0;
    foreach(GLplaneList, p, DebugVectors) {
      drawVector(p->base, p->dir);
    }
  }

  static GLParticle * DebugParticle;
  static void drawVector(GLvector2f base, GLvector2f dir) {
    if(!DebugActive) return;
    if(DebugParticle == 0) DebugParticle = new GLParticle(6, 6, 1.0f, 1.0f, 1.0f, PARTICLE_FORM_SOLID);
    int repeat = 10;
    bool showdir = false;

    for(int i = 0; i < repeat; i++) {
      GLvector2f dest = base + dir / (showdir ? ((i+1 == repeat) ? 1.5f : 2.0f) : 1.0f) * (GLfloat)i / (GLfloat)(repeat - 1);
      DebugParticle->moveTo(dest.x, dest.y);
      DebugParticle->draw();
    }
  }

};

#endif
