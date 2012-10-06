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
  
  mMinimap.clear();
  mMinimapZoom = 20.0f;
  mMinimapParticle = new GLParticle(5, 5, 0.0f, 1.0f, 0.0f, 1.0f, glpSolid);
  mMinimapMask.resize(1);
  genCirclePolygon(GLvector2f(0.0f, 0.0f), 2000.0f, mMinimapMask[0], false, 64);
  
  mGaussShader = new GLGaussShader();
  assert(mGaussShader->initialized());

  glGenTextures(1, &mFramebufferTexture);
  glBindTexture(GL_TEXTURE_2D, mFramebufferTexture);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glGenFramebuffersEXT(1, &mFramebuffer);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, mFramebuffer);
  glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, mFramebufferTexture, 0);
  glGenRenderbuffersEXT(1, &mRenderbuffer);
  glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, mRenderbuffer);
  glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT24, GL_SCREEN_IWIDTH, GL_SCREEN_IHEIGHT);
  glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, mRenderbuffer);
  
  GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  assert(status == GL_FRAMEBUFFER_COMPLETE_EXT);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
  glBindTexture(GL_TEXTURE_2D, 0);

  mFramebufferList = glGenLists(1);
  glNewList(mFramebufferList, GL_COMPILE);
  glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
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
  glEndList();

  mCallback = 0;
  mUpdate = true;
  generate();
}

