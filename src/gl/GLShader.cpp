#include "GLShader.h"

#include "Debug.h"

GLShader::~GLShader()
{
  glDeleteProgram(mProgram);
}

bool GLShader::compile()
{
  bool result = true;
  GLuint shader;

  if(result) {
    GLint length = strlen(mSource);
    shader = glCreateShader(mType);
    glShaderSource(shader, 1, (const GLchar **) &mSource, &length);
    glCompileShader(shader);

    GLint compiled;
    glGetObjectParameterivARB(shader, GL_COMPILE_STATUS, &compiled);
    if(!compiled) {
      GLchar* log = new GLchar[65636];
      glGetProgramInfoLog(shader, 65636, NULL, log);
      Debug::LogToFile("glsl_compiler_errors.txt", "%s", log);
      result = false;
      delete[] log;
    }
  }

  if(result) {
    mProgram = glCreateProgram();
    glAttachShader(mProgram, shader);
    glLinkProgram(mProgram);    

    GLint linked;
    glGetProgramiv(mProgram, GL_LINK_STATUS, &linked);
    if(!linked) {
      Debug::Log("Could not link, saving log.");
      GLchar* log = new GLchar[65636];
      glGetProgramInfoLog(mProgram, 65636, NULL, log);
      Debug::LogToFile("glsl_linker_errors.txt", "%s", log);
      result = false;
      delete[] log;
    }
    glDeleteShader(shader);
  }

  return result;
}

static const char * GaussShaderSource =
  "#version 120\n"
  "uniform sampler2D tex;"
  "uniform vec2 u_direction;"
  "uniform vec2 u_size;"

  "const vec2 gaussFilter[7] = const vec2[7]("
  "  -3.0,	0.015625,"
  "  -2.0,	0.09375 ,"
  "  -1.0,	0.234375,"
  "   0.0,	0.3125  ,"
  "   1.0,	0.234375,"
  "   2.0,	0.09375 ,"
  "   3.0,	0.015625"
  ");"

  "void main()"
  "{"
  "  vec4 color;"
  "  color = vec4(0.0, 0.0, 0.0, 0.0);"
  "  for(int i = 0; i < 7; i++) {"
  "    color += texture2D( tex, vec2( gl_TexCoord[0].x + gaussFilter[i].x / u_size.x * u_direction.x,"
  "                                   gl_TexCoord[0].y + gaussFilter[i].x / u_size.y * u_direction.y ) ) * gaussFilter[i].y;"
  "  }"
  "  gl_FragColor = color;"
  "}"
  "";

GLGaussShader::GLGaussShader()
{
  mType = GL_FRAGMENT_SHADER;
  mSource = GaussShaderSource;

  if(mInitialized = compile()) {
    mUniformLocations["u_direction"] = (GLint) glGetUniformLocation(mProgram, "u_direction");
    mUniformLocations["u_size"] = glGetUniformLocation(mProgram, "u_size");
    GL_ASSERT();
  }
}

GLGaussShader::~GLGaussShader()
{
}