#ifndef _GLRESOURCES_H_
#define _GLRESOURCES_H_

#ifdef _WIN32
#include <windows.h>
#endif
#include <gl\glew.h>
#include <gl\gl.h>
#include <gl\glu.h>
#include <assert.h>
#define _USE_MATH_DEFINES
#include <math.h>
#include <list>

class GLResource { public: unsigned int type; };
#define GL_RESOURCE_TEXTURE 0

class GLTextureResource : public GLResource {
public:
  GLTextureResource(GLuint _texture, GLfloat _width, GLfloat _height)
    : texture(_texture), width(_width), height(_height) { type = GL_RESOURCE_TEXTURE; }
  
  GLuint texture;
  GLfloat width;
  GLfloat height;
};

typedef struct {
  char path[256];
  GLResource * value;
} ResourcePair;
typedef std::list<ResourcePair *> ResourceList;

class GLResources {
public:
  GLResources() { }
  ~GLResources() { }

  void init();
  void clear();

  GLResource * get(const char * path);
private:
  ResourceList mResources;  
  static bool load(const char * filename, unsigned char ** data, int & width, int & height, unsigned int & size);
};

#endif
