/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#include "LEFTsettings.h"

#include "GLResources.h"
#include "GLWindow.h"
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#include "GLSprite.h"
#include "GLParticle.h"
#include "GLFont.h"

#include <fstream>

bool running = false;
GLResources * gResources;
GLvector2f gScreen(0.0f, 0.0f);
GLvector2f gScreenSize(1280.0f, 720.0f);
Settings * gSettings = 0;
GLvector2f gMousePos;

#include "Debug.h"
GLParticle * Debug::DebugParticle = 0;
std::list<Debug::DebugVectorType> Debug::DebugVectors;
bool Debug::DebugActive = true;
void * Debug::DebugMutex;

bool showpolygon = false;
Polygon polygon;
typedef std::list<GLParticle *> ParticleList;
ParticleList particles;
GLSprite * bg = 0;
bm_font * font;
char gBGFilename[256];

#define POLYLEFT_MODE_ADD          0
#define POLYLEFT_MODE_MOVE         1
#define POLYLEFT_MODE_DELETE       2
#define POLYLEFT_MODES             3
unsigned int mode = 0;

struct {
  GLParticle * p;
  unsigned int index;
} selected = { 0, 0 };

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
        int x = windowrect.left + clientrect.left + fx; 
        int y = windowrect.top + clientrect.top + fy;
        gMousePos = GLvector2f((GLfloat)cursorpos.x - x, GL_SCREEN_FHEIGHT);
        gMousePos.y -= (cursorpos.y - y);
      }
    }
  }
}

inline GLvector2f scaleToScreen(GLvector2f point, GLfloat width, GLfloat height)
{
  GLfloat aspect = width / height;
  GLfloat raspect = 1.0f / aspect;
  if(width > height) {
    point.x = GL_SCREEN_FWIDTH;
    point.y = raspect * GL_SCREEN_FWIDTH;
  } else {
    point.y = GL_SCREEN_FHEIGHT;
    point.x = aspect * GL_SCREEN_FHEIGHT;
  }
  return point;
}

inline GLvector2f scaleFromScreen(GLvector2f point, GLfloat width, GLfloat height)
{
  GLvector2f screen = scaleToScreen(point, width, height);
  GLfloat aspect = width / height;
  GLfloat raspect = 1.0f / aspect;
  if(width > height) {
    point.x = width * point.x / GL_SCREEN_FWIDTH;
    point.y = height * (point.y + screen.y / 2.0f - GL_SCREEN_FHEIGHT / 2.0f) / (raspect * GL_SCREEN_FWIDTH);
  } else {
    point.y = height * point.y / GL_SCREEN_FHEIGHT;
    point.x = width * (point.x + screen.x / 2.0f - GL_SCREEN_FWIDTH / 2.0f) / (aspect * GL_SCREEN_FHEIGHT);
  }
  return point;
}

void render()
{
  glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();
  if(!bg) {
    bg = new GLSprite(gBGFilename);
  }

  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
  switch(mode) {
  case POLYLEFT_MODE_ADD:
    glFontPrint(font, GLvector2f(0.0f, 15.0f), "ADD");
    break;
  case POLYLEFT_MODE_MOVE:
    glFontPrint(font, GLvector2f(0.0f, 15.0f), "MOVE");
    break;
  case POLYLEFT_MODE_DELETE:
    glFontPrint(font, GLvector2f(0.0f, 15.0f), "DELETE");
    break;
  }
  glFontPrint(font, GLvector2f(300.0f, 15.0f), "F1: toggle modes     F2: save poly     F3: load background     F4: show poly");
  GLvector2f scaledpos = scaleFromScreen(gMousePos, bg->w(), bg->h());
  char s[128]; sprintf(s, "screen(%-4.2f, %-4.2f) tex(%-4.2f, %-4.2f)", gMousePos.x, gMousePos.y, scaledpos.x, scaledpos.y);
  glFontPrint(font, GLvector2f(0.0f, GL_SCREEN_FHEIGHT), s);
  
  GLvector2f size(bg->w(), bg->h());
  GLvector2f scaled = scaleToScreen(size, bg->w(), bg->h());
  bg->setSize(scaled.x, scaled.y);
  bg->moveTo(GL_SCREEN_FWIDTH / 2.0f, GL_SCREEN_FHEIGHT / 2.0f);
  bg->draw();
  bg->setSize(size.x, size.y);

  Debug::clear();
  GLParticle * p = 0;
  GLParticle * last = 0, * first = 0;
  foreach(ParticleList, p, particles) {
    if(showpolygon && !first) first = p;
    if(showpolygon && last) {
      Debug::add(new GLplane(last->pos(), p->pos() - last->pos()), GLvector3f(0.6f, 0.0f, 0.0f));
    }
    p->draw();
    last = p;
  }
  if(first && last) {
    Debug::add(new GLplane(last->pos(), first->pos() - last->pos()), GLvector3f(0.6f, 0.0f, 0.0f));
  }
  Debug::drawVectors(GL_SCREEN_CENTER);
}

