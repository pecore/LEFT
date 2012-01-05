/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#define LEFT_VERSION "0.54"

#include <windows.h>
#include <time.h>
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#include "GLWindow.h"
#include "GLDefines.h"
#include "Map.h"
#include "RobotModel.h"

extern "C" {
#include "glfont.h"
}
#include "Debug.h"

bool gActive;
bool gRunning = false;

GLWindow * gWindow = 0;
unsigned long long gTimer = 0;
unsigned long long gTimerCounter = 0;
GLfloat gFPS = 0.0f;
GLfloat gDebugValue;

Map        * gMap = 0;
RobotModel * gRobot = 0;
GLParticle * gCross = 0;

GLParticle * gBalls[1024];
int gBallCount = 0;

bool gKeydown[256];
GLvector2f gMousePos;

LARGE_INTEGER gPerformanceFrequency;
const unsigned int gFramerate = 60;
unsigned __int64 gFrameCounter = 0;
GLFONT gFont;
GLuint gFontTex;

int gDebugIndex;
GLParticle * Debug::DebugParticle = 0;
std::list<Debug::DebugVectorType> Debug::DebugVectors;
bool Debug::DebugActive = false;
HANDLE Debug::DebugMutex;

void updateMousePosition(GLWindow * window)
{
  POINT cursorpos;
  if(GetCursorPos(&cursorpos)) {
    RECT clientrect, windowrect;
    if(GetClientRect(window->hWnd(), &clientrect)) {
      if(GetWindowRect(window->hWnd(), &windowrect)) {
        POINT delta;
        delta.x = (windowrect.right - windowrect.left) - clientrect.right;
        delta.y = (windowrect.bottom - windowrect.top) - clientrect.bottom;
        gMousePos.x = gWindow->x() + (GLfloat)cursorpos.x - (clientrect.left + delta.x);
        gMousePos.y = gWindow->y() + GL_SCREEN_FHEIGHT - (cursorpos.y - (clientrect.top + delta.y));
      }
    }
  }
}

void renderScene()	
{
  updateMousePosition(gWindow);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
  
  for(int i = 0; i < gBallCount; i++)
    gBalls[i]->move();
  gRobot->control(gKeydown, gMousePos);
  gRobot->integrate(0.1f);
  gCross->moveTo(gMousePos.x, gMousePos.y);

  gMap->draw();
  gMap->collide();
  gRobot->draw();

  for(int i = 0; i < gBallCount; i++)
    gBalls[i]->draw();

  DWORD tickdelta = gTimer - GetTickCount();
  gTimer = GetTickCount();
  if(gTimer / 1000 > gTimerCounter) {
    int delta = (gTimer / 1000) - gTimerCounter;
    gTimerCounter = gTimer / 1000;
    gFPS = (GLfloat) gFrameCounter / delta;
    gFrameCounter = 0;
  }
 
  GLvector2f pos = gRobot->pos();
  GLfloat radius = 1280.0f;

  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glEnable(GL_BLEND);
  GLplane * p;
  glBegin(GL_QUADS);
  GLplaneList collision = gMap->collision();
  foreach(GLplaneList, p, collision) {
    if((p->base.x < pos.x - GL_SCREEN_FWIDTH / 2.0f 
    ||  p->base.x > pos.x + GL_SCREEN_FWIDTH / 2.0f)
    || (p->base.y < pos.y - GL_SCREEN_FHEIGHT / 2.0f 
    ||  p->base.y > pos.y + GL_SCREEN_FHEIGHT / 2.0f)) continue;
    if((p->dest.x < pos.x - GL_SCREEN_FWIDTH / 2.0f 
    ||  p->dest.x > pos.x + GL_SCREEN_FWIDTH / 2.0f)
    || (p->dest.y < pos.y - GL_SCREEN_FHEIGHT / 2.0f 
    ||  p->dest.y > pos.y + GL_SCREEN_FHEIGHT / 2.0f)) continue;

    // project quad for each plane
    GLvector2f base = p->base;
    GLvector2f dest = p->dest;
    GLvector2f baseproj = p->base - pos;
    GLvector2f destproj = p->dest - pos;
    GLvector2f bproj = p->base + baseproj.normal() * (radius - baseproj.len());
    GLvector2f dproj = p->dest + destproj.normal() * (radius - destproj.len());  

    if(base.y < gWindow->y() + 15.0f) base.y = gWindow->y() + 15.0f;
    if(bproj.y < gWindow->y() + 15.0f) bproj.y = gWindow->y() + 15.0f;
    if(dest.y < gWindow->y() + 15.0f) dest.y = gWindow->y() + 15.0f;
    if(dproj.y < gWindow->y() + 15.0f) dproj.y = gWindow->y() + 15.0f;

    glColor4f(0.0f, 0.0f, 0.0f, 0.99f);
    glVertex3f(base.x, base.y,  0.0f);
    glVertex3f(bproj.x, bproj.y,  0.0f);
    glVertex3f(dproj.x, dproj.y,  0.0f);
    glVertex3f(dest.x, dest.y,  0.0f);
  }
  glEnd();
  glDisable(GL_BLEND);
  gCross->draw();

  gWindow->updateCenter(gRobot->pos().x, gRobot->pos().y);
  int x = gWindow->x();
  int y = gWindow->y();
  glViewport(-x, -y, GL_SCREEN_IWIDTH * GL_SCREEN_FACTOR, GL_SCREEN_IHEIGHT * GL_SCREEN_FACTOR);
  glUseProgram(gWindow->getLightShaderObject());

  char s[512]; sprintf(s, "%.2f FPS Map: %.2fms             Debug: F1 Quit: ESC             [W]/[S]: Boost [A]/[D]: Turn [Space]: Stop", gFPS, gDebugValue); int i = strlen(s);
  memset(&s[i], 0x20, 512 - i); s[511] = 0; 
  glFontBegin(&gFont);
  glFontTextOut(s, x, y + 15.0f, 0);
  glFontEnd();

  if(tickdelta < (1000 / gFramerate)) Sleep((1000 / gFramerate) - tickdelta);
  gTimer += tickdelta;
  glFlush();
  gFrameCounter++;
}