Map::~Map()
{
  CloseMutex(mMutex);
  CloseMutex(mCollidableMutex);

  LightSource * s = 0;
  foreach(LightSourceList, s, mLightSources) {
    delete s;
  }
  MapObject * o = 0;
  foreach(MapObjectList, o, mMapObjects) {
    delete o;
  }
  if(mSpot) {
    delete mSpot;
  }
  if(mMinimapParticle) {
    delete mMinimapParticle;
  }
  if(mGaussShader) {
    delete mGaussShader;
  }

  glDeleteLists(mFramebufferList, 1);
  glDeleteRenderbuffersEXT(1, &mRenderbuffer);
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

void Map::drawMinimap()
{
  updateMinimap();
  GLvector2f center = GLvector2f(GL_SCREEN_FWIDTH - 100.0f, 100.0f);
  mMinimapParticle->moveTo(GL_SCREEN_BOTTOMLEFT.x + center.x, GL_SCREEN_BOTTOMLEFT.y + center.y);
  mMinimapParticle->draw();
  Polygons::iterator pit;
  for(pit = mMinimap.begin(); pit != mMinimap.end(); pit++) {
    Polygon p = *pit;
    Polygon::iterator vit;
    for(vit = p.begin(); vit != p.end(); vit++) {
      IntPoint current = *vit, next;
      if(++vit != p.end()) next = *vit; else next = *p.begin(); vit--;
      GLvector2f A(current.X, current.Y);
      GLvector2f B(next.X, next.Y);
      A = ((A - GL_SCREEN_CENTER) / mMinimapZoom) + center;
      B = ((B - GL_SCREEN_CENTER) / mMinimapZoom) + center;
      glLineWidth(1.0f);
      glBegin(GL_LINES);
        glColor3f(1.0f, 1.0f, 1.0f);
        glVertex2f(A.x, A.y);
        glVertex2f(B.x, B.y);
      glEnd();
    }
  }
}

void Map::drawShadows()
{
  Lock(mMutex);
  Polygons copy = mCMap;
  Unlock(mMutex);

  MapObject * o = 0;
  foreach(MapObjectList, o, mMapObjects) {
    GLvector2f center((o->pos().x) - (o->w() / 2.0f), (o->pos().y) - (o->h() / 2.0f));
    Polygon p;
    Polygon::reverse_iterator vit;
    for(vit = o->collision().rbegin(); vit != o->collision().rend(); vit++) {
      IntPoint current = *vit;
      current.X += center.x;
      current.Y += center.y;
      p.push_back(current);
    }
    copy.push_back(p);
  }
  
  LightSource * s = 0;
  foreach(LightSourceList, s, mLightSources) {
    GLParticle * spot = 0;
    GLvector2f pos = s->pos;
    if(!s->visible) continue;

    if(s->particle) {
      spot = s->particle;
    } else {
      spot = mSpot;
    }

    if(pos.x < GL_SCREEN_BOTTOMLEFT.x - spot->w() / 2.0f ||
       pos.x > GL_SCREEN_BOTTOMLEFT.x + 1.5f * spot->w() ||
       pos.y < GL_SCREEN_BOTTOMLEFT.y - spot->h() / 2.0f ||
       pos.y > GL_SCREEN_BOTTOMLEFT.y + 1.5f * spot->h()) {
      continue;
    }

    renderTarget(true);
    GLfloat radius = sqrt(spot->w() * spot->w() + spot->h() * spot->h());
    GLvector3f spotcolor(0.4f + s->rgb.x, 0.3f + s->rgb.y, 0.3f + s->rgb.z);
    spot->setColor(spotcolor, s->intensity);
    spot->setRotation(pos.x , pos.y, s->angle);
    spot->moveTo(pos.x, pos.y);
    spot->draw();

    Polygons::iterator pit;
    for(pit = copy.begin(); pit != copy.end(); pit++) {
      Polygon p = *pit;
      Polygon::iterator vit;
      for(vit = p.begin(); vit != p.end(); vit++) {
        IntPoint current = *vit, next;
        if(++vit != p.end()) next = *vit; else next = *p.begin(); vit--;
        GLvector2f A(current.X, current.Y);
        GLvector2f B(next.X, next.Y);
        GLvector2f baseproj = A - pos;
        GLvector2f destproj = B - pos;
        GLvector2f bproj = A + baseproj.normal() * (radius);
        GLvector2f dproj = B + destproj.normal() * (radius);  

        if(baseproj.len() > radius && destproj.len() > radius) continue;
        GLvector2f AB = B - A;
        GLplane plane(A, AB); GLvector2f n = plane.n(); if(n.dot(baseproj) <= 0.0f && n.dot(destproj) <= 0.0f) continue;

        A -= GL_SCREEN_BOTTOMLEFT;
        B -= GL_SCREEN_BOTTOMLEFT;
        bproj -= GL_SCREEN_BOTTOMLEFT;
        dproj -= GL_SCREEN_BOTTOMLEFT;

        glBegin(GL_QUADS);
          glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
          glVertex3f(A.x, A.y,  0.0f);
          glVertex3f(bproj.x, bproj.y,  0.0f);
          glVertex3f(dproj.x, dproj.y,  0.0f);
          glVertex3f(B.x, B.y,  0.0f);
        glEnd();
      }
    }
    
    GLplane * p = 0;
    foreach(GLplaneList, p, mExtraShadows) {
      GLvector2f base = p->base;
      GLvector2f dest = p->dest;
      GLvector2f baseproj = p->base - pos;
      GLvector2f destproj = p->dest - pos;
      GLvector2f bproj = p->base + baseproj.normal() * (radius - baseproj.len());
      GLvector2f dproj = p->dest + destproj.normal() * (radius - destproj.len());  

      base -= GL_SCREEN_BOTTOMLEFT;
      dest -= GL_SCREEN_BOTTOMLEFT;
      bproj -= GL_SCREEN_BOTTOMLEFT;
      dproj -= GL_SCREEN_BOTTOMLEFT;

      glBegin(GL_QUADS);
        glColor4f(0.0f, 0.0f, 0.0f, 0.0f);
        glVertex3f(base.x, base.y,  0.0f);
        glVertex3f(bproj.x, bproj.y,  0.0f);
        glVertex3f(dproj.x, dproj.y,  0.0f);
        glVertex3f(dest.x, dest.y,  0.0f);
      glEnd();
    }

    renderTarget(false);

#if 0
    mGaussShader->use();
    mGaussShader->setUniform2f("u_size", GL_SCREEN_FWIDTH, GL_SCREEN_FHEIGHT);
    mGaussShader->setUniform2f("u_direction", 1.0f, 0.0f);;
    
    mGaussShader->use();
    mGaussShader->setUniform2f("u_size", GL_SCREEN_FWIDTH, GL_SCREEN_FHEIGHT);
    mGaussShader->setUniform2f("u_direction", 0.0f, 1.0f);
#endif

    glEnable(GL_BLEND);
    glBlendFunc(GL_ONE, GL_ONE);
    glCallList(mFramebufferList);
    glDisable(GL_BLEND);

#if 0    
    mGaussShader->unuse();
#endif
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

GLvector2f Map::randomSpawn(GLfloat size)
{
  GLvector2f spawn = GLvector2f(frand() * 8000.0f, frand() * 5000.0f);;
  addCirclePolygon(spawn, size);
  return spawn;
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
#ifndef _DEBUG
  for(int shots = 0; shots < 64; shots++)
    addCirclePolygon(GLvector2f(frand() * 8000.0f, frand() * 5000.0f), 350.0f, 30);
#endif
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
    polygon.push_back(IntPoint((long64)(p.x), (long64)(p.y)));
  }
}

void Map::addCirclePolygon(GLvector2f pos, GLfloat size, GLfloat segments)
{
  Clipper c;
  Polygons p;
  p.resize(1);
  genCirclePolygon(pos, size, p[0], true, segments);
  if(mCallback) (*mCallback)(mCallbackUserData, p[0], 0, 0);
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

void Map::updateMinimap()
{
  Clipper c;
  Polygons p;
  p.resize(1);
#if 0
  IntPoint base(GL_SCREEN_CENTER.x - 2000, GL_SCREEN_CENTER.y - 2000);
  p[0].push_back(IntPoint(base.X      , base.Y));
  p[0].push_back(IntPoint(base.X + 4000, base.Y));
  p[0].push_back(IntPoint(base.X + 4000, base.Y + 4000));
  p[0].push_back(IntPoint(base.X      , base.Y + 4000));
#else
  Polygon::iterator vit;
  for(vit = mMinimapMask[0].begin(); vit != mMinimapMask[0].end(); vit++) {
    IntPoint current = *vit;
    current.X += GL_SCREEN_CENTER.x;
    current.Y += GL_SCREEN_CENTER.y;
    p[0].push_back(current);
  }
#endif
  c.AddPolygons(mCMap, ptSubject);
  c.AddPolygons(p, ptClip);
  c.Execute(ctIntersection, mMinimap, pftEvenOdd, pftEvenOdd);
}

void Map::collideProjectiles()
{
  Projectile * proj = 0;

  Lock(mCollidableMutex);
  foreach(ProjectileList, proj, mProjectiles) {
    proj->move();

    if((proj->maxdistance() > 0.0f) && ((proj->start() - proj->pos()).len() >= proj->maxdistance())) {
      deleteProjectile(proj);
      removeCollidable(proj, true);
      foreach_erase(proj, mProjectiles);
      continue;
    }

    Collidable * c = 0;
    foreach(CollidableList, c, mCollidables) {
      if(proj == c || isProjectile(c, true) || c == proj->owner) continue;
      GLfloat distance = (c->pos() - proj->pos()).len();
      if(distance < c->h()) {
        proj->collide(GLvector2f(0.0f, 0.0f), distance);
        Polygon polygon;
        if(mCallback) (*mCallback)(mCallbackUserData, polygon, c, proj);
        deleteProjectile(proj);
        removeCollidable(proj, true);
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
  if(p->bordered)  {
    GLvector2f base = p->base - GL_SCREEN_BOTTOMLEFT;
    GLvector2f dest = p->dest - GL_SCREEN_BOTTOMLEFT;
    glLineWidth(2.0f);
    glBegin(GL_LINES);
      glColor3f(0.1f, 0.05f, 0.05f);
      glVertex2f(base.x, base.y);
      glVertex2f(dest.x, dest.y);
    glEnd();
  }

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
    GLvector2f n = p->n().normal();
    GLvector2f::intersect(pos, n, p->base, p->dir.normal() * -1.0f, distance, planedistance);
    Debug::drawVector(p->base + p->dir * 0.5f, n * 20.0f, pos, GLvector3f(1.0f, 0.0f, 0.0f));
    if(planedistance >= 0.0f && planedistance <= p->dir.len()) {
      if((distance > -radius && distance <= 0.0f) || (distance > 0.0f && distance <= radius)) {
        if(!c->collide(n, distance)) {
          if(isProjectile(c, true)) {
            removeProjectile((Projectile *) c, true);
          }
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
      GLvector2f A(current.X, current.Y);
      GLvector2f B(next.X, next.Y);

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
      
      GLvector2f center((o->pos().x) - (o->w() / 2.0f), (o->pos().y) - (o->h() / 2.0f));
      current.X += center.x;
      current.Y += center.y;
      next.X += center.x;
      next.Y += center.y;
      GLvector2f A(current.X, current.Y);
      GLvector2f B(next.X, next.Y);
      plane->bordered = false;
      plane->base = A;
      plane->dest = B;
      plane->dir = B - A;
      collide(plane);
    }
  }
  delete plane;
}

void Map::addCollidable(Collidable * c, bool locked)
{ 
  if(!locked) Lock(mCollidableMutex);
  mCollidables.push_back(c);
  if(!locked) Unlock(mCollidableMutex);
}

void Map::removeCollidable(Collidable * c, bool locked)
{
  if(!locked) Lock(mCollidableMutex);
  mCollidables.remove(c);
  if(!locked) Unlock(mCollidableMutex);
}

void Map::addProjectile(Projectile * proj, bool locked)
{
  if(!locked) Lock(mCollidableMutex);
  mProjectiles.push_back(proj);
  addCollidable((Collidable *) proj, true);
  if(!locked) Unlock(mCollidableMutex);
}

void Map::removeProjectile(Projectile * proj, bool locked)
{
  if(!locked) Lock(mCollidableMutex);
  mProjectiles.remove(proj);
  deleteProjectile(proj);
  if(!locked) Unlock(mCollidableMutex);
}

void Map::deleteProjectile(Projectile * proj)
{
  switch(proj->type) {
  case PROJECTILE_TYPE_ROCKET: delete ((RocketProjectile *) proj); break;
  case PROJECTILE_TYPE_SHOTGUN: delete ((ShotgunProjectile *) proj); break;
  case PROJECTILE_TYPE_BFG: delete ((BFGProjectile *) proj); break;
  case PROJECTILE_TYPE_GRENADE: delete ((GrenadeProjectile *) proj); break;
  case PROJECTILE_TYPE_NAIL: delete ((NailProjectile *) proj); break;
  }
}

bool Map::isProjectile(Collidable * c, bool locked)
{
  if(!locked) Lock(mCollidableMutex);
  ProjectileList::iterator iter = std::find(mProjectiles.begin(), mProjectiles.end(), c);
  if(!locked) Unlock(mCollidableMutex);
  return iter != mProjectiles.end();
}

void Map::playAnimation(GLAnimatedSprite * sprite)
{
  Animation * anim = new Animation;
  anim->frameCount = 0;
  anim->sprite = sprite;
  mAnimations.push_back(anim);
}
