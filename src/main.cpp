/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#define LEFT_VERSION "0.69"

#include "pthread_winapi.h"
#include "LEFTnet.h"
boost::asio::io_service io_service;
tcp_server * server;
message_pool * left_net_message_pool;

#include "pthread_winapi.h"
#include "LEFTsettings.h"
#include "GLResources.h"
#ifdef _WIN32
LRESULT	CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
#endif
#include "GLWindow.h"
#include "GLDefines.h"
//#include "SoundPlayer.h"
#include "RobotModel.h"
#include "Map.h"
#include "GLFont.h"
#include "BFGEffect.h"

#include "Debug.h"

#ifndef _WIN32
#define SINGLE_THREADED
#endif

bool gActive;
GLResources * gResources = 0;
Settings * gSettings = 0;
GLvector2f gScreen;
GLvector2f gScreenSize;
const unsigned int gFramerate = 60;

//SoundThreadList SoundPlayer::mThreads;

typedef struct {
  bool running;

  struct {
    HANDLE render;
    HANDLE tcp;
    HANDLE msg;
    bool msgrunning;
  } threads;

  Settings * settings;
  GLResources * resources;
  GLWindow * window;
  Map * map;

  GLParticle * cross;
  RobotModel * robot;
  LightSource * robotlight;
  LightSource * tactical[3];
  MapObject * house;
  bool dead;

  GLParticle * balls[1024];
  LightSource * lightballs[1024];
  int ballcount;
  
  struct {
    boost::asio::io_service * io_service;
    tcp_client * client;

    bool showscore;
    char score[1024];

    struct {
      bool dead;
      char name[256];
      GLfloat health;
      int frags;
      int latency;
      LARGE_INTEGER latencytimer;

      RobotModel * model;
      LightSource * robotlight;
    } friends[1024];
    HANDLE friendmutex;
  } net;

  struct {
    bool keydown[256];
    GLvector2f mousepos;
    unsigned int mousebutton;
  } control;

  bool consoleactive;
  bool inputactive;
  struct {
    HANDLE mutex;
    bool recenttimer;
    unsigned int recent;
    unsigned int recorder;
    unsigned int linecount;
    char linebuffer[16][129];
    GLParticle * bg;
  } console;

  struct {
    GLfloat fps;
    unsigned int framecounter;
    LARGE_INTEGER interval[16];
    LARGE_INTEGER performancefrequency;
  } timing;
} left_handle;

std::list<Debug::DebugVectorType> Debug::DebugVectors;
bool Debug::DebugActive = false;
void * Debug::DebugMutex;
GLfloat gDebugValue;

#ifndef _WIN32

// pthread WINAPI
void * CreateMutex(void * lpMutexAttributes, int bInitialOwner, const char * pName)
{
  pthread_mutex_t * result = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
  if(!pthread_mutex_init(result, 0)) {
    return result;
  }
  free(result);
  return 0;
}

void CloseMutex(void * mutex)
{
  pthread_mutex_destroy((pthread_mutex_t *) mutex);
  free(mutex);
}

void * CreateEvent(void * lpEventAttributes, int bManualReset, int bInitialState, const char * pName)
{
  pthread_event_t * result = (pthread_event_t *) malloc(sizeof(pthread_event_t));
  if(!pthread_cond_init(&result->cond, 0)) {
    if(!pthread_mutex_init(&result->mutex, 0)) {
      return result;
    }
    pthread_cond_destroy(&result->cond);
  }
  free(result);
  return 0;
}

void CloseEvent(void * vevent)
{
  pthread_event_t * event = (pthread_event_t *) vevent;
  pthread_cond_destroy(&event->cond);
  pthread_mutex_destroy(&event->mutex);
  free(event);
}

void PulseEvent(void * vevent)
{
  pthread_event_t * event = (pthread_event_t *) vevent;
  pthread_cond_broadcast(&event->cond);
}

void WaitForEvent(void * vevent)
{
  pthread_event_t * event = (pthread_event_t *) vevent;
  Lock(&event->mutex);
  pthread_cond_wait(&event->cond, &event->mutex);
  Unlock(&event->mutex);
}

void QueryPerformanceCounter(LARGE_INTEGER * pCounter)
{
  struct timespec tp;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &tp);
  if(pCounter) pCounter->QuadPart = (tp.tv_sec * 1000 + tp.tv_nsec / 1000000); 
}

void QueryPerformanceFrequency(LARGE_INTEGER * pFreq)
{
  if(pFreq) pFreq->QuadPart = 1000;
}

#define WINAPI *
#define INVALID_HANDLE_VALUE 0
void * CreateThread(int lpThreadAttributes, int dwStackSize, unsigned int *(*lpStartAddress)(void*), void * lpParameter, int dwCreationFlags, int lpThreadId)
{
  pthread_t * result = (pthread_t *) malloc(sizeof(pthread_t));
  if(!pthread_create(result, 0, (void* (*)(void*))lpStartAddress, lpParameter)) {
    return result;
  }
  free(result);
  return 0;
}

void WaitForThread(void * thread)
{
  void * thread_result;
  pthread_join(*(pthread_t *)thread, &thread_result);
}
#endif

