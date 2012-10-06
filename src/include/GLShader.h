#ifndef _GL_SHADER_H
#define _GL_SHADER_H

#include "GLDefines.h"
#include <map>
#include <list>
#include <string>

class GLShader {
public:
  virtual ~GLShader();

  bool initialized() { return mInitialized; } 
  bool compile();
  
  void use()
  {
    //glUseProgram(mProgram);
  }
  void unuse()
  {
    //glUseProgram(0);
  }

  void setUniform1d(std::string name, GLint a) 
  {
    //glUniform1d(mUniformLocations[name], a);
  }
  void setUniform1f(std::string name, GLfloat a) 
  {
    //glUniform1f(mUniformLocations[name], a);
  }
  void setUniform2f(std::string name, GLfloat a, GLfloat b) 
  {
    //glUniform2f(mUniformLocations[name], a, b);
  }

protected:
  bool mInitialized;
  const char * mSource;
  unsigned int mType;
  GLuint mProgram;
  std::map<std::string, GLint> mUniformLocations;
};

class GLGaussShader : public GLShader {
public:
  GLGaussShader();
  ~GLGaussShader();
};

#endif