void explodeMap(GLplane * p, GLvector2f center)
{
  LARGE_INTEGER perfstart, perfend;
  QueryPerformanceCounter(&perfstart);
  gMap->addCirclePolygon(center, 150.0f);
  gMap->updateCollision();
  QueryPerformanceCounter(&perfend);
  gDebugValue = ((GLfloat)(perfend.QuadPart - perfstart.QuadPart) * 1000.0f) / (GLfloat)gPerformanceFrequency.QuadPart;
}

int onMouseDown(unsigned int button, unsigned int x, unsigned int y)
{
  GLvector2f crossHair = GLvector2f((GLfloat) x - GL_SCREEN_FWIDTH / 2.0f, (GLfloat) y - GL_SCREEN_FHEIGHT / 2.0f);
  GLvector2f pos = gRobot->pos();

  GLplane * p;
  GLplaneList collision = gMap->collision();
  foreach(GLplaneList, p, collision) {
    GLfloat ca, cb;
    GLvector2f screen = GLvector2f(gRobot->pos().x - GL_SCREEN_IWIDTH / 2, gRobot->pos().y - GL_SCREEN_IHEIGHT / 2);
    if(gRobot->pos().x < GL_SCREEN_IWIDTH / 2) screen.x = 0.0f;
    if(gRobot->pos().y < GL_SCREEN_IHEIGHT / 2) screen.y = 0.0f;

    if(p->base.x > screen.x && p->base.x < screen.x + GL_SCREEN_IWIDTH && p->base.y > screen.y && p->base.y < screen.y + GL_SCREEN_IHEIGHT)
    {
      GLfloat distance;
      GLfloat planedistance;
      GLvector2f::crossing(pos, crossHair, p->base, p->dir.normal() * -1.0f, distance, planedistance);

      if(planedistance > 0.0f && planedistance < p->dir.len()) {
        if(distance > 0.0f) {
          GLvector2f target = pos + (crossHair * distance);
          if(target.x > GL_MAP_THRESHOLD && target.x < (GL_SCREEN_IWIDTH * GL_SCREEN_FACTOR) - GL_MAP_THRESHOLD && target.y > GL_MAP_THRESHOLD && target.y < (GL_SCREEN_IHEIGHT * GL_SCREEN_FACTOR) - GL_MAP_THRESHOLD) {
            explodeMap(p, target);
          }
          break;
        }
      }
    }
  }

  return 0;
}

