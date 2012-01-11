/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#define LEFT_VERSION "0.58"

#include "GLResources.h"
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#include "GLWindow.h"
#include "GLDefines.h"
#include "SoundPlayer.h"
#include "RobotModel.h"
#include "Map.h"
#include "Debug.h"

#include <fstream>

bool gActive;
GLResources * gResources = 0;
GLvector2f gScreen;
const unsigned int gFramerate = 60;

typedef struct {
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

  bool consoleactive;
  struct {
    HANDLE mutex;
    unsigned int recorder;
    unsigned int linecount;
    char linebuffer[16][129];
  } console;

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
  gScreen = left->robot->pos() - GLvector2f(GL_SCREEN_FWIDTH / 2.0f, GL_SCREEN_FHEIGHT / 2.0f);

  left->map->draw();
  left->map->drawShadows();
  left->map->collide();
  left->map->drawProjectiles();
  left->map->drawAnimations();
  left->robot->draw();
  for(int i = 0; i < left->ballcount; i++)
    left->balls[i]->draw();
  left->cross->draw();
  Debug::drawVectors(left->robot->pos());

  Lock(left->console.mutex);
  if(left->consoleactive) {
    char s[129];
    left->console.linecount = 16;

    for(int i = 0; i < left->console.linecount; i++) {
      if(left->console.linebuffer[i][0]) {
        int offset = sprintf(s, "%s", left->console.linebuffer[i]);
        memset(&s[offset], 0x20, 128 - offset); s[128] = 0;
      } else {
        if(i == 0) {
          sprintf(s, "%.2f FPS                                                                               %d Vertices %d Lights", left->timing.fps, left->map->collision().size(), left->map->LightSources().size());
        } else {
          memset(s, 0x20, 128); s[128] = 0;
        }
      }
      glFontBegin(&left->font.couriernew);
      glFontTextOut(s, 0.0f, 15.0f + (i * 15.0f), 0);
      glFontEnd();
    }    
  }
  Unlock(left->console.mutex);

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

void cprintf(left_handle * left, const char * fmt, ...) 
{
  Lock(left->console.mutex);
  char msg[129];
  va_list args;
  va_start(args, fmt);
  vsprintf(msg, fmt, args);
  for(int i = left->console.linecount - 2; i >= 0 ; i--) {
    strcpy((char *)&left->console.linebuffer[i+1], (i == 0) ? msg : (char *)&left->console.linebuffer[i]);
  }
  va_end(args);
  Unlock(left->console.mutex);
}

void parseConsoleCommand(left_handle * left, char * cmd)
{
  char tmp[129]; strcpy(tmp, cmd);
  char * op = strtok(tmp, " ");
  char * param[16]; 
  int pcount = 0;
  while(param[pcount] = strtok(0, " ")) pcount++;
  if(!op) return;

  if(strcmp(op, "help") == 0 || strcmp(op, "h") == 0 || strcmp(op, "?") == 0) {
    cprintf(left, "");
    cprintf(left, "ls                         : list resources");
    cprintf(left, "save <filename>            : save current map");
    cprintf(left, "load <filename>            : load map");
    cprintf(left, "help                       : show help");
    cprintf(left, "");
  }

  if(strcmp(op, "save") == 0) {
    if(pcount == 1) {
      std::ofstream f(param[0]);
      f << left->map->polygons();
      f.close();
    }
  }

  if(strcmp(op, "load") == 0) {
    if(pcount == 1) {
      std::ifstream f(param[0]);
      if(f.good()) {
        Polygons p;
        f >> p;
        if(p.size() > 0) {
          left->map->setPolygons(p);
        }
      }
      f.close();
    }
  }

  if(strcmp(op, "ls") == 0) {
    int i = 0;
    ResourceList resources = left->resources->list();
    ResourcePair * rp = 0;
    cprintf(left, "Resources: %d", resources.size());
    foreach(ResourceList, rp, resources) {
      GLResource * res = rp->value;
      switch(res->type) {
        case GL_RESOURCE_TEXTURE: {
          GLTextureResource * tres = (GLTextureResource *) res;
          cprintf(left, "%d: Texture %dx%d:%d [%s]", i, (int)tres->width, (int)tres->height, tres->texture, rp->path);
          break;
        }
        case GL_RESOURCE_SOUND: {
          GLSoundResource * sres = (GLSoundResource *) res;
          cprintf(left, "%d: Sound size:%dkB format:%04x rate:%d [%s]", i, (int)sres->sound->size/1000, (int)sres->sound->format, (int)sres->sound->freq, rp->path);
          break;
        }
        case GL_RESOURCE_FONT: {
          GLFontResource * fres = (GLFontResource *) res;
          cprintf(left, "%d: Font %dx%d:%d [%s]", i, fres->font.TexWidth, fres->font.TexHeight, fres->font.Tex, rp->path);
          break;
        }
        case GL_RESOURCE_POLYGON: {
          GLPolygonResource * pres = (GLPolygonResource *) res;
          int vertices = 0;
          Polygons::iterator pit = pres->polygons.begin();
          for(; pit != pres->polygons.end(); ++pit) {
            vertices += (*pit).size();
          }
          cprintf(left, "%d: Polygon(%d) %d Vertices [%s]", i, pres->polygons.size(), vertices, rp->path);
          break;
        }
      } i++;
    }
  }

  cprintf(left, left->console.linebuffer[0]);
  memset(left->console.linebuffer[0], 0, 129);
  left->console.recorder = 0;
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
  case WM_CHAR:
    switch (wParam) {
    case '\r':
      parseConsoleCommand(left, (char *) left->console.linebuffer[0]);
      break;
    case '\b':
      if(left->console.recorder <= 0)
        break;
      left->console.linebuffer[0][--left->console.recorder] = 0;
      break;
    case '\t':
    case '\n':
    case  27 :
      break;
    default:
      if(left->console.recorder < 128) {
        left->console.linebuffer[0][left->console.recorder++] = wParam;
        left->console.linebuffer[0][left->console.recorder+1] = 0;
      }
    }
    break;
  case WM_KEYDOWN:
    left->control.keydown[wParam] = true;
    switch(wParam) {
    case VK_ESCAPE:
      left->running = false;
      PostQuitMessage(0);
    case VK_F1:
      Debug::DebugActive = !Debug::DebugActive;
      return 0;
    case VK_F2:
    case VK_OEM_3: // '`' or '^'
      left->consoleactive = !left->consoleactive;
      left->console.recorder = 0;
      break;
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
    MapObject * house = new MapObject("house");
    left->map->MapObjects().push_back(house);
    house->moveTo(2000.0f, 2000.0f);
    left->map->updateCollision();

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
  left->console.mutex = CreateMutex(0, FALSE, "LeftConsoleMutex");
  left->consoleactive = false;
  left->ballcount = 0;
  left->control.mousebutton = 0;
  memset(&left->control, 0, 256);
  memset(&left->timing, 0, sizeof(left->timing));
  memset(&left->console, 0, sizeof(left->console));

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
      if(left->consoleactive) TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
    Sleep(16);
	}
  WaitForSingleObject(renderThread, INFINITE);
  CloseHandle(Debug::DebugMutex);
  CloseHandle(left->console.mutex);

  delete left->window;
  delete left;
  timeEndPeriod(1);
	return 0;	
}

