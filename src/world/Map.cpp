/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#pragma warning( disable: 4503 )
#include "Map.h"
#include "Debug.h"

#include <algorithm>

Map::Map()
{
  mMutex = CreateMutex(NULL, FALSE, "LeftMapMutex");
  mCollidableMutex = CreateMutex(NULL, FALSE, "LeftMapCollidableMutex");
  mSpot = new GLParticle(1280, 1280, 1.0f, 1.0f, 1.0f, 1.0f, glpLight);

  glGenTextures(1, &mFramebufferTexture);
  glBindTexture(GL_TEXTURE_2D, mFramebufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenFramebuffersEXT(1, &mFramebuffer);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFramebuffer);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mFramebufferTexture, 0);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  
  mCallback = 0;
  mUpdate = true;
  generate();
}

Map::~Map()
{
  CloseHandle(mMutex);
  CloseHandle(mCollidableMutex);

  LightSource * s = 0;
  foreach(LightSourceList, s, mLightSources) {
    delete s;
  }
  MapObject * o = 0;
  foreach(MapObjectList, o, mMapObjects) {
    delete o;
  }

  glDeleteFramebuffersEXT(1, &mFramebuffer);
  glDeleteTextures(1, &mFramebufferTexture);
}

void Map::draw()
{
  MapObject * o = 0;
  foreach(MapObjectList, o, mMapObjects) {
    o->setAlpha(getOpacity(o->pos()));
    o->draw();
  }
}

void Map::drawShadows(GLuint shader, GLint dirloc)
{
  GLfloat radius = GL_SCREEN_FWIDTH;

  Lock(mMutex);
  Polygons copy = mCMap;
  Unlock(mMutex);

  LightSource * s = 0;
  foreach(LightSourceList, s, mLightSources) {
    GLvector2f pos = s->pos;
    if(!s->visible) continue;
    if(pos.x < GL_SCREEN_BOTTOMLEFT.x - GL_SCREEN_FWIDTH / 2.0f ||
       pos.x > GL_SCREEN_BOTTOMLEFT.x + 1.5f * GL_SCREEN_FWIDTH ||
       pos.y < GL_SCREEN_BOTTOMLEFT.y - GL_SCREEN_FHEIGHT / 2.0f ||
       pos.y > GL_SCREEN_BOTTOMLEFT.y + 1.5f * GL_SCREEN_FHEIGHT) {
      continue;
    }

    renderTarget(true);
    GLParticle * spot = s->particle ? s->particle : mSpot;

    spot->setColor((GLvector3f(0.4f, 0.3f, 0.3f) + s->rgb) * s->intensity, 1.0f);
    spot->moveTo(pos.x, pos.y);
    spot->setRotation(pos.x , pos.y, s->angle);
    spot->draw();
    radius = spot->w();

    GLplane * plane = new GLplane();
    Polygons::iterator pit;
    for(pit = copy.begin(); pit != copy.end(); pit++) {
      Polygon p = *pit;
      Polygon::iterator vit;
      for(vit = p.begin(); vit != p.end(); vit++) {
        IntPoint current = *vit, next;
        if(++vit != p.end()) next = *vit; else next = *p.begin(); vit--;
        GLvector2f A(current.X / CLIPPER_PRECISION, current.Y / CLIPPER_PRECISION);
        GLvector2f B(next.X / CLIPPER_PRECISION, next.Y / CLIPPER_PRECISION);

        plane->bordered = true;
        plane->base = A;
        plane->dest = B;
        plane->dir = B - A;

        GLvector2f base = A;
        GLvector2f dest = B;
        GLvector3f scolor = GLvector3f(0.0f, 0.0f, 0.0f);

        GLvector2f baseproj = base - pos;
        GLvector2f destproj = dest - pos;
        GLvector2f bproj = base + baseproj.normal() * (radius);
        GLvector2f dproj = dest + destproj.normal() * (radius);  

        if(baseproj.len() > radius || destproj.len() > radius) continue; 

        base -= GL_SCREEN_BOTTOMLEFT;
        dest -= GL_SCREEN_BOTTOMLEFT;
        bproj -= GL_SCREEN_BOTTOMLEFT;
        dproj -= GL_SCREEN_BOTTOMLEFT;

        glBegin(GL_QUADS);
          glColor4f(scolor.x, scolor.y, scolor.z, 0.0f);
          glVertex3f(base.x, base.y,  0.0f);
          glVertex3f(bproj.x, bproj.y,  0.0f);
          glVertex3f(dproj.x, dproj.y,  0.0f);
          glVertex3f(dest.x, dest.y,  0.0f);
        glEnd();
      }
    }
    
    GLplane * p = 0;
    foreach(GLplaneList, p, mExtraShadows) {
      GLvector2f base = p->base;
      GLvector2f dest = p->dest;
      GLvector3f scolor = GLvector3f(0.0f, 0.0f, 0.0f);

      GLvector2f baseproj = base - pos;
      GLvector2f destproj = dest - pos;
      GLvector2f bproj = base + baseproj.normal() * (radius - baseproj.len());
      GLvector2f dproj = dest + destproj.normal() * (radius - destproj.len());  

      base -= GL_SCREEN_BOTTOMLEFT;
      dest -= GL_SCREEN_BOTTOMLEFT;
      bproj -= GL_SCREEN_BOTTOMLEFT;
      dproj -= GL_SCREEN_BOTTOMLEFT;

      glBegin(GL_QUADS);
        glColor4f(scolor.x, scolor.y, scolor.z, 0.0f);
        glVertex3f(base.x, base.y,  0.0f);
        glVertex3f(bproj.x, bproj.y,  0.0f);
        glVertex3f(dproj.x, dproj.y,  0.0f);
        glVertex3f(dest.x, dest.y,  0.0f);
      glEnd();
    }

// shader
#if 0
    glUseProgram(shader);
    glUniform2f(dirloc, 1.0f, 0.0f);
    glUseProgram(shader);
    glUniform2f(dirloc, 0.0f, 1.0f);
#endif

    glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

    renderTarget(false);
    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glBindTexture(GL_TEXTURE_2D, mFramebufferTexture);
    glBegin(GL_QUADS);
      glTexCoord2f(0.0f, 0.0f); 
      glVertex3f(0.0f, 0.0f,  0.0f);
      glTexCoord2f(1.0f, 0.0f);
      glVertex3f(GL_SCREEN_FWIDTH, 0.0f,  0.0f);
      glTexCoord2f(1.0f, 1.0f); 
      glVertex3f(GL_SCREEN_FWIDTH, GL_SCREEN_FHEIGHT,  0.0f);
      glTexCoord2f(0.0f, 1.0f); 
      glVertex3f(0.0f, GL_SCREEN_FHEIGHT,  0.0f);
    glEnd();
    glBindTexture(GL_TEXTURE_2D, 0);
    glDisable(GL_BLEND);
  }
}