LRESULT CALLBACK WndProc(	HWND	hWnd,	UINT	uMsg,	WPARAM	wParam,	LPARAM	lParam)			// Additional Message Information
{
  switch(uMsg) {
  case WM_ACTIVATE:
    gActive = !HIWORD(wParam);					
    return 0;
  case WM_SYSCOMMAND:				
    switch(wParam) {
    case SC_SCREENSAVE:		
    case SC_MONITORPOWER:	
      return 0;						
    }
    break;
  case WM_CLOSE:					
    PostQuitMessage(0);		
    return 0;							
  case WM_KEYDOWN:
    gKeydown[wParam] = true;
    switch(wParam) {
    case VK_ESCAPE:
      gRunning = false;
      PostQuitMessage(0);
#if 0
    case VK_ADD:
      gDebugIndex++;
      showPolygon(gDebugIndex);
      break;
    case VK_SUBTRACT:
      if(gDebugIndex > 0)
        gDebugIndex--;
      showPolygon(gDebugIndex);
      break;
#endif
    case VK_F1:
      Debug::DebugActive = !Debug::DebugActive;
      return 0;
    }
    return 0;								
  case WM_KEYUP:
    gKeydown[wParam] = false;
    return 0;
  case WM_LBUTTONDOWN:
  case WM_RBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_XBUTTONDOWN:
    return onMouseDown(wParam, lParam & 0xFFFF, GL_SCREEN_IHEIGHT - ((lParam >> 16) & 0xFFFF));
  case WM_SIZE:								
    return 0;								
  }

  return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

DWORD WINAPI run(void *)
{
  srand( (unsigned int)GetTickCount() );
  wglMakeCurrent(gWindow->DC(), gWindow->RC());

  if(gRunning) {
    gMap = new Map();
  }

  glGenTextures(1, &gFontTex);
  gRunning = glFontCreate(&gFont, "data\\couriernew.glf", gFontTex);

  if(gRunning) {
    gRobot = new RobotModel();
    gRobot->moveTo(1100.0f, 1000.0f);
    gMap->addCollidable(gRobot);
  }

  if(gRunning) {
    gCross = new GLParticle(50, 50, 1.0f, 1.0f, 1.0f, 1.0f, glpCross);
  }

  if(gRunning) {
    for(int i = 0; i < 16; i++) {
      gBalls[gBallCount] = new GLParticle(8, 8, frand(), frand(), frand(), 1.0f, glpSolid);
      gBalls[gBallCount]->moveTo(1000.0f, 1000.0f);
      gBalls[gBallCount]->setVelocity(GLvector2f((frand() * 8.0f) - 4.0f, (frand() * 8.0f) - 4.0f));
      gMap->addCollidable(gBalls[gBallCount]);
      gBallCount++;
    }
  }

	while(gRunning) {
    if(gActive)	{
			renderScene();			
			gWindow->swapBuffers();	
		}
  }

  for(int i = 0; i < gBallCount; i++) {
    delete gBalls[i];
  }

  delete gMap;

  glFontDestroy(&gFont);
  delete Debug::DebugParticle;
  Debug::clear();

  delete gCross;
  delete gRobot;
  return 0;
}

int WINAPI WinMain(	HINSTANCE	hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  HANDLE renderThread;
	MSG msg;
  timeBeginPeriod(1);
  srand( (unsigned int)GetTickCount() );
  QueryPerformanceFrequency(&gPerformanceFrequency);
  Debug::DebugMutex = CreateMutex(NULL, FALSE, "LeftDebugMutex");

  char s[64]; sprintf(s, "LEFT %s", LEFT_VERSION);
  gWindow = new GLWindow(s, GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT, 16, false, (WNDPROC) WndProc);
  
  if(gWindow) {
    gRunning = gWindow->isInitialized();
    if(gRunning) {
      wglMakeCurrent(0, 0);
      renderThread = CreateThread(NULL, 0, &run, 0, 0, 0);
      if(renderThread == INVALID_HANDLE_VALUE) {
        delete gWindow;
        return 0;
      }
    }
  }

	while(gRunning) {
    while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) {
        gRunning = false;
        break;
      }
      DispatchMessage(&msg);
    }

    Sleep(16);
	}

  WaitForSingleObject(renderThread, INFINITE);
  
  CloseHandle(Debug::DebugMutex);
  delete gWindow;

  timeEndPeriod(1);
	return 0;	
}

