/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#pragma warning( disable: 4503 )
#include "Map.h"
#include "Debug.h"

Map::Map() : mMap(0), mCollision(0), mCollidableCount(0)
{
  mMutex = CreateMutex(NULL, FALSE, "LeftMapMutex");
  mSpot = new GLParticle(GL_SCREEN_IWIDTH, GL_SCREEN_IWIDTH, 1.0f, 1.0f, 1.0f, 1.0f, glpLight);

  glGenTextures(1, &mFramebufferTexture);
  glBindTexture(GL_TEXTURE_2D, mFramebufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glGenFramebuffersEXT(1, &mFramebuffer);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFramebuffer);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mFramebufferTexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, NULL);
  
  generate();
}

Map::~Map()
{
  CloseHandle(mMutex);

  GLtriangle * t = 0;
  foreach(GLtriangleList, t, mMap) {
    delete t;
  }
  GLplane * p = 0;
  foreach(GLplaneList, p, mCollision) {
    delete p;
  }
  LightSource * s = 0;
  foreach(LightSourceList, s, mLightSources) {
    delete s;
  }

  glDeleteFramebuffersEXT(1, &mFramebuffer);
  glDeleteTextures(1, &mFramebufferTexture);
}

void Map::draw()
{
  Lock(mMutex);
  drawTriangleList(GLvector2f(0.0f, 0.0f), mMap, GLvector3f(0.4f, 0.3f, 0.3f), 0.0f);
  Unlock(mMutex);
}

void Map::drawShadows(GLvector2f window)
{
  GLplane * p;
  GLfloat radius = 1280.0f;
  Debug::clear();

  Lock(mMutex);
  LightSource * s = 0;
  foreach(LightSourceList, s, mLightSources) {
    GLvector2f pos = s->pos;   

    renderTarget(true);
    mSpot->setColor(GLvector3f(0.4f, 0.3f, 0.3f) + s->rgb, 1.0f);
    mSpot->moveTo(pos.x, pos.y);
    mSpot->draw();

    foreach(GLplaneList, p, mCollision) {
      GLvector2f base = p->base;
      GLvector2f dest = p->dest;
      GLvector3f scolor = GLvector3f(0.0f, 0.0f, 0.0f);

#if 0
      GLvector2f surf = base + p->dir / 2.0f; // midpoint of surface
      GLvector2f snorm = GLvector2f((dest.y - base.y), -(dest.x - base.x)).normal(); // surface normal
      Debug::add(new GLplane(surf, snorm * 15.0f), GLvector3f(1.0f, 0.0f, 0.0f), 4);
      scolor.x = (int(color.x) & int((*o)[i].color.x)) * (*o)[i].opacity; if(scolor.x < 0) scolor.x = 0;
      scolor.y = (int(color.y) & int((*o)[i].color.y)) * (*o)[i].opacity; if(scolor.y < 0) scolor.y = 0;
      scolor.z = (int(color.z) & int((*o)[i].color.z)) * (*o)[i].opacity; if(scolor.z < 0) scolor.z = 0;
#endif

      GLvector2f baseproj = base - pos;
      GLvector2f destproj = dest - pos;
      GLvector2f bproj = base + baseproj.normal() * (radius - baseproj.len());
      GLvector2f dproj = dest + destproj.normal() * (radius - destproj.len());  

      glBegin(GL_QUADS);
      glColor4f(scolor.x, scolor.y, scolor.z, 0.0f);
      glVertex3f(base.x, base.y,  0.0f);
      glVertex3f(bproj.x, bproj.y,  0.0f);
      glVertex3f(dproj.x, dproj.y,  0.0f);
      glVertex3f(dest.x, dest.y,  0.0f);
      glEnd();
    }
    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    renderTarget(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, mFramebufferTexture);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); 
      glVertex3f(window.x, window.y,  0.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(window.x + GL_SCREEN_FWIDTH, window.y,  0.0f);
      glTexCoord2f(1.0f, 1.0f); 
      glVertex3f(window.x + GL_SCREEN_FWIDTH, window.y + GL_SCREEN_FHEIGHT,  0.0f);
      glTexCoord2f(0.0f, 1.0f); 
      glVertex3f(window.x, window.y + GL_SCREEN_FHEIGHT,  0.0f);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
  }
  
  Unlock(mMutex);
}

void Map::generate()
{
  Clipper c;
  Polygons p, q;
  p.resize(1); q.resize(1);

  genCirclePolygon(GLvector2f(1000.0f, 1000.0f), 300.0f, mMap, *(p.begin()));
  genCirclePolygon(GLvector2f(1300.0f, 1000.0f), 300.0f, mMap, *(q.begin()));
  c.AddPolygons(p, ptSubject);
  c.AddPolygons(q, ptClip);
  c.Execute(ctUnion, mCMap, pftEvenOdd, pftEvenOdd);

  addCirclePolygon(GLvector2f(1600.0f, 1000.0f), 300.0f);
  addCirclePolygon(GLvector2f(1900.0f, 1000.0f), 300.0f);
  addCirclePolygon(GLvector2f(2200.0f, 1000.0f), 300.0f);
  addCirclePolygon(GLvector2f(2500.0f, 1000.0f), 300.0f);

  addCirclePolygon(GLvector2f(2500.0f, 1300.0f), 300.0f);
  addCirclePolygon(GLvector2f(2500.0f, 1600.0f), 300.0f);
  addCirclePolygon(GLvector2f(2500.0f, 1900.0f), 300.0f);

  addCirclePolygon(GLvector2f(2200.0f, 1900.0f), 300.0f);
  addCirclePolygon(GLvector2f(1900.0f, 1900.0f), 300.0f);
  addCirclePolygon(GLvector2f(1600.0f, 1900.0f), 300.0f);
                              
  addCirclePolygon(GLvector2f(1300.0f, 1900.0f), 300.0f);
  addCirclePolygon(GLvector2f(1300.0f, 1600.0f), 450.0f);

  updateCollision();
}                           