void Map::drawProjectiles()
{
  Lock(mCollidableMutex);
  Projectile * proj = 0;
  foreach(ProjectileList, proj, mProjectiles) {
    proj->draw();
  }
  Unlock(mCollidableMutex);
}

void Map::drawAnimations()
{
  Animation * anim = 0;
  foreach(AnimationList, anim, mAnimations) {
    if(!anim->sprite->draw(anim->frameCount++)) {
      delete anim->sprite;
      delete anim;
      foreach_erase(anim, mAnimations);
    }
  }
}


void Map::generate()
{
  Clipper c;
  Polygons p, q;
  p.resize(1); q.resize(1);

  genCirclePolygon(GLvector2f(1000.0f, 1000.0f), 300.0f, *(p.begin()), true, 30);
  genCirclePolygon(GLvector2f(1300.0f, 1000.0f), 300.0f, *(q.begin()), true, 30);
  c.AddPolygons(p, ptSubject);
  c.AddPolygons(q, ptClip);
  c.Execute(ctUnion, mCMap, pftEvenOdd, pftEvenOdd);

  addCirclePolygon(GLvector2f(1600.0f, 1000.0f), 300.0f, 30);
  addCirclePolygon(GLvector2f(1900.0f, 1000.0f), 300.0f, 30);
  addCirclePolygon(GLvector2f(2200.0f, 1000.0f), 300.0f, 30);
  addCirclePolygon(GLvector2f(2500.0f, 1000.0f), 300.0f, 30);

  addCirclePolygon(GLvector2f(2500.0f, 1300.0f), 300.0f, 30);
  addCirclePolygon(GLvector2f(2500.0f, 1600.0f), 300.0f, 30);
  addCirclePolygon(GLvector2f(2500.0f, 1900.0f), 300.0f, 30);

  addCirclePolygon(GLvector2f(2200.0f, 1900.0f), 300.0f, 30);
  addCirclePolygon(GLvector2f(1900.0f, 1900.0f), 300.0f, 30);
  addCirclePolygon(GLvector2f(1600.0f, 1900.0f), 300.0f, 30);
                              
  addCirclePolygon(GLvector2f(1300.0f, 1900.0f), 300.0f, 30);
  addCirclePolygon(GLvector2f(1300.0f, 1600.0f), 450.0f, 30);
}                           