inline void setpoly(unsigned int index, GLvector2f & pos) 
{

}

int findpoly(GLvector2f & pos, GLParticle * & particle)
{
  GLParticle * p = particle = 0;
  foreach(ParticleList, p, particles) {
    if(pos.x > p->pos().x - (p->w() / 2.0f) &&
       pos.x < p->pos().x + (p->w() / 2.0f) &&
       pos.y > p->pos().y - (p->h() / 2.0f) &&
       pos.y < p->pos().y + (p->h() / 2.0f) ) {
      particle = p;
      break;
    }
  }
  if(particle) {
    for(Polygon::size_type i = 0; i < polygon.size(); i++) {
      GLvector2f scaled = scaleFromScreen(p->pos(), bg->w(), bg->h()) * CLIPPER_PRECISION;
      if(polygon[i].X > (scaled.x - ((p->w() / 2.0f)) * CLIPPER_PRECISION) &&
         polygon[i].X < (scaled.x + ((p->w() / 2.0f)) * CLIPPER_PRECISION) &&
         polygon[i].Y > (scaled.y - ((p->h() / 2.0f)) * CLIPPER_PRECISION) &&
         polygon[i].Y < (scaled.y + ((p->h() / 2.0f)) * CLIPPER_PRECISION) ) {
        return i;
      }
    }
  }
  return -1;
}

