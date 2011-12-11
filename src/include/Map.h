/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _MAP_H_
#define _MAP_H_

#include "GLDefines.h"

#include <CGAL/Exact_predicates_exact_constructions_kernel.h>
#include <CGAL/Boolean_set_operations_2.h>
#include <CGAL/Cartesian.h>
typedef CGAL::Cartesian<GLfloat>            Kernel;
typedef Kernel::Point_2                     Point_2;
typedef CGAL::Polygon_2<Kernel>             Polygon_2;
typedef CGAL::Polygon_with_holes_2<Kernel>  Polygon_with_holes_2;

#include "Collidable.h"
#define MAP_COLLIDABLES_MAX 2048
typedef std::list<Collidable *> CollidableList;

class Map {
public:
  Map();
  ~Map();

  GLplaneList collision() { return mCollision; }

  void lock() { Lock(mMutex); }
  void unlock() { Unlock(mMutex); }

  void draw();
  void collide();
  
  void addCollidable(Collidable * c) { 
    if(mCollidableCount < MAP_COLLIDABLES_MAX) 
      mCollidables[mCollidableCount++] = c; 
  }

  void updateCollision();
  void addCirclePolygon(GLvector2f pos, GLfloat size);

private:
  HANDLE mMutex;

  GLtriangleList mMap;
  GLplaneList mCollision;
  Polygon_with_holes_2 mCGALMap;

  Collidable * mCollidables[MAP_COLLIDABLES_MAX];
  int mCollidableCount;

  // Cache
  int mVertexCacheSize;
  GLfloat ** mVertX;
  GLfloat ** mVertY;
  int * mnVert;

  GLfloat * mTmpVertX;
  GLfloat * mTmpVertY;

  void generate();
  void genCirclePolygon(GLvector2f pos, GLfloat size, GLtriangleList & triangles, Polygon_2 & polygon);
};




#if 0
  const GLfloat targetwidth = 100.0f;
  const GLfloat twdelta = 50.0f;
  const GLfloat targetheight = 500.0f;
  const GLfloat thdelta = 50.0f;
  const GLfloat miny = 10.0f;
  
  int count = 80;

  GLtriangle * triangle = 0;
  GLplane * plane = 0;
  GLvector2f srcA = GLvector2f(_x, _y + targetheight);
  GLvector2f srcB = GLvector2f(_x, _y);
  GLvector2f srcC = GLvector2f(_x + targetwidth, _y);
  GLvector2f srcD = GLvector2f(_x + targetwidth, _y + targetheight);

  gMap = new GLtriangle(srcA, srcB, srcC);
  triangle = gMap->add(srcA, srcC, srcD);

  gMapCollision = new GLplane(srcA, srcB - srcA); // A -> B
  plane = gMapCollision->add(srcD, srcA - srcD);  // A <- D
  plane = plane->add(srcB, srcC - srcB);          // B -> C
  
  for(int i = 0; i < count; i++)
  {
    GLfloat width = targetwidth + (((frand() * 2) - 1) * twdelta);
    GLfloat height = targetheight + (((frand() * 2) - 1) * thdelta);
    GLfloat hdiff = height - targetheight;

    if(srcD.x + width > GL_SCREEN_FWIDTH * GL_SCREEN_FACTOR) 
      break;

    srcA = triangle->triangle[2];
    srcB = triangle->triangle[1];
    srcC = GLvector2f(triangle->triangle[1].x + width, _y - hdiff / 2.0f > miny ? _y - hdiff / 2.0f : miny);
    srcD = GLvector2f(triangle->triangle[2].x + width, _y + targetheight + hdiff / 2.0f);

    triangle = triangle->add(srcA, srcB, srcC);
    triangle = triangle->add(srcA, srcC, srcD);
    plane = plane->add(srcD, srcA - srcD);
    plane = plane->add(srcB, srcC - srcB);
  }
  plane = plane->add(srcC, srcD - srcC);

  // circle time
  //gMapCollision->prev = gMapCollision->tail(NULL);
#endif


#endif
