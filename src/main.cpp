/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#define LEFT_VERSION "0.57"

#include <windows.h>
#include <time.h>
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);

#include "GLWindow.h"
#include "GLResources.h"
#include "GLDefines.h"
#include "SoundPlayer.h"
#include "RobotModel.h"
#include "Map.h"
#include "Debug.h"

bool gActive;
GLResources * gResources = 0;
GLvector2f gScreen;
const unsigned int gFramerate = 60;

typedef struct {
  char * version;
  bool running;

  GLResources * resources;
  GLWindow * window;
  Map * map;

  GLParticle * cross;
  RobotModel * robot;
  LightSource * robotlight;
  
  GLParticle * balls[1024];
  LightSource * lightballs[1024];
  int ballcount;

  struct {
    bool keydown[256];
    // FIXME mutex
    GLvector2f mousepos;
    unsigned int mousebutton;
  } control;

  struct {
    GLFONT couriernew;
  } font;

  struct {
    GLfloat fps;
    unsigned long long timer;
    unsigned long long counter;
    unsigned long long framecounter;
    LARGE_INTEGER performancefrequency;
  } timing;
} left_handle;

GLParticle * Debug::DebugParticle = 0;
std::list<Debug::DebugVectorType> Debug::DebugVectors;
bool Debug::DebugActive = false;
void * Debug::DebugMutex;
GLfloat gDebugValue;

void updateMousePosition(left_handle * left)
{
  static int fx = GetSystemMetrics(SM_CXSIZEFRAME);
  static int fy = GetSystemMetrics(SM_CYSIZEFRAME);
  static int sc = GetSystemMetrics(SM_CYCAPTION);

  POINT cursorpos;
  if(GetCursorPos(&cursorpos)) {
    RECT clientrect, windowrect;
    if(GetClientRect(left->window->hWnd(), &clientrect)) {
      if(GetWindowRect(left->window->hWnd(), &windowrect)) {
        POINT delta;
        int x = windowrect.left + clientrect.left + fx; 
        int y = windowrect.top + clientrect.top + fy;
        left->control.mousepos = gScreen + GLvector2f(cursorpos.x - x, GL_SCREEN_FHEIGHT);
        left->control.mousepos.y -= (cursorpos.y - y);
      }
    }
  }
}

void renderScene(left_handle * left)	
{
  updateMousePosition(left);

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

  for(int i = 0; i < left->ballcount; i++) {
    left->balls[i]->move();
    left->lightballs[i]->pos = left->balls[i]->pos();
  }
  left->robot->control(left->control.keydown, left->control.mousepos, left->control.mousebutton); left->control.mousebutton = 0;
  left->robot->integrate(0.1f);
  left->robotlight->pos = left->robot->pos();
  left->cross->moveTo(left->control.mousepos.x, left->control.mousepos.y);

  left->map->drawShadows();
  left->map->collide();
  left->map->drawProjectiles();
  left->map->drawAnimations();
  left->robot->draw();
 
  for(int i = 0; i < left->ballcount; i++)
    left->balls[i]->draw();
  
  left->cross->draw();
  gScreen = left->robot->pos() - GLvector2f(GL_SCREEN_FWIDTH / 2.0f, GL_SCREEN_FHEIGHT / 2.0f);
  
  Debug::drawVectors(left->robot->pos());

  char s[512]; sprintf(s, "%.2f FPS Map: %.2fms             Debug: F1 Quit: ESC             [W]/[S]: Boost [A]/[D]: Turn [Space]: Stop", left->timing.fps, gDebugValue); int i = strlen(s);
  memset(&s[i], 0x20, 512 - i); s[511] = 0; 
  glFontBegin(&left->font.couriernew);
  glFontTextOut(s, 0.0f, 15.0f, 0);
  glFontEnd();

  unsigned long tickdelta = left->timing.timer - GetTickCount();
  left->timing.timer = GetTickCount();
  if(left->timing.timer / 1000 > left->timing.counter) {
    int delta = (left->timing.timer / 1000) - left->timing.counter;
    left->timing.counter = left->timing.timer / 1000;
    left->timing.fps = (GLfloat) left->timing.framecounter / delta;
    left->timing.framecounter = 0;
  }
  if(tickdelta < (1000 / gFramerate)) Sleep((1000 / gFramerate) - tickdelta);
  left->timing.timer += tickdelta;
  left->timing.framecounter++;
}

int onMouseDown(left_handle * left, unsigned int button, unsigned int x, unsigned int y)
{
  left->control.mousebutton = button;
  return 0;
}