LRESULT CALLBACK WndProc(	HWND	hWnd,	UINT	uMsg,	WPARAM	wParam,	LPARAM	lParam)
{
  GLvector2f pos;
  switch(uMsg) {
  case WM_ACTIVATE:
    // active
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
    break;
  case WM_KEYDOWN:
    switch(wParam) {
      case VK_ESCAPE:
        running = false;
        PostQuitMessage(0);
        break;
      case VK_F1:
        if(++mode >= POLYLEFT_MODES) mode = 0;
        break;
      case VK_F2: {
        OPENFILENAME ofn;
        char szFile[260];
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = ".\\";
        ofn.Flags = OFN_PATHMUSTEXIST;
        if(GetOpenFileName(&ofn)) {
          std::ofstream f(ofn.lpstrFile);
          f << polygon;
          f.close();
        } break;
      }
      case VK_F3: {
        OPENFILENAME ofn;
        char szFile[260];
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = ".\\";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if(GetOpenFileName(&ofn)) {
          strcpy(gBGFilename, ofn.lpstrFile);
          delete bg; bg = 0;
        } break;
      }
      case VK_F4:
        showpolygon = !showpolygon;
        break;
      case VK_F5: {
        OPENFILENAME ofn;
        char szFile[260];
        ZeroMemory(&ofn, sizeof(ofn));
        ofn.lStructSize = sizeof(ofn);
        ofn.hwndOwner = hWnd;
        ofn.lpstrFile = szFile;
        ofn.lpstrFile[0] = '\0';
        ofn.nMaxFile = sizeof(szFile);
        ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
        ofn.nFilterIndex = 1;
        ofn.lpstrFileTitle = NULL;
        ofn.nMaxFileTitle = 0;
        ofn.lpstrInitialDir = ".\\";
        ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
        if(GetOpenFileName(&ofn)) {
          std::ifstream f(ofn.lpstrFile);
          Polygons p;
          f >> p;
          polygon = p[0];
          f.close();
        } break;
      }
    }
    return 0;								
  case WM_KEYUP:
    return 0;
  case WM_RBUTTONDOWN: {
    OPENFILENAME ofn;
    char szFile[260];
    ZeroMemory(&ofn, sizeof(ofn));
    ofn.lStructSize = sizeof(ofn);
    ofn.hwndOwner = hWnd;
    ofn.lpstrFile = szFile;
    ofn.lpstrFile[0] = '\0';
    ofn.nMaxFile = sizeof(szFile);
    ofn.lpstrFilter = "All\0*.*\0Text\0*.TXT\0";
    ofn.nFilterIndex = 1;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = ".\\";
    ofn.Flags = OFN_PATHMUSTEXIST;
    if(GetOpenFileName(&ofn)) {
      std::ofstream f(ofn.lpstrFile);
      f << polygon;
      f.close();
    }
  } break;
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_XBUTTONDOWN:
    pos = GLvector2f((GLfloat)(lParam & 0xFFFF), GL_SCREEN_IHEIGHT - ((GLfloat)((lParam >> 16) & 0xFFFF)));
    switch(mode) {
    case POLYLEFT_MODE_DELETE: {
        GLParticle * p = 0;
        int test = findpoly(pos, p);
        if(test >= 0) {
          polygon.erase(polygon.begin() + test);
          particles.remove(p);
        }
      } break;
    case POLYLEFT_MODE_ADD: {
        GLParticle * p = new GLParticle(10, 10, 1.0f, 1.0f, 1.0f, 1.0f, glpSolid);
        p->moveTo(pos.x, pos.y);
        particles.push_back(p);
        pos = scaleFromScreen(pos, bg->w(), bg->h());
        polygon.push_back(IntPoint((long64)pos.x * CLIPPER_PRECISION, (long64)pos.y * CLIPPER_PRECISION));
      } break;
    case POLYLEFT_MODE_MOVE: {
        GLParticle * p = 0;
        int test = findpoly(pos, p);
        if(test >= 0) {
          if(p) {
            if(selected.p) {
              selected.p->setColor4f(1.0f, 1.0f, 1.0f, 1.0f);
            }
            selected.index = test;
            selected.p = p;
            selected.p->setColor4f(0.3f, 0.6f, 1.0f, 1.0f);
          }
        } else {
          if(selected.index >= 0 && selected.p) {
            setpoly(selected.index, selected.p->pos());
            selected.p->moveTo(pos.x, pos.y);
          }
        }
      } break;
    } break;
  case WM_SIZE:								
    return 0;								
  }

  return DefWindowProc(hWnd,uMsg,wParam,lParam);
}

int WINAPI WinMain(	HINSTANCE	hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  GLWindow * window = new GLWindow("POLYLEFT Polygon Editor", GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT, 32, false, (WNDPROC) WndProc);
  
  if(window) {
    running = window->isInitialized();
    ShowCursor(TRUE);
  }

  if(running) {
    gResources = new GLResources();
    font = gResources->getFont("data\\couriernew.fnt")->font;
    strcpy(gBGFilename, "data\\house.png");
  }

	while(running) {
    MSG msg;
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) {
        running = false;
        break;
      }
      DispatchMessage(&msg);
    }
    updateMousePosition(window);
    render();
    window->swapBuffers();
    Sleep(16);
	}

  delete bg;
  GLParticle * p = 0;
  foreach(ParticleList, p, particles) {
    delete p;
  }
	return 0;	
}