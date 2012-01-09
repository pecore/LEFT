/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _GLWINDOW_H
#define _GLWINDOW_H

#include <windows.h>
#include <gl\glew.h>
#include <gl\gl.h>
#include <gl\glu.h>

class GLWindow {
public:
  GLWindow(const char * title, int width, int height, int bits, bool fullscreen, WNDPROC wndproc);
  ~GLWindow();

  HDC DC() { return mhDC; };
  HGLRC RC() { return mhRC; };
  HWND hWnd() { return mhWnd; };

  GLuint getLightShaderObject() { return mLightShaderObject; }
  GLint getLightSourcesLocation() { return mLightSourcesLocation; }
  GLint getLightMapLocation() { return mLightMapLocation; }

  bool initOpenGL();
  void swapBuffers() { SwapBuffers(mhDC); };
  bool isInitialized() { return mInitialized; };

private:
  void free();

private:
  HDC mhDC;
  HGLRC mhRC;	
  HWND mhWnd;
  HINSTANCE	mhInstance;
  bool mInitialized;

  GLuint mLightShaderObject;
  GLint mLightSourcesLocation;
  GLint mLightMapLocation;

  int mScreenX;
  int mScreenY;
};

#endif