void stop(left_handle * left)
{
  left->threads.msgrunning = false;
  if(left->net.client) left->net.client->wakeup();
  server->wakeup();
  WaitForThread(left->threads.msg);
  left->running = false;
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

void updateMousePosition(left_handle * left)
{
#ifdef _WIN32
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
#else
  int root_x, root_y;
  int win_x, win_y;
  Window window_returned;
  unsigned int mask_return;
  XQueryPointer(left->window->display(), left->window->window(), &window_returned, &window_returned, &root_x, &root_y, &win_x, &win_y, &mask_return);
  left->control.mousepos = GL_SCREEN_BOTTOMLEFT + GLvector2f((GLfloat)win_x, GL_SCREEN_FHEIGHT - (GLfloat)win_y);
#endif
}

int isPlayer(left_handle * left, Collidable * c)
{
  for(int i = 0; i < 1024; i++) {
    if(left->net.friends[i].model == c) {
      return i;
    }
  }
  return -1;
}

void removePlayer(left_handle * left, int index)
{
  Lock(left->net.friendmutex);
  if(left->net.friends[index].model) {
    left->map->removeCollidable(left->net.friends[index].model);
    left->map->LightSources().remove(left->net.friends[index].robotlight);
    delete left->net.friends[index].robotlight;
    delete left->net.friends[index].model;
    left->net.friends[index].model = 0;
  }
  Unlock(left->net.friendmutex);
}

GLfloat getDamage(unsigned int type)
{
  switch(type) {
  case PROJECTILE_TYPE_ROCKET:
    return 10.0f;
  case PROJECTILE_TYPE_SHOTGUN:
    return 5.0f;
  case PROJECTILE_TYPE_GRENADE:
    return 50.0f;
  case PROJECTILE_TYPE_NAIL:
    return 9.0f;
  } 
  return 0.0f;
}

void collide(void * vleft, Polygon & polygon, Collidable * c, Projectile * p)
{
  left_handle * left = (left_handle *) vleft;

  if(c && p) {
    bool frag = false;
    int index = isPlayer(left, c);
    if(index > 0 && !left->net.client && !left->net.friends[index].dead) {
      left->net.friends[index].health -= getDamage(p->type);
      left_message * m = new_message(LEFT_NET_MSG_UPDATE_STATS);
      m->msg.stats.health = left->net.friends[index].health;
      server->send_message(m, index);
      if(left->net.friends[index].health <= 0.0f) {
        m->header.msg = LEFT_NET_MSG_DEAD;
        m->header.sender = index;
        m->header.size = 0;
        server->distribute(m);
        server->push(m);
        frag = true;
      }
      left_net_message_pool->del(m);
    } else if(c == left->robot && !left->dead && left->robot->getHUD()) {
      left->robot->getHUD()->addHealth(- getDamage(p->type));
      if(left->robot->getHUD()->getHealth() <= 0.0f) {
        left_message * m = new_message(LEFT_NET_MSG_DEAD);
        m->header.sender = 0;
        left->dead = true;
        server->distribute(m);
        frag = true;
      }
    }
    if(frag) {
      int owner = isPlayer(left, (Collidable *) p->owner);
      if(owner > 0) {
        left->net.friends[owner].frags++;
      } else if(p->owner == left->robot) {
        left->net.friends[0].frags++;
      }
    }
  } else
  if(!left->net.client) {
    left_message * map = new_message(LEFT_NET_MSG_DESTROY_MAP);
    uintptr p = (uintptr) &map->msg;

    unsigned int vertcount = polygon.size();

    memcpy((void *) p, &vertcount, sizeof(unsigned int));
    p += sizeof(unsigned int);

    Polygon::iterator vit;
    for(vit = polygon.begin(); vit != polygon.end(); ++vit) {
      IntPoint current = *vit;
      float x = (GLfloat) (current.X);
      float y = (GLfloat) (current.Y);
      memcpy((void *) p, &x, sizeof(float));
      p += sizeof(float);
      memcpy((void *) p, &y, sizeof(float));
      p += sizeof(float);
    }

    map->header.size = sizeof(unsigned int) +
                       sizeof(float) * 2 * vertcount;

    server->distribute(map);

    left_net_message_pool->del(map);
  }
}

void drawScore(left_handle * left)
{
  bm_font * font = left->resources->getFont(GL_RESOURCE_DATAPATH "couriernew.fnt")->font;

  GLfloat width = 600.0f, height = 500.0f;
  GLvector2f center = GL_SCREEN_CENTER;
  GLvector2f cursor = (gScreenSize / 2.0f) - (GLvector2f(width, -height) / 2.0f);
  cursor.x += 50.0f;
  cursor.y -= 2 * font->line_h;

  left->console.bg->setSize(width, height);
  left->console.bg->moveTo(center.x, center.y);
  left->console.bg->draw();

  glColor4f(1.0f, 0.0f, 0.0f, 1.0f);
  glFontPrint(font, cursor, "LEFT SCORE - THINGS THAT MATTER");
  cursor.y -= 3 * font->line_h;
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

  int index = 0, frags, latency, len;
  int scores;
  memcpy(&scores, &left->net.score[index], sizeof(int));
  index += sizeof(int);
  for(int i = 0; i < scores; i++) {
    len = strlen(&left->net.score[index]);
    memcpy(&frags, &left->net.score[index + len + 1], sizeof(int));
    memcpy(&latency, &left->net.score[index + len + 1 + sizeof(int)], sizeof(int));
    glFontPrint(font, cursor + GLvector2f( 0.0f, 0.0f), "%s", &left->net.score[index]);
    glFontPrint(font, cursor + GLvector2f(350.0f, 0.0f), "%d", frags);
    glFontPrint(font, cursor + GLvector2f(450.0f, 0.0f), "%d", latency);
    cursor.y -= 1.5f * font->line_h;
    index += len + 1 + sizeof(int) + sizeof(int);
  }
}

void renderScene(left_handle * left)	
{
  updateMousePosition(left);
  left->map->setUpdate(left->net.client == 0);

  if(server->count() > 0 || left->net.client) {
    GLvector2f pos = left->robot->pos();
    left_message * posmsg = new_message(LEFT_NET_MSG_UPDATE_POS);
    posmsg->msg.position.weaponangle = ((left->control.mousepos - pos).angle() / M_PI) * 180.0f;
    posmsg->msg.position.robotangle = left->robot->angle();
    posmsg->msg.position.xpos = pos.x;
    posmsg->msg.position.ypos = pos.y;
    if(left->net.client) {
      left->net.client->send_message(posmsg);
    } else {
      server->distribute(posmsg);
    }
    left_net_message_pool->del(posmsg);
  }

	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

  for(int i = 0; i < left->ballcount; i++) {
    left->balls[i]->move();
    left->lightballs[i]->pos = left->balls[i]->pos();
  }
  if(!left->control.keydown['S']) left->control.keydown['S'] = left->consoleactive;
  
  if(left->dead) {
    left->control.mousebutton = 0;
  }
  
  ProjectileList projectiles = left->robot->control(left->control.keydown, left->control.mousepos, left->control.mousebutton);
  left->control.mousebutton = 0;
  
  if(!projectiles.empty()) {
    Projectile * p = 0;
    left_message * proj = new_message(LEFT_NET_MSG_PROJECTILE);
    foreach(ProjectileList, p, projectiles) {
      proj->msg.projectile.type = p->type;
      proj->msg.projectile.dirx = p->velocity().x;
      proj->msg.projectile.diry = p->velocity().y;
      if(left->net.client) {
        left->net.client->send_message(proj);
      } else {
        server->distribute(proj);
      }
    }
    left_net_message_pool->del(proj);
  }

  left->robot->integrate(0.1f);
  left->robotlight->pos = left->robot->pos();
  left->robotlight->angle = ((left->control.mousepos - left->robot->pos()).angle() / M_PI) * 180.0f;
  left->cross->moveTo(left->control.mousepos.x, left->control.mousepos.y);
  gScreen = left->robot->pos() - GLvector2f(GL_SCREEN_FWIDTH / 2.0f, GL_SCREEN_FHEIGHT / 2.0f);
  left->map->drawShadows();

  left->map->draw();
  left->map->collision();
  left->map->drawProjectiles();
  left->map->drawAnimations();

  if(left->dead && !left->net.client && left->robot->getHUD()) {
    left->robot->getHUD()->addHealth(0.1f);
    if(left->robot->getHUD()->getHealth() >= 100.0f) {
      left->dead = false;
      left_message * m = new_message(LEFT_NET_MSG_RESPAWN);
      m->header.sender = 0;
      server->distribute(m);
      left_net_message_pool->del(m);
    }
  }

  if(left->dead) {
    left->robot->setAlpha(0.1f);
  } else {
    left->robot->setAlpha(1.0f);
  }
  left->robot->draw();
  for(int i = 0; i < left->ballcount; i++)
    left->balls[i]->draw();
  left->cross->draw();

  //FIXME
  if(false) for(int i = 0; i < 1024; i++) {
    Lock(left->net.friendmutex);
    if(left->net.friends[i].model) {
      if(!left->net.friends[i].dead) {
        left->net.friends[i].model->integrate(0.1f);
        left->net.friends[i].model->setAlpha(left->map->getOpacity(left->net.friends[i].model->pos()));
        left->net.friends[i].robotlight->pos = left->net.friends[i].model->pos();
        left->net.friends[i].model->setAlpha(left->map->getOpacity(left->net.friends[i].model->pos()));
        left->net.friends[i].model->draw();
      } else {
        left->net.friends[i].robotlight->visible = false;
      }
        
      if(!left->net.client && left->net.friends[i].dead) {
        left->net.friends[i].health += 0.1f;
        if(left->net.friends[i].health >= 100.0f) {
          left->net.friends[i].health = 100.0f;
          left->net.friends[i].dead = false;
          left->net.friends[i].robotlight->visible = true;
          left_message * m = new_message(LEFT_NET_MSG_RESPAWN);
          m->header.sender = i;
          server->distribute(m);
          m->header.msg = LEFT_NET_MSG_UPDATE_STATS;
          m->header.size = sizeof_message(m->header.msg);
          m->msg.stats.health = 100.0f;
          server->send_message(m, i);
          left_net_message_pool->del(m);
        } else {
          left_message * m = new_message(LEFT_NET_MSG_UPDATE_STATS);
          m->msg.stats.health = left->net.friends[i].health;
          server->send_message(m, i);
          left_net_message_pool->del(m);
        }
      }
    }

    Unlock(left->net.friendmutex);
  }

  if(left->net.showscore) {
    drawScore(left);
  }

  bm_font * font = left->resources->getFont(GL_RESOURCE_DATAPATH "couriernew.fnt")->font;
  Lock(left->console.mutex);
  if(left->consoleactive || left->inputactive || left->console.recent > 0) {
    unsigned int count = (left->console.recent > 0 && !left->consoleactive ? left->console.recent : left->console.linecount);
    unsigned int offset = left->console.recent > 0 ? 1 : 0;
    char s[129];
    if(left->console.recent == 0 && left->inputactive) count = 1;
    if(left->console.recent > 0 && left->inputactive) { 
      count += 1;
      offset = 0;
    }

    left->console.bg->setSize(GL_SCREEN_FWIDTH, 3.0f + count * font->line_h);
    left->console.bg->moveTo(GL_SCREEN_BOTTOMLEFT.x + left->console.bg->w() / 2.0f, GL_SCREEN_BOTTOMLEFT.y + left->console.bg->h() / 2.0f);
    left->console.bg->draw();
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
    for(int i = offset; i < offset+count; i++) {
      if(left->console.linebuffer[i][0]) {
        glFontPrint(font, GLvector2f(0.0f, (3.0f + (i - (left->inputactive ? 0 : offset)) * font->line_h)), left->console.linebuffer[i]);
      }
    }
  }
  Unlock(left->console.mutex);
  GLvector2f fpspos(GL_SCREEN_FWIDTH - 50.0f, GL_SCREEN_FHEIGHT - 30.0f);
  glColor4f(0.8f, 0.8f, 0.0f, 1.0f);
  glFontPrint(font, fpspos, "%-2.0f", left->timing.fps);

  LARGE_INTEGER now;
  QueryPerformanceCounter(&now);
  for(int interval = 0; interval < 16; interval++) {
    if(now.QuadPart - left->timing.interval[interval].QuadPart > (interval + 1) * left->timing.performancefrequency.QuadPart) {
      switch(interval) {
      case 0: // 1 sec
        left->timing.fps = (GLfloat) left->timing.framecounter;
        left->timing.framecounter = 0;
        break;
      case 2: // 3 sec
        if(left->console.recent > 0) { 
          if(left->console.recenttimer) {
            left->console.recent--;
          } else {
            left->console.recenttimer = true;
          }
        } else {
          left->console.recenttimer = false;
        }
        break;
      case 4:
        if(!left->net.client) {
          left_message * ping = new_message(LEFT_NET_MSG_PING);
          for(int i = 0; i < 1024; i++) {
            if(left->net.friends[i].model) {
              QueryPerformanceCounter(&left->net.friends[i].latencytimer);
              server->send_message(ping, i);
            }
          }
          left_net_message_pool->del(ping);
        }
      }
      left->timing.interval[interval] = now;
    }
  }
  left->timing.framecounter++;
}

int onMouseDown(left_handle * left, unsigned int button, unsigned int x, unsigned int y)
{
  left->control.mousebutton = button;
  return 0;
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
    cprintf(left, "ls                         : list settings");
    cprintf(left, "ls res                     : list resources");
    cprintf(left, "set <name> <value>         : set setting");
    cprintf(left, "save <filename>            : save current map");
    cprintf(left, "load <filename>            : load map");
    cprintf(left, "load <filename>            : load old map");
    cprintf(left, "help                       : show help");
    cprintf(left, "");
  } else

  if(strcmp(op, "save") == 0) {
    if(pcount == 1) {
      std::ofstream f(param[0]);
      //f << left->map->polygons();
      f.close();
    }
  } else
  if(strcmp(op, "loadold") == 0) {
    if(pcount == 1) {
      std::ifstream f(param[0]);
      if(f.good()) {
        Polygons read, convert;
        f >> read;
        Polygons::iterator pit;
        for(pit = read.begin(); pit != read.end(); pit++) {
          Polygon p = *pit;
          Polygon convertp;
          Polygon::iterator vit;
          for(vit = p.begin(); vit != p.end(); vit++) {
            IntPoint current = *vit, next;
            current.X /= 100000L;
            current.Y /= 100000L;
            convertp.push_back(current);
          }
          convert.push_back(convertp);
        }

        if(!convert.empty()) {
          left->map->setPolygons(convert);
        }
      }
      f.close();
    }
  } else

  if(strcmp(op, "load") == 0) {
    if(pcount == 1) {
      std::ifstream f(param[0]);
      if(f.good()) {
        Polygons p;
        f >> p;
        if(!p.empty()) {
          left->map->setPolygons(p);
        }
      }
      f.close();
    }
  } else

  if(strcmp(op, "ls") == 0 && pcount > 0 && strcmp(param[0], "res") == 0) {
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
//          GLSoundResource * sres = (GLSoundResource *) res;
//          cprintf(left, "%d: Sound size:%dkB format:%04x rate:%d [%s]", i, (int)sres->sound->size/1000, (int)sres->sound->format, (int)sres->sound->freq, rp->path);
          break;
        }
        case GL_RESOURCE_FONT: {
          GLFontResource * fres = (GLFontResource *) res;
          cprintf(left, "%d: Font [%s]", i, rp->path);
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
  } else if(strcmp(op, "ls") == 0) {
    std::list<std::string> keys;
    left->settings->getkeys(keys);
    for(std::list<std::string>::iterator iter = keys.begin(); iter != keys.end(); iter++) {
      SettingsValue value = left->settings->get(*iter);
      switch(value.type()) {
      case stString:
        cprintf(left, "%s %s", (*iter).c_str(), value.gets().c_str());
        break;
      case stInt:
        cprintf(left, "%s %d", (*iter).c_str(), value.geti());
        break;
      case stFloat:
        cprintf(left, "%s %f", (*iter).c_str(), value.getf());
        break;
      }
    }
  } else if(left->settings->get(op).type() != stNone) {
    SettingsValue value = left->settings->get(op);
    switch(value.type()) {
    case stString:
      cprintf(left, "%s", value.gets().c_str());
      break;
    case stInt:
      cprintf(left, "%d", value.geti());
      break;
    case stFloat:
      cprintf(left, "%f", value.getf());
      break;
    }
  } else if(strcmp(op, "set") == 0) {
    if(pcount >= 2) {
      if(left->settings->get(param[0]).type() != stNone) {
        SettingsValue value = left->settings->get(param[0]);
        switch(value.type()) {
        case stString:
          left->settings->set(param[0], std::string(param[1]));
          break;
        case stInt:
          left->settings->set(param[0], (int) atoi(param[1]));
          break;
        case stFloat:
          left->settings->set(param[0], (float) atof(param[1]));
          break;
        }
      }
    }
  } else {
    left_message * chat = new_message(LEFT_NET_MSG_CHAT);
    strcpy(chat->msg.chat.name, left->settings->gets("p_name").c_str());
    strcpy(chat->msg.chat.msg, cmd);
    if(left->net.client) {
      left->net.client->send_message(chat);
    } else {  
      server->distribute(chat);
    }
    cprintf(left, "<%s> %s", left->settings->gets("p_name").c_str(), left->console.linebuffer[0]);
    if(left->console.recent < 4) left->console.recent++;
    left_net_message_pool->del(chat);
  }

  memset(left->console.linebuffer[0], 0, 129);
  left->console.recorder = 0;
}

