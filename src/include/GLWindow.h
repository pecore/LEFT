/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _GLWINDOW_H
#define _GLWINDOW_H

#include "GLResources.h"
#ifndef _WIN32
typedef void* WNDPROC;
#endif

class GLWindow {
public:
  GLWindow(const char * title, int width, int height, int bits, bool fullscreen, WNDPROC wndproc);
  ~GLWindow();

  void makeCurrent()
  {
#ifdef _WIN32  
    wglMakeCurrent(mhDC, mhRC);
#else
    glXMakeCurrent(mDisplay, mWindow, mCtx);
#endif
  }

  bool initOpenGL();
  bool isInitialized() { return mInitialized; };
  
#ifdef _WIN32
  HDC DC() { return mhDC; };
  HGLRC RC() { return mhRC; };
  HWND hWnd() { return mhWnd; };

  void swapBuffers() { SwapBuffers(mhDC); };
  
private:
  HDC mhDC;
  HGLRC mhRC;	
  HWND mhWnd;
  HINSTANCE	mhInstance;
#else

  void swapBuffers() { glFlush(); if(mDoubleBuffered) glXSwapBuffers(mDisplay, mWindow); };
  Display * display() { return mDisplay; };
  Window window() { return mWindow; };
  
private:
  Display * mDisplay;
  int mScreen;  
  Window mWindow;  
  GLXContext mCtx;

  XSetWindowAttributes mWinAttr;
  Bool mDoubleBuffered;
  XF86VidModeModeInfo mDeskMode;
  
  unsigned int mWidth, mHeight;  
  unsigned int mDepth; 
#endif
  bool mFullscreen;
  bool mInitialized;

  int mScreenX;
  int mScreenY;
  
private:
  void free();
};

#endif