GLfloat Map::getOpacity(GLvector2f pos)
{
  LightSource * s = 0;
  const GLfloat maxdistance = 1200.0f;
  GLfloat mindistance = maxdistance;
  bool iscone = false;
  bool hitcone = false;

  foreach(LightSourceList, s, mLightSources) {
    GLvector2f delta = (pos - s->pos);
    GLfloat len = delta.len();
    if(s->particle && s->particle->getForm() == glpLightCone) {
      GLfloat angle = (delta.angle() / M_PI) * 180.0f;
      hitcone = (s->angle > angle - 25.0f) && (s->angle < angle + 25.0f);
    } else {
      hitcone = true;
    }
    if(len < mindistance && hitcone) {
      mindistance = len;
    }
  }

  GLfloat alpha = 0.0f;
  if(mindistance <= maxdistance) {
    alpha =  pow(2, -mindistance/200);
  } else {
    alpha = 0.0f;
  }
  if(alpha > 1.0f) alpha = 1.0f; 
  return alpha > GL_ALPHA_CUTOFF ? alpha : 0.0f;
}

void Map::genCirclePolygon(GLvector2f pos, GLfloat size, Polygon & polygon, bool random, GLfloat segments)
{
  GLvector2f radius(size, 0.0f);
  GLvector2f p;

  GLfloat delta = 2.0f * M_PI / segments;
  for(GLfloat angle = 0.0f; angle < (2.0f * M_PI); angle += delta) {
    p = pos + (radius.rotate(angle) * (random?(0.9f + frand()/5):1.0f));
    polygon.push_back(IntPoint((long64)(p.x * CLIPPER_PRECISION), (long64)(p.y * CLIPPER_PRECISION)));
  }
}

void Map::addCirclePolygon(GLvector2f pos, GLfloat size, GLfloat segments)
{
  Clipper c;
  Polygons p;
  p.resize(1);
  genCirclePolygon(pos, size, p[0], true, segments);
  if(mCallback) (*mCallback)(mCallbackUserData, p[0], 0, -1);
  if(!mUpdate) return;

  c.AddPolygons(mCMap, ptSubject);
  c.AddPolygons(p, ptClip);
  Lock(mMutex);
  c.Execute(ctUnion, mCMap, pftEvenOdd, pftEvenOdd);
  Unlock(mMutex);
}

void Map::addPolygon(Polygon & polygon)
{
  Clipper c;
  Polygons p;
  p.push_back(polygon);
  c.AddPolygons(mCMap, ptSubject);
  c.AddPolygons(p, ptClip);
  Lock(mMutex);
  c.Execute(ctUnion, mCMap, pftEvenOdd, pftEvenOdd);
  Unlock(mMutex);
}

void Map::addPolygons(Polygons & p)
{
  Clipper c;
  c.AddPolygons(mCMap, ptSubject);
  c.AddPolygons(p, ptClip);
  Lock(mMutex);
  c.Execute(ctUnion, mCMap, pftEvenOdd, pftEvenOdd);
  Unlock(mMutex);
}

void Map::collideProjectiles()
{
  Projectile * proj = 0;

  Lock(mCollidableMutex);
  foreach(ProjectileList, proj, mProjectiles) {
    proj->move();

    if((proj->maxdistance() > 0.0f) && ((proj->start() - proj->pos()).len() >= proj->maxdistance())) {
      deleteProjectile(proj);
      removeCollidable(proj);
      foreach_erase(proj, mProjectiles);
      continue;
    }

    Collidable * c = 0;
    foreach(CollidableList, c, mCollidables) {
      if(proj == c || isProjectile(c) || c == proj->owner) continue;
      GLfloat distance = (c->pos() - proj->pos()).len();
      if(distance < c->h()) {
        proj->collide(GLvector2f(0.0f, 0.0f), distance);
        if(mCallback) (*mCallback)(mCallbackUserData, Polygon(), c, proj->type);
        deleteProjectile(proj);
        removeCollidable(proj);
        foreach_erase(proj, mProjectiles);
        break;
      }
    }
    if(proj_ref == mProjectiles.end()) break;
  }
  Unlock(mCollidableMutex);
}

void Map::collide(GLplane * p)
{
  if(p->bordered) Debug::drawVector(p->base, p->dir, GL_SCREEN_CENTER, GLvector3f(1.0f, 1.0f, 1.0f));

  Lock(mCollidableMutex);
  Collidable * c = 0;
  foreach(CollidableList, c, mCollidables) {
    GLfloat radius = (c->h() > c->w() ? c->h() : c->w()) / 2.0f;
    GLvector2f pos = c->pos();

    if((p->base.x < pos.x - GL_SCREEN_FWIDTH / 2.0f 
    ||  p->base.x > pos.x + GL_SCREEN_FWIDTH / 2.0f)
    || (p->base.y < pos.y - GL_SCREEN_FHEIGHT / 2.0f 
    ||  p->base.y > pos.y + GL_SCREEN_FHEIGHT / 2.0f)) continue;

    GLfloat distance;
    GLfloat planedistance;
    GLvector2f n = GLvector2f((p->dest.y - p->base.y), -(p->dest.x - p->base.x)).normal();
    GLvector2f::crossing(pos, n, p->base, p->dir.normal() * -1.0f, distance, planedistance);

    if(planedistance >= 0.0f && planedistance <= p->dir.len()) {
      if((distance > -radius && distance <= 0.0f) || (distance > 0.0f && distance <= radius)) {
        if(!c->collide(n, distance)) {
          if(isProjectile(c)) removeProjectile((Projectile *) c);
          foreach_erase(c, mCollidables);
          continue;
        }
      }
    }
  }
  Unlock(mCollidableMutex);
}

