/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

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
  genCircleSegment(GLvector2f(1000.0f, 1000.0f), 300.0f, mMap, mCollision);
  addCircleSegment(GLvector2f(1300.0f, 1000.0f), 300.0f);
  addCircleSegment(GLvector2f(1600.0f, 1000.0f), 300.0f);
  addCircleSegment(GLvector2f(1900.0f, 1000.0f), 300.0f);
  addCircleSegment(GLvector2f(2200.0f, 1000.0f), 300.0f);
  addCircleSegment(GLvector2f(2500.0f, 1000.0f), 300.0f);

  addCircleSegment(GLvector2f(2500.0f, 1300.0f), 300.0f);
  addCircleSegment(GLvector2f(2500.0f, 1600.0f), 300.0f);
  addCircleSegment(GLvector2f(2500.0f, 1900.0f), 300.0f);

  addCircleSegment(GLvector2f(2200.0f, 1900.0f), 300.0f);
  addCircleSegment(GLvector2f(1900.0f, 1900.0f), 300.0f);
  addCircleSegment(GLvector2f(1600.0f, 1900.0f), 300.0f);
  
  addCircleSegment(GLvector2f(1300.0f, 1900.0f), 300.0f);
  addCircleSegment(GLvector2f(1300.0f, 1600.0f), 450.0f);
}

void Map::genCircleSegment(GLvector2f pos, GLfloat size, GLtriangleList & triangles, GLplaneList & planes)
{
  const int segments = 24;

  GLvector2f radius(size, 0.0f);
  GLvector2f lastradius = radius;
  GLvector2f A, B, CB;

  for(GLfloat angle = 2.0f * M_PI / segments; angle < 2.0f * M_PI; angle += 2.0f * M_PI / segments) {
    CB = radius.rotate(angle) * (0.9f + frand()/5);
    A = pos + lastradius;
    B = pos + CB;
    lastradius = CB;

    triangles.push_back(new GLtriangle(A, B, pos));
    planes.push_back(new GLplane(A, B - A));
  }
  triangles.push_back(new GLtriangle(B, pos + radius, pos));
  planes.push_back(new GLplane(B, (pos + radius) - B));
}

void Map::makePolygons(GLplaneList segment, int *nvert, float **vertx, float **verty, int & n)
{
  int count = 0, sumcount = 0;
  int max = n;
  GLplane * start = *segment.begin();

  n = 0;
  vertx[n][count] = start->base.x;
  verty[n][count++] = start->base.y;
  vertx[n][count] = start->dest.x;
  verty[n][count++] = start->dest.y;

  while(sumcount < segment.size()) {
    GLplane * p;
    bool found;
    if(!count) return;

    found = false;
    foreach(GLplaneList, p, segment) {
      if(p == start) continue;
      bool contains = false;
      for(int i = 0; count > 1 && i < count; i++) {
        if((p->dest.x == vertx[n][i] && p->dest.y == verty[n][i])) {
          contains = true;
        }
      } if(contains) continue;

      if(p->base.x == vertx[n][count - 1] && p->base.y == verty[n][count - 1]) {
        vertx[n][count] = p->dest.x;
        verty[n][count++] = p->dest.y;
        found = true;
      }
    }

    if(!found) {
      if(!count) return;
      nvert[n] = count;
      sumcount += count;
      count = 0;

      foreach(GLplaneList, p, segment) {
        bool contains = false;

        for(int j = 0; j < n+1; j++) {
          for(int i = 0; i < nvert[j]; i++) {
            if((p->dest.x == vertx[j][i] && p->dest.y == verty[j][i])) {
              contains = true;
            }
          }
        }

        if(!contains) {
          start = p;
          vertx[n+1][count] = start->base.x;
          verty[n+1][count++] = start->base.y;
          vertx[n+1][count] = start->dest.x;
          verty[n+1][count++] = start->dest.y;
          break;
        }
      }
      n++;
    }

    if(n >= max - 1) {
      return;
    }
  }
  return;
}

