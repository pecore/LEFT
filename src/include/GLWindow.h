/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _GLWINDOW_H
#define _GLWINDOW_H

#include <windows.h>
#include <gl\gl.h>
#include <gl\glu.h>

#define GL_SCREEN_IWIDTH 1280
#define GL_SCREEN_FWIDTH 1280.0f
#define GL_SCREEN_IHEIGHT 720
#define GL_SCREEN_FHEIGHT 720.0f
#define GL_SCREEN_FACTOR 5

class GLWindow {
public:
  GLWindow(const char * title, int width, int height, int bits, bool fullscreen, WNDPROC wndproc);
  ~GLWindow();

  HDC DC() { return mhDC; };
  HGLRC RC() { return mhRC; };
  HWND hWnd() { return mhWnd; };

  bool initOpenGL();
  void swapBuffers() { SwapBuffers(mhDC); };
  bool isInitialized() { return mInitialized; };

  int x() { return mScreenX; }
  int y() { return mScreenY; }
  void updateCenter(int x, int y) {
    mScreenX = (x - GL_SCREEN_IWIDTH / 2);
    mScreenY = (y - GL_SCREEN_IHEIGHT / 2);
  }


private:
  void free();

private:
  HDC mhDC;
  HGLRC mhRC;	
  HWND mhWnd;
  HINSTANCE	mhInstance;
  bool mInitialized;

  int mScreenX;
  int mScreenY;
};

#endif