#ifdef _WIN32
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
      left->inputactive = false;
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
      stop(left);
      PostQuitMessage(0);
      break;
    case VK_F1:
      Debug::DebugActive = !Debug::DebugActive;
      return 0;
    case VK_F2:
    case VK_OEM_3: // '`' or '^'
      left->consoleactive = !left->consoleactive;
      left->control.keydown['S'] = left->consoleactive;
      left->console.recorder = 0;
      break;
    case VK_F3:
      left->inputactive = !left->inputactive;
      break;
    case 'T':
      if(!left->consoleactive) left->inputactive = true;
      break;
    case VK_F5: {
        if(!left->net.client) {
          tcp::resolver resolver(io_service);
          tcp::resolver::query query(left->settings->gets("net_host"), left->settings->gets("net_port"));
          tcp::resolver::iterator iterator = resolver.resolve(query);

          cprintf(left, "> connecting...");
          left->net.client = new tcp_client(io_service, iterator);
        }
      } break;
    case VK_TAB: {
      if(!left->net.showscore) {
        left_message * m = new_message(LEFT_NET_MSG_GET_SCORE);
        if(left->net.client) {
          left->net.client->send_message(m);
        } else {
          server->push(m);
        }
        left_net_message_pool->del(m);
      }
      left->net.showscore = true;
      } break;
    }
    return 0;								
  case WM_KEYUP:
    left->control.keydown[wParam] = false;
    switch(wParam) {
    case VK_TAB:
      left->net.showscore = false;
      break;
    }
    return 0;
  case WM_RBUTTONDOWN:
  case WM_LBUTTONDOWN:
  case WM_MBUTTONDOWN:
  case WM_XBUTTONDOWN:
    return onMouseDown(left, wParam, LOWORD(lParam), GL_SCREEN_IHEIGHT - HIWORD(lParam));
  case WM_RBUTTONUP:
  case WM_LBUTTONUP:
  case WM_MBUTTONUP:
  case WM_XBUTTONUP:
    left->control.mousebutton = wParam;
    return 0;
  case WM_MOUSEWHEEL: {
      if((int)wParam < 0) {
        left->map->addMinimapZoom(1.0f);
      } else {
        left->map->addMinimapZoom(-1.0f);
      }
    } return 0;
  case WM_SIZE:
    //gScreenSize.x = (GLfloat) ((lParam >> 16) & 0xFFFF);
    //gScreenSize.y = (GLfloat) (lParam & 0xFFFF);
    return 0;
  }

  return DefWindowProc(hWnd,uMsg,wParam,lParam);
}
#else
void XEventHandler(left_handle * left)
{
  XEvent event;
  while(XPending(left->window->display()) > 0)        
  {
    XNextEvent(left->window->display(), &event);     
    KeySym keysym;
    
    switch(event.type) {                                
    case Expose:                 
      if(event.xexpose.count != 0)
        break;   
      break;                       
    case ConfigureNotify:
#if 0
      /* call resizeGL only if our window-size changed */
      if ((event.xconfigure.width != GLWin.width) || (event.xconfigure.height != GLWin.height)) {
        width = event.xconfigure.width;
        height = event.xconfigure.height;
        resizeGL(width, height);
      }
#endif
      break;
    case ButtonPress:
      onMouseDown(left, 1, 0, 0);
      break;
    case KeyPress:
      keysym = XLookupKeysym(&event.xkey, 0);
      if(keysym >= 0x20 && keysym <= 0x80) {
        left->control.keydown[keysym] = true;
      }
      switch(keysym) {
      case XK_Escape:
        stop(left);
        break;
      case XK_F1:
        Debug::DebugActive = !Debug::DebugActive;
        break;
      case XK_F2:
        left->consoleactive = !left->consoleactive;
        left->control.keydown['S'] = left->consoleactive;
        left->console.recorder = 0;
        break;
      case XK_F3:
        left->inputactive = !left->inputactive;
        break;
      case XK_T:
        if(!left->consoleactive) left->inputactive = true;
        break;
      case XK_F5:
        if(!left->net.client) {
          tcp::resolver resolver(io_service);
          tcp::resolver::query query(left->settings->gets("net_host"), left->settings->gets("net_port"));
          tcp::resolver::iterator iterator = resolver.resolve(query);

          cprintf(left, "> connecting...");
          left->net.client = new tcp_client(io_service, iterator);
        }
        break;
      case XK_Tab:
        if(!left->net.showscore) {
          left_message * m = new_message(LEFT_NET_MSG_GET_SCORE);
          if(left->net.client) {
            left->net.client->send_message(m);
          } else {
            server->push(m);
          }
          left_net_message_pool->del(m);
        }
        left->net.showscore = true;
        break;
      }
      break;
      
    case KeyRelease:
      keysym = XLookupKeysym(&event.xkey, 0);
      if(keysym >= 0x20 && keysym <= 0x80) {
        left->control.keydown[keysym] = false;
      }
      switch(keysym) {
      case XK_Tab:
        left->net.showscore = false;
        break;
      }
      break;      
      
    case ClientMessage:
      if(strcmp(XGetAtomName(left->window->display(), event.xclient.message_type), "WM_PROTOCOLS") == 0) {
        left->running = false;
      }
      break;
    default:
      break;
    }
  }
}
#endif

