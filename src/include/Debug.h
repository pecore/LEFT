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
    MessageBox(0, msg, "Debug", 0);
    va_end(args);
#endif
  }

  static void LogToFile(const char * filename, const char * fmt, ...) {
#ifdef _DEBUG
    va_list args;
    va_start(args, fmt);
    FILE * fp = 0;
    fopen_s(&fp, filename, "w");
    if(fp) {
      vfprintf(fp, fmt, args);
      fclose(fp);
    }
    va_end(args);
#endif
  }

  typedef struct {
    GLplane * p;
    GLvector3f color;
    int size;
  } DebugVectorType;

  static HANDLE DebugMutex;
  static bool DebugActive;
  static std::list<DebugVectorType> DebugVectors;
  static void add(GLplane * p, GLvector3f color = GLvector3f(1.0f, 1.0f, 1.0f), int size = 10) {
    if(!DebugActive) return;
    Lock(DebugMutex);
    DebugVectorType d;
    d.p = p;
    d.color = color;
    d.size = size;
    DebugVectors.push_back(d);
    Unlock(DebugMutex);
  }
  static void clear() {
    Lock(DebugMutex);
    std::list<DebugVectorType>::iterator iter;
    for(iter = DebugVectors.begin(); iter != DebugVectors.end(); iter++) {
      if((*iter).p) delete (*iter).p;
    }
    DebugVectors.clear();
    Unlock(DebugMutex);
  }
  static void drawVectors(GLvector2f pos) {
    if(!DebugActive) return;
    Lock(DebugMutex);
    std::list<DebugVectorType>::iterator iter;
    for(iter = DebugVectors.begin(); iter != DebugVectors.end(); iter++) {
      drawVector((*iter).p->base, (*iter).p->dir, pos, (*iter).color, (*iter).size);
    }
    Unlock(DebugMutex);
  }

  static GLParticle * DebugParticle;
  static void drawVector(GLvector2f base, GLvector2f dir, GLvector2f pos, GLvector3f color = GLvector3f(1.0f, 1.0f, 1.0f), int size = 6) {
    if(!DebugActive) return;
    if(pos.x != 0.0f && pos.y != 0.0f) 
    if((base.x < pos.x - GL_SCREEN_FWIDTH / 2.0f 
    ||  base.x > pos.x + GL_SCREEN_FWIDTH / 2.0f)
    && (base.y < pos.y - GL_SCREEN_FHEIGHT / 2.0f 
    ||  base.y > pos.y + GL_SCREEN_FHEIGHT / 2.0f)) return;
    
    if(DebugParticle == 0 || DebugParticle->w() != size) {
      delete DebugParticle;
      DebugParticle = new GLParticle(size, size, 1.0f, 1.0f, 1.0f, glpSolid);
    }

    int repeat = 5;
    bool showdir = false;

    Lock(DebugMutex);
    for(int i = 0; i < repeat; i++) {
      GLvector2f dest = base + dir / (showdir ? ((i+1 == repeat) ? 1.5f : 2.0f) : 1.0f) * (GLfloat)i / (GLfloat)(repeat - 1);
      DebugParticle->moveTo(dest.x, dest.y);
      DebugParticle->setColor(color, 1.0f);
      DebugParticle->draw();
    }
    Unlock(DebugMutex);
  }

};

#endif