void Map::collision()
{
  collideProjectiles();

  Lock(mMutex);
  Polygons copy = mCMap;
  Unlock(mMutex);

  GLplane * plane = new GLplane();
  Polygons::iterator pit;
  for(pit = copy.begin(); pit != copy.end(); pit++) {
    Polygon p = *pit;
    Polygon::iterator vit;
    for(vit = p.begin(); vit != p.end(); vit++) {
      IntPoint current = *vit, next;
      if(++vit != p.end()) next = *vit; else next = *p.begin(); vit--;
      GLvector2f A(current.X / CLIPPER_PRECISION, current.Y / CLIPPER_PRECISION);
      GLvector2f B(next.X / CLIPPER_PRECISION, next.Y / CLIPPER_PRECISION);

      plane->bordered = true;
      plane->base = A;
      plane->dest = B;
      plane->dir = B - A;
      collide(plane);
    }
  }
  MapObject * o = 0;
  foreach(MapObjectList, o, mMapObjects) {
    Polygon p = o->collision();
    Polygon::iterator vit;
    for(vit = p.begin(); vit != p.end(); ++vit) {
      IntPoint current = *vit, next;
      if(++vit != p.end()) next = *vit; else next = *p.begin(); vit--;
      
      GLvector2f center((o->pos().x * CLIPPER_PRECISION) - (o->w() * CLIPPER_PRECISION / 2.0f), (o->pos().y * CLIPPER_PRECISION) - (o->h() * CLIPPER_PRECISION / 2.0f));
      current.X += center.x;
      current.Y += center.y;
      next.X += center.x;
      next.Y += center.y;
      GLvector2f A(current.X / CLIPPER_PRECISION, current.Y / CLIPPER_PRECISION);
      GLvector2f B(next.X / CLIPPER_PRECISION, next.Y / CLIPPER_PRECISION);
      plane->bordered = false;
      plane->base = A;
      plane->dest = B;
      plane->dir = B - A;
      collide(plane);
    }
  }
}

void Map::addCollidable(Collidable * c)
{ 
  Lock(mCollidableMutex);
  mCollidables.push_back(c);
  Unlock(mCollidableMutex);
}

void Map::removeCollidable(Collidable * c)
{
  Lock(mCollidableMutex);
  mCollidables.remove(c);
  Unlock(mCollidableMutex);
}

void Map::addProjectile(Projectile * proj)
{
  Lock(mCollidableMutex);
  mProjectiles.push_back(proj);
  addCollidable((Collidable *) proj);
  Unlock(mCollidableMutex);
}

void Map::removeProjectile(Projectile * proj)
{
  Lock(mCollidableMutex);
  mProjectiles.remove(proj);
  switch(proj->type) {
  case PROJECTILE_TYPE_ROCKET: delete ((RocketProjectile *) proj); break;
  case PROJECTILE_TYPE_SHOTGUN: delete ((ShotgunProjectile *) proj); break;
  case PROJECTILE_TYPE_BFG: delete ((BFGProjectile *) proj); break;
  }
  Unlock(mCollidableMutex);
}

void Map::deleteProjectile(Projectile * proj)
{
  switch(proj->type) {
  case PROJECTILE_TYPE_ROCKET: delete ((RocketProjectile *) proj); break;
  case PROJECTILE_TYPE_SHOTGUN: delete ((ShotgunProjectile *) proj); break;
  case PROJECTILE_TYPE_BFG: delete ((BFGProjectile *) proj); break;
  }
}

bool Map::isProjectile(Collidable * c)
{
  Lock(mCollidableMutex);
  ProjectileList::iterator iter = std::find(mProjectiles.begin(), mProjectiles.end(), c);
  Unlock(mCollidableMutex);
  return iter != mProjectiles.end();
}

void Map::playAnimation(GLAnimatedSprite * sprite)
{
  Animation * anim = new Animation;
  anim->frameCount = 0;
  anim->sprite = sprite;
  mAnimations.push_back(anim);
}