DWORD WINAPI run_messages(void * data)
{
  left_handle * left = (left_handle *) data;
  left->threads.msgrunning = true;
  while(left->threads.msgrunning) {
    if(left->net.client && !left->net.client->isconnected()) {
      int timeout = 1000; // 1 sec
      cprintf(left, "> connecting");
      while(!left->net.client->isconnected() && --timeout) { 
        Sleep(1);
      }

      if(!timeout) {
        for(int i = 0; i < 1024; i++) {
          removePlayer(left, i);
        }
        delete left->net.client;
        left->net.client = 0;
        cprintf(left, "> disconnected from server");
        left->console.recent++;
      }
    }
    
    left_message * m = 0;
    if(left->net.client) {
      left->net.client->wait();
      m = left->net.client->get_message();
    } else {
      server->wait();
      m = server->get_message();
    }

    if(!m) {
      continue;
    }

    if(m && m->header.sender < 1024U) {
      switch(m->header.msg) {
      case LEFT_NET_MSG_PING:
        if(left->net.client) {
          left_message * pong = new_message(LEFT_NET_MSG_PONG);
          left->net.client->send_message(pong);
          left_net_message_pool->del(pong);
        }
        break;
      case LEFT_NET_MSG_PONG:
        if(!left->net.client) {
          LARGE_INTEGER now;
          QueryPerformanceCounter(&now);
          left->net.friends[m->header.sender].latency = ((GLfloat)(now.QuadPart - left->net.friends[m->header.sender].latencytimer.QuadPart) / (GLfloat)left->timing.performancefrequency.QuadPart) * 1000.0f;
        }
        break;
      case LEFT_NET_MSG_GET_SCORE: {
          left_message * score = new_message(LEFT_NET_MSG_SCORE);
          int indices[1024];
          int frags[1024];
          int scores = 0;
          for(int i = 0; i < 1024; i++) {
            if(left->net.friends[i].model || i == 0) {
              indices[scores] = i; 
              frags[scores] = left->net.friends[i].frags;
              scores++;
            }
          }
          bool swapped = true;
          while(swapped) {
            swapped = false;
            for(int i = 0; i < scores - 1; i++) {
              int tmp = 0;
              if(frags[i] < frags[i + 1]) {
                tmp = frags[i];
                frags[i] = frags[i + 1];
                frags[i+1] = tmp;
                tmp = indices[i];
                indices[i] = indices[i + 1];
                indices[i+1] = tmp;
                swapped = true;
              }
            }
          }
          
          uintptr p = (uintptr) &score->msg;
          memcpy((void *) p, &scores, sizeof(int));
          p += sizeof(int);
          for(int i = 0; i < scores; i++) {
            strcpy((char *) p, left->net.friends[indices[i]].name);
            p += strlen(left->net.friends[indices[i]].name) + 1;
            memcpy((void *) p, &left->net.friends[indices[i]].frags, sizeof(int));
            p += sizeof(int);
            memcpy((void *) p, &left->net.friends[indices[i]].latency, sizeof(int));
            p += sizeof(int);
          }
          score->header.size = p - (uintptr) &score->msg;
          if(m->header.sender != 0) {
            server->send_message(score, m->header.sender);
          } else {
            server->push(score);
          }
          left_net_message_pool->del(score);
        } break;
      case LEFT_NET_MSG_SCORE: {
          memcpy(left->net.score, &m->msg, sizeof(m->header) + m->header.size);
          left->net.showscore = true;
        } break;
      case LEFT_NET_MSG_DEAD:     
        left->net.friends[m->header.sender].dead = true;
        break;
      case LEFT_NET_MSG_RESPAWN:
        left->net.friends[m->header.sender].dead = false;
        break;
      case LEFT_NET_MSG_WUI:
        cprintf(left, "> %sconnected: %s", !left->net.client ? "client " : "", m->msg.wui.name);
        if(!left->net.client) {     
          Polygons cmap = left->map->polygons();
          #define uintptr unsigned long
          left_message * map = new_message(LEFT_NET_MSG_UPDATE_MAP);
          uintptr p = (uintptr) &map->msg;
          unsigned int polycount = cmap.size();
          unsigned int allvertcount = 0;

          memcpy((void *) p, &polycount, sizeof(unsigned int));
          p += sizeof(unsigned int);

          Polygons::iterator pit;
          for(pit = cmap.begin(); pit != cmap.end(); ++pit) {
            Polygon polygon = *pit;
            unsigned int vertcount = polygon.size();
            allvertcount += vertcount;

            memcpy((void *) p, &vertcount, sizeof(unsigned int));
            p += sizeof(unsigned int);

            Polygon::iterator vit;
            for(vit = polygon.begin(); vit != polygon.end(); ++vit) {
              IntPoint current = *vit;
              float x = (GLfloat) (current.X);
              float y = (GLfloat) (current.Y);
              memcpy((void *) p, &x, sizeof(float));
              p += sizeof(float);
              memcpy((void *) p, &y, sizeof(float));
              p += sizeof(float);
            }
          }

          map->header.size = sizeof(unsigned int) +
                             polycount * sizeof(unsigned int) +
                             sizeof(float) * 2 * allvertcount;

          cprintf(left, "> sending map p:%d size:%db", polycount, sizeof(map->header) + map->header.size);
          server->send_message(map, m->header.sender);

          left->net.friends[m->header.sender].health = 100.0f;
          strcpy(left->net.friends[m->header.sender].name, m->msg.wui.name);
           
          left_net_message_pool->del(map);
        } else {
          left_message * wui = new_message(LEFT_NET_MSG_WUI);
          strcpy(wui->msg.wui.name, left->settings->gets("p_name").c_str());
          left->net.client->send_message(wui);
          left_net_message_pool->del(wui);
        }
        break;
      case LEFT_NET_MSG_UPDATE_STATS:{
          if(left->net.client && left->robot->getHUD()) {
            left->robot->getHUD()->setHealth(m->msg.stats.health);
            if(left->robot->getHUD()->getHealth() <= 0.0f) {
              left->dead = true;
            } else if(left->robot->getHUD()->getHealth() >= 100.0f) {
              left->dead = false;
            }
          }
        } break;
      case LEFT_NET_MSG_PROJECTILE: {
          Projectile * p = 0;
          Lock(left->net.friendmutex);
          switch(m->msg.projectile.type) {
          case PROJECTILE_TYPE_ROCKET:
            p = new RocketProjectile(left->net.friends[m->header.sender].model->pos(), GLvector2f(m->msg.projectile.dirx, m->msg.projectile.diry), left->map);
            break;
          case PROJECTILE_TYPE_SHOTGUN:
            p = new ShotgunProjectile(left->net.friends[m->header.sender].model->pos(), GLvector2f(m->msg.projectile.dirx, m->msg.projectile.diry), left->map);
            break;
          case PROJECTILE_TYPE_GRENADE:
            p = new GrenadeProjectile(left->net.friends[m->header.sender].model->pos(), GLvector2f(m->msg.projectile.dirx, m->msg.projectile.diry), left->map);
            break;
          case PROJECTILE_TYPE_NAIL:
            p = new NailProjectile(left->net.friends[m->header.sender].model->pos(), GLvector2f(m->msg.projectile.dirx, m->msg.projectile.diry), left->map);
            break;
          }
          p->owner = left->net.friends[m->header.sender].model;
          Unlock(left->net.friendmutex);
          left->map->addProjectile(p);
        } break;
      case LEFT_NET_MSG_BYE:
        cprintf(left, "> %s disconnected from server", left->net.friends[m->header.sender].name);
        left->console.recent++;
        removePlayer(left, m->header.sender);
        break;
      case LEFT_NET_MSG_CHAT:
        cprintf(left, "<%s> %s", m->msg.chat.name, m->msg.chat.msg);
        if(left->console.recent < 4) left->console.recent++;
        break;
      case LEFT_NET_MSG_UPDATE_POS: {
          Lock(left->net.friendmutex);
          if(!left->net.friends[m->header.sender].model) {
            left->net.friends[m->header.sender].model = new RobotModel(left->map, left_net_models[m->header.sender % left_net_modelcount], GL_RESOURCE_DATAPATH "arm_grey.png", true);
            left->net.friends[m->header.sender].robotlight = new LightSource(GLvector2f(m->msg.position.xpos, m->msg.position.ypos), GLvector3f(0.0f, 0.0f, 0.0f), 1.0f, glpLightCone);
            left->map->LightSources().push_back(left->net.friends[m->header.sender].robotlight);
            left->net.friends[m->header.sender].model->moveTo(m->msg.position.xpos, m->msg.position.ypos);
            left->map->addCollidable(left->net.friends[m->header.sender].model);
          }
          GLvector2f newvelocity = GLvector2f(m->msg.position.xpos,  m->msg.position.ypos) - left->net.friends[m->header.sender].model->pos();
          left->net.friends[m->header.sender].model->setVelocity(newvelocity);
          left->net.friends[m->header.sender].model->setWeaponAngle(m->msg.position.weaponangle);
          left->net.friends[m->header.sender].model->setAngle(m->msg.position.robotangle);
          left->net.friends[m->header.sender].robotlight->angle = m->msg.position.weaponangle;
          Unlock(left->net.friendmutex);
        } break;
      case LEFT_NET_MSG_UPDATE_MAP: {
          if(left->net.client) {
            uintptr p = (uintptr) &m->msg.buffer;
            Polygons servermap;
            unsigned int polycount = 0;
            
            memcpy(&polycount, (void *) p, sizeof(unsigned int));
            p += sizeof(unsigned int);

            for(int poly = 0; poly < polycount; poly++) {
              Polygon polygon;
              unsigned int vertcount = 0;
              
              memcpy(&vertcount, (void *) p, sizeof(unsigned int));
              p += sizeof(unsigned int);

              for(int vert = 0; vert < vertcount; vert++) {
                IntPoint point;
                float x = 0.0f;
                float y = 0.0f;
                memcpy(&x, (void *) p, sizeof(float));
                p += sizeof(float);
                memcpy(&y, (void *) p, sizeof(float));
                p += sizeof(float);
              
                point.X = (x);
                point.Y = (y);
                polygon.push_back(point);
              }

              servermap.push_back(polygon);
            }
            left->map->setPolygons(servermap);
          }
        } break;

      case LEFT_NET_MSG_DESTROY_MAP: {
          if(left->net.client) {
            uintptr p = (uintptr) &m->msg.buffer;
            Polygon polygon;
            unsigned int vertcount = 0;
            
            memcpy(&vertcount, (void *) p, sizeof(unsigned int));
            p += sizeof(unsigned int);

            for(int vert = 0; vert < vertcount; vert++) {
              IntPoint point;
              float x = 0.0f;
              float y = 0.0f;
              memcpy(&x, (void *) p, sizeof(float));
              p += sizeof(float);
              memcpy(&y, (void *) p, sizeof(float));
              p += sizeof(float);
            
              point.X = (x);
              point.Y = (y);
              polygon.push_back(point);
            }

            left->map->addPolygon(polygon);
          }
        } break;
      }
      left_net_message_pool->del(m);
    }
  };
#ifndef _WIN32
  pthread_exit(0);
#endif
  return 0;
}

