/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _MAP_H_
#define _MAP_H_

#include "GLDefines.h"
#include "clipper.hpp"
using namespace ClipperLib;
#define Polygon ClipperLib::Polygon
#define CLIPPER_PRECISION 100000.0

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

  Polygons mCMap;
  GLtriangleList mMap;
  GLplaneList mCollision;

  Collidable * mCollidables[MAP_COLLIDABLES_MAX];
  int mCollidableCount;

  void generate();
  void genCirclePolygon(GLvector2f pos, GLfloat size, GLtriangleList & triangles, Polygon & polygon);
};

#endif