void Map::addCircleSegment(GLvector2f pos, GLfloat size)
{
  GLplaneList segment;
  GLplaneList segmentcopy;
  genCircleSegment(pos, size, mMap, segment);
  
  {
    GLplane * p = 0;
    foreach(GLplaneList, p, segment) {
      segmentcopy.push_back(new GLplane(*p));
    }
  }

  GLfloat ** vertx = new GLfloat*[256], * combinedx;
  GLfloat ** verty = new GLfloat*[256], * combinedy;
  GLfloat tmpx[1024], tmpy[1024];
  int ntmp = 0, ncombined = 0;

  for(int i = 0; i < 256; i++) {
    vertx[i] = new GLfloat[mCollision.size() * 2];
    verty[i] = new GLfloat[mCollision.size() * 2];
  }
  
  int nvert[256];
  int n = 256, lastn = 0;
  int ncross;

  Lock(mMutex);
  {
    makePolygons(mCollision, nvert, vertx, verty, n);
  
    GLplane * p;    
    GLplane * cross[16];
    GLfloat ca, cb;
    bool bbase = false;
    bool bdest = false;

    GLplaneList::iterator iter = segment.begin();
    while(iter != segment.end()) {
      p = *iter;
      
      bbase = false;
      bdest = false;
      if(!ntmp || lastn != n) {
        lastn = n;
        if(n > 1) {
          ncombined = 0;
          for(int i = 0; i < n; i++) {
            tmpx[ntmp] = 0;
            tmpy[ntmp++] = 0;
            for(int j = 0; j < nvert[i]; j++) {
              tmpx[ntmp] = vertx[i][j];
              tmpy[ntmp++] = verty[i][j];
            }
            tmpx[ntmp] = vertx[i][0];
            tmpy[ntmp++] = verty[i][0];
          }
          tmpx[ntmp] = 0;
          tmpy[ntmp++] = 0;
          ncombined = ntmp;
          combinedx = tmpx;
          combinedy = tmpy;
        } else {
          ncombined = nvert[0];
          combinedx = vertx[0];
          combinedy = verty[0];
        }
      }

      bbase = false;
      bdest = false;

      if(isPlaneInsideOf(p, mCollision, false, ncombined, combinedx, combinedy, ncross, cross, bbase, bdest, ca, cb)) {
        if(ncross == 2) {
          mCollision.remove(cross[0]);
          mCollision.remove(cross[1]);
          Debug::DebugVectors.push_back(new GLplane(cross[0]->base, GLvector2f(0.0f, 50.0f)));
          Debug::DebugVectors.push_back(new GLplane(cross[1]->dest, GLvector2f(0.0f, 50.0f)));

          // FIXME
          mCollision.push_back(new GLplane(cross[0]->base, cross[1]->dest - cross[0]->base));
          //GLvector2f c = p->base + p->dir.normal() * ca;
          //mMap.push_back(new GLtriangle((bbase ? b : p)->base, (bbase ? p : b)->dest, c));
          delete cross[0];
          delete cross[1];
        } else if(ncross) {
          ncross = ncross;
        }
        iter = segment.erase(iter);
        delete p;
      } else if(ncross) {
        GLplane * b = cross[0];
        mCollision.remove(b);
        mCollision.push_back(new GLplane((bbase ? b : p)->base, (bbase ? p : b)->dest - (bbase ? b : p)->base));

        GLvector2f c = p->base + p->dir.normal() * ca;
        mMap.push_back(new GLtriangle((bbase ? b : p)->base, (bbase ? p : b)->dest, c));
        iter = segment.erase(iter);
        delete p;
        delete b;
      } else {
        iter++;
      }
    }
  }

  n = 256;
  makePolygons(segmentcopy, nvert, vertx, verty, n);
  ncombined = nvert[0];
  combinedx = vertx[0];
  combinedy = verty[0];

  {
    GLplane * p;    
    GLplane * cross[16];
    GLfloat ca, cb;
    bool bbase = false;
    bool bdest = false;

    GLplaneList::iterator iter = mCollision.begin();
    while(iter != mCollision.end()) {
      GLplane * p = *iter;

      if(isPlaneInsideOf(p, segmentcopy, false, ncombined, combinedx, combinedy, ncross, cross, bbase, bdest, ca, cb)) {
         iter = mCollision.erase(iter);
         delete p;
      } else {
        iter++;
      }
    }
  }
  Unlock(mMutex);
  mCollision.splice(mCollision.end(), segment);

  for(int i = 0; i < 256; i++) {
    delete vertx[i];
    delete verty[i];
  }
  delete[] vertx;
  delete[] verty;

  {
    GLplane * p = 0;
    foreach(GLplaneList, p, segmentcopy) {
      delete p;
    }
  }

  
}

void Map::collide()
{
  if(mCollidableCount == 0) return;

  Lock(mMutex);
  GLplane * p = 0;
  foreach(GLplaneList, p, mCollision) {
    Debug::drawVector(p->base, p->dir, mCollidables[0]->pos());

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

int pnpoly(int nvert, float *vertx, float *verty, float testx, float testy)
{
  int i, j, c = 0;
  for (i = 0, j = nvert-1; i < nvert; j = i++) {
    if ( ((verty[i]>testy) != (verty[j]>testy)) && (testx < (vertx[j]-vertx[i]) * (testy-verty[i]) / (verty[j]-verty[i]) + vertx[i]) )
       c = !c;
  }
  return c;
}

bool Map::isPlaneInsideOf(GLplane * plane, GLplaneList segment, bool inside, int nvert, GLfloat * vertx, GLfloat * verty, int & ncross, GLplane ** crossplane, bool & bbase, bool & bdest, GLfloat & ca, GLfloat & cb)
{
  bbase = pnpoly(nvert, vertx, verty, plane->base.x, plane->base.y);
  bdest = pnpoly(nvert, vertx, verty, plane->dest.x, plane->dest.y);
  ncross = 0;

  if(bbase || bdest) {
    GLplane * p;
    foreach(GLplaneList, p, segment) {
      GLfloat cplane, cp;
      GLfloat planedistance;

      GLvector2f::crossing(plane->base, plane->dir.normal(), p->base, p->dir.normal() * -1.0f, cplane, cp);
      if(cplane >= 0.0f && cplane <= plane->dir.len() && cp >= 0.0f && cp <= p->dir.len()) {
        crossplane[ncross++] = p;
        ca = cplane;
        cb = cp;
      }
    }
  }

  return bbase && bdest;
}