void Map::updateCollision()
{
  Lock(mMutex);
  if(mCollision.size() > 0) {
    GLplane * p = 0;
    foreach(GLplaneList, p, mCollision) {
      delete p;
    }
    mCollision.clear();
  }

  Polygons::iterator pit;
  for(pit = mCMap.begin(); pit != mCMap.end(); ++pit) {
    Polygon p = *pit;
    Polygon::iterator vit;
    for(vit = p.begin(); vit != p.end(); ++vit) {
      IntPoint current = *vit, next;
      if(++vit != p.end()) next = *vit; else next = *p.begin(); vit--;
      
      GLvector2f A(current.X / CLIPPER_PRECISION, current.Y / CLIPPER_PRECISION);
      GLvector2f B(next.X / CLIPPER_PRECISION, next.Y / CLIPPER_PRECISION);
      mCollision.push_back(new GLplane(A, B - A));
    }
  }
  Unlock(mMutex);
}

void Map::genCirclePolygon(GLvector2f pos, GLfloat size, GLtriangleList & triangles, Polygon & polygon, bool random, GLfloat _segments_per_100)
{
  const GLfloat segments_per_100 = _segments_per_100;
  int segments = (int) ((size / 100.0f) * segments_per_100) | 0x01;

  GLvector2f radius(size, 0.0f);
  GLvector2f p, last = pos + radius;

  GLfloat delta = 2.0f * M_PI / segments;
  for(GLfloat angle = 0.0f; angle < (2.0f * M_PI); angle += delta) {
    last = p;
    p = pos + (radius.rotate(angle) * (random?(0.9f + frand()/5):1.0f));
    
    polygon.push_back(IntPoint((long64)(p.x * CLIPPER_PRECISION), (long64)(p.y * CLIPPER_PRECISION)));
    if(angle > 0.0f) {
      triangles.push_back(new GLtriangle(pos, p, last));
    }
  }
  triangles.push_back(new GLtriangle(pos, p, pos + radius));
}

void Map::addCirclePolygon(GLvector2f pos, GLfloat size)
{
  Clipper c;
  Polygons p;
  p.resize(1);
  genCirclePolygon(pos, size, mMap, p[0]);
  c.AddPolygons(mCMap, ptSubject);
  c.AddPolygons(p, ptClip);
  c.Execute(ctUnion, mCMap, pftEvenOdd, pftEvenOdd);
}

void Map::collide()
{
  if(mCollidableCount == 0) return;
  CollidableList toremove;
  bool update = false;

  // handle Projectiles
  Projectile * proj = 0;
  foreach(ProjectileList, proj, mProjectiles) {
    proj->move();
  }

  GLfloat record = 200.0f;

  Lock(mMutex);
  GLplane * p = 0;
  foreach(GLplaneList, p, mCollision) {
    Debug::drawVector(p->base, p->dir, GL_SCREEN_CENTER, GLvector3f(1.0f, 1.0f, 1.0f));

    Collidable * c = 0;
    foreach(CollidableList, c, mCollidables) {
      GLfloat radius = (c->h() > c->w() ? c->h() : c->w()) / 2.0f;
      GLvector2f pos = c->pos();

      if((p->base.x < pos.x - GL_SCREEN_FWIDTH / 2.0f 
      ||  p->base.x > pos.x + GL_SCREEN_FWIDTH / 2.0f)
      && (p->base.y < pos.y - GL_SCREEN_FHEIGHT / 2.0f 
      ||  p->base.y > pos.y + GL_SCREEN_FHEIGHT / 2.0f)) continue;

      GLfloat distance;
      GLfloat planedistance;
      GLvector2f n = GLvector2f((p->dest.y - p->base.y), -(p->dest.x - p->base.x)).normal();
      GLvector2f::crossing(pos, n, p->base, p->dir.normal() * -1.0f, distance, planedistance);

      if(planedistance >= 0.0f && planedistance <= p->dir.len()) {
        if((distance > -radius && distance <= 0.0f) || (distance > 0.0f && distance <= radius)) {
          if(isProjectile(c)) {
            c = c;
          }
          if(!c->collide(n, distance)) {
            toremove.push_back(c);
            if(isProjectile(c)) {
              removeProjectile((Projectile *) c);
              gProjectileCount--;
            }
            continue;
          }
        }
      }
    }

    if(toremove.size() > 0) {
      Collidable * c = 0;
      foreach(CollidableList, c, toremove) {
        mCollidables.remove(c);
      }
      update = true;
    }
  }

  if(record != 200.0f) {
    record = record;
  }

  Unlock(mMutex);
  if(update) updateCollision();
}