LRESULT CALLBACK WndProc(	HWND	hWnd,	UINT	uMsg,	WPARAM	wParam,	LPARAM	lParam)
{
  left_handle * left = (left_handle *) GetWindowLongPtr(hWnd, GWLP_USERDATA);
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
    left->control.keydown[wParam] = true;
    switch(wParam) {
    case VK_ESCAPE:
      left->running = false;
      PostQuitMessage(0);
    case VK_F1:
      Debug::DebugActive = !Debug::DebugActive;
      return 0;
    }
    return 0;								
  case WM_KEYUP:
    left->control.keydown[wParam] = false;
    return 0;
  case WM_RBUTTONDOWN:
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_XBUTTONDOWN:
    return onMouseDown(left, wParam, lParam & 0xFFFF, GL_SCREEN_IHEIGHT - ((lParam >> 16) & 0xFFFF));
  case WM_SIZE:								
    return 0;								
  }

  return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

DWORD WINAPI run(void * lh)
{
  left_handle * left = (left_handle *) lh;
  srand( (unsigned int)GetTickCount() );
  wglMakeCurrent(left->window->DC(), left->window->RC());

  if(left->running) {
    left->resources = new GLResources();
    gResources = left->resources;
    left->font.couriernew = ((GLFontResource *)left->resources->get("data\\couriernew.glf"))->font;
  }

  if(left->running) {
    left->map = new Map();
    
    left->robot = new RobotModel(left->map);
    left->robot->moveTo(1100.0f, 1000.0f);
    left->map->addCollidable(left->robot);
    left->robotlight = new LightSource(left->robot->pos(), GLvector3f(0.0f, 0.0f, 0.0f), 1.0f);
    left->map->LightSources().push_back(left->robotlight);
    left->cross = new GLParticle(50, 50, 1.0f, 1.0f, 1.0f, 1.0f, glpCross);

    for(int i = 0; i < 4; i++) {
      unsigned int ballcount = left->ballcount;
      left->balls[ballcount] = new GLParticle(8, 8, frand(), frand(), frand(), 1.0f, glpSolid);
      left->balls[ballcount]->moveTo(1000.0f, 1000.0f);
      left->balls[ballcount]->setVelocity(GLvector2f((frand() * 8.0f) - 4.0f, (frand() * 8.0f) - 4.0f));
      left->lightballs[ballcount] = new LightSource(left->balls[ballcount]->pos(), left->balls[ballcount]->getColor() / 5.0f, 0.1f);
      left->map->LightSources().push_back(left->lightballs[ballcount]);
      left->map->addCollidable(left->balls[ballcount]);
      left->ballcount++;
    }
  }

	while(left->running) {
    if(gActive)	{
			renderScene(left);			
			left->window->swapBuffers();
		}
  }

  for(int i = 0; i < left->ballcount; i++) {
    delete left->balls[i];
  }
  Debug::clear();
  delete left->map;
  delete left->cross;
  delete left->robot;
  delete Debug::DebugParticle;
  delete left->resources;
  return 0;
}

int WINAPI WinMain(	HINSTANCE	hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  HANDLE renderThread;
	MSG msg;
  timeBeginPeriod(1);
  srand( (unsigned int)GetTickCount() );

  left_handle * left = new left_handle;
  left->ballcount = 0;
  left->control.mousebutton = 0;
  memset(&left->control, 0, 256);
  memset(&left->timing, 0, sizeof(left->timing));

  QueryPerformanceFrequency(&left->timing.performancefrequency);
  Debug::DebugMutex = CreateMutex(0, FALSE, "LeftDebugMutex");
  char s[64]; sprintf(s, "LEFT %s", LEFT_VERSION);
  left->window = new GLWindow(s, GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT, 32, false, (WNDPROC) WndProc);
  
  if(left->window) {
    left->running = left->window->isInitialized();
    if(left->running) {
      SetWindowLongPtr(left->window->hWnd(), GWLP_USERDATA, (LONG_PTR) left);
      wglMakeCurrent(0, 0);
      renderThread = CreateThread(0, 0, &run, left, 0, 0);
      if(renderThread == INVALID_HANDLE_VALUE) {
        delete left->window;
        return 0;
      }
    }
  }

	while(left->running) {
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) {
        left->running = false;
        break;
      }
      DispatchMessage(&msg);
    }
    Sleep(16);
	}
  WaitForSingleObject(renderThread, INFINITE);
  CloseHandle(Debug::DebugMutex);

  delete left->window;
  delete left;
  timeEndPeriod(1);
	return 0;	
}

