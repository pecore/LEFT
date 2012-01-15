/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#pragma warning( disable: 4503 )
#include "Map.h"
#include "Debug.h"

Map::Map()
{
  mMutex = CreateMutex(NULL, FALSE, "LeftMapMutex");
  mSpot = new GLParticle(GL_SCREEN_IWIDTH, GL_SCREEN_IWIDTH, 1.0f, 1.0f, 1.0f, 1.0f, glpLight);

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
  
  generate();
}

Map::~Map()
{
  CloseHandle(mMutex);

  GLplane * p = 0;
  foreach(GLplaneList, p, mCollision) {
    delete p;
  }
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

void Map::drawShadows()
{
  GLplane * p;
  GLfloat radius = 1280.0f;

  Lock(mMutex);
  LightSource * s = 0;
  foreach(LightSourceList, s, mLightSources) {
    GLvector2f pos = s->pos;
    if(pos.x < GL_SCREEN_BOTTOMLEFT.x - GL_SCREEN_FWIDTH / 2.0f ||
       pos.x > GL_SCREEN_BOTTOMLEFT.x + GL_SCREEN_FWIDTH ||
       pos.y < GL_SCREEN_BOTTOMLEFT.y - GL_SCREEN_FHEIGHT / 2.0f ||
       pos.y > GL_SCREEN_BOTTOMLEFT.y + GL_SCREEN_FHEIGHT) {
      continue;
    }

    renderTarget(true);
    
    GLParticle * spot = s->particle;
    if(spot) {
      spot->setRotation(spot->pos().x, spot->pos().y, s->angle);
      spot->setScale(0.3);
    }
    else spot = mSpot;

    spot->setColor((GLvector3f(0.4f, 0.3f, 0.3f) + s->rgb) * s->intensity, 1.0f);
    spot->moveTo(pos.x, pos.y);
    spot->draw();

    {
    foreach(GLplaneList, p, mCollision) {
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
    }
    {
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
    }
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
  
  Unlock(mMutex);
}

void Map::drawProjectiles()
{
  Projectile * proj = 0;
  foreach(ProjectileList, proj, mProjectiles) {
    proj->draw();
  }
}

void Map::drawAnimations()
{
  AnimationList toerase;
  Animation * anim = 0;
  foreach(AnimationList, anim, mAnimations) {
    if(!anim->sprite->draw(anim->frameCount++)) {
      toerase.push_back(anim);
    }
  }
  if(toerase.size() > 0) {
    foreach(AnimationList, anim, toerase) {
      mAnimations.remove(anim);
      delete anim->sprite;
      delete anim;
    }
  }
}


void Map::generate()
{
  Clipper c;
  Polygons p, q;
  p.resize(1); q.resize(1);

  genCirclePolygon(GLvector2f(1000.0f, 1000.0f), 300.0f, *(p.begin()));
  genCirclePolygon(GLvector2f(1300.0f, 1000.0f), 300.0f, *(q.begin()));
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

GLfloat Map::getOpacity(GLvector2f pos)
{
  LightSource * s = 0;
  const GLfloat maxdistance = 1200.0f;
  GLfloat mindistance = maxdistance;
  
  foreach(LightSourceList, s, mLightSources) {
    if((pos - s->pos).len() < mindistance) {
      mindistance = (pos - s->pos).len();
    }
  }

  GLfloat alpha = 0.0f;
  if(mindistance <= maxdistance) {
    alpha = 5000.0f / (mindistance * mindistance);
  } else {
    alpha = 0.0f;
  }
  if(alpha > 1.0f) alpha = 1.0f; 
  return alpha;
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
      mCollision.push_back(new GLplane(A, B - A));
    }
  }
  Unlock(mMutex);
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
  c.AddPolygons(mCMap, ptSubject);
  c.AddPolygons(p, ptClip);
  c.Execute(ctUnion, mCMap, pftEvenOdd, pftEvenOdd);
}

void Map::collide()
{
  if(mCollidables.size() == 0) return;
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

      Projectile * proj = isProjectile(c) ? (Projectile *) c : 0;
      if(proj && (proj->maxdistance() > 0.0f) && ((proj->start() - proj->pos()).len() >= proj->maxdistance())) {
        toremove.push_back(c);
        removeProjectile(proj);
      }

      GLfloat distance;
      GLfloat planedistance;
      GLvector2f n = GLvector2f((p->dest.y - p->base.y), -(p->dest.x - p->base.x)).normal();
      GLvector2f::crossing(pos, n, p->base, p->dir.normal() * -1.0f, distance, planedistance);

      if(planedistance >= 0.0f && planedistance <= p->dir.len()) {
        if((distance > -radius && distance <= 0.0f) || (distance > 0.0f && distance <= radius)) {
          if(!c->collide(n, distance)) {
            toremove.push_back(c);
            if(proj) removeProjectile(proj);
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
  if(true) updateCollision();
}

void Map::addCollidable(Collidable * c)
{ 
  mCollidables.push_back(c);
}

void Map::addProjectile(Projectile * proj)
{
  mProjectiles.push_back(proj);
  addCollidable((Collidable *) proj);
}

void Map::removeProjectile(Projectile * proj)
{
  mProjectiles.remove(proj);
  switch(proj->type) {
  case PROJECTILE_TYPE_ROCKET: delete ((RocketProjectile *) proj); break;
  case PROJECTILE_TYPE_SHOTGUN: delete ((ShotgunProjectile *) proj); break;
  case PROJECTILE_TYPE_BFG: delete ((BFGProjectile *) proj); break;
  }
}

bool Map::isProjectile(Collidable * c)
{
  Projectile * proj = 0;
  foreach(ProjectileList, proj, mProjectiles) {
    if(proj == c) {
      return true;
    }
  }
  return false;
}

void Map::playAnimation(GLAnimatedSprite * sprite)
{
  Animation * anim = new Animation;
  anim->frameCount = 0;
  anim->sprite = sprite;
  mAnimations.push_back(anim);
}
