#ifndef _GLRESOURCES_H_
#define _GLRESOURCES_H_

#ifdef _WIN32
#include <windows.h>
#include <gl\glew.h>
#endif
#include <gl\gl.h>
#include <gl\glu.h>

#define _USE_MATH_DEFINES
#include <math.h>
#include <assert.h>
#include <list>
extern "C" {
#include "glfont.h"
}

#define foreach(T, iter, list)  T::iterator iter ## _ref = list.begin(); \
                                if(iter ## _ref != list.end()) iter = *iter ## _ref; \
                                for(; iter && iter ## _ref != list.end(); iter ## _ref++, iter = (iter ## _ref != list.end()) ? *iter ## _ref : 0)

#define GL_RESOURCE_TEXTURE   0
#define GL_RESOURCE_SOUND     1
#define GL_RESOURCE_FONT      2

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
class GLFontResource : public GLResource {
public:
  GLFontResource(GLFONT _font, GLuint _texture)
    : font(_font), texture(_texture)
    { type = GL_RESOURCE_FONT; }
  GLFONT font;
  GLuint texture;
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
  ResourceList list() { return mResources; }
private:
  ResourceList mResources;
  static bool load(const char * filename, unsigned char ** data, int & width, int & height, unsigned int & size);
};

#endif
