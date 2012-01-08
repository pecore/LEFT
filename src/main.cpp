/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#define LEFT_VERSION "0.55"

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
HANDLE gFrameSyncMutex;

GLWindow * gWindow = 0;
unsigned long long gTimer = 0;
unsigned long long gTimerCounter = 0;
GLfloat gFPS = 0.0f;
GLfloat gDebugValue;

Map        * gMap = 0;
RobotModel * gRobot = 0;
GLParticle * gCross = 0;
LightSource * gRobotLight = 0;
unsigned int gProjectileCount = 0;

GLParticle * gBalls[1024];
int gBallCount = 0;
LightSource * gLightBalls[1024];

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
  static int fx = GetSystemMetrics(SM_CXSIZEFRAME);
  static int fy = GetSystemMetrics(SM_CYSIZEFRAME);
  static int sc = GetSystemMetrics(SM_CYCAPTION);

  POINT cursorpos;
  if(GetCursorPos(&cursorpos)) {
    RECT clientrect, windowrect;
    if(GetClientRect(window->hWnd(), &clientrect)) {
      if(GetWindowRect(window->hWnd(), &windowrect)) {
        POINT delta;
        int x = windowrect.left + clientrect.left + fx; 
        int y = windowrect.top + clientrect.top + fy;
        gMousePos.x = gWindow->x() + cursorpos.x - x;
        gMousePos.y = (gWindow->y() + GL_SCREEN_FHEIGHT) - (cursorpos.y - y);
      }
    }
  }
}

void renderScene()	
{
  Lock(gFrameSyncMutex);
  updateMousePosition(gWindow);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
  
  if(gProjectileCount < gMap->Projectiles().size()) {
    Projectile * proj = 0;
    foreach(ProjectileList, proj, gMap->Projectiles()) {
      proj->init();
    }
    gProjectileCount = gMap->Projectiles().size();
  }

  for(int i = 0; i < gBallCount; i++) {
    gBalls[i]->move();
    gLightBalls[i]->pos = gBalls[i]->pos();
  }
  gRobot->control(gKeydown, gMousePos);
  gRobot->integrate(0.1f);
  gCross->moveTo(gMousePos.x, gMousePos.y);

  
  gMap->drawShadows(GL_SCREEN_BOTTOMLEFT);
  gMap->collide();
  gMap->drawProjectiles();
  gRobot->draw();
 
  for(int i = 0; i < gBallCount; i++)
    gBalls[i]->draw();

  gRobotLight->pos = gRobot->pos();
  gCross->draw();

  gWindow->updateCenter(gRobot->pos().x, gRobot->pos().y);
  int x = gWindow->x();
  int y = gWindow->y();
  glViewport(-x, -y, GL_SCREEN_IWIDTH * GL_SCREEN_FACTOR, GL_SCREEN_IHEIGHT * GL_SCREEN_FACTOR);
  Debug::drawVectors(gRobot->pos());

  char s[512]; sprintf(s, "%.2f FPS Map: %.2fms             Debug: F1 Quit: ESC             [W]/[S]: Boost [A]/[D]: Turn [Space]: Stop", gFPS, gDebugValue); int i = strlen(s);
  memset(&s[i], 0x20, 512 - i); s[511] = 0; 
  glFontBegin(&gFont);
  glFontTextOut(s, x, y + 15.0f, 0);
  glFontEnd();
  Unlock(gFrameSyncMutex);

  DWORD tickdelta = gTimer - GetTickCount();
  gTimer = GetTickCount();
  if(gTimer / 1000 > gTimerCounter) {
    int delta = (gTimer / 1000) - gTimerCounter;
    gTimerCounter = gTimer / 1000;
    gFPS = (GLfloat) gFrameCounter / delta;
    gFrameCounter = 0;
  }

  if(tickdelta < (1000 / gFramerate)) Sleep((1000 / gFramerate) - tickdelta);
  gTimer += tickdelta;
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
  GLvector2f crossHair = GL_SCREEN_BOTTOMLEFT + GLvector2f(x, y);
  GLvector2f pos = gRobot->pos();
  gMap->addProjectile(new RocketProjectile(pos, (crossHair - pos).normal() * 5.0f, gMap));
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
  case WM_RBUTTONDOWN:
    Lock(gFrameSyncMutex);
    gMap->LightSources().push_back(new LightSource(gRobot->pos(), GLvector3f(0.1f, 0.1f, 0.1f), 1.0f));
    Unlock(gFrameSyncMutex);
    break;
  case WM_LBUTTONDOWN:
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
    gRobotLight = new LightSource(gRobot->pos(), GLvector3f(0.0f, 0.0f, 0.0f), 1.0f);
    gMap->LightSources().push_back(gRobotLight);
  }

  if(gRunning) {
    gCross = new GLParticle(50, 50, 1.0f, 1.0f, 1.0f, 1.0f, glpCross);
  }

  if(gRunning) {
    for(int i = 0; i < 4; i++) {
      gBalls[gBallCount] = new GLParticle(8, 8, frand(), frand(), frand(), 1.0f, glpSolid);
      gBalls[gBallCount]->moveTo(1000.0f, 1000.0f);
      gBalls[gBallCount]->setVelocity(GLvector2f((frand() * 8.0f) - 4.0f, (frand() * 8.0f) - 4.0f));
      gLightBalls[gBallCount] = new LightSource(gBalls[gBallCount]->pos(), gBalls[gBallCount]->getColor() / 5.0f, 0.1f);
      gMap->LightSources().push_back(gLightBalls[gBallCount]);
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
  gFrameSyncMutex = CreateMutex(NULL, FALSE, "LeftFrameSyncMutex");

  char s[64]; sprintf(s, "LEFT %s", LEFT_VERSION);
  gWindow = new GLWindow(s, GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT, 32, false, (WNDPROC) WndProc);
  
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
  CloseHandle(gFrameSyncMutex);
  delete gWindow;

  timeEndPeriod(1);
	return 0;	
}

