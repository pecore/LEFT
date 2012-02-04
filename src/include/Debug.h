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
    static char msg[4096];
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

  static void * DebugMutex;
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

  static void drawVector(GLvector2f base, GLvector2f dir, GLvector2f pos, GLvector3f color = GLvector3f(1.0f, 1.0f, 1.0f), int size = 6) {
    if(!DebugActive) return;
    if((base.x < pos.x - GL_SCREEN_FWIDTH / 2.0f 
    ||  base.x > pos.x + GL_SCREEN_FWIDTH / 2.0f)
    && (base.y < pos.y - GL_SCREEN_FHEIGHT / 2.0f 
    ||  base.y > pos.y + GL_SCREEN_FHEIGHT / 2.0f)) return;

    Lock(DebugMutex);
    base -=  GL_SCREEN_BOTTOMLEFT;
    GLvector2f dest = base + dir;
    glLineWidth(2.0f);
    glBegin(GL_LINES);
    glColor3f(0.1f, 0.05f, 0.05f);
    glVertex2f(base.x, base.y);
    glVertex2f(dest.x, dest.y);
    glEnd();
    Unlock(DebugMutex);
  }

};

#endif
