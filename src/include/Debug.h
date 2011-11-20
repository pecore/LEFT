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
  static void drawVectors(GLvector2f pos) {
    if(!DebugActive) return;
    GLplane * p = 0;
    foreach(GLplaneList, p, DebugVectors) {
      drawVector(p->base, p->dir, pos);
    }
  }

  static GLParticle * DebugParticle;
  static void drawVector(GLvector2f base, GLvector2f dir, GLvector2f pos) {
    if(!DebugActive) return;
    if(pos.x != 0.0f && pos.y != 0.0f) 
    if((base.x < pos.x - GL_SCREEN_FWIDTH / 2.0f 
    ||  base.x > pos.x + GL_SCREEN_FWIDTH / 2.0f)
    && (base.y < pos.y - GL_SCREEN_FHEIGHT / 2.0f 
    ||  base.y > pos.y + GL_SCREEN_FHEIGHT / 2.0f)) return;
    if(DebugParticle == 0) DebugParticle = new GLParticle(2, 2, 1.0f, 1.0f, 1.0f, PARTICLE_FORM_SOLID);
    
    int repeat = 32;
    bool showdir = false;

    for(int i = 0; i < repeat; i++) {
      GLvector2f dest = base + dir / (showdir ? ((i+1 == repeat) ? 1.5f : 2.0f) : 1.0f) * (GLfloat)i / (GLfloat)(repeat - 1);
      DebugParticle->moveTo(dest.x, dest.y);
      DebugParticle->draw();
    }
  }

};

#endif