DWORD WINAPI run_asio(void * data)
{
  left_handle * left = (left_handle *) data;
  try
  {
    left->net.io_service->run();
  }
  catch (std::exception& e)
  {
    std::cerr << e.what() << std::endl;
  }

  assert(!left->running);
#ifndef _WIN32
  pthread_exit(0);
#endif
  return 0;
}

DWORD WINAPI run(void * lh)
{
  left_handle * left = (left_handle *) lh;
#ifndef SINGLE_THREADED  
  srand( (unsigned int)GetTickCount() );
  left->window->makeCurrent();
  QueryPerformanceFrequency(&left->timing.performancefrequency);
#endif
  
  if(left->running && !left->resources) {
    left->resources = new GLResources();
    gResources = left->resources;
    left->console.bg = new GLParticle(1, 1, 0.0f, 0.0f, 0.0f, 0.9f, glpSolid);
  }

  if(left->running && !left->map) {
    left->map = new Map();
    left->map->setCallback(&collide, left);

    left->robot = new RobotModel(left->map);
#ifndef _DEBUG
    GLvector2f spawn = left->map->randomSpawn();
#else
      GLvector2f spawn = GLvector2f(1000.0f, 1000.0f);
#endif
    left->robot->moveTo(spawn.x, spawn.y);
    left->map->addCollidable(left->robot);
    left->robotlight = new LightSource(left->robot->pos(), GLvector3f(0.0f, 0.0f, 0.0f), 1.0f, glpLightCone);
    left->map->LightSources().push_back(left->robotlight);
    left->dead = false;

    left->house = new MapObject("house");
    left->map->MapObjects().push_back(left->house);
    left->house->moveTo(2000.0f, 2000.0f);

    left->cross = new GLParticle(50, 50, 1.0f, 1.0f, 1.0f, 1.0f, glpCross);
#ifndef _DEBUG
    for(int i = 0; i < 64; i++) {
      left->balls[i] = new GLParticle(8, 8, frand(), frand(), frand(), 1.0f, glpCircle);
      GLvector2f spawn = left->map->randomSpawn(300.0f);
#else
    for(int i = 0; i < 2; i++) {
      left->balls[i] = new GLParticle(8, 8, frand(), frand(), frand(), 1.0f, glpCircle);
      GLvector2f spawn = GLvector2f(1000.0f, 1000.0f);
#endif
      GLvector2f velocity((frand() * 8.0f) - 4.0f, (frand() * 8.0f) - 4.0f);
      left->balls[i]->moveTo(spawn.x, spawn.y);
      left->balls[i]->setVelocity(velocity);
      left->lightballs[i] = new LightSource(left->balls[i]->pos(), left->balls[i]->getColor(), 0.3f, glpLight);
      left->map->LightSources().push_back(left->lightballs[i]);
      left->map->addCollidable(left->balls[i]);
      left->ballcount++;
    }
  }

#ifndef SINGLE_THREADED
  while(left->running) {
    if(gActive)	{
#else
  if(left->running) {
#endif
      LARGE_INTEGER begin, end;
      QueryPerformanceCounter(&begin);
      renderScene(left);
      QueryPerformanceCounter(&end);
      static const unsigned long long waitinticks = (left->timing.performancefrequency.QuadPart / 60);
      while(end.QuadPart - begin.QuadPart < waitinticks) {
        Sleep(0);
        QueryPerformanceCounter(&end);
      }
      left->window->swapBuffers();
#ifndef SINGLE_THREADED
    }
#endif
  }
  
#ifdef SINGLE_THREADED
  if(!left->running) {
#endif   
  for(int i = 0; i < left->ballcount; i++) {
    if(left->balls[i]) {
      delete left->balls[i];
      left->balls[i] = 0;
    }
  }

  Debug::clear();
  if(left->map) {
    delete left->map;
    left->map = 0;
  }
  if(left->cross) {
    delete left->cross;
    left->cross = 0;
  }
  if(left->robot) {
    delete left->robot;
    left->robot = 0;
  }
  if(left->resources) {
    delete left->resources;
    left->resources = 0;
    gResources = 0;
  }
  if(left->console.bg) {
    delete left->console.bg;
    left->console.bg = 0;
  }
#ifdef SINGLE_THREADED
  }
#endif
  return 0;
}

#ifdef _WIN32
int WINAPI WinMain(	HINSTANCE	hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
  MSG msg;
  timeBeginPeriod(1);
#else
int main(int argc, char ** argv)
{
  XInitThreads();
#endif  
  srand( (unsigned int)GetTickCount() );
  left_handle * left = new left_handle;
  memset(left, 0, sizeof(left_handle));
  left->console.mutex = CreateMutex(0, FALSE, "LeftConsoleMutex");
  left->console.linecount = 16; 
  left->settings = new Settings();
  gSettings = left->settings;

  left->net.io_service = &io_service;
  left->net.friendmutex = CreateMutex(0, FALSE, "LeftFriendMutex");
  left_net_message_pool = new message_pool();
  server = new tcp_server(io_service, left->settings->gets("p_name"));
  strcpy(left->net.friends[0].name, left->settings->gets("p_name").c_str());

  std::ifstream f(GL_RESOURCE_DATAPATH "config.cfg");
  if(f.good()) {
    char line[128];
    while(!f.eof()) {
      f.getline(line, 64, '\n');
      parseConsoleCommand(left, line);
    }
  }

  gScreenSize.x = (GLfloat) left->settings->geti("r_xsize");
  gScreenSize.y = (GLfloat) left->settings->geti("r_ysize");

#ifndef _WIN32
// FIXME
void * WndProc = 0;
#endif
  
  QueryPerformanceFrequency(&left->timing.performancefrequency);
  Debug::DebugMutex = CreateMutex(0, FALSE, "LeftDebugMutex");
  char s[64]; sprintf(s, "LEFT %s", LEFT_VERSION);
  left->window = new GLWindow(s, GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT, 32, left->settings->geti("r_fullscreen") != 0, (WNDPROC) WndProc);
  
  if(left->window) {
#if 0 //def _WIN32
    if(left->settings->geti("r_fullscreen") != 0) {
      wglSwapIntervalEXT(1);
    } else {
      wglSwapIntervalEXT(0);
    }
#endif
    left->running = left->window->isInitialized();

    if(left->running) {
#ifndef SINGLE_THREADED
#ifdef _WIN32
      SetWindowLongPtr(left->window->hWnd(), GWLP_USERDATA, (LONG_PTR) left);
      wglMakeCurrent(0, 0);
#else
      glXMakeCurrent(left->window->display(), None, 0);
#endif
      left->threads.render = CreateThread(0, 0, &run, left, 0, 0);
#endif
      left->threads.tcp = CreateThread(0, 0, &run_asio, left, 0, 0);
      left->threads.msg = CreateThread(0, 0, &run_messages, left, 0, 0);
#ifndef SINGLE_THREADED
      assert(left->threads.render != INVALID_HANDLE_VALUE);
#endif
      assert(left->threads.tcp != INVALID_HANDLE_VALUE);
      assert(left->threads.msg != INVALID_HANDLE_VALUE);
    }
  }

  while(left->running) {
#ifdef _WIN32    
    while(PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
      if(msg.message == WM_QUIT) {
        stop(left);
        break;
      }
      if(left->consoleactive || left->inputactive) TranslateMessage(&msg);
      DispatchMessage(&msg);
    }
#else
    XEventHandler(left);
    run(left);
#endif
  }
  
#ifdef SINGLE_THREADED
  left->running = false;
  run(left);
#endif
  
  left->net.io_service->stop();
#ifndef SINGLE_THREADED
  WaitForThread(left->threads.render);
#endif
  WaitForThread(left->threads.tcp);
  WaitForThread(left->threads.msg);
#ifndef _WIN32
  free(left->threads.tcp);
  free(left->threads.msg);
#endif
  CloseMutex(left->console.mutex);
  CloseMutex(left->net.friendmutex);
  CloseMutex(Debug::DebugMutex);
  
  for(int i = 0; i < 1024; i++) {
    if(left->net.friends[i].model) {
      delete left->net.friends[i].model;
    }
  }
  if(left->net.client) {
    delete left->net.client;
  }
  delete server;
  delete left_net_message_pool;
  delete left->window;
  delete left->settings;
  delete left;

#ifdef _WIN32
  timeEndPeriod(1);
#endif
	return 0;	
}

