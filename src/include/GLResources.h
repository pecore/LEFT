/*
    Copyright (c) 2011   LEFT PROJECT
    All rights reserved.

    file authors:
    Jan Christian Meyer
*/

#ifndef _GLRESOURCES_H_
#define _GLRESOURCES_H_

#ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#endif
#ifndef _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#endif

#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
#include <gl\glew.h>
#include <gl\wglew.h>
#endif
#include <gl\gl.h>
#include <gl\glu.h>
#include "clipper.hpp"
using namespace ClipperLib;
#define Polygon ClipperLib::Polygon
#define CLIPPER_PRECISION 100000L

#include <stdarg.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <fstream>
#include <list>
#include <deque>

#define foreach(T, iter, list)    T::iterator iter ## _ref = list.begin(); \
                                  if(iter ## _ref != list.end()) iter = *iter ## _ref; \
                                  for(; iter && iter ## _ref != list.end(); iter ## _ref++, iter = (iter ## _ref != list.end()) ? *iter ## _ref : 0)

#define foreach_erase(iter, list) iter ## _ref = list.erase(iter ## _ref); \
                                  if(iter ## _ref == list.end()) break;

#define GL_RESOURCE_TEXTURE   0
#define GL_RESOURCE_SOUND     1
#define GL_RESOURCE_FONT      2
#define GL_RESOURCE_POLYGON   3

class GLResource { public: unsigned int type; };
class GLTextureResource : public GLResource {
public:
  GLTextureResource(GLuint _texture, GLfloat _width, GLfloat _height)
    : texture(_texture), width(_width), height(_height) 
    { type = GL_RESOURCE_TEXTURE; }
  GLuint texture;
  GLfloat width;
  GLfloat height;
};
struct Sound;
class GLSoundResource : public GLResource {
public:
  GLSoundResource(Sound * _sound)
    : sound(_sound)
    { type = GL_RESOURCE_SOUND; }
  Sound * sound;
};
struct S_bm_font;
class GLFontResource : public GLResource {
public:
  GLFontResource(struct S_bm_font * _font)
    : font(_font)
    { type = GL_RESOURCE_FONT; }
  struct S_bm_font * font;
};
class GLPolygonResource : public GLResource {
public:
  GLPolygonResource(Polygons _polygons)
    : polygons(_polygons)
    { type = GL_RESOURCE_POLYGON; }
  Polygons polygons;
};

typedef struct {
  char path[256];
  GLResource * value;
} ResourcePair;
typedef std::list<ResourcePair *> ResourceList;

class GLResources {
public:
  GLResources();
  ~GLResources();
  GLResource * get(const char * path);
  GLSoundResource * getSound(const char * path) { return (GLSoundResource *) get(path); }
  GLTextureResource * getTexture(const char * path) { return (GLTextureResource *) get(path); }
  GLPolygonResource * getPolygon(const char * path) { return (GLPolygonResource *) get(path); }
  GLFontResource * getFont(const char * path) { return (GLFontResource *) get(path); }

  ResourceList list() { return mResources; }

  GLuint loadTexture(const char * filename);
  static bool load(const char * filename, unsigned char ** data, int & width, int & height, unsigned int & size);
private:
  ResourceList mResources;
};

#endif
