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
}

void Map::draw()
{
  Lock(mMutex);

  glBegin(GL_TRIANGLES);
  GLtriangle * t = 0;
  foreach(GLtriangleList, t, mMap) {
    glColor3f(0.4f, 0.3f, 0.3f);
    glVertex3f(t->A.x, t->A.y, 0.0f);
    glVertex3f(t->B.x, t->B.y, 0.0f);
    glVertex3f(t->C.x, t->C.y, 0.0f);
  }
  glColor3f(1.0f, 1.0f, 1.0f);
  glEnd();

  Unlock(mMutex);
}

void Map::generate()
{
  Polygon_2 p, q;
  genCirclePolygon(GLvector2f(1000.0f, 1000.0f), 300.0f, mMap, p);
  genCirclePolygon(GLvector2f(1300.0f, 1000.0f), 300.0f, mMap, q);
  CGAL::join(p, q, mCGALMap);

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

  if(!mCGALMap.is_unbounded()) {
    Polygon_2 outer = mCGALMap.outer_boundary();
    Polygon_2::Vertex_const_iterator vit;
    for(vit = outer.vertices_begin(); vit != outer.vertices_end(); ++vit) {
      Point_2 current = *vit, next;
      if(++vit != outer.vertices_end())
      next = *vit; else next = *outer.vertices_begin(); vit--;
      
      GLvector2f A(current.x(), current.y());
      GLvector2f B(next.x(), next.y());
      mCollision.push_back(new GLplane(A, B - A));
    }
  }

  if(mCGALMap.number_of_holes() > 0) {
    Polygon_with_holes_2::Hole_const_iterator hit;
    for(hit = mCGALMap.holes_begin(); hit != mCGALMap.holes_end(); ++hit) {
      Polygon_2 p = *hit;

      Polygon_2::Vertex_const_iterator vit;
      for(vit = p.vertices_begin(); vit != p.vertices_end(); ++vit) {
        Point_2 current = *vit, next;
        if(++vit != p.vertices_end())
        next = *vit; else next = *p.vertices_begin(); vit--;
        
        GLvector2f A(current.x(), current.y());
        GLvector2f B(next.x(), next.y());
        mCollision.push_back(new GLplane(A, B - A));
      }
    }
  }
  Unlock(mMutex);
}

void Map::genCirclePolygon(GLvector2f pos, GLfloat size, GLtriangleList & triangles, Polygon_2 & polygon)
{
  const GLfloat segments_per_100 = 8;
  int segments = (int) ((size / 100.0f) * segments_per_100);

  GLvector2f radius(size, 0.0f);
  GLvector2f p, last = pos + radius;

  GLfloat delta = 2.0f * M_PI / segments;
  for(GLfloat angle = 0.0f; angle < 2.0f * M_PI - delta; angle += delta) {
    last = p;
    p = pos + (radius.rotate(angle) * (0.9f + frand()/5));
    
    polygon.push_back(Point_2(p.x, p.y));
    if(angle > 0.0f) {
      triangles.push_back(new GLtriangle(pos, p, last));
    }
  }
  triangles.push_back(new GLtriangle(pos, p, pos + radius));
}

void Map::addCirclePolygon(GLvector2f pos, GLfloat size)
{
  Polygon_2 p;
  genCirclePolygon(pos, size, mMap, p);
  CGAL::join(mCGALMap, p, mCGALMap);
}

void Map::collide()
{
  if(mCollidableCount == 0) return;

  Lock(mMutex);
  GLplane * p = 0;
  foreach(GLplaneList, p, mCollision) {
    Debug::drawVector(p->base, p->dir, mCollidables[0]->pos(), GLvector3f(0.0f, 1.0f, 1.0f));

    for(int i = 0; i < mCollidableCount; i++) {  
      GLfloat radius = (mCollidables[i]->h() > mCollidables[i]->w() ? mCollidables[i]->h() : mCollidables[i]->w()) / 2.0f;
      GLvector2f pos = mCollidables[i]->pos();

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
          mCollidables[i]->collide(n, distance);
        }
      }
    }
  }
  Unlock(mMutex);
}